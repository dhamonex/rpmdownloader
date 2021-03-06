/***************************************************************************
 *   Copyright (C) 2008 by Dirk Hartmann                                   *
 *   2monex@gmx.net                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "yumrepositorycontentdownloader.h"

#include "checksumcheck.h"
#include "yumcachebuilder.h"
#include "yumfileconstants.h"
#include "repositorysqlitecontentlister.h"
#include "rpmdownloadersettings.h"
#include "rdhttp.h"

#include <QFtp>
#include <QUrl>

YumRepositoryContentDownloader::YumRepositoryContentDownloader ( QObject *parent )
    : AbstractContentDownloader ( parent ), state ( FINISHED )
{
  checkSumChecker = new CheckSumCheck ( this );
  cacheBuilder = new YumCacheBuilder ( this );

  connect ( ftp, SIGNAL ( done ( bool ) ), this, SLOT ( ftpDownloadFinished ( bool ) ) );
  connect ( http, SIGNAL ( downloadWithRedirectToFileFinished ( bool ) ), this, SLOT ( httpDownloadFinished ( bool ) ) );

  connect ( checkSumChecker, SIGNAL ( checkFinished ( bool ) ), this, SLOT ( checkSumFinished ( bool ) ) );
  connect ( checkSumChecker, SIGNAL ( checkFailed ( QString ) ), this, SLOT ( checkSumError ( QString ) ) );

  connect ( cacheBuilder, SIGNAL ( buildFinished ( bool ) ), this, SLOT ( cacheBuildFinished ( bool ) ) );
}

YumRepositoryContentDownloader::~YumRepositoryContentDownloader()
{
}

void YumRepositoryContentDownloader::startContentUpdate ( int profileNumber, const QString &databasePath, const QString &repoName, const QString &serverUrl, const QStringList &architectures )
{
  // qDebug("start yum content update server %s", qPrintable(serverUrl));

  repoUrl = serverUrl;
  archs = architectures;
  curProfile = profileNumber;
  dbPath = databasePath;
  this->repoName = repoName;

  aborted = false; // reset aborted var


  repomdMetaInfos.clear();

  if ( !createDirsIfNeeded() ) {
    errMsg = tr ( "Could not create cache or temp Directory.\nVerify that your settings are correct and that both directories are writable for you" );
    abortContentUpdate();
    return;
  }

  processNextState();
}

void YumRepositoryContentDownloader::abortContentUpdate ( const bool userCancelled )
{
  if ( aborted )
    return;

  aborted = true;

  // qDebug("cancel content update %s %s", qPrintable(repoUrl), qPrintable(currentFile.fileName()));

  closeConnections();

  if ( currentFile.exists() ) {
    currentFile.close();
    currentFile.remove();
  }

  ftp->abort();

  http->abort();

  if ( state == BUILDCACHE )
    cacheBuilder->terminate();

  removeRepoFiles(); // remove repos profiles that forces an refresh next time for this unfinished profile

  state = FINISHED;

  if ( !userCancelled )
    emit ( finished ( curProfile, true ) );
}

void YumRepositoryContentDownloader::cancelContentUpdate()
{
  abortContentUpdate ( true );

  // emit(finished(curProfile, false));
}

void YumRepositoryContentDownloader::processNextState()
{
  if ( aborted )
    return;

  if ( state == FINISHED ) { // start new refresh
    state = REPOMDDOWNLOAD;
    fetchRepoMd();

  } else if ( state == REPOMDDOWNLOAD ) {
    if ( !repoUpToDate ( true ) ) { // with database checking
      if ( aborted ) // check aborted again becuase it could have changed in last function
        return;

      if (!refreshRepo()) {
        abortContentUpdate();
      }

    } else {
      if ( aborted )
        return;

      // repo seems to be up to date but are the files stored in cache correct?
      // better do a checksum check
      state = PRECHECKSUMCHECK;

      checksumCheck();
    }

  } else if ( state == PRECHECKSUMCHECK ) {
    // read cached values and emit file list
    // cache was up to date
    readContent();

  } else if ( state == DOWNLOADOTHERS ) {
    // do another checksum check
    // for the new downloaded files

    if ( !repoUpToDate() ) { // just one more check but this time without database checking
      if ( errMsg.isEmpty() )
        errMsg = tr ( "Unknown error: Cannot refresh repository" );

      abortContentUpdate();

      return;
    }

    state = POSTCHECKSUMCHECK;

    checksumCheck();

  } else if ( state == POSTCHECKSUMCHECK ) {
    // now build cache
    buildCache();

  } else if ( state == BUILDCACHE ) {
    readContent();
  }
}

bool YumRepositoryContentDownloader::createDirsIfNeeded()
{
  if ( !rpmDownloaderSettings().cacheDir().exists() ) { // try create one
    if ( !rpmDownloaderSettings().cacheDir().mkpath ( rpmDownloaderSettings().cacheDir().absolutePath() ) )
      return false;
  }

  if ( !rpmDownloaderSettings().tempDir().exists() ) { // create tmp dir
    if ( !rpmDownloaderSettings().tempDir().mkpath ( rpmDownloaderSettings().tempDir().absolutePath() ) )
      return false;
  }

  return true;
}

void YumRepositoryContentDownloader::ftpDownloadFinished ( bool error )
{
  currentFile.close();
  currentFile.setFileName ( "" );

  if ( error ) {
    if ( !aborted ) {
      errMsg = tr ( "FTP Error %1" ).arg ( ftp->errorString() );
      abortContentUpdate();
    }

    return;
  }

  downloadFiles();
}

void YumRepositoryContentDownloader::httpDownloadFinished ( bool error )
{
  currentFile.close();

  if ( error ) {
    if ( !aborted ) {
      errMsg = tr ( "HTTP Error %1" ).arg ( http->errorString() );
      abortContentUpdate();
    }

    return;
  }

  downloadFiles();
}

void YumRepositoryContentDownloader::downloadFiles()
{
  if ( filesToDownload.isEmpty() ) {
    processNextState();
    return;
  }

  QUrl nextUrl ( filesToDownload.keys().at ( 0 ) );

  QString destination ( filesToDownload.take ( nextUrl ) );

  downloadFile ( nextUrl, destination );
}

void YumRepositoryContentDownloader::downloadFile ( const QUrl & url, const QString & destination )
{
  currentFile.close();
  currentFile.setFileName ( destination );

  if ( !currentFile.open ( QIODevice::WriteOnly ) ) {
    errMsg = tr ( "Cannot Open File %1 for writing: %2" ).arg ( currentFile.fileName() ).arg ( currentFile.errorString() );

    abortContentUpdate();

    return;
  }

  if ( url.scheme() == "ftp" ) {
    ftp->clearPendingCommands(); // clear all pending commands

    if ( ftp->state() != QFtp::Unconnected ) { // disconnect if already connected
      ftp->abort();
      ftp->close();
    }

    ftp->connectToHost ( url.host(), url.port ( 21 ) );

    ftp->login();

    ftp->get ( url.path(), &currentFile );
    ftp->close();

  } else {
    // http->clearPendingRequests();
    if ( http->state() != QHttp::Unconnected ) {
      http->abort();
      http->closeConnection();
    }

    http->setHost ( url.host(), url.port ( 80 ) );

    QHttpRequestHeader header ( "GET", url.path() );
    header.setValue ( "Host", url.host() );
    http->downloadWithRedirectToFile ( header, &currentFile );
  }
}

void YumRepositoryContentDownloader::fetchRepoMd()
{
  QUrl url ( repoUrl + "/" + Yum::repoServerDir + "/" + Yum::repomdFileName );

  if ( !url.isValid() ) {
    errMsg = tr ( "Invalid URL %1" ).arg ( url.toString() );
    abortContentUpdate();
    return;
  }

  // the destination is /tempdir/reponame.xml
  filesToDownload.insert ( url, getTemporaryRepomdFullPath() );

  downloadFiles();
}

bool YumRepositoryContentDownloader::repoUpToDate ( bool checkDatabase )
{
  YumRepomdDomParser repomdParser;

  if ( !repomdParser.parseRepomd ( getRepomdFullPath() ) ) {
    qDebug ( "failed to parse %s, thats nothing to worry about it will be downloaded again", qPrintable ( getRepomdFullPath() ) );
    // repo is not up to date because parsing failed for some reasons
    // repomd doesn't exist for example.
    errMsg = tr ( "invalid repomd file or repomd was not found on server" );
    return false;
  }

  // clear error when repomd is valid
  errMsg.clear();

  repomdMetaInfos = repomdParser.getParsedMetaInfos();

  // parse the actual downloaded repomd file
  repomdParser.parseRepomd ( getTemporaryRepomdFullPath() );

  QMap<QString, FileMetaInfo> tmpRepomdMetaInfos = repomdParser.getParsedMetaInfos();

  if ( tmpRepomdMetaInfos.value ( "primary" ).timestamp != repomdMetaInfos.value ( "primary" ).timestamp ) {
    // at least one timestamp has changed assume that newer is available
    return false;
  }

  // try to read database contents
  RepositorySqliteContentLister lister;

  lister.fetchContent ( dbPath, repoName, archs );

  if ( checkDatabase && lister.dbWasRecreated() ) {
    // database was corrupted and recreated need to fetch contents again
    return false;
  }

  // otherwise repos seems to be up to date
  return true;
}

bool YumRepositoryContentDownloader::refreshRepo()
{
  state = DOWNLOADOTHERS;

  // first remove old files
  removeRepoFiles();

  QFile file ( getTemporaryRepomdFullPath() );

  if ( !file.exists() || !file.copy ( getRepomdFullPath() ) ) {
    errMsg = tr ( "Could not copy repomd file to cache dir %1. Make sure that the cache directory is writable" ).arg ( rpmDownloaderSettings().cacheDir().absolutePath() );
    return false;
  }
  
  YumRepomdDomParser repomdParser;
  if ( !repomdParser.parseRepomd ( getRepomdFullPath() ) ) {
    errMsg = tr ( "Curious Error, could not parse %1" ).arg( getRepomdFullPath() );
    return false;
  }
  
  repomdMetaInfos = repomdParser.getParsedMetaInfos();
  

  filesToDownload.insert ( QUrl ( repoUrl + "/" + repomdMetaInfos.value( "primary" ).location ), QString ( rpmDownloaderSettings().cacheDir().absolutePath() + "/" + repoName + "/" + Yum::primaryGzFileName ) );

  downloadFiles();
  
  return true;
}

QString YumRepositoryContentDownloader::getTemporaryRepomdFullPath() const
{
  return QString ( rpmDownloaderSettings().tempDir().absolutePath() + "/" + repoName + ".xml" );
}

QString YumRepositoryContentDownloader::getRepomdFullPath() const
{
  return QString ( rpmDownloaderSettings().cacheDir().absolutePath() + "/" + repoName + "/" + Yum::repomdFileName );
}

void YumRepositoryContentDownloader::checksumCheck()
{
  if ( repomdMetaInfos.isEmpty() ) {
    processNextState(); // go to next state

  } else {
    if ( !repomdMetaInfos.contains ( "primary" ) ) {
      abortContentUpdate();
      processNextState();
      return;
    }

    FileMetaInfo metaInfo = repomdMetaInfos.take ( "primary" );

    // only primary is downloaded and needed
    QString fileName ( rpmDownloaderSettings().cacheDir().absolutePath() + "/" + repoName + "/" + Yum::primaryGzFileName );
    checkSumChecker->setCheckSumCommand ( rpmDownloaderSettings().checksumCommand() );
    checkSumChecker->setChecksumAlgorithm(metaInfo.checksumAlgorithm);
    checkSumChecker->checkSumCheckForFile ( fileName, metaInfo.checksum );
  }
}

void YumRepositoryContentDownloader::removeRepoFiles()
{
  QFile::remove ( getRepomdFullPath() );
  QFile::remove ( rpmDownloaderSettings().cacheDir().absolutePath() + "/" + Yum::primaryGzFileName );
}

void YumRepositoryContentDownloader::checkSumFinished ( bool ok )
{
  if ( !ok ) { // checksum failed
    if ( state == PRECHECKSUMCHECK ) {
      // need to refresh repository when post check is running
      // else the refresh failed so do not make another refresh now
      // the timer in the widget will try another refresh later
      refreshRepo();

    } else {
      // accept that refresh was not possible this time (see above)
      abortContentUpdate();
    }

  } else {
    processNextState();
  }
}

void YumRepositoryContentDownloader::checkSumError ( QString error )
{
  abortContentUpdate();
  errMsg = error;
}

void YumRepositoryContentDownloader::buildCache()
{
  state = BUILDCACHE;
  cacheBuilder->setGunzipCommand ( rpmDownloaderSettings().gunzipCommand() );
  cacheBuilder->buildCache ( dbPath, repoName, rpmDownloaderSettings().cacheDir().absolutePath() + "/" + repoName );
}

void YumRepositoryContentDownloader::cacheBuildFinished ( bool success )
{
  if ( !success ) {
    errMsg = cacheBuilder->readError();
    abortContentUpdate();
    return;
  }

  processNextState();
}

void YumRepositoryContentDownloader::readContent()
{
  state = FINISHED;
  RepositorySqliteContentLister dbHandler;

  if ( !dbHandler.fetchContent ( rpmDownloaderSettings().cacheDir().absolutePath() + "/" + repoName + "/" + repoName + ".db", repoName, archs ) ) {
    refreshRepo();
    return;
  }

  emit ( finished ( curProfile, false ) );
}

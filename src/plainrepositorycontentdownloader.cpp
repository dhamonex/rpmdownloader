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
#include "plainrepositorycontentdownloader.h"

#include "packagemetadata.h"
#include "rpmdownloadersettings.h"

#include <QApplication>

size_t PlainRepositoryContentDownloader::plainContentCallback( char *ptr, size_t size, size_t nmemb, void *userdata )
{
  PlainRepositoryContentDownloader *plainContentUpdate = reinterpret_cast<PlainRepositoryContentDownloader *>( userdata );
  
  if ( plainContentUpdate->aborted ) {
    return CURLE_WRITE_ERROR;
  }
  size_t realsize = size * nmemb;
  
  
  
  return realsize;
}


PlainRepositoryContentDownloader::PlainRepositoryContentDownloader ( QObject *parent )
    : AbstractContentDownloader ( parent ), isActive ( false )
{
//   connect ( ftp, SIGNAL ( listInfo ( QUrlInfo ) ), this, SLOT ( newFtpContentsLine ( const QUrlInfo& ) ) );
//   connect ( ftp, SIGNAL ( done ( bool ) ), this, SLOT ( ftpFinished ( bool ) ) );
//   connect ( http, SIGNAL ( done ( bool ) ), this, SLOT ( httpFinished ( bool ) ) );
}


PlainRepositoryContentDownloader::~PlainRepositoryContentDownloader()
{
}

void PlainRepositoryContentDownloader::startContentUpdate ( int profileNumber, const QString &databasePath, const QString &repoName, const QString &serverUrl, const QStringList &architectures )
{
  // qDebug("start content update for %i url: %s", profileNumber, qPrintable(serverUrl));
  // store variables
  repoUrl = serverUrl;
  archs = architectures;
  curProfile = profileNumber;
  this->repoName = repoName;
  dbPath = databasePath;

  aborted = false; // reset aborted var
  updatedArchs.clear();

  if ( !initDb() ) {
    emit ( finished ( curProfile, true ) );
    return;
  }

  isActive = true;

  updateNextArch();
}

void PlainRepositoryContentDownloader::abortContentUpdate( const bool userCancelled )
{
  aborted = true;

  if ( isActive )
    dbHandler.rollback();

//   http->abort();

//   ftp->abort();

  isActive = false;

  if ( !userCancelled )
    emit ( finished ( curProfile, true ) );
}

void PlainRepositoryContentDownloader::cancelContentUpdate()
{
  abortContentUpdate ( true ); // this time cancelled by user

  // emit(finished(curProfile, false));
}

void PlainRepositoryContentDownloader::updateNextArch()
{
  bool allUpdated = true;

  foreach ( const QString &arch , archs ) {
    if ( !updatedArchs.contains ( arch ) ) {
      allUpdated = false;

      // set current arch
      currentArch = arch;

      break; // found one
    }
  }

  updatedArchs << currentArch; // add current arch to updated archs

  if ( allUpdated && !aborted ) {
    isActive = false;
    dbHandler.finish();
    emit ( finished ( curProfile, false ) );

  } else {
    QUrl url ( repoUrl + "/" + currentArch );

    if ( !url.isValid() ) { // got an error abort all
      abortContentUpdate();
      errMsg = tr ( "Invalid url: %1" ).arg ( url.toString() );
      return;
    }

    if ( url.scheme() == "ftp" ) { // use ftp
//       startFtpListCommand ( url );

    } else if ( url.scheme() == "http" ) { // use http
//       startHttpIndexCommand ( url );

    } else {
      abortContentUpdate();
      errMsg = tr ( "url scheme %1 not supported"
                    "Please select ftp or http" ).arg ( url.scheme() );
      return;
    }
  }
}

void PlainRepositoryContentDownloader::downloadFinished()
{
  if ( aborted ) {
    return;
  }
}

// void PlainRepositoryContentDownloader::startFtpListCommand ( const QUrl & url )
// {
//   ftp->clearPendingCommands(); // clear all pending commands
// 
//   if ( ftp->state() != QFtp::Unconnected ) { // disconnect if already connected
//     ftp->abort();
//     ftp->close();
//   }
// 
//   ftp->connectToHost ( url.host(), url.port ( 21 ) );
// 
//   ftp->login();
//   ftp->cd ( url.path() );
//   ftp->list();
//   ftp->close();
// }

// void PlainRepositoryContentDownloader::newFtpContentsLine ( const QUrlInfo & i )
// {
//   if ( i.isFile() ) { // only files are relevant
//     PackageMetaData metaData ( i.name(), currentArch );
//     metaData.setSize ( i.size() );
// 
//     if ( !dbHandler.insertPackage ( metaData, false ) )
//       abortContentUpdate();
//   }
// }

// void PlainRepositoryContentDownloader::ftpFinished ( bool error )
// {
//   if ( error && !aborted ) {
//     errMsg = tr ( "FTP error: %1" ).arg ( ftp->errorString() );
//     abortContentUpdate();
//     return;
//   }
// 
//   if ( !aborted )
//     updateNextArch();
// }

// void PlainRepositoryContentDownloader::httpFinished ( bool error )
// {
//   if ( error && !aborted ) {
//     errMsg = tr ( "HTTP error %1" ).arg ( http->errorString() );
//     abortContentUpdate();
//     return;
//   }
// 
//   QString contents = http->readAll();
// 
//   // qDebug("contents %s", qPrintable(contents));
//   QStringList contentLines = contents.split ( "\n" );
//   qDebug ( "%i", contentLines.size() );
// 
//   for ( int i = 0; i < contentLines.size(); ++i ) {
//     if ( ( i % 500 ) == 0 )
//       qApp->processEvents(); // process events
// 
//     QRegExp htmlLinkRegExp ( ".*<A HREF=.*>(\\S*)</A>.*(\\d+[.|]\\d*[M|B|K])" );
// 
//     htmlLinkRegExp.setCaseSensitivity ( Qt::CaseInsensitive );
// 
//     if ( htmlLinkRegExp.indexIn ( contentLines.at ( i ) ) != -1 ) {
//       PackageMetaData metaData ( htmlLinkRegExp.cap ( 1 ), currentArch );
//       metaData.setSize ( htmlLinkRegExp.cap ( 2 ) );
// 
//       if ( !dbHandler.insertPackage ( metaData, false ) ) {
//         abortContentUpdate();
//         return;
//       }
// 
//       // qDebug("rpm %s, size %s", qPrintable(htmlLinkRegExp.cap(1)), qPrintable(htmlLinkRegExp.cap(2)));
//     }
//   }
// 
//   updateNextArch();
// }

bool PlainRepositoryContentDownloader::initDb()
{
  if ( !dbHandler.init ( dbPath, repoName ) ) {
    errMsg = tr ( "Could not open Database: %1" ).arg ( dbHandler.errorMsg() );
    abortContentUpdate();
    return false;
  }

  dbHandler.clear();

  return true;
}

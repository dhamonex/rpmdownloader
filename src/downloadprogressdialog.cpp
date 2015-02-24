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
#include "downloadprogressdialog.h"

#include "rpmdownloadersettings.h"
#include "checksumcheck.h"

DownloadProgressDialog::DownloadProgressDialog ( QWidget* parent, Qt::WFlags fl )
    : QDialog ( parent, fl ), Ui::DownloadProgressDialog(),
    currentProfile ( -1 ), gotError ( false ), nothingTodo ( false ), redirected ( false ),
    haveOverallDownloadSize ( false )
{
  setupUi ( this );

//   ftp = new QFtp ( this );
//   http = new RDHttp ( this );
  checkSumChecker = new CheckSumCheck ( this );
  layout()->setSizeConstraint ( QLayout::SetFixedSize );

  connect ( this, SIGNAL ( readyForDownload() ), this, SLOT ( startDownload() ) );
  connect ( abortPushButton, SIGNAL ( clicked() ), this, SLOT ( abortDownloads() ) );

//   connect ( ftp, SIGNAL ( dataTransferProgress ( qint64, qint64 ) ), this, SLOT ( updateTransferProgress ( qint64, qint64 ) ) );
//   connect ( ftp, SIGNAL ( done ( bool ) ), this, SLOT ( ftpFinished ( bool ) ) );

//   connect ( http, SIGNAL ( dataReadProgress ( int, int ) ), this, SLOT ( httpTransferData ( int, int ) ) );
//   connect ( http, SIGNAL ( downloadWithRedirectToFileFinished ( bool ) ), this, SLOT ( httpFinished ( bool ) ) );

  connect ( checkSumChecker, SIGNAL ( checkFinished ( bool ) ), this, SLOT ( checkSumFinished ( bool ) ) );
  connect ( checkSumChecker, SIGNAL ( checkFailed ( QString ) ), this, SLOT ( checkSumError ( QString ) ) );
}

DownloadProgressDialog::~DownloadProgressDialog()
{

}

void DownloadProgressDialog::setProfilesForDownload ( const QList< RepositoryProfile *> profiles )
{
  this->profiles = profiles;
}

void DownloadProgressDialog::setProfilesForDownload ( RepositoryProfile* profile )
{
  profiles.append ( profile );
}

void DownloadProgressDialog::abortDownloads()
{
//   /*if ( ftp->state() != QFtp::Unconnected ) {
//     ftp->clearPendingCommands();
//     ftp->close();
//     // ftp->abort();
//   }
// 
//   if ( http->state() != QHttp::Unconnected ) {
//     http->clearPendingRequests();
//     http->closeConnection();
//     // http->abort();
//   }

  if ( oldProfileUrl.isValid() && currentProfile >= 0 ) {
    profiles[currentProfile]->setServerUrl ( oldProfileUrl );
  }

  currentFile.remove(); // remove unfinished rpm

  reject(); // reject indicates an error
}

void DownloadProgressDialog::startDownload()
{
  gotError = false;
  nothingTodo = false;

  errorMsg = tr ( "Interrupted through user." );

  checkSumChecker->setCheckSumCommand ( rpmDownloaderSettings().checksumCommand() );

  currentProfile = -1;

  overallProgressBar->setMaximum ( overallDownloadSize() );

  if ( !haveOverallDownloadSize )
    overallProgressBar->setMaximum ( numberOfRpms );

  overallProgressBar->setValue ( 0 );

  currentRpmProgressBar->setValue ( 0 );

  if ( profiles.size() < 1 || numberOfRpms < 1 ) {
    nothingTodo = true;
  }

  downloadNextProfile();
}

void DownloadProgressDialog::downloadNextProfile()
{
  if ( currentProfile >= 0 && oldProfileUrl.isValid() ) {
    // restore old profile url
    profiles[currentProfile]->setServerUrl ( oldProfileUrl );
    oldProfileUrl.clear();
  }

  if ( currentProfile + 1 < profiles.size() ) {
    ++currentProfile;
    packageMetaDatasOfCurrentProfile = profiles.at ( currentProfile )->packageMetaDatasOfpackagesToDownload();
    downloadNextRpm();

  } else {
    accept(); // accept to quit;
  }
}

void DownloadProgressDialog::downloadNextRpm()
{
  if ( packageMetaDatasOfCurrentProfile.size() < 1 ) {
    downloadNextProfile();

  } else {
    currentRpm = packageMetaDatasOfCurrentProfile.takeFirst();
    downloadCurrentPackage();
  }
}

void DownloadProgressDialog::downloadCurrentPackage()
{
  currentArch = currentRpm.architecture();

  currentOnlineFilename = currentRpm.fileName();

  // start ftp download
  currentFile.setFileName ( profiles.at ( currentProfile )->downloadDir() + "/" + currentArch + "/" + currentOnlineFilename );

  QDir dir ( profiles.at ( currentProfile )->downloadDir() + "/" + currentArch );

  if ( !dir.exists() ) {
    if ( !dir.mkpath ( profiles.at ( currentProfile )->downloadDir() + "/" + currentArch ) ) {
      errorMsg = tr ( "Could not create download directory: %1" ).arg ( profiles.at ( currentProfile )->downloadDir() + "/" + currentArch );
      reject();
      return;
    }
  }

  if ( !currentFile.open ( QIODevice::WriteOnly ) ) {
    errorMsg = tr ( "Could not open file %1 for writing: %2" ).arg ( currentFile.fileName() ).arg ( currentFile.errorString() );
    reject();
    return;
  }

  QUrl url;

  if ( currentRpm.location().isEmpty() ) // have no location
    url.setUrl ( profiles.at ( currentProfile )->serverUrl().toString() + "/" + currentArch + "/" + currentOnlineFilename );
  else // can use location href so that I don't need to add the architecture to the url which is more flexible
    url.setUrl ( profiles.at ( currentProfile )->serverUrl().toString() + "/" + currentRpm.location() );

  if ( !url.isValid() ) {
    errorMsg = tr ( "Invalid url: %1" ).arg ( url.toString() );
    reject();
    return;
  }

  rpmNameLabel->setText ( currentOnlineFilename );

  if ( url.scheme() == "ftp" ) {
//     ftp->connectToHost ( url.host(), url.port ( 21 ) );
//     ftp->login();
//     ftp->get ( url.path(), &currentFile );
//     ftp->close();*/
    qDebug ( "Downloading %s to file %s", qPrintable ( url.toString() ), qPrintable ( currentFile.fileName() ) );

  } else if ( url.scheme() == "http" ) {
//     http->setHost ( url.host(), url.port ( 80 ) );

//     QHttpRequestHeader header ( "GET", url.path() );
//     header.setValue ( "Host", url.host() );
    //http->request(header, 0, &currentFile);
//     http->downloadWithRedirectToFile ( header, &currentFile );

    qDebug ( "Downloading %s to file %s", qPrintable ( url.toString() ), qPrintable ( currentFile.fileName() ) );

  } else {
    errorMsg = tr ( "Scheme %1 is not supported."
                    "Please select ftp or http." ).arg ( url.scheme() );
  }
}

void DownloadProgressDialog::ftpFinished ( bool error )
{
  currentFile.close();
  // qDebug("finished %s", qPrintable(currentFile.fileName()));

  if ( error ) {
//     errorMsg = tr ( "FTP transfer error: %1" ).arg ( ftp->errorString() );
    abortDownloads();

  } else {
    if ( rpmDownloaderSettings().doCheckSumCheckOnDownloadedPackages() && !currentRpm.shaCheckSum().isEmpty() ) {
      checkSumChecker->setChecksumAlgorithm( currentRpm.checkSumAlgorithm() );
      checkSumChecker->checkSumCheckForFile ( currentFile.fileName(), currentRpm.shaCheckSum() );
    } else {
      downloadNext();
    }
  }
}

void DownloadProgressDialog::httpFinished ( bool error )
{
  currentFile.close();

  if ( error ) {
//     errorMsg = tr ( "HTTP transfer error: %1" ).arg ( http->errorString() );
    abortDownloads();
    qDebug ( "http error %s", qPrintable ( errorMsg ) );

  } else {
    if ( rpmDownloaderSettings().doCheckSumCheckOnDownloadedPackages() && !currentRpm.shaCheckSum().isEmpty() ) {
      checkSumChecker->setChecksumAlgorithm( currentRpm.checkSumAlgorithm() );
      checkSumChecker->checkSumCheckForFile ( currentFile.fileName(), currentRpm.shaCheckSum() );
    } else {
      downloadNext();
    }
  }
}

void DownloadProgressDialog::checkSumFinished ( bool ok )
{
  if ( !ok ) {
    qWarning ( "Check sum check failed for %s, removing file", qPrintable ( currentFile.fileName() ) );
    currentFile.remove();

    errorMsg = tr ( "Check sum check failed for %1, something is wrong with that repository please check the profile settings for this repository %2" ).arg ( currentFile.fileName() ).arg ( profiles.at ( currentProfile )->profileName() );

    reject();

    return;
  }

  qDebug ( "Check sum check finished succesfully for %s", qPrintable ( currentFile.fileName() ) );

  downloadNext();
}

void DownloadProgressDialog::checkSumError ( QString error )
{
  errorMsg = tr ( "Could not do a checksum check on %1: %2" ).arg ( currentFile.fileName() ).arg ( error );
  reject();
}

void DownloadProgressDialog::updateTransferProgress ( qint64 done, qint64 total )
{
  if ( total == -1 ) {
    if ( currentRpm.size() == 0 ) // it's unknown
      currentRpmProgressBar->setMaximum ( -1 );
    else
      currentRpmProgressBar->setMaximum ( currentRpm.size() );

  } else {
    currentRpmProgressBar->setMaximum ( total );
  }

  currentRpmProgressBar->setValue ( done );
}

void DownloadProgressDialog::downloadNext()
{
  if ( !haveOverallDownloadSize )
    overallProgressBar->setValue ( overallProgressBar->value() + 1 );
  else
    overallProgressBar->setValue ( overallProgressBar->value() + currentRpm.size() );

  downloadNextRpm();
}

void DownloadProgressDialog::httpTransferData ( int done, int total )
{
  updateTransferProgress ( done, total );
}

void DownloadProgressDialog::setDeleteOldVersion ( bool deleteOld )
{
  deleteOldVersions = deleteOld;
}

int DownloadProgressDialog::exec()
{
  emit ( readyForDownload() );

  if ( gotError )
    return QDialog::Rejected;
  else if ( nothingTodo )
    return QDialog::Accepted;

  return QDialog::exec();
}

void DownloadProgressDialog::reject()
{
  gotError = true;
  QDialog::reject();
}

void DownloadProgressDialog::accept()
{
  QDialog::accept();
}

void DownloadProgressDialog::setNumberOfRpms ( int number )
{
  numberOfRpms = number;
}

qint64 DownloadProgressDialog::overallDownloadSize()
{
  qint64 size = 0;
  QList<RepositoryProfile *>::const_iterator profileIter = profiles.begin();

  for ( ; profileIter != profiles.end(); ++profileIter ) {
    size += ( *profileIter )->downloadSizeInBytes();
  }

  if ( size > 0 )
    haveOverallDownloadSize = true;
  else
    haveOverallDownloadSize = false;

  return size;
}

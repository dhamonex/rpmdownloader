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

int DownloadProgressDialog::curlDownloadCallback( char *ptr, size_t size, size_t nmemb, void *userdata )
{
  DownloadProgressDialog *dialog = static_cast<DownloadProgressDialog *>( userdata );
  if ( dialog->m_aborted ) {
    return CURLE_WRITE_ERROR;
  }
  
  size_t realsize = size * nmemb;
  
  dialog->currentFile.write( ptr, realsize );
  
  return realsize;
}

int DownloadProgressDialog::curlProgressCallback( void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow )
{
//   qDebug( "progress dialog: dltotal %li dlnow %li ultotal %li ulnow %li", dltotal, dlnow, ultotal, ulnow );
  DownloadProgressDialog *dialog = static_cast<DownloadProgressDialog *>( clientp );
  if ( dltotal == 0 ) {
    if ( dialog->currentRpm.size() == 0 ) {
      // size is unknown
      QMetaObject::invokeMethod( dialog->currentRpmProgressBar, "setMaximum", Qt::QueuedConnection, Q_ARG(int, -1 ) );
    } else {
      QMetaObject::invokeMethod( dialog->currentRpmProgressBar, "setMaximum", Qt::QueuedConnection, Q_ARG( int, dialog->currentRpm.size() ) );
    }

  } else {
    QMetaObject::invokeMethod( dialog->currentRpmProgressBar, "setMaximum", Qt::QueuedConnection, Q_ARG( int, dltotal ) );
  }

  QMetaObject::invokeMethod( dialog->currentRpmProgressBar, "setValue", Qt::QueuedConnection, Q_ARG(int, dlnow ) );
  
  return 0;
}


DownloadProgressDialog::DownloadProgressDialog ( QWidget* parent, Qt::WFlags fl )
    : QDialog ( parent, fl ), Ui::DownloadProgressDialog(),
      profiles(), numberOfRpms( 0 ), currentProfile ( -1 ), oldProfileUrl(), deleteOldVersions( false ),  gotError ( false ), nothingTodo ( false ), redirected ( false ),
      haveOverallDownloadSize ( false ), m_aborted( false ), packageMetaDatasOfCurrentProfile(), currentArch(), currentOnlineFilename(), currentRpm(), currentFile(), errorMsg(),
      checkSumChecker(), m_currentUrl(), m_curl( new AsyncCurlHandle( this ) )
{
  setupUi ( this );

  checkSumChecker = new CheckSumCheck ( this );
  layout()->setSizeConstraint ( QLayout::SetFixedSize );

  connect ( this, SIGNAL ( readyForDownload() ), this, SLOT ( startDownload() ) );
  connect ( abortPushButton, SIGNAL ( clicked() ), this, SLOT ( abortDownloads() ) );
  
  connect( m_curl, SIGNAL( finished() ), this, SLOT( downloadFinished() ) );

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
  
  m_aborted = true;

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
  m_aborted = false;

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
  if ( gotError ) {
    reject();
  }
  
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
  if ( gotError ) {
    reject();
  }
  
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

  if ( currentRpm.location().isEmpty() ) { // have no location
    m_currentUrl.setUrl ( profiles.at ( currentProfile )->serverUrl().toString() + "/" + currentArch + "/" + currentOnlineFilename );
  } else { // can use location href so that I don't need to add the architecture to the url which is more flexible
    m_currentUrl.setUrl ( profiles.at ( currentProfile )->serverUrl().toString() + "/" + currentRpm.location() );
  }

  if ( !m_currentUrl.isValid() ) {
    errorMsg = tr ( "Invalid url: %1" ).arg ( m_currentUrl.toString() );
    reject();
    return;
  }

  rpmNameLabel->setText ( currentOnlineFilename );

  if ( m_curl->isRunning() ) {
    errorMsg = tr( "pending download is already is already running" );
    reject();
    return;
  }
  
  curl_easy_setopt( m_curl->get(), CURLOPT_FOLLOWLOCATION, 1L );
  curl_easy_setopt( m_curl->get(), CURLOPT_WRITEFUNCTION, &curlDownloadCallback );
  curl_easy_setopt( m_curl->get(), CURLOPT_WRITEDATA, this );
  curl_easy_setopt( m_curl->get(), CURLOPT_XFERINFOFUNCTION, &curlProgressCallback );
  curl_easy_setopt( m_curl->get(), CURLOPT_XFERINFODATA, this );
  curl_easy_setopt( m_curl->get(), CURLOPT_NOPROGRESS, 0L );
  
  curl_easy_setopt( m_curl->get(), CURLOPT_URL, m_currentUrl.toString().toAscii().data() );
  
  m_curl->perform();

  qDebug ( "Downloading %s to file %s", qPrintable ( m_currentUrl.toString() ), qPrintable ( currentFile.fileName() ) );
}

void DownloadProgressDialog::downloadFinished()
{
  currentFile.close();
  
  if ( m_aborted ) {
    abortDownloads();
    return;
  }
  
  if ( m_curl->result() != CURLE_OK && !m_aborted ) {
    errorMsg = tr( "error on download of %1 : %2" ).arg( m_currentUrl.toString() ).arg( curl_easy_strerror( m_curl->result() ) );
    abortDownloads();
    return;
  }
  
  if ( rpmDownloaderSettings().doCheckSumCheckOnDownloadedPackages() && !currentRpm.shaCheckSum().isEmpty() ) {
    checkSumChecker->setChecksumAlgorithm( currentRpm.checkSumAlgorithm() );
    checkSumChecker->checkSumCheckForFile ( currentFile.fileName(), currentRpm.shaCheckSum() );
  } else {
    downloadNext();
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

void DownloadProgressDialog::downloadNext()
{
  if ( !haveOverallDownloadSize )
    overallProgressBar->setValue ( overallProgressBar->value() + 1 );
  else
    overallProgressBar->setValue ( overallProgressBar->value() + currentRpm.size() );

  downloadNextRpm();
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

/***************************************************************************
 *   Copyright (C) 2009 by Dirk Hartmann                                   *
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

#ifndef DOWNLOADPROGRESSDIALOG_H
#define DOWNLOADPROGRESSDIALOG_H

#include <QtWidgets/QDialog>
#include <QtCore/QList>
#include "ui_downloadprogressdialog.h"
#include "repositoryprofile.h"
#include "rpm.h"
#include "asynccurlhandle.h"

class CheckSumCheck;

// This class downloads all rpms and displays a nice progress bar

class DownloadProgressDialog : public QDialog, private Ui::DownloadProgressDialog
{
    Q_OBJECT
  public:
    DownloadProgressDialog ( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~DownloadProgressDialog();

    void setNumberOfRpms ( int number );
    void setProfilesForDownload ( QList<RepositoryProfile *> profiles );
    void setProfilesForDownload ( RepositoryProfile * profile );
    void setDeleteOldVersion ( bool deleteOld );

    QString getError() const {return errorMsg;}

  signals:
    void readyForDownload();

  public slots:
    int exec(); // overwrite exec
    void reject();
    void accept();

  private slots:
    void abortDownloads();
    void downloadFinished();
        
    void startDownload(); // delete old version if update is available?

    // checksum check
    void checkSumFinished ( bool ok );
    void checkSumError ( QString error );

  private:
    static int curlProgressCallback( void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow );
    static int curlDownloadCallback( char *ptr, size_t size, size_t nmemb, void *userdata );
    
    void downloadNextProfile();
    void downloadNextRpm();
    void downloadNext();
    void downloadCurrentPackage();
    qint64 overallDownloadSize();

    QList<RepositoryProfile *> profiles; // profiles for downloading

    int numberOfRpms;
    int currentProfile; // current processed profile
    QUrl oldProfileUrl; // when following a redirect store old url here and reset it later

    bool deleteOldVersions;
    bool gotError;
    bool nothingTodo;
    bool redirected;
    bool haveOverallDownloadSize;
    
    bool  m_aborted;

    QList<PackageMetaData> packageMetaDatasOfCurrentProfile;
    QString currentArch;
    QString currentOnlineFilename; // the online filename of the current downloaded rpm
    PackageMetaData currentRpm; // the rpm which is currently downloaded
    QFile currentFile; // the current opened file for the download
    QString errorMsg; // error message for the download
    CheckSumCheck *checkSumChecker;
    
    QUrl m_currentUrl;
    AsyncCurlHandle *m_curl;

};

#endif


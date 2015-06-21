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
#ifndef YUMREPOSITORYCONTENTDOWNLOADER_H
#define YUMREPOSITORYCONTENTDOWNLOADER_H

#include "abstractcontentdownloader.h"
#include "yumrepomddomparser.h"

#include <QtCore/QDir>

class CheckSumCheck;
class YumCacheBuilder;

class YumRepositoryContentDownloader : public AbstractContentDownloader
{
    Q_OBJECT
  public:
    YumRepositoryContentDownloader(QObject *parent = 0);
    
    void startContentUpdate(int profileNumber, const QString &databasePath, const QString &repoName, const QString &serverUrl, const QStringList &architectures);
    void cancelContentUpdate();

    ~YumRepositoryContentDownloader();
      
    protected:
    void abortContentUpdate(const bool userCancelled = false);
  
  private slots:
    void downloadFinished();
    
    void checkSumFinished(bool ok);
    void checkSumError(QString error);
    
    void cacheBuildFinished(bool success);
  
  private:
    static size_t yumContentDownloaderCallback( char *ptr, size_t size, size_t nmemb, void *userdata );
    
    void buildCache();
    bool createDirsIfNeeded();
    void fetchRepoMd();
    bool refreshRepo();
    void checksumCheck(); // using sha command for check and waiting for signals because of blocking
    void fetchOthers(); // downloads the other necessary repo files
    void readContent(); // reads the content from the database and emits the gotContent signal
    
    
    QString getTemporaryRepomdFullPath() const;
    QString getRepomdFullPath() const;
    
    void processNextState();
    bool repoUpToDate(bool checkDatabase = false);
    
    void downloadFile(const QUrl &url, const QString &destination);
    void downloadFiles(); // downloads all files specifed in filesToDownload map
    
    void removeRepoFiles(); // removes repomd and primary
    
    enum RefreshStates {REPOMDDOWNLOAD, POSTCHECKSUMCHECK, DOWNLOADOTHERS,
      PRECHECKSUMCHECK,  BUILDCACHE, FINISHED};
    
    RefreshStates state;
    
    QFile currentFile; // the current opened file for the download
    
    // informations from the repomd parser
    QMap<QString, FileMetaInfo> repomdMetaInfos;
    
    QMap<QUrl, QString> filesToDownload; // server path, destination
    
    // for checksum check
    CheckSumCheck *checkSumChecker;
    
    // for building the cache
    YumCacheBuilder *cacheBuilder;

};

#endif



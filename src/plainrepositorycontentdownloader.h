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
#ifndef REPOSITORYCONTENTDOWNLOADER_H
#define REPOSITORYCONTENTDOWNLOADER_H

#include "abstractcontentdownloader.h"
#include "rddatabaseinserter.h"

class PlainRepositoryContentDownloader : public AbstractContentDownloader
{
    Q_OBJECT
  public:
    PlainRepositoryContentDownloader ( QObject *parent = 0 );

    void startContentUpdate ( int profileNumber, const QString &databasePath, const QString &repoName, const QString &serverUrl, const QStringList &architectures );

    void cancelContentUpdate();

    ~PlainRepositoryContentDownloader();

  protected:
    void abortContentUpdate ( const bool userCancelled = false );

  private slots:
    void downloadFinished();

  private:
    static size_t plainContentCallback( char *ptr, size_t size, size_t nmemb, void *userdata );
    
    void updateNextArch();
//     void startFtpListCommand ( const QUrl &url );
//     void startHttpIndexCommand ( const QUrl &url );
    void parseContents();
    bool initDb();

    RDDatabaseInserter dbHandler;
    QStringList updatedArchs;
    bool isActive;
    
    QUrl m_currentUrl;
    QByteArray m_contents;
};

#endif

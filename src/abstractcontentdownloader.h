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
#ifndef ABSTRACTCONTENTDOWNLOADER_H
#define ABSTRACTCONTENTDOWNLOADER_H

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QDir>

#include "asynccurlhandle.h"
#include "repositoryprofile.h"

class AbstractContentDownloader : public QObject
{
    Q_OBJECT
  public:
    AbstractContentDownloader ( QObject *parent = 0 );

    virtual void startContentUpdate ( int profileNumber, const QString &databasePath, const QString &repoName, const QString &serverUrl, const QStringList &architectures ) = 0;
    virtual void cancelContentUpdate() = 0;

    virtual ~AbstractContentDownloader();

    // when profilenumber changes during update, then use this function
    void changeCurrentProfileNumber ( const int newNumber ) {curProfile = newNumber;}

    QString readError() const {return errMsg;}

  signals:
    void finished ( int, bool ); // finished successful or not


  protected:
    virtual void abortContentUpdate ( const bool userCancelled = false ) = 0; // internal abort

    int curProfile;
    QString repoName;
    QString dbPath;
    volatile bool aborted;
    
    AsyncCurlHandle *m_curl;

    QStringList archs;
    QString repoUrl;
    QString currentArch;

    QString errMsg;

};

#endif

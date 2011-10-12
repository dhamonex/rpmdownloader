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
#ifndef RDPACKAGELISTERTHREAD_H
#define RDPACKAGELISTERTHREAD_H

#include <QThread>
#include "repositorysqlitecontentlister.h"

class QDir;

class RDPackageListerThread : public QThread
{
  public:
    RDPackageListerThread ( QObject* parent = 0 );

    void setDatabasePath ( const QString &dbPath ) {databasePath = dbPath;}

    void setArchitectures ( const QStringList &archs ) {architectures = archs;}

    void setRepoName ( const QString &name ) {repoName = name;}

    void setDownloadPath ( const QString &path ) {localPackagePath = path;} // for reading the local contents

    MultipleArchMetaData
    getRepoPackageInformations() const {return repositoryMetaInformations;}

    MultipleArchMetaData
    getLocalPackageInformations() const {return localPackageinformations;}

    bool finishedSuccesfull() const {return error;}

    QString readErrorMsg() const {return errMsg;}

    ~RDPackageListerThread();

  protected:
    void run();

  private:
    void getLocalContents();
    MultiplePackageMetaData
    getContentsFromDir ( const QDir &dir, const QString &arch );

    QStringList architectures;
    MultipleArchMetaData
    repositoryMetaInformations;
    MultipleArchMetaData
    localPackageinformations;
    QString repoName;
    QString localPackagePath;

    QString databasePath;
    bool error;
    QString errMsg;

};

#endif

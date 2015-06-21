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
#ifndef RPM_H
#define RPM_H

#include "rdnamespace.h"
#include "packagemetadata.h"

#include <QtCore/QString>

class Rpm
{
  public:
    Rpm();
    Rpm ( const QString &rpmname, const QString &architecture );

    void setlocalMetaData ( const PackageMetaData &metaData, const bool recomputeStatus = true );
    void setRemoteMetaData ( const PackageMetaData &metaData, const bool recomputeStatus = true );
    void setName ( const QString &name ) {this->name = name;}

    QString localVersion() const {return localMetaData.version();}

    QString availableVersion() const {return remoteMetaData.version();}

    QString localFileName() const {return localMetaData.fileName();}

    QString availFileName() const {return remoteMetaData.fileName();}

    QString rpmName() const {return name;}

    QString architecture() const {return arch;}

    PackageMetaData rMetaData() const {return remoteMetaData;}

    PackageMetaData lMetaData() const {return localMetaData;}

    void rpmUpdated ( const QString &baseDownloadpath, bool deleteOldversion = true ); // rpm was downloaded avail version is now version

    bool removeLocalVersion ( const QString &baseDownloadPath, const bool recomputeStatus = true );

    void clearStatus() {status = UNKNOWN;}

    Status rpmStatus() const {return status;}

  private:
    void computeStatus();
    bool verifyMetaData ( const PackageMetaData &metaData );

    // definition for versions: 	 0 == unknown
    //				-1 == not available
    //			 all other == version
    QString name;
    QString arch;
    PackageMetaData localMetaData;
    PackageMetaData remoteMetaData;
    Status status;
};

#endif

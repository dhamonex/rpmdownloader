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
#ifndef PACKAGE_H
#define PACKAGE_H

#include <QtCore/QString>
#include <QtCore/QDir>
#include <QtCore/QUrl>
#include <QtCore/QStringList>
#include <QtCore/QMap>

#include "rdnamespace.h"
#include "packagemetadata.h"
#include "rpm.h"

typedef QMap<QString, QString> PackageVersions; // arch version

class Package
{
  public:
    Package();
    Package ( const QString &packageName );

    QString packageName() const {return name;}

    void setPackageName ( const QString &packageName );

    void setArchitectures ( const QStringList &archs );

    PackageVersions localVersions() const;
    PackageVersions availableversions() const;

    void packageUpdated ( const QString &repoDownloadPath, const bool removeOldVersion = false ); // available files are now local files if status AVAILABLE

    void setRemoteVersions ( const MetaData::MultipleArchMetaData &metaDatas );
    void setLocalVersions ( const MetaData::MultipleArchMetaData &metaDatas );

    // interface for getting the package meta data
    MetaData::ArchitecutreDependentMetaData getRemotePackageMetaData() const;
    MetaData::ArchitecutreDependentMetaData getLocalPackageMetaData() const;

    QList<PackageMetaData> requiredForDownload() const;

    void clearStatus(); // completly resets the RPM status/versions to defaults

    bool deleteLocalFiles ( const QString &directory );

    Status packageStatus() const {return status;}

    // ~Package();

  private:
    void computeStatus();
    PackageMetaData generateFailedMetaData ( const QString &arch ) const;

    QString name;
    QMap<QString, Rpm> rpms;
    QStringList architectures;
    Status status; // the status of the package

};

#endif

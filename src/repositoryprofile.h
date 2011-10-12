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
#ifndef REPOSITORYPROFILE_H
#define REPOSITORYPROFILE_H

#include <QString>
#include <QDir>
#include <QMultiHash>
#include <QUrl>
#include <QObject>

#include "rdnamespace.h"
#include "package.h"
#include "packagemetadata.h"

class RDPackageListerThread;

enum RepositoryType {PLAIN, YUM};

class RepositoryProfile : public QObject
{
    Q_OBJECT
  public:
    RepositoryProfile ( QObject *parent = 0 );

    QString profileName() const {return curProfileName;}

    void setProfileName ( const QString &newProfileName, bool renameDirectory = true );

    QUrl serverUrl() const {return curUrl;}

    void setServerUrl ( const QUrl &newUrl );
    void setServerUrl ( const QString &newUrl ); //overloaded member function, provided for convenience.

    QString downloadDir() const;
    void setDownloadDir ( const QString &newDownloadDir );

    QString databaseFile() const; // returns the complete path to the sqlite database file

    RepositoryType repoType() const {return repositoryType;}

    void setRepoType ( RepositoryType type );

    RPM::Architectures architecures() const {return curArchitectures;}

    void setArchitectures ( const RPM::Architectures newArchitectures );

    void addPackage ( const Package &package );
    void removePackage ( const QString &packageName ); // doesn't recompute status
    void changePackageName ( const QString &oldName, const QString &newName );

    bool containsPackage ( const QString &name ) const;

    bool deleteAllPackagesFromDisk();
    bool deletePackageFromDisk ( const QString &rpmName, const bool recomputeStatus = true );

    void removeCacheDir(); // removes the cache dir for this profile e.g. when it is removed

    void clearStatus(); // clears the status of the profile includign all RPMS

    void packagesUpdated ( const bool removeOldVersions ); // means the packages with status AVAILABLE or UPDATE were updated and remote file is now local file
    // void packageUpdated(QString packageName, QString architecture, const bool removeOldVersions);

    int numberOfPackagesToDownload() const;
    QList<PackageMetaData> packageMetaDatasOfpackagesToDownload() const; // returns the filenames which sould be downloaded

    QHash<QString, Package> selectedPackages() const {return packages;} // returns all packages selected dor this repo

    void computeProfileStatus();
    Status profileStatus() const {return status;}

    // Status packageStatus(const QString &packageName) const {return packages.value(packageName).packageStatus();}

    int numberOfSelectedPackages() const; // return number of tpms
    int numberOfOkPackages() const {return okPackages;} // number of rpms with status = OK

    int numberOfUnknownPackages() const {return unknownPackages;} // number of rpms with status = UNKNOWN

    int numberOfFailedPackages() const {return failedPackages;} // number of rpms with status = FAILED

    int numberOfAvailablePackages() const {return availPackages;} // number of rpms with status = AVAILABLE

    int numberOfLocalAvailPackages() const {return localAvailPackages;} // number of rpms with status = LOCALAVAIL

    int numberOfUpdatedPackages() const {return updatedPackages;} // number of rpms with status = UPDATE

    qint64 downloadSizeInBytes() const {return requiredDownlodSize;}

    QStringList getProvidedPackages() const {return repoContent;} // get all packages provided by this repo

    Status getPackageStatus ( const QString &packageName ) const;
    PackageVersions getLocalPackageVersions ( const QString &packageName ) const;
    PackageVersions getRemotePackageVersions ( const QString &packageName ) const;

    // for displaying details about the package
    MetaData::ArchitecutreDependentMetaData getLocalMetaData ( const QString &packageName ) const;
    MetaData::ArchitecutreDependentMetaData getRemoteMetaData ( const QString &packageName ) const;

    const Package getPackageData ( const QString &packageName ) const;

    void refreshStatus(); // call this function when database content was updated or to compute initial status

    bool hasPackage ( const QString &packageName ) const; // is package already selected?

    int cleanupOrphanedPackages(); // removes orphaned packages from disks, if status = local avail and from the package selecion if status = failed or status = local avail

    static void clearDirectory ( const QString &path );

    ~RepositoryProfile();

  signals:
    void statusRefreshed();

  private slots:
    void threadFinished();

  private:
    static void renameDatabaseFile ( const QString &oldName, const QString &newName );
    static void renameRepoCacheDir ( const QString &path, const QString &oldName, const QString &newName );
    static void removeDirectory ( const QString &path );

    void clearStats();
    void updatePackagesStatus(); // uses the available infomations to update the packages (ALL packages)
    void updatePackageStatus ( const QString &packageName, bool recomputeStatus = true );

    static qint64 getRequiredDownloadSizeForPackage ( const Package &package );

    QString curProfileName;
    QUrl curUrl;
    QDir curDownloadDir;
    RPM::Architectures curArchitectures;
    RepositoryType repositoryType;

    Status status;
    QHash<QString, Package> packages;
    MetaData::MultipleArchMetaData repositoryContent;
    MetaData::MultipleArchMetaData localContent;
    QStringList repoContent; // provides a list of available package names

    // for stats
    int okPackages;
    int availPackages;
    int updatedPackages;
    int failedPackages;
    int localAvailPackages;
    int unknownPackages;
    qint64 requiredDownlodSize;

    // thread for content reading
    RDPackageListerThread *lister;
};

#endif

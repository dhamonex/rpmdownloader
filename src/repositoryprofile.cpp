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
#include "repositoryprofile.h"
#include "rdpackagelisterthread.h"
#include "rpmdownloadersettings.h"
#include "rddatabasehandler.h"

#include <QApplication>
#include <QSet>

RepositoryProfile::RepositoryProfile ( QObject *parent )
    : QObject ( parent ), curProfileName ( QApplication::translate ( "RepositoryProfile", "new profile" ) ),
    curUrl ( "ftp://" ),
    curDownloadDir ( QDir::current() ),
    curArchitectures ( RPM::i586 ),
    repositoryType ( YUM ),
    status ( UNKNOWN )
{
  clearStats();

  lister = new RDPackageListerThread ( this );
  connect ( lister, SIGNAL ( finished() ), this, SLOT ( threadFinished() ) );
}


RepositoryProfile::~RepositoryProfile()
{
  // remove existing db connection
  RDDatabaseHandler::removeDbConnection ( curProfileName );
}

void RepositoryProfile::clearStats()
{
  okPackages = 0;
  availPackages = 0;
  updatedPackages = 0;
  failedPackages = 0;
  localAvailPackages = 0;
  unknownPackages = 0;
  requiredDownlodSize = -1; // indicates is unknown
}

void RepositoryProfile::setServerUrl ( const QUrl & newUrl )
{
  curUrl = newUrl;
}

void RepositoryProfile::setServerUrl ( const QString & newUrl )
{
  setServerUrl ( QUrl ( newUrl ) );
}

QString RepositoryProfile::downloadDir() const
{
  return curDownloadDir.path();
}

void RepositoryProfile::setDownloadDir ( const QString & newDownloadDir )
{
  curDownloadDir = QDir ( newDownloadDir );
}

void RepositoryProfile::setRepoType ( RepositoryType type )
{
  repositoryType = type;
}

void RepositoryProfile::setArchitectures ( const RPM::Architectures newArchitectures )
{
  curArchitectures = newArchitectures;

  // update all package types
  foreach ( const QString &package, packages.keys() ) {
    packages[package].setArchitectures ( PackageMetaData::archStringList ( curArchitectures ) );
  }
}

void RepositoryProfile::setProfileName ( const QString & newProfileName, bool renameDirectory )
{
  if ( curProfileName == newProfileName ) // nothing to do
    return;

  // should therminate thread because it is using the current database connection
  // and restart it after changing the repository name
  bool restartLister = false;

  if ( lister->isRunning() ) {
    lister->terminate();
    lister->wait();
    restartLister = true;
  }

  // remove existing db conenction
  RDDatabaseHandler::removeDbConnection ( curProfileName );

  if ( renameDirectory ) {
    renameDatabaseFile ( databaseFile(), rpmDownloaderSettings().cacheDir().absolutePath() + "/" + curProfileName + "/" + newProfileName + ".db" );
    renameRepoCacheDir ( rpmDownloaderSettings().cacheDir().absolutePath(), curProfileName, newProfileName );
  }

  curProfileName = newProfileName;
}

void RepositoryProfile::removeCacheDir()
{
  removeDirectory ( rpmDownloaderSettings().cacheDir().absolutePath() + "/" + curProfileName );
}


void RepositoryProfile::addPackage ( const Package &package )
{
  packages.insert ( package.packageName(), package );
  // set the architecture for this package
  packages[package.packageName() ].setArchitectures ( PackageMetaData::archStringList ( architecures() ) );
  updatePackageStatus ( package.packageName() );
}

bool RepositoryProfile::containsPackage ( const QString &name ) const
{
  return packages.contains ( name );
}

void RepositoryProfile::removePackage ( const QString & packageName )
{
  // don't recompute status here for every deleted package
  // call computeProfileStatus instead
  if ( packages.contains ( packageName ) )
    packages.remove ( packageName );
}

void RepositoryProfile::changePackageName ( const QString & oldName, const QString & newName )
{
  Package package = packages.take ( oldName );
  package.setPackageName ( newName );
  packages.insert ( newName, package );
}

void RepositoryProfile::computeProfileStatus()
{
  // clear stats
  clearStats();

  // set download size to zero so that calculate works as expected
  requiredDownlodSize = 0;

  if ( packages.size() < 1 ) {
    status = UNKNOWN;
    return;
  }

  bool allOk = true;

  bool allFailed = true;
  bool allUnknown = true;
  bool atLeastOneUpdate = false;
  bool someAvailable = false;
  bool someLocalAvailable = false;

  QHash<QString, Package>::const_iterator packageIterator = packages.begin();

  for ( ; packageIterator != packages.end(); ++packageIterator ) {
    if ( packageIterator.value().packageStatus() == OK ) {
      allUnknown = false;
      allFailed = false;
      ++okPackages;

    } else if ( packageIterator.value().packageStatus() == FAILED ) {
      allOk = false;
      allUnknown = false;
      ++failedPackages;

    } else if ( packageIterator.value().packageStatus() == UNKNOWN ) {
      allOk = false;
      allFailed = false;
      ++unknownPackages;

    } else if ( packageIterator.value().packageStatus() == LOCALAVAIL ) {
      allOk = false;
      allFailed = false;
      someLocalAvailable = true;
      ++localAvailPackages;

    } else if ( packageIterator.value().packageStatus() == UPDATE ) {
      allOk = false;
      allUnknown = false;
      allFailed = false;
      atLeastOneUpdate = true;
      ++updatedPackages;

    } else if ( packageIterator.value().packageStatus() == AVAILABLE ) {
      allOk = false;
      allUnknown = false;
      someAvailable = true;
      ++availPackages;

    } else {
      allOk = false;
      allFailed = false;
    }

    requiredDownlodSize += getRequiredDownloadSizeForPackage ( packageIterator.value() );
  }

  if ( atLeastOneUpdate )
    status = UPDATE;
  else if ( allOk && !atLeastOneUpdate && !someLocalAvailable )
    status = OK;
  else if ( someAvailable )
    status = AVAILABLE;
  else if ( allFailed )
    status = FAILED;
  else if ( allUnknown )
    status = UNKNOWN;
  else if ( someLocalAvailable )
    status = LOCALAVAIL;

  if ( requiredDownlodSize < 0 ) // could not calculate size
    requiredDownlodSize = -1;
}

int RepositoryProfile::numberOfPackagesToDownload() const
{
  int numberOfPackages = 0;

  QHash<QString, Package>::const_iterator packageIter;

  for ( packageIter = packages.begin(); packageIter != packages.end(); ++packageIter ) {
    if ( packageIter.value().packageStatus() == UPDATE || packageIter.value().packageStatus() == AVAILABLE ) {
      numberOfPackages += packageIter.value().requiredForDownload().size();
    }
  }

  return numberOfPackages;
}

QList< PackageMetaData > RepositoryProfile::packageMetaDatasOfpackagesToDownload() const
{
  QList <PackageMetaData> packagesToDownload;

  QHash<QString, Package>::const_iterator packageIter;

  for ( packageIter = packages.begin(); packageIter != packages.end(); ++packageIter ) {
    if ( packageIter.value().packageStatus() == UPDATE || packageIter.value().packageStatus() == AVAILABLE ) {
      packagesToDownload << packageIter.value().requiredForDownload();
    }
  }

  return packagesToDownload;
}

bool RepositoryProfile::deleteAllPackagesFromDisk()
{
  bool success = true;

  QHash<QString, Package>::iterator packageIter;

  for ( packageIter = packages.begin(); packageIter != packages.end(); ++packageIter ) {
    if ( success )
      success = deletePackageFromDisk ( packageIter.value().packageName(), false ); // it is not necessary to compute status on every iteration setting recompute to false
    else
      return success;
  }

  computeProfileStatus();

  return success;
}

bool RepositoryProfile::deletePackageFromDisk ( const QString & rpmName, const bool recomputeStatus )
{
  bool success = packages[rpmName].deleteLocalFiles ( downloadDir() );

  if ( recomputeStatus )
    computeProfileStatus();

  return success;
}

void RepositoryProfile::clearStatus()
{
  foreach ( const QString &key, packages.keys() ) {
    packages[key].clearStatus();
  }

  computeProfileStatus();
}

void RepositoryProfile::packagesUpdated ( const bool removeOldVersions )
{
  QHash<QString, Package>::iterator packageIter;

  for ( packageIter = packages.begin(); packageIter != packages.end(); ++packageIter ) {
    if ( packageIter.value().packageStatus() == UPDATE || packageIter.value().packageStatus() == AVAILABLE )
      packageIter.value().packageUpdated ( downloadDir(), removeOldVersions );
  }

  computeProfileStatus();
}

int RepositoryProfile::numberOfSelectedPackages() const
{
  return packages.size();
}

void RepositoryProfile::threadFinished()
{
  repositoryContent = lister->getRepoPackageInformations();
  localContent = lister->getLocalPackageInformations();

  // create a list of provided packages for faster access later
  QSet<QString> c;
  foreach ( const QString &arch, repositoryContent.keys() ) {
    foreach ( const QString &name, repositoryContent.value ( arch ).keys() ) {
      c << name;
    }
  }

  repoContent = c.values();

  updatePackagesStatus();
  computeProfileStatus();
  emit ( statusRefreshed() );
}

void RepositoryProfile::updatePackagesStatus()
{
  foreach ( const QString &packageName, packages.keys() ) {
    updatePackageStatus ( packageName, false ); // don't compute profile status
  }
}

void RepositoryProfile::updatePackageStatus ( const QString& packageName, bool recomputeStatus )
{
  if ( !repositoryContent.isEmpty() ) // if it is not available yet don't set remote version
    packages[packageName].setRemoteVersions ( repositoryContent );

  packages[packageName].setLocalVersions ( localContent );

  if ( recomputeStatus )
    computeProfileStatus();
}


void RepositoryProfile::refreshStatus()
{
  if ( lister->isRunning() ) // thread is running do nothing
    return;

  lister->setArchitectures ( PackageMetaData::archStringList ( architecures() ) );

  lister->setDatabasePath ( databaseFile() );

  lister->setRepoName ( profileName() );

  lister->setDownloadPath ( downloadDir() );

  lister->start();
}

QString RepositoryProfile::databaseFile() const
{
  QDir dir ( rpmDownloaderSettings().cacheDir().absolutePath() + "/" + curProfileName );

  if ( !dir.exists() )
    dir.mkpath ( rpmDownloaderSettings().cacheDir().absolutePath() + "/" + curProfileName );

  return rpmDownloaderSettings().cacheDir().absolutePath() + "/" + curProfileName + "/" + curProfileName + ".db";
}

void RepositoryProfile::removeDirectory ( const QString& path )
{
  QDir dir ( path );

  if ( !dir.exists() ) // nothing to delete
    return;

  clearDirectory ( path );

  if ( !dir.rmdir ( path ) )
    qDebug ( "could not remove directory: %s", qPrintable ( path ) );
}

void RepositoryProfile::clearDirectory ( const QString& path )
{
  QDir dir ( path );

  if ( !dir.exists() ) // nothing to delete
    return;

  QStringList files = dir.entryList();

  foreach ( const QString &file, files ) {
    // only remove files
    QFileInfo fileInfo ( dir, file );

    if ( fileInfo.isFile() ) {
      if ( !dir.remove ( file ) )
        qDebug ( "could not remove file: %s", qPrintable ( file ) );
    }
  }
}

void RepositoryProfile::renameRepoCacheDir ( const QString& path, const QString& oldName, const QString& newName )
{
  QDir dir ( path );

  if ( !dir.exists() ) // given dir does not exist
    return;

  if ( !dir.rename ( oldName, newName ) )
    qDebug ( "could not rename directory: path %s, old name %s, new name %s", qPrintable ( path ), qPrintable ( oldName ), qPrintable ( newName ) );
}

void RepositoryProfile::renameDatabaseFile ( const QString& oldName, const QString& newName )
{
  QFile file ( oldName );

  if ( !file.exists() )
    return;

  if ( !file.rename ( newName ) )
    qDebug ( "could not rename file: %s", qPrintable ( oldName ) );
}

Status RepositoryProfile::getPackageStatus ( const QString& packageName ) const
{
  return packages.value ( packageName ).packageStatus();
}

PackageVersions RepositoryProfile::getLocalPackageVersions ( const QString& packageName ) const
{
  return packages.value ( packageName ).localVersions();
}

PackageVersions RepositoryProfile::getRemotePackageVersions ( const QString& packageName ) const
{
  return packages.value ( packageName ).availableversions();
}

bool RepositoryProfile::hasPackage ( const QString& packageName ) const
{
  if ( packages.contains ( packageName ) )
    return true;

  return false;
}

ArchitecutreDependentMetaData RepositoryProfile::getLocalMetaData ( const QString& packageName ) const
{
  return packages.value ( packageName ).getLocalPackageMetaData();
}

ArchitecutreDependentMetaData RepositoryProfile::getRemoteMetaData ( const QString& packageName ) const
{
  return packages.value ( packageName ).getRemotePackageMetaData();
}

const Package RepositoryProfile::getPackageData ( const QString& packageName ) const
{
  return packages.value ( packageName );
}

qint64 RepositoryProfile::getRequiredDownloadSizeForPackage ( const Package& package )
{
  QList<PackageMetaData> mData = package.requiredForDownload();
  qint64 size = 0;

  QList<PackageMetaData>::const_iterator iter = mData.begin();

  for ( ; iter != mData.end(); ++iter ) {
    size += iter->size();
  }

  return size;
}

int RepositoryProfile::cleanupOrphanedPackages()
{
  int removedOrphanedPackages = 0;

  QHash<QString, Package>::iterator packageIter = packages.begin();

  while ( packageIter != packages.end() ) {
    if ( packageIter->packageStatus() == LOCALAVAIL ) {
      deletePackageFromDisk ( packageIter->packageName(), false );

    }

    if ( packageIter->packageStatus() == FAILED || packageIter->packageStatus() == LOCALAVAIL ) {
      packageIter = packages.erase ( packageIter );
      ++removedOrphanedPackages;

    } else {
      ++packageIter;
    }
  }

  computeProfileStatus();

  return removedOrphanedPackages;
}


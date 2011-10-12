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
#include "package.h"

Package::Package()
    : status ( UNKNOWN )
{
}

Package::Package ( const QString & packageName )
    : name ( packageName ), status ( UNKNOWN )
{
}

void Package::setArchitectures ( const QStringList &archs )
{
  architectures = archs;

  // create for each arch a representative package
  foreach ( const QString &arch, architectures ) {
    if ( !rpms.contains ( arch ) )
      rpms[arch] = Rpm ( packageName(), arch );
  }

  // now clean up obseolete architecure entries
  foreach ( const QString &arch, rpms.keys() ) {
    if ( !architectures.contains ( arch ) )
      architectures.removeAll ( arch );
  }

  computeStatus();
}

void Package::setPackageName ( const QString & packageName )
{
  if ( name == packageName )
    return;

  // because of the name change clear all rpms and set status to unknown
  rpms.clear();

  foreach ( const QString &arch, architectures ) {
    rpms.insert ( arch, Rpm ( packageName, arch ) );
  }

  status = UNKNOWN;

  name = packageName;
}

PackageVersions Package::localVersions() const
{
  PackageVersions versions;
  foreach ( const QString &arch, architectures ) {
    versions.insert ( arch, rpms.value ( arch ).localVersion() );
  }

  return versions;
}

PackageVersions Package::availableversions() const
{
  PackageVersions versions;

  foreach ( const QString &arch, architectures ) {
    versions.insert ( arch, rpms.value ( arch ).availableVersion() );
  }

  return versions;
}

void Package::clearStatus()
{
  status = UNKNOWN;

  // clear status for all rpms too
  foreach ( const QString &arch, architectures ) {
    rpms[arch].clearStatus();
  }
}

void Package::computeStatus()
{
  bool oneAvail = false;
  bool oneUnknown = false;
  bool oneOk = false;
  bool oneUpdate = false;
  bool oneLocalAvail = false;

  foreach ( const QString &arch, architectures ) {
    switch ( rpms.value ( arch ).rpmStatus() ) {
      case AVAILABLE:
        oneAvail = true;
        break;
      case UNKNOWN:
        oneUnknown = true;
        break;
      case OK:
        oneOk = true;
        break;
      case UPDATE:
        oneUpdate = true;
        break;
      case LOCALAVAIL:
        oneLocalAvail = true;
      case FAILED: // don't nees to handle this
        break;
    }
  }

  if ( oneUnknown )
    status = UNKNOWN;
  else if ( oneLocalAvail )
    status = LOCALAVAIL;
  else if ( oneAvail )
    status = AVAILABLE;
  else if ( oneUpdate )
    status = UPDATE;
  else if ( oneOk )
    status = OK;
  else
    status = FAILED;
}

void Package::packageUpdated ( const QString &repoDownloadPath, const bool removeOldVersion )
{
  foreach ( const QString &arch, architectures ) {
    if ( rpms.value ( arch ).rpmStatus() == AVAILABLE || rpms.value ( arch ).rpmStatus() == UPDATE ) {
      rpms[arch].rpmUpdated ( repoDownloadPath, removeOldVersion );
      status = OK;
    }
  }
}


bool Package::deleteLocalFiles ( const QString & directory )
{
  foreach ( const QString &arch, architectures ) {
    if ( !rpms[arch].removeLocalVersion ( directory ) )
      return false;
  }

  computeStatus();

  return true;
}

void Package::setLocalVersions ( const MetaData::MultipleArchMetaData &metaDatas )
{
  foreach ( const QString &arch, architectures ) {
    if ( metaDatas.contains ( arch ) && metaDatas.value ( arch ).contains ( name ) ) {
      PackageMetaData metaData = metaDatas.value ( arch ).value ( name );
      rpms[arch].setlocalMetaData ( metaData );

    } else {
      rpms[arch].setlocalMetaData ( generateFailedMetaData ( arch ) );
    }
  }

  computeStatus();
}

void Package::setRemoteVersions ( const MetaData::MultipleArchMetaData &metaDatas )
{
  foreach ( const QString &arch, architectures ) {
    if ( metaDatas.contains ( arch ) && metaDatas.value ( arch ).contains ( name ) ) {
      PackageMetaData metaData = metaDatas.value ( arch ).value ( name );
      rpms[arch].setRemoteMetaData ( metaData );

    } else {

      rpms[arch].setRemoteMetaData ( generateFailedMetaData ( arch ) );
    }
  }

  computeStatus();
}

PackageMetaData Package::generateFailedMetaData ( const QString &arch ) const
{
  PackageMetaData metaData;
  metaData.setVersion ( "-1" );
  metaData.setPackageName ( name );
  metaData.setArch ( arch );
  metaData.setSize ( 0 );

  return metaData;
}


QList< PackageMetaData > Package::requiredForDownload() const
{
  QList<PackageMetaData> requiredMetaDatas;

  foreach ( const QString &arch, architectures ) {
    if ( rpms.value ( arch ).rpmStatus() == AVAILABLE || rpms.value ( arch ).rpmStatus() == UPDATE )
      requiredMetaDatas.append ( rpms.value ( arch ).rMetaData() );
  }

  return requiredMetaDatas;
}

MetaData::ArchitecutreDependentMetaData Package::getLocalPackageMetaData() const
{
  MetaData::ArchitecutreDependentMetaData localMetaData;

  foreach ( const QString& arch, architectures ) {
    localMetaData.insert ( arch, rpms.value ( arch ).lMetaData() );
  }

  return localMetaData;
}

MetaData::ArchitecutreDependentMetaData Package::getRemotePackageMetaData() const
{
  MetaData::ArchitecutreDependentMetaData remoteMetaData;

  foreach ( const QString& arch, architectures ) {
    remoteMetaData.insert ( arch, rpms.value ( arch ).rMetaData() );
  }

  return remoteMetaData;
}


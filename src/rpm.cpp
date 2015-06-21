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
#include "rpm.h"

#include <QtCore/QDir>

Rpm::Rpm()
    : name(), status ( UNKNOWN )
{
  localMetaData.setVersion ( "0" );
  remoteMetaData.setVersion ( "0" );
}

Rpm::Rpm ( const QString & rpmname, const QString &architecture )
    : name ( rpmname ), arch ( architecture ), status ( UNKNOWN )
{
  localMetaData.setVersion ( "0" );
  remoteMetaData.setVersion ( "0" );
}

bool Rpm::removeLocalVersion ( const QString &baseDownloadPath, const bool recomputeStatus )
{
  QDir path ( baseDownloadPath + "/" + architecture() );

  if ( localMetaData.version() == "-1" ) // nothing to do because no local version available
    return true;

  if ( !path.exists() )
    return false;

  if ( localFileName().isEmpty() ) // no filename set
    return false;

  QFile file ( path.canonicalPath() + "/" + localFileName() );

  if ( !file.remove() )
    return false;

  localMetaData.setVersion ( "-1" );

  localMetaData.setFile ( QString() );

  if ( recomputeStatus )
    computeStatus();

  return true;
}

void Rpm::computeStatus()
{
  QString localV = localMetaData.version();
  QString availV = remoteMetaData.version();

  if ( localV == "0" && availV == "0" ) {
    status = UNKNOWN;

  } else if ( localV == "-1" ) {
    // got no local version
    if ( availV != "-1" && availV != "0" )
      status = AVAILABLE;
    else if ( availV == "0" )
      status = UNKNOWN;
    else
      status = FAILED;

  } else if ( localV != "0" && availV == "-1" ) {
    status = LOCALAVAIL;

  } else if ( localV != "0" && availV != "0" ) {
    if ( PackageMetaData::versionIsGreaterThan ( localV, availV ) )
      status = UPDATE;
    else
      status = OK;

  } else if ( localV == "0" && availV != "0" ) {
    status = AVAILABLE;

  } else if ( localV != "-1" && availV == "0" ) {
    status = UNKNOWN;

  } else {
    status = OK;
  }
}

void Rpm::setlocalMetaData ( const PackageMetaData& metaData, const bool recomputeStatus )
{
  if ( !verifyMetaData ( metaData ) )
    return;

  localMetaData = metaData;

  arch = metaData.architecture();

  status = UNKNOWN;

  if ( recomputeStatus )
    computeStatus();
}

void Rpm::setRemoteMetaData ( const PackageMetaData& metaData, const bool recomputeStatus )
{
  if ( !verifyMetaData ( metaData ) ) {
    return;
  }

  remoteMetaData = metaData;

  arch = metaData.architecture();
  status = UNKNOWN;

  if ( recomputeStatus )
    computeStatus();
}

bool Rpm::verifyMetaData ( const PackageMetaData& metaData )
{
  if ( name != metaData.packageName() ) { // the name does not match
    qDebug ( "Package name does not equal: package name %s meta data %s", qPrintable ( name ), qPrintable ( metaData.packageName() ) );
    return false;
  }

  if ( !architecture().isEmpty() ) { // architecture already set
    if ( metaData.architecture() != architecture() ) {
      qDebug ( "Wrong architecture for this rpm %s submitted", qPrintable ( name ) );
      return false;
    }
  }

  return true;
}

void Rpm::rpmUpdated ( const QString &baseDownloadpath, bool deleteOldversion )
{
  if ( status != AVAILABLE && status != UPDATE ) // nothing shuld be done
    return;

  if ( deleteOldversion )
    removeLocalVersion ( baseDownloadpath, false );

  // set available version to local version
  localMetaData = remoteMetaData;

  // status should be OK now
  status = OK;
}


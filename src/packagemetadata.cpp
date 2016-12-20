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
#include "packagemetadata.h"

#include <QtCore/QProcess>

PackageMetaData::PackageMetaData()
    : m_fileSize ( 0 )
{
}

void PackageMetaData::addRequire ( const QString & requirement )
{
  if ( !requirement.isEmpty() ) // do not add empty values
    m_packageRequires.append ( requirement );
}

void PackageMetaData::addProvide ( const QString & provide )
{
  if ( !provide.isEmpty() ) // do not append empty values
    m_packageProvides.append ( provide );
}

void PackageMetaData::clear()
{
  m_packageProvides.clear();
  m_packageRequires.clear();

  m_name.clear();
  m_arch.clear();
  m_file.clear();
  m_fileSize = 0;
}

PackageMetaData::PackageMetaData ( const QString& fileName, const QString& packageName, const QString& version, const quint64 size, const QString& architecture, const QString& shaCheckSum, const QString& checkSumAlgorithm, const QString& location )
    : m_file ( fileName ), m_name ( packageName ), m_arch ( architecture ), m_packageVersion ( version ), m_fileSize ( size ), m_checkSum ( shaCheckSum ), m_checksumAlgorithm( checkSumAlgorithm ), m_location ( location )
{
}

PackageMetaData::PackageMetaData ( const QFileInfo &fileInfo, const QString& architecture, const quint64 size )
    : m_file( fileInfo.fileName() ), m_arch ( architecture ), m_fileSize ( size )
{
  PackageVersionAndName versionAndFileName = extractVersionAndName ( fileInfo, m_arch );
  m_name = versionAndFileName.packageName;
  m_packageVersion = versionAndFileName.packageVersion;
}

QString PackageMetaData::sizeAsString() const
{
  return sizeAsString ( m_fileSize );
}

void PackageMetaData::setSize ( const QString& size )
{
  QRegExp sizeExp ( "(\\d+[.|]\\d*)([M|B|K])" );
  sizeExp.setCaseSensitivity ( Qt::CaseInsensitive );

  if ( sizeExp.indexIn ( size ) != -1 ) {
    quint64 fSize = sizeExp.cap ( 1 ).toInt();

    if ( sizeExp.cap ( 2 ).compare ( "M", Qt::CaseInsensitive ) == 0 )
      fSize = fSize * 1000000;
    else if ( sizeExp.cap ( 2 ).compare ( "K", Qt::CaseInsensitive ) == 0 )
      fSize = fSize * 1000;

    setSize ( fSize );
  }
}

PackageVersionAndName PackageMetaData::extractVersionAndName( const QFileInfo &fileInfo, const QString & archString )
{
  QProcess rpmSubprocess;
  rpmSubprocess.start( "rpm", QStringList() << "-qpi" << fileInfo.absoluteFilePath(), QIODevice::ReadOnly );
  
  PackageVersionAndName value;
  value.packageName = "Unknown";
  value.packageVersion = "-1";
  
  if ( !rpmSubprocess.waitForFinished() ) {
      return value;
  }

  if ( rpmSubprocess.exitCode() != 0 ) {
    qCritical("%s, %s", qPrintable( QString( rpmSubprocess.readAllStandardOutput() ) ), qPrintable( QString( rpmSubprocess.readAllStandardError() ) ) );
    return value;
  }
  
  enum { KeyPos, ValuePos };
  
  const QString informations( rpmSubprocess.readAllStandardOutput() );
  
  for ( const QString &part : informations.split( QRegExp("[\r\n]"),QString::SkipEmptyParts ) ) {
    QStringList splittedValues = part.split( ":", QString::SkipEmptyParts );
    if ( splittedValues.size() != 2 ) {
      continue;
    }
    
    if ( splittedValues.at( KeyPos ).trimmed() == "Name" ) {
      value.packageName = splittedValues.at( ValuePos ).trimmed();
      
    } else if ( splittedValues.at( KeyPos ).trimmed() == "Version" ) {
      value.packageVersion = value.packageVersion == "-1" ? 
        splittedValues.at( ValuePos ).trimmed() : splittedValues.at( ValuePos ).trimmed() + value.packageVersion;
      
    } else if ( splittedValues.at( KeyPos ).trimmed() == "Release" ) {
      value.packageVersion += splittedValues.at( ValuePos ).trimmed();
    }

  }

  return value;
}

QStringList PackageMetaData::archStringList ( const RPM::Architectures archs )
{
  QStringList archStringList;

  if ( archs & RPM::i586 ) {
    archStringList.push_back ( "i586" );
  }

  if ( archs & RPM::i686 ) {
    archStringList.push_back ( "i686" );
  }

  if ( archs & RPM::x86_64 ) {
    archStringList.push_back ( "x86_64" );
  }

  if ( archs & RPM::noarch ) {
    archStringList.push_back ( "noarch" );
  }

  return archStringList;
}

bool PackageMetaData::versionIsGreaterThan ( const QString & base, const QString & other )
{
  return other > base;
//   if ( base == other ) // versions are identic
//     return false;
// 
//   QStringList baseVersionParts = ( base.split ( "." ) );
// 
//   QStringList otherVersionParts = fixRpmVersionPart ( other.split ( "." ) );
// 
//   for ( int i = 0; i < baseVersionParts.size(); ++i ) {
//     if ( i < otherVersionParts.size() ) { // avoid out of range error
//       // qDebug("baseStringPart %s otherStringPart %s", qPrintable(baseVersionParts.at(i)), qPrintable(otherVersionParts.at(i)));
//       QRegExp noNumberRegExp ( "\\D" );
// 
//       if ( baseVersionParts.at ( i ).contains ( noNumberRegExp ) ||
//            otherVersionParts.at ( i ).contains ( noNumberRegExp ) ) {
//         if ( baseVersionParts.at ( i ) < otherVersionParts.at ( i ) ) {
//           return true;
//           break;
//         }
// 
//       } else {
//         int basePart = baseVersionParts.at ( i ).toInt();
//         int otherPart = otherVersionParts.at ( i ).toInt();
// 
//         if ( basePart < otherPart ) { // one part is newer than the other that implies a newer version
//           return true;
//           break;
//         }
//       }
//     }
//   }
// 
//   return false;
}

void PackageMetaData::setLocation ( const QString& location )
{
  m_location = location;
}

QStringList PackageMetaData::fixRpmVersionPart ( const QStringList & versionParts )
{
  QStringList newVersionParts = versionParts;

  for ( int i = 0; i < newVersionParts.size(); ++i ) {
    if ( newVersionParts.at ( i ).contains ( "-" ) ) {
      QString rpmVersionPart = newVersionParts.takeAt ( i );
      newVersionParts << rpmVersionPart.split ( "-" );
    }
  }

  return newVersionParts;
}

bool PackageMetaData::operator > ( const PackageMetaData & rhs ) const
{
  if ( versionIsGreaterThan ( m_packageVersion, rhs.version() ) )
    return false;

  return true;
}

bool PackageMetaData::operator < ( const PackageMetaData & rhs ) const
{
  if ( versionIsGreaterThan ( m_packageVersion, rhs.version() ) )
    return true;

  return false;
}

QString PackageMetaData::sizeAsString ( const qint64 size )
{
  // do some more human readable output
  if ( size / 1000000 > 0 ) {
    // round 2 decimal 2 places
    qreal tmpSize = qRound64 ( size / 10000.0 );
    return QString ( "%1 M" ).arg ( tmpSize / 100 );

  } else if ( size / 1000 ) {
    qreal tmpSize = qRound64 ( size / 10.0 );
    return QString ( "%1 K" ).arg ( tmpSize / 100 );
  }

  // default are bytes
  return QString ( "%1 B" ).arg ( size );
}

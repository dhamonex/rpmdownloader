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
#ifndef PACKAGEMETADATA_H
#define PACKAGEMETADATA_H

#include "rdnamespace.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMap>
#include <QtCore/QHash>
#include <QtCore/QFileInfo>

class PackageMetaData;

struct PackageVersionAndName {
  QString packageName;
  QString packageVersion;
};

namespace MetaData
{
  typedef QHash<QString, PackageMetaData> MultiplePackageMetaData; // only for one architecture <package name, meta datas>
  typedef QMap<QString, MultiplePackageMetaData> MultipleArchMetaData; // for all archs

  // contains only one meta data for one package in different architectures
  // for convinient use
  typedef QMap<QString, PackageMetaData> ArchitecutreDependentMetaData; // <arch, meta data>
}

class PackageMetaData
{
  public:
    PackageMetaData();
    PackageMetaData ( const QString &fileName, const QString &packageName, const QString &version, const quint64 size, const QString &architecture, const QString &shaCheckSum, const QString &checkSumAlgorithm, const QString &location );
    PackageMetaData ( const QFileInfo &fileInfo, const QString &architecture, const quint64 size = -1 ); // tries to extract informations from the filename

    static PackageVersionAndName extractVersionAndName ( const QFileInfo &fileInfo, const QString &archString );
    static QStringList archStringList ( const RPM::Architectures archs ); // generates a string list from the given archs
    static bool versionIsGreaterThan ( const QString &base, const QString &other );

    bool operator> ( const PackageMetaData &rhs ) const;
    bool operator< ( const PackageMetaData &rhs ) const;

    void clear();
    void setFile ( const QString &fileName ) {m_file = fileName;}

    void setPackageName ( const QString &pName ) {m_name = pName;}

    void setSize ( const quint64 &size ) {m_fileSize = size;}

    void setSize ( const QString &size );
    void addRequire ( const QString &requirement );
    void addProvide ( const QString &provide );
    void setArch ( const QString &arch ) {this->m_arch = arch;};

    void setVersion ( const QString &version ) {m_packageVersion = version;}

    void setShaCheckSum ( const QString &cSum ) {m_checkSum = cSum;}
    
    void setChecksumAlgorithm( const QString & algorithm ) {m_checksumAlgorithm = algorithm;}

    void setLocation ( const QString &location );

    QString packageName() const {return m_name;}

    QString fileName() const {return m_file;}

    QString architecture() const {return m_arch;}

    QStringList provides() const {return m_packageProvides;}

    QStringList requires() const {return m_packageRequires;}

    QString version() const {return m_packageVersion;}

    QString shaCheckSum() const {return m_checkSum;}
    
    QString checkSumAlgorithm() const {return m_checksumAlgorithm;}

    quint64 size() const {return m_fileSize;}

    QString sizeAsString() const;
    QString location() const {return m_location;}

    static QString sizeAsString ( const qint64 size );

  private:
    static QStringList fixRpmVersionPart ( const QStringList &versionParts );

    QString m_file;
    QString m_name;
    QString m_arch;
    QString m_packageVersion;
    quint64 m_fileSize;
    QStringList m_packageRequires;
    QStringList m_packageProvides;
    QString m_checkSum; // SHA check sum only for remote meta datas
    QString m_checksumAlgorithm;
    QString m_location;

};

#endif

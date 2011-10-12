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
#ifndef RPMDOWNLOADERSETTINGS_H
#define RPMDOWNLOADERSETTINGS_H

#include <QDir>
#include <QString>

class RpmDownloaderSettings
{
  public:
    RpmDownloaderSettings();

    ~RpmDownloaderSettings();

    void setUpdateInterval ( int interval );
    int updateInterval() const {return m_rpmUpdateInterval;}

    void setDeleteOldVersions ( const bool deleteOld );
    bool deleteOldVersions() const {return m_deleteOldVersionsFromDisk;}

    QDir tempDir() const {return m_tempDirectory;}

    void setTempDir ( const QDir &newDir );

    QDir cacheDir() const {return m_cacheDirectory;}

    void setCacheDir ( const QDir &newDir );

    QString checksumCommand() const {return m_chcksumCommand;}

    void setChecksumCommand ( const QString &newCommand );

    QString gunzipCommand() const {return m_g_unzipCommand;}

    void setGunzipCommand ( const QString &newCommand );

    bool useMemDbSatSolving() const {return m_memDbSatSolve;}

    void setMemDbSatSolving ( const bool use );

    bool doCheckSumCheckOnDownloadedPackages() const {return m_checkSumCheck;}

    void setDoCheckSumCheckOnDownloadedPackages ( const bool doIt );

    void setMaximumPrimaryFileSizeForMemoryLoad ( const qint64 size );
    qint64 maximumPrimaryFileSizeForMemoryLoad() const {return m_loadPrimaryFileToMemoryMaxSize;}


  private:
    int m_rpmUpdateInterval;
    bool m_deleteOldVersionsFromDisk;

    QDir m_cacheDirectory; // for yum repositories
    QDir m_tempDirectory; // for temporary files
    QString m_chcksumCommand; // for doing a checksum check of downloaded yum repo meta files
    QString m_g_unzipCommand; // unzip yum meta files

    // specifies if the whole database should be loaded into the memory
    // when resolving satisfactions which will increase the speed a lot
    // but consumes more memory
    bool m_memDbSatSolve;
    bool m_checkSumCheck;

    // specify a maximum primary.xml file size which will be loaded into memory for a faster parsing
    // if file is bigger file is parsed from disk completly
    qint64 m_loadPrimaryFileToMemoryMaxSize;

};

RpmDownloaderSettings &rpmDownloaderSettings(); // satic (singleton) settings object

#endif

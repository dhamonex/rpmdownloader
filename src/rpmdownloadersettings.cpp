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
#include "rpmdownloadersettings.h"

RpmDownloaderSettings::RpmDownloaderSettings()
    : m_rpmUpdateInterval ( 30000 ), m_deleteOldVersionsFromDisk ( true ),
    m_cacheDirectory ( QDir::currentPath() + "/cache" ), m_tempDirectory ( QDir::currentPath() + "/tmp" ),
    m_chcksumCommand ( "/usr/bin/shasum" ), m_g_unzipCommand ( "/usr/bin/gunzip" ), m_memDbSatSolve ( true ), m_checkSumCheck ( true ), m_loadPrimaryFileToMemoryMaxSize ( 60000000 )
{
}


RpmDownloaderSettings::~RpmDownloaderSettings()
{
}

void RpmDownloaderSettings::setUpdateInterval ( int interval )
{
  m_rpmUpdateInterval = interval;
}

void RpmDownloaderSettings::setDeleteOldVersions ( const bool deleteOld )
{
  m_deleteOldVersionsFromDisk = deleteOld;
}

void RpmDownloaderSettings::setTempDir ( const QDir & newDir )
{
  m_tempDirectory = newDir;
}

void RpmDownloaderSettings::setCacheDir ( const QDir & newDir )
{
  m_cacheDirectory = newDir;
}


void RpmDownloaderSettings::setGunzipCommand ( const QString & newCommand )
{
  m_g_unzipCommand = newCommand;
}

void RpmDownloaderSettings::setChecksumCommand ( const QString & newCommand )
{
  m_chcksumCommand = newCommand;
}

void RpmDownloaderSettings::setMemDbSatSolving ( const bool use )
{
  m_memDbSatSolve = use;
}

void RpmDownloaderSettings::setDoCheckSumCheckOnDownloadedPackages ( const bool doIt )
{
  m_checkSumCheck = doIt;
}

void RpmDownloaderSettings::setMaximumPrimaryFileSizeForMemoryLoad ( const qint64 size )
{
  m_loadPrimaryFileToMemoryMaxSize = size;
}

RpmDownloaderSettings & rpmDownloaderSettings()
{
  static RpmDownloaderSettings settings;
  return settings;
}

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
#include "yumcachebuilderthread.h"

#include "yumfileconstants.h"

#include <QtCore/QFile>
#include <QtXml/QXmlSimpleReader>
#include <QtCore/QBuffer>

YumCacheBuilderThread::YumCacheBuilderThread ( QObject *parent )
    : QThread ( parent ), m_success ( true ), m_maxPrimaryFileSize ( 0 )
{
}

YumCacheBuilderThread::~YumCacheBuilderThread()
{
  terminateCacheBuilder();
}

void YumCacheBuilderThread::setMaxPrimaryFileSizeForMemoryLoad ( const qint64 size )
{
  m_maxPrimaryFileSize = size;
}

void YumCacheBuilderThread::setDatabase ( const QString & pathToDatabase )
{
  m_databasePath = pathToDatabase;
}

void YumCacheBuilderThread::setPathToCacheDir ( const QString & path )
{
  m_cache = path;
}

void YumCacheBuilderThread::setRepoName ( const QString & name )
{
  m_repoName = name;
}

void YumCacheBuilderThread::run()
{
  // here runs the threaded part
  m_errMsg.clear();
  m_success = true;

  QFile file ( m_cache + "/" + Yum::primaryFileName );

  if ( !m_reader.initDatabase ( m_databasePath, m_repoName ) ) {
    m_errMsg = m_reader.readError();
    m_success = false;
    return;
  }

  if ( !file.open ( QFile::ReadOnly | QFile::Text ) ) {
    m_errMsg = tr ( "Cannot read file %1: %2" ).arg ( file.fileName() ).arg ( file.errorString() );
    m_success = false;
    return;
  }

  if ( file.size() < m_maxPrimaryFileSize ) {
    qDebug ( "YumCacheBuilderThread: loading complete primary into memory" );
    QByteArray bArry = file.readAll();
    QBuffer buffer;
    buffer.setBuffer ( &bArry );
    buffer.open ( QIODevice::ReadOnly );

    m_success = m_reader.read ( &buffer );

  } else {
    m_success = m_reader.read ( &file );
  }

  m_errMsg = m_reader.readError();
}

void YumCacheBuilderThread::terminateCacheBuilder()
{
  if ( isRunning() )
    m_reader.abortUpdate();
}




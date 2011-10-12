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
#include "yumcachebuilder.h"

#include "yumfileconstants.h"
#include "yumcachebuilderthread.h"
#include "rpmdownloadersettings.h"

YumCacheBuilder::YumCacheBuilder ( QObject *parent )
    : QObject ( parent ), aborted ( false )
{
  uncompressMetaFiles = new YumUncompressMetaFiles ( this );
  cacheBuilderThread = new YumCacheBuilderThread ( this );

  connect ( uncompressMetaFiles, SIGNAL ( finishedUncompress ( bool ) ), this, SLOT ( uncompressFinished ( bool ) ) );
  connect ( cacheBuilderThread, SIGNAL ( finished() ), this, SLOT ( threadFinished() ) );
}


YumCacheBuilder::~YumCacheBuilder()
{
  if ( cacheBuilderThread->isRunning() ) {
    terminate();

    cacheBuilderThread->terminate();
    cacheBuilderThread->wait ( 1000 );
  }

  // the meta files are not needed anymore so remove them now because the take a big amount of disk space
  removeUncompressedMetaFiles();
}

void YumCacheBuilder::buildCache ( const QString & pathToDatabase, const QString & repoName, const QString & cache )
{
  aborted = false;
  cachePath = cache;

  if ( cacheBuilderThread->isRunning() || uncompressMetaFiles->isActive() ) // work in progress nothing can be done now
    return;

  cacheBuilderThread->setPathToCacheDir ( cache );

  cacheBuilderThread->setDatabase ( pathToDatabase );

  cacheBuilderThread->setRepoName ( repoName );

  cacheBuilderThread->setMaxPrimaryFileSizeForMemoryLoad ( rpmDownloaderSettings().maximumPrimaryFileSizeForMemoryLoad() );

  uncompressMetaFiles->uncompressMetaFiles ( cache );
}

void YumCacheBuilder::uncompressFinished ( bool success )
{
  if ( !success ) {
    errorMsg = tr ( "could not uncompress meta file" );
    emit ( buildFinished ( false ) );
    return;
  }

  cacheBuilderThread->start();
}

void YumCacheBuilder::removeUncompressedMetaFiles()
{
  // qDebug("%s", qPrintable(QString(cachePath + "/" + Yum::primaryFileName)));
  QFile::remove ( cachePath + "/" + Yum::primaryFileName );
}

void YumCacheBuilder::threadFinished()
{
  if ( !aborted ) {
    errorMsg = cacheBuilderThread->readError(); // read error message
    // qDebug("%s", qPrintable(errorMsg));
    emit ( buildFinished ( cacheBuilderThread->parsedSuccessfull() ) );
  }

  removeUncompressedMetaFiles();
}

void YumCacheBuilder::terminate()
{
  aborted = true;
  cacheBuilderThread->terminateCacheBuilder();
  // the meta files are not needed anymore so remove them now because the take a big amount of disk space
  removeUncompressedMetaFiles();
}

void YumCacheBuilder::setGunzipCommand ( const QString & command )
{
  uncompressMetaFiles->setGunzipCommand ( command );
}

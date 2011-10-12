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
#include "abstractcontentdownloader.h"

#include <QFtp>
#include <QRegExp>

#include "rdhttp.h"

AbstractContentDownloader::AbstractContentDownloader ( QObject *parent )
    : QObject ( parent ), curProfile ( -1 ), aborted ( false )
{
  // initalize ftp and http objects
  ftp = new QFtp ( this );
  http = new RDHttp ( this );
}

AbstractContentDownloader::~AbstractContentDownloader()
{
}

void AbstractContentDownloader::closeConnections()
{
  if ( ftp->state() != QFtp::Unconnected ) {
    ftp->clearPendingCommands();
    ftp->close();
    // ftp->abort();
  }

  if ( http->state() != QHttp::Unconnected ) {
    http->clearPendingRequests();
    http->closeConnection();
    // http->abort();
  }
}

// void AbstractContentDownloader::setCacheDir(const QDir & newCacheDir)
// {
// 	cacheDir = newCacheDir;
// 	QRegExp nameSep("\\/+");
//
// 	repoName = cacheDir.absolutePath().section(nameSep, -1, -1);
// }


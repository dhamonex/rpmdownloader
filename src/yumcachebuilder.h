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
#ifndef YUMCACHEBUILDER_H
#define YUMCACHEBUILDER_H

#include "yumuncompressmetafiles.h"

#include <QObject>

class YumCacheBuilderThread;

class YumCacheBuilder : public QObject
{
    Q_OBJECT
  public:
    YumCacheBuilder ( QObject *parent = 0 );

    void buildCache ( const QString &pathToDatabase, const QString &repoName, const QString &cache );
    // reponame is used to the identify the connectionName
    QString readError() const {return errorMsg;}

    void setGunzipCommand ( const QString &command );

    void terminate();

    ~YumCacheBuilder();

  signals:
    void buildFinished ( bool ); // when finished emit this signal

  private slots:
    void uncompressFinished ( bool success );
    void threadFinished();

  private:
    void removeUncompressedMetaFiles();

    QString cachePath;
    QString errorMsg;

    YumUncompressMetaFiles *uncompressMetaFiles;
    YumCacheBuilderThread *cacheBuilderThread;

    bool aborted; // prevent emiiting error signals when aborted

};

#endif

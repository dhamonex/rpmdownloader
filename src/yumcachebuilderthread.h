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
#ifndef YUMCACHEBUILDERTHREAD_H
#define YUMCACHEBUILDERTHREAD_H

#include <QtCore/QThread>

// #include "yumsaxmetaparser.h"
#include "yumxmlmetaparser.h"

class YumCacheBuilderThread : public QThread
{
    Q_OBJECT
  public:
    YumCacheBuilderThread ( QObject *parent = 0 );

    // things to set before starting the thread
    void setDatabase ( const QString &pathToDatabase );
    void setPathToCacheDir ( const QString &path );
    void setRepoName ( const QString &name );

    // specify a maximum primary.xml file size which will be loaded into memory for a faster parsing
    // if file is bigger file is parsed from disk completly
    void setMaxPrimaryFileSizeForMemoryLoad ( const qint64 size );

    bool parsedSuccessfull() const {return m_success;}

    QString readError() const {return m_errMsg;}

    void terminateCacheBuilder();

    ~YumCacheBuilderThread();

  protected:
    virtual void run();

  private:
    QString m_databasePath;
    QString m_cache;
    QString m_repoName;

    QString m_errMsg;
    // YumSaxMetaParser handler;
    YumXmlMetaParser m_reader;

    bool m_success;
    qint64 m_maxPrimaryFileSize;
};

#endif

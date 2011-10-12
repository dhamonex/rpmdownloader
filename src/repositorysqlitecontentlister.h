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
#ifndef REPOSITORYSQLITECONTENTLISTER_H
#define REPOSITORYSQLITECONTENTLISTER_H

#include "rddatabasehandler.h"
#include "packagemetadata.h"

#include <QHash>
#include <QMap>
#include <QString>
#include <QStringList>

using namespace MetaData;

class RepositorySqliteContentLister : public RDDatabaseHandler
{
    Q_OBJECT
  public:
    RepositorySqliteContentLister ( QObject* parent = 0 );

    bool fetchContent ( const QString &databasePath, const QString &repoName, const QStringList &architectures );

    MultipleArchMetaData getMetaData() const {return contents;}

    ~RepositorySqliteContentLister();

  private:
    MultipleArchMetaData contents;
};

#endif

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
#ifndef RDDATABASEINSERTER_H
#define RDDATABASEINSERTER_H

#include "rddatabasehandler.h"
#include "packagemetadata.h"

class RDDatabaseInserter : public RDDatabaseHandler
{
  public:
    RDDatabaseInserter ( QObject* parent = 0 );

    void clear(); // clears the database
    bool init ( const QString &databasePath, const QString conenctionName );
    bool insertPackage ( const PackageMetaData &data, const bool yumRepo = false ); // inserts the package to the database

    void finish();
    void rollback(); // revert current transaction

    ~RDDatabaseInserter();

  private:
    bool insertMetaData ( const QString& packageName, const QString& packageArchitecture, const QString& version, const quint64 size, const QString& fileName, const QString checkSum, const QString& checkSumAlgorithm, const QString& location );

    bool insertProvides ( const QString &packageName, const QStringList &provides );
    bool insertRequires ( const QString &packageName, const QStringList &requires );

    void endTransaction();
    void startTransaction();

};

#endif

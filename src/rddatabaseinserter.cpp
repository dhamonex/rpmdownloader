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
#include "rddatabaseinserter.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

RDDatabaseInserter::RDDatabaseInserter ( QObject* parent )
    : RDDatabaseHandler ( parent )
{
}


RDDatabaseInserter::~RDDatabaseInserter()
{
}

void RDDatabaseInserter::clear()
{
  clearContent();
}

bool RDDatabaseInserter::init ( const QString & databasePath, const QString connectionName )
{
  setDatabasePath ( databasePath );
  setConnectionName ( connectionName );

  bool success = checkDatabase();

  startTransaction();

  return success;
}

bool RDDatabaseInserter::insertPackage ( const PackageMetaData & data, const bool yumRepo )
{
  if ( !insertMetaData ( data.packageName(), data.architecture(), data.version(), data.size(), data.fileName(), data.shaCheckSum(), data.checkSumAlgorithm(), data.location() ) )
    return false;

  if ( yumRepo ) {
    if ( !insertProvides ( data.packageName(), data.provides() ) )
      return false;

    if ( !insertRequires ( data.packageName(), data.requires() ) )
      return false;
  }

  return true;
}

void RDDatabaseInserter::finish()
{
  endTransaction();
}

void RDDatabaseInserter::rollback()
{
  QSqlDatabase db = getDatabase();

  if ( db.isValid() ) {
    QSqlQuery q ( db );
    q.exec ( "rollback transaction" );
  }
}

bool RDDatabaseInserter::insertProvides ( const QString & packageName, const QStringList & provides )
{
  // get database
  QSqlDatabase db = getDatabase();
  QSqlQuery q ( db );

  QStringListIterator i ( provides );

  while ( i.hasNext() ) {
    if ( !q.exec ( QString ( "insert into provides (requirement, providedBy) values(\"%1\", \"%2\")" ).arg ( i.next() ).arg ( packageName ) ) )
      return false;
  }

  return true;
}

bool RDDatabaseInserter::insertRequires ( const QString & packageName, const QStringList & requires )
{
  // get database
  QSqlDatabase db = getDatabase();
  QSqlQuery q ( db );

  QStringListIterator i ( requires );

  while ( i.hasNext() ) {
    if ( !q.exec ( QString ( "insert into requires (packageName, requires) values(\"%1\", \"%2\")" ).arg ( packageName ).arg ( i.next() ) ) )
      return false;
  }

  return true;
}

void RDDatabaseInserter::endTransaction()
{
  if ( connName().isEmpty() )
    return;

  QSqlDatabase db = getDatabase();;

  QSqlQuery q ( db );

  q.exec ( "end transaction" );

  q.exec ( "PRAGMA synchronous=FULL" );
}

void RDDatabaseInserter::startTransaction()
{
  // commit all inserts as one transaction for a better performance
  QSqlDatabase db = getDatabase();;
  QSqlQuery q ( db );
  q.exec ( "PRAGMA synchronous=OFF" );
  q.exec ( "begin transaction" );
}

bool RDDatabaseInserter::insertMetaData ( const QString& packageName, 
                                          const QString& packageArchitecture, 
                                          const QString& version, 
                                          const quint64  size, 
                                          const QString& fileName, 
                                          const QString  checkSum,
                                          const QString& checkSumAlgorithm,
                                          const QString& location )
{
  // get database
  QSqlDatabase db = getDatabase();
  QSqlQuery q ( db );

  QString query = QString("insert into package_meta_data (packageName, architecture, packageVersion, size, fileName, shaCheckSum, shaCheckSumAlgorithm, location) values (\"%1\", \"%2\", \"%3\", \"%4\", \"%5\", \"%6\", \"%7\", \"%8\")").arg ( packageName ).arg ( packageArchitecture ).arg ( version ).arg ( size ).arg ( fileName ).arg ( checkSum ).arg ( checkSumAlgorithm ).arg( location );
  
  if ( !q.exec ( query ) ) {
    qDebug( qPrintable( query ) );
    return false;
  }

  return true;
}


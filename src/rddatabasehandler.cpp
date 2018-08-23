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
#include "rddatabasehandler.h"

#include <QtSql/QSqlDatabase>
#include <QtCore/QStringList>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtCore/QMap>
#include <QtCore/QVariant>
#include <QtCore/QUuid>

QMap<QString, QStringList> RDDatabaseHandler::tableFields;
QString RDDatabaseHandler::databaseVersion = "0.11.4";

RDDatabaseHandler::RDDatabaseHandler ( QObject * parent )
    : QObject ( parent ), m_wasRecreated ( false )
{
  if ( tableFields.isEmpty() ) {
    // need to fill in fields
    // provides table
    tableFields["provides"] << "id" << "requirement" << "providedBy";

    // pacakge meta data table
    tableFields["package_meta_data"] << "id" << "packageName" << "architecture" << "packageVersion" << "size" << "fileName" << "shaCheckSum" << "shaChecksumAlgorithm" << "location";

    // requires table
    tableFields["requires"] << "id" << "packageName" << "requires";

    // for database version
    tableFields["version"] << "db_version";
  }
}

void RDDatabaseHandler::removeDbConnection ( const QString & connectionName )
{
  if ( QSqlDatabase::contains ( connectionName ) ) {
    QSqlDatabase::removeDatabase ( connectionName );
  }
}

void RDDatabaseHandler::setDatabasePath ( const QString & database )
{
  databasePath = database;
}

void RDDatabaseHandler::setConnectionName ( const QString & name )
{
  connectionName = name + "_" + QUuid::createUuid().toString();
}

RDDatabaseHandler::~ RDDatabaseHandler()
{
}

bool RDDatabaseHandler::checkDatabase()
{
  // set recreated flag to false
  m_wasRecreated = false;

  if ( !QSqlDatabase::drivers().contains ( "QSQLITE" ) ) {
    errMsg = tr ( "Unable to load database, sqlite database driver not available" );
    return false;
  }

  QSqlDatabase db;

  if ( QSqlDatabase::contains ( connectionName ) ) {
    db = getDatabase();

  } else {
    if ( connectionName.isEmpty() ) {
      qDebug ( "empty connection name" );
      return false;
    }

    db = QSqlDatabase::addDatabase ( "QSQLITE", connectionName );

    db.setDatabaseName ( databasePath );

    // open database connection

    if ( !db.open() ) {
      errMsg = tr ( "Could not open database: %1" ).arg ( databasePath );
      return false;
    }
  }

  QStringList tables = db.tables();

  if ( tables.size() == 0 ) { // create database becuse it's empty
    QSqlError err = initDb();

    if ( err.type() == QSqlError::NoError ) {
      return true;

    } else {
      errMsg = tr ( "Could not create database: %1" ).arg ( err.text() );
      return false;
    }
  }

  if ( tables.contains ( "provides", Qt::CaseInsensitive )
       && tables.contains ( "package_meta_data", Qt::CaseInsensitive )
       && tables.contains ( "requires", Qt::CaseInsensitive ) ) {
    // check database structure
    if ( !checkDatabaseStructure() ) {
      // structure wasn't ok recreate database
      qDebug ( "Recreating Database because of corrupt table structure" );
      return recreateDatabase();
    }

  } else {
    qDebug ( "Recreating Database because of missing tables" );
    return recreateDatabase();
  }

  QSqlQuery q ( db );

  if ( !q.exec ( "select * from version limit 1" ) ) {
    return recreateDatabase();
  }

  bool haveVersionString = false;

  while ( q.next() ) {
    haveVersionString = true;

    if ( q.value ( 0 ).toString() != databaseVersion ) {
      qDebug ( "Recreating Database because of old database version" );
      q.clear();
      return recreateDatabase();
    }
  }

  if ( !haveVersionString ) {
    qDebug ( "Recreating Database because of missing version" );
    return recreateDatabase();
  }

  return true;
}

bool RDDatabaseHandler::checkDatabaseStructure()
{
  QSqlDatabase db = getDatabase();

  foreach ( const QString &table, tableFields.keys() ) {
    QSqlRecord rec = db.record ( table );

    foreach ( const QString &field, tableFields.value ( table ) ) {
      if ( !rec.contains ( field ) )
        return false;
    }
  }

  return true;
}

void RDDatabaseHandler::clearContent()
{
  if ( connectionName.isEmpty() )
    return;

  QSqlDatabase db = getDatabase();

  QSqlQuery q ( db );

  q.exec ( "delete from package_meta_data" );

  q.exec ( "delete from provides" );

  q.exec ( "delete from requires" );
}

QSqlDatabase RDDatabaseHandler::getDatabase()
{
  if ( connectionName.isEmpty() ) {
    qCritical ( "Error: No connection name specified" );
    return QSqlDatabase();
  }

  return QSqlDatabase::database ( connectionName );
}

QSqlError RDDatabaseHandler::initDb()
{
  QSqlDatabase db = getDatabase();

  QSqlQuery q ( db );

  if ( !q.exec ( "create table provides(id integer primary key autoincrement, requirement varchar, providedBy varchar)" ) || !q.exec ( "create index 'provides_idx_1' on provides (requirement)" ) || !q.exec ( "create index 'provides_idx_2' on provides (providedBy)" ) ) {
    qDebug ( "failed to create table provides %s", qPrintable ( q.lastError().text() ) );
    return q.lastError();
  }

  if ( !q.exec ( "create table package_meta_data(id integer primary key autoincrement, packageName varchar, architecture varchar, packageVersion varchar, size int, fileName varchar, shaCheckSum varchar, shaCheckSumAlgorithm varchar, location varchar)" ) || !q.exec ( "create index 'package_meta_data_idx_1' on package_meta_data (packageName)" ) ) {
    qDebug ( "failed to create package_meta_data provides %s", qPrintable ( q.lastError().text() ) );
    return q.lastError();
  }

  if ( !q.exec ( "create table requires(id integer primary key autoincrement, packageName varchar, requires varchar(1024))" ) ) {
    qDebug ( "failed to create requires provides %s", qPrintable ( q.lastError().text() ) );
    return q.lastError();
  }

  if ( !q.exec ( "create table version (db_version varchar)" ) ) {
    qDebug ( "failed to create requires version %s", qPrintable ( q.lastError().text() ) );
    return q.lastError();
  }

  if ( !q.exec ( QString ( "insert into version values('%1')" ).arg ( databaseVersion ) ) )
    return q.lastError();

  m_wasRecreated = true;

  return QSqlError();
}

bool RDDatabaseHandler::recreateDatabase()
{
  QSqlDatabase db = getDatabase();
  QStringList tables = db.tables();
  QSqlQuery q ( db );

  foreach ( const QString &table, tableFields.keys() ) {
    if (tables.contains( table ) && !q.exec ( QString ( "drop table %1" ).arg ( table ) ) ) {
      qDebug("Could not clean up database: %s", qPrintable( q.lastError().text() ) );
      return false;
    }
  }

  QSqlError err = initDb();

  if ( err.type() == QSqlError::NoError ) {
    m_wasRecreated = true;
    return true;
  }

  return false;
}


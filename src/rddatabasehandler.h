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
#ifndef RDDATABASEHANDLER_H
#define RDDATABASEHANDLER_H

#include <QObject>

template<class Key, class T >

class QMap;

class QSqlError;

class QSqlDatabase;

class RDDatabaseHandler : public QObject
{
    Q_OBJECT

  public:
    RDDatabaseHandler ( QObject *parent = 0 );

    static void removeDbConnection ( const QString &connectionName );

    void setDatabasePath ( const QString &database ); // sets the path to the database
    void setConnectionName ( const QString &name ); // sets the connection name
    bool dbWasRecreated() const {return m_wasRecreated;}

    QString dbPath() const {return databasePath;}

    QString connName() const {return connectionName;}

    QString errorMsg() const {return errMsg;}

    virtual ~RDDatabaseHandler();

  protected:
    bool checkDatabase(); // checks the database integrity, opens the database and creates new one if needed
    void clearContent(); // clears the complete table contents

    QSqlDatabase getDatabase();

    QString errMsg;

  private:
    QSqlError initDb(); // initialize the database
    bool recreateDatabase();

    bool checkDatabaseStructure(); // check if all fields are correct

    typedef QMap<QString, QStringList> TableFields;

    static TableFields tableFields;
    static QString databaseVersion;

    QString databasePath;
    QString connectionName;

    bool m_wasRecreated;

};

#endif // RDDATABASEHANDLER_H

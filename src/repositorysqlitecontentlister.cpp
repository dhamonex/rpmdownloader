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
#include "repositorysqlitecontentlister.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

RepositorySqliteContentLister::RepositorySqliteContentLister ( QObject * parent )
    : RDDatabaseHandler ( parent )
{
}

bool RepositorySqliteContentLister::fetchContent ( const QString & databasePath, const QString & repoName, const QStringList & architectures )
{
  setDatabasePath ( databasePath );
  setConnectionName ( repoName );
  contents.clear();

  if ( !checkDatabase() || architectures.isEmpty() ) {
    return false;
  }

  QSqlDatabase db = getDatabase();

  QSqlQuery q ( db );

  QString query ( "select packageName, architecture, packageVersion, size, fileName, shaCheckSum, shaCheckSumAlgorithm, location from package_meta_data where architecture=\"" );

  for ( int i = 0; i < architectures.size(); ++i ) {
    query += architectures.at ( i );
    query += "\" ";

    if ( i + 1 < architectures.size() ) {
      query += "or architecture=\"";
    }
  }

  // qDebug("%s", qPrintable(query));

  if ( !q.exec ( query ) )
    return false;

  while ( q.next() ) {
    PackageMetaData metaData ( q.value ( 4 ).toString(), // file name
                               q.value ( 0 ).toString(), // package name
                               q.value ( 2 ).toString(), // package version
                               q.value ( 3 ).toInt(), // size
                               q.value ( 1 ).toString(), // arch
                               q.value ( 5 ).toString(), // check sum
                               q.value ( 6 ).toString(), // check sum algorithm
                               q.value ( 7 ).toString() ); // location

    if ( contents.contains ( metaData.architecture() ) ) { // check if arch is already known
      if ( contents.value ( metaData.architecture() ).contains ( metaData.packageName() ) ) {
        // package is already in the hash so check if current version is newer
        if ( contents.value ( metaData.architecture() ).value ( metaData.packageName() ) < metaData ) // newer version was found replace old with new one
          contents[metaData.architecture() ].insert ( metaData.packageName(), metaData );

      } else { // insert package because it's not already in there
        contents[metaData.architecture() ].insert ( metaData.packageName(), metaData );
      }

    } else { // achritecture does not exist
      MultiplePackageMetaData tmpHash;
      tmpHash.insert ( metaData.packageName(), metaData );
      contents.insert ( metaData.architecture(), tmpHash );
    }
  }

  return true;
}

RepositorySqliteContentLister::~ RepositorySqliteContentLister()
{
}



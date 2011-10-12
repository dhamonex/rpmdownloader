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
#include "rdpackagelisterthread.h"

#include <QDir>

RDPackageListerThread::RDPackageListerThread ( QObject* parent )
    : QThread ( parent )
{
}


RDPackageListerThread::~RDPackageListerThread()
{
}

void RDPackageListerThread::run()
{
  error = false;
  errMsg.clear();
  repositoryMetaInformations.clear();
  localPackageinformations.clear();

  RepositorySqliteContentLister dbHandler;

  if ( dbHandler.fetchContent ( databasePath, repoName, architectures ) ) {
    repositoryMetaInformations = dbHandler.getMetaData();

  } else {
    error = true;
    errMsg = dbHandler.errorMsg();
    return;
  }

  getLocalContents();
}

void RDPackageListerThread::getLocalContents()
{
  // read direcotry contents of local packages
  foreach ( const QString &arch, architectures ) {
    MultiplePackageMetaData archDependMetaDatas = getContentsFromDir ( QDir ( localPackagePath + "/" + arch ), arch );
    localPackageinformations.insert ( arch, archDependMetaDatas );

    if ( error ) // error occured
      return;
  }
}

MultiplePackageMetaData RDPackageListerThread::getContentsFromDir ( const QDir & dir, const QString & arch )
{
  MultiplePackageMetaData contents;

  if ( !dir.exists() ) {
    qDebug ( "could not read directory contents of %s. Maybe it is not yet created.", qPrintable ( dir.absolutePath() ) );
    return contents;
  }

  QFileInfoList fileInfoList = dir.entryInfoList ( QStringList() << "*.rpm", QDir::Files );

  foreach ( const QFileInfo &fileInfo, fileInfoList ) {
    PackageMetaData metaData ( fileInfo.fileName(), arch, fileInfo.size() );

    if ( contents.contains ( metaData.packageName() ) ) { // at least one verision already found
      if ( metaData > contents.value ( metaData.packageName() ) ) // the current version is newer than the stored one
        contents.insert ( metaData.packageName(), metaData );

    } else { // no version in the contents add current version
      contents.insert ( metaData.packageName(), metaData );
    }
  }

  return contents;
}



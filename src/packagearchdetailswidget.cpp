/***************************************************************************
 *   Copyright (C) 2009 by Dirk Hartmann                                   *
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

#include "packagearchdetailswidget.h"


PackageArchDetailsWidget::PackageArchDetailsWidget ( const QString &arch, const PackageMetaData& remoteMetaData, const PackageMetaData& localMetaData, QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
  setupUi ( this );

  archNameGroupBox->setTitle ( arch );

  QString fileName = checkFileNameIfEmpty ( remoteMetaData.fileName() );

  remotePackageNameDisplayLineEdit->setText ( fileName );
  remotePackageDisplaySizeLineEdit->setText ( remoteMetaData.sizeAsString() );

  fileName = checkFileNameIfEmpty ( localMetaData.fileName() );
  localPackageNameDisplayLineEdit->setText ( fileName );
  localPackageDisplaySizeLineEdit->setText ( localMetaData.sizeAsString() );
}


QString PackageArchDetailsWidget::checkFileNameIfEmpty ( const QString& fileName )
{
  if ( fileName.isEmpty() )
    return tr ( "Not Available" );

  return fileName;
}

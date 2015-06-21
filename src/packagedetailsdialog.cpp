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

#include "packagedetailsdialog.h"

#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

#include "packagearchdetailswidget.h"

PackageDetailsDialog::PackageDetailsDialog ( const Package &package, const QIcon& icon, QWidget* parent, Qt::WindowFlags f )
    : QDialog ( parent, f )
{
  QPushButton *okButton = new QPushButton ( tr ( "OK" ) );

  connect ( okButton, SIGNAL ( clicked() ), this, SLOT ( accept() ) );

  QVBoxLayout *vboxLayout = new QVBoxLayout;

  MetaData::ArchitecutreDependentMetaData remoteMetaData = package.getRemotePackageMetaData();
  MetaData::ArchitecutreDependentMetaData localMetaData = package.getLocalPackageMetaData();

  // set label text
  QHBoxLayout *packageNameLayout = new QHBoxLayout;
  QLabel *packageNameLabel = new QLabel;
  packageNameLabel->setAlignment ( Qt::AlignCenter );

  QLabel *iconLabel = new QLabel;

  packageNameLabel->setText ( QString ( "<h1>%1</h1>" ).arg ( package.packageName() ) );
  iconLabel->setPixmap ( icon.pixmap ( QSize ( 32, 32 ) ) );

  packageNameLayout->addWidget ( iconLabel, Qt::AlignLeft );
  packageNameLayout->addWidget ( packageNameLabel, Qt::AlignCenter );

  vboxLayout->addLayout ( packageNameLayout );

  MetaData::ArchitecutreDependentMetaData::const_iterator archIter = remoteMetaData.begin();
  const MetaData::ArchitecutreDependentMetaData::const_iterator archEndIter = remoteMetaData.end();

  for ( ; archIter != archEndIter; ++archIter ) {
    PackageArchDetailsWidget *pdw = new PackageArchDetailsWidget ( archIter.key(), archIter.value(), localMetaData.value ( archIter.key() ), this );
    vboxLayout->addWidget ( pdw );
  }

  QHBoxLayout *hbOkButtonLayout = new QHBoxLayout;

  hbOkButtonLayout->addStretch();
  hbOkButtonLayout->addWidget ( okButton );
  hbOkButtonLayout->addStretch();

  vboxLayout->addLayout ( hbOkButtonLayout );
  setLayout ( vboxLayout );

  layout()->setSizeConstraint ( QLayout::SetFixedSize );
}

PackageDetailsDialog::~PackageDetailsDialog()
{

}


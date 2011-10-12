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

#ifndef PACKAGEARCHDETAILSWIDGET_H
#define PACKAGEARCHDETAILSWIDGET_H

#include <QWidget>
#include "ui_packagearchdetailswidget.h"

#include "packagemetadata.h"


class PackageArchDetailsWidget : public QWidget, private Ui::PackageArchDetailsWidget
{
    Q_OBJECT

  public:
    PackageArchDetailsWidget ( const QString &arch, const PackageMetaData &remoteMetaData, const PackageMetaData &localMetaData, QWidget* parent = 0, Qt::WindowFlags f = 0 );

  private:
    static QString checkFileNameIfEmpty ( const QString &fileName );
};

#endif // PACKAGEARCHDETAILSWIDGET_H

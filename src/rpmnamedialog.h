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

#ifndef RPMNAMEDIALOG_H
#define RPMNAMEDIALOG_H

#include <QDialog>
#include "ui_rpmnamedialog.h"

class RpmNameDialog : public QDialog, private Ui::RpmNameDialog
{
    Q_OBJECT

  public:
    RpmNameDialog ( const QString name, const QStringList rpmNamesList, bool canResolveDeps = false, bool showDepsCheckbox = false, QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~RpmNameDialog();
    /*$PUBLIC_FUNCTIONS$*/
    QString getName() const;
    bool resolveDeps() const;

  public slots:
    /*$PUBLIC_SLOTS$*/

  protected:
    /*$PROTECTED_FUNCTIONS$*/

  protected slots:
    /*$PROTECTED_SLOTS$*/
    void on_nameLineEdit_textChanged ( const QString &text );

};

#endif


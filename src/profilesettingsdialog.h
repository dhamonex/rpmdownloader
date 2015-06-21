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

#ifndef PROFILESETTINGSDIALOG_H
#define PROFILESETTINGSDIALOG_H

#include <QtWidgets//QDialog>
#include "ui_profilesettingsdialog.h"

#include "repositoryprofile.h"

class ProfileSettingsDialog : public QDialog, private Ui::profileSettingsDialog
{
    Q_OBJECT

  public:
    ProfileSettingsDialog ( RepositoryProfile *repositoryProfile, QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~ProfileSettingsDialog();
    /*$PUBLIC_FUNCTIONS$*/

  public slots:
    /*$PUBLIC_SLOTS$*/
    void accept();

  protected slots:
    /*$PROTECTED_SLOTS$*/
    void on_profileNameLineEdit_textChanged ( const QString &text );
    void on_browseDownloadDirButton_clicked();
    void on_completeUrlLineEdit_textChanged ( const QString &text );
    void on_repositoryTypeComboBox_currentIndexChanged ( int index );

  private:
    RepositoryProfile *profile;

};

#endif


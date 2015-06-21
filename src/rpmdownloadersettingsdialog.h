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

#ifndef RPMDOWNLOADERSETTINGSDIALOG_H
#define RPMDOWNLOADERSETTINGSDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_rpmdownloadersettingsdialog.h"
#include "rpmdownloadersettings.h"

class RpmDownloaderSettingsDialog : public QDialog, private Ui::RpmDownloaderSettingsDialog
{
    Q_OBJECT

  public:
    RpmDownloaderSettingsDialog ( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~RpmDownloaderSettingsDialog();

  public slots:
    void accept();

  private slots:
    void on_selectCacheDirPushButton_clicked();
    void on_selectTempDirPushButton_clicked();

};

#endif


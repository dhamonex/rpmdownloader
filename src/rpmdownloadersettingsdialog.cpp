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


#include "rpmdownloadersettingsdialog.h"

#include <QtWidgets/QFileDialog>

RpmDownloaderSettingsDialog::RpmDownloaderSettingsDialog ( QWidget* parent, Qt::WindowFlags fl )
    : QDialog ( parent, fl ), Ui::RpmDownloaderSettingsDialog()
{
  setupUi ( this );
  layout()->setSizeConstraint ( QLayout::SetFixedSize );

  if ( rpmDownloaderSettings().deleteOldVersions() ) {
    keepOldVersionsCheckBox->setCheckState ( Qt::Checked );
  }

  if ( rpmDownloaderSettings().useMemDbSatSolving() ) {
    useMemDbCheckBox->setCheckState ( Qt::Checked );
  }

  if ( rpmDownloaderSettings().doCheckSumCheckOnDownloadedPackages() ) {
    doCheckSumCheck->setCheckState ( Qt::Checked );
  }

  cacheDirLineEdit->setText ( rpmDownloaderSettings().cacheDir().absolutePath() );

  tempDirLineEdit->setText ( rpmDownloaderSettings().tempDir().absolutePath() );
  checksumCommandLineEdit->setText ( rpmDownloaderSettings().checksumCommand() );
  gunzipCommandLineEdit->setText ( rpmDownloaderSettings().gunzipCommand() );

  updateIntervalSpinBox->setValue ( rpmDownloaderSettings().updateInterval() / 1000 );

  maxPrimaryMemSizeSpinBox->setValue ( rpmDownloaderSettings().maximumPrimaryFileSizeForMemoryLoad() / 1000000 );
}

RpmDownloaderSettingsDialog::~RpmDownloaderSettingsDialog()
{
}

/*$SPECIALIZATION$*/
void RpmDownloaderSettingsDialog::accept()
{
  if ( keepOldVersionsCheckBox->checkState() & Qt::Checked ) {
    rpmDownloaderSettings().setDeleteOldVersions ( true );

  } else {
    rpmDownloaderSettings().setDeleteOldVersions ( false );
  }

  if ( useMemDbCheckBox->checkState() & Qt::Checked ) {
    rpmDownloaderSettings().setMemDbSatSolving ( true );

  } else {
    rpmDownloaderSettings().setMemDbSatSolving ( false );
  }

  if ( doCheckSumCheck->checkState() & Qt::Checked ) {
    rpmDownloaderSettings().setDoCheckSumCheckOnDownloadedPackages ( true );

  } else {
    rpmDownloaderSettings().setDoCheckSumCheckOnDownloadedPackages ( false );
  }

  rpmDownloaderSettings().setUpdateInterval ( updateIntervalSpinBox->value() * 1000 );

  rpmDownloaderSettings().setCacheDir ( QDir ( cacheDirLineEdit->text() ) );
  rpmDownloaderSettings().setTempDir ( QDir ( tempDirLineEdit->text() ) );
  rpmDownloaderSettings().setChecksumCommand ( checksumCommandLineEdit->text() );
  rpmDownloaderSettings().setGunzipCommand ( gunzipCommandLineEdit->text() );
  rpmDownloaderSettings().setMaximumPrimaryFileSizeForMemoryLoad ( maxPrimaryMemSizeSpinBox->value() * 1000000 );

  // call base class
  QDialog::accept();
}

void RpmDownloaderSettingsDialog::on_selectCacheDirPushButton_clicked()
{
  QString newDir = QFileDialog::getExistingDirectory ( this, tr ( "Select Cache Directory" ),
                   cacheDirLineEdit->text(),
                   QFileDialog::ShowDirsOnly
                   | QFileDialog::DontResolveSymlinks );

  if ( !newDir.isEmpty() )
    cacheDirLineEdit->setText ( newDir );
}

void RpmDownloaderSettingsDialog::on_selectTempDirPushButton_clicked()
{
  QString newDir = QFileDialog::getExistingDirectory ( this, tr ( "Select Temp Directory" ),
                   tempDirLineEdit->text(),
                   QFileDialog::ShowDirsOnly
                   | QFileDialog::DontResolveSymlinks );

  if ( !newDir.isEmpty() )
    tempDirLineEdit->setText ( newDir );
}



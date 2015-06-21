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
#include "profilesettingsdialog.h"

#include <QtWidgets/QFileDialog>

ProfileSettingsDialog::ProfileSettingsDialog ( RepositoryProfile *repositoryProfile,
    QWidget* parent, Qt::WindowFlags fl )
    : QDialog ( parent, fl ), Ui::profileSettingsDialog()
{
  setupUi ( this );

  layout()->setSizeConstraint ( QLayout::SetFixedSize );

  profileNameLineEdit->setText ( repositoryProfile->profileName() );
  completeUrlLineEdit->setText ( repositoryProfile->serverUrl().toString() );
  downloadDirLineEdit->setText ( repositoryProfile->downloadDir() );

  repositoryTypeComboBox->setCurrentIndex ( static_cast<int> ( repositoryProfile->repoType() ) );

  if ( repositoryProfile->architecures() & RPM::i586 )
    i586CheckBox->setCheckState ( Qt::Checked );

  if ( repositoryProfile->architecures() & RPM::i686 )
    i686CheckBox->setCheckState ( Qt::Checked );

  if ( repositoryProfile->architecures() & RPM::x86_64 )
    x86_64CheckBox->setCheckState ( Qt::Checked );

  if ( repositoryProfile->architecures() & RPM::noarch )
    noarchCheckBox->setCheckState ( Qt::Checked );

  profile = repositoryProfile;
}

ProfileSettingsDialog::~ProfileSettingsDialog()
{
}

/*$SPECIALIZATION$*/

void ProfileSettingsDialog::accept()
{
  profile->setProfileName ( profileNameLineEdit->text() );
  profile->setServerUrl ( completeUrlLineEdit->text() );
  profile->setDownloadDir ( downloadDirLineEdit->text() );

  profile->setRepoType ( static_cast<RepositoryType> ( repositoryTypeComboBox->currentIndex() ) );

  RPM::Architectures archs = RPM::UNDEFINED;

  if ( i586CheckBox->checkState() & Qt::Checked )
    archs = RPM::Architectures ( archs | RPM::i586 );

  if ( i686CheckBox->checkState() & Qt::Checked )
    archs = RPM::Architectures ( archs | RPM::i686 );

  if ( x86_64CheckBox->checkState() & Qt::Checked )
    archs = RPM::Architectures ( archs | RPM::x86_64 );

  if ( noarchCheckBox->checkState() & Qt::Checked )
    archs = RPM::Architectures ( archs | RPM::noarch );

  profile->setArchitectures ( archs );

  QDialog::accept();
}

void ProfileSettingsDialog::on_profileNameLineEdit_textChanged ( const QString & text )
{
  if ( text.isEmpty() )
    okButton->setEnabled ( false );
  else
    okButton->setEnabled ( true );
}

void ProfileSettingsDialog::on_browseDownloadDirButton_clicked()
{
  QString newDir = QFileDialog::getExistingDirectory ( this, tr ( "Open Directory" ),
                   downloadDirLineEdit->text(),
                   QFileDialog::ShowDirsOnly
                   | QFileDialog::DontResolveSymlinks );

  if ( !newDir.isEmpty() )
    downloadDirLineEdit->setText ( newDir );
}

void ProfileSettingsDialog::on_completeUrlLineEdit_textChanged ( const QString& text )
{
  if ( !QUrl ( text ).isValid() ) {
    okButton->setEnabled ( false );

  } else {
    okButton->setEnabled ( true );
  }

  if ( static_cast<RepositoryType> ( repositoryTypeComboBox->currentIndex() ) == PLAIN ) {

    if ( QUrl ( text ).scheme() == "http" )
      okButton->setEnabled ( false );
    else
      okButton->setEnabled ( true );
  }
}

void ProfileSettingsDialog::on_repositoryTypeComboBox_currentIndexChanged ( int /*index*/ )
{
  if ( static_cast<RepositoryType> ( repositoryTypeComboBox->currentIndex() ) == PLAIN ) {
    if ( QUrl ( completeUrlLineEdit->text() ).scheme() == "http" )
      okButton->setEnabled ( false );
    else
      okButton->setEnabled ( true );

  } else {
    okButton->setEnabled ( true );
  }
}

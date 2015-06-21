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
#include "rpmnamedialog.h"

#include <QtWidgets/QCompleter>

RpmNameDialog::RpmNameDialog ( const QString name, const QStringList rpmNamesList, bool canResolveDeps,
                               bool showDepsCheckbox, QWidget * parent, Qt::WindowFlags fl )
    : QDialog ( parent, fl ), Ui::RpmNameDialog()
{
  setupUi ( this );
  layout()->setSizeConstraint ( QLayout::SetFixedSize );

  QCompleter *completer = new QCompleter ( rpmNamesList, this );
  completer->setCaseSensitivity ( Qt::CaseInsensitive );
  nameLineEdit->setCompleter ( completer );

  nameLineEdit->setText ( name );
  nameLineEdit->setSelection ( 0, name.length() );

  if ( canResolveDeps )
    solveDepsCheckBox->setCheckState ( Qt::Checked );
  else
    solveDepsCheckBox->setCheckState ( Qt::Unchecked );

  if ( !canResolveDeps )
    solveDepsCheckBox->setDisabled ( true );

  if ( !showDepsCheckbox ) {
    solveDepsCheckBox->hide();
  }
}

RpmNameDialog::~RpmNameDialog()
{
}

/*$SPECIALIZATION$*/

QString RpmNameDialog::getName() const
{
  return nameLineEdit->text();
}

void RpmNameDialog::on_nameLineEdit_textChanged ( const QString & text )
{
  if ( text.isEmpty() )
    okButton->setEnabled ( false );
  else
    okButton->setEnabled ( true );
}

bool RpmNameDialog::resolveDeps() const
{
  if ( solveDepsCheckBox->checkState() == Qt::Checked )
    return true;

  return false;
}


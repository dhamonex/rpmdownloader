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
#include "searchfieldlineedit.h"

#include <QtWidgets/QToolButton>
#include <QtWidgets/QStyle>
#include <QtCore/QTimer>

SearchFieldLineEdit::SearchFieldLineEdit ( QWidget* parent )
    : QLineEdit ( parent ), emptyHelpText ( ), submitTimeout ( 200 )
{
  clearButton = new QToolButton ( this );
  clearButton->setCursor ( Qt::ArrowCursor );
  clearButton->setStyleSheet ( "QToolButton { border: none; padding: 0px; }" );
  clearButton->hide();
  updateMinimumSizeHint();

  waitUntilSubmitNewTextTimer = new QTimer ( this );
  waitUntilSubmitNewTextTimer->setSingleShot ( true );

  connect ( clearButton, SIGNAL ( clicked() ), this, SLOT ( clearLineEdit() ) );
  connect ( this, SIGNAL ( textEdited ( const QString& ) ), this, SLOT ( updateCloseButton ( const QString& ) ) );
  connect ( this, SIGNAL ( editingFinished() ), this, SLOT ( lostFocus() ) );
  connect ( waitUntilSubmitNewTextTimer, SIGNAL ( timeout() ), this, SLOT ( submitTimerExpired() ) );
}

void SearchFieldLineEdit::resizeEvent ( QResizeEvent* /*event*/ )
{
  QSize size = clearButton->sizeHint();
  int frameWidth = style()->pixelMetric ( QStyle::PM_DefaultFrameWidth );
  clearButton->move ( rect().right() - frameWidth - size.width(),
                      ( rect().bottom() - 1.5 - size.height() ) / 2 );
}

void SearchFieldLineEdit::focusInEvent ( QFocusEvent* event )
{
  if ( !clearButton->isVisible() ) {
    clear();
    setStyleSheet ( "QLineEdit {color: black}" );
  }

  QLineEdit::focusInEvent ( event );
}

void SearchFieldLineEdit::updateCloseButton ( const QString& text )
{
  clearButton->setVisible ( !text.isEmpty() );

  // stop timer if active

  if ( waitUntilSubmitNewTextTimer->isActive() )
    waitUntilSubmitNewTextTimer->stop();

  if ( !text.isEmpty() ) // start timer only if field is not empty
    waitUntilSubmitNewTextTimer->start ( submitTimeout );
  else // direct submit the empty text if field is empty
    emit ( newSearchText ( QString() ) );
}

void SearchFieldLineEdit::lostFocus()
{
  if ( hasFocus() || clearButton->isVisible() )
    return;

  displayEmptyLineEditHelpText();
}

void SearchFieldLineEdit::setDisplayedOnEmptyText ( const QString& text )
{
  emptyHelpText = text;

  if ( hasFocus() && clearButton->isVisible() ) {
    return;
  }

  displayEmptyLineEditHelpText();
}

void SearchFieldLineEdit::displayEmptyLineEditHelpText()
{
  setText ( emptyHelpText );
  setStyleSheet ( "QLineEdit {color: grey}" );
}

void SearchFieldLineEdit::clearLineEdit()
{
  clear();

  // also hide clear button
  clearButton->hide();

  if ( waitUntilSubmitNewTextTimer->isActive() )
    waitUntilSubmitNewTextTimer->stop();

  emit ( newSearchText ( QString() ) ); // emit empty string
}

void SearchFieldLineEdit::updateMinimumSizeHint()
{
  int frameWidth = style()->pixelMetric ( QStyle::PM_DefaultFrameWidth );

  QSize msz = minimumSizeHint();
  setMinimumSize ( qMax ( msz.width(), clearButton->sizeHint().height() + frameWidth * 2 + 2 ),
                   qMax ( msz.height(), clearButton->sizeHint().height() + frameWidth * 2 + 2 ) );
}

void SearchFieldLineEdit::submitTimerExpired()
{
  emit ( newSearchText ( text() ) );
}

void SearchFieldLineEdit::setClearButtonIcon ( const QPixmap& icon )
{
  cIcon = icon;

  clearButton->setIcon ( QIcon ( icon ) );
  clearButton->setIconSize ( icon.size() );
  updateMinimumSizeHint();
}

void SearchFieldLineEdit::setTextSubmitTimeout ( const int timeout )
{
  submitTimeout = timeout;

  // update running timer

  if ( waitUntilSubmitNewTextTimer->isActive() ) {
    waitUntilSubmitNewTextTimer->stop();
    waitUntilSubmitNewTextTimer->start ( submitTimeout );
  }
}

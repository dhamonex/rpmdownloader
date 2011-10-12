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
#ifndef SEARCHFIELDLINEEDIT_H
#define SEARCHFIELDLINEEDIT_H

#include <QLineEdit>

class QToolButton;

class SearchFieldLineEdit : public QLineEdit
{
    Q_OBJECT
    Q_PROPERTY ( QString displayedOnEmptyText READ displayedOnEmptyText WRITE setDisplayedOnEmptyText );
    Q_PROPERTY ( QPixmap clearButtonIcon READ clearButtonIcon WRITE setClearButtonIcon );
    Q_PROPERTY ( int textSubmitTimout READ textSubmitTimout WRITE setTextSubmitTimeout );
  public:
    SearchFieldLineEdit ( QWidget *parent = 0 );

    // properties
    QString displayedOnEmptyText() const {return emptyHelpText;}

    void setDisplayedOnEmptyText ( const QString &text );

    QPixmap clearButtonIcon() const {return cIcon;}

    void setClearButtonIcon ( const QPixmap &icon );

    quint32 textSubmitTimout() const {return submitTimeout;}

    void setTextSubmitTimeout ( const int timeout );

  signals:
    void newSearchText ( const QString & );

  protected:
    void resizeEvent ( QResizeEvent * event );
    void focusInEvent ( QFocusEvent *event );

  private slots:
    void updateCloseButton ( const QString &text );
    void lostFocus();
    void clearLineEdit();
    void submitTimerExpired();

  private:
    void displayEmptyLineEditHelpText();
    void updateMinimumSizeHint();

    QToolButton *clearButton;

    QString emptyHelpText;
    QPixmap cIcon;

    // wait a while before emit new search text
    int submitTimeout;
    QTimer *waitUntilSubmitNewTextTimer;
};

#endif // SEARCHFIELDLINEEDIT_H

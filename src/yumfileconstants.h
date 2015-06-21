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
#ifndef YUMFILECONSTANTS_H
#define YUMFILECONSTANTS_H

#include <QtCore/QString>

namespace Yum {
  extern const QString filelistGzFileName;
  extern const QString filelistFileName;
  extern const QString tmpFilelistGzFileName;
  extern const QString tmpFilelistFileName;

  extern const QString otherGzFileName;
  extern const QString otherFileName;
  extern const QString tmpOtherGzFileName;
  extern const QString tmpOtherFileName;

  extern const QString primaryGzFileName;
  extern const QString primaryFileName;
  extern const QString tmpPrimaryGzFileName;
  extern const QString tmpPrimaryFileName;

  extern const QString repoServerDir;
  extern const QString repomdFileName;
};

#endif // YUMFILECONSTANTS_H

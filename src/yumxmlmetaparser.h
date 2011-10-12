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

#ifndef YUMXMLMETAPARSER_H
#define YUMXMLMETAPARSER_H

#include <QXmlStreamReader>
#include "rddatabaseinserter.h"

class YumXmlMetaParser : public QXmlStreamReader
{

  public:
    YumXmlMetaParser();

    bool initDatabase ( const QString &dbPath, const QString &repoName );
    bool read ( QIODevice *device );
    void abortUpdate();

    QString readError() const {
      return errMsg;
    }

  private:
    void parseMetaDataElement();
    void parsePackageElement();
    void readUnknownElement();
    void readNameElement();
    void readLocationElement();
    void readArchElement();
    void readSizeElement();
    void readVersionElement();
    void readFormatTag();
    void readChecksumTag();
    void readProvidesAndRequires ( const bool isProvidesElement );

    RDDatabaseInserter dbHandler;
    QString errMsg;
    PackageMetaData currentPackageData;
    volatile bool abort;
};

#endif // YUMXMLMETAPARSER_H


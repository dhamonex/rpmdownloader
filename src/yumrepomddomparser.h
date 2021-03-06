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
#ifndef YUMREPOMDDOMPARSER_H
#define YUMREPOMDDOMPARSER_H

#include <QString>
#include <QMap>

class QDomElement;

struct FileMetaInfo {
  QString checksum; // sha!!!
  QString checksumAlgorithm;
  QString timestamp;
  QString location;
};

class YumRepomdDomParser
{
  public:
    YumRepomdDomParser();

    bool parseRepomd ( const QString &repomdFileName );
    QMap<QString, FileMetaInfo> getParsedMetaInfos() const {return parsedFileInfos;}

    QString errorMessage() const {return errMsg;}

    ~YumRepomdDomParser();

  private:
    bool parseEntry ( const QDomElement &element );

    QString errMsg;
    QMap <QString, FileMetaInfo> parsedFileInfos; // filename, infos (filellist.xml.gz ...)
};

#endif

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
#include "yumrepomddomparser.h"

#include <QtXml/QDomDocument>
#include <QtCore/QFile>
#include <QtWidgets/QApplication>
#include <QtCore/QRegExp>
#include "checksumcheck.h"

namespace RepoMDXmlTags {
  const QString RepoMd   ( "repomd" );
  const QString Data     ( "data" );
  const QString Checksum ( "checksum" );
  const QString TimeStamp( "timestamp" );
  const QString Location ( "location" );
}

YumRepomdDomParser::YumRepomdDomParser()
{
}


YumRepomdDomParser::~YumRepomdDomParser()
{
}

bool YumRepomdDomParser::parseRepomd ( const QString & repomdFileName )
{
  parsedFileInfos.clear();

  QDomDocument repomd ( RepoMDXmlTags::RepoMd );
  QFile file ( repomdFileName );

  if ( !file.exists() ) { // parsing failed becuase file does not exist
    errMsg = QApplication::translate ( "repomdDomParser", "File does not exist" );
    return false;
  }

  if ( !file.open ( QIODevice::ReadOnly ) ) {
    errMsg = QApplication::translate ( "repomdDomParser", "could not open file for reading %1" ).arg ( file.fileName() );
  }

  if ( !repomd.setContent ( &file, true, &errMsg ) ) {
    file.close();
    return false;
  }

  file.close();

  QDomElement root = repomd.documentElement();

  if ( root.tagName() != RepoMDXmlTags::RepoMd ) {
    // it's not a valid repomd file
    errMsg = QApplication::translate ( "repomdDomParser", "not a valid repomd file" );
    return false;
  }

  bool parsedOk = true;

  QDomNode node = root.firstChild();

  while ( !node.isNull() ) {
    if ( node.toElement().tagName() == RepoMDXmlTags::Data ) {
      parsedOk = parseEntry ( node.toElement() );
    }

    if ( !parsedOk )
      return false;

    node = node.nextSibling();
  }

  return true;
}

bool YumRepomdDomParser::parseEntry ( const QDomElement &element )
{
  FileMetaInfo fileInfo;
  QString fileName ( element.attribute ( "type" ) );

  QDomNode node = element.firstChild();

  while ( !node.isNull() ) {
    // qDebug("%s", qPrintable(node.toElement().tagName()));
    if ( node.toElement().tagName() == RepoMDXmlTags::Checksum ) {
      bool unknownChecksum;
      fileInfo.checksumAlgorithm = CheckSumCheck::extractAlgorithmFromTag(node.toElement().attribute ( "type" ),
                                                                          &unknownChecksum);
      
      if ( unknownChecksum ) {
        errMsg = QApplication::translate ( "repomdDomParser", "checksum type not supported" );
        return false;
      }

      QDomNode childNode = node.firstChild();

      if ( childNode.nodeType() == QDomNode::TextNode ) {
        fileInfo.checksum = childNode.toText().data();
      }

    } else if ( node.toElement().tagName() == RepoMDXmlTags::TimeStamp ) {
      
      QDomNode childNode = node.firstChild();

      if ( childNode.nodeType() == QDomNode::TextNode ) {
        fileInfo.timestamp = childNode.toText().data();
      }
    
      
    } else if ( node.toElement().tagName() == RepoMDXmlTags::Location ) {
        fileInfo.location = node.toElement().attribute( "href" );
    }

    node = node.nextSibling();
  }

  parsedFileInfos.insert ( fileName, fileInfo );

  return true;
}



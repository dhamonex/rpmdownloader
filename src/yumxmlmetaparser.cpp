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

#include "yumxmlmetaparser.h"
#include "checksumcheck.h"

YumXmlMetaParser::YumXmlMetaParser()
    : QXmlStreamReader(), abort ( false )
{
}

bool YumXmlMetaParser::initDatabase ( const QString& dbPath, const QString& repoName )
{
  if ( !dbHandler.init ( dbPath, repoName ) ) {
    errMsg = dbHandler.errorMsg();
    return false;
  }

  dbHandler.clear();

  return true;
}

bool YumXmlMetaParser::read ( QIODevice* device )
{
  setDevice ( device );
  abort = false;

  while ( !atEnd() && !abort ) {
    if ( !error() ) {
      readNext();

    } else

      if ( error() == PrematureEndOfDocumentError ) { // ignore PrematureEndOfDocumentError
        qDebug ( "recovering from PrematureEndOfDocumentError for %s at %li", qPrintable ( name().toString() ), static_cast<long int> ( lineNumber() ) );
        readNext();
      }

    if ( isStartElement() ) {
      if ( name() == "metadata" ) {
        parseMetaDataElement();

      } else {
        // qDebug("Unknown Element %s", qPrintable(name().toString()));
        readUnknownElement();
      }
    }
  }

  if ( error() != QXmlStreamReader::NoError && error() != QXmlStreamReader::PrematureEndOfDocumentError ) {
    errMsg = errorString();
    return false;
  }

  if ( abort )
    dbHandler.clear();

  dbHandler.finish();

  return true;
}

void YumXmlMetaParser::parseMetaDataElement()
{
  Q_ASSERT ( isStartElement() && name() == "metadata" );

  while ( !atEnd() && !abort ) {
    readNext();

    if ( isEndElement() ) {
      // qDebug("End Element %s", qPrintable(name().toString()));
      break;
    }

    if ( isStartElement() ) {
      if ( name() == "package" )
        parsePackageElement();
      else
        readUnknownElement();
    }
  }
}

void YumXmlMetaParser::parsePackageElement()
{
  Q_ASSERT ( isStartElement() && name() == "package" && attributes().value ( "type" ) == "rpm" );

  currentPackageData.clear(); // clear old package content

  while ( !atEnd() && !abort ) {
    readNext();

    if ( isEndElement() ) { // reached end of package tag
      // qDebug("End Element %s", qPrintable(name().toString()));
      break;
    }

    if ( isStartElement() ) {
      if ( name() == "name" ) {
        readNameElement();

      } else

        if ( name() == "arch" ) {
          readArchElement();

        } else

          if ( name() == "location" ) {
            readLocationElement();

          } else

            if ( name() == "size" ) {
              readSizeElement();

            } else

              if ( name() == "version" ) {
                readVersionElement();

              } else

                if ( name() == "format" ) {
                  readFormatTag();

                } else

                  if ( name() == "checksum" ) {
                    readChecksumTag();

                  } else {
                    readUnknownElement();
                  }
    }
  }

  if ( !abort )
    dbHandler.insertPackage ( currentPackageData, true );

  // qDebug("package inserted %s", qPrintable(currentPackageData.packageName()));
}

void YumXmlMetaParser::readNameElement()
{
  Q_ASSERT ( isStartElement() && name() == "name" );

  currentPackageData.setPackageName ( readElementText() );
  // this should forward to the end tag of name
}

void YumXmlMetaParser::readArchElement()
{
  Q_ASSERT ( isStartElement() && name() == "arch" );

  currentPackageData.setArch ( readElementText() );
  // this should forward to the end tag of arch
}

void YumXmlMetaParser::readLocationElement()
{
  Q_ASSERT ( isStartElement() && name() == "location" );

  QString location = attributes().value ( "href" ).toString();
  currentPackageData.setFile ( location.section ( "/", -1 ) );
  currentPackageData.setLocation ( location );

  readNext();
}

void YumXmlMetaParser::readSizeElement()
{
  Q_ASSERT ( isStartElement() && name() == "size" );

  currentPackageData.setSize ( attributes().value ( "package" ).toString().toInt() );
  readNext(); // forward to end of tag
}

void YumXmlMetaParser::readVersionElement()
{
  Q_ASSERT ( isStartElement() && name() == "version" );

  currentPackageData.setVersion ( attributes().value ( "ver" ).toString() + "-" + attributes().value ( "rel" ).toString() );
  readNext(); // forward to end of tag
}

void YumXmlMetaParser::readFormatTag()
{
  Q_ASSERT ( isStartElement() && name() == "format" );

  while ( !atEnd() && !abort ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      // qDebug("format %s", qPrintable(name().toString()));
      if ( name() == "provides" ) {
        readProvidesAndRequires ( true );

      } else

        if ( name() == "requires" ) {
          readProvidesAndRequires ( false );

        } else {
          readUnknownElement();
        }
    }
  }
}

void YumXmlMetaParser::readChecksumTag()
{
  Q_ASSERT ( isStartElement() && name() == "checksum" );
  
  bool unknownChecksumAlgorithm;
  QString checksumType = attributes().value ( "type" ).toString();
  currentPackageData.setChecksumAlgorithm ( CheckSumCheck::extractAlgorithmFromTag( checksumType,
                                                                                    &unknownChecksumAlgorithm) );

  if ( unknownChecksumAlgorithm ) {
    qWarning ( "Checksum Type not supported only SHA is supported." );

  } else {
    currentPackageData.setShaCheckSum ( readElementText() );
  }
}

void YumXmlMetaParser::readProvidesAndRequires ( const bool isProvidesElement )
{
  Q_ASSERT ( isStartElement() );

  if ( isProvidesElement )
    Q_ASSERT ( name() == "provides" );
  else
    Q_ASSERT ( name() == "requires" );

  while ( !atEnd() && !abort ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() ) {
      // qDebug("format %s", qPrintable(name().toString()));
      // qDebug("format %s", qPrintable(attributes().value("name").toString()));
      if ( name() == "entry" ) {
        if ( isProvidesElement )
          currentPackageData.addProvide ( attributes().value ( "name" ).toString() );
        else
          currentPackageData.addRequire ( attributes().value ( "name" ).toString() );

        readNext(); // forward to end of tag

      } else {
        readUnknownElement();
      }
    }
  }
}

void YumXmlMetaParser::readUnknownElement()
{
  Q_ASSERT ( isStartElement() );

  if ( !isStartElement() )
    return;

  while ( !atEnd() && !abort ) {
    readNext();

    if ( isEndElement() )
      break;

    if ( isStartElement() )
      readUnknownElement();
  }
}

void YumXmlMetaParser::abortUpdate()
{
  abort = true;

  dbHandler.clear();
  dbHandler.finish();
}


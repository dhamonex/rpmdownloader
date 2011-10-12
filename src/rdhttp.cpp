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

#include "rdhttp.h"


RDHttp::RDHttp ( QObject* parent )
    : QHttp ( parent ), m_destFile ( 0 ), m_dlWithRedirect ( false ), m_redirected ( false ), m_closing ( false )
{
  connect ( this, SIGNAL ( done ( bool ) ), this, SLOT ( downloadFinished ( bool ) ) );
  connect ( this, SIGNAL ( responseHeaderReceived ( const QHttpResponseHeader & ) ), this, SLOT ( processResponseHeader ( const QHttpResponseHeader & ) ) );
}

void RDHttp::downloadFinished ( bool error )
{

  if ( !m_dlWithRedirect ) // do nothing when not downloading with redirect signal is received somewhere outside
    return;

  if ( error ) {
    emit ( downloadWithRedirectToFileFinished ( true ) );
    return;
  }

  if ( m_closing || QHttp::error() == QHttp::Aborted ) {
    // last issued command was close/abort so ignore this one
    m_closing = false;
    return;
  }

  if ( m_redirected ) {
    m_destFile->close();
    m_destFile->remove();
    m_destFile->open ( QIODevice::WriteOnly );

    setHost ( m_redirectedUrl.host(), m_redirectedUrl.port ( 80 ) );

    QHttpRequestHeader header ( "GET", m_redirectedUrl.path() );
    header.setValue ( "Host", m_redirectedUrl.host() );

    m_redirected = false;

    qDebug ( "following redirect to %s", qPrintable ( m_redirectedUrl.toString() ) );
    downloadWithRedirectToFile ( header, m_destFile );

  } else {
    m_dlWithRedirect = false;
    m_destFile = 0;
    m_redirected = false;

    emit ( downloadWithRedirectToFileFinished ( false ) );
  }
}

void RDHttp::processResponseHeader ( const QHttpResponseHeader& rHeader )
{
  qDebug ( "HTTP Response Status Code %i", rHeader.statusCode() );

  if ( rHeader.statusCode() >= 300 && rHeader.statusCode() < 400 ) {
    QString headerString = rHeader.toString();
    QRegExp locationRegExp ( "Location:\\s+(\\S+)" );

    foreach ( const QString &line, headerString.split ( "\n" ) ) {
      if ( locationRegExp.indexIn ( line ) != -1 ) {
        // found location string

        m_redirectedUrl.setUrl ( locationRegExp.cap ( 1 ) );
        m_redirected = true;

        // finished now
        return;
      }
    }
  }
}

void RDHttp::downloadWithRedirectToFile ( const QHttpRequestHeader& header, QFile* destination )
{
  m_dlWithRedirect = true;

  // remember file object
  m_destFile = destination;
  m_closing = false;

  request ( header, 0, destination );
}

void RDHttp::closeConnection()
{
  m_closing = true;
  close();
}


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
#include "plainrepositorycontentdownloader.h"

#include "packagemetadata.h"
#include "rpmdownloadersettings.h"

#include <QApplication>

size_t PlainRepositoryContentDownloader::plainContentCallback( char *ptr, size_t size, size_t nmemb, void *userdata )
{
  PlainRepositoryContentDownloader *plainContentUpdate = reinterpret_cast<PlainRepositoryContentDownloader *>( userdata );
  
  if ( plainContentUpdate->aborted ) {
    return CURLE_WRITE_ERROR;
  }
  size_t realsize = size * nmemb;
  
  plainContentUpdate->m_contents.append( ptr, realsize );
  
  return realsize;
}


PlainRepositoryContentDownloader::PlainRepositoryContentDownloader ( QObject *parent )
    : AbstractContentDownloader ( parent ), dbHandler(), updatedArchs(), isActive( false ), m_contents()
{
    connect( m_curl, SIGNAL( finished() ) , this, SLOT( downloadFinished() ) );
}


PlainRepositoryContentDownloader::~PlainRepositoryContentDownloader()
{
}

void PlainRepositoryContentDownloader::startContentUpdate ( int profileNumber, const QString &databasePath, const QString &repoName, const QString &serverUrl, const QStringList &architectures )
{
  // qDebug("start content update for %i url: %s", profileNumber, qPrintable(serverUrl));
  // store variables
  repoUrl = serverUrl;
  archs = architectures;
  curProfile = profileNumber;
  this->repoName = repoName;
  dbPath = databasePath;

  aborted = false; // reset aborted var
  updatedArchs.clear();

  if ( !initDb() ) {
    emit ( finished ( curProfile, true ) );
    return;
  }

  isActive = true;

  updateNextArch();
}

void PlainRepositoryContentDownloader::abortContentUpdate( const bool userCancelled )
{
  aborted = true;

  if ( isActive ) {
    dbHandler.rollback();
  }

  isActive = false;

  if ( !userCancelled ) {
    emit ( finished ( curProfile, true ) );
  }
}

void PlainRepositoryContentDownloader::cancelContentUpdate()
{
  abortContentUpdate ( true ); // this time cancelled by user

  // emit(finished(curProfile, false));
}

void PlainRepositoryContentDownloader::updateNextArch()
{
  bool allUpdated = true;
  m_contents.clear();

  foreach ( const QString &arch , archs ) {
    if ( !updatedArchs.contains ( arch ) ) {
      allUpdated = false;

      // set current arch
      currentArch = arch;

      break; // found one
    }
  }

  updatedArchs << currentArch; // add current arch to updated archs

  if ( allUpdated && !aborted ) {
    isActive = false;
    dbHandler.finish();
    emit ( finished ( curProfile, false ) );

  } else {
    m_currentUrl = repoUrl + "/" + currentArch + "/";

    if ( !m_currentUrl.isValid() ) { // got an error abort all
      abortContentUpdate();
      errMsg = tr ( "Invalid url: %1" ).arg ( m_currentUrl.toString() );
      return;
    }

    if ( m_curl->isRunning() ) {
      qFatal( "There is already a pending request running" );
      abortContentUpdate();
      return;
    }
    
    curl_easy_setopt( m_curl->get(), CURLOPT_FOLLOWLOCATION, 1L );
    curl_easy_setopt( m_curl->get(), CURLOPT_WRITEFUNCTION, &plainContentCallback );
    curl_easy_setopt( m_curl->get(), CURLOPT_WRITEDATA, this );
  
    curl_easy_setopt( m_curl->get(), CURLOPT_URL, m_currentUrl.toString().toAscii().data() );
  
    m_curl->perform();
    
  }
}

void PlainRepositoryContentDownloader::downloadFinished()
{ 
  if ( m_curl->result() != CURLE_OK ) {
    if ( !aborted ) {
      errMsg = tr ( "Error on content plain content update %1 url: %2)" ).arg( curl_easy_strerror( m_curl->result() ) ).arg( m_currentUrl.toString() );
      abortContentUpdate();
    }

    return;
  }
  
  parseContents();
  updateNextArch();
}

void PlainRepositoryContentDownloader::parseContents()
{
//   qDebug("contents %s", qPrintable(m_contents));
  
  QStringList contentLines = QString( m_contents ).split( "\n" );
  qDebug ( "%i", contentLines.size() );

  for ( int i = 0; i < contentLines.size(); ++i ) {
    if ( ( i % 500 ) == 0 ) {
      qApp->processEvents(); // process events
    }
    
    PackageMetaData metaData;
    
    if ( m_currentUrl.scheme() == "http" ) {
      QRegExp htmlLinkRegExp ( ".*<A HREF=.*>(\\S*)</A>.*(\\d+[.|]\\d*[M|B|K])" );

      htmlLinkRegExp.setCaseSensitivity ( Qt::CaseInsensitive );

      if ( htmlLinkRegExp.indexIn ( contentLines.at ( i ) ) != -1 ) {
        metaData = PackageMetaData( htmlLinkRegExp.cap ( 1 ), currentArch );
        metaData.setSize( htmlLinkRegExp.cap ( 2 ) );
      }
      
    } else {
      enum { SizePos = 4, FileNamePos = 8 };
      QStringList lineParts( contentLines.at( i ).split( " ", QString::SplitBehavior::SkipEmptyParts ) );
      
      if ( lineParts.size() < 8 ) {
        continue;
      }
      
      metaData = PackageMetaData( lineParts.at( FileNamePos ), currentArch, lineParts.at( SizePos ).toUInt() );
    }

    if ( !dbHandler.insertPackage ( metaData, false ) ) {
      abortContentUpdate();
      return;
    }

  }
}

bool PlainRepositoryContentDownloader::initDb()
{
  if ( !dbHandler.init ( dbPath, repoName ) ) {
    errMsg = tr ( "Could not open Database: %1" ).arg ( dbHandler.errorMsg() );
    abortContentUpdate();
    return false;
  }

  dbHandler.clear();

  return true;
}

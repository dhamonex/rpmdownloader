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
#include "checksumcheck.h"

#include <QtCore/QFile>

CheckSumCheck::CheckSumCheck ( QObject *parent )
    : QObject ( parent ), cSumCommand ( "/usr/bin/shasum" ), cAlgorithm("1")
{
  process = new QProcess ( this );

  connect ( process, SIGNAL ( error ( QProcess::ProcessError ) ), this, SLOT ( processError ( QProcess::ProcessError ) ) );
  connect ( process, SIGNAL ( finished ( int, QProcess::ExitStatus ) ), this, SLOT ( processFinished ( int, QProcess::ExitStatus ) ) );
}


CheckSumCheck::~CheckSumCheck()
{
}

void CheckSumCheck::setCheckSumCommand ( const QString & command )
{
  cSumCommand = command;
}

void CheckSumCheck::setChecksumAlgorithm(const QString& algorithm)
{
  if ( algorithm.isEmpty() ) {
    cAlgorithm = "1";
  } else {
    cAlgorithm = algorithm;
  }
}

bool CheckSumCheck::checkSumCheckForFile ( const QString & file, const QString & sum )
{
  if ( !QFile::exists ( file ) ) {
    // file does not exist
    emit ( checkFinished ( false ) );
    return true;
  }

  if ( process->state() != QProcess::NotRunning )
    return false;

  // set arguments
  QStringList arguments;

  arguments << "-a" << cAlgorithm << file;

  checkSum = sum;

  process->start ( cSumCommand, arguments );

  return true;
}

void CheckSumCheck::processError ( QProcess::ProcessError error )
{
  // produce some error texts
  switch ( error ) {
    case QProcess::FailedToStart:
      emit ( checkFailed ( tr ( "Process failed to start %1. Check that the program is installed and executable." ).arg ( cSumCommand ) ) );
      break;

    case QProcess::Crashed:
      emit ( checkFailed ( tr ( "Program %1 crashed during execution." ).arg ( cSumCommand ) ) );
      break;

    case QProcess::Timedout:
      emit ( checkFailed ( tr ( "Command %1 runs too long." ).arg ( cSumCommand ) ) );
      break;

    default:
      emit ( checkFailed ( tr ( "Unknown error while executing %1." ).arg ( cSumCommand ) ) );
  }
}

void CheckSumCheck::processFinished ( int exitCode, QProcess::ExitStatus status )
{
  if ( status != QProcess::NormalExit ) {
    // do nothing error signal should be already sent
    return;
  }

  if ( exitCode != 0 ) {
    qDebug("%s, %s", qPrintable(QString (process->readAllStandardOutput())), qPrintable(QString(process->readAllStandardError())));
    emit ( checkFailed ( tr ( "Unknown error during checksum calculation." ) ) );
  }

  QString output ( process->readAllStandardOutput() );

  QStringList splittedOutput = output.split ( QRegExp ( "\\s+" ) );

  if ( splittedOutput.size() < 1 ) {
    emit ( checkFinished ( false ) );
  }

  if ( splittedOutput.at ( 0 ) == checkSum )
    emit ( checkFinished ( true ) );
  else
    emit ( checkFinished ( false ) );
}

void CheckSumCheck::abort()
{
  process->kill();
}

QString CheckSumCheck::extractAlgorithmFromTag(const QString& tagname, bool * error)
{
  QRegExp shaRegExp("sha(\\w*)");
  if ( shaRegExp.indexIn( tagname ) < 0) {
    if (error) {
      *error = true;
    }
    return QString();
  }
  
  if (error) {
    *error = false;
  }
  
  return shaRegExp.cap(1);
}



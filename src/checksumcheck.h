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
#ifndef CHECKSUMCHECK_H
#define CHECKSUMCHECK_H

#include <QObject>
#include <QProcess>

class QProcess;

class CheckSumCheck : public QObject
{
    Q_OBJECT
    Q_PROPERTY ( QString checkSumCommand READ checkSumCommand WRITE setCheckSumCommand )
  public:
    CheckSumCheck ( QObject *parent = 0 );

    // checks the check sum for the given filename against the specified checksum
    // emits true if both matches
    bool checkSumCheckForFile ( const QString &file, const QString &sum );

    void setCheckSumCommand ( const QString &command );
    QString checkSumCommand() const {return cSumCommand;}
    
    void setChecksumAlgorithm( const QString &algorithm );
    QString checksumAlgorithm() const {return cAlgorithm;}

    void abort();

    ~CheckSumCheck();
  
  static QString extractAlgorithmFromTag(const QString &tagname, bool * error);

  signals:
    void checkFinished ( bool ); // signal is emitted when check finished
    void checkFailed ( QString ); // sends an error message when it failed

  private slots:
    void processError ( QProcess::ProcessError error );
    void processFinished ( int exitCode, QProcess::ExitStatus status );

  private:
    QString cSumCommand; // running an external command for the checksum check
    QString fileName; // filename which is currentl checked
    QString checkSum; // checksum to test against
    QString cAlgorithm;

    QProcess *process;

};

#endif

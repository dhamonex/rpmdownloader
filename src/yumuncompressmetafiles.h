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
#ifndef YUMUNCOMPRESSMETAFILES_H
#define YUMUNCOMPRESSMETAFILES_H

#include <QObject>
#include <QFile>
#include <QProcess>

class YumUncompressMetaFiles : public QObject
{
		Q_OBJECT
	public:
		YumUncompressMetaFiles(QObject *parent = 0);
		
		QString readError()const {return errMsg;}
		bool uncompressMetaFiles(const QString &path);
		
		void setGunzipCommand(const QString &command);
		
		bool isActive() const {return active;}

		~YumUncompressMetaFiles();
	
	signals:
		void finishedUncompress(bool);
	
	private slots:
		void processError(QProcess::ProcessError error);
		void processFinished(int exitCode, QProcess::ExitStatus status);

	private:
		void deleteAllTemporaryFiles();
		
		QString errMsg;
		QProcess *process;
		
		QString gunzipCommand;
		// QFile fileListFile;
		// QFile otherFile;
		QFile primaryFile;
		QString path;
		bool active;
};

#endif

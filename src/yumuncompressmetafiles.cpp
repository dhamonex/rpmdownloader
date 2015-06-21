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
#include "yumuncompressmetafiles.h"
#include "yumfileconstants.h"

#include <QtCore/QProcess>

YumUncompressMetaFiles::YumUncompressMetaFiles(QObject *parent)
		: QObject(parent), gunzipCommand("/usr/bin/gunzip"), active(false)
{
	process = new QProcess(this);
	
	connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(processError(QProcess::ProcessError)));
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));
}


YumUncompressMetaFiles::~YumUncompressMetaFiles()
{
	deleteAllTemporaryFiles();
}

bool YumUncompressMetaFiles::uncompressMetaFiles(const QString & path)
{
	this->path = path;
	
	if (process->state() != QProcess::NotRunning)
		return false;
	
	active = true;
	
	// fileListFile.setFileName(path + "/" + Yum::filelistGzFileName);
	// otherFile.setFileName(path + "/" + Yum::otherGzFileName);
	primaryFile.setFileName(path + "/" + Yum::primaryGzFileName);
	
	if (!primaryFile.exists()) {
		errMsg = tr("some needed files do not exist in %1").arg(path);
		return false;
	}
	
	// QFile::remove(path + "/" + Yum::tmpFilelistGzFileName);
	// QFile::remove(path + "/" + Yum::tmpOtherGzFileName);
	QFile::remove(path + "/" + Yum::tmpPrimaryGzFileName);
	
	// QFile::remove(path + "/" + Yum::tmpFilelistFileName);
	// QFile::remove(path + "/" + Yum::tmpOtherFileName);
	QFile::remove(path + "/" + Yum::tmpPrimaryFileName);
	
	bool success = primaryFile.copy(path + "/" + Yum::tmpPrimaryGzFileName);
	// fileListFile.copy(path + "/" + Yum::tmpFilelistGzFileName);
	// otherFile.copy(path + "/" + Yum::tmpOtherGzFileName);
	
	if (!success) {
		errMsg = tr("%1 is not writable").arg(path);
		return false;
	}
	
	// close all files
	// fileListFile.close();
	// otherFile.close();
	primaryFile.close();
	
	QStringList arguments;
	arguments <<  Yum::tmpPrimaryGzFileName;
	process->setWorkingDirectory(path);
	process->start(gunzipCommand, arguments);
	
	return true;
}

void YumUncompressMetaFiles::processFinished(int exitCode, QProcess::ExitStatus status)
{
	if (status != QProcess::NormalExit) {
		emit(finishedUncompress(false));
		return;
	}
	
	if (exitCode != 0) {
		emit(finishedUncompress(false));
		errMsg = tr("Process exited with status %1"
			"with following error message: %2")
			.arg(exitCode).arg(QString(process->readAllStandardError()));
		return;
	}
	
	primaryFile.setFileName(path + "/" + Yum::tmpPrimaryFileName);
	
	// remove existing files
	QFile::remove(path + "/" + Yum::primaryFileName);
	
	primaryFile.rename(path + "/" + Yum::primaryFileName);
	
	active = false;
	
	emit(finishedUncompress(true));
}

void YumUncompressMetaFiles::processError(QProcess::ProcessError error)
{
	switch(error) {
		case QProcess::FailedToStart:
			errMsg = tr("Process  %1 failed to start. Check that the program is installed and executable.").arg(gunzipCommand);
			break;
		
		case QProcess::Crashed:
			errMsg = tr("Program %1 crashed during execution").arg(gunzipCommand);
			break;
		
		default:
			errMsg = tr("Unknown error while execting %1").arg(gunzipCommand);
	}
}

void YumUncompressMetaFiles::deleteAllTemporaryFiles()
{
	QFile::remove(path + "/" + Yum::tmpPrimaryGzFileName);
	QFile::remove(path + "/" + Yum::tmpPrimaryFileName);
}

void YumUncompressMetaFiles::setGunzipCommand(const QString & command)
{
	gunzipCommand = command;
}

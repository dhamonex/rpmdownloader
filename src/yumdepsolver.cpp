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
#include "yumdepsolver.h"

#include "rpm.h"

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QApplication>
#include <QProgressDialog>

YumDepSolver::YumDepSolver(QObject *parent)
		: RDDatabaseHandler(parent), useMemDb(false)
{
}


YumDepSolver::~YumDepSolver()
{
}


bool YumDepSolver::resolveSatFor(const QString & packageName, const QString & databasePath, const QString & connName, QProgressDialog &dialog, RPM::Architectures archs, bool useMemDb)
{
	setConnectionName(connName);
	setDatabasePath(databasePath);
	if (!checkDatabase())
		return false;
	
	archsToConsider = PackageMetaData::archStringList(archs);
	
	packagesToCheck.clear();
	requiredRpms.clear();
	packageProvides.clear();
	packageFileNames.clear();
	packageRequires.clear();
	
	packagesToCheck << packageName;
	
	this->useMemDb = useMemDb;
	
	if (useMemDb)
		buildMemCache();
	
	dialog.setMaximum(MaxRPMToInspect);
	
	int i = 0;
	while (!packagesToCheck.isEmpty()) {
		qApp->processEvents();
		if (i >= MaxRPMToInspect) {
			return false;
		}
		// qDebug("rpms to check %i", rpmsToCheck.size());
		if (dialog.wasCanceled()) {
			return false;
		}
		
		checkNextRpm();
		dialog.setValue(++i);
	}
	
	return true;
}

void YumDepSolver::checkNextRpm()
{
	QString packageName = packagesToCheck.takeFirst();
	if (!providedByRepo(packageName)) {
		qWarning("Dep solver: nothing in this repo provides %s, maybe it's available for a different architecture?", qPrintable(packageName));
		return;
	}
	
	requiredRpms << packageName;
	
	QStringList neededRpms = getRpmsForRequires(getRequires(packageName));
	
	QStringListIterator i(neededRpms);
	while(i.hasNext()) {
		QString requiredRpm(i.next());
		if (!requiredRpms.contains(requiredRpm))
			packagesToCheck << requiredRpm;
	}
}

QStringList YumDepSolver::getRequires(const QString &rpmName)
{
	QStringList requirements;
	
	if (useMemDb) {
		requirements << packageRequires.values(rpmName);
	} else {
		QSqlDatabase db = getDatabase();
		QSqlQuery q(db);
		
		q.exec("select requires from requires where packageName=\"" + rpmName +"\"");
		
		while (q.next()) {
			if (!requirements.contains(q.value(0).toString()))
				requirements << q.value(0).toString();
		}
	}
	
	return requirements;
}

QStringList YumDepSolver::getRpmsForRequires(const QStringList & requires)
{
	QStringList neededRpms;
	
	if (useMemDb) {
		QStringListIterator i(requires);
		while(i.hasNext()) {
			QString req(i.next());
			QMultiHash<QString, QString>::iterator i = packageProvides.find(req);
			while (i != packageProvides.end() && i.key() == req) {
				if (!neededRpms.contains(i.value()))
					neededRpms << i.value();
				i++;
			}
		}
	} else {
		QSqlDatabase db = getDatabase();
		QSqlQuery q(db);
		
		QStringListIterator i(requires);
		while(i.hasNext()) {
			QString req(i.next());
			
			q.exec("select providedBy from provides where requirement=\"" + req +"\"");
		
			while (q.next()) {
				if (!neededRpms.contains(q.value(0).toString()))
					neededRpms << q.value(0).toString();
			}
		}
	}
	
	return neededRpms;
}

bool YumDepSolver::providedByRepo(const QString & rpmName)
{
	QRegExp archRegExp(".*\\.(i586|x86_64|i686|noarch)\\.rpm");
	if (useMemDb) {
		if (packageFileNames.contains(rpmName)) {
			QList<QString> fileNames = packageFileNames.values(rpmName);
			foreach (QString fileName, fileNames) {
				if (archRegExp.indexIn(fileName) != -1) {
					if (archsToConsider.contains(archRegExp.cap(1))) {
						return true;
					}
				}
			}
		}
	} else {
		QSqlDatabase db = getDatabase();
		QSqlQuery q(db);
		
		q.exec("select architecture from package_meta_data where packageName=\"" + rpmName + "\"");
		while (q.next()) {
			if (archsToConsider.contains(q.value(0).toString()))
				return true;
		}
	}
	
	return false;
}

void YumDepSolver::buildMemCache()
{
	QSqlDatabase db = getDatabase();
	QSqlQuery q(db);
	
	packageRequires.clear();
	packageProvides.clear();
	packageFileNames.clear();
	
	q.exec("select requirement, providedBy from provides");
	while (q.next()) {
		packageProvides.insert(q.value(0).toString(), q.value(1).toString());
	}
	
	q.exec("select packageName, requires from requires");
	while (q.next()) {
		packageRequires.insert(q.value(0).toString(), q.value(1).toString());
	}
	
	q.exec("select packageName, fileName from package_meta_data");
	while (q.next()) {
		packageFileNames.insert(q.value(0).toString(), q.value(1).toString());
	}
}



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
#ifndef YUMDEPSOLVER_H
#define YUMDEPSOLVER_H

#include "rddatabasehandler.h"
#include "rdnamespace.h"

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QMultiHash>

class QProgressDialog;

class YumDepSolver : public RDDatabaseHandler
{
	public:
		YumDepSolver(QObject *parent = 0);
		
		bool resolveSatFor(const QString &packageName, const QString &databasePath, const QString &connName, QProgressDialog &dialog, RPM::Architectures archs, bool useMemDb = true); // the dialog is for aborting and user feedback
		QStringList getRpms() const {return requiredRpms;}

		~YumDepSolver();
	
	private:
		QStringList getRequires(const QString &rpmName);
		QStringList getRpmsForRequires(const QStringList &requires);
		bool providedByRepo(const QString &rpmName);
		void buildMemCache(); // read the tables provides and requires into the memory
		
		void checkNextRpm();
		
		enum {MaxRPMToInspect = 3000}; // do net check more rpms let it fail instead
		
		QStringList requiredRpms;
		QStringList packagesToCheck;
		QStringList archsToConsider;
		
		bool useMemDb;
		QMultiHash<QString, QString> packageRequires;
		QMultiHash<QString, QString> packageProvides;
		QMultiHash<QString, QString> packageFileNames;

};

#endif

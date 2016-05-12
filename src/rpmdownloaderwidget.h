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
#ifndef RPMDOWNLOADERWIDGET_H
#define RPMDOWNLOADERWIDGET_H

#include <QtWidgets/QWidget>
#include "repositoryprofile.h"
#include "rpmdownloadersettings.h"
#include "downloadprogressdialog.h"

class QTableWidget;
class QTableWidgetItem;
class PlainRepositoryContentDownloader;
class YumRepositoryContentDownloader;
class QProgressDialog;

class RpmDownloaderWidget : public QWidget
{

        Q_OBJECT

    public:
        RpmDownloaderWidget( QWidget *parent = 0 );

        bool loadProfilesFile( const QString &fileName );
        void addProfile( RepositoryProfile *newProfile );
        void profileForCurrentProfileChanged();
        void addPackageToCurProfile( const Package &package );
        void addPackageToCurProfileWithDeps( const Package &package );
        void changePackageName( const QString &oldName, const QString &newName, const int rpmsTableRow );
        void changePackageName( const QString &oldName, const QString &newName );  // uses current row

        bool currentRepoSupportsDepSolving() const;

        void addActionToProfilesTable( QAction *action );
        QAction *addSeparatorToProfilesTable();

        void addActionToPackagesTable( QAction *action );
        QAction *addSeparatorToPackagesTable();

        void applyNewSettings(); // applies recently changes settings to the widget (settings is static object)

        RepositoryProfile *getCurrentRepositoryProfile(); // return current selected profile
        QString getCurrentPackageName() const; // get current selected rpm name

        void clearAll();
        void clearPackagesTable();
        void clearProfilesTable();

        bool saveToFile( const QString &fileName );

        void triggerComputeStats();

        QStringList getRepositoryContentsForCurrentRepo() const;

        ~RpmDownloaderWidget();

    signals:
        void gotProfiles( bool );
        void modified();
        void repositoryProfileDoubleClicked( RepositoryProfile * );
        void packageDoubleClicked( QString packageName, int rpmsTableRow );
        void downloadFinished();
        void isBusy( bool );
        void hasPackageSelection( bool, bool );  // second arguement means is bool when more than one item is selected
        void hasProfileSelection( bool );
        void hasPackages( bool );
        void currentProfileHasPackages( bool );
        void profileStats( int numberOfProfiles, int numberOfPackages, int okPackages, int failedPackages, int updatedPackages, int localAvailPackages, int availPackages, int unknownPackages, const QString &downloadSize );
        void packageDetailsReady( const Package&, const QIcon & );  // emited when meta data for details view are prepared

    public slots:
        void deleteCurrentItemsFromActiveTable();
        void deleteSelectedPackagesFromDisk();
        void deleteAllPackagesFromDisk();
        void clearAndInsertPackagesTable( int row, int column, int oldRow, int oldColumn );  // after profile change
        void downloadPackagesForCurrentProfile();
        void downloadPackagesForAllProfiles();
        void completeStatusUpdate( bool showProgress = false );
        void cancelStatusUpdate();
        void duplicateCurrentProfile();
        void copySelectedPackages(); // creates a one space seperated list and copys that to the clipboard
        void cutSelectedPackages();
        void insertPackagesFromClipboard();
        void clearCacheDirectory();
        void resolveDependenciesForCurrentSelectedPackage();
        void applyPackageFilter( const QString &filterText );
        void prepareDetailsView(); // collects the needed informations to display a detaild overview of the current package and emits the signal that the details are ready
        void cleanOrphanedPackageEntries();

    private slots:
        void profileDoubleClicked( int row, int column );
        void packageDoubleClicked( int row, int column );
        void restartUpdateTimer();
        void rpmsTableSelectionChanged();
        void profilesTableWidgetSelectionChanged();
        void repositoryStatusUpdated();
        void contentDownloaderFinished( int profileNumber, bool error );

    private:
        void profileDebugOutput();
        void updatePackagesTable(); // updates status of rpms in the table
        void updateProfileTableStatus(); // updates status of all profiles
        void updateNextProfile(); // starts the update for the next profile
        QIcon iconForStatus( Status status );
        void downloadPackages( bool all = true );
        int computeNumberOfDownloadRpms( bool all = true );
        void computeAndEmitStats();
        bool confirmProfileDelete( const QString &profileName );  // confirm the deletion of the given profile

        void addPackageToPackagesTableWidget( const QString &packageName );  // helper function for adding an element
        void addItemToPackagesTableWidget( QTableWidgetItem *item, const int row, const int coloumn );  // helper function for setting the flags for this item before inserting

        QString formatPackageVersionsForDisplay( const PackageVersions &versions, const bool multipleLines = false ) const;

        void checkForPackagesInAllProfiles(); // checks for rpms in all profiles and emits signals

        enum { MagicNumber = 0x7F51C886, MagicNumberOld = 0x7F51C885,
               PackageColumns = 4, ProfileColumns = 2, MaxRows = 999
         };

        QTableWidget *m_profilesTableWidget;
        QTableWidget *m_packagesTableWidget;
        bool m_loading; // load profiles from file in progress

        QList<RepositoryProfile*> m_profiles;

        int m_currentUpdatedProfile;
        QTimer *m_updateTimer;

        QProgressDialog *m_statusUpdateProgressDialog;

        PlainRepositoryContentDownloader *m_plainContentDownloader;
        YumRepositoryContentDownloader *m_yumContentDownloader;

        // for filter need last filter
        QString m_lastFilterString;
};

#endif


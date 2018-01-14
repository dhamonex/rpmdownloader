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
#include "mainwindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QMessageBox>
#include <QToolBar>
#include <QCloseEvent>
#include <QFileDialog>

#include <QtCore/QSettings>

#include <curl/curl.h>

#include "rpmdownloaderwidget.h"
#include "profilesettingsdialog.h"
#include "rpm.h"
#include "rpmnamedialog.h"
#include "rpmdownloadersettingsdialog.h"
#include "searchfieldlineedit.h"
#include "packagedetailsdialog.h"
#include "version.h"


MainWindow::MainWindow()
    : QMainWindow(), 
      m_addRpmsState ( false ), 
      m_rpmActionsState ( false ), 
      m_profileActionsState ( true ), 
      m_profileDownloadActionState ( false ), 
      m_globalRpmsActionsState ( false )
{
  curl_global_init( CURL_GLOBAL_ALL );
  
  rpmDownloaderWidget = new RpmDownloaderWidget;
  setCentralWidget ( rpmDownloaderWidget );

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  createContextMenu();

  readSettings();
  resize ( QSize ( 860, 480 ) );
  setWindowIcon ( QIcon ( ":/images/rpm.png" ) );

  connect ( rpmDownloaderWidget, SIGNAL ( gotProfiles ( bool ) ), this, SLOT ( enableAddRpms ( bool ) ) );
  connect ( rpmDownloaderWidget, SIGNAL ( repositoryProfileDoubleClicked ( RepositoryProfile * ) ), this, SLOT ( editProfile ( RepositoryProfile * ) ) );
  connect ( rpmDownloaderWidget, SIGNAL ( packageDoubleClicked ( QString, int ) ), this, SLOT ( editRpmName ( QString, int ) ) );
  connect ( rpmDownloaderWidget, SIGNAL ( modified() ), this, SLOT ( tablesModified() ) );
  connect ( rpmDownloaderWidget, SIGNAL ( downloadFinished() ), this, SLOT ( downloadFinished() ) );
  connect ( rpmDownloaderWidget, SIGNAL ( isBusy ( bool ) ), this, SLOT ( disableEditActions ( bool ) ) );
  connect ( rpmDownloaderWidget, SIGNAL ( hasPackageSelection ( bool, bool ) ), this, SLOT ( enableRpmActions ( bool, bool ) ) );
  connect ( rpmDownloaderWidget, SIGNAL ( hasProfileSelection ( bool ) ), this, SLOT ( enableProfileActions ( bool ) ) );
  connect ( rpmDownloaderWidget, SIGNAL ( hasPackages ( bool ) ), this, SLOT ( enableGlobalRpmsActions ( bool ) ) );
  connect ( rpmDownloaderWidget, SIGNAL ( currentProfileHasPackages ( bool ) ), this, SLOT ( enableProfileDownloadAction ( bool ) ) );
  connect ( rpmDownloaderWidget, SIGNAL ( packageDetailsReady ( Package, QIcon ) ), this, SLOT ( showDetailsForPackage ( Package, QIcon ) ) );

  setCurrentFile ( "" );
  rpmDownloaderWidget->triggerComputeStats();
}

void MainWindow::createActions()
{
  m_newAction = new QAction ( tr ( "&New" ), this );
  m_newAction->setIcon ( QIcon ( ":/images/new.png" ) );
  m_newAction->setShortcut ( tr ( "Ctrl+N" ) );
  m_newAction->setToolTip ( tr ( "Create a new profiles file" ) );
  connect ( m_newAction, SIGNAL ( triggered() ), this, SLOT ( newFile() ) );

  m_openAction = new QAction ( tr ( "&Open..." ), this );
  m_openAction->setIcon ( QIcon ( ":/images/open.png" ) );
  m_openAction->setShortcut ( tr ( "Ctrl+O" ) );
  m_openAction->setToolTip ( tr ( "Open an existing profiles file" ) );
  connect ( m_openAction, SIGNAL ( triggered() ), this, SLOT ( open() ) );

  m_saveAction = new QAction ( tr ( "&Save" ), this );
  m_saveAction->setIcon ( QIcon ( ":/images/save.png" ) );
  m_saveAction->setShortcut ( tr ( "Ctrl+S" ) );
  m_saveAction->setToolTip ( tr ( "Save the profiles file to disk" ) );
  connect ( m_saveAction, SIGNAL ( triggered() ), this, SLOT ( save() ) );

  m_saveAsAction = new QAction ( tr ( "Save &As..." ), this );
  m_saveAsAction->setToolTip ( tr ( "Save the profiles file under a new name" ) );
  connect ( m_saveAsAction, SIGNAL ( triggered() ), this, SLOT ( saveAs() ) );

  for ( int i = 0; i < MaxRecentFiles; ++i ) {
    m_recentFileActions[i] = new QAction ( this );
    m_recentFileActions[i]->setVisible ( false );
    connect ( m_recentFileActions[i], SIGNAL ( triggered() ),
              this, SLOT ( openRecentFile() ) );
  }

  m_exitAction = new QAction ( tr ( "E&xit" ), this );

  m_exitAction->setShortcut ( tr ( "Ctrl+Q" ) );
  m_exitAction->setToolTip ( tr ( "Exit the application" ) );
  connect ( m_exitAction, SIGNAL ( triggered() ), this, SLOT ( close() ) );

  m_addNewProfileAction = new QAction ( tr ( "New &Profile" ), this );
  m_addNewProfileAction->setIcon ( QIcon ( ":/images/add.png" ) );
  m_addNewProfileAction->setShortcut ( tr ( "Ctrl+P" ) );
  m_addNewProfileAction->setToolTip ( tr ( "Adds a new Profile" ) );
  connect ( m_addNewProfileAction, SIGNAL ( triggered() ), this, SLOT ( addNewProfile() ) );

  m_deleteAction = new QAction ( tr ( "&Delete current item" ), this );
  m_deleteAction->setIcon ( QIcon ( ":/images/remove.png" ) );
  m_deleteAction->setShortcut ( tr ( "Del" ) );
  m_deleteAction->setToolTip ( tr ( "Deletes the current selected item in the highlited table" ) );
  connect ( m_deleteAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( deleteCurrentItemsFromActiveTable() ) );

  m_addNewRpmToCurProfileAction = new QAction ( tr ( "Add &RPM" ), this );
  m_addNewRpmToCurProfileAction->setIcon ( QIcon ( ":/images/addRpm.png" ) );
  m_addNewRpmToCurProfileAction->setShortcut ( tr ( "Ctrl+R" ) );
  m_addNewRpmToCurProfileAction->setToolTip ( tr ( "Adds a RPM to the current profile" ) );
  connect ( m_addNewRpmToCurProfileAction, SIGNAL ( triggered() ), this, SLOT ( addNewRpm() ) );
  m_addNewRpmToCurProfileAction->setDisabled ( true );

  m_duplicateProfileAction = new QAction ( tr ( "D&uplicate Profile" ), this );
  m_duplicateProfileAction->setIcon ( QIcon ( ":/images/duplicate.png" ) );
  m_duplicateProfileAction->setShortcut ( tr ( "Ctrl+D" ) );
  m_duplicateProfileAction->setToolTip ( tr ( "Duplicates the current selected profile" ) );
  connect ( m_duplicateProfileAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( duplicateCurrentProfile() ) );
  m_duplicateProfileAction->setDisabled ( true );

  m_deleteSelectedRpmsFromDiskAction = new QAction ( tr ( "&Delete local selceted RPMs" ), this );
  m_deleteSelectedRpmsFromDiskAction->setIcon ( QIcon ( ":/images/removeRpmFromDisk.png" ) );
  m_deleteSelectedRpmsFromDiskAction->setToolTip ( tr ( "Deletes the local version of the selected RPMs" ) );
  connect ( m_deleteSelectedRpmsFromDiskAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( deleteSelectedPackagesFromDisk() ) );
  m_deleteSelectedRpmsFromDiskAction->setDisabled ( true );

  m_deleteAllRpmsFromDiskAction = new QAction ( tr ( "Delete all &local RPMs" ), this );
  m_deleteAllRpmsFromDiskAction->setIcon ( QIcon ( ":/images/removeAllRpmsFromDisk.png" ) );
  m_deleteAllRpmsFromDiskAction->setToolTip ( tr ( "Deletes the local version of all RPMs in all profiles" ) );
  connect ( m_deleteAllRpmsFromDiskAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( deleteAllPackagesFromDisk() ) );
  m_deleteAllRpmsFromDiskAction->setDisabled ( true );

  m_copyAction = new QAction ( tr ( "C&opy selected RPMs" ), this );
  m_copyAction->setIcon ( QIcon ( ":/images/copy.png" ) );
  m_copyAction->setShortcut ( tr ( "Ctrl+C" ) );
  m_copyAction->setToolTip ( tr ( "Copy the selected RPMs to Clipboard" ) );
  connect ( m_copyAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( copySelectedPackages() ) );
  m_copyAction->setDisabled ( true );

  m_cutAction = new QAction ( tr ( "C&ut selected RPMs" ), this );
  m_cutAction->setIcon ( QIcon ( ":/images/cut.png" ) );
  m_cutAction->setShortcut ( tr ( "Ctrl+X" ) );
  m_cutAction->setToolTip ( tr ( "Removes and copies the selected RPMs to clipboard" ) );
  connect ( m_cutAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( cutSelectedPackages() ) );
  m_cutAction->setDisabled ( true );

  m_pasteAction = new QAction ( tr ( "&Paste RPMS" ), this );
  m_pasteAction->setIcon ( QIcon ( ":/images/paste.png" ) );
  m_pasteAction->setShortcut ( tr ( "Ctrl+V" ) );
  m_pasteAction->setToolTip ( tr ( "Paste RPMs from clipboard" ) );
  connect ( m_pasteAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( insertPackagesFromClipboard() ) );
  m_pasteAction->setDisabled ( true );

  m_editProfileAction = new QAction ( tr ( "Edit pro&file" ), this );
  m_editProfileAction->setIcon ( QIcon ( ":/images/editProfile.png" ) );
  m_editProfileAction->setToolTip ( tr ( "Edit the properties of the current selected profile" ) );
  connect ( m_editProfileAction, SIGNAL ( triggered() ), this, SLOT ( editProfile() ) );
  m_editProfileAction->setDisabled ( true );

  m_editRpmNameAction = new QAction ( tr ( "Edit RPM &name" ), this );
  m_editRpmNameAction->setIcon ( QIcon ( ":/images/editRpmName.png" ) );
  m_editRpmNameAction->setToolTip ( tr ( "Edit the name of the current selcted RPM" ) );
  connect ( m_editRpmNameAction, SIGNAL ( triggered() ), this, SLOT ( editRpmName() ) );
  m_editRpmNameAction->setDisabled ( true );

  m_cleanOrphanedEntriesAction = new QAction ( tr ( "Clean up &orphaned RPM entries" ), this );
  m_cleanOrphanedEntriesAction->setToolTip ( tr ( "Removes orpharned RPMs from disk and from package selection" ) );
  connect ( m_cleanOrphanedEntriesAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( cleanOrphanedPackageEntries() ) );

  m_clearCacheDirectoryAction = new QAction ( tr ( "&Clear cache directory" ), this );
  m_clearCacheDirectoryAction->setIcon ( QIcon ( ":/images/clearCache.png" ) );
  m_clearCacheDirectoryAction->setToolTip ( tr ( "Clears the whole cache directory which enforces a complete refresh of all repositories" ) );
  connect ( m_clearCacheDirectoryAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( clearCacheDirectory() ) );

  m_resolveDepsForSelectedRpmAction = new QAction ( tr ( "Resol&ve dependencies" ), this );
  m_resolveDepsForSelectedRpmAction->setIcon ( QIcon ( ":/images/resolveDeps.png" ) );
  m_resolveDepsForSelectedRpmAction->setToolTip ( tr ( "Resolve the dependencies for the current selcted RPM" ) );
  connect ( m_resolveDepsForSelectedRpmAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( resolveDependenciesForCurrentSelectedPackage() ) );
  m_resolveDepsForSelectedRpmAction->setDisabled ( true );

  m_showDetailsForSelectedRpmAction = new QAction ( tr ( "Show Package details" ), this );
  m_showDetailsForSelectedRpmAction->setToolTip ( tr ( "Shows a detailed overview of the current selected package." ) );
  connect ( m_showDetailsForSelectedRpmAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( prepareDetailsView() ) );
  m_showDetailsForSelectedRpmAction->setDisabled ( true );

  m_downloadCurrentProfile = new QAction ( tr ( "Download cur&rent profile" ), this );
  m_downloadCurrentProfile->setIcon ( QIcon ( ":/images/downloadOne.png" ) );
  m_downloadCurrentProfile->setToolTip ( tr ( "Downloads all RPMs from the current profile" ) );
  connect ( m_downloadCurrentProfile, SIGNAL ( triggered() ), this, SLOT ( startDownloadOne() ) );
  m_downloadCurrentProfile->setDisabled ( true );

  m_downloadAllProfiles = new QAction ( tr ( "Download &all profiles" ), this );
  m_downloadAllProfiles->setIcon ( QIcon ( ":/images/downloadAll.png" ) );
  m_downloadAllProfiles->setToolTip ( tr ( "Downloads all rpms from all profiles" ) );
  connect ( m_downloadAllProfiles, SIGNAL ( triggered() ), this, SLOT ( startDownloadAll() ) );
  m_downloadAllProfiles->setDisabled ( true );

  m_refreshStatus = new QAction ( tr ( "Refre&sh profile status" ), this );
  m_refreshStatus->setIcon ( QIcon ( ":/images/refresh.png" ) );
  m_refreshStatus->setToolTip ( tr ( "Refresh repositories and update profiles status" ) );
  connect ( m_refreshStatus, SIGNAL ( triggered() ), this, SLOT ( refreshProfileStatus() ) );

  m_changeSettingsAction = new QAction ( tr ( "Change &settings" ), this );
  m_changeSettingsAction->setIcon ( QIcon ( ":/images/changeSettings.png" ) );
  m_changeSettingsAction->setToolTip ( tr ( "Changes the global settings of RPM Downloader" ) );
  connect ( m_changeSettingsAction, SIGNAL ( triggered() ), this, SLOT ( changeSettingsViaDialog() ) );

  m_showStatusBarAction = new QAction ( tr ( "Display stats status bar" ), this );
  m_showStatusBarAction->setToolTip ( tr ( "Switch stats status bar on or off" ) );
  m_showStatusBarAction->setCheckable ( true );
  m_showStatusBarAction->setChecked ( false );
  connect ( m_showStatusBarAction, SIGNAL ( toggled ( bool ) ), this, SLOT ( showStatusBar ( bool ) ) );

  m_toolButtonStyleActionGroup = new QActionGroup ( this );
  m_toolButtonStyleIconOnlyAction = new QAction ( tr ( "Display icons only" ), m_toolButtonStyleActionGroup );
  m_toolButtonStyleIconOnlyAction->setCheckable ( true );
  m_toolButtonStyleIconOnlyAction->setData ( Qt::ToolButtonIconOnly );

  m_toolButtonStyleTextOnlyAction = new QAction ( tr ( "Display text only" ), m_toolButtonStyleActionGroup );
  m_toolButtonStyleTextOnlyAction->setCheckable ( true );
  m_toolButtonStyleTextOnlyAction->setData ( Qt::ToolButtonTextOnly );

  m_toolButtonStyleTextAndIconAction = new QAction ( tr ( "Display text and icons" ), m_toolButtonStyleActionGroup );
  m_toolButtonStyleTextAndIconAction->setCheckable ( true );
  m_toolButtonStyleTextAndIconAction->setData ( Qt::ToolButtonTextUnderIcon );

  m_toolButtonStyleIconOnlyAction->setChecked ( true );
  connect ( m_toolButtonStyleActionGroup, SIGNAL ( triggered ( QAction* ) ), this, SLOT ( changeToolBarStyle ( QAction* ) ) );

  m_increaseToolIconSize = new QAction ( tr ( "Increase button size" ), this );
  m_increaseToolIconSize->setToolTip ( tr ( "Increases the button size in the main tool bar" ) );
  m_increaseToolIconSize->setIcon ( QIcon ( ":images/increaseButtonSize.png" ) );
  connect ( m_increaseToolIconSize, SIGNAL ( triggered() ), this, SLOT ( increaseToolBarIconSize() ) );

  m_decreaseToolIconSize = new QAction ( tr ( "Decrease button size" ), this );
  m_decreaseToolIconSize->setToolTip ( tr ( "Decreases the button size in the main tool bar" ) );
  m_decreaseToolIconSize->setIcon ( QIcon ( ":images/decreaseButtonSize.png" ) );
  connect ( m_decreaseToolIconSize, SIGNAL ( triggered() ), this, SLOT ( decreaseToolBarIconSize() ) );

  m_showLegendAction = new QAction ( tr ( "&Legend" ), this );
  m_showLegendAction->setToolTip ( tr ( "Show the application's symbol legend" ) );
  connect ( m_showLegendAction, SIGNAL ( triggered() ), this, SLOT ( showLegend() ) );

  m_aboutAction = new QAction ( tr ( "&About" ), this );
  m_aboutAction->setToolTip ( tr ( "Show the application's About box" ) );
  connect ( m_aboutAction, SIGNAL ( triggered() ), this, SLOT ( about() ) );

  m_aboutQtAction = new QAction ( tr ( "About &Qt" ), this );
  m_aboutQtAction->setToolTip ( tr ( "Show the Qt library's About box" ) );
  connect ( m_aboutQtAction, SIGNAL ( triggered() ), qApp, SLOT ( aboutQt() ) );
}

void MainWindow::createMenus()
{
  m_fileMenu = menuBar()->addMenu ( tr ( "&File" ) );
  m_fileMenu->addAction ( m_newAction );
  m_fileMenu->addAction ( m_openAction );
  m_fileMenu->addAction ( m_saveAction );
  m_fileMenu->addAction ( m_saveAsAction );

  m_separatorAction = m_fileMenu->addSeparator();

  for ( int i = 0; i < MaxRecentFiles; ++i )
    m_fileMenu->addAction ( m_recentFileActions[i] );

  m_fileMenu->addSeparator();

  m_fileMenu->addAction ( m_exitAction );

  m_editMenu = menuBar()->addMenu ( tr ( "&Edit" ) );

  m_editMenu->addAction ( m_addNewProfileAction );

  m_editMenu->addAction ( m_addNewRpmToCurProfileAction );

  m_editMenu->addAction ( m_duplicateProfileAction );

  m_editMenu->addAction ( m_editProfileAction );

  m_editMenu->addAction ( m_editRpmNameAction );

  m_editMenu->addAction ( m_resolveDepsForSelectedRpmAction );

  m_editMenu->addAction ( m_cleanOrphanedEntriesAction );

  m_editMenu->addAction ( m_deleteAction );

  m_editMenu->addAction ( m_clearCacheDirectoryAction );

  m_editMenu->addSeparator();

  m_editMenu->addAction ( m_copyAction );

  m_editMenu->addAction ( m_cutAction );

  m_editMenu->addAction ( m_pasteAction );

  m_downloadMenu = menuBar()->addMenu ( tr ( "&Download" ) );

  m_downloadMenu->addAction ( m_downloadCurrentProfile );

  m_downloadMenu->addAction ( m_downloadAllProfiles );

  m_downloadMenu->addAction ( m_refreshStatus );

  m_downloadMenu->addSeparator();

  m_downloadMenu->addAction ( m_deleteSelectedRpmsFromDiskAction );

  m_downloadMenu->addAction ( m_deleteAllRpmsFromDiskAction );

  m_settingsMenu = menuBar()->addMenu ( tr ( "&Settings" ) );

  m_settingsMenu->addAction ( m_changeSettingsAction );

  m_settingsMenu->addAction ( m_showStatusBarAction );

  // create submenu
  m_toolButtonStyleMenu = m_settingsMenu->addMenu ( tr ( "&Change Icon Style" ) );

  m_toolButtonStyleMenu->addActions ( m_toolButtonStyleActionGroup->actions() );

  m_toolButtonStyleMenu->addSeparator();

  m_toolButtonStyleMenu->addAction ( m_increaseToolIconSize );

  m_toolButtonStyleMenu->addAction ( m_decreaseToolIconSize );

  m_helpMenu = menuBar()->addMenu ( tr ( "&Help" ) );

  m_helpMenu->addAction ( m_showLegendAction );

  m_helpMenu->addAction ( m_aboutAction );

  m_helpMenu->addAction ( m_aboutQtAction );

}

void MainWindow::createToolBars()
{
  m_fileToolBar = addToolBar ( tr ( "&File" ) );
  m_fileToolBar->setObjectName ( "FileToolBar" );
  m_fileToolBar->addAction ( m_newAction );
  m_fileToolBar->addAction ( m_openAction );
  m_fileToolBar->addAction ( m_saveAction );

  m_editToolBar = addToolBar ( tr ( "&Edit" ) );
  m_editToolBar->setObjectName ( "EditToolBar" );
  m_editToolBar->addAction ( m_addNewProfileAction );
  m_editToolBar->addAction ( m_addNewRpmToCurProfileAction );
  m_editToolBar->addAction ( m_duplicateProfileAction );
  m_editToolBar->addAction ( m_editProfileAction );
  m_editToolBar->addAction ( m_editRpmNameAction );
  m_editToolBar->addAction ( m_resolveDepsForSelectedRpmAction );
  m_editToolBar->addAction ( m_deleteAction );

  m_downloadToolBar = addToolBar ( tr ( "&Download" ) );
  m_downloadToolBar->setObjectName ( "DownloadToolbar" );
  m_downloadToolBar->addAction ( m_downloadCurrentProfile );
  m_downloadToolBar->addAction ( m_downloadAllProfiles );
  m_downloadToolBar->addAction ( m_refreshStatus );
  m_downloadToolBar->addAction ( m_deleteSelectedRpmsFromDiskAction );
  m_downloadToolBar->addAction ( m_deleteAllRpmsFromDiskAction );

  m_searchToolBar = addToolBar ( tr ( "&Search Filter" ) );
  m_searchToolBar->setObjectName ( "SearchToolBar" );
  m_searchFieldLineEdit = new SearchFieldLineEdit;
  m_searchFieldLineEdit->setDisplayedOnEmptyText ( tr ( "Filter for packages" ) );
  m_searchFieldLineEdit->setClearButtonIcon ( QPixmap ( ":/images/edit-clear.png" ) );
  m_searchFieldLineEdit->setTextSubmitTimeout ( 1000 );

  connect ( m_searchFieldLineEdit, SIGNAL ( newSearchText ( const QString & ) ), rpmDownloaderWidget, SLOT ( applyPackageFilter ( const QString & ) ) );

  m_searchLabel = new QLabel ( tr ( "Filter Packages:" ) );
  m_searchLabelAction = m_searchToolBar->addWidget ( m_searchLabel );
  m_searchLineEditAction = m_searchToolBar->addWidget ( m_searchFieldLineEdit );
}

void MainWindow::createStatusBar()
{
  m_numberOfProfilesLabel = new QLabel ( tr ( "Profiles: %1 " ).arg ( 9999 ) );
  m_numberOfPackagesLabel = new QLabel ( tr ( "RPMs: %1 " ).arg ( 9999 ) );
  m_numberOfOkPackagesLabel = new QLabel ( tr ( "Ok: %1 " ).arg ( 9999 ) );
  m_numberOfFailedRpmsLabel = new QLabel ( tr ( "Failed: %1 " ).arg ( 9999 ) );
  m_numberOfAvailPackagesLabel = new QLabel ( tr ( "Avail: %1 " ).arg ( 9999 ) );
  m_numberOfLocalAvailPackagesLabel = new QLabel ( tr ( "Local Avail: %1 " ).arg ( 9999 ) );
  m_numberOfUpdatedPackagesLabel = new QLabel ( tr ( "Updated: %1 " ).arg ( 9999 ) );
  m_numberOfUnknownPackagesLabel = new QLabel ( tr ( "Unknown: %1 " ).arg ( 9999 ) );
  m_downloadSizeLabel = new QLabel ( tr ( "Download Size: %1 M" ).arg ( 9999.99 ) );

  // set a minmal size
  m_numberOfProfilesLabel->setMinimumSize ( m_numberOfProfilesLabel->sizeHint() );
  m_numberOfPackagesLabel->setMinimumSize ( m_numberOfPackagesLabel->sizeHint() );
  m_numberOfOkPackagesLabel->setMinimumSize ( m_numberOfOkPackagesLabel->sizeHint() );
  m_numberOfFailedRpmsLabel->setMinimumSize ( m_numberOfFailedRpmsLabel->sizeHint() );
  m_numberOfAvailPackagesLabel->setMinimumSize ( m_numberOfAvailPackagesLabel->sizeHint() );
  m_numberOfLocalAvailPackagesLabel->setMinimumSize ( m_numberOfLocalAvailPackagesLabel->sizeHint() );
  m_numberOfUpdatedPackagesLabel->setMinimumSize ( m_numberOfUpdatedPackagesLabel->sizeHint() );
  m_numberOfUnknownPackagesLabel->setMinimumSize ( m_numberOfUnknownPackagesLabel->sizeHint() );
  m_downloadSizeLabel->setMinimumSize ( m_downloadSizeLabel->sizeHint() );

  // add to status bar
  statusBar()->addWidget ( m_numberOfProfilesLabel, 1 );
  statusBar()->addWidget ( m_numberOfPackagesLabel, 1 );
  statusBar()->addWidget ( m_numberOfOkPackagesLabel, 1 );
  statusBar()->addWidget ( m_numberOfAvailPackagesLabel, 1 );
  statusBar()->addWidget ( m_numberOfLocalAvailPackagesLabel, 1 );
  statusBar()->addWidget ( m_numberOfUpdatedPackagesLabel, 1 );
  statusBar()->addWidget ( m_numberOfUnknownPackagesLabel, 1 );
  statusBar()->addWidget ( m_numberOfFailedRpmsLabel, 1 );
  statusBar()->addWidget ( m_downloadSizeLabel, 1 );

  connect ( rpmDownloaderWidget, SIGNAL ( profileStats ( int, int, int, int, int, int, int, int, const QString& ) ), this, SLOT ( updateStatusBar ( int, int, int, int, int, int, int, int, const QString& ) ) );
  statusBar()->hide();
}

void MainWindow::createContextMenu()
{
  rpmDownloaderWidget->addActionToProfilesTable ( m_addNewProfileAction );
  rpmDownloaderWidget->addActionToProfilesTable ( m_addNewRpmToCurProfileAction );
  rpmDownloaderWidget->addActionToProfilesTable ( m_downloadCurrentProfile );
  rpmDownloaderWidget->addActionToProfilesTable ( m_duplicateProfileAction );
  rpmDownloaderWidget->addActionToProfilesTable ( m_editProfileAction );
  rpmDownloaderWidget->addSeparatorToProfilesTable();
  rpmDownloaderWidget->addActionToProfilesTable ( m_deleteAction );

  rpmDownloaderWidget->addActionToPackagesTable ( m_addNewRpmToCurProfileAction );
  rpmDownloaderWidget->addActionToPackagesTable ( m_deleteSelectedRpmsFromDiskAction );
  rpmDownloaderWidget->addActionToPackagesTable ( m_editRpmNameAction );
  rpmDownloaderWidget->addActionToPackagesTable ( m_resolveDepsForSelectedRpmAction );
  rpmDownloaderWidget->addSeparatorToPackagesTable();
  rpmDownloaderWidget->addActionToPackagesTable ( m_deleteAction );
  rpmDownloaderWidget->addSeparatorToPackagesTable();
  rpmDownloaderWidget->addActionToPackagesTable ( m_showDetailsForSelectedRpmAction );
}

void MainWindow::closeEvent ( QCloseEvent * event )
{
  if ( okToContinue() ) {
    writeSettings();
    event->accept();

  } else {
    event->ignore();
  }
}

void MainWindow::updateRecentFileActions()
{
  QMutableStringListIterator i ( m_recentFiles );

  while ( i.hasNext() ) {
    if ( !QFile::exists ( i.next() ) )
      i.remove();
  }

  for ( int j = 0; j < MaxRecentFiles; ++j ) {
    if ( j < m_recentFiles.count() ) {
      QString text = tr ( "&%1 %2" )
                     .arg ( j + 1 )
                     .arg ( strippedName ( m_recentFiles[j] ) );
      m_recentFileActions[j]->setText ( text );
      m_recentFileActions[j]->setData ( m_recentFiles[j] );
      m_recentFileActions[j]->setVisible ( true );

    } else {
      m_recentFileActions[j]->setVisible ( false );
    }
  }

  m_separatorAction->setVisible ( !m_recentFiles.isEmpty() );
}

void MainWindow::readSettings()
{
  QSettings settings ( "Monex Software", "RpmDownloader" );
  QRect rect = settings.value ( "geometry", QRect ( 200, 200, 720, 480 ) ).toRect();

  move ( rect.topLeft() );
  resize ( rect.size() );

  m_recentFiles = settings.value ( "recentFiles" ).toStringList();
  updateRecentFileActions();

  rpmDownloaderSettings().setDeleteOldVersions ( settings.value ( "deleteOldVersions", true ).toBool() );
  rpmDownloaderSettings().setUpdateInterval ( settings.value ( "updateInterval", 30000 ).toInt() );

  rpmDownloaderSettings().setCacheDir ( QDir ( settings.value ( "cacheDir", rpmDownloaderSettings().cacheDir().absolutePath() ).toString() ) );
  rpmDownloaderSettings().setTempDir ( QDir ( settings.value ( "tempDir", rpmDownloaderSettings().tempDir().absolutePath() ).toString() ) );

  rpmDownloaderSettings().setChecksumCommand ( settings.value ( "checksumCommand", rpmDownloaderSettings().checksumCommand() ).toString() );

  rpmDownloaderSettings().setGunzipCommand ( settings.value ( "gunzipCommand", rpmDownloaderSettings().gunzipCommand() ).toString() );
  rpmDownloaderSettings().setMaximumPrimaryFileSizeForMemoryLoad ( settings.value ( "maxPrimaryFileSizeForMemoryLoad", rpmDownloaderSettings().maximumPrimaryFileSizeForMemoryLoad() ).toInt() );

  m_showStatusBarAction->setChecked ( settings.value ( "showStatusBar", false ).toBool() );

  rpmDownloaderSettings().setMemDbSatSolving ( settings.value ( "useMemDabSatSolving", false ).toBool() );

  rpmDownloaderSettings().setDoCheckSumCheckOnDownloadedPackages ( settings.value ( "doCheckSumCheck", true ).toBool() );

  rpmDownloaderWidget->applyNewSettings();

  Qt::ToolButtonStyle actToolButtonStyle = static_cast <Qt::ToolButtonStyle> ( settings.value ( "toolButtonStyle", Qt::ToolButtonIconOnly ).toInt() );

  switch ( actToolButtonStyle ) {
    case Qt::ToolButtonTextUnderIcon:
      setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
      m_toolButtonStyleTextAndIconAction->setChecked ( true );
      break;
    case Qt::ToolButtonTextOnly:
      setToolButtonStyle ( Qt::ToolButtonTextOnly );
      m_toolButtonStyleTextOnlyAction->setChecked ( true );
      break;
    default:
      setToolButtonStyle ( Qt::ToolButtonIconOnly );
      m_toolButtonStyleIconOnlyAction->setChecked ( true );
      break;
  }

  setIconSize ( settings.value ( "toolButtonSize", iconSize() ).toSize() );

  restoreState ( settings.value ( "toolBarState" ).toByteArray() );
}

void MainWindow::writeSettings()
{
  QSettings settings ( "Monex Software", "RpmDownloader" );
  settings.setValue ( "geometry", geometry() );
  settings.setValue ( "recentFiles", m_recentFiles );
  settings.setValue ( "deleteOldVersions", rpmDownloaderSettings().deleteOldVersions() );
  settings.setValue ( "updateInterval", rpmDownloaderSettings().updateInterval() );
  settings.setValue ( "cacheDir", rpmDownloaderSettings().cacheDir().absolutePath() );
  settings.setValue ( "tempDir", rpmDownloaderSettings().tempDir().absolutePath() );
  settings.setValue ( "checksumCommand", rpmDownloaderSettings().checksumCommand() );
  settings.setValue ( "gunzipCommand", rpmDownloaderSettings().gunzipCommand() );
  settings.setValue ( "showStatusBar", m_showStatusBarAction->isChecked() );
  settings.setValue ( "useMemDabSatSolving", rpmDownloaderSettings().useMemDbSatSolving() );
  settings.setValue ( "doCheckSumCheck", rpmDownloaderSettings().doCheckSumCheckOnDownloadedPackages() );
  settings.setValue ( "maxPrimaryFileSizeForMemoryLoad", rpmDownloaderSettings().maximumPrimaryFileSizeForMemoryLoad() );

  settings.setValue ( "toolButtonStyle", toolButtonStyle() );
  settings.setValue ( "toolButtonSize", iconSize() );

  settings.setValue ( "toolBarState", saveState() );
}

bool MainWindow::okToContinue()
{
  if ( isWindowModified() ) {
    int r = QMessageBox::warning ( this, tr ( "RPM Downloader" ),
                                   tr ( "The profiles has been modified.\nDo you want to save the changes?" ),
                                   QMessageBox::Yes | QMessageBox::Default,
                                   QMessageBox::No,
                                   QMessageBox::Cancel | QMessageBox::Escape );

    if ( r == QMessageBox::Yes ) {
      return ( save() );

    } else if ( r == QMessageBox::Cancel ) {
      return false;
    }
  }

  return true;
}

void MainWindow::addNewProfile()
{
  RepositoryProfile * profile = new RepositoryProfile ( rpmDownloaderWidget );
  ProfileSettingsDialog dialog ( profile, this );

  if ( dialog.exec() ) {
    rpmDownloaderWidget->addProfile ( profile );
  }
}

void MainWindow::addNewRpm()
{
  Package package;
  RpmNameDialog dialog ( package.packageName(), rpmDownloaderWidget->getRepositoryContentsForCurrentRepo(),
                         rpmDownloaderWidget->currentRepoSupportsDepSolving(), true, this );

  if ( dialog.exec() ) {
    package.setPackageName ( dialog.getName() );

    if ( rpmDownloaderWidget->currentRepoSupportsDepSolving() && dialog.resolveDeps() )
      rpmDownloaderWidget->addPackageToCurProfileWithDeps ( package );
    else
      rpmDownloaderWidget->addPackageToCurProfile ( package );
  }
}

void MainWindow::editRpmName ( QString actName, int rpmTableRow )
{
  RpmNameDialog dialog ( actName, rpmDownloaderWidget->getRepositoryContentsForCurrentRepo(),
                         false, false, this );

  if ( dialog.exec() ) {
    if ( rpmTableRow >= 0 )
      rpmDownloaderWidget->changePackageName ( actName, dialog.getName(), rpmTableRow );
    else
      rpmDownloaderWidget->changePackageName ( actName, dialog.getName() );
  }
}

void MainWindow::editRpmName()
{
  editRpmName ( rpmDownloaderWidget->getCurrentPackageName(), -1 );
}

void MainWindow::enableAddRpms ( bool enable )
{
  m_addNewRpmToCurProfileAction->setEnabled ( enable );
  m_pasteAction->setEnabled ( enable );
  m_addRpmsState = enable;
}

void MainWindow::enableRpmActions ( bool enable, bool disableSingleItemActions )
{
  m_deleteSelectedRpmsFromDiskAction->setEnabled ( enable );

  if ( enable ) {
    m_editRpmNameAction->setDisabled ( disableSingleItemActions );
    m_resolveDepsForSelectedRpmAction->setDisabled ( disableSingleItemActions );
    m_showDetailsForSelectedRpmAction->setDisabled ( disableSingleItemActions );

  } else {
    m_editRpmNameAction->setEnabled ( false );
    m_resolveDepsForSelectedRpmAction->setEnabled ( false );
    m_showDetailsForSelectedRpmAction->setEnabled ( false );
  }

  m_cutAction->setEnabled ( enable );

  m_copyAction->setEnabled ( enable );

  m_rpmActionsState = enable;
}

void MainWindow::enableProfileActions ( bool enable )
{
  m_duplicateProfileAction->setEnabled ( enable );
  m_editProfileAction->setEnabled ( enable );

  m_profileActionsState = enable;
}

void MainWindow::enableGlobalRpmsActions ( bool enable )
{
  m_deleteAllRpmsFromDiskAction->setEnabled ( enable );
  m_downloadAllProfiles->setEnabled ( enable );

  m_globalRpmsActionsState = enable;
}

void MainWindow::enableProfileDownloadAction ( bool enable )
{
  m_downloadCurrentProfile->setEnabled ( enable );

  m_profileDownloadActionState = enable;
}

void MainWindow::editProfile ( RepositoryProfile *profile )
{
  ProfileSettingsDialog dialog ( profile, this );
  // chages are directly done on the pointed profile

  if ( dialog.exec() ) {
    setWindowModified ( true );
    rpmDownloaderWidget->profileForCurrentProfileChanged();
  }
}

void MainWindow::editProfile()
{
  editProfile ( rpmDownloaderWidget->getCurrentRepositoryProfile() );
}

void MainWindow::tablesModified()
{
  setWindowModified ( true );
}

void MainWindow::setCurrentFile ( const QString & fileName )
{
  m_curFile = fileName;
  setWindowModified ( false );

  QString shownName = tr ( "Untitled" );

  if ( !m_curFile.isEmpty() ) {
    shownName = strippedName ( m_curFile );
    m_recentFiles.removeAll ( m_curFile );
    m_recentFiles.prepend ( m_curFile );
    updateRecentFileActions();
  }

  setWindowTitle ( tr ( "%1[*] - %2" ).arg ( shownName ).arg ( tr ( "RPM Downloader" ) ) );
}

QString MainWindow::strippedName ( const QString & fullFileName )
{
  return QFileInfo ( fullFileName ).fileName();
}

void MainWindow::openRecentFile()
{
  if ( okToContinue() ) {
    QAction *action = qobject_cast<QAction *> ( sender() );

    if ( action )
      loadFile ( action->data().toString() );
  }
}

bool MainWindow::save()
{
  if ( m_curFile.isEmpty() ) {
    return saveAs();

  } else {
    return saveFile ( m_curFile );
  }
}

bool MainWindow::saveAs()
{
  QString fileName = QFileDialog::getSaveFileName ( this,
                     tr ( "Save RPM Downloader Profiles and RPMS" ), ".",
                     tr ( "RPM Downloader files (*.rdl)" ) );

  if ( fileName.isEmpty() ) {
    return false;
  }

  if ( !fileName.endsWith ( ".rdl" ) ) {
    fileName += ".rdl";
  }

  return saveFile ( fileName );
}

bool MainWindow::saveFile ( const QString & fileName )
{
  if ( !rpmDownloaderWidget->saveToFile ( fileName ) ) {
    statusBar()->showMessage ( tr ( "Saving canceled" ), 2000 );
    return false;
  }

  setCurrentFile ( fileName );

  statusBar()->showMessage ( tr ( "File saved" ), 2000 );
  return true;
}

void MainWindow::open()
{
  if ( okToContinue() ) {
    QString fileName = QFileDialog::getOpenFileName ( this,
                       tr ( "Open RPM Downloader Profiles and RPMS" ), ".",
                       tr ( "RPM Downloader files (*.rdl)" ) );

    if ( !fileName.isEmpty() )
      loadFile ( fileName );
  }
}

bool MainWindow::loadFile ( const QString & fileName )
{
  if ( !rpmDownloaderWidget->loadProfilesFile ( fileName ) ) {
    statusBar()->showMessage ( tr ( "Loading canceled" ), 2000 );
    return false;
  }

  setCurrentFile ( fileName );

  statusBar()->showMessage ( tr ( "File loaded" ), 2000 );
  return true;
}

void MainWindow::newFile()
{
  if ( okToContinue() ) {
    rpmDownloaderWidget->clearAll();
    setCurrentFile ( "" );
  }
}

void MainWindow::startDownloadOne()
{
  enableDownloadButtons ( false );
  rpmDownloaderWidget->downloadPackagesForCurrentProfile();
}

void MainWindow::startDownloadAll()
{
  enableDownloadButtons ( false );
  rpmDownloaderWidget->downloadPackagesForAllProfiles();
}

void MainWindow::enableDownloadButtons ( bool enable )
{
  m_downloadAllProfiles->setEnabled ( enable );
  m_downloadCurrentProfile->setEnabled ( enable );
}

void MainWindow::downloadFinished()
{
  enableDownloadButtons ( true );
}

void MainWindow::refreshProfileStatus()
{
  rpmDownloaderWidget->completeStatusUpdate ( true );
}

void MainWindow::disableEditActions ( bool disable )
{
  m_deleteAction->setDisabled ( disable );
  m_refreshStatus->setDisabled ( disable );
  m_addNewProfileAction->setDisabled ( disable );

  if ( !disable ) {
    enableAddRpms ( m_addRpmsState );
    enableDownloadButtons ( m_addRpmsState );
    m_deleteSelectedRpmsFromDiskAction->setDisabled ( !m_rpmActionsState );
    m_deleteAllRpmsFromDiskAction->setDisabled ( !m_rpmActionsState );
    m_duplicateProfileAction->setDisabled ( !m_profileActionsState );

  } else {
    m_addNewRpmToCurProfileAction->setEnabled ( false );
    m_pasteAction->setEnabled ( false );
    enableDownloadButtons ( false );
    m_deleteSelectedRpmsFromDiskAction->setDisabled ( true );
    m_deleteAllRpmsFromDiskAction->setDisabled ( true );
    m_duplicateProfileAction->setDisabled ( true );
  }

  // editRpmNameAction->setDisabled(disable);
}

void MainWindow::changeSettingsViaDialog()
{
  // static settings object is updated witin the dialog
  RpmDownloaderSettingsDialog dialog ( this );

  if ( dialog.exec() ) {
    rpmDownloaderWidget->applyNewSettings();
  }
}

void MainWindow::about()
{
  QMessageBox::about ( this, tr ( "About rpmdownloader" ),
                       tr ( "<h2>RPMDownloader version. " RPMDOWNLOADER_VERSION "</h2>"
                            "<p>RPMDownloader is for downloading a set of RPMs from"
                            "<br/>different RPM repositories. It also informs you if a newer version"
                            "<br/>is available and allows you to redownload only the changed RPMs"
                            "<br/>It can resolve the dependencies of a single yum repository" ) );
}

void MainWindow::showLegend()
{
  QMessageBox messageBox ( QMessageBox::NoIcon, tr ( "Symbol Legend" ),
                           tr ( "<h2>RpmDownloader Symbol Legend</h2>"
                                "<p><table width='100%'><tr><th>Symbol</th><th>Meaning</th></tr>"
                                "<tr><td align='center'><img src=':/images/ok.png'>"
                                "</td><td valign='middle'>RPM is on local disk<br/>and is up to date</td>"
                                "<tr><td align='center'><img src=':/images/notOk.png'>"
                                "</td><td valign='middle'>RPM is not on local store <br/>neither is available online</td>"
                                "<tr><td align='center'><img src=':/images/avail.png'>"
                                "</td><td valign='middle'>RPM is not on local disk<br/>but is available in repository</td>"
                                "<tr><td align='center'><img src=':/images/localAvail.png'>"
                                "</td><td valign='middle'>RPM is on local disk<br/>but is not available in repository</td>"
                                "<tr><td align='center'><img src=':/images/newVersion.png'>"
                                "</td><td valign='middle'>RPM is on local disk<br/>but version in repository is newer</td>"
                                "<tr><td align='center'><img src=':/images/unknown.png'>"
                                "</td><td valign='middle'>Status unknown because<br/>online status is not updated yet</td>"
                                "</table>" ),
                           QMessageBox::Ok, this );
  messageBox.exec();
}

void MainWindow::updateStatusBar ( int numberOfProfiles, int numberOfRpms, int okRpms, int failedRpms, int updatedRpms, int localAvailRpms, int availRpms, int unknownRpms, const QString &downloadSize )
{
  m_numberOfProfilesLabel->setText ( tr ( "Profiles: %1 " ).arg ( numberOfProfiles ) );
  m_numberOfPackagesLabel->setText ( tr ( "RPMs: %1 " ).arg ( numberOfRpms ) );
  m_numberOfOkPackagesLabel->setText ( tr ( "Ok: %1 " ).arg ( okRpms ) );
  m_numberOfFailedRpmsLabel->setText ( tr ( "Failed: %1 " ).arg ( failedRpms ) );
  m_numberOfAvailPackagesLabel->setText ( tr ( "Avail: %1 " ).arg ( availRpms ) );
  m_numberOfLocalAvailPackagesLabel->setText ( tr ( "Local Avail: %1 " ).arg ( localAvailRpms ) );
  m_numberOfUpdatedPackagesLabel->setText ( tr ( "Updated: %1 " ).arg ( updatedRpms ) );
  m_numberOfUnknownPackagesLabel->setText ( tr ( "Unknown: %1 " ).arg ( unknownRpms ) );
  m_downloadSizeLabel->setText ( tr ( "Download Size: %1" ).arg ( downloadSize ) );
}

void MainWindow::showStatusBar ( bool show )
{
  if ( show )
    statusBar()->show();
  else
    statusBar()->hide();
}

void MainWindow::changeToolBarStyle ( QAction* action )
{
  setToolButtonStyle ( static_cast<Qt::ToolButtonStyle> ( action->data().toInt() ) );
}

void MainWindow::increaseToolBarIconSize()
{
  QSize actSize = iconSize();
  actSize.scale ( actSize.width() + 5, actSize.height() + 5, Qt::KeepAspectRatio );
  setIconSize ( actSize );
}

void MainWindow::decreaseToolBarIconSize()
{
  QSize actSize = iconSize();
  actSize.scale ( actSize.width() - 5, actSize.height() - 5, Qt::KeepAspectRatio );
  setIconSize ( actSize );
}

void MainWindow::showDetailsForPackage ( const Package& packageData, const QIcon& icon )
{
  PackageDetailsDialog dialog ( packageData, icon, this );

  dialog.exec();
}

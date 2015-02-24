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

#include <QtGui>
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
    : QMainWindow(), addRpmsState ( false ), rpmActionsState ( false ), profileActionsState ( true ), profileDownloadActionState ( false ), globalRpmsActionsState ( false )
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
  newAction = new QAction ( tr ( "&New" ), this );
  newAction->setIcon ( QIcon ( ":/images/new.png" ) );
  newAction->setShortcut ( tr ( "Ctrl+N" ) );
  newAction->setToolTip ( tr ( "Create a new profiles file" ) );
  connect ( newAction, SIGNAL ( triggered() ), this, SLOT ( newFile() ) );

  openAction = new QAction ( tr ( "&Open..." ), this );
  openAction->setIcon ( QIcon ( ":/images/open.png" ) );
  openAction->setShortcut ( tr ( "Ctrl+O" ) );
  openAction->setToolTip ( tr ( "Open an existing profiles file" ) );
  connect ( openAction, SIGNAL ( triggered() ), this, SLOT ( open() ) );

  saveAction = new QAction ( tr ( "&Save" ), this );
  saveAction->setIcon ( QIcon ( ":/images/save.png" ) );
  saveAction->setShortcut ( tr ( "Ctrl+S" ) );
  saveAction->setToolTip ( tr ( "Save the profiles file to disk" ) );
  connect ( saveAction, SIGNAL ( triggered() ), this, SLOT ( save() ) );

  saveAsAction = new QAction ( tr ( "Save &As..." ), this );
  saveAsAction->setToolTip ( tr ( "Save the profiles file under a new name" ) );
  connect ( saveAsAction, SIGNAL ( triggered() ), this, SLOT ( saveAs() ) );

  for ( int i = 0; i < MaxRecentFiles; ++i ) {
    recentFileActions[i] = new QAction ( this );
    recentFileActions[i]->setVisible ( false );
    connect ( recentFileActions[i], SIGNAL ( triggered() ),
              this, SLOT ( openRecentFile() ) );
  }

  exitAction = new QAction ( tr ( "E&xit" ), this );

  exitAction->setShortcut ( tr ( "Ctrl+Q" ) );
  exitAction->setToolTip ( tr ( "Exit the application" ) );
  connect ( exitAction, SIGNAL ( triggered() ), this, SLOT ( close() ) );

  addNewProfileAction = new QAction ( tr ( "New &Profile" ), this );
  addNewProfileAction->setIcon ( QIcon ( ":/images/add.png" ) );
  addNewProfileAction->setShortcut ( tr ( "Ctrl+P" ) );
  addNewProfileAction->setToolTip ( tr ( "Adds a new Profile" ) );
  connect ( addNewProfileAction, SIGNAL ( triggered() ), this, SLOT ( addNewProfile() ) );

  deleteAction = new QAction ( tr ( "&Delete current item" ), this );
  deleteAction->setIcon ( QIcon ( ":/images/remove.png" ) );
  deleteAction->setShortcut ( tr ( "Del" ) );
  deleteAction->setToolTip ( tr ( "Deletes the current selected item in the highlited table" ) );
  connect ( deleteAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( deleteCurrentItemsFromActiveTable() ) );

  addNewRpmToCurProfileAction = new QAction ( tr ( "Add &RPM" ), this );
  addNewRpmToCurProfileAction->setIcon ( QIcon ( ":/images/addRpm.png" ) );
  addNewRpmToCurProfileAction->setShortcut ( tr ( "Ctrl+R" ) );
  addNewRpmToCurProfileAction->setToolTip ( tr ( "Adds a RPM to the current profile" ) );
  connect ( addNewRpmToCurProfileAction, SIGNAL ( triggered() ), this, SLOT ( addNewRpm() ) );
  addNewRpmToCurProfileAction->setDisabled ( true );

  duplicateProfileAction = new QAction ( tr ( "D&uplicate Profile" ), this );
  duplicateProfileAction->setIcon ( QIcon ( ":/images/duplicate.png" ) );
  duplicateProfileAction->setShortcut ( tr ( "Ctrl+D" ) );
  duplicateProfileAction->setToolTip ( tr ( "Duplicates the current selected profile" ) );
  connect ( duplicateProfileAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( duplicateCurrentProfile() ) );
  duplicateProfileAction->setDisabled ( true );

  deleteSelectedRpmsFromDiskAction = new QAction ( tr ( "&Delete local selceted RPMs" ), this );
  deleteSelectedRpmsFromDiskAction->setIcon ( QIcon ( ":/images/removeRpmFromDisk.png" ) );
  deleteSelectedRpmsFromDiskAction->setToolTip ( tr ( "Deletes the local version of the selected RPMs" ) );
  connect ( deleteSelectedRpmsFromDiskAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( deleteSelectedPackagesFromDisk() ) );
  deleteSelectedRpmsFromDiskAction->setDisabled ( true );

  deleteAllRpmsFromDiskAction = new QAction ( tr ( "Delete all &local RPMs" ), this );
  deleteAllRpmsFromDiskAction->setIcon ( QIcon ( ":/images/removeAllRpmsFromDisk.png" ) );
  deleteAllRpmsFromDiskAction->setToolTip ( tr ( "Deletes the local version of all RPMs in all profiles" ) );
  connect ( deleteAllRpmsFromDiskAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( deleteAllPackagesFromDisk() ) );
  deleteAllRpmsFromDiskAction->setDisabled ( true );

  copyAction = new QAction ( tr ( "C&opy selected RPMs" ), this );
  copyAction->setIcon ( QIcon ( ":/images/copy.png" ) );
  copyAction->setShortcut ( tr ( "Ctrl+C" ) );
  copyAction->setToolTip ( tr ( "Copy the selected RPMs to Clipboard" ) );
  connect ( copyAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( copySelectedPackages() ) );
  copyAction->setDisabled ( true );

  cutAction = new QAction ( tr ( "C&ut selected RPMs" ), this );
  cutAction->setIcon ( QIcon ( ":/images/cut.png" ) );
  cutAction->setShortcut ( tr ( "Ctrl+X" ) );
  cutAction->setToolTip ( tr ( "Removes and copies the selected RPMs to clipboard" ) );
  connect ( cutAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( cutSelectedPackages() ) );
  cutAction->setDisabled ( true );

  pasteAction = new QAction ( tr ( "&Paste RPMS" ), this );
  pasteAction->setIcon ( QIcon ( ":/images/paste.png" ) );
  pasteAction->setShortcut ( tr ( "Ctrl+V" ) );
  pasteAction->setToolTip ( tr ( "Paste RPMs from clipboard" ) );
  connect ( pasteAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( insertPackagesFromClipboard() ) );
  pasteAction->setDisabled ( true );

  editProfileAction = new QAction ( tr ( "Edit pro&file" ), this );
  editProfileAction->setIcon ( QIcon ( ":/images/editProfile.png" ) );
  editProfileAction->setToolTip ( tr ( "Edit the properties of the current selected profile" ) );
  connect ( editProfileAction, SIGNAL ( triggered() ), this, SLOT ( editProfile() ) );
  editProfileAction->setDisabled ( true );

  editRpmNameAction = new QAction ( tr ( "Edit RPM &name" ), this );
  editRpmNameAction->setIcon ( QIcon ( ":/images/editRpmName.png" ) );
  editRpmNameAction->setToolTip ( tr ( "Edit the name of the current selcted RPM" ) );
  connect ( editRpmNameAction, SIGNAL ( triggered() ), this, SLOT ( editRpmName() ) );
  editRpmNameAction->setDisabled ( true );

  cleanOrphanedEntriesAction = new QAction ( tr ( "Clean up &orphaned RPM entries" ), this );
  cleanOrphanedEntriesAction->setToolTip ( tr ( "Removes orpharned RPMs from disk and from package selection" ) );
  connect ( cleanOrphanedEntriesAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( cleanOrphanedPackageEntries() ) );

  clearCacheDirectoryAction = new QAction ( tr ( "&Clear cache directory" ), this );
  clearCacheDirectoryAction->setIcon ( QIcon ( ":/images/clearCache.png" ) );
  clearCacheDirectoryAction->setToolTip ( tr ( "Clears the whole cache directory which enforces a complete refresh of all repositories" ) );
  connect ( clearCacheDirectoryAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( clearCacheDirectory() ) );

  resolveDepsForSelectedRpmAction = new QAction ( tr ( "Resol&ve dependencies" ), this );
  resolveDepsForSelectedRpmAction->setIcon ( QIcon ( ":/images/resolveDeps.png" ) );
  resolveDepsForSelectedRpmAction->setToolTip ( tr ( "Resolve the dependencies for the current selcted RPM" ) );
  connect ( resolveDepsForSelectedRpmAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( resolveDependenciesForCurrentSelectedPackage() ) );
  resolveDepsForSelectedRpmAction->setDisabled ( true );

  showDetailsForSelectedRpmAction = new QAction ( tr ( "Show Package details" ), this );
  showDetailsForSelectedRpmAction->setToolTip ( tr ( "Shows a detailed overview of the current selected package." ) );
  connect ( showDetailsForSelectedRpmAction, SIGNAL ( triggered() ), rpmDownloaderWidget, SLOT ( prepareDetailsView() ) );
  showDetailsForSelectedRpmAction->setDisabled ( true );

  downloadCurrentProfile = new QAction ( tr ( "Download cur&rent profile" ), this );
  downloadCurrentProfile->setIcon ( QIcon ( ":/images/downloadOne.png" ) );
  downloadCurrentProfile->setToolTip ( tr ( "Downloads all RPMs from the current profile" ) );
  connect ( downloadCurrentProfile, SIGNAL ( triggered() ), this, SLOT ( startDownloadOne() ) );
  downloadCurrentProfile->setDisabled ( true );

  downloadAllProfiles = new QAction ( tr ( "Download &all profiles" ), this );
  downloadAllProfiles->setIcon ( QIcon ( ":/images/downloadAll.png" ) );
  downloadAllProfiles->setToolTip ( tr ( "Downloads all rpms from all profiles" ) );
  connect ( downloadAllProfiles, SIGNAL ( triggered() ), this, SLOT ( startDownloadAll() ) );
  downloadAllProfiles->setDisabled ( true );

  refreshStatus = new QAction ( tr ( "Refre&sh profile status" ), this );
  refreshStatus->setIcon ( QIcon ( ":/images/refresh.png" ) );
  refreshStatus->setToolTip ( tr ( "Refresh repositories and update profiles status" ) );
  connect ( refreshStatus, SIGNAL ( triggered() ), this, SLOT ( refreshProfileStatus() ) );

  changeSettingsAction = new QAction ( tr ( "Change &settings" ), this );
  changeSettingsAction->setIcon ( QIcon ( ":/images/changeSettings.png" ) );
  changeSettingsAction->setToolTip ( tr ( "Changes the global settings of RPM Downloader" ) );
  connect ( changeSettingsAction, SIGNAL ( triggered() ), this, SLOT ( changeSettingsViaDialog() ) );

  showStatusBarAction = new QAction ( tr ( "Display stats status bar" ), this );
  showStatusBarAction->setToolTip ( tr ( "Switch stats status bar on or off" ) );
  showStatusBarAction->setCheckable ( true );
  showStatusBarAction->setChecked ( false );
  connect ( showStatusBarAction, SIGNAL ( toggled ( bool ) ), this, SLOT ( showStatusBar ( bool ) ) );

  toolButtonStyleActionGroup = new QActionGroup ( this );
  toolButtonStyleIconOnlyAction = new QAction ( tr ( "Display icons only" ), toolButtonStyleActionGroup );
  toolButtonStyleIconOnlyAction->setCheckable ( true );
  toolButtonStyleIconOnlyAction->setData ( Qt::ToolButtonIconOnly );

  toolButtonStyleTextOnlyAction = new QAction ( tr ( "Display text only" ), toolButtonStyleActionGroup );
  toolButtonStyleTextOnlyAction->setCheckable ( true );
  toolButtonStyleTextOnlyAction->setData ( Qt::ToolButtonTextOnly );

  toolButtonStyleTextAndIconAction = new QAction ( tr ( "Display text and icons" ), toolButtonStyleActionGroup );
  toolButtonStyleTextAndIconAction->setCheckable ( true );
  toolButtonStyleTextAndIconAction->setData ( Qt::ToolButtonTextUnderIcon );

  toolButtonStyleIconOnlyAction->setChecked ( true );
  connect ( toolButtonStyleActionGroup, SIGNAL ( triggered ( QAction* ) ), this, SLOT ( changeToolBarStyle ( QAction* ) ) );

  increaseToolIconSize = new QAction ( tr ( "Increase button size" ), this );
  increaseToolIconSize->setToolTip ( tr ( "Increases the button size in the main tool bar" ) );
  increaseToolIconSize->setIcon ( QIcon ( ":images/increaseButtonSize.png" ) );
  connect ( increaseToolIconSize, SIGNAL ( triggered() ), this, SLOT ( increaseToolBarIconSize() ) );

  decreaseToolIconSize = new QAction ( tr ( "Decrease button size" ), this );
  decreaseToolIconSize->setToolTip ( tr ( "Decreases the button size in the main tool bar" ) );
  decreaseToolIconSize->setIcon ( QIcon ( ":images/decreaseButtonSize.png" ) );
  connect ( decreaseToolIconSize, SIGNAL ( triggered() ), this, SLOT ( decreaseToolBarIconSize() ) );

  showLegendAction = new QAction ( tr ( "&Legend" ), this );
  showLegendAction->setToolTip ( tr ( "Show the application's symbol legend" ) );
  connect ( showLegendAction, SIGNAL ( triggered() ), this, SLOT ( showLegend() ) );

  aboutAction = new QAction ( tr ( "&About" ), this );
  aboutAction->setToolTip ( tr ( "Show the application's About box" ) );
  connect ( aboutAction, SIGNAL ( triggered() ), this, SLOT ( about() ) );

  aboutQtAction = new QAction ( tr ( "About &Qt" ), this );
  aboutQtAction->setToolTip ( tr ( "Show the Qt library's About box" ) );
  connect ( aboutQtAction, SIGNAL ( triggered() ), qApp, SLOT ( aboutQt() ) );
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu ( tr ( "&File" ) );
  fileMenu->addAction ( newAction );
  fileMenu->addAction ( openAction );
  fileMenu->addAction ( saveAction );
  fileMenu->addAction ( saveAsAction );

  separatorAction = fileMenu->addSeparator();

  for ( int i = 0; i < MaxRecentFiles; ++i )
    fileMenu->addAction ( recentFileActions[i] );

  fileMenu->addSeparator();

  fileMenu->addAction ( exitAction );

  editMenu = menuBar()->addMenu ( tr ( "&Edit" ) );

  editMenu->addAction ( addNewProfileAction );

  editMenu->addAction ( addNewRpmToCurProfileAction );

  editMenu->addAction ( duplicateProfileAction );

  editMenu->addAction ( editProfileAction );

  editMenu->addAction ( editRpmNameAction );

  editMenu->addAction ( resolveDepsForSelectedRpmAction );

  editMenu->addAction ( cleanOrphanedEntriesAction );

  editMenu->addAction ( deleteAction );

  editMenu->addAction ( clearCacheDirectoryAction );

  editMenu->addSeparator();

  editMenu->addAction ( copyAction );

  editMenu->addAction ( cutAction );

  editMenu->addAction ( pasteAction );

  downloadMenu = menuBar()->addMenu ( tr ( "&Download" ) );

  downloadMenu->addAction ( downloadCurrentProfile );

  downloadMenu->addAction ( downloadAllProfiles );

  downloadMenu->addAction ( refreshStatus );

  downloadMenu->addSeparator();

  downloadMenu->addAction ( deleteSelectedRpmsFromDiskAction );

  downloadMenu->addAction ( deleteAllRpmsFromDiskAction );

  settingsMenu = menuBar()->addMenu ( tr ( "&Settings" ) );

  settingsMenu->addAction ( changeSettingsAction );

  settingsMenu->addAction ( showStatusBarAction );

  // create submenu
  toolButtonStyleMenu = settingsMenu->addMenu ( tr ( "&Change Icon Style" ) );

  toolButtonStyleMenu->addActions ( toolButtonStyleActionGroup->actions() );

  toolButtonStyleMenu->addSeparator();

  toolButtonStyleMenu->addAction ( increaseToolIconSize );

  toolButtonStyleMenu->addAction ( decreaseToolIconSize );

  helpMenu = menuBar()->addMenu ( tr ( "&Help" ) );

  helpMenu->addAction ( showLegendAction );

  helpMenu->addAction ( aboutAction );

  helpMenu->addAction ( aboutQtAction );

}

void MainWindow::createToolBars()
{
  fileToolBar = addToolBar ( tr ( "&File" ) );
  fileToolBar->setObjectName ( "FileToolBar" );
  fileToolBar->addAction ( newAction );
  fileToolBar->addAction ( openAction );
  fileToolBar->addAction ( saveAction );

  editToolBar = addToolBar ( tr ( "&Edit" ) );
  editToolBar->setObjectName ( "EditToolBar" );
  editToolBar->addAction ( addNewProfileAction );
  editToolBar->addAction ( addNewRpmToCurProfileAction );
  editToolBar->addAction ( duplicateProfileAction );
  editToolBar->addAction ( editProfileAction );
  editToolBar->addAction ( editRpmNameAction );
  editToolBar->addAction ( resolveDepsForSelectedRpmAction );
  editToolBar->addAction ( deleteAction );

  downloadToolBar = addToolBar ( tr ( "&Download" ) );
  downloadToolBar->setObjectName ( "DownloadToolbar" );
  downloadToolBar->addAction ( downloadCurrentProfile );
  downloadToolBar->addAction ( downloadAllProfiles );
  downloadToolBar->addAction ( refreshStatus );
  downloadToolBar->addAction ( deleteSelectedRpmsFromDiskAction );
  downloadToolBar->addAction ( deleteAllRpmsFromDiskAction );

  searchToolBar = addToolBar ( tr ( "&Search Filter" ) );
  searchToolBar->setObjectName ( "SearchToolBar" );
  searchFieldLineEdit = new SearchFieldLineEdit;
  searchFieldLineEdit->setDisplayedOnEmptyText ( tr ( "Filter for packages" ) );
  searchFieldLineEdit->setClearButtonIcon ( QPixmap ( ":/images/edit-clear.png" ) );
  searchFieldLineEdit->setTextSubmitTimeout ( 1000 );

  connect ( searchFieldLineEdit, SIGNAL ( newSearchText ( const QString & ) ), rpmDownloaderWidget, SLOT ( applyPackageFilter ( const QString & ) ) );

  searchLabel = new QLabel ( tr ( "Filter Packages:" ) );
  searchLabelAction = searchToolBar->addWidget ( searchLabel );
  searchLineEditAction = searchToolBar->addWidget ( searchFieldLineEdit );
}

void MainWindow::createStatusBar()
{
  numberOfProfilesLabel = new QLabel ( tr ( "Profiles: %1 " ).arg ( 9999 ) );
  numberOfPackagesLabel = new QLabel ( tr ( "RPMs: %1 " ).arg ( 9999 ) );
  numberOfOkPackagesLabel = new QLabel ( tr ( "Ok: %1 " ).arg ( 9999 ) );
  numberOfFailedRpmsLabel = new QLabel ( tr ( "Failed: %1 " ).arg ( 9999 ) );
  numberOfAvailPackagesLabel = new QLabel ( tr ( "Avail: %1 " ).arg ( 9999 ) );
  numberOfLocalAvailPackagesLabel = new QLabel ( tr ( "Local Avail: %1 " ).arg ( 9999 ) );
  numberOfUpdatedPackagesLabel = new QLabel ( tr ( "Updated: %1 " ).arg ( 9999 ) );
  numberOfUnknownPackagesLabel = new QLabel ( tr ( "Unknown: %1 " ).arg ( 9999 ) );
  downloadSizeLabel = new QLabel ( tr ( "Download Size: %1 M" ).arg ( 9999.99 ) );

  // set a minmal size
  numberOfProfilesLabel->setMinimumSize ( numberOfProfilesLabel->sizeHint() );
  numberOfPackagesLabel->setMinimumSize ( numberOfPackagesLabel->sizeHint() );
  numberOfOkPackagesLabel->setMinimumSize ( numberOfOkPackagesLabel->sizeHint() );
  numberOfFailedRpmsLabel->setMinimumSize ( numberOfFailedRpmsLabel->sizeHint() );
  numberOfAvailPackagesLabel->setMinimumSize ( numberOfAvailPackagesLabel->sizeHint() );
  numberOfLocalAvailPackagesLabel->setMinimumSize ( numberOfLocalAvailPackagesLabel->sizeHint() );
  numberOfUpdatedPackagesLabel->setMinimumSize ( numberOfUpdatedPackagesLabel->sizeHint() );
  numberOfUnknownPackagesLabel->setMinimumSize ( numberOfUnknownPackagesLabel->sizeHint() );
  downloadSizeLabel->setMinimumSize ( downloadSizeLabel->sizeHint() );

  // add to status bar
  statusBar()->addWidget ( numberOfProfilesLabel, 1 );
  statusBar()->addWidget ( numberOfPackagesLabel, 1 );
  statusBar()->addWidget ( numberOfOkPackagesLabel, 1 );
  statusBar()->addWidget ( numberOfAvailPackagesLabel, 1 );
  statusBar()->addWidget ( numberOfLocalAvailPackagesLabel, 1 );
  statusBar()->addWidget ( numberOfUpdatedPackagesLabel, 1 );
  statusBar()->addWidget ( numberOfUnknownPackagesLabel, 1 );
  statusBar()->addWidget ( numberOfFailedRpmsLabel, 1 );
  statusBar()->addWidget ( downloadSizeLabel, 1 );

  connect ( rpmDownloaderWidget, SIGNAL ( profileStats ( int, int, int, int, int, int, int, int, const QString& ) ), this, SLOT ( updateStatusBar ( int, int, int, int, int, int, int, int, const QString& ) ) );
  statusBar()->hide();
}

void MainWindow::createContextMenu()
{
  rpmDownloaderWidget->addActionToProfilesTable ( addNewProfileAction );
  rpmDownloaderWidget->addActionToProfilesTable ( addNewRpmToCurProfileAction );
  rpmDownloaderWidget->addActionToProfilesTable ( downloadCurrentProfile );
  rpmDownloaderWidget->addActionToProfilesTable ( duplicateProfileAction );
  rpmDownloaderWidget->addActionToProfilesTable ( editProfileAction );
  rpmDownloaderWidget->addSeparatorToProfilesTable();
  rpmDownloaderWidget->addActionToProfilesTable ( deleteAction );

  rpmDownloaderWidget->addActionToPackagesTable ( addNewRpmToCurProfileAction );
  rpmDownloaderWidget->addActionToPackagesTable ( deleteSelectedRpmsFromDiskAction );
  rpmDownloaderWidget->addActionToPackagesTable ( editRpmNameAction );
  rpmDownloaderWidget->addActionToPackagesTable ( resolveDepsForSelectedRpmAction );
  rpmDownloaderWidget->addSeparatorToPackagesTable();
  rpmDownloaderWidget->addActionToPackagesTable ( deleteAction );
  rpmDownloaderWidget->addSeparatorToPackagesTable();
  rpmDownloaderWidget->addActionToPackagesTable ( showDetailsForSelectedRpmAction );
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
  QMutableStringListIterator i ( recentFiles );

  while ( i.hasNext() ) {
    if ( !QFile::exists ( i.next() ) )
      i.remove();
  }

  for ( int j = 0; j < MaxRecentFiles; ++j ) {
    if ( j < recentFiles.count() ) {
      QString text = tr ( "&%1 %2" )
                     .arg ( j + 1 )
                     .arg ( strippedName ( recentFiles[j] ) );
      recentFileActions[j]->setText ( text );
      recentFileActions[j]->setData ( recentFiles[j] );
      recentFileActions[j]->setVisible ( true );

    } else {
      recentFileActions[j]->setVisible ( false );
    }
  }

  separatorAction->setVisible ( !recentFiles.isEmpty() );
}

void MainWindow::readSettings()
{
  QSettings settings ( "Monex Software", "RpmDownloader" );
  QRect rect = settings.value ( "geometry", QRect ( 200, 200, 720, 480 ) ).toRect();

  move ( rect.topLeft() );
  resize ( rect.size() );

  recentFiles = settings.value ( "recentFiles" ).toStringList();
  updateRecentFileActions();

  rpmDownloaderSettings().setDeleteOldVersions ( settings.value ( "deleteOldVersions", true ).toBool() );
  rpmDownloaderSettings().setUpdateInterval ( settings.value ( "updateInterval", 30000 ).toInt() );

  rpmDownloaderSettings().setCacheDir ( QDir ( settings.value ( "cacheDir", rpmDownloaderSettings().cacheDir().absolutePath() ).toString() ) );
  rpmDownloaderSettings().setTempDir ( QDir ( settings.value ( "tempDir", rpmDownloaderSettings().tempDir().absolutePath() ).toString() ) );

  rpmDownloaderSettings().setChecksumCommand ( settings.value ( "checksumCommand", rpmDownloaderSettings().checksumCommand() ).toString() );

  rpmDownloaderSettings().setGunzipCommand ( settings.value ( "gunzipCommand", rpmDownloaderSettings().gunzipCommand() ).toString() );
  rpmDownloaderSettings().setMaximumPrimaryFileSizeForMemoryLoad ( settings.value ( "maxPrimaryFileSizeForMemoryLoad", rpmDownloaderSettings().maximumPrimaryFileSizeForMemoryLoad() ).toInt() );

  showStatusBarAction->setChecked ( settings.value ( "showStatusBar", false ).toBool() );

  rpmDownloaderSettings().setMemDbSatSolving ( settings.value ( "useMemDabSatSolving", false ).toBool() );

  rpmDownloaderSettings().setDoCheckSumCheckOnDownloadedPackages ( settings.value ( "doCheckSumCheck", true ).toBool() );

  rpmDownloaderWidget->applyNewSettings();

  Qt::ToolButtonStyle actToolButtonStyle = static_cast <Qt::ToolButtonStyle> ( settings.value ( "toolButtonStyle", Qt::ToolButtonIconOnly ).toInt() );

  switch ( actToolButtonStyle ) {
    case Qt::ToolButtonTextUnderIcon:
      setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
      toolButtonStyleTextAndIconAction->setChecked ( true );
      break;
    case Qt::ToolButtonTextOnly:
      setToolButtonStyle ( Qt::ToolButtonTextOnly );
      toolButtonStyleTextOnlyAction->setChecked ( true );
      break;
    default:
      setToolButtonStyle ( Qt::ToolButtonIconOnly );
      toolButtonStyleIconOnlyAction->setChecked ( true );
      break;
  }

  setIconSize ( settings.value ( "toolButtonSize", iconSize() ).toSize() );

  restoreState ( settings.value ( "toolBarState" ).toByteArray() );
}

void MainWindow::writeSettings()
{
  QSettings settings ( "Monex Software", "RpmDownloader" );
  settings.setValue ( "geometry", geometry() );
  settings.setValue ( "recentFiles", recentFiles );
  settings.setValue ( "deleteOldVersions", rpmDownloaderSettings().deleteOldVersions() );
  settings.setValue ( "updateInterval", rpmDownloaderSettings().updateInterval() );
  settings.setValue ( "cacheDir", rpmDownloaderSettings().cacheDir().absolutePath() );
  settings.setValue ( "tempDir", rpmDownloaderSettings().tempDir().absolutePath() );
  settings.setValue ( "checksumCommand", rpmDownloaderSettings().checksumCommand() );
  settings.setValue ( "gunzipCommand", rpmDownloaderSettings().gunzipCommand() );
  settings.setValue ( "showStatusBar", showStatusBarAction->isChecked() );
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
  addNewRpmToCurProfileAction->setEnabled ( enable );
  pasteAction->setEnabled ( enable );
  addRpmsState = enable;
}

void MainWindow::enableRpmActions ( bool enable, bool disableSingleItemActions )
{
  deleteSelectedRpmsFromDiskAction->setEnabled ( enable );

  if ( enable ) {
    editRpmNameAction->setDisabled ( disableSingleItemActions );
    resolveDepsForSelectedRpmAction->setDisabled ( disableSingleItemActions );
    showDetailsForSelectedRpmAction->setDisabled ( disableSingleItemActions );

  } else {
    editRpmNameAction->setEnabled ( false );
    resolveDepsForSelectedRpmAction->setEnabled ( false );
    showDetailsForSelectedRpmAction->setEnabled ( false );
  }

  cutAction->setEnabled ( enable );

  copyAction->setEnabled ( enable );

  rpmActionsState = enable;
}

void MainWindow::enableProfileActions ( bool enable )
{
  duplicateProfileAction->setEnabled ( enable );
  editProfileAction->setEnabled ( enable );

  profileActionsState = enable;
}

void MainWindow::enableGlobalRpmsActions ( bool enable )
{
  deleteAllRpmsFromDiskAction->setEnabled ( enable );
  downloadAllProfiles->setEnabled ( enable );

  globalRpmsActionsState = enable;
}

void MainWindow::enableProfileDownloadAction ( bool enable )
{
  downloadCurrentProfile->setEnabled ( enable );

  profileDownloadActionState = enable;
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
  curFile = fileName;
  setWindowModified ( false );

  QString shownName = tr ( "Untitled" );

  if ( !curFile.isEmpty() ) {
    shownName = strippedName ( curFile );
    recentFiles.removeAll ( curFile );
    recentFiles.prepend ( curFile );
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
  if ( curFile.isEmpty() ) {
    return saveAs();

  } else {
    return saveFile ( curFile );
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
  downloadAllProfiles->setEnabled ( enable );
  downloadCurrentProfile->setEnabled ( enable );
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
  deleteAction->setDisabled ( disable );
  refreshStatus->setDisabled ( disable );
  addNewProfileAction->setDisabled ( disable );

  if ( !disable ) {
    enableAddRpms ( addRpmsState );
    enableDownloadButtons ( addRpmsState );
    deleteSelectedRpmsFromDiskAction->setDisabled ( !rpmActionsState );
    deleteAllRpmsFromDiskAction->setDisabled ( !rpmActionsState );
    duplicateProfileAction->setDisabled ( !profileActionsState );

  } else {
    addNewRpmToCurProfileAction->setEnabled ( false );
    pasteAction->setEnabled ( false );
    enableDownloadButtons ( false );
    deleteSelectedRpmsFromDiskAction->setDisabled ( true );
    deleteAllRpmsFromDiskAction->setDisabled ( true );
    duplicateProfileAction->setDisabled ( true );
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
  numberOfProfilesLabel->setText ( tr ( "Profiles: %1 " ).arg ( numberOfProfiles ) );
  numberOfPackagesLabel->setText ( tr ( "RPMs: %1 " ).arg ( numberOfRpms ) );
  numberOfOkPackagesLabel->setText ( tr ( "Ok: %1 " ).arg ( okRpms ) );
  numberOfFailedRpmsLabel->setText ( tr ( "Failed: %1 " ).arg ( failedRpms ) );
  numberOfAvailPackagesLabel->setText ( tr ( "Avail: %1 " ).arg ( availRpms ) );
  numberOfLocalAvailPackagesLabel->setText ( tr ( "Local Avail: %1 " ).arg ( localAvailRpms ) );
  numberOfUpdatedPackagesLabel->setText ( tr ( "Updated: %1 " ).arg ( updatedRpms ) );
  numberOfUnknownPackagesLabel->setText ( tr ( "Unknown: %1 " ).arg ( unknownRpms ) );
  downloadSizeLabel->setText ( tr ( "Download Size: %1" ).arg ( downloadSize ) );
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

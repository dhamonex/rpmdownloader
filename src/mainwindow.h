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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>

#include "repositoryprofile.h"

class QIcon;

class QAction;

class QActionGroup;

class QLabel;

class RpmDownloaderWidget;

class SearchFieldLineEdit;

class MainWindow : public QMainWindow
{

    Q_OBJECT

  public:
    MainWindow();

  protected:
    void closeEvent ( QCloseEvent *event );

  private slots:
    void addNewProfile();
    void addNewRpm();

    void enableAddRpms ( bool enable );
    void enableRpmActions ( bool enable, bool disableSingleItemActions );
    void enableProfileActions ( bool enable );
    void enableProfileDownloadAction ( bool enable );
    void enableGlobalRpmsActions ( bool enable );

    void editProfile ( RepositoryProfile *profile );
    void editProfile(); // edit current profile slot for editProfileAction
    void editRpmName ( QString actName, int rpmTableRow );
    void editRpmName(); // edit current rpm name slot for editRpmNameAction

    void tablesModified();
    void openRecentFile();

    bool save();
    bool saveAs();
    void open();
    void newFile();

    void startDownloadOne();
    void startDownloadAll();
    void refreshProfileStatus();
    void enableDownloadButtons ( bool enable );
    void downloadFinished();
    void disableEditActions ( bool disable );
    void changeSettingsViaDialog();
    void showStatusBar ( bool show );

    void changeToolBarStyle ( QAction *action );
    void increaseToolBarIconSize();
    void decreaseToolBarIconSize();

    void updateStatusBar ( int numberOfProfiles, int numberOfRpms, int okRpms, int failedRpms, int updatedRpms, int localAvailRpms, int availRpms, int unknownRpms, const QString &downloadSize );

    void showDetailsForPackage ( const Package &packageData, const QIcon &icon );

    void showLegend();
    void about();

  private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createContextMenu();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool okToContinue();
    bool loadProfileSet ( const QString &fileName );
    bool saveProfileSet ( const QString &fileName );
    void setCurrentProfileSet ( const QString &fileName );

    bool saveFile ( const QString &fileName );
    bool loadFile ( const QString &fileName );

    void setCurrentFile ( const QString &fileName );
    void updateRecentFileActions();
    QString strippedName ( const QString &fullFileName );

    RpmDownloaderWidget *rpmDownloaderWidget;

    // menus
    QMenu *fileMenu;
    QMenu *editMenu;
    QMenu *selectSubMenu;
    QMenu *toolsMenu;
    QMenu *optionsMenu;
    QMenu *downloadMenu;
    QMenu *settingsMenu;
    QMenu *helpMenu;

    // submenu for buttons style
    QMenu *toolButtonStyleMenu;

    // tool bar
    QToolBar *fileToolBar;
    QToolBar *editToolBar;
    QToolBar *downloadToolBar;
    QToolBar *searchToolBar;
    // search line edit
    SearchFieldLineEdit *searchFieldLineEdit;
    QLabel *searchLabel;
    // maybe unneeded: container for QToolbar::addWidget which returns a QAction
    QAction *searchLineEditAction;
    QAction *searchLabelAction;

    // for statusbar
    QLabel *numberOfProfilesLabel;
    QLabel *numberOfPackagesLabel;
    QLabel *numberOfOkPackagesLabel;
    QLabel *numberOfFailedRpmsLabel;
    QLabel *numberOfAvailPackagesLabel;
    QLabel *numberOfLocalAvailPackagesLabel;
    QLabel *numberOfUpdatedPackagesLabel;
    QLabel *numberOfUnknownPackagesLabel;
    QLabel *downloadSizeLabel;

    // actions
    QAction *newAction;
    QAction *openAction;
    QAction *saveAction;
    QAction *saveAsAction;
    QAction *exitAction;

    enum { MaxRecentFiles = 5 };
    QAction *recentFileActions[MaxRecentFiles];
    QAction *separatorAction;

    QAction *addNewProfileAction;
    QAction *deleteAction; // deleting current selected item form table
    QAction *addNewRpmToCurProfileAction;
    QAction *duplicateProfileAction;
    QAction *copyAction;
    QAction *cutAction;
    QAction *pasteAction;
    QAction *deleteAllRpmsFromDiskAction;
    QAction *deleteSelectedRpmsFromDiskAction;
    QAction *editProfileAction;
    QAction *editRpmNameAction;
    QAction *cleanOrphanedEntriesAction;
    QAction *resolveDepsForSelectedRpmAction;
    QAction *showDetailsForSelectedRpmAction;
    QAction *clearCacheDirectoryAction;

    QAction *downloadCurrentProfile;
    QAction *downloadAllProfiles;
    QAction *refreshStatus;

    QAction *changeSettingsAction;
    QAction *showStatusBarAction;

    // group the next three actions
    QAction *toolButtonStyleIconOnlyAction;
    QAction *toolButtonStyleTextOnlyAction;
    QAction *toolButtonStyleTextAndIconAction;
    QActionGroup *toolButtonStyleActionGroup;
    QAction *increaseToolIconSize;
    QAction *decreaseToolIconSize;

    QAction *showLegendAction;
    QAction *aboutAction;
    QAction *aboutQtAction;

    QString curFile;
    QStringList recentFiles;

    // action states
    bool addRpmsState;
    bool rpmActionsState;
    bool profileActionsState;
    bool profileDownloadActionState;
    bool globalRpmsActionsState;
};

#endif


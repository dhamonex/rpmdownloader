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

#include <QtWidgets/QMainWindow>

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
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_selectSubMenu;
    QMenu *m_toolsMenu;
    QMenu *m_optionsMenu;
    QMenu *m_downloadMenu;
    QMenu *m_settingsMenu;
    QMenu *m_helpMenu;

    // submenu for buttons style
    QMenu *m_toolButtonStyleMenu;

    // tool bar
    QToolBar *m_fileToolBar;
    QToolBar *m_editToolBar;
    QToolBar *m_downloadToolBar;
    QToolBar *m_searchToolBar;
    // search line edit
    SearchFieldLineEdit *m_searchFieldLineEdit;
    QLabel *m_searchLabel;
    // maybe unneeded: container for QToolbar::addWidget which returns a QAction
    QAction *m_searchLineEditAction;
    QAction *m_searchLabelAction;

    // for statusbar
    QLabel *m_numberOfProfilesLabel;
    QLabel *m_numberOfPackagesLabel;
    QLabel *m_numberOfOkPackagesLabel;
    QLabel *m_numberOfFailedRpmsLabel;
    QLabel *m_numberOfAvailPackagesLabel;
    QLabel *m_numberOfLocalAvailPackagesLabel;
    QLabel *m_numberOfUpdatedPackagesLabel;
    QLabel *m_numberOfUnknownPackagesLabel;
    QLabel *m_downloadSizeLabel;

    // actions
    QAction *m_newAction;
    QAction *m_openAction;
    QAction *m_saveAction;
    QAction *m_saveAsAction;
    QAction *m_exitAction;

    enum { MaxRecentFiles = 5 };
    QAction *m_recentFileActions[MaxRecentFiles];
    QAction *m_separatorAction;

    QAction *m_addNewProfileAction;
    QAction *m_deleteAction; // deleting current selected item form table
    QAction *m_addNewRpmToCurProfileAction;
    QAction *m_duplicateProfileAction;
    QAction *m_copyAction;
    QAction *m_cutAction;
    QAction *m_pasteAction;
    QAction *m_deleteAllRpmsFromDiskAction;
    QAction *m_deleteSelectedRpmsFromDiskAction;
    QAction *m_editProfileAction;
    QAction *m_editRpmNameAction;
    QAction *m_cleanOrphanedEntriesAction;
    QAction *m_resolveDepsForSelectedRpmAction;
    QAction *m_showDetailsForSelectedRpmAction;
    QAction *m_clearCacheDirectoryAction;

    QAction *m_downloadCurrentProfile;
    QAction *m_downloadAllProfiles;
    QAction *m_refreshStatus;

    QAction *m_changeSettingsAction;
    QAction *m_showStatusBarAction;

    // group the next three actions
    QAction *m_toolButtonStyleIconOnlyAction;
    QAction *m_toolButtonStyleTextOnlyAction;
    QAction *m_toolButtonStyleTextAndIconAction;
    QActionGroup *m_toolButtonStyleActionGroup;
    QAction *m_increaseToolIconSize;
    QAction *m_decreaseToolIconSize;

    QAction *m_showLegendAction;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;

    QString m_curFile;
    QStringList m_recentFiles;

    // action states
    bool m_addRpmsState;
    bool m_rpmActionsState;
    bool m_profileActionsState;
    bool m_profileDownloadActionState;
    bool m_globalRpmsActionsState;
};

#endif


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
#include "rpmdownloaderwidget.h"

#include <QtGui>
#include "plainrepositorycontentdownloader.h"
#include "yumrepositorycontentdownloader.h"
#include "rddatabasehandler.h"
#include "yumdepsolver.h"

RpmDownloaderWidget::RpmDownloaderWidget ( QWidget *parent )
    : QWidget ( parent )
{
  profilesTableWidget = new QTableWidget;
  profilesTableWidget->verticalHeader()->hide();
  packagesTableWidget = new QTableWidget;
  packagesTableWidget->verticalHeader()->hide();

  QGridLayout *mainGridLayout = new QGridLayout;
  mainGridLayout->addWidget ( profilesTableWidget, 0, 0 );
  mainGridLayout->addWidget ( packagesTableWidget, 0, 1 );
  // add a bit of stretching
  mainGridLayout->setColumnStretch ( 0, 4 );
  mainGridLayout->setColumnStretch ( 1, 10 );

  setLayout ( mainGridLayout );

  statusUpdateProgressDialog = 0;

  profilesTableWidget->setContextMenuPolicy ( Qt::ActionsContextMenu );
  packagesTableWidget->setContextMenuPolicy ( Qt::ActionsContextMenu );
  packagesTableWidget->setMouseTracking ( true );

  currentUpdatedProfile = -1;
  updateTimer = new QTimer ( this );
  updateTimer->setSingleShot ( true );

  // content downloaders
  plainContentDownloader = new PlainRepositoryContentDownloader ( this );
  yumContentDownloader = new YumRepositoryContentDownloader ( this );

  loading = false;

  clearAll();
  connect ( profilesTableWidget, SIGNAL ( currentCellChanged ( int, int, int, int ) ), this, SLOT ( clearAndInsertPackagesTable ( int, int, int, int ) ) );
  connect ( profilesTableWidget, SIGNAL ( cellDoubleClicked ( int, int ) ), this, SLOT ( profileDoubleClicked ( int, int ) ) );
  connect ( packagesTableWidget, SIGNAL ( cellDoubleClicked ( int, int ) ), this, SLOT ( packageDoubleClicked ( int, int ) ) );
  connect ( packagesTableWidget, SIGNAL ( itemSelectionChanged() ), this, SLOT ( rpmsTableSelectionChanged() ) );
  connect ( profilesTableWidget, SIGNAL ( itemSelectionChanged() ), this, SLOT ( profilesTableWidgetSelectionChanged() ) );

  // connect content downloader signals
  connect ( plainContentDownloader, SIGNAL ( finished ( int, bool ) ), this, SLOT ( contentDownloaderFinished ( int, bool ) ) );
  connect ( yumContentDownloader, SIGNAL ( finished ( int, bool ) ), this, SLOT ( contentDownloaderFinished ( int, bool ) ) );

  connect ( updateTimer, SIGNAL ( timeout() ), this, SLOT ( completeStatusUpdate() ) );

  updateTimer->start ( rpmDownloaderSettings().updateInterval() );
}

void RpmDownloaderWidget::addActionToPackagesTable ( QAction * action )
{
  packagesTableWidget->addAction ( action );
}

void RpmDownloaderWidget::addActionToProfilesTable ( QAction * action )
{
  profilesTableWidget->addAction ( action );
}

QAction * RpmDownloaderWidget::addSeparatorToPackagesTable()
{
  QAction *separatorAction = new QAction ( this );
  separatorAction->setSeparator ( true );
  packagesTableWidget->addAction ( separatorAction );
  return separatorAction;
}

QAction * RpmDownloaderWidget::addSeparatorToProfilesTable()
{
  QAction *separatorAction = new QAction ( this );
  separatorAction->setSeparator ( true );
  profilesTableWidget->addAction ( separatorAction );
  return separatorAction;
}

void RpmDownloaderWidget::clearPackagesTable()
{
  packagesTableWidget->clear();
  packagesTableWidget->setRowCount ( 0 );
  packagesTableWidget->setColumnCount ( 0 );
  packagesTableWidget->setColumnCount ( PackageColumns );

  packagesTableWidget->setHorizontalHeaderLabels ( QStringList()
      << tr ( "Download Status" )
      << tr ( "RPM Name" )
      << tr ( "Local Version" )
      << tr ( "Available Version" ) );

  // add column size policy
  packagesTableWidget->resizeColumnsToContents();
  // rpmsTableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  packagesTableWidget->horizontalHeader()->setStretchLastSection ( true );
  packagesTableWidget->horizontalHeader()->resizeSection ( 1, 100 );
  packagesTableWidget->horizontalHeader()->resizeSection ( 2, 120 );
  packagesTableWidget->horizontalHeader()->setHighlightSections ( false );
  packagesTableWidget->setAlternatingRowColors ( true );

  packagesTableWidget->setSelectionBehavior ( QAbstractItemView::SelectRows );
}

void RpmDownloaderWidget::clearProfilesTable()
{
  profilesTableWidget->clear();
  profilesTableWidget->setRowCount ( 0 );
  profilesTableWidget->setColumnCount ( 0 );
  profilesTableWidget->setColumnCount ( ProfileColumns );

  profilesTableWidget->setHorizontalHeaderLabels ( QStringList()
      << tr ( "Status" )
      << tr ( "Profile Name" ) );

  // add coloumn size policy
  profilesTableWidget->resizeColumnsToContents();
  profilesTableWidget->horizontalHeader()->setResizeMode ( 1, QHeaderView::Stretch );
  profilesTableWidget->horizontalHeader()->setHighlightSections ( false );
  profilesTableWidget->setAlternatingRowColors ( true );

  profilesTableWidget->setSelectionBehavior ( QAbstractItemView::SelectRows );
  profilesTableWidget->setSelectionMode ( QAbstractItemView::SingleSelection );

  QList<RepositoryProfile *>::iterator profileIter;

  for ( profileIter = profiles.begin(); profileIter != profiles.end(); ++profileIter ) {
    disconnect ( ( *profileIter ) );
  }

  // clear all profiles
  qDeleteAll ( profiles.begin(), profiles.end() ); // delete explicit all elements in the list

  profiles.clear();
}

void RpmDownloaderWidget::clearAll()
{
  cancelStatusUpdate();

  restartUpdateTimer();
  clearProfilesTable();
  clearPackagesTable();
  emit ( currentProfileHasPackages ( false ) );
  emit ( hasPackages ( false ) );
}

void RpmDownloaderWidget::addProfile ( RepositoryProfile *newProfile )
{
  int row = profilesTableWidget->rowCount();

  newProfile->setParent ( this );

  if ( row + 1 >= MaxRows ) {
    delete newProfile;
    return; // accept no more rows
  }

  QTableWidgetItem *profileNameItem = new QTableWidgetItem;

  profileNameItem->setText ( newProfile->profileName() );
  QTableWidgetItem *statusItem = new QTableWidgetItem;

  // items not editable
  profileNameItem->setFlags ( statusItem->flags() &~ Qt::ItemIsEditable );
  statusItem->setFlags ( statusItem->flags() &~ Qt::ItemIsEditable );

  // set icon
  statusItem->setIcon ( QIcon ( ":/images/unknown.png" ) );

  // insert a new row
  profilesTableWidget->setRowCount ( row + 1 );
  profilesTableWidget->setItem ( row, 0, statusItem );
  profilesTableWidget->setItem ( row, 1, profileNameItem );
  profiles.push_back ( newProfile );

  // set new item as selected
  profilesTableWidget->setCurrentCell ( row, 1 );

  connect ( newProfile, SIGNAL ( statusRefreshed() ), this, SLOT ( repositoryStatusUpdated() ) );
  emit ( modified() );
  emit ( gotProfiles ( true ) );
  computeAndEmitStats();
}

void RpmDownloaderWidget::duplicateCurrentProfile()
{
  int row = profilesTableWidget->currentRow();

  if ( row < 0 )
    return;

  RepositoryProfile *duplicatedProfile = new RepositoryProfile ( this );

  QHash<QString, Package> packages = profiles.at ( row )->selectedPackages();

  QHash<QString, Package>::const_iterator packageIter;

  for ( packageIter = packages.begin(); packageIter != packages.end(); ++packageIter ) {
    duplicatedProfile->addPackage ( packageIter.value() );
  }

  duplicatedProfile->setArchitectures ( profiles.at ( row )->architecures() );

  duplicatedProfile->setDownloadDir ( profiles.at ( row )->downloadDir() );
  duplicatedProfile->setProfileName ( profiles.at ( row )->profileName(), false ); // don't rename directory
  duplicatedProfile->setServerUrl ( profiles.at ( row )->serverUrl() );
  duplicatedProfile->setRepoType ( profiles.at ( row )->repoType() );

  duplicatedProfile->clearStatus();
  addProfile ( duplicatedProfile );
}

void RpmDownloaderWidget::profileForCurrentProfileChanged()
{
  int row = profilesTableWidget->currentRow();

  if ( currentUpdatedProfile == row ) { // interrupt udapte when profile changed
    cancelStatusUpdate();
  }

  if ( row < 0 ) // nothing selected
    return;


  profiles[row]->clearStatus();

  profilesTableWidget->item ( row, 1 )->setText ( profiles.at ( row )->profileName() );

  updateProfileTableStatus();

  updatePackagesTable();

  emit ( modified() );
}

void RpmDownloaderWidget::deleteCurrentItemsFromActiveTable()
{
  int row = profilesTableWidget->currentRow();

  if ( profilesTableWidget->hasFocus() && row >= 0 ) {
    // delete current profile
    if ( !confirmProfileDelete ( profiles.at ( row )->profileName() ) ) // to avoid removing profiles by mistake
      return;

    if ( currentUpdatedProfile == row ) {
      cancelStatusUpdate();

    } else if ( currentUpdatedProfile > row ) {
      --currentUpdatedProfile;

      if ( profiles.at ( currentUpdatedProfile )->repoType() == YUM ) // correct profile number
        yumContentDownloader->changeCurrentProfileNumber ( currentUpdatedProfile );
      else
        plainContentDownloader->changeCurrentProfileNumber ( currentUpdatedProfile );
    }

    // RDDatabaseHandler::removeDbConnection(profiles.at(row)->profileName());

    profilesTableWidget->removeRow ( row );

    // disconenct signals and destroy object
    RepositoryProfile *profile = profiles.at ( row );

    disconnect ( profile );

    profile->removeCacheDir();

    delete profile;

    // remove profile from list
    profiles.removeAt ( row );

    // select current row
    if ( row > 0 && row < profilesTableWidget->rowCount() )
      profilesTableWidget->setCurrentCell ( row, 1 );
    else if ( row > 0 ) // removed row was last row
      profilesTableWidget->setCurrentCell ( row - 1, 1 );
    else {
      emit ( gotProfiles ( false ) );
      clearPackagesTable();
    }

    emit ( modified() );

    checkForPackagesInAllProfiles();

  } else if ( packagesTableWidget->hasFocus() && row >= 0 ) {
    QList<QTableWidgetItem *> selectedPackageItems = packagesTableWidget->selectedItems();

    int packageRow = packagesTableWidget->currentRow();

    if ( packageRow < 0 ) // nothing selected
      return;

    QList<int> rowsToDelete; // collect rows shich should be deleted tu avoid counter conflicts

    foreach ( QTableWidgetItem *item, selectedPackageItems ) {
      if ( item->column() == 1 ) {
        int selectedRow = item->row();
        profiles[row]->removePackage ( packagesTableWidget->item ( selectedRow, 1 )->text() );
        rowsToDelete.push_back ( selectedRow ); // mark for delete
      }
    }

    qSort ( rowsToDelete );

    // now delete uneeded rows

    for ( int i = 0; i < rowsToDelete.size(); ++i ) {
      packagesTableWidget->removeRow ( rowsToDelete.at ( i ) - i );
    }

    if ( packageRow >= 0 && packageRow < packagesTableWidget->rowCount() )
      packagesTableWidget->setCurrentCell ( packageRow, 1 );
    else if ( packageRow > 0 ) // removed row was last row
      packagesTableWidget->setCurrentCell ( packageRow - rowsToDelete.size(), 1 );

    profiles[row]->computeProfileStatus();

    // update profile status display
    updateProfileTableStatus();

    emit ( modified() );

    checkForPackagesInAllProfiles();
  }

  computeAndEmitStats();
}

bool RpmDownloaderWidget::confirmProfileDelete ( const QString & profileName )
{
  int r = QMessageBox::warning ( this, tr ( "RPM Downloader" ),
                                 tr ( "Are you sure that you want to delete the \"%1\" profile?" ).arg ( profileName ),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No,
                                 QMessageBox::Cancel | QMessageBox::Escape );

  if ( r == QMessageBox::Yes ) {
    return true;

  } else if ( r == QMessageBox::Cancel ) {
    return false;
  }

  return false;
}

void RpmDownloaderWidget::deleteSelectedPackagesFromDisk()
{
  QApplication::setOverrideCursor ( Qt::BusyCursor );

  int row = profilesTableWidget->currentRow();

  if ( row < 0 ) // no profile selceted
    return;

  QList<QTableWidgetItem *> selectedPackageItems = packagesTableWidget->selectedItems();

  QProgressDialog progress ( tr ( "removing packages please wait ..." ), tr ( "Abort" ), 0, selectedPackageItems.size() / 4.0, this ); // always four per column represents one package

  progress.setWindowModality ( Qt::WindowModal );

  int packageCounter = 0;

  foreach ( QTableWidgetItem *item, selectedPackageItems ) {
    if ( item->column() == 1 ) {
      qApp->processEvents();

      if ( progress.wasCanceled() ) {
        break;
      }

      profiles[row]->deletePackageFromDisk ( item->text() );

      progress.setValue ( ++packageCounter );
    }
  }

  profiles[row]->computeProfileStatus(); // only compute once

  updatePackagesTable();
  updateProfileTableStatus();
  computeAndEmitStats();

  QApplication::restoreOverrideCursor();
}

void RpmDownloaderWidget::deleteAllPackagesFromDisk()
{
  QApplication::setOverrideCursor ( Qt::BusyCursor );

  QProgressDialog progress ( tr ( "removing packages please wait ..." ), tr ( "Abort" ), 0, profiles.size(), this );
  progress.setWindowModality ( Qt::WindowModal );

  for ( int i = 0; i < profiles.size(); ++i ) {
    qApp->processEvents();

    if ( progress.wasCanceled() ) {
      break;
    }

    profiles[i]->deleteAllPackagesFromDisk();

    progress.setValue ( i + 1 );
  }

  updatePackagesTable();

  updateProfileTableStatus();
  computeAndEmitStats();
  QApplication::restoreOverrideCursor();
}

void RpmDownloaderWidget::copySelectedPackages()
{
  QString str;

  QList<QTableWidgetItem *> selectedPackageItems = packagesTableWidget->selectedItems();
  bool firstItem = true;

  QList<QTableWidgetItem *>::const_iterator itemsIter;

  for ( itemsIter = selectedPackageItems.begin(); itemsIter != selectedPackageItems.end(); ++itemsIter ) {
    if ( ( *itemsIter )->column() == 1 ) {
      if ( firstItem ) {
        str = ( *itemsIter )->text();
        firstItem = false;

      } else {
        str += " " + ( *itemsIter )->text();
      }
    }
  }

  QApplication::clipboard()->setText ( str );
}

void RpmDownloaderWidget::cutSelectedPackages()
{
  copySelectedPackages();
  deleteCurrentItemsFromActiveTable();
}

void RpmDownloaderWidget::insertPackagesFromClipboard()
{
  QString str = QApplication::clipboard()->text();
  str.replace ( "\n", " " );
  str.replace ( "\t", " " );
  QStringList packageNames = str.split ( " " );

  foreach ( const QString &packageName, packageNames ) {
    if ( !packageName.isEmpty() ) {
      Package package ( packageName );
      addPackageToCurProfile ( package );
    }
  }
}

void RpmDownloaderWidget::profileDebugOutput()
{
  for ( int i = 0; i < profiles.size(); ++i ) {
    qDebug ( "Profile at %i name %s", i, qPrintable ( profiles.at ( i )->profileName() ) );
  }
}

void RpmDownloaderWidget::addPackageToCurProfileWithDeps ( const Package & package )
{
  if ( profilesTableWidget->rowCount() <= 0 ) // nothing to do
    return;

  int row = profilesTableWidget->currentRow();

  if ( row < 0 ) // should not happen
    return;

  if ( !profiles.at ( row )->repoType() == YUM || currentUpdatedProfile == row ) {
    // only yum can resolve deps
    // or database is incomplete because it's currently updated then
    // add rpm as normal
    addPackageToCurProfile ( package );
    return;
  }

  // make busy cursor
  QApplication::setOverrideCursor ( Qt::BusyCursor );

  emit ( isBusy ( true ) );

  QProgressDialog progress ( tr ( "Solving Dependencies..." ), tr ( "Abort" ), 0, 300, this );

  progress.setWindowModality ( Qt::WindowModal );

  YumDepSolver depSolver;

  if ( !depSolver.resolveSatFor ( package.packageName(), rpmDownloaderSettings().cacheDir().absolutePath() + "/" + profiles.at ( row )->profileName() + " " + profiles.at ( row )->profileName() + ".db", profiles.at ( row )->profileName(), progress, profiles.at ( row )->architecures(), rpmDownloaderSettings().useMemDbSatSolving() ) ) {
    // could not resolve deps
    // unbusy
    QApplication::restoreOverrideCursor();
    emit ( isBusy ( false ) );
    addPackageToCurProfile ( package );
    return;
  }

  QStringList requiredRpms = depSolver.getRpms();

  QStringListIterator i ( requiredRpms );

  while ( i.hasNext() ) {
    Package newPackage;
    newPackage.setPackageName ( i.next() );
    addPackageToCurProfile ( newPackage );
  }

  emit ( isBusy ( false ) );

  QApplication::restoreOverrideCursor();

}

void RpmDownloaderWidget::addPackageToCurProfile ( const Package & package )
{
  if ( profilesTableWidget->rowCount() <= 0 ) // nothing to do
    return;

  int row = profilesTableWidget->currentRow();

  if ( row < 0 ) // should not happen
    return;

  if ( profiles.at ( row )->hasPackage ( package.packageName() ) )
    return;

  if ( profiles.at ( row )->numberOfSelectedPackages() + 1 > MaxRows ) {
    // no more packages accepted
    qDebug ( "maximum number of packages reached" );
    return;
  }

  profiles[row]->addPackage ( package );

  // apply filter if needed

  if ( !lastFilterString.isEmpty() )
    applyPackageFilter ( lastFilterString );
  else
    addPackageToPackagesTableWidget ( package.packageName() );

  emit ( modified() );

  emit ( hasPackages ( true ) );

  emit ( currentProfileHasPackages ( true ) );

  updatePackagesTable();

  updateProfileTableStatus(); // update the status display

  computeAndEmitStats();
}

void RpmDownloaderWidget::addPackageToPackagesTableWidget ( const QString& packageName )
{
  int packageRow = packagesTableWidget->rowCount();

  packagesTableWidget->setRowCount ( packageRow + 1 );

  // add items

  for ( int i = 0; i < PackageColumns; ++i ) {
    QTableWidgetItem *item = new QTableWidgetItem;

    if ( i == 1 )
      item->setText ( packageName );

    addItemToPackagesTableWidget ( item, packageRow, i );
  }

  packagesTableWidget->setCurrentCell ( packageRow, 1 );

}

void RpmDownloaderWidget::addItemToPackagesTableWidget ( QTableWidgetItem* item, const int row, const int column )
{
  // items not editable
  item->setFlags ( item->flags() &~ Qt::ItemIsEditable );
  item->setFlags ( item->flags() &~ Qt::ItemIsEditable );
  packagesTableWidget->setItem ( row, column, item );
}

void RpmDownloaderWidget::applyPackageFilter ( const QString& filterText )
{
  clearPackagesTable();
  lastFilterString = filterText;
  int row = profilesTableWidget->currentRow();

  if ( row < 0 ) // nothing to do
    return;

  QApplication::setOverrideCursor ( Qt::BusyCursor );

  QStringList packageNames = profiles.at ( row )->selectedPackages().keys();

  QStringList::const_iterator packageNamesIter;

  const QStringList::const_iterator end = packageNames.end();

  for ( packageNamesIter = packageNames.begin(); packageNamesIter != end; ++packageNamesIter ) {
    if ( lastFilterString.isEmpty() || packageNamesIter->startsWith ( lastFilterString ) ) { // add package
      addPackageToPackagesTableWidget ( *packageNamesIter );
    }
  }

  QApplication::restoreOverrideCursor();

  // add the status icons
  updatePackagesTable();
}

void RpmDownloaderWidget::resolveDependenciesForCurrentSelectedPackage()
{
  QList<QTableWidgetItem *> selectedItems = packagesTableWidget->selectedItems();

  if ( packagesTableWidget->rowCount() <= 0 || selectedItems.size() != 4 ) // not exactly one row selected
    return;

  QListIterator<QTableWidgetItem *> i ( selectedItems );

  while ( i.hasNext() ) {
    QTableWidgetItem *item = i.next();

    if ( item->column() == 1 ) {
      addPackageToCurProfileWithDeps ( Package ( item->text() ) );
      break; // only interested in column 1
    }
  }
}

QIcon RpmDownloaderWidget::iconForStatus ( Status status )
{
  switch ( status ) {
    case UNKNOWN:
      return QIcon ( ":/images/unknown.png" );
      break;
    case OK:
      return QIcon ( ":/images/ok.png" );
      break;
    case FAILED:
      return QIcon ( ":/images/notOk.png" );
      break;
    case AVAILABLE:
      return QIcon ( ":/images/avail.png" );
      break;
    case UPDATE:
      return QIcon ( ":/images/newVersion.png" );
      break;
    case LOCALAVAIL:
      return QIcon ( ":/images/localAvail.png" );
      break;
  }

  return QIcon ( ":/images/unknown.png" ); // default return unknown
}

void RpmDownloaderWidget::updatePackagesTable()
{
  int row = profilesTableWidget->currentRow();

  if ( row < 0 )
    return;

  if ( !loading )
    QApplication::setOverrideCursor ( Qt::BusyCursor );

  for ( int i = 0; i < packagesTableWidget->rowCount(); ++i ) {
    QTableWidgetItem *packageNameItem = packagesTableWidget->item ( i, 1 );
    PackageVersions localVersions = profiles.at ( row )->getLocalPackageVersions ( packageNameItem->text() );
    PackageVersions remoteVersions = profiles.at ( row )->getRemotePackageVersions ( packageNameItem->text() );

    for ( int j = 0; j < PackageColumns; ++j ) {
      QTableWidgetItem *item = packagesTableWidget->item ( i, j );

      if ( j == 0 ) {
        item->setIcon ( iconForStatus ( profiles.at ( row )->getPackageStatus ( packageNameItem->text() ) ) );

      } else if ( j == 2 ) {
        item->setText ( formatPackageVersionsForDisplay ( localVersions ) );
        item->setToolTip ( formatPackageVersionsForDisplay ( localVersions, true ) );

      } else if ( j == 3 ) {
        item->setText ( formatPackageVersionsForDisplay ( remoteVersions ) );
        item->setToolTip ( formatPackageVersionsForDisplay ( remoteVersions, true ) );
      }
    }
  }

  if ( !loading )
    QApplication::restoreOverrideCursor();
}

QString RpmDownloaderWidget::formatPackageVersionsForDisplay ( const PackageVersions &versions, const bool multipleLines ) const
{
  QString formated;
  QString versionBefore;

  PackageVersions::const_iterator versionsIter;

  for ( versionsIter = versions.begin(); versionsIter != versions.end(); ++versionsIter ) {

    if ( versionsIter != versions.begin() ) {
      if ( multipleLines )
        formated += "<br>";

    } else {
      versionBefore = versionsIter.value();
    }

    if ( multipleLines ) {
      formated += versionsIter.key() + ":" + versionsIter.value();

    } else {
      if ( formated.isEmpty() || formated == "-1" )
        formated = versionsIter.value();

      if ( versionsIter.value() == "-1" ) { // ignore -1
        continue;
      }

      // find identic substring between the versions

      if ( formated != versionsIter.value() ) {
        int index = versionsIter.value().lastIndexOf ( formated );

        if ( index == versionsIter.value().size() - 1 ) {
          formated = versionsIter.value();

        } else {
          formated = versionsIter.value().left ( index ) + "...";
        }
      }
    }
  }

  return formated;
}

void RpmDownloaderWidget::updateProfileTableStatus()
{
  for ( int i = 0; i < profiles.size(); ++i ) {
    // profiles[i]->computeProfileStatus(); // compute overall status
    profilesTableWidget->item ( i, 0 )->setIcon ( iconForStatus ( profiles.at ( i )->profileStatus() ) );
  }
}

void RpmDownloaderWidget::clearAndInsertPackagesTable ( int row, int column, int oldRow, int oldColumn )
{
  Q_UNUSED ( column );
  Q_UNUSED ( oldColumn );

  if ( row == oldRow ) // no update needed
    return;

  if ( row < 0 )
    return;

  clearPackagesTable();

  QStringList packageNames = profiles.at ( row )->selectedPackages().keys();

  if ( packageNames.size() >= MaxRows ) // error
    return;

  if ( !loading )
    QApplication::setOverrideCursor ( Qt::BusyCursor );

  if ( lastFilterString.isEmpty() ) { // insert packages
    QStringList::const_iterator packageNameIter;

    for ( packageNameIter = packageNames.begin(); packageNameIter != packageNames.end(); ++packageNameIter ) {
      addPackageToPackagesTableWidget ( *packageNameIter );
    }

  } else { // let the filter insert the packages
    applyPackageFilter ( lastFilterString );
  }

  if ( !loading )
    QApplication::restoreOverrideCursor();

  packagesTableWidget->setCurrentCell ( packagesTableWidget->rowCount() - 1, 1 );

  updatePackagesTable();
}

void RpmDownloaderWidget::profileDoubleClicked ( int row, int column )
{
  Q_UNUSED ( column );
  emit ( repositoryProfileDoubleClicked ( profiles.at ( row ) ) );
}

void RpmDownloaderWidget::packageDoubleClicked ( int row, int column )
{
  Q_UNUSED ( column );
  emit ( packageDoubleClicked ( packagesTableWidget->item ( row, 1 )->text(), packagesTableWidget->currentRow() ) );
}

bool RpmDownloaderWidget::saveToFile ( const QString & fileName )
{
  QFile file ( fileName );

  if ( !file.open ( QIODevice::WriteOnly ) ) {
    QMessageBox::warning ( this, tr ( "RPM Downloader" ),
                           tr ( "Cannot write file %1:\n%2." )
                           .arg ( file.fileName() )
                           .arg ( file.errorString() ) );
    return false;
  }

  QDataStream out ( &file );

  out.setVersion ( QDataStream::Qt_4_3 );

  // write magic number
  out << quint32 ( MagicNumber );

  QApplication::setOverrideCursor ( Qt::WaitCursor );

  for ( int i = 0; i < profiles.size(); ++i ) {
    QHashIterator<QString, Package> packageIter ( profiles.at ( i )->selectedPackages() );
    bool added = false;

    while ( packageIter.hasNext() ) {
      packageIter.next();
      out << static_cast<quint16> ( i ) << profiles.at ( i )->profileName()
      << profiles.at ( i )->serverUrl().toString()
      << profiles.at ( i )->downloadDir()
      << static_cast<quint16> ( profiles.at ( i )->architecures() )
      << static_cast<quint16> ( profiles.at ( i )->repoType() )
      << packageIter.value().packageName()
      << false; // its not a fake rpm
      added = true;
    }

    if ( !added ) { // force profile add
      out << static_cast<quint16> ( i ) << profiles.at ( i )->profileName()
      << profiles.at ( i )->serverUrl().toString()
      << profiles.at ( i )->downloadDir()
      << static_cast<quint16> ( profiles.at ( i )->architecures() )
      << static_cast<quint16> ( profiles.at ( i )->repoType() )
      << "Fake"
      << true;
    }
  }

  QApplication::restoreOverrideCursor();

  file.close();
  return true;
}

bool RpmDownloaderWidget::loadProfilesFile ( const QString & fileName )
{
  cancelStatusUpdate();
  updateTimer->stop(); // stop timer

  QFile file ( fileName );

  if ( !file.open ( QIODevice::ReadOnly ) ) {
    QMessageBox::warning ( this, tr ( "RPM Downloader" ),
                           tr ( "Cannot read file %1:\n%2." )
                           .arg ( file.fileName() )
                           .arg ( file.errorString() ) );
    return false;
  }

  QDataStream in ( &file );

  in.setVersion ( QDataStream::Qt_4_3 );

  quint32 magic;
  in >> magic;

  if ( magic != MagicNumber && magic != MagicNumberOld ) {
    QMessageBox::warning ( this, tr ( "RPM Downloader" ),
                           tr ( "The selected file is not a RPM Downloader file." ) );
    return false;
  }

  clearAll();

  quint16 architectures;
  quint16 pos;
  quint16 repoType;
  QString profileName;
  QString serverUrl;
  QString downloadDir;
  QString packageName;
  bool fakeRpm;

  loading = true;
  QApplication::setOverrideCursor ( Qt::WaitCursor );

  while ( !in.atEnd() ) {
    if ( magic == MagicNumberOld ) {
      // the new scheme uses complete url so build url from server address and server path
      QString serverAddress, serverPath;
      in >> pos >> profileName >> serverAddress >> serverPath >> downloadDir >> architectures >> repoType >> packageName >> fakeRpm;
      serverUrl = serverAddress + "/" + serverPath;

    } else {
      in >> pos >> profileName >> serverUrl >> downloadDir >> architectures >> repoType >> packageName >> fakeRpm;
    }

    // qDebug("Profile size %i", profiles.size());

    if ( pos == profiles.size() && !profileName.isEmpty() ) {
      RepositoryProfile *profile = new RepositoryProfile ( this );
      profile->setProfileName ( profileName, false ); // don't rename directory and database
      profile->setServerUrl ( serverUrl );
      profile->setDownloadDir ( downloadDir );
      profile->setArchitectures ( static_cast<RPM::Architectures> ( architectures ) );
      profile->setRepoType ( static_cast<RepositoryType> ( repoType ) );
      addProfile ( profile );
    }

    if ( !fakeRpm && !packageName.isEmpty() ) {
      Package package ( packageName );
      addPackageToCurProfile ( package );
    }
  }

  QApplication::restoreOverrideCursor();

  loading = false;

  // completeRpmStatusUpdate();
  updateTimer->start ( 3000 ); // avoid signal race condition DON'T CALL completeRpmStatusUpdate without timer!!!
  return true;
}

RpmDownloaderWidget::~RpmDownloaderWidget()
{
  RepositoryProfile::clearDirectory ( rpmDownloaderSettings().tempDir().absolutePath() );
}

void RpmDownloaderWidget::completeStatusUpdate ( bool showProgress )
{
  if ( profiles.size() < 1 ) // no profiles
    return;

  if ( showProgress ) {
    if ( !statusUpdateProgressDialog ) {
      statusUpdateProgressDialog = new QProgressDialog ( tr ( "Status update in progress ..." ), tr ( "Cancel" ), 0, profiles.size(), this );
      connect ( statusUpdateProgressDialog, SIGNAL ( canceled() ), this, SLOT ( cancelStatusUpdate() ) );

    } else {
      statusUpdateProgressDialog->setMaximum ( profiles.size() );
    }

    statusUpdateProgressDialog->setVisible ( true );

    statusUpdateProgressDialog->setValue ( 0 );
    QApplication::setOverrideCursor ( Qt::BusyCursor );
    emit ( isBusy ( true ) );
  }

  if ( updateTimer->isActive() ) // stop update timer when running
    updateTimer->stop();

  if ( currentUpdatedProfile >= 0 && showProgress ) {
    statusUpdateProgressDialog->setValue ( currentUpdatedProfile );
    return;
  }

  currentUpdatedProfile = -1;

  updateNextProfile();

}

void RpmDownloaderWidget::updateNextProfile()
{
  if ( currentUpdatedProfile + 1 < profiles.size() ) {

    ++currentUpdatedProfile;

    if ( statusUpdateProgressDialog ) {
      if ( !statusUpdateProgressDialog->isHidden() )
        statusUpdateProgressDialog->setValue ( currentUpdatedProfile ); // not hidden
    }

    // initiate repository contents update
    QString serverUrl = profiles.at ( currentUpdatedProfile )->serverUrl().toString();

    // differnet update methods for different repository types
    if ( profiles.at ( currentUpdatedProfile )->repoType() == YUM ) {
      // start yum content update
      yumContentDownloader->startContentUpdate ( currentUpdatedProfile, profiles.at ( currentUpdatedProfile )->databaseFile(), profiles.at ( currentUpdatedProfile )->profileName(), serverUrl, PackageMetaData::archStringList ( profiles.at ( currentUpdatedProfile )->architecures() ) );

    } else {
      plainContentDownloader->startContentUpdate ( currentUpdatedProfile, profiles.at ( currentUpdatedProfile )->databaseFile(), profiles.at ( currentUpdatedProfile )->profileName(), serverUrl, PackageMetaData::archStringList ( profiles.at ( currentUpdatedProfile )->architecures() ) );
    }

  } else {
    if ( statusUpdateProgressDialog ) {
      if ( !statusUpdateProgressDialog->isHidden() )
        statusUpdateProgressDialog->setValue ( currentUpdatedProfile + 1 ); // should hide now

      QApplication::restoreOverrideCursor(); // reset cursor

      emit ( isBusy ( false ) );
    }

    currentUpdatedProfile = -1;

    updateTimer->start ( rpmDownloaderSettings().updateInterval() );
  }
}

void RpmDownloaderWidget::repositoryStatusUpdated()
{
  QApplication::setOverrideCursor ( Qt::WaitCursor );
  updateProfileTableStatus();
  updatePackagesTable();
  computeAndEmitStats();
  QApplication::restoreOverrideCursor(); // reset cursor
}

void RpmDownloaderWidget::changePackageName ( const QString & oldName, const QString & newName, const int rpmsTableRow )
{
  profiles[profilesTableWidget->currentRow() ]->changePackageName ( oldName, newName );
  packagesTableWidget->item ( rpmsTableRow, 1 )->setText ( newName );
  emit ( modified() );
}

void RpmDownloaderWidget::changePackageName ( const QString & oldName, const QString & newName )
{
  changePackageName ( oldName, newName, packagesTableWidget->currentRow() );
}

void RpmDownloaderWidget::contentDownloaderFinished ( int profileNumber, bool error )
{
  if ( profileNumber != currentUpdatedProfile ) {
    QMessageBox::critical ( this, tr ( "RPM Downloader" ),
                            tr ( "Something went wrong, this should not happen but the updated profile doesn't match to the profile where contents was updated" ), QMessageBox::Ok | QMessageBox::Default );
    updateNextProfile();
    return;
  }

  if ( error ) {
    QString errMsg;

    if ( profiles.at ( profileNumber )->repoType() == YUM )
      errMsg = yumContentDownloader->readError();
    else
      errMsg = plainContentDownloader->readError();

    QMessageBox::critical ( this, tr ( "RPM Downloader" ),
                            tr ( "Content update failed %1" ).arg ( errMsg ), QMessageBox::Ok | QMessageBox::Default );

    updateNextProfile();

    return;
  }

  if ( profileNumber < profiles.size() && profileNumber >= 0 ) {
    profiles[profileNumber]->refreshStatus();
    updateNextProfile();
  }
}

void RpmDownloaderWidget::downloadPackagesForCurrentProfile()
{
  downloadPackages ( false );
}

void RpmDownloaderWidget::downloadPackagesForAllProfiles()
{
  downloadPackages ( true );
}

void RpmDownloaderWidget::downloadPackages ( bool all )
{
  updateTimer->stop();
  DownloadProgressDialog dialog ( this );
  int row = profilesTableWidget->currentRow();

  if ( row < 0 ) {
    emit ( downloadFinished() );
    return;
  }

  int numberOfRpms = computeNumberOfDownloadRpms ( all );

  if ( all ) {
    dialog.setProfilesForDownload ( profiles );

  } else {
    dialog.setProfilesForDownload ( profiles.at ( row ) );
  }

  if ( numberOfRpms <= 0 ) {
    emit ( downloadFinished() );
    return;
  }

  dialog.setDeleteOldVersion ( rpmDownloaderSettings().deleteOldVersions() );

  dialog.setNumberOfRpms ( numberOfRpms );

  if ( !dialog.exec() ) {
    QString errorString = dialog.getError();
    QMessageBox::critical ( this, tr ( "RPM Downloader" ), errorString, QMessageBox::Ok | QMessageBox::Default );
    completeStatusUpdate ( true );

  } else {
    if ( all ) {
      QList<RepositoryProfile *>::iterator profileIter;

      for ( profileIter = profiles.begin(); profileIter != profiles.end(); ++profileIter ) {
        ( *profileIter )->packagesUpdated ( rpmDownloaderSettings().deleteOldVersions() );
      }

    } else {
      profiles[row]->packagesUpdated ( rpmDownloaderSettings().deleteOldVersions() );
    }
  }

  emit ( downloadFinished() );

  computeAndEmitStats();

  // status update not necessary all is fine only need to update the display tables
  updatePackagesTable();
  updateProfileTableStatus();
}

int RpmDownloaderWidget::computeNumberOfDownloadRpms ( bool all )
{
  if ( !all ) {
    int row = profilesTableWidget->currentRow();

    if ( row < 0 )
      return 0;

    return profiles.at ( row )->numberOfPackagesToDownload();

  } else {
    int numberOfRpms = 0;

    QListIterator<RepositoryProfile *> profileIter ( profiles );

    while ( profileIter.hasNext() ) {
      numberOfRpms += profileIter.next()->numberOfPackagesToDownload();
    }

    return numberOfRpms;
  }

  return 0;
}

void RpmDownloaderWidget::restartUpdateTimer()
{
  if ( updateTimer->isActive() )
    updateTimer->stop();

  currentUpdatedProfile = -1;

  updateTimer->start ( rpmDownloaderSettings().updateInterval() );
}

void RpmDownloaderWidget::cancelStatusUpdate()
{
  if ( statusUpdateProgressDialog ) {
    if ( !statusUpdateProgressDialog->isHidden() )
      statusUpdateProgressDialog->setValue ( profiles.size() );
  }

  if ( currentUpdatedProfile < 0 )
    return; // no update in progress

  if ( profiles.at ( currentUpdatedProfile )->repoType() == YUM )
    yumContentDownloader->cancelContentUpdate();
  else
    plainContentDownloader->cancelContentUpdate();

  currentUpdatedProfile = -1;

  restartUpdateTimer();

  QApplication::restoreOverrideCursor(); // reset cursor

  emit ( isBusy ( false ) );
}

void RpmDownloaderWidget::rpmsTableSelectionChanged()
{
  if ( packagesTableWidget->selectedItems().size() > 4 ) // because one row has 4 items
    emit ( hasPackageSelection ( true, true ) );
  else if ( packagesTableWidget->selectedItems().size() == 4 )
    emit ( hasPackageSelection ( true, false ) );
  else
    emit ( hasPackageSelection ( false, false ) );
}

void RpmDownloaderWidget::profilesTableWidgetSelectionChanged()
{
  if ( profilesTableWidget->selectedItems().size() > 0 ) {
    emit ( hasProfileSelection ( true ) );

    if ( profiles.at ( profilesTableWidget->currentRow() )->selectedPackages().size() > 0 ) {
      emit ( currentProfileHasPackages ( true ) );

    } else {
      emit ( currentProfileHasPackages ( false ) );
    }

  } else {
    emit ( hasProfileSelection ( false ) );
    emit ( currentProfileHasPackages ( false ) );
  }
}

void RpmDownloaderWidget::applyNewSettings()
{
  cancelStatusUpdate();

  if ( updateTimer->isActive() ) {
    updateTimer->stop();
    updateTimer->start ( rpmDownloaderSettings().updateInterval() );
  }
}

QStringList RpmDownloaderWidget::getRepositoryContentsForCurrentRepo() const
{
  if ( profilesTableWidget->currentRow() >= 0 )
    return profiles.at ( profilesTableWidget->currentRow() )->getProvidedPackages();

  return QStringList();
}

RepositoryProfile *RpmDownloaderWidget::getCurrentRepositoryProfile()
{
  if ( profilesTableWidget->currentRow() >= 0 )
    return profiles.at ( profilesTableWidget->currentRow() );

  return NULL;
}

QString RpmDownloaderWidget::getCurrentPackageName() const
{
  if ( packagesTableWidget->currentRow() >= 0 )
    return packagesTableWidget->item ( packagesTableWidget->currentRow(), 1 ) ->text();

  return QString();
}

void RpmDownloaderWidget::checkForPackagesInAllProfiles()
{
  int row = profilesTableWidget->currentRow();

  if ( row < 0 ) { // got no profiles
    emit ( currentProfileHasPackages ( false ) );
    emit ( hasPackages ( false ) );
    return;
  }

  // check for rpms in the current profile

  if ( profiles.at ( row )->selectedPackages().size() < 1 ) {
    emit ( currentProfileHasPackages ( false ) );

    // search for rpms in other profiles
    bool repositoriesHasPackagess = false;

    for ( int i = 0; i < profiles.size(); ++i ) {
      if ( i == row ) // no need to search act row
        continue;

      if ( profiles.at ( i )->selectedPackages().size() > 0 ) {
        repositoriesHasPackagess = true;
        break;
      }
    }

    emit ( hasPackages ( repositoriesHasPackagess ) );
  }
}

void RpmDownloaderWidget::computeAndEmitStats()
{
  int numberOfProfiles, numberOfRpms, okPackages, failedPackages, updatedPackages, localAvailPackages, availPackages, unknownPackages;

  qint64 downloadSize = 0;

  numberOfProfiles = profiles.size();
  numberOfRpms = okPackages = failedPackages = updatedPackages = localAvailPackages = availPackages = unknownPackages = 0;

  QList<RepositoryProfile *>::const_iterator profileIterator;

  for ( profileIterator = profiles.begin(); profileIterator != profiles.end(); ++profileIterator ) {
    numberOfRpms += ( *profileIterator )->numberOfSelectedPackages();
    okPackages += ( *profileIterator )->numberOfOkPackages();
    failedPackages += ( *profileIterator )->numberOfFailedPackages();
    updatedPackages += ( *profileIterator )->numberOfUpdatedPackages();
    localAvailPackages += ( *profileIterator )->numberOfLocalAvailPackages();
    availPackages += ( *profileIterator )->numberOfAvailablePackages();
    unknownPackages += ( *profileIterator )->numberOfUnknownPackages();
    downloadSize += ( *profileIterator )->downloadSizeInBytes();
  }

  if ( downloadSize < 0 ) {
    // just indicate that it is unknown
    downloadSize = -1;
  }

  emit ( profileStats ( numberOfProfiles, numberOfRpms, okPackages, failedPackages, updatedPackages, localAvailPackages, availPackages, unknownPackages, PackageMetaData::sizeAsString ( downloadSize ) ) );
}

void RpmDownloaderWidget::triggerComputeStats()
{
  computeAndEmitStats();
}

bool RpmDownloaderWidget::currentRepoSupportsDepSolving() const
{
  int row = profilesTableWidget->currentRow();

  if ( row < 0 )
    return false;

  if ( profiles.at ( row )->repoType() == YUM ) {
    return true;
  }

  return false;
}

void RpmDownloaderWidget::prepareDetailsView()
{
  int row = profilesTableWidget->currentRow();

  if ( row < 0 )
    return;

  QString currentPackageName = getCurrentPackageName();

  if ( currentPackageName.isEmpty() )
    return;

  emit ( packageDetailsReady ( profiles.at ( row )->getPackageData ( currentPackageName ), iconForStatus ( profiles.at ( row )->getPackageStatus ( currentPackageName ) ) ) );
}

void RpmDownloaderWidget::clearCacheDirectory()
{
  QDir cacheDir ( rpmDownloaderSettings().cacheDir() );
  cacheDir.setFilter ( QDir::Dirs );

  foreach ( const QString &subDir, cacheDir.entryList() ) {
    if ( subDir != "." && subDir != ".." ) {
      RepositoryProfile::clearDirectory ( cacheDir.absolutePath() + "/" + subDir );
      cacheDir.rmdir ( subDir );
    }
  }
}

void RpmDownloaderWidget::cleanOrphanedPackageEntries()
{
  int removedOrphanedPackages = 0;

  foreach ( RepositoryProfile *profile, profiles ) {
    removedOrphanedPackages += profile->cleanupOrphanedPackages();
  }

  int row = profilesTableWidget->currentRow();

  if ( row < 1 )
    return;

  if ( removedOrphanedPackages > 0 )
    emit ( modified() );

  clearAndInsertPackagesTable ( row, 0, -1, 0 ); // force redraw of packages
}


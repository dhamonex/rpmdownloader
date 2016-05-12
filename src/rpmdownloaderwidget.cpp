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

#include <QtGui/QHeaderView>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressBar>
#include <QtGui/QProgressDialog>
#include <QtGui/QClipboard>
#include <QtWidgets/QTableWidget>
#include <QtCore/QTimer>

#include "plainrepositorycontentdownloader.h"
#include "yumrepositorycontentdownloader.h"
#include "rddatabasehandler.h"
#include "yumdepsolver.h"

RpmDownloaderWidget::RpmDownloaderWidget( QWidget *parent )
  : QWidget( parent )
{
  m_profilesTableWidget = new QTableWidget;
  m_profilesTableWidget->verticalHeader()->hide();
  m_packagesTableWidget = new QTableWidget;
  m_packagesTableWidget->verticalHeader()->hide();

  QGridLayout *mainGridLayout = new QGridLayout;
  mainGridLayout->addWidget( m_profilesTableWidget, 0, 0 );
  mainGridLayout->addWidget( m_packagesTableWidget, 0, 1 );
  // add a bit of stretching
  mainGridLayout->setColumnStretch( 0, 4 );
  mainGridLayout->setColumnStretch( 1, 10 );

  setLayout( mainGridLayout );

  m_statusUpdateProgressDialog = 0;

  m_profilesTableWidget->setContextMenuPolicy( Qt::ActionsContextMenu );
  m_packagesTableWidget->setContextMenuPolicy( Qt::ActionsContextMenu );
  m_packagesTableWidget->setMouseTracking( true );

  m_currentUpdatedProfile = -1;
  m_updateTimer = new QTimer( this );
  m_updateTimer->setSingleShot( true );

  // content downloaders
  m_plainContentDownloader = new PlainRepositoryContentDownloader( this );
  m_yumContentDownloader = new YumRepositoryContentDownloader( this );

  m_loading = false;

  clearAll();
  connect( m_profilesTableWidget, SIGNAL( currentCellChanged( int, int, int, int ) ), this, SLOT( clearAndInsertPackagesTable( int, int, int, int ) ) );
  connect( m_profilesTableWidget, SIGNAL( cellDoubleClicked( int, int ) ), this, SLOT( profileDoubleClicked( int, int ) ) );
  connect( m_packagesTableWidget, SIGNAL( cellDoubleClicked( int, int ) ), this, SLOT( packageDoubleClicked( int, int ) ) );
  connect( m_packagesTableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( rpmsTableSelectionChanged() ) );
  connect( m_profilesTableWidget, SIGNAL( itemSelectionChanged() ), this, SLOT( profilesTableWidgetSelectionChanged() ) );

  // connect content downloader signals
  connect( m_plainContentDownloader, SIGNAL( finished( int, bool ) ), this, SLOT( contentDownloaderFinished( int, bool ) ) );
  connect( m_yumContentDownloader, SIGNAL( finished( int, bool ) ), this, SLOT( contentDownloaderFinished( int, bool ) ) );

  connect( m_updateTimer, SIGNAL( timeout() ), this, SLOT( completeStatusUpdate() ) );

  m_updateTimer->start( rpmDownloaderSettings().updateInterval() );
}

void RpmDownloaderWidget::addActionToPackagesTable( QAction *action )
{
  m_packagesTableWidget->addAction( action );
}

void RpmDownloaderWidget::addActionToProfilesTable( QAction *action )
{
  m_profilesTableWidget->addAction( action );
}

QAction *RpmDownloaderWidget::addSeparatorToPackagesTable()
{
  QAction *separatorAction = new QAction( this );
  separatorAction->setSeparator( true );
  m_packagesTableWidget->addAction( separatorAction );
  return separatorAction;
}

QAction *RpmDownloaderWidget::addSeparatorToProfilesTable()
{
  QAction *separatorAction = new QAction( this );
  separatorAction->setSeparator( true );
  m_profilesTableWidget->addAction( separatorAction );
  return separatorAction;
}

void RpmDownloaderWidget::clearPackagesTable()
{
  m_packagesTableWidget->clear();
  m_packagesTableWidget->setRowCount( 0 );
  m_packagesTableWidget->setColumnCount( 0 );
  m_packagesTableWidget->setColumnCount( PackageColumns );

  m_packagesTableWidget->setHorizontalHeaderLabels( QStringList()
      << tr( "Download Status" )
      << tr( "RPM Name" )
      << tr( "Local Version" )
      << tr( "Available Version" ) );

  // add column size policy
  m_packagesTableWidget->resizeColumnsToContents();
  // rpmsTableWidget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  m_packagesTableWidget->horizontalHeader()->setStretchLastSection( true );
  m_packagesTableWidget->horizontalHeader()->resizeSection( 1, 100 );
  m_packagesTableWidget->horizontalHeader()->resizeSection( 2, 120 );
  m_packagesTableWidget->horizontalHeader()->setHighlightSections( false );
  m_packagesTableWidget->setAlternatingRowColors( true );

  m_packagesTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
}

void RpmDownloaderWidget::clearProfilesTable()
{
  m_profilesTableWidget->clear();
  m_profilesTableWidget->setRowCount( 0 );
  m_profilesTableWidget->setColumnCount( 0 );
  m_profilesTableWidget->setColumnCount( ProfileColumns );

  m_profilesTableWidget->setHorizontalHeaderLabels( QStringList()
      << tr( "Status" )
      << tr( "Profile Name" ) );

  // add coloumn size policy
  m_profilesTableWidget->resizeColumnsToContents();
  m_profilesTableWidget->horizontalHeader()->setSectionResizeMode( 1, QHeaderView::Stretch );
  m_profilesTableWidget->horizontalHeader()->setHighlightSections( false );
  m_profilesTableWidget->setAlternatingRowColors( true );

  m_profilesTableWidget->setSelectionBehavior( QAbstractItemView::SelectRows );
  m_profilesTableWidget->setSelectionMode( QAbstractItemView::SingleSelection );

  QList<RepositoryProfile *>::iterator profileIter;

  for ( profileIter = m_profiles.begin(); profileIter != m_profiles.end(); ++profileIter ) {
    disconnect( ( *profileIter ) );
  }

  // clear all profiles
  qDeleteAll( m_profiles.begin(), m_profiles.end() );  // delete explicit all elements in the list

  m_profiles.clear();
}

void RpmDownloaderWidget::clearAll()
{
  cancelStatusUpdate();

  restartUpdateTimer();
  clearProfilesTable();
  clearPackagesTable();
  emit( currentProfileHasPackages( false ) );
  emit( hasPackages( false ) );
}

void RpmDownloaderWidget::addProfile( RepositoryProfile *newProfile )
{
  int row = m_profilesTableWidget->rowCount();

  newProfile->setParent( this );

  if ( row + 1 >= MaxRows ) {
    delete newProfile;
    return; // accept no more rows
  }

  QTableWidgetItem *profileNameItem = new QTableWidgetItem;

  profileNameItem->setText( newProfile->profileName() );
  QTableWidgetItem *statusItem = new QTableWidgetItem;

  // items not editable
  profileNameItem->setFlags( statusItem->flags() &~ Qt::ItemIsEditable );
  statusItem->setFlags( statusItem->flags() &~ Qt::ItemIsEditable );

  // set icon
  statusItem->setIcon( QIcon( ":/images/unknown.png" ) );

  // insert a new row
  m_profilesTableWidget->setRowCount( row + 1 );
  m_profilesTableWidget->setItem( row, 0, statusItem );
  m_profilesTableWidget->setItem( row, 1, profileNameItem );
  m_profiles.push_back( newProfile );

  // set new item as selected
  m_profilesTableWidget->setCurrentCell( row, 1 );

  connect( newProfile, SIGNAL( statusRefreshed() ), this, SLOT( repositoryStatusUpdated() ) );
  emit( modified() );
  emit( gotProfiles( true ) );
  computeAndEmitStats();
}

void RpmDownloaderWidget::duplicateCurrentProfile()
{
  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 )
    return;

  RepositoryProfile *duplicatedProfile = new RepositoryProfile( this );

  QHash<QString, Package> packages = m_profiles.at( row )->selectedPackages();

  QHash<QString, Package>::const_iterator packageIter;

  for ( packageIter = packages.begin(); packageIter != packages.end(); ++packageIter ) {
    duplicatedProfile->addPackage( packageIter.value() );
  }

  duplicatedProfile->setArchitectures( m_profiles.at( row )->architecures() );

  duplicatedProfile->setDownloadDir( m_profiles.at( row )->downloadDir() );
  duplicatedProfile->setProfileName( m_profiles.at( row )->profileName(), false );   // don't rename directory
  duplicatedProfile->setServerUrl( m_profiles.at( row )->serverUrl() );
  duplicatedProfile->setRepoType( m_profiles.at( row )->repoType() );

  duplicatedProfile->clearStatus();
  addProfile( duplicatedProfile );
}

void RpmDownloaderWidget::profileForCurrentProfileChanged()
{
  int row = m_profilesTableWidget->currentRow();

  if ( m_currentUpdatedProfile == row ) { // interrupt udapte when profile changed
    cancelStatusUpdate();
  }

  if ( row < 0 ) // nothing selected
    return;


  m_profiles[row]->clearStatus();

  m_profilesTableWidget->item( row, 1 )->setText( m_profiles.at( row )->profileName() );

  updateProfileTableStatus();

  updatePackagesTable();

  emit( modified() );
}

void RpmDownloaderWidget::deleteCurrentItemsFromActiveTable()
{
  int row = m_profilesTableWidget->currentRow();

  if ( m_profilesTableWidget->hasFocus() && row >= 0 ) {
    // delete current profile
    if ( !confirmProfileDelete( m_profiles.at( row )->profileName() ) )   // to avoid removing profiles by mistake
      return;

    if ( m_currentUpdatedProfile == row ) {
      cancelStatusUpdate();

    } else if ( m_currentUpdatedProfile > row ) {
      --m_currentUpdatedProfile;

      if ( m_profiles.at( m_currentUpdatedProfile )->repoType() == YUM )  // correct profile number
        m_yumContentDownloader->changeCurrentProfileNumber( m_currentUpdatedProfile );
      else
        m_plainContentDownloader->changeCurrentProfileNumber( m_currentUpdatedProfile );
    }

    // RDDatabaseHandler::removeDbConnection(profiles.at(row)->profileName());

    m_profilesTableWidget->removeRow( row );

    // disconenct signals and destroy object
    RepositoryProfile *profile = m_profiles.at( row );

    disconnect( profile );

    profile->removeCacheDir();

    delete profile;

    // remove profile from list
    m_profiles.removeAt( row );

    // select current row
    if ( row > 0 && row < m_profilesTableWidget->rowCount() )
      m_profilesTableWidget->setCurrentCell( row, 1 );
    else if ( row > 0 ) // removed row was last row
      m_profilesTableWidget->setCurrentCell( row - 1, 1 );
    else {
      emit( gotProfiles( false ) );
      clearPackagesTable();
    }

    emit( modified() );

    checkForPackagesInAllProfiles();

    clearAndInsertPackagesTable( m_profilesTableWidget->currentRow(), -1, -1, -1 );
    profilesTableWidgetSelectionChanged();

  } else if ( m_packagesTableWidget->hasFocus() && row >= 0 ) {
    QList<QTableWidgetItem *> selectedPackageItems = m_packagesTableWidget->selectedItems();

    int packageRow = m_packagesTableWidget->currentRow();

    if ( packageRow < 0 ) // nothing selected
      return;

    QList<int> rowsToDelete; // collect rows shich should be deleted tu avoid counter conflicts

    foreach ( QTableWidgetItem * item, selectedPackageItems ) {
      if ( item->column() == 1 ) {
        int selectedRow = item->row();
        m_profiles[row]->removePackage( m_packagesTableWidget->item( selectedRow, 1 )->text() );
        rowsToDelete.push_back( selectedRow );  // mark for delete
      }
    }

    qSort( rowsToDelete );

    // now delete uneeded rows

    for ( int i = 0; i < rowsToDelete.size(); ++i ) {
      m_packagesTableWidget->removeRow( rowsToDelete.at( i ) - i );
    }

    if ( packageRow >= 0 && packageRow < m_packagesTableWidget->rowCount() )
      m_packagesTableWidget->setCurrentCell( packageRow, 1 );
    else if ( packageRow > 0 ) // removed row was last row
      m_packagesTableWidget->setCurrentCell( packageRow - rowsToDelete.size(), 1 );

    m_profiles[row]->computeProfileStatus();

    // update profile status display
    updateProfileTableStatus();

    emit( modified() );

    checkForPackagesInAllProfiles();
  }

  computeAndEmitStats();
}

bool RpmDownloaderWidget::confirmProfileDelete( const QString &profileName )
{
  int r = QMessageBox::warning( this, tr( "RPM Downloader" ),
                                tr( "Are you sure that you want to delete the \"%1\" profile?" ).arg( profileName ),
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
  QApplication::setOverrideCursor( Qt::BusyCursor );

  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 ) // no profile selceted
    return;

  QList<QTableWidgetItem *> selectedPackageItems = m_packagesTableWidget->selectedItems();

  QProgressDialog progress( tr( "removing packages please wait ..." ), tr( "Abort" ), 0, selectedPackageItems.size() / 4.0, this );    // always four per column represents one package

  progress.setWindowModality( Qt::WindowModal );

  int packageCounter = 0;

  foreach ( QTableWidgetItem * item, selectedPackageItems ) {
    if ( item->column() == 1 ) {
      qApp->processEvents();

      if ( progress.wasCanceled() ) {
        break;
      }

      m_profiles[row]->deletePackageFromDisk( item->text() );

      progress.setValue( ++packageCounter );
    }
  }

  m_profiles[row]->computeProfileStatus(); // only compute once

  updatePackagesTable();
  updateProfileTableStatus();
  computeAndEmitStats();

  QApplication::restoreOverrideCursor();
}

void RpmDownloaderWidget::deleteAllPackagesFromDisk()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );

  QProgressDialog progress( tr( "removing packages please wait ..." ), tr( "Abort" ), 0, m_profiles.size(), this );
  progress.setWindowModality( Qt::WindowModal );

  for ( int i = 0; i < m_profiles.size(); ++i ) {
    qApp->processEvents();

    if ( progress.wasCanceled() ) {
      break;
    }

    m_profiles[i]->deleteAllPackagesFromDisk();

    progress.setValue( i + 1 );
  }

  updatePackagesTable();

  updateProfileTableStatus();
  computeAndEmitStats();
  QApplication::restoreOverrideCursor();
}

void RpmDownloaderWidget::copySelectedPackages()
{
  QString str;

  QList<QTableWidgetItem *> selectedPackageItems = m_packagesTableWidget->selectedItems();
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

  QApplication::clipboard()->setText( str );
}

void RpmDownloaderWidget::cutSelectedPackages()
{
  copySelectedPackages();
  deleteCurrentItemsFromActiveTable();
}

void RpmDownloaderWidget::insertPackagesFromClipboard()
{
  QString str = QApplication::clipboard()->text();
  str.replace( "\n", " " );
  str.replace( "\t", " " );
  QStringList packageNames = str.split( " " );

  foreach ( const QString & packageName, packageNames ) {
    if ( !packageName.isEmpty() ) {
      Package package( packageName );
      addPackageToCurProfile( package );
    }
  }
}

void RpmDownloaderWidget::profileDebugOutput()
{
  for ( int i = 0; i < m_profiles.size(); ++i ) {
    qDebug( "Profile at %i name %s", i, qPrintable( m_profiles.at( i )->profileName() ) );
  }
}

void RpmDownloaderWidget::addPackageToCurProfileWithDeps( const Package &package )
{
  if ( m_profilesTableWidget->rowCount() <= 0 ) // nothing to do
    return;

  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 ) // should not happen
    return;

  if ( !m_profiles.at( row )->repoType() == YUM || m_currentUpdatedProfile == row ) {
    // only yum can resolve deps
    // or database is incomplete because it's currently updated then
    // add rpm as normal
    addPackageToCurProfile( package );
    return;
  }

  // make busy cursor
  QApplication::setOverrideCursor( Qt::BusyCursor );

  emit( isBusy( true ) );

  QProgressDialog progress( tr( "Solving Dependencies..." ), tr( "Abort" ), 0, 300, this );

  progress.setWindowModality( Qt::WindowModal );

  YumDepSolver depSolver;

  if ( !depSolver.resolveSatFor( package.packageName(), rpmDownloaderSettings().cacheDir().absolutePath() + "/" + m_profiles.at( row )->profileName() + " " + m_profiles.at( row )->profileName() + ".db", m_profiles.at( row )->profileName(), progress, m_profiles.at( row )->architecures(), rpmDownloaderSettings().useMemDbSatSolving() ) ) {
    // could not resolve deps
    // unbusy
    QApplication::restoreOverrideCursor();
    emit( isBusy( false ) );
    addPackageToCurProfile( package );
    return;
  }

  QStringList requiredRpms = depSolver.getRpms();

  QStringListIterator i( requiredRpms );

  while ( i.hasNext() ) {
    Package newPackage;
    newPackage.setPackageName( i.next() );
    addPackageToCurProfile( newPackage );
  }

  emit( isBusy( false ) );

  QApplication::restoreOverrideCursor();

}

void RpmDownloaderWidget::addPackageToCurProfile( const Package &package )
{
  if ( m_profilesTableWidget->rowCount() <= 0 ) // nothing to do
    return;

  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 ) // should not happen
    return;

  if ( m_profiles.at( row )->hasPackage( package.packageName() ) )
    return;

  if ( m_profiles.at( row )->numberOfSelectedPackages() + 1 > MaxRows ) {
    // no more packages accepted
    qDebug( "maximum number of packages reached" );
    return;
  }

  m_profiles[row]->addPackage( package );

  // apply filter if needed

  if ( !m_lastFilterString.isEmpty() )
    applyPackageFilter( m_lastFilterString );
  else
    addPackageToPackagesTableWidget( package.packageName() );

  emit( modified() );

  emit( hasPackages( true ) );

  emit( currentProfileHasPackages( true ) );

  updatePackagesTable();

  updateProfileTableStatus(); // update the status display

  computeAndEmitStats();
}

void RpmDownloaderWidget::addPackageToPackagesTableWidget( const QString &packageName )
{
  int packageRow = m_packagesTableWidget->rowCount();

  m_packagesTableWidget->setRowCount( packageRow + 1 );

  // add items

  for ( int i = 0; i < PackageColumns; ++i ) {
    QTableWidgetItem *item = new QTableWidgetItem;

    if ( i == 1 )
      item->setText( packageName );

    addItemToPackagesTableWidget( item, packageRow, i );
  }

  m_packagesTableWidget->setCurrentCell( packageRow, 1 );

}

void RpmDownloaderWidget::addItemToPackagesTableWidget( QTableWidgetItem *item, const int row, const int column )
{
  // items not editable
  item->setFlags( item->flags() &~ Qt::ItemIsEditable );
  item->setFlags( item->flags() &~ Qt::ItemIsEditable );
  m_packagesTableWidget->setItem( row, column, item );
}

void RpmDownloaderWidget::applyPackageFilter( const QString &filterText )
{
  clearPackagesTable();
  m_lastFilterString = filterText;
  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 ) // nothing to do
    return;

  QApplication::setOverrideCursor( Qt::BusyCursor );

  QStringList packageNames = m_profiles.at( row )->selectedPackages().keys();

  QStringList::const_iterator packageNamesIter;

  const QStringList::const_iterator end = packageNames.end();

  for ( packageNamesIter = packageNames.begin(); packageNamesIter != end; ++packageNamesIter ) {
    if ( m_lastFilterString.isEmpty() || packageNamesIter->startsWith( m_lastFilterString ) ) {  // add package
      addPackageToPackagesTableWidget( *packageNamesIter );
    }
  }

  QApplication::restoreOverrideCursor();

  // add the status icons
  updatePackagesTable();
}

void RpmDownloaderWidget::resolveDependenciesForCurrentSelectedPackage()
{
  QList<QTableWidgetItem *> selectedItems = m_packagesTableWidget->selectedItems();

  if ( m_packagesTableWidget->rowCount() <= 0 || selectedItems.size() != 4 ) // not exactly one row selected
    return;

  QListIterator<QTableWidgetItem *> i( selectedItems );

  while ( i.hasNext() ) {
    QTableWidgetItem *item = i.next();

    if ( item->column() == 1 ) {
      addPackageToCurProfileWithDeps( Package( item->text() ) );
      break; // only interested in column 1
    }
  }
}

QIcon RpmDownloaderWidget::iconForStatus( Status status )
{
  switch ( status ) {

    case UNKNOWN:
      return QIcon( ":/images/unknown.png" );
      break;

    case OK:
      return QIcon( ":/images/ok.png" );
      break;

    case FAILED:
      return QIcon( ":/images/notOk.png" );
      break;

    case AVAILABLE:
      return QIcon( ":/images/avail.png" );
      break;

    case UPDATE:
      return QIcon( ":/images/newVersion.png" );
      break;

    case LOCALAVAIL:
      return QIcon( ":/images/localAvail.png" );
      break;
  }

  return QIcon( ":/images/unknown.png" );  // default return unknown
}

void RpmDownloaderWidget::updatePackagesTable()
{
  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 )
    return;

  if ( !m_loading )
    QApplication::setOverrideCursor( Qt::BusyCursor );

  for ( int i = 0; i < m_packagesTableWidget->rowCount(); ++i ) {
    QTableWidgetItem *packageNameItem = m_packagesTableWidget->item( i, 1 );
    PackageVersions localVersions = m_profiles.at( row )->getLocalPackageVersions( packageNameItem->text() );
    PackageVersions remoteVersions = m_profiles.at( row )->getRemotePackageVersions( packageNameItem->text() );

    for ( int j = 0; j < PackageColumns; ++j ) {
      QTableWidgetItem *item = m_packagesTableWidget->item( i, j );

      if ( j == 0 ) {
        item->setIcon( iconForStatus( m_profiles.at( row )->getPackageStatus( packageNameItem->text() ) ) );

      } else if ( j == 2 ) {
        item->setText( formatPackageVersionsForDisplay( localVersions ) );
        item->setToolTip( formatPackageVersionsForDisplay( localVersions, true ) );

      } else if ( j == 3 ) {
        item->setText( formatPackageVersionsForDisplay( remoteVersions ) );
        item->setToolTip( formatPackageVersionsForDisplay( remoteVersions, true ) );
      }
    }
  }

  if ( !m_loading )
    QApplication::restoreOverrideCursor();
}

QString RpmDownloaderWidget::formatPackageVersionsForDisplay( const PackageVersions &versions, const bool multipleLines ) const
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
        int index = versionsIter.value().lastIndexOf( formated );

        if ( index == versionsIter.value().size() - 1 ) {
          formated = versionsIter.value();

        } else {
          formated = versionsIter.value().left( index ) + "...";
        }
      }
    }
  }

  return formated;
}

void RpmDownloaderWidget::updateProfileTableStatus()
{
  for ( int i = 0; i < m_profiles.size(); ++i ) {
    // profiles[i]->computeProfileStatus(); // compute overall status
    m_profilesTableWidget->item( i, 0 )->setIcon( iconForStatus( m_profiles.at( i )->profileStatus() ) );
  }
}

void RpmDownloaderWidget::clearAndInsertPackagesTable( int row, int column, int oldRow, int oldColumn )
{
  Q_UNUSED( column );
  Q_UNUSED( oldColumn );

  if ( row == oldRow ) // no update needed
    return;

  if ( row < 0 )
    return;

  clearPackagesTable();

  QStringList packageNames = m_profiles.at( row )->selectedPackages().keys();

  if ( packageNames.size() >= MaxRows ) // error
    return;

  if ( !m_loading )
    QApplication::setOverrideCursor( Qt::BusyCursor );

  if ( m_lastFilterString.isEmpty() ) { // insert packages
    QStringList::const_iterator packageNameIter;

    for ( packageNameIter = packageNames.begin(); packageNameIter != packageNames.end(); ++packageNameIter ) {
      addPackageToPackagesTableWidget( *packageNameIter );
    }

  } else { // let the filter insert the packages
    applyPackageFilter( m_lastFilterString );
  }

  if ( !m_loading )
    QApplication::restoreOverrideCursor();

  m_packagesTableWidget->setCurrentCell( m_packagesTableWidget->rowCount() - 1, 1 );

  updatePackagesTable();
}

void RpmDownloaderWidget::profileDoubleClicked( int row, int column )
{
  Q_UNUSED( column );
  emit( repositoryProfileDoubleClicked( m_profiles.at( row ) ) );
}

void RpmDownloaderWidget::packageDoubleClicked( int row, int column )
{
  Q_UNUSED( column );
  emit( packageDoubleClicked( m_packagesTableWidget->item( row, 1 )->text(), m_packagesTableWidget->currentRow() ) );
}

bool RpmDownloaderWidget::saveToFile( const QString &fileName )
{
  QFile file( fileName );

  if ( !file.open( QIODevice::WriteOnly ) ) {
    QMessageBox::warning( this, tr( "RPM Downloader" ),
                          tr( "Cannot write file %1:\n%2." )
                          .arg( file.fileName() )
                          .arg( file.errorString() ) );
    return false;
  }

  QDataStream out( &file );

  out.setVersion( QDataStream::Qt_4_3 );

  // write magic number
  out << quint32( MagicNumber );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  for ( int i = 0; i < m_profiles.size(); ++i ) {
    QHashIterator<QString, Package> packageIter( m_profiles.at( i )->selectedPackages() );
    bool added = false;

    while ( packageIter.hasNext() ) {
      packageIter.next();
      out << static_cast<quint16>( i ) << m_profiles.at( i )->profileName()
          << m_profiles.at( i )->serverUrl().toString()
          << m_profiles.at( i )->downloadDir()
          << static_cast<quint16>( m_profiles.at( i )->architecures() )
          << static_cast<quint16>( m_profiles.at( i )->repoType() )
          << packageIter.value().packageName()
          << false; // its not a fake rpm
      added = true;
    }

    if ( !added ) { // force profile add
      out << static_cast<quint16>( i ) << m_profiles.at( i )->profileName()
          << m_profiles.at( i )->serverUrl().toString()
          << m_profiles.at( i )->downloadDir()
          << static_cast<quint16>( m_profiles.at( i )->architecures() )
          << static_cast<quint16>( m_profiles.at( i )->repoType() )
          << "Fake"
          << true;
    }
  }

  QApplication::restoreOverrideCursor();

  file.close();
  return true;
}

bool RpmDownloaderWidget::loadProfilesFile( const QString &fileName )
{
  cancelStatusUpdate();
  m_updateTimer->stop(); // stop timer

  QFile file( fileName );

  if ( !file.open( QIODevice::ReadOnly ) ) {
    QMessageBox::warning( this, tr( "RPM Downloader" ),
                          tr( "Cannot read file %1:\n%2." )
                          .arg( file.fileName() )
                          .arg( file.errorString() ) );
    return false;
  }

  QDataStream in( &file );

  in.setVersion( QDataStream::Qt_4_3 );

  quint32 magic;
  in >> magic;

  if ( magic != MagicNumber && magic != MagicNumberOld ) {
    QMessageBox::warning( this, tr( "RPM Downloader" ),
                          tr( "The selected file is not a RPM Downloader file." ) );
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

  m_loading = true;
  QApplication::setOverrideCursor( Qt::WaitCursor );

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

    if ( pos == m_profiles.size() && !profileName.isEmpty() ) {
      RepositoryProfile *profile = new RepositoryProfile( this );
      profile->setProfileName( profileName, false );  // don't rename directory and database
      profile->setServerUrl( serverUrl );
      profile->setDownloadDir( downloadDir );
      profile->setArchitectures( static_cast<RPM::Architectures>( architectures ) );
      profile->setRepoType( static_cast<RepositoryType>( repoType ) );
      addProfile( profile );
    }

    if ( !fakeRpm && !packageName.isEmpty() ) {
      Package package( packageName );
      addPackageToCurProfile( package );
    }
  }

  QApplication::restoreOverrideCursor();

  m_loading = false;

  // completeRpmStatusUpdate();
  m_updateTimer->start( 3000 );  // avoid signal race condition DON'T CALL completeRpmStatusUpdate without timer!!!
  return true;
}

RpmDownloaderWidget::~RpmDownloaderWidget()
{
  RepositoryProfile::clearDirectory( rpmDownloaderSettings().tempDir().absolutePath() );
}

void RpmDownloaderWidget::completeStatusUpdate( bool showProgress )
{
  if ( m_profiles.size() < 1 ) // no profiles
    return;

  if ( showProgress ) {
    if ( !m_statusUpdateProgressDialog ) {
      m_statusUpdateProgressDialog = new QProgressDialog( tr( "Status update in progress ..." ), tr( "Cancel" ), 0, m_profiles.size(), this );
      connect( m_statusUpdateProgressDialog, SIGNAL( canceled() ), this, SLOT( cancelStatusUpdate() ) );

    } else {
      m_statusUpdateProgressDialog->setMaximum( m_profiles.size() );
    }

    m_statusUpdateProgressDialog->setVisible( true );

    m_statusUpdateProgressDialog->setValue( 0 );
    QApplication::setOverrideCursor( Qt::BusyCursor );
    emit( isBusy( true ) );
  }

  if ( m_updateTimer->isActive() ) // stop update timer when running
    m_updateTimer->stop();

  if ( m_currentUpdatedProfile >= 0 && showProgress ) {
    m_statusUpdateProgressDialog->setValue( m_currentUpdatedProfile );
    return;
  }

  m_currentUpdatedProfile = -1;

  updateNextProfile();

}

void RpmDownloaderWidget::updateNextProfile()
{
  if ( m_currentUpdatedProfile + 1 < m_profiles.size() ) {

    ++m_currentUpdatedProfile;

    if ( m_statusUpdateProgressDialog ) {
      if ( !m_statusUpdateProgressDialog->isHidden() )
        m_statusUpdateProgressDialog->setValue( m_currentUpdatedProfile );  // not hidden
    }

    // initiate repository contents update
    QString serverUrl = m_profiles.at( m_currentUpdatedProfile )->serverUrl().toString();

    // differnet update methods for different repository types
    if ( m_profiles.at( m_currentUpdatedProfile )->repoType() == YUM ) {
      // start yum content update
      m_yumContentDownloader->startContentUpdate( m_currentUpdatedProfile, m_profiles.at( m_currentUpdatedProfile )->databaseFile(), m_profiles.at( m_currentUpdatedProfile )->profileName(), serverUrl, PackageMetaData::archStringList( m_profiles.at( m_currentUpdatedProfile )->architecures() ) );

    } else {
      m_plainContentDownloader->startContentUpdate( m_currentUpdatedProfile, m_profiles.at( m_currentUpdatedProfile )->databaseFile(), m_profiles.at( m_currentUpdatedProfile )->profileName(), serverUrl, PackageMetaData::archStringList( m_profiles.at( m_currentUpdatedProfile )->architecures() ) );
    }

  } else {
    if ( m_statusUpdateProgressDialog ) {
      if ( !m_statusUpdateProgressDialog->isHidden() )
        m_statusUpdateProgressDialog->setValue( m_currentUpdatedProfile + 1 );  // should hide now

      QApplication::restoreOverrideCursor(); // reset cursor

      emit( isBusy( false ) );
    }

    m_currentUpdatedProfile = -1;

    m_updateTimer->start( rpmDownloaderSettings().updateInterval() );
  }
}

void RpmDownloaderWidget::repositoryStatusUpdated()
{
  QApplication::setOverrideCursor( Qt::WaitCursor );
  updateProfileTableStatus();
  updatePackagesTable();
  computeAndEmitStats();
  QApplication::restoreOverrideCursor(); // reset cursor
}

void RpmDownloaderWidget::changePackageName( const QString &oldName, const QString &newName, const int rpmsTableRow )
{
  m_profiles[m_profilesTableWidget->currentRow()]->changePackageName( oldName, newName );
  m_packagesTableWidget->item( rpmsTableRow, 1 )->setText( newName );
  emit( modified() );
}

void RpmDownloaderWidget::changePackageName( const QString &oldName, const QString &newName )
{
  changePackageName( oldName, newName, m_packagesTableWidget->currentRow() );
}

void RpmDownloaderWidget::contentDownloaderFinished( int profileNumber, bool error )
{
  if ( profileNumber != m_currentUpdatedProfile ) {
    QMessageBox::critical( this, tr( "RPM Downloader" ),
                           tr( "Something went wrong, this should not happen but the updated profile doesn't match to the profile where contents was updated" ), QMessageBox::Ok | QMessageBox::Default );
    updateNextProfile();
    return;
  }

  if ( error ) {
    QString errMsg;

    if ( m_profiles.at( profileNumber )->repoType() == YUM )
      errMsg = m_yumContentDownloader->readError();
    else
      errMsg = m_plainContentDownloader->readError();

    QMessageBox::critical( this, tr( "RPM Downloader" ),
                           tr( "Content update failed %1" ).arg( errMsg ), QMessageBox::Ok | QMessageBox::Default );

    updateNextProfile();

    return;
  }

  if ( profileNumber < m_profiles.size() && profileNumber >= 0 ) {
    m_profiles[profileNumber]->refreshStatus();
    updateNextProfile();
  }
}

void RpmDownloaderWidget::downloadPackagesForCurrentProfile()
{
  downloadPackages( false );
}

void RpmDownloaderWidget::downloadPackagesForAllProfiles()
{
  downloadPackages( true );
}

void RpmDownloaderWidget::downloadPackages( bool all )
{
  m_updateTimer->stop();
  DownloadProgressDialog dialog( this );
  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 ) {
    emit( downloadFinished() );
    return;
  }

  int numberOfRpms = computeNumberOfDownloadRpms( all );

  if ( all ) {
    dialog.setProfilesForDownload( m_profiles );

  } else {
    dialog.setProfilesForDownload( m_profiles.at( row ) );
  }

  if ( numberOfRpms <= 0 ) {
    emit( downloadFinished() );
    return;
  }

  dialog.setDeleteOldVersion( rpmDownloaderSettings().deleteOldVersions() );

  dialog.setNumberOfRpms( numberOfRpms );

  if ( !dialog.exec() ) {
    QString errorString = dialog.getError();
    QMessageBox::critical( this, tr( "RPM Downloader" ), errorString, QMessageBox::Ok | QMessageBox::Default );
    completeStatusUpdate( true );

  } else {
    if ( all ) {
      QList<RepositoryProfile *>::iterator profileIter;

      for ( profileIter = m_profiles.begin(); profileIter != m_profiles.end(); ++profileIter ) {
        ( *profileIter )->packagesUpdated( rpmDownloaderSettings().deleteOldVersions() );
      }

    } else {
      m_profiles[row]->packagesUpdated( rpmDownloaderSettings().deleteOldVersions() );
    }
  }

  emit( downloadFinished() );

  computeAndEmitStats();

  // status update not necessary all is fine only need to update the display tables
  updatePackagesTable();
  updateProfileTableStatus();
}

int RpmDownloaderWidget::computeNumberOfDownloadRpms( bool all )
{
  if ( !all ) {
    int row = m_profilesTableWidget->currentRow();

    if ( row < 0 )
      return 0;

    return m_profiles.at( row )->numberOfPackagesToDownload();

  } else {
    int numberOfRpms = 0;

    QListIterator<RepositoryProfile *> profileIter( m_profiles );

    while ( profileIter.hasNext() ) {
      numberOfRpms += profileIter.next()->numberOfPackagesToDownload();
    }

    return numberOfRpms;
  }

  return 0;
}

void RpmDownloaderWidget::restartUpdateTimer()
{
  if ( m_updateTimer->isActive() )
    m_updateTimer->stop();

  m_currentUpdatedProfile = -1;

  m_updateTimer->start( rpmDownloaderSettings().updateInterval() );
}

void RpmDownloaderWidget::cancelStatusUpdate()
{
  if ( m_statusUpdateProgressDialog ) {
    if ( !m_statusUpdateProgressDialog->isHidden() )
      m_statusUpdateProgressDialog->setValue( m_profiles.size() );
  }

  if ( m_currentUpdatedProfile < 0 )
    return; // no update in progress

  if ( m_profiles.at( m_currentUpdatedProfile )->repoType() == YUM )
    m_yumContentDownloader->cancelContentUpdate();
  else
    m_plainContentDownloader->cancelContentUpdate();

  m_currentUpdatedProfile = -1;

  restartUpdateTimer();

  QApplication::restoreOverrideCursor(); // reset cursor

  emit( isBusy( false ) );
}

void RpmDownloaderWidget::rpmsTableSelectionChanged()
{
  if ( m_packagesTableWidget->selectedItems().size() > 4 ) // because one row has 4 items
    emit( hasPackageSelection( true, true ) );
  else if ( m_packagesTableWidget->selectedItems().size() == 4 )
    emit( hasPackageSelection( true, false ) );
  else
    emit( hasPackageSelection( false, false ) );
}

void RpmDownloaderWidget::profilesTableWidgetSelectionChanged()
{
  if ( m_profilesTableWidget->selectedItems().size() > 0 ) {
    emit( hasProfileSelection( true ) );

    if ( m_profiles.at( m_profilesTableWidget->currentRow() )->selectedPackages().size() > 0 ) {
      emit( currentProfileHasPackages( true ) );

    } else {
      emit( currentProfileHasPackages( false ) );
    }

  } else {
    emit( hasProfileSelection( false ) );
    emit( currentProfileHasPackages( false ) );
  }
}

void RpmDownloaderWidget::applyNewSettings()
{
  cancelStatusUpdate();

  if ( m_updateTimer->isActive() ) {
    m_updateTimer->stop();
    m_updateTimer->start( rpmDownloaderSettings().updateInterval() );
  }
}

QStringList RpmDownloaderWidget::getRepositoryContentsForCurrentRepo() const
{
  if ( m_profilesTableWidget->currentRow() >= 0 )
    return m_profiles.at( m_profilesTableWidget->currentRow() )->getProvidedPackages();

  return QStringList();
}

RepositoryProfile *RpmDownloaderWidget::getCurrentRepositoryProfile()
{
  if ( m_profilesTableWidget->currentRow() >= 0 )
    return m_profiles.at( m_profilesTableWidget->currentRow() );

  return NULL;
}

QString RpmDownloaderWidget::getCurrentPackageName() const
{
  if ( m_packagesTableWidget->currentRow() >= 0 )
    return m_packagesTableWidget->item( m_packagesTableWidget->currentRow(), 1 ) ->text();

  return QString();
}

void RpmDownloaderWidget::checkForPackagesInAllProfiles()
{
  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 ) { // got no profiles
    emit( currentProfileHasPackages( false ) );
    emit( hasPackages( false ) );
    return;
  }

  // check for rpms in the current profile

  if ( m_profiles.at( row )->selectedPackages().size() < 1 ) {
    emit( currentProfileHasPackages( false ) );

    // search for rpms in other profiles
    bool repositoriesHasPackagess = false;

    for ( int i = 0; i < m_profiles.size(); ++i ) {
      if ( i == row ) // no need to search act row
        continue;

      if ( m_profiles.at( i )->selectedPackages().size() > 0 ) {
        repositoriesHasPackagess = true;
        break;
      }
    }

    emit( hasPackages( repositoriesHasPackagess ) );
  }
}

void RpmDownloaderWidget::computeAndEmitStats()
{
  int numberOfProfiles, numberOfRpms, okPackages, failedPackages, updatedPackages, localAvailPackages, availPackages, unknownPackages;

  qint64 downloadSize = 0;

  numberOfProfiles = m_profiles.size();
  numberOfRpms = okPackages = failedPackages = updatedPackages = localAvailPackages = availPackages = unknownPackages = 0;

  QList<RepositoryProfile *>::const_iterator profileIterator;

  for ( profileIterator = m_profiles.begin(); profileIterator != m_profiles.end(); ++profileIterator ) {
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

  emit( profileStats( numberOfProfiles, numberOfRpms, okPackages, failedPackages, updatedPackages, localAvailPackages, availPackages, unknownPackages, PackageMetaData::sizeAsString( downloadSize ) ) );
}

void RpmDownloaderWidget::triggerComputeStats()
{
  computeAndEmitStats();
}

bool RpmDownloaderWidget::currentRepoSupportsDepSolving() const
{
  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 )
    return false;

  if ( m_profiles.at( row )->repoType() == YUM ) {
    return true;
  }

  return false;
}

void RpmDownloaderWidget::prepareDetailsView()
{
  int row = m_profilesTableWidget->currentRow();

  if ( row < 0 )
    return;

  QString currentPackageName = getCurrentPackageName();

  if ( currentPackageName.isEmpty() )
    return;

  emit( packageDetailsReady( m_profiles.at( row )->getPackageData( currentPackageName ), iconForStatus( m_profiles.at( row )->getPackageStatus( currentPackageName ) ) ) );
}

void RpmDownloaderWidget::clearCacheDirectory()
{
  QDir cacheDir( rpmDownloaderSettings().cacheDir() );
  cacheDir.setFilter( QDir::Dirs );

  foreach ( const QString & subDir, cacheDir.entryList() ) {
    if ( subDir != "." && subDir != ".." ) {
      RepositoryProfile::clearDirectory( cacheDir.absolutePath() + "/" + subDir );
      cacheDir.rmdir( subDir );
    }
  }
}

void RpmDownloaderWidget::cleanOrphanedPackageEntries()
{
  int removedOrphanedPackages = 0;

  foreach ( RepositoryProfile * profile, m_profiles ) {
    removedOrphanedPackages += profile->cleanupOrphanedPackages();
  }

  int row = m_profilesTableWidget->currentRow();

  if ( row < 1 )
    return;

  if ( removedOrphanedPackages > 0 )
    emit( modified() );

  clearAndInsertPackagesTable( row, 0, -1, 0 );  // force redraw of packages
  computeAndEmitStats();
}


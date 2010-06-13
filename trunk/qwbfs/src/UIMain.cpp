/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : UIMain.cpp
** Date      : 2010-04-25T13:05:33
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a crossplatform WBFS Manager developed using Qt4/C++.
** It's currently working under Unix/Linux, Mac OS X, and build under windows (but not yet working).
** 
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR WII CONSOLE OR WII PARTITION
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "UIMain.h"
#include "UIAbout.h"
#include "models/DiscModel.h"
#include "models/DiscDelegate.h"
#include "wiitdb/Covers.h"
#include "datacache/DataNetworkCache.h"
#include "ProgressDialog.h"
#include "PropertiesDialog.h"
#include "Properties.h"

#include <QFileSystemModel>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QDebug>

UIMain::UIMain( QWidget* parent )
	: QMainWindow( parent )
{
	mCache = new DataNetworkCache( this );
	
	setUnifiedTitleAndToolBarOnMac( true );
	setupUi( this );
	
	mFoldersModel = new QFileSystemModel( this );
	mFoldersModel->setFilter( QDir::Dirs | QDir::NoDotAndDotDot );
	
	mFilesModel = new QFileSystemModel( this );
	mFilesModel->setFilter( QDir::Files );

	tvFolders->setModel( mFoldersModel );
	tvFolders->setColumnHidden( 1, true );
	tvFolders->setColumnHidden( 2, true );
	tvFolders->setColumnHidden( 3, true );
	
	lvFiles->setModel( mFilesModel );
	
	mExportModel = new QWBFS::Model::DiscModel( this );
	
	lvExport->setModel( mExportModel );
	lvExport->setItemDelegate( new QWBFS::Model::DiscDelegate( mExportModel, mCache ) );
	
	mLastDiscId = -1;
	
	pwMainView->setMainView( true );
	pwMainView->showHideImportViewButton()->setChecked( false );
	connectView( pwMainView );
	
	aReloadPartitions->trigger();
	
	propertiesChanged();
	
	connect( mCache, SIGNAL( dataCached( const QUrl& ) ), this, SLOT( dataNetworkCache_dataCached( const QUrl& ) ) );
	connect( mCache, SIGNAL( error( const QString&, const QUrl& ) ), this, SLOT( dataNetworkCache_error( const QString&, const QUrl& ) ) );
	connect( mCache, SIGNAL( invalidated() ), this, SLOT( dataNetworkCache_invalidated() ) );
}

UIMain::~UIMain()
{
	//qWarning() << Q_FUNC_INFO;
}

DataNetworkCache* UIMain::cache() const
{
	return mCache;
}

QPixmap UIMain::cachedPixmap( const QUrl& url ) const
{
	const QByteArray* data = mCache->cachedData( url );
	QPixmap pixmap;
	
	if ( !data ) {
		return pixmap;
	}
	
	pixmap.loadFromData( *data );
	
	return pixmap;
}

void UIMain::showEvent( QShowEvent* event )
{
	QMainWindow::showEvent( event );
	
	static bool shown = false;
	
	if ( !shown ) {
		shown = true;
		Properties properties;
		properties.restoreState( this );
	}
}

void UIMain::closeEvent( QCloseEvent* event )
{
	Properties properties;
	properties.saveState( this );
	
	QMainWindow::closeEvent( event );
}

void UIMain::connectView( PartitionWidget* widget )
{
	connect( widget, SIGNAL( openViewRequested() ), this, SLOT( openViewRequested() ) );
	connect( widget, SIGNAL( closeViewRequested() ), this, SLOT( closeViewRequested() ) );
	connect( widget, SIGNAL( coverRequested( const QString& ) ), this, SLOT( coverRequested( const QString& ) ) );
}

void UIMain::propertiesChanged()
{
	Properties properties( this );
	
	mCache->setDiskCacheSize( properties.cacheDiskSize() );
	mCache->setMemoryCacheSize( properties.cacheMemorySize() );
	mCache->setWorkingPath( properties.cacheUseTemporaryPath() ? properties.temporaryPath() : properties.cacheWorkingPath() );
}

void UIMain::openViewRequested()
{
	PartitionWidget* pw = new PartitionWidget( this );
	pw->setMainView( false );
	pw->setPartitions( mPartitions );
	pw->showHideImportViewButton()->setChecked( false );
	connectView( pw );
	sViews->addWidget( pw );
}

void UIMain::closeViewRequested()
{
	sender()->deleteLater();
}

void UIMain::coverRequested( const QString& id )
{
	mLastDiscId = id;
	
	const QUrl urlCD = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Disc, id );
	const QUrl urlCDCustom = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::DiscCustom, id );
	const QUrl urlCover = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Cover, id );
	
	lCDCover->clear();
	lCover->clear();
	
	if ( mCache->hasCachedData( urlCD ) || mCache->hasCachedData( urlCDCustom ) || mCache->hasCachedData( urlCover ) ) {
		dataNetworkCache_dataCached( QUrl() );
	}
	
	if ( !lCDCover->pixmap() ) {
		mCache->cacheData( urlCD );
	}
	
	if ( !lCover->pixmap() ) {
		mCache->cacheData( urlCover );
	}
}

void UIMain::progress_jobFinished( const QWBFS::Model::Disc& disc )
{
	mExportModel->updateDisc( disc );
}

void UIMain::dataNetworkCache_dataCached( const QUrl& url )
{
	Q_UNUSED( url );
	
	// update all views
	const QList<QAbstractItemView*> views = findChildren<QAbstractItemView*>();
	
	foreach ( QAbstractItemView* view, views ) {
		view->viewport()->update();
	}
	
	// update preview dock
	if ( mLastDiscId.isEmpty() ) {
		return;
	}
	
	const QUrl urlCD = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Disc, mLastDiscId );
	const QUrl urlCDCustom = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::DiscCustom, mLastDiscId );
	const QUrl urlCover = QWBFS::WiiTDB::Covers::url( QWBFS::WiiTDB::Covers::Cover, mLastDiscId );
	
	if ( mCache->hasCachedData( urlCD ) ) {
		lCDCover->setPixmap( cachedPixmap( urlCD ) );
	}
	
	if ( mCache->hasCachedData( urlCDCustom ) ) {
		lCDCover->setPixmap( cachedPixmap( urlCDCustom ) );
	}
	
	if ( mCache->hasCachedData( urlCover ) ) {
		lCover->setPixmap( cachedPixmap( urlCover ) );
	}
}

void UIMain::dataNetworkCache_error( const QString& message, const QUrl& url )
{
	switch ( QWBFS::WiiTDB::Covers::type( url ) )
	{
		case QWBFS::WiiTDB::Covers::Disc:
			mCache->cacheData( QWBFS::WiiTDB::Covers( url ).url( QWBFS::WiiTDB::Covers::DiscCustom ) );
			return;
		case QWBFS::WiiTDB::Covers::HQ:
		case QWBFS::WiiTDB::Covers::Cover:
		case QWBFS::WiiTDB::Covers::_3D:
		case QWBFS::WiiTDB::Covers::DiscCustom:
		case QWBFS::WiiTDB::Covers::Full:
		case QWBFS::WiiTDB::Covers::Invalid:
			break;
	}
	
	QMessageBox::information( this, QString::null, message );
}

void UIMain::dataNetworkCache_invalidated()
{
	dataNetworkCache_dataCached( QUrl() );
}

void UIMain::on_aReloadPartitions_triggered()
{
	mPartitions.clear();

#if defined( Q_OS_WIN )
	foreach ( const QFileInfo& drive, QDir::drives() ) {
		mPartitions << drive.absoluteFilePath().remove( ":" ).remove( "/" ).remove( "\\" );
	}
	process.start( "diskutil list" );
	process.waitForFinished();
	
	const QStringList partitions = QString::fromLocal8Bit( process.readAll() ).split( "\n" );
		if ( partition.startsWith( "/" ) || partition.startsWith( "#" ) || partition.isEmpty() ) {
			continue;
		}
		
		partition = partition.simplified().section( ' ', -1 );
		
		// skip disks
		if ( partition[ partition.size() -2 ].toLower() != 's' ) {
			continue;
		}
		
		mPartitions << QString( "/dev/%1" ).arg( partition );
	}
#elif defined( __linux__ )
	QProcess process;
	process.start( "cat /proc/partitions" );
	process.waitForFinished();
	
	const QStringList partitions = QString::fromLocal8Bit( process.readAll() ).split( "\n" );
	
	foreach ( QString partition, partitions ) {
		if ( partition.startsWith( "major" ) || partition.isEmpty() ) {
			continue;
		}
		
		partition = partition.simplified().section( ' ', -1 );
		
		// skip disks
		if ( !partition[ partition.size() -1 ].isDigit() ) {
			continue;
		}
		
		mPartitions << QString( "/dev/%1" ).arg( partition );
	}
#else
	QMessageBox::information( this, QString::null,
		tr(
			"I don't know how to list partition for this platform.\n"
			"You will have to set the correct partition path yourself for mounting partitions."
		) );
#endif
	
	const QList<PartitionWidget*> widgets = sViews->findChildren<PartitionWidget*>();
	
	foreach ( PartitionWidget* widget, widgets ) {
		widget->setPartitions( mPartitions );
	}
}

void UIMain::on_aAbout_triggered()
{
	UIAbout* about = new UIAbout( this );
	about->open();
}

void UIMain::on_aProperties_triggered()
{
	PropertiesDialog* dlg = new PropertiesDialog( this );
	connect( dlg, SIGNAL( propertiesChanged() ), this, SLOT( propertiesChanged() ) );
	dlg->open();
}

void UIMain::on_tvFolders_activated( const QModelIndex& index )
{
	const QString filePath = mFoldersModel->filePath( index );
	mFilesModel->setRootPath( filePath );
	lvFiles->setRootIndex( mFilesModel->index( filePath ) );
}
	QStringList pathsToScan;
	pathsToScan << "/Volumes";
	pathsToScan  << "/media" << "/mnt";

	foreach ( const QString& path, pathsToScan ) {
		foreach ( const QFileInfo& fi, QDir( path ).entryInfoList( QDir::Dirs | QDir::NoDotAndDotDot ) ) {
	}
	mFoldersModel->setRootPath( text );
	on_tvFolders_activated( tvFolders->rootIndex() );

void UIMain::on_tbClearExport_clicked()
{
	mExportModel->clear();
}

void UIMain::on_tbRemoveExport_clicked()
{
	mExportModel->removeSelection( lvExport->selectionModel()->selection() );
}

void UIMain::on_tbExport_clicked()
{
	if ( mExportModel->rowCount() == 0 ) {
		return;
	}
	
	const QString path = QFileDialog::getExistingDirectory( this, tr( "Choose a folder to export the discs" ), QString::null );
	
	if ( path.isEmpty() ) {
		return;
	}
	
	ProgressDialog* dlg = new ProgressDialog( this );
	
	connect( dlg, SIGNAL( jobFinished( const QWBFS::Model::Disc& ) ), this, SLOT( progress_jobFinished( const QWBFS::Model::Disc& ) ) );
	
	dlg->exportDiscs( mExportModel->discs(), path );
}
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
#include "ProgressDialog.h"

#include <QFileSystemModel>
#include <QFileDialog>
#include <QProcess>
#include <QDebug>

UIMain::UIMain( QWidget* parent )
	: QMainWindow( parent )
{
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
	lvExport->setItemDelegate( new QWBFS::Model::DiscDelegate( mExportModel ) );
	
	pwMainView->setMainView( true );
	pwMainView->showHideImportViewButton()->setChecked( false );
	connectView( pwMainView );
	
	aReloadPartitions->trigger();
}

UIMain::~UIMain()
{
	//qWarning() << Q_FUNC_INFO;
}

void UIMain::connectView( PartitionWidget* widget )
{
	connect( widget, SIGNAL( openViewRequested() ), this, SLOT( openViewRequested() ) );
	connect( widget, SIGNAL( closeViewRequested() ), this, SLOT( closeViewRequested() ) );
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

void UIMain::progress_jobFinished( const QWBFS::Model::Disc& disc )
{
	mExportModel->updateDisc( disc );
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
#else
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

void UIMain::on_tvFolders_activated( const QModelIndex& index )
{
	const QString filePath = mFoldersModel->filePath( index );
	mFilesModel->setRootPath( filePath );
	lvFiles->setRootIndex( mFilesModel->index( filePath ) );
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
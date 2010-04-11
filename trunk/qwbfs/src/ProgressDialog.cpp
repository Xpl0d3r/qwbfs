#include "ProgressDialog.h"
#include "ExportThread.h"

#include <QPushButton>
#include <QDebug>

ProgressDialog::ProgressDialog( QWidget* parent )
	: QDialog( parent )
{
	mThread = 0;
	
	setupUi( this );
	setAttribute( Qt::WA_DeleteOnClose );
	cbDetails->setChecked( false );
	dbbButtons->button( QDialogButtonBox::Ok )->setEnabled( false );
	dbbButtons->button( QDialogButtonBox::Cancel )->setEnabled( false );
}

ProgressDialog::~ProgressDialog()
{
	qWarning() << Q_FUNC_INFO;
}

void ProgressDialog::closeEvent( QCloseEvent* event )
{
	if ( mThread && mThread->isRunning() ) {
		//mThread->stop();
		event->ignore();
		return;
	}
	
	QDialog::closeEvent( event );
}

void ProgressDialog::exportDiscs( const DiscList& discs, const QString& path )
{
	mThread = new ExportThread( this );
	
	connect( dbbButtons->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( close() ) );
	connect( dbbButtons->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), mThread, SLOT( stop() ) );
	connect( mThread, SIGNAL( started() ), this, SLOT( thread_started() ) );
	connect( mThread, SIGNAL( message( const QString& ) ), lCurrentInformations, SLOT( setText( const QString& ) ) );
	connect( mThread, SIGNAL( error( const QString& ) ), pteErrors, SLOT( appendPlainText( const QString& ) ) );
	connect( mThread, SIGNAL( currentProgressChanged( int, int, const QTime& ) ), this, SLOT( thread_currentProgressChanged( int, int, const QTime& ) ) );
	connect( mThread, SIGNAL( globalProgressChanged( int ) ), pbGlobal, SLOT( setValue( int ) ) );
	connect( mThread, SIGNAL( finished() ), this, SLOT( thread_finished() ) );
	
	setWindowTitle( tr( "Exporting discs..." ) );
	pbGlobal->setMaximum( discs.count() );
	open();
	
	if ( !mThread->exportDiscs( discs, path ) ) {
		deleteLater();
	}
}

void ProgressDialog::importDiscs( const DiscList& discs, const QString& partition )
{
	mThread = new ExportThread( this );
	
	connect( dbbButtons->button( QDialogButtonBox::Ok ), SIGNAL( clicked() ), this, SLOT( close() ) );
	connect( dbbButtons->button( QDialogButtonBox::Cancel ), SIGNAL( clicked() ), mThread, SLOT( stop() ) );
	connect( mThread, SIGNAL( started() ), this, SLOT( thread_started() ) );
	connect( mThread, SIGNAL( message( const QString& ) ), lCurrentInformations, SLOT( setText( const QString& ) ) );
	connect( mThread, SIGNAL( error( const QString& ) ), pteErrors, SLOT( appendPlainText( const QString& ) ) );
	connect( mThread, SIGNAL( currentProgressChanged( int, int, const QTime& ) ), this, SLOT( thread_currentProgressChanged( int, int, const QTime& ) ) );
	connect( mThread, SIGNAL( globalProgressChanged( int ) ), pbGlobal, SLOT( setValue( int ) ) );
	connect( mThread, SIGNAL( finished() ), this, SLOT( thread_finished() ) );
	
	setWindowTitle( tr( "Importing discs..." ) );
	pbGlobal->setMaximum( discs.count() );
	open();
	
	if ( !mThread->importDiscs( discs, partition ) ) {
		deleteLater();
	}
}

void ProgressDialog::thread_started()
{
	mElapsed.restart();
	dbbButtons->button( QDialogButtonBox::Cancel )->setEnabled( true );
}

void ProgressDialog::thread_currentProgressChanged( int value, int maximum, const QTime& remaining )
{
	pbCurrent->setMaximum( maximum );
	pbCurrent->setValue( value );
	lCurrentRemaining->setText( tr( "Time remaining: %1" ).arg( remaining.toString() ) );
}

void ProgressDialog::thread_finished()
{
	QTime time( 0, 0, 0, 0 );
	time = time.addMSecs( mElapsed.elapsed() );
	
	lCurrentInformations->setText( "Export finished." );
	lCurrentRemaining->clear();
	lGlobalInformations->setText( tr( "The thread finished in %1" ).arg( time.toString() ) );
	delete mThread;
	dbbButtons->button( QDialogButtonBox::Ok )->setEnabled( true );
	dbbButtons->button( QDialogButtonBox::Cancel )->setEnabled( false );
}
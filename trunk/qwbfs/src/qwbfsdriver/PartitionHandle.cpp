/****************************************************************************
**
** 		Created using Monkey Studio v1.8.4.0b2 (1.8.4.0b2)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : qwbfs
** FileName  : PartitionHandle.cpp
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
#include "PartitionHandle.h"

#include <QDebug>

using namespace QWBFS::Partition;
using namespace QWBFS::Partition::Internal;

// HandleData

HandleData::HandleData( const QWBFS::Partition::Properties& _properties )
{
	properties = _properties;
	handle = _properties.partition.isEmpty() ? 0 : wbfs_try_open_partition( properties.partition.toLocal8Bit().data(), properties.reset ? 1 : 0 );
	
	if ( handle ) {
		qWarning() << QString( "*** Opened partition: %1" ).arg( properties.partition ).toLocal8Bit().constData();
	}
}

HandleData::HandleData( const HandleData& other )
	: QSharedData( other )
{
	properties = other.properties;
	handle = other.handle;
}

HandleData::~HandleData()
{	
	if ( handle ) {
		wbfs_close( handle );
		qWarning() << QString( "*** Closed partition: %1" ).arg( properties.partition ).toLocal8Bit().constData();
	}
	
	//qWarning() << Q_FUNC_INFO;
}

// Handle

Handle::Handle( const QWBFS::Partition::Properties& properties )
{
	d = new HandleData( properties );
}

Handle::Handle( const QString& partition )
{
	d = new HandleData( QWBFS::Partition::Properties( partition ) );
}

Handle::~Handle()
{
	//qWarning() << Q_FUNC_INFO;
}

bool Handle::isValid() const
{
	return d->handle;
}

wbfs_t* Handle::ptr() const
{
	return d->handle;
}

QWBFS::Partition::Properties Handle::properties() const
{
	return d->properties;
}

bool Handle::reset() const
{
	return d->properties.reset;
}

QString Handle::partition() const
{
	return d->properties.partition;
}
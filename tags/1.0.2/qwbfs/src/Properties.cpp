/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe Azevedo aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Properties.cpp
** Date      : 2010-06-16T14:19:29
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This package is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**
** In addition, as a special exception, the copyright holders give permission
** to link this program with the OpenSSL project's "OpenSSL" library (or with
** modified versions of it that use the same license as the "OpenSSL"
** library), and distribute the linked executables. You must obey the GNU
** General Public License in all respects for all of the code used other than
** "OpenSSL".  If you modify file(s), you may extend this exception to your
** version of the file(s), but you are not obligated to do so. If you do not
** wish to do so, delete this exception statement from your version.
**
****************************************************************************/
#include "Properties.h"
#include "datacache/DataNetworkCache.h"
#include "UIMain.h"

#include <QSettings>
#include <QDir>
#include <QDebug>

#define SETTINGS_CACHE_WORKING_PATH "cache/workingPath"
#define SETTINGS_CACHE_DISK_SIZE "cache/diskSize"
#define SETTINGS_CACHE_MEMORY_SIZE "cache/memorySize"
#define SETTINGS_CACHE_USE_TEMPORARY_WORKING_PATH "cache/useTemporaryWorkingPath"
#define SETTINGS_PROXY_TYPE "proxy/type"
#define SETTINGS_PROXY_SERVER "proxy/server"
#define SETTINGS_PROXY_PORT "proxy/port"
#define SETTINGS_PROXY_LOGIN "proxy/login"
#define SETTINGS_PROXY_PASSWORD "proxy/password"
#define SETTINGS_WINDOW_GEOMETRY "window/geometry"
#define SETTINGS_WINDOW_STATE "window/state"
#define SETTINGS_PARTITION_WIDGET_STATE "partitionWidget"

Properties::Properties( QObject* parent )
	: QObject( parent )
{
	mSettings = new QSettings( this );
	//qWarning() << mSettings->fileName();
}

Properties::~Properties()
{
}

QString Properties::temporaryPath() const
{
	return QDir::tempPath();
}

QString Properties::cacheWorkingPath() const
{
	return mSettings->value( SETTINGS_CACHE_WORKING_PATH, temporaryPath() ).toString();
}

void Properties::setCacheWorkingPath( const QString& path )
{
	mSettings->setValue( SETTINGS_CACHE_WORKING_PATH, path );
}

qint64 Properties::cacheDiskSize() const
{
	return mSettings->value( SETTINGS_CACHE_DISK_SIZE, DATA_NETWORK_CACHE_DEFAULT_DISK_SIZE ).value<qint64>();
}

void Properties::setCacheDiskSize( qint64 sizeKb )
{
	mSettings->setValue( SETTINGS_CACHE_DISK_SIZE, sizeKb );
}

qint64 Properties::cacheMemorySize() const
{
	return mSettings->value( SETTINGS_CACHE_MEMORY_SIZE, DATA_NETWORK_CACHE_DEFAULT_MEMORY_SIZE ).value<qint64>();
}

void Properties::setCacheMemorySize( qint64 sizeKb )
{
	mSettings->setValue( SETTINGS_CACHE_MEMORY_SIZE, sizeKb );
}

bool Properties::cacheUseTemporaryPath() const
{
	return mSettings->value( SETTINGS_CACHE_USE_TEMPORARY_WORKING_PATH, false ).toBool();
}

void Properties::setCacheUseTemporaryPath( bool useTemporary )
{
	mSettings->setValue( SETTINGS_CACHE_USE_TEMPORARY_WORKING_PATH, useTemporary );
}

QNetworkProxy::ProxyType Properties::proxyType() const
{
	return QNetworkProxy::ProxyType( mSettings->value( SETTINGS_PROXY_TYPE, QNetworkProxy::NoProxy ).toInt() );
}

void Properties::setProxyType( QNetworkProxy::ProxyType type )
{
	mSettings->setValue( SETTINGS_PROXY_TYPE, type );
}

QString Properties::proxyServer() const
{
	return mSettings->value( SETTINGS_PROXY_SERVER ).toString();
}

void Properties::setProxyServer( const QString& server )
{
	mSettings->setValue( SETTINGS_PROXY_SERVER, server );
}

int Properties::proxyPort() const
{
	return mSettings->value( SETTINGS_PROXY_PORT, -1 ).toInt();
}

void Properties::setProxyPort( int port )
{
	mSettings->setValue( SETTINGS_PROXY_PORT, port );
}

QString Properties::proxyLogin() const
{
	return mSettings->value( SETTINGS_PROXY_LOGIN ).toString();
}

void Properties::setProxyLogin( const QString& login )
{
	mSettings->setValue( SETTINGS_PROXY_LOGIN, login );
}

QString Properties::proxyPassword() const
{
	return decrypt( mSettings->value( SETTINGS_PROXY_PASSWORD ).toByteArray() );
}

void Properties::setProxyPassword( const QString& password )
{
	mSettings->setValue( SETTINGS_PROXY_PASSWORD, crypt( password ) );
}

void Properties::restoreState( UIMain* window ) const
{
	const QRect geometry = mSettings->value( SETTINGS_WINDOW_GEOMETRY ).toRect();
	const QByteArray state = mSettings->value( SETTINGS_WINDOW_STATE ).toByteArray();
	
	if ( geometry.isNull() ) {
		window->showMaximized();
	}
	else {
		window->setGeometry( geometry );
	}
	
	window->restoreState( state );
}

void Properties::saveState( UIMain* window )
{
	const QRect geometry = window->geometry();
	const QByteArray state = window->saveState();
	
	mSettings->setValue( SETTINGS_WINDOW_GEOMETRY, geometry );
	mSettings->setValue( SETTINGS_WINDOW_STATE, state );
}

QString Properties::decrypt( const QByteArray& data )
{
	return data.isEmpty() ? QString::null : QString::fromUtf8( qUncompress( QByteArray::fromBase64( data ) ) );
}

QByteArray Properties::crypt( const QString& string )
{
	return string.isEmpty() ? QByteArray() : qCompress( string.toUtf8(), 9 ).toBase64();
}
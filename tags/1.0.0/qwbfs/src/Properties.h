/****************************************************************************
**
** 		Created using Monkey Studio IDE v1.8.4.0 (1.8.4.0)
** Authors   : Filipe AZEVEDO aka Nox P@sNox <pasnox@gmail.com>
** Project   : QWBFS Manager
** FileName  : Properties.h
** Date      : 2010-06-15T23:21:10
** License   : GPL
** Home Page : http://code.google.com/p/qwbfs
** Comment   : QWBFS Manager is a cross platform WBFS manager developed using C++/Qt4.
** It's currently working fine under Windows (XP to Seven, 32 & 64Bits), Mac OS X (10.4.x to 10.6.x), Linux & unix like.
**
** DISCLAIMER: THIS APPLICATION COMES WITH NO WARRANTY AT ALL, NEITHER EXPRESS NOR IMPLIED.
** I DO NOT TAKE ANY RESPONSIBILITY FOR ANY DAMAGE TO YOUR HARDWARE OR YOUR DATA
** BECAUSE OF IMPROPER USAGE OF THIS SOFTWARE.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QObject>
#include <QNetworkProxy>

class QSettings;
class UIMain;

class Properties : public QObject
{
	Q_OBJECT
	
public:
	Properties( QObject* parent = 0 );
	virtual ~Properties();
	
	QString temporaryPath() const;
	
	QString cacheWorkingPath() const;
	void setCacheWorkingPath( const QString& path );
	
	qint64 cacheDiskSize() const;
	void setCacheDiskSize( qint64 sizeKb );
	
	qint64 cacheMemorySize() const;
	void setCacheMemorySize( qint64 sizeKb );
	
	bool cacheUseTemporaryPath() const;
	void setCacheUseTemporaryPath( bool useTemporary );
	
	QNetworkProxy::ProxyType proxyType() const;
	void setProxyType( QNetworkProxy::ProxyType type );
	
	QString proxyServer() const;
	void setProxyServer( const QString& server );
	
	int proxyPort() const;
	void setProxyPort( int port );
	
	QString proxyLogin() const;
	void setProxyLogin( const QString& login );
	
	QString proxyPassword() const;
	void setProxyPassword( const QString& password );
	
	void restoreState( UIMain* window ) const;
	void saveState( UIMain* window );
	
	static QString decrypt( const QByteArray& data );
	static QByteArray crypt( const QString& string );

protected:
	QSettings* mSettings;
};

#endif // PROPERTIES_H

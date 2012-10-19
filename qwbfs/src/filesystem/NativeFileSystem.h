#ifndef NATIVEFILESYSTEM_H
#define NATIVEFILESYSTEM_H

#include "AbstractFileSystem.h"

#include <QHash>

class NativeFileSystem : public AbstractFileSystem
{
    Q_OBJECT
    
public:
    NativeFileSystem( FileSystemManager* manager );
    virtual ~NativeFileSystem();
    
    virtual bool open( const QString& mountPoint );
    virtual bool close();
    virtual bool format();
    
    virtual QWBFS::FileSystemType type() const;
    virtual QWBFS::EntryTypes supportedFormats() const;
    virtual QWBFS::EntryType preferredFormat() const;
    
    virtual FileSystemEntry::List entries() const;
    virtual FileSystemEntry entry( int row ) const;
    virtual FileSystemEntry entry( const QString& id ) const;
    virtual bool hasEntry( const QString& id ) const;
    
    virtual bool addEntry( const FileSystemEntry& entry, QWBFS::EntryType format = QWBFS::EntryTypeUnknown );
    virtual bool removeEntry( const FileSystemEntry& entry );
    virtual void clear();

protected:
    virtual void setEntriesInternal( const FileSystemEntry::List& entries );
    void buildCache();

protected:
    FileSystemEntry::List mEntries;
    QHash<QString, FileSystemEntry*> mCache;
};

#endif // NATIVEFILESYSTEM_H

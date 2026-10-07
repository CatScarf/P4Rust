/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * fileio.h - subclasses of FileSys that actually operate on files
 *
 * Classes defined:
 *
 *	FileIO - hulk that acts on files, but not content
 *	FileIOBinary - raw binary files (unbuffered)
 *	FileIODir - not a file, but a dir - useful for auto-cleaned temp dirs
 *	FileIOBuffer - buffered files: raw/cr/crlf
 *	FileIOUnicode - buffered files: raw/cr/crlf w/UTF8 translation
 *	FileIOUTF16 - buffered files: raw/cr/crlf w/UTF8vs16 translation
 *	FileIOEmpty - empty file (simulates an empty file)
 *	FileIOAppend - text file in append only mode (write only)
 *	FileIOGzip - compressed binary file
 *	FileIOGunzip - binary file via compressed stream
 *	FileIOSymlink - symbolic links (UNIX only!)
 *	FileIOResource - Macintosh resource fork
 *	FileIOApple - apple double format or native on Macintosh
 */

class FileIOBuffer;
class FileIOUnicode;
class Gzip;
class StrBufDict;

class FileIO : public FileSys {

    public:
			FileIO();

	virtual int	Stat();
	virtual int     GetOwner();
	virtual P4INT64	StatAccessTime();
	virtual P4INT64	StatModTime();
	virtual void	StatModTimeHP(DateTimeHighPrecision *modTime);
	virtual bool	HasOnlyPerm( FilePerm perms );
	virtual void	Truncate( Error *e );
	virtual void	Truncate( offL_t offset, Error *e );
	virtual void	Chmod( FilePerm perms, Error *e );
	virtual void	ChmodTime( Error *e );
	virtual void	ChmodTime( P4INT64 modTime, Error *e );
	virtual void	ChmodTimeHP( const DateTimeHighPrecision &modTime, Error *e );
	virtual void	Unlink( Error *e );
	virtual void	UnlinkNoRetry( Error* e );
	virtual void	Rename( FileSys *target, Error *e );
	virtual void	RenameSourceSubstrInTargetSubdir(
	                    StrBuf &currentName,
	                    FileSys *target,
	                    Error *e );

	virtual void	RenameTargetSubStrSubdirInSource(
	                    StrBuf &currentName,
	                    FileSys *target,
	                    Error *e );

	virtual int	OsRename( StrPtr *source, StrPtr *target,
	                          FileSys *origTarget );

	virtual void    DepotSize( offL_t &len, Error *e );

	virtual void	SetExtendedAttribute( StrPtr *name, StrPtr *val, Error *e );
	virtual void	SetExtendedAttributes( StrDict *vals, Error *e );
	virtual void	GetExtendedAttribute( StrPtr *name, StrBuf *val, Error *e );
	virtual void	GetExtendedAttributes( StrBufDict *xattrs, Error *e );

# ifdef OS_NT
	// Currently only implements hidden file handling on NT
	virtual void	SetAttribute( FileSysAttr attrs, Error *e );
	static wchar_t	*UnicodeName( StrBuf *fname, int lfn );
# endif
} ;

class FileIOBinary : public FileIO {

    public:
			FileIOBinary() { fd=FD_INIT; isStd = 0; tellpos = 0; };
	virtual   	~FileIOBinary();

	virtual void	Open( FileOpenMode mode, Error *e );
	virtual void	Write( const char *buf, int len, Error *e );
	virtual int	Read( char *buf, int len, Error *e );
	virtual void	Close( Error *e );

	virtual FD_PTR	GetFd() { return fd; }
	virtual offL_t	GetSize();
	virtual void	Seek( offL_t offset, Error *e );
	virtual offL_t	Tell();
	virtual void	Fsync( Error *e );
	virtual void    DepotSize( offL_t &len, Error *e );
	virtual int     RetryCreate();
	virtual int     LinkCount();

# ifdef OS_NT
	static int      GetModeAflags( int mode )
	                { return openModes[mode].aflags; }
# endif

    protected:

	FD_PTR		fd;
	int		isStd;
	offL_t		tellpos;

	static const struct OpenMode {
		const char *modeName;
		int bflags;	// binary mode
		int aflags;	// append text mode
		int standard;
	} openModes[4];
    private:
	int             lastOSError;

} ;

class FileIODir : public FileIOBinary {
    public:

	 FileIODir();
	~FileIODir();

	void	Open( FileOpenMode mode, Error *e );
	void	Write( const char *buf, int len, Error *e );
	int	Read( char *buf, int len, Error *e );
	void	Close( Error *e );

	FD_PTR	GetFd();
	offL_t	GetSize();
	void	Seek( offL_t offset, Error *e );
	offL_t	Tell();
	void	Fsync( Error *e );

	void	Cleanup();
} ;

class FileIOCompress : public FileIOBinary {
    public:
	FileIOCompress() : gzip( NULL ), gzbuf( NULL ), pos( 0 ), size( -1 ),
	                   loop( 0 )
	                   { compMode = FIOC_PASS; }
	virtual	~FileIOCompress();

	virtual void	Open( FileOpenMode mode, Error *e );
	virtual void	Write( const char *buf, int len, Error *e );
	virtual int	Read( char *buf, int len, Error *e );
	virtual void	Close( Error *e );
	virtual offL_t	GetRealSize();
	virtual offL_t	Tell() { return pos; }
	virtual void	Seek( offL_t offset, Error *e );

    private:
	enum { FIOC_PASS, FIOC_GZIP, FIOC_GUNZIP } compMode;
	Gzip		*gzip;
	StrFixed	*gzbuf;
	offL_t		pos;
	offL_t		size;
	FileIOBuffer	*loop;

    private:
	friend class FileIOBuffer;
	friend class FileIOUnicode;
	void		SetLoopback(FileIOBuffer* l) { loop =l; }
	virtual void	WriteThrough( const char *buf, int len, Error *e );
	int		IsUnCompress() {
			    return ( GetType() & FST_C_MASK ) == FST_C_GUNZIP;
			}

} ;

class FileIOBuffer : public FileIOCompress {

    public:
			FileIOBuffer( LineType lineType ) : 
			    rcv( 0 ), snd( 0 ),
			    lineType( lineType ),
			    iobuf( BufferSize() ),
			    checksum( 0 )
	                {}

	virtual void	Open( FileOpenMode mode, Error *e );
	virtual void	Write( const char *buf, int len, Error *e );
	virtual int	Read( char *buf, int len, Error *e );
	virtual void	Close( Error *e );
	virtual void	Seek( offL_t offset, Error *e );
	virtual offL_t	Tell();
	virtual int	ReadLine( StrBuf *buf, Error *e );
	virtual void	SetBufferSize( size_t l );
	virtual void	SetDigest( MD5 *m );

    protected:
	char		*ptr;
	int		rcv;
	int		snd;

	LineType	lineType;
	StrFixed	iobuf;
	MD5		*checksum;

	virtual void	FlushBuffer( Error * );
	virtual void	FillBuffer( Error * );

	void		WriteText( const char *buf, int len, Error *e );

    private:
	friend class FileIOCompress;
	void		WriteLoop( const char *buf, int len, int flush, Error *e );
} ;

class FileIOUnicode : public FileIOBuffer {
    public:
			FileIOUnicode( LineType lineType ) : 
			    FileIOBuffer( lineType ),
			    trans( NULL ),
			    tbuf( BufferSize() ),
			    tsz( 0 )
	                {}

	virtual void	Close( Error * );
	virtual void	Translator( CharSetCvt * );

    protected:
	CharSetCvt	*trans;
	StrFixed	tbuf;
	int		tsz;

	virtual void	FlushBuffer( Error * );
	virtual void	FillBuffer( Error * );
} ;

class FileIOUTF16 : public FileIOUnicode {
    public:
	FileIOUTF16( LineType lineType );

	virtual void	Set( const StrPtr &name );
	virtual void	Set( const StrPtr &name, Error *e );
	virtual void	Open( FileOpenMode mode, Error *e );
	virtual void	Close( Error *e );
	virtual void	Translator( CharSetCvt * );
};

class FileIOUTF8 : public FileIOUTF16 {
    public:
	FileIOUTF8( LineType lineType );

	virtual void	Set( const StrPtr &name, Error *e );
	virtual void	Open( FileOpenMode mode, Error *e );

    private:
	void	SetCharSetPriv();
};

class FileIOAppend : public FileIOBuffer {

    public:

	                FileIOAppend( LineType lineType ) :
	                    FileIOBuffer( lineType )
	                {} ;

	virtual         ~FileIOAppend();

	virtual void    Close( Error *e );
	virtual void	Open( FileOpenMode mode, Error *e );
	virtual void	Write( const char *buf, int len, Error *e );

	// for atomic filesize
	virtual offL_t	GetSize();

	// size of current (not rename()'d) file
	virtual offL_t	GetCurrentSize();

	// Copies on Windows.
	// Locks, renames, and removes write on UNIX.

	virtual void	Rename( FileSys *target, Error *e );

} ;

class FileIOEmpty : public FileSys {

    public:
			FileIOEmpty() {}
			~FileIOEmpty() {}

	virtual int	Stat() { return FSF_EMPTY; }
	virtual P4INT64	StatModTime() { return 0; }
	virtual P4INT64	StatAccessTime() { return 0; }
	virtual void	Truncate( Error *e ) {}
	virtual void	Truncate( offL_t offset, Error *e ) {}
	virtual void	Chmod( FilePerm perms, Error *e ) {}
	virtual void	ChmodTime( Error *e ) {}
	virtual void	ChmodTime( P4INT64 modTime, Error *e ) {}
	virtual void	Unlink( Error *e ) {}
	virtual void	Rename( FileSys *target, Error *e ) {}

	virtual void	Open( FileOpenMode mode, Error *e ) {};
	virtual void	Write( const char *buf, int len, Error *e ) {}
	virtual int	Read( char *buf, int len, Error *e ) { return 0; }
	virtual void	Close( Error *e ) {}

	virtual FD_PTR	GetFd() { return FD_ERR; }
	virtual offL_t	GetSize() { return 0; }
	virtual void	Seek( offL_t offset, Error * ) {}
} ;

# ifdef HAVE_SYMLINKS

class FileIOSymlink : public FileIO {

    public:
			~FileIOSymlink();

	virtual void	Open( FileOpenMode mode, Error *e );
	virtual void	Write( const char *buf, int len, Error *e );
	virtual int	Read( char *buf, int len, Error *e );
	virtual void	Close( Error *e );

	virtual P4INT64	StatModTime();
	virtual P4INT64	StatAccessTime();
	virtual void	Truncate( Error *e );
	virtual void	Truncate( offL_t offset, Error *e );
	virtual void	Chmod( FilePerm perms, Error *e );
	virtual void	ChmodTime( P4INT64 modTime, Error *e );

    private:
	StrBuf		value;
	int		offset;

} ;

# else

class FileIOSymlink : public FileIOBinary { } ;

# endif // HAVE_SYMLINKS


class AppleForkSplit;
class AppleForkCombine;
class DataFork;

# if !(defined ( OS_MACOSX ) && OS_VER < 1010)

class FileIOResource : public FileIOBinary { } ;

class FileIOApple : public FileIO {

    public:

			FileIOApple();
			~FileIOApple();

	virtual void	Open( FileOpenMode mode, Error *e );
	virtual void	Write( const char *buf, int len, Error *e );
	virtual int	Read( char *buf, int len, Error *e );
	virtual void	Close( Error *e );

    private:

	// Split, and combine AppleSingle format

	AppleForkSplit 		*split;
	AppleForkCombine	*combine;
	DataFork		*dataFork;


	// Access to underlying data and %header files.

	FileIO		*data;

    // NON-MAC SPECIFIC
    //
    public:

	virtual bool    HasOnlyPerm( FilePerm perms );
	virtual int	Stat();
	virtual void	Set( const StrPtr &name );
	virtual void	Set( const StrPtr &name, Error *e );

	virtual P4INT64	StatModTime();
	virtual P4INT64	StatAccessTime();
	virtual void	StatModTimeHP(DateTimeHighPrecision *modTime);
	virtual void	Truncate( Error *e );
	virtual void	Truncate( offL_t offset, Error *e );
	virtual void	Chmod( FilePerm perms, Error *e );
	virtual void	ChmodTime( P4INT64 modTime, Error *e );
	virtual void	ChmodTimeHP( const DateTimeHighPrecision &modTime, Error *e );
	virtual void	Unlink( Error *e );
	virtual void	Rename( FileSys *target, Error *e );

    private:
	FileIO		*header;

} ;

# else // !OS_MACOSX

class MacFile;

class FileIOMac : public FileIO {

    public:
		FileIOMac();
		~FileIOMac();

	virtual void	Set( const StrPtr &name );
	virtual void	Set( const StrPtr &name, Error *e );
	void	Set( const char *name ) { FileSys::Set( name ); }
	void	Set( const char *name, Error *e )
	        { FileSys::Set( name, e ); }
	
	int	Stat( bool includeRsrcFork = false );

	void	Open( FileOpenMode mode, Error *e );
	void	Write( const char *buf, int len, Error *e );
	int	Read( char *buf, int len, Error *e );
	void	Close( Error *e );
	void	Unlink( Error *e );

	virtual void	Rename( FileSys *target, Error *e );

	offL_t	GetSize();
	void	Seek( offL_t offset, Error * );

    protected:
	MacFile * GetMacFile( Error *e );
	MacFile * CreateMacFile( Error *e );

    private:
	MacFile * mf;
} ;


class FileIOResource : public FileIOMac {

    public:

    			FileIOResource();
    			~FileIOResource();

	virtual int	Stat() { return FileIOMac::Stat( true ); };

	virtual void	Set( const StrPtr &name );
	virtual void	Set( const StrPtr &name, Error *e );
	virtual void	Open( FileOpenMode mode, Error *e );
	virtual void	Write( const char *buf, int len, Error *e );
	virtual int	Read( char *buf, int len, Error *e );
	virtual void	Close( Error *e );

	virtual void	Unlink( Error *e );

    private:
	StrBuf		*resourceData;		// content of resource fork
	int		ioOffset;		// data read from resourceData
	int		hasDotInName;		// resource fork?


} ;


class FileIOApple : public FileIOMac {

    public:

			FileIOApple();
			~FileIOApple();

	virtual void	Open( FileOpenMode mode, Error *e );
	virtual void	Write( const char *buf, int len, Error *e );
	virtual int	Read( char *buf, int len, Error *e );
	virtual void	Close( Error *e );

    private:

	// Split, and combine AppleSingle format

	AppleForkSplit 		*split;
	AppleForkCombine	*combine;
	DataFork		*dataFork;

	// Access to underlying data and %header files.

	FileIO		*data;

    // MAC-BUILD SPECIFIC
    //
    public:
	// Special for detecting files with resource forks
	virtual int	Stat() { return FileIOMac::Stat( true ); };

	int		HasResourceFork();

    private:
	class FinfoFork		*finfoFork;
	class ResourceFork	*resourceFork;
	class CommentFork	*commentFork;
} ;

# endif // !OS_MACOSX


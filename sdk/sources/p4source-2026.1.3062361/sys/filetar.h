/*
 * Copyright 2026 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * FileTar.h - FileSys interface to tar image generator.
 *
 */

// from the Posix spec
// https://pubs.opengroup.org/onlinepubs/9699919799/utilities/pax.html
# define TARSZ    512
# define HDRSZ  10240
# define COMPSZ 65536

struct tarheader {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
} ;


enum CallbackState {
    Init,    // just starting, need to add the header.
    Header,  // some bytes of the header still need sending
    Payload, // SOme of the payload is still pending
    Pad,     // We had a short read, need to send some padding chars
    Fill,    // We have finished the payload, fill out to the next 512 block boundary
    Done,    // I'm done, skip to the next.
} ;

class Gzip;
class Lbr;
class TarCallback;
typedef void (*TarCbFunc_t) ( TarCallback *cb, Error *e ) ;
typedef void (*TarCbDelFunc_t) ( TarCallback *cb ) ;

class TarCallback {
    public:
	// Filebase payload
	TarCallback( const char *path, const char *name, P4INT64 mtime,
	             FileSys *input );
	// StrBuf based payload
	TarCallback( const char *path, const char *name, P4INT64 mtime );
	// Lbr based payload
	TarCallback( const char *path, const char *name, P4INT64 mtime,
	             Lbr *lbr, TarCbFunc_t i, TarCbDelFunc_t d );
	~TarCallback();
	CallbackState State() { return state; }
	StrBuf *GetPayload() { return &payload; }
	void SetHdrPtr( const char *late, char *headers );
	int Read( char *buf, int len, Error *e );
	FileSys *source;
	Lbr *lbr;
    private:
	void Initialize( const char *path, const char *name, P4INT64 mtme );
	int SetTarHeader();
	static void dochksum( tarheader *th );
	static int calc_tag_len( int keylen, int vallen );
	static int tag_fudge( int base );
	static int vsize( P4INT64 val );

	StrBuf path;
	StrBuf name;

	P4INT64 mtime;
	CallbackState state;
	char *tarpos;
	int headercnt;
	const char *late;
	char *headers;
	int fillcnt;
	P4INT64 padcnt;
	P4INT64 paycnt;
	P4INT64 paysize;
	StrBuf payload;

	TarCbFunc_t lbrinit;
	TarCbDelFunc_t lbrdel;
} ;



class FileTar : public FileIOBinary
{
    public:
	FileTar( int compressed );
	~FileTar();
	void AddCallback( TarCallback *cb );

	// FileSys methods needed by ReadFile

	int	Read( char *buf, int len, Error *e );
	// FileSys stubs
	void	Open( FileOpenMode mode, Error *e ) {}
	void	Seek( offL_t offset, Error *e ) {}

	void	Close( Error *e ) {};
	void	Write( const char *buf, int len, Error *e ) {};
	int	Stat() { return FSF_EXISTS; }
	P4INT64	StatModTime() { return 0; }
	P4INT64	StatAccessTime() { return 0; }
	void	Truncate( Error *e ) {};
	void	Truncate( offL_t offset, Error *e ) {};
	void	Unlink( Error *e = 0 ) {};
	void	Rename( FileSys *target, Error *e ) {};
	void	Chmod( FilePerm perms, Error *e ) {};
	void	ChmodTime( Error *e ) {};

    private:
	int	real_read( char *buf, int len, Error *e );
	int	comp_read( char *buf, int len, Error *e );

	VarArray cblist; // list of callbacks
	int cbidx;       // current callback
	Gzip *gzip;
	StrFixed *gzbuf;
	StrFixed *headers;
	int trailcnt;
	int done;
};

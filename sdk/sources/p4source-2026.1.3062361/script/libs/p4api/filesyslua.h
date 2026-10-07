/*
 * Copyright 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

// This binding does not call parent functionality.  If a function is not implemented
// by the script, it is a NOOP.  This binding is intended for interfacing with existing
// native code, not as a general purpose utility - hence some functions are not possible
// to overload.

class FileSysLua : public FileSys {

	public:

	     FileSysLua( FileSysType type, const char* impl, const int apiVersion,
	                 sol::table data );
	    ~FileSysLua();

	    static void doBindings( sol::state* lua, sol::table& ns,
	                            const char* implName, const int apiVersion );

	    static std::unique_ptr< FileSysLua >
	        Make( FileSysType type, const char* impl, const int apiVersion,
	              sol::table data );

	    void Open( FileOpenMode mode, Error *e );
	    void Write( const char *buf, int len, Error *e );
	    int  Read( char *buf, int len, Error *e );
	    int  ReadLine( StrBuf *buf, Error *e );
	    void Close( Error *e );
	    int  Stat();
	    P4INT64  StatModTime();
	    P4INT64  StatAccessTime(){ return 0; }
	    void Truncate( Error *e );
	    void Truncate( offL_t offset, Error *e ) ;
	    void Unlink( Error *e = 0 );
	    void Rename( FileSys *target, Error *e );
	    void Chmod( FilePerm perms, Error *e );
	    void ChmodTime( Error *e );

	    std::string fPath() { std::string p( path.Text() ); return p; }

	    sol::protected_function fOpen, fWrite, fRead, fReadLine, fClose,
	                            fStat, fStatModTime, fTruncate, fTruncate1,
	                            fUnlink, fRename, fChmod, fChmodTime;

	    // Something for the Lua code to stash instance data in.
	    sol::table data;

	    const char* impl;
	    const int apiVersion;
} ;

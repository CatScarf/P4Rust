/*
 * Copyright 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef HAS_EXTENSIONS

# include <limits>

# include <error.h>
# include <strbuf.h>
# include <filesys.h>
# include <p4script53.h>
# include <filesyslua.h>

extern bool solfnCheck( const sol::protected_function_result& r,
	                const char* impl, const char* where, Error* e );

extern void solExcpESet( const sol::error& err, const char* impl,
	                 const char* where, Error* e );

FileSysLua::FileSysLua( FileSysType type, const char* impl, const int apiVersion,
	                sol::table data )
	: impl( impl ), apiVersion( apiVersion )
{
	this->data = data;
}

FileSysLua::~FileSysLua()
{

}

void FileSysLua::doBindings( sol::state* lua, sol::table& ns,
	                     const char* implName, const int apiVersion )
{
	ns.new_enum( "FileOpenMode",
	    "FOM_READ"  , FileOpenMode::FOM_READ,
	    "FOM_WRITE" , FileOpenMode::FOM_WRITE,
	    "FOM_RW"    , FileOpenMode::FOM_RW,
	    "FOM_UWRITE", FileOpenMode::FOM_UWRITE );

	ns.new_usertype< FileSysLua >( "FileSys", 
	    "Open"        , &FileSysLua::fOpen,
	    "Write"       , &FileSysLua::fWrite,
	    "Read"        , &FileSysLua::fRead,
	    "ReadLine"    , &FileSysLua::fReadLine,
	    "Close"       , &FileSysLua::fClose,
	    "Stat"        , &FileSysLua::fStat,
	    "StatModeTime", &FileSysLua::fStatModTime,
	    "Truncate"    , sol::overload( &FileSysLua::fTruncate,
	                                   &FileSysLua::fTruncate1 ),
	    "Unlink"      , &FileSysLua::fUnlink,
	    "Rename"      , &FileSysLua::fRename,
	    "Chmod"       , &FileSysLua::fChmod,
	    "ChmodTime"   , &FileSysLua::fChmodTime,
	    "Path"        , &FileSysLua::fPath,
	    "data"        , &FileSysLua::data,
	    "new", sol::factories( [=]( FileSysType type,
	                                const sol::variadic_args data )
	        {
	            auto table = data.size() == 1 ? data[ 0 ].get< sol::table >()
	                                          : sol::table();
	            return FileSysLua::Make( type, implName,
	                                     apiVersion, table );
	        } ),

	    sol::base_classes, sol::bases< FileSys >()
	);
}

std::unique_ptr< FileSysLua >
FileSysLua::Make( FileSysType type, const char* impl, const int apiVersion,
	          sol::table data )
{
	return
	    std::unique_ptr< FileSysLua >( new FileSysLua( type, impl,
	                                                   apiVersion, data ) );
}


void FileSysLua::Open( FileOpenMode mode, Error *e )
{
	if( !fOpen )
	    return;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fOpen( mode, es ) : fOpen( this, mode, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "FileSysLua::Open", e );
}

void FileSysLua::Write( const char *buf, int len, Error *e )
{
	if( !fWrite )
	    return;

	std::string str;
	str.resize( len );
	str.assign( buf, len );

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fWrite( str, len, es )
	                         : fWrite( this, str, len, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "FileSysLua::Write", e );
}

int FileSysLua::Read( char *buf, int len, Error *e )
{
	if( !fRead )
	    return 0;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fRead( len, es ) : fRead( this, len, es );

	if( esp->Test() )
	    e->Merge( *esp );

	if( solfnCheck( r, impl, "FileSysLua::Read", e ) )
	    return 0;

	try {
	    std::string b = r.get< std::string >( 0 );
	    int lr = r.get< int >( 1 );
	    int l = ( lr >= 0 && lr <= len ) ? lr : 0;
	    memcpy( buf, b.c_str(), l );
	    return l;
	} catch( const sol::error& err )
	{ solExcpESet( err, impl, "FileSysLua::Read", e ); }

	return 0;
}

int FileSysLua::ReadLine( StrBuf *buf, Error *e )
{
	if( !fReadLine )
	    return 0;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fReadLine( es ) : fReadLine( this, es );

	if( esp->Test() )
	    e->Merge( *esp );

	if( solfnCheck( r, impl, "FileSysLua::ReadLine", e ) )
	    return 0;

	try {
	    int ret = r.get< int >();
	    // todo: Need to make this work for binary files?  See Read().
	    std::string b = r.get< std::string >( 1 );
	    buf->Set( b.c_str() );
	    return ret;
	
	} catch( const sol::error& err )
	{ solExcpESet( err, impl, "FileSysLua::ReadLine", e ); }

	return 0;
}

void FileSysLua::Close( Error *e )
{
	if( !fClose )
	    return;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fClose( es ) : fClose( this, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "FileSysLua::close", e );
}

int FileSysLua::Stat()
{
	if( !fStat )
	    return 0;

	auto r = fStat( this );

	if( !r.valid() )
	    return 0;

	try {
	    return r.get< int >();
	} catch( const sol::error& err )
	{
	    // Nothing we can do.
	}

	return 0;
}

P4INT64 FileSysLua::StatModTime()
{
	if( !fStatModTime )
	    return 0;

	auto r = fStatModTime( this );

	if( !r.valid() )
	    return 0;

	try {
	    return r.get< P4INT64 >();
	} catch( const sol::error& err )
	{
	    // Nothing we can do.
	}

	return 0;
}

void FileSysLua::Truncate( Error *e )
{
	if( !fTruncate )
	    return;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fTruncate( es ) : fTruncate( this, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "FileSysLua::Truncate", e );
}

void FileSysLua::Truncate( offL_t offset, Error *e ) 
{
	if( !fTruncate1 )
	    return;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fTruncate1( offset, es )
	                         : fTruncate1( this, offset, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "FileSysLua::Truncate1", e );
}

void FileSysLua::Unlink( Error *e )
{
	if( !fUnlink )
	    return;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fUnlink( es ) : fUnlink( this, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "FileSysLua::Unlink", e );
}

void FileSysLua::Rename( FileSys *target, Error *e )
{
	if( !fRename )
	    return;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fRename( target, es )
	                         : fRename( this, target, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "FileSysLua::Rename", e );
}

void FileSysLua::Chmod( FilePerm perms, Error *e )
{
	if( !fChmod )
	    return;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fChmod( perms, es )
	                         : fChmod( this, perms, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "FileSysLua::Chmod", e );
}

void FileSysLua::ChmodTime( Error *e )
{
	if( !fChmodTime )
	    return;

	auto es = std::make_shared< Error >();
	Error* esp = es.get();

	auto r = apiVersion == 1 ? fChmodTime( es ) : fChmodTime( this, es );

	if( esp->Test() )
	    e->Merge( *esp );

	solfnCheck( r, impl, "FileSysLua::ChmodTime", e );
}

# endif

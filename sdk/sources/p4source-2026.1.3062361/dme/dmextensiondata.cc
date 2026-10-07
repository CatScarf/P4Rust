/*
 * Copyright 1995, 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <charman.h>

# include <stdhdrs.h>
# include <strbuf.h>
# include <error.h>
# include <debug.h>
# include <debugextension.h>
# include <msgscript.h>
# include <msgdm.h>
# include <strops.h>
# include <filesys.h>
# include <pathsys.h>
# include <unzip.h>
# include <uuid.h>
# include <uuidimpl.h>
# include <msgsupp.h>
# include <msgserver.h>
# include <msgserver2.h>
# include <i18napi.h>
# include <charcvt.h>
# include <validate.h>

# include "dmextensiondata.h"

# include <dmetypes.h>

# ifdef HAS_EXTENSIONS

# include <string> 
# include <iostream>
# include <unordered_set>
# include <set>
# include <regex>
# include <algorithm>

# include <json.hpp>
using json = nlohmann::json;

# define MAX_ZIPFILENAME 1024
# define READ_ZIPSIZE 8192

// JSON parse limits.
# define PRE_MAXTOKENS 200

static const std::unordered_set< int > known_api_versions { 1, 20191 };

const std::string ExtensionData::nameDelimiter = "::";
const std::string ExtensionData::revDelimiter = "#";

ExtensionData::ExtensionData( const StrBuf& archiveFile,
	                      p4_std_optional::optional< StrBuf > unzipPath, Error* e )
{
	archiveDir = Unzip( archiveFile, unzipPath, {}, e );
	revision = 1;
}

ExtensionData::ExtensionData( const StrBuf& depotFile, const int& depotRev,
	                      const StrBuf& srvExtsDir,
	                      const p4_std_optional::optional< StrBuf > archiveFile,
	                      Error* e )
{
	this->srvExtsDir = srvExtsDir;
	archiveDir = FileSys::CreateUPtr( FST_DIRECTORY );
	key = GetUUIDFromDepotFile( depotFile );
	revision = depotRev;
	if( archiveFile )
	    archiveDir = Unzip( *archiveFile, srvExtsDir, {}, e );
	else
	    archiveDir->Set( GetExtSrvExtDir() );
}

ExtensionData::~ExtensionData()
{
}

std::pair< StrBuf, StrBuf >
ExtensionData::SplitSpecUUID( const StrBuf& SpecAndUUID )
{
	std::string t = SpecAndUUID.Text();
	auto off = t.find( "/" );
	StrBuf s( t.substr( 0, off ).c_str() ), k( t.substr( off + 1 ).c_str() );
	return std::pair< StrBuf, StrBuf >( s, k );
}

// dfile    //.p4-exts/UUID/name
// lbr      p4root/.p4exts/UUID/name
// unpacked p4root/s.e.d/UUID/(revision-arch|revision-data)

StrBuf ExtensionData::GetUUIDFromDepotFile( const StrBuf& depotFile )
{
	//.p4-extensions/UUID/name.p4-extension
	std::string file( depotFile.Text() );
	//.p4-extensions/UUID/name.p4-extension
	//              ^
	auto offA = file.find( "/", 2 );
	//.p4-extensions/UUID/name.p4-extension
	//                   ^
	auto offB = file.find( "/", offA + 2 );

	return StrBuf( file.substr( offA + 1, offB - offA - 1 ).c_str() );
}

std::tuple< StrBuf, StrBuf, StrBuf > 
        ExtensionData::ParseFullname( const StrPtr& fullname, Error *e )
{
 	StrBuf namespc, extname, extrev;

	std::string t1 = fullname.Text();
	auto off1 = t1.find( ExtensionData::nameDelimiter );
	if( off1 == std::string::npos )
	    e->Set( MsgDm::ExtensionBadName );
	else
	{
	    namespc = t1.substr( 0, off1 ).c_str();

	    std::string t2 = t1.substr( off1 +
	                ExtensionData::nameDelimiter.length() );
	    auto off2 = t2.find( ExtensionData::revDelimiter );

	    if( off2 != std::string::npos )
	    {
	        extname = t2.substr( 0, off2 ).c_str();
	        extrev = t2.substr( off2 + ExtensionData::revDelimiter.length(),
	                                std::string::npos ).c_str();

	        if( !extrev.IsNumeric() || extrev.Atoi() <= 0 ||
	                                   extrev.Atoi() > INT_MAX )
	            e->Set( MsgDm::ExtensionBadName );
	    }
	    else
	        extname = t2.c_str();
	}

        return std::tuple< StrBuf, StrBuf, StrBuf >( namespc, extname, extrev );
}

StrBuf ExtensionData::MakeFullname( const std::string& namespc,
	                            const std::string& extname,
	                            const std::string& extrev )
{
	std::string ts = namespc + ExtensionData::nameDelimiter + extname;
	if( extrev.length() )
	{
	    ts += ExtensionData::revDelimiter;
	    ts += extrev;
	}
	StrBuf t = ts.c_str();
	return t;
}

void ExtensionData::SetSrvExtDir( const StrBuf& dir )
{
	srvExtsDir = dir;
}

void ExtensionData::SetKey( const char* key )
{
	this->key = key;
}

void ExtensionData::SetRevision( const int rev )
{
	revision = rev;
}

void ExtensionData::SetProduct( const char* p )
{
	std::string s = p;
	std::transform( s.begin(), s.end(), s.begin(), ::tolower );
	prod = s.c_str();
}

StrBuf ExtensionData::GetExtSrvExtDir() const
{
	StrBuf dst;
	dst << srvExtsDir << "/" << key << "/" << revision << "-arch" ;
	return dst;
}

StrBuf ExtensionData::GetExtSrvExtDataDir() const
{
	StrBuf dst;
	dst << srvExtsDir << "/" << key << "/" << revision << "-data";
	return dst;
}

StrBuf ExtensionData::GetDepotPath( const StrPtr& extsDepot )
{
	StrBuf dp;
	dp << "//" << extsDepot << "/" << key << "/"
	   << name << ".p4-extension";
	return dp;
}

StrBuf ExtensionData::GetExtNameFromDepotPath( const StrPtr& depotFile )
{
	//.p4-extensions/123e4567-e89b-12d3-a456-426655440001/name.p4-extension

	std::string file( depotFile.Text() );
	auto offA = file.rfind( "." );
	auto offB = file.rfind( "/" );
	StrBuf name = file.substr( offB + 1, offA - 1 - offB  ).c_str();
	return name;
}

int ExtensionData::GetAPIVersion() const
{
	return apiVersion;
}

int ExtensionData::GetManifestVersion() const
{
	return manifestVersion;
}

StrPtr* ExtensionData::GetDescription()
{
	return &description;
}

StrPtr* ExtensionData::GetNamespc()
{
	return &namespc;
}

StrPtr* ExtensionData::GetName()
{
	return &name;
}

// todo: bleh
StrPtr* ExtensionData::GetRevisionStr()
{
	StrNum n( revision );
	revisionStr = n;
	return &revisionStr;
}

StrPtr* ExtensionData::GetUUID()
{
	return &key;
}

StrPtr* ExtensionData::GetVersion()
{
	return &version;
}

StrPtr* ExtensionData::GetLicense()
{
	return &license;
}

StrPtr* ExtensionData::GetDeveloperUrl()
{
	return &developerUrl;
}

StrPtr* ExtensionData::GetHomepageUrl()
{
	return &homepageUrl;
}

StrPtr* ExtensionData::GetLimitInstance()
{
	return &limitInstance;
}

bool ExtensionData::GetRequireUser()
{
	return requireUser;
}

StrBuf ExtensionData::SerializeCompatProds() const
{
	StrBuf s;
	bool first = true;

	for( const auto& c : compatProds )
	{
	    if( !first )
	        s << ", ";
	    else
	        first = false;

	    s << c.c_str();
	}

	return s;
}

StrBuf ExtensionData::GetScriptMainPath()
{
	StrBuf path;

	Error eIgnore;
	// This always uses Unix paths since it works on Windows and Unix
	// and this path ends up in db.trigger where it'll be run on both
	// platforms in mixed Windows/Unix replication setups.
	PathSys *p = PathSys::Create( StrRef( "UNIX" ), &eIgnore );
	p->Set( archiveDir->Path() );
	p->SetLocal( *p, StrRef( "main.lua" ) );
	path = p->Text();
	delete p;

	return path;
}

StrBuf ExtensionData::GetArchDir()
{
	StrBuf path;
	
	path << archiveDir->Path();
	return path;
}

int ExtensionData::GetRevision() const
{
	return revision;
}

SCR_VERSION ExtensionData::GetScrVersion() const
{
	return P4SCRIPT_LUA_53;
}

StrPtr* ExtensionData::GetDeveloper()
{
	return &developerName;
}

StrPtr* ExtensionData::GetDefaultLocale()
{
	return &defaultLocale;
}

std::unordered_map< std::string,
	            std::unordered_map< std::string, std::string > >
ExtensionData::GetTranslationMap()
{
	return translationMap;
}

bool ExtensionData::Install( Error* e )
{
	auto t = FileSys::CreateUPtr( FST_DIRECTORY );

	StrBuf dst = GetExtSrvExtDir();

	t->Set( dst );
	t->MkDir( e );

	if( e->Test() )
	    return false;

	archiveDir->Rename( t.get(), e );

	if( e->Test() )
	    return false;

	dst = GetExtSrvExtDataDir();
	dst << "/nosuchfile";
	t->Set( dst );

	t->MkDir( e );

	if( e->Test() )
	    return false;
	
	archiveDir->ClearDeleteOnClose();

	return true;
}

bool ExtensionData::checkProductCompat( Error* e ) const
{
	if( compatProds.find( prod ) == compatProds.end() )
	{
	    StrBuf msg;
	    std::set< std::string > s( compatProds.begin(), compatProds.end() );
	    for( const auto& p : s )
	        msg << p.c_str() << ", ";
	    msg.SetLength( msg.Length() - 2 );
	    e->Set( MsgScript::ExtWrongProduct ) << prod.c_str() << msg;
	    return false;
	}

	if( known_api_versions.find( apiVersion ) == known_api_versions.end() )
	{
	    e->Set( E_FAILED, "Unknown api_version" );
	    return false;
	}

	return true;
}

bool ExtensionData::LoadMetadata( Error* e )
{
	return LoadManifest( e ) && !e->Test() && LoadTranslations( e );
}

bool ExtensionData::LoadManifest( Error* e )
{
	auto fsys = FileSys::CreateUPtr( FST_TEXT );
	auto filePath = PathSys::CreateUPtr();

	filePath->Set( archiveDir->Path() );
	filePath->SetLocal( *filePath, StrRef( "manifest.json" ) );

	fsys->Set( *filePath );
	fsys->Open( FOM_READ, e );

	if( e->Test() )
	    return false;

	StrBuf manifestBuf;
	fsys->ReadWhole( &manifestBuf, e );

	if( e->Test() )
	    return false;

	fsys->Close( e );

	if( e->Test() )
	    return false;

	json j;

	try {
	    j = json::parse( manifestBuf.Text() );
	} catch( const json::parse_error& err )
	{
	    StrBuf error;
	    error << "expected string, boolean or number; " << err.what();
	    e->Set( MsgSupp::ManifestParseError ) << error;
	    return false;
	}

	if( !j.size() || !j.is_object() )
	{
	    e->Set( MsgSupp::ManifestParseError ) << "not an object, or empty";
	    return false;
	}

	auto check = [&]( const json& js, const char* k,
	                  std::function< void() > fn = 0 )
	{
	    try {
	        if( js.find( k ) == js.end() )
	            throw( std::runtime_error("Unexpected end of JSON file") );
	        if( fn )
	            fn();
	    } catch( const std::exception& err )
	    {
	        e->Set( MsgSupp::ManifestKeyNotFound ) << k << err.what();
	        throw( err );
	    }
	};

	auto gs = [&]( const json& js, const char* k, StrBuf& v )
	{
	    check( js, k, [&](){ v = js[ k ].get< std::string >().c_str(); } );
	};

	auto gi = [&]( const json& js, const char* k, int& v )
	{
	    check( js, k, [&](){ v = js[ k ]; } );
	};

	auto gb = [&]( const json& js, const char* k, bool& v )
	{
	    check( js, k, [&](){ v = js[ k ]; } );
	};

	try {
	    gi( j, "api_version", apiVersion );
	    gi( j, "manifest_version", manifestVersion );
	    gs( j, "default_locale", defaultLocale );
	    gs( j, "description", description );
	    gs( j, "homepage_url", homepageUrl );
	    gs( j, "key", key );
	    gs( j, "license", license );
	    gs( j, "license_body", licenseBody );
	    gs( j, "name", name );
	    if( apiVersion == 1 )
	        namespc = "ExampleInc";
	    else
	        gs( j, "namespace", namespc );
	    gs( j, "version", version );
	    gs( j, "version_name", versionName );
	    gs( j[ "developer" ], "name", developerName );
	    gs( j[ "developer" ], "url", developerUrl );
	    gs( j[ "script_runtime" ], "language", runtimeLanguage );
	    gs( j[ "script_runtime" ], "version", runtimeVersion );
	    if( manifestVersion < 2 )
	        limitInstance = "multiple";
	    else
	        gs( j, "limit_instance", limitInstance );
	    if( manifestVersion < 2 )
	        requireUser = true;
	    else
	        gb( j, "require_user", requireUser );
	    check( j, "supported_locales" );
	    for( const auto& lc : j[ "supported_locales" ] )
	        supportedLocales.push_back( lc );
	    check( j, "compatible_products" );
	    if( apiVersion == 1 )
	        compatProds.insert( "p4d" );
	    else
	        for( const auto& pl : j[ "compatible_products" ] )
	        {
	            std::string p = pl;
	            std::transform( p.begin(), p.end(), p.begin(), ::tolower );
	            compatProds.insert( p );
	        }
	    } catch( const std::exception& err )
	    {
	        if( !e->Test() )
	        {
	            StrBuf msg;
	            msg << "Error processing manifest.json:  " << err.what();
	            e->Set( MsgSupp::ManifestParseError ) << msg.Text();
	        }

	        return false;
	    }

	DmtExtName extname;
	extname.ParseExt( namespc, DET_PREFIX, e );
	if( e->Test() )
	    return false;

	if( !UUID().Parse( key ) )
	{
	    e->Set( E_FAILED, "Error in the key field: Invalid UUID" );
	    return false;
	}

	if( runtimeLanguage != "Lua" )
	{
	    e->Set( E_FAILED, "Error in the runtimeLanguage field" );
	    return false;
	}

	if( runtimeVersion != "5.3" )
	{
	    e->Set( E_FAILED, "Error in the runtimeVersion field" );
	    return false;
	}

	if( limitInstance != "single" && limitInstance != "multiple" )
	{
	    e->Set( E_FAILED, "Error in the limitInstance field" );
	    return false;
	}

	extname.Clear();
	extname.ParseExt( name, DET_NAME, e );
	if( e->Test() )
	    return false;

	// todo: validate locales

	if( !ValidateManifest( e ) || e->Test() )
	    return false;

	return true;
}

bool ExtensionData::ValidateManifest( Error* e ) const
{
	return true;
}

bool ExtensionData::LoadTranslations( Error* e )
{
	// Validation of the manifest should prevent this from ever happening.

	if( !supportedLocales.size() )
	    return true;

	// If there's no _locales directory, the Ext might be doing their own
	// thing for translation but still wants to declare what they support, so
	// we don't error-out here.

	auto ldPath = PathSys::CreateUPtr();
	ldPath->Set( archiveDir->Path() );
	ldPath->SetLocal( *ldPath, StrRef( "_locales" ) );

	const auto fsys = FileSys::CreateUPtr( FST_DIRECTORY );

	if( !fsys->FileExists( ldPath->Text() ) )
	    return true;

	for( const auto& l : supportedLocales )
	{
	    auto fsys = FileSys::CreateUPtr( FST_TEXT );
	    auto filePath = PathSys::CreateUPtr();

	    filePath->Set( archiveDir->Path() );
	    StrBuf xlpath;
	    xlpath << "_locales" << "/" << l.c_str() << "/" << "messages.json";
	    filePath->SetLocal( *filePath, xlpath );
	    fsys->Set( *filePath );
	    fsys->Open( FOM_READ, e );

	    if( e->Test() )
	        return false;

	    StrBuf msgBuf;
	    fsys->ReadWhole( &msgBuf, e );

	    if( e->Test() )
	        return false;

	    fsys->Close( e );

	    if( e->Test() )
	        return false;

	    DEBUGPRINTF(EXTS_DEBUG, 
	                "LoadTranslation: read file at %s, size=%d\n", 
	                filePath->Text(), msgBuf.Length() );

	    json j;

	    try {
	        j = json::parse( msgBuf.Text() );
	    } catch( const json::parse_error& err )
	    {
	        StrBuf msg;
	        msg << "translation JSON parse error: " << err.what();
	        e->Set( MsgScript::ExtResourceErr ) << filePath->Text()
	                                            << msg;
	        return false;
	    }

	    // Make a rough check of what we're getting into.

	    if( !j.size() || !j.is_object() )
	    {
	        StrBuf msg;
	        msg << "translation JSON bad format:  not an object or is empty";
	        e->Set( MsgScript::ExtResourceErr ) << filePath->Text()
	                                            << msg;
	        return false;
	    }

	    std::unordered_map< std::string, std::string > msgMap;

	    try {
	    for( const auto& x : j.items() )
	    {
	        auto k = x.key();
	        auto v = x.value();
	        auto msg = v[ "message" ];
	        auto msgStr = msg.get< std::string >();
	        std::unordered_map< std::string, std::string > phs;

	        if( v.find( "placeholders" ) != v.end() )
	            for( const auto& p : v[ "placeholders" ].items() )
	            {
	                auto w = p.value()[ "content" ].get< std::string >();
	                std::transform( w.begin(), w.end(), w.begin(),
	                                ::tolower );
	                phs[ p.key() ] = w;
	            }

	        std::regex subs_rx( "\\$\\w+\\$", std::regex::icase );
	        auto words_begin = std::sregex_iterator( msgStr.begin(),
	                                                 msgStr.end(), subs_rx );

	        // Maps the literal string in the message to the
	        // replacements key:  var -> $VAR$
	        std::unordered_map< std::string, std::string > words;

	        for( std::sregex_iterator i = words_begin;
	             i != std::sregex_iterator(); ++i )
	        {
	            std::smatch match = *i;
	            std::string match_str = match.str();
	            auto w = match_str.substr( 1, match_str.length() - 2 );
	            std::transform( w.begin(), w.end(), w.begin(), ::tolower );
	            words[ w ] = match_str;
	        }

	        // All words exist in substitutions?

	        for( const auto& w : words )
	            if( phs.find( w.first ) == phs.end() )
	        {
	            StrBuf msg;
	            msg << "translation JSON substitution mismatch"
	                << w.first.c_str();
	            e->Set( MsgScript::ExtResourceErr ) << msg;
	            return false;
	        }

	        // All substitutions exist in words?

	        for( const auto& p : phs )
	            if( words.find( p.first ) == words.end() )
	        {
	            StrBuf msg;
	            msg << "translation JSON substitution mismatch"
	                << p.first.c_str();
	            e->Set( MsgScript::ExtResourceErr ) << msg;
	            return false;
	        }

	        // Convert placeholder $var$'s into positional $1's.
	        for( const auto& w : words )
	        {
	            // TODO: allow multiple replacements?
	            msgStr.replace( msgStr.find( w.second ), w.second.length(),
	                            phs[ w.first ] );
	        }

	        msgMap[ k ] = msgStr;
	    }
	    } catch( const std::exception& err )
	    {
	        StrBuf msg;
	        msg << "translation JSON bad format: " << err.what();
	        e->Set( MsgScript::ExtResourceErr ) << fsys->Name() << msg;
	        return false;
	    }

	    translationMap[ l ] = std::move( msgMap );
	}

	return true;
}

FileSysUPtr ExtensionData::Unzip( const StrBuf &zipFileName,
	                          p4_std_optional::optional< StrBuf > unzipPath,
	                          p4_std_optional::optional< StrBuf > oneFile, Error *e )
{
	// Make sure the temp directory is as close to the final location as
	// possible so we can avoid failing when rename() crosses partitions.

	auto p = PathSys::CreateUPtr();
	p->Set( *unzipPath );
	*p << "/nosuchfile";

	auto tmpZipDir = FileSys::CreateUPtr( FST_DIRECTORY );
	tmpZipDir->MakeLocalTemp( p->Text() );
	tmpZipDir->SetDeleteOnClose();

	unzFile *zipfile = (unzFile*) unzOpen64( zipFileName.Text() );

	if( zipfile == NULL )
	{
	    e->Set( MsgScript::ExtResourceErr ) << zipFileName
	        << "ExtensionData::Unzip: zipfile open failed";
	    unzClose( zipfile );
	    return nullptr;
	}

	unz_global_info global_info;

	if( unzGetGlobalInfo( zipfile, &global_info ) != UNZ_OK )
	{
	    e->Set( MsgScript::ExtResourceErr ) << zipFileName
	        << "ExtensionData::Unzip: could not read zipfile global info";
	    unzClose( zipfile );
	    return nullptr;
	}

	char read_buffer[ READ_ZIPSIZE ] = { 0 };

	// Loop to extract all zip entries
	for( unsigned long i = 0; i < global_info.number_entry; ++i )
	{
	    // Get info about current file
	    unz_file_info file_info;
	    char filename[ MAX_ZIPFILENAME ];
	    if( unzGetCurrentFileInfo( zipfile,
	                               &file_info,
	                               filename,
	                               MAX_ZIPFILENAME,
	                               NULL, 0, NULL, 0 ) != UNZ_OK )
	    {
	        e->Set( MsgScript::ExtResourceErr ) << zipFileName
	            << "ExtensionData::Unzip: could not read current zipfile "
	               "entry info";
	        unzClose( zipfile );
	        return nullptr;
	    }

	    if( oneFile && *oneFile != filename )
	        continue;

	    if( !FileSys::IsRelative( StrRef( filename ) ) )
	    {
	        e->Set( MsgScript::ExtResourceErr ) << zipFileName
	            << "ExtensionData::Unzip: Cannot process absolute path "
	               "in zipfile";
	        unzClose( zipfile );
	        return nullptr;
	    }

	    // check filename is made of valid chars [0-9a-zA-Z-._\/]
	    const char *p = filename;
	    while( isalnum( *p ) || *p == '-' || *p == '.' || *p == '_' ||
	                            *p == '/' || *p == '\\' ) ++p;

	    if( p == filename || *p != '\0' )
	    {
	        e->Set( MsgServer::ExtPkgBadFilename ) << filename;
	        unzClose( zipfile );
	        return nullptr;
	    }

            auto extp = PathSys::CreateUPtr();
	    extp->Set( tmpZipDir->Path() );
	    extp->SetLocal( *extp, StrRef( filename ) );

	    if( !extp->IsUnderRoot( *tmpZipDir->Path() ) )
	    {
	        e->Set( MsgServer2::ExtPkgPathTraversal ) << filename;
	        unzClose( zipfile );
	        return nullptr;
	    }

	    auto fsys = FileSys::CreateUPtr( FST_BINARY );
	    fsys->MkDir( *extp, e );

	    bool utf8check = false;

	    if( StrRef( filename ) == "main.lua" ||
	          StrRef( filename ) == "manifest.json" ||
	          strncmp(  filename, "_locales", strlen( "_locales" ) ) == 0 )
	        utf8check = true;

	    // extract entry content
	    if( unzOpenCurrentFile( zipfile ) != UNZ_OK )
	    {
	        e->Set( MsgScript::ExtResourceErr ) << zipFileName
	            << "ExtensionData::Unzip: could not open current "
	               "entry in zipfile";
	        unzClose( zipfile );
	        return nullptr;
	    }

	    // Open a file to write out the data
	    fsys->Set( *extp );
	    // TODO: restrict the file and paths more?
	    fsys->Perms( FPM_RO );
	    fsys->Open( FOM_WRITE, e );

	    int nBytes = UNZ_OK;

	    CharSetUTF8Valid validator;
	    int v = 1;

	    do    
	    {
	        nBytes = unzReadCurrentFile( zipfile, read_buffer, READ_ZIPSIZE );

	        if( nBytes < 0 )
	        {
	            e->Set( MsgScript::ExtResourceErr ) << zipFileName
	                << "ExtensionData::Unzip: could not read current"
	                    " zipfile entry";
	            unzCloseCurrentFile( zipfile );
	            unzClose( zipfile );
	            return nullptr;
	        }

	        if( nBytes > 0 )
	        {
	            fsys->Write( read_buffer, nBytes, e );

	            if( e->Test() )
	            {
	                e->Set( MsgScript::ExtResourceErr ) << zipFileName
	                    << "ExtensionData::Unzip: Write() error";
	                unzClose( zipfile );
	                return nullptr;
	            }

	            if( utf8check )
	            {
	                validator.Reset();
	                v = validator.Valid( read_buffer, nBytes );
	                if( v == 0 )
	                    break; // not UTF8, don't continue
	            }
	        }
	    } while ( nBytes > 0 );

	    if( v != 1 )
	        e->Set( MsgServer::FileNotUTF8 ) << filename;

	    fsys->Close( e );

	    if( e->Test() )
	    {
	        e->Set( MsgScript::ExtResourceErr ) << zipFileName
	            << "ExtensionData::Unzip: Close() error";
	        unzClose( zipfile );
	        return nullptr;
	    }

	    unzCloseCurrentFile( zipfile );

	    if( oneFile )
	        break;

	    // Go the the next entry listed in the zip file
	    if( ( i + 1 ) < global_info.number_entry )
	    {
	        if( unzGoToNextFile( zipfile ) != UNZ_OK )
	        {
	            e->Set( MsgScript::ExtResourceErr ) << zipFileName
	                << "ExtensionData::Unzip: could not read next "
	                    "zip entry";
	            unzClose( zipfile );
	            return nullptr;
	        }
	    }
	}

	unzClose( zipfile );
	return tmpZipDir;
}

# else

const char* ExtensionData::nameDelimiter = "::";
const char* ExtensionData::revDelimiter = "#";

# endif

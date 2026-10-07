/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * clientextpackage() - Perforce client extension package signing
 *
 */

# include <stdhdrs.h>

# include <strbuf.h>
# include <error.h>
# include <options.h>
# include <zipfile.h>
# include <debug.h>
# include <debugextension.h>
# include <filesys.h>
# include <pathsys.h>

# if defined( HAS_CPP11 ) && !defined( HAS_BROKEN_CPP11 ) && defined( USE_SSL )

# include <algorithm>
# include <functional>
# include <map>
# include <memory>
# include <string>
# include <unordered_map>
# include <vector>

# include <extsigndata.h>

# include <msgclient.h>
# include <msgsupp.h>
# include <msgdm.h>

const static ErrorId usage = { ErrorOf( 0, 0, E_FAILED, 0, 0 ),
	"Usage: p4 extension --package dir [ --sign keydir ]\n"
	"\tSee 'p4 help extension' for detailed help."
};

void DoSigning( const StrPtr *signDir, ExtSignData& signData,
	        const StrBuf& sDir, Error* e )
{
	PathSysUPtr privKey = PathSys::CreateUPtr();
	privKey->SetLocal( *signDir, StrRef( "privatekey.txt" ) );

	if( !FileSys::FileExists( privKey->Text() ) )
	{
	    e->Set( MsgSupp::MissingKeyCert ) << privKey->Text();
	    return;
	}

	// Verify .p4-certificate.txt exist directly under signDir

	FileSysUPtr srcCertFile = FileSys::CreateUPtr( FST_TEXT );
	PathSysUPtr srcCert = PathSys::CreateUPtr();

	// Note that the file name here doesn't have the '.p4-' prefix since
	// it is intended to match the server's auto-generated '-Gc' cert name.

	srcCert->SetLocal( *signDir, StrRef( "certificate.txt" ) );
	srcCertFile->Set( *srcCert );

	if( !(srcCertFile->Stat() & FSF_EXISTS) )
	{
	    e->Set( MsgSupp::MissingKeyCert ) << srcCert->Text();
	    return;
	}

	auto dirCheck = FileSys::CreateUPtr( FST_DIRECTORY );
	dirCheck->Set( signDir->Text() );

	if( (!dirCheck->HasOnlyPerm( FPM_RWXO )) &&
	    (!dirCheck->HasOnlyPerm( FPM_RXO )) )
	{
	    e->Set( MsgClient::PrivatekeyNotSecure ) << signDir->Text();
	    return;
	}

	// Put .p4-certificate.txt and .p4-signatures.json in the package.

	auto pkgCertFile = FileSys::CreateUPtr( FST_TEXT );
	auto pkgCert = PathSys::CreateUPtr();

	pkgCert->SetLocal( sDir, StrRef( ".p4-certificate.txt" ) );
	pkgCertFile->Set( *pkgCert );
	srcCertFile->Copy( pkgCertFile.get(), FPM_RW, e );

	if( e->Test() )
	    return;

	signData.AddFile( sDir, pkgCertFile->Name(), e );

	if( e->Test() )
	    return;

	auto p4sigpath = PathSys::CreateUPtr();
	p4sigpath->Set( sDir.Text() );
	p4sigpath->SetLocal( *p4sigpath, StrRef( ".p4-signatures.json" ) );

	signData.BuildSigFile( p4sigpath->Text(),
	                       srcCert->Text(), 
	                       privKey->Text(), e );

	if( e->Test() )
	    return;

	// Add this at the end since we don't want it to be included in itself.

	std::unordered_map< std::string, std::string > m;
	m[ "name" ] = ".p4-signatures.json";
	signData.GetFiles().emplace_back( m );
}

int
clientSignPackage( int argc, char **argv, Options &preops, Error *e )
{
	Options opts;
	int longOpts[] = { Options::Sign, Options::PkgExtension,
	                   Options::Help, Options::Quiet, 0 };
	opts.ParseLong( argc, argv, "", longOpts, OPT_ANY, usage, e );

	if( e->Test() )
	    return 1;

	const StrPtr *srcDir = opts[ Options::PkgExtension ];
	const StrPtr *signDir = opts[ Options::Sign ];

	if( *srcDir == "." || *srcDir == "/" || *srcDir == "\\" )
	{
	    e->Set( MsgDm::ExtensionBadDirectory );
	    return 1;
	}

	StrBuf sDir;

	if( srcDir->EndsWith( "/", 1 ) || srcDir->EndsWith( "\\", 1 ) )
	    sDir.Set( srcDir->Text(), srcDir->Length() - 1 );
	else
	    sDir.Set( srcDir->Text() );

	// Verify target file doesn't exist already.

	StrBuf targetFile( sDir );
	targetFile.Append( ".p4-extension" );

	DEBUGPRINTF( EXTS_INFO, "Package Extension for %s\n",
	             targetFile.Text());

	if( FileSys::FileExists( targetFile.Text() ) )
	{
	    e->Set( MsgClient::FileExists ) << targetFile;
	    return 1;
	}

	// Verify manifest.json exist directly under sDir

	PathSysUPtr manifestFile = PathSys::CreateUPtr();
	manifestFile->SetLocal( sDir, StrRef( "manifest.json" ) );

	if( !FileSys::FileExists( manifestFile->Text() ) )
	{
	    e->Set( MsgClient::NoSuchFile ) << manifestFile->Text();
	    return 1;
	}

	ExtSignData signData( &sDir, e );

	if( e->Test() )
	    return 1;

	if( signDir )
	    DoSigning( signDir, signData, sDir, e );

	if( e->Test() )
	    return 1;

	auto cleanup = FileSys::CreateUPtr( FST_BINARY );
	cleanup->Set( targetFile.Text() );
	cleanup->SetDeleteOnClose();

	ZipFile zf;
	zf.Open( targetFile.Text(), e );

	if( e->Test() )
	    return 1;

	const int bufSize = FileSys::BufferSize();
	std::vector< char > buffer( bufSize );
	int rsz = 0;

	for( auto& fe : signData.GetFiles() )
	{
	    const auto path = fe[ "name" ];

	    zf.StartEntry( path.c_str(), e );

	    if( e->Test() )
	        return 1;

	    auto ps = PathSys::CreateUPtr();
	    ps->SetLocal( sDir, StrRef( path.c_str() ) );
	    auto fs = FileSys::CreateUPtr( FST_BINARY );
	    fs->Set( *ps );
	    fs->Open( FOM_READ, e );

	    if( e->Test() )
	        return 1;

	    while( ( rsz = fs->Read( buffer.data(), bufSize, e ) )
	           && !e->Test() )
	        zf.AppendBytes( buffer.data(), rsz, e );

	    fs->Close( e );

	    if( e->Test() )
	        return 1;

	    zf.FinishEntry( e );

	    if( e->Test() )
	        return 1;
	}

	zf.Close();

	if( e->Test() )
	    return 1;

	Error out;
	StrBuf outmsg;
	out.Set( MsgSupp::ExtensionPackingSuccess );
	out.Fmt( outmsg, EF_NEWLINE );
	printf( "%s", outmsg.Text() );

	cleanup->ClearDeleteOnClose();

	return 0;
}

# else

int
clientSignPackage( int argc, char **argv, Options &preops, Error *e )
{
	fprintf( stderr, "Unimplemented in this build\n" );
	return 1;
}

# endif // HAS_CPP11 !HAS_BROKEN_CPP11 USE_SSL

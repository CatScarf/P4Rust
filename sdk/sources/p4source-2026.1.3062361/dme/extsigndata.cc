/*
 * Copyright 1995, 2020 Perforce Software.  All rights reserved.
 */

# include <stdhdrs.h>

# if defined( HAS_CPP11 ) && !defined( HAS_BROKEN_CPP11 ) && defined( USE_SSL )

# include <algorithm>
# include <functional>
# include <memory>
# include <set>
# include <unordered_map>
# include <vector>
# include <string>

# include <strbuf.h>
# include <strarray.h>
# include <error.h>
# include <sha256.h>
# include <debug.h>

# include <filesys.h>
# include <pathsys.h>

# include <msgsupp.h>

# include <extsigndata.h>

extern "C"
{
# include "openssl/err.h"
# include <openssl/x509v3.h>
# include <openssl/ssl.h>
# include <openssl/x509_vfy.h>
}

# include <json.hpp>
using json = nlohmann::json;

// Differences from Base64::Encode():
//   1) Output text is properly null terminated.
//   2) The output is smart pointer managed, preventing memory leak.
//   3) The output format is multilined, matching opessl command output.

static std::unique_ptr< char[] > Base64Encode( const unsigned char* buffer,
	                                       const size_t length )
{
	BIO *bio, *b64;
	BUF_MEM *bufferPtr;
	b64 = BIO_new( BIO_f_base64() );
	bio = BIO_new( BIO_s_mem() );
	bio = BIO_push( b64, bio );
	BIO_write( bio, buffer, length );
	(void)BIO_flush( bio );
	BIO_get_mem_ptr( bio, &bufferPtr );

	auto base64Text = std::make_unique< char[] >( bufferPtr->length + 1 );
	memcpy( base64Text.get(), bufferPtr->data, bufferPtr->length );
	base64Text[ bufferPtr->length ] = '\0';

	BIO_free_all( b64 );
	return base64Text;
}

static size_t calcDecodeLength( const char* b64input )
{
	size_t len = strlen( b64input ), padding = 0;
	if( b64input[ len - 1 ] == '=' && b64input[ len - 2 ] == '=' )
	    //last two chars are =
	    padding = 2;
	else if( b64input[ len - 1 ] == '=' ) // last char is =
	    padding = 1; 
	return ( len * 3 ) / 4 - padding;
}

// Same as Base64::Decode(), but the output is smart pointer managed,
// preventing memory leaks.

static void Base64Decode( const char* b64message,
	                  std::unique_ptr< unsigned char[] >& buffer,
	                  size_t* length )
{
	BIO *bio, *b64;
	int decodeLen = calcDecodeLength( b64message );
	buffer = std::make_unique< unsigned char[] >( decodeLen + 1 );
	buffer[ decodeLen ] = '\0';
	bio = BIO_new_mem_buf( b64message, -1 );
	b64 = BIO_new( BIO_f_base64() );
	bio = BIO_push( b64, bio );
	*length = BIO_read( bio, buffer.get(), strlen( b64message ) );
	BIO_free_all( b64 );
}

ExtSignData::ExtSignData( const StrPtr *dir, Error *e )
{
	StrBuf pkgDir;

	if( dir->EndsWith( "/", 1 ) || dir->EndsWith( "\\", 1 ) )
	    pkgDir.Set( dir->Text(), dir->Length() - 1 );
	else
	    pkgDir.Set( dir->Text() );

	std::set< std::string > skips;
	auto sigFile = PathSys::CreateUPtr();
	sigFile->SetLocal( pkgDir, StrRef( ".p4-signatures.json" ) );
	skips.insert( sigFile->Text() );

	std::vector< std::string > pDirs, pFiles;
	pDirs.push_back( pkgDir.Text() );

	do
	{
	    const auto dir = pDirs.back();
	    pDirs.pop_back();

	    // Note that there's no SetCharset() calls like in
	    // ClientScript::FindLooseExts() because we mandate that
	    // all file names be UTF-8.

	    auto dfsys = FileSys::CreateUPtr( FST_DIRECTORY );
	    dfsys->Set( dir.c_str() );

	    std::unique_ptr< StrArray, std::function< void( StrArray* ) > >
	        a( dfsys->ScanDir( e ), [&]( StrArray* ptr ){ delete ptr; } );

	    if( e->Test() )
	        return;

	    for( int i = 0; i < a->Count(); i++ )
	    {
	        auto path = PathSys::CreateUPtr();
	        path->SetLocal( StrRef( dir.c_str() ), *(a->Get( i )) );

	        auto file = FileSys::CreateUPtr( FST_TEXT );
	        file->Set( *path );

	        if( skips.find( file->Name() ) != skips.end() )
	            continue;

	        if( file->Stat() & FSF_DIRECTORY )
	            pDirs.emplace_back( file->Name() );
	        else
	            pFiles.emplace_back( file->Name() );
	    }
	}
	while( pDirs.size() );

	for( const auto& f : pFiles )	
	{
	    AddFile( pkgDir, f.c_str(), e );

	    if( e->Test() )
	        return;
	}
}

// Builds the corresponding JSON
// If sig is null, it returns json output of Filelist only (to be 
//     signed or verified)
// If sig is not null, it builds the entire json output (what would be
//     written to the .p4-signatures.json file)
// IMPORTANT: This function MUST BE the same used by both the client
//            and the server so the content that must be signed
//            is identical.

bool
ExtSignData::BuildSigList( StrBuf *sig, StrBuf *jout, Error *e )
{
	json p4sig, siglist = json::array();
	std::string joutput;
	StrBuf sjoutput;

	jout->Clear();

	// Sort for signature stability.  Must be the same across platforms.

	std::sort( files.begin(), files.end(),
	    [] ( auto& lhs, auto& rhs )
	    {
	        return lhs[ "name" ] < rhs[ "name" ];
	    } );

	try {
	    for( auto& fe : files )
	    {
	        json js;
	        js[ "file" ] = fe[ "name" ];
	        js[ "SHA256" ] = fe[ "hash" ];
	        siglist.emplace_back( std::move( js ) );
	    }

	    joutput = siglist.dump( 4 );

	    if( !sig )
	    {
	        jout->Append( joutput.c_str() );
	        return true;
	    }

	    p4sig[ "filelist" ] = siglist;
	    p4sig[ "signature" ] = sig->Text();

	    sjoutput.Set( p4sig.dump( 4 ).c_str() );
	    sjoutput.Append( "\n" );
	}
	catch( const std::exception& err )
	{
	    e->Set( MsgSupp::ManifestParseError ) << err.what();
	    return false;
	}

	jout->Set( &sjoutput );

	return true;
}

bool
ExtSignData::SignData( const char *certFile,
	               const char *privKey,
	               const StrBuf *tosign,
	               StrBuf &sig64, Error *e )
{
	unsigned char *sig = nullptr;
	EVP_MD_CTX *mdctx = nullptr;
	EVP_PKEY *privateKey = nullptr;
	X509 *certificate = nullptr;
	FILE *fp;
	size_t slen = 0;

	std::unique_ptr< char[], std::function< void( char[] ) > >
	    Cleanup( new char[ 1 ], [&]( char* x )

	{
	    delete[] x;
	    EVP_PKEY_free( privateKey );
	    X509_free( certificate );
	    OPENSSL_free( sig );
	    EVP_MD_CTX_destroy( mdctx );
	} );

	FileSysUPtr pf = FileSys::CreateUPtr( FST_TEXT );
	pf->Set( StrRef( privKey ) );

	if( !pf->FileExists( privKey ) )
	{
	    e->Set( MsgSupp::MissingKeyCert ) << privKey;
	    return false;
	}

	FileSysUPtr cf = FileSys::CreateUPtr( FST_TEXT );
	cf->Set( certFile );

	if( !cf->FileExists( certFile ) )
	{
	    e->Set( MsgSupp::MissingKeyCert ) << certFile;
	    return false;
	}

	fp = fopen( privKey, "r" );

	if( fp == NULL )
	{
	    e->Set( MsgSupp::MissingKeyCert ) << privKey;
	    return false;
	}

	privateKey = PEM_read_PrivateKey( fp, nullptr, 0, nullptr );
	fclose( fp );
	fp = fopen( certFile, "r" );

	if( fp == NULL )
	{
	    e->Set( MsgSupp::MissingKeyCert ) << certFile;
	    return false;
	}

	certificate = PEM_read_X509( fp, nullptr, 0, nullptr );
	fclose( fp );

	if( !( mdctx = EVP_MD_CTX_create() ) )
	{
	    e->Set( MsgSupp::InternalSSLerror ) << "EVP_MD_CTX_create";
	    return false;
	}

	// Initialise the DigestSign operation - with SHA-256 as digest function
	if( EVP_DigestSignInit( mdctx, nullptr, EVP_sha256(), nullptr, privateKey ) != 1 )
	{
	    e->Set( MsgSupp::InternalSSLerror ) << "EVP_DigestSignInit";
	    return false;
	}

	if( EVP_DigestSignUpdate( mdctx, tosign->Text(), tosign->Length() ) != 1 )
	{
	    e->Set( MsgSupp::InternalSSLerror ) << "EVP_DigestSignUpdate";
	    return false;
	}

	// Finalise the DigestSign operation
	// First call EVP_DigestSignFinal with a NULL sig parameter to obtain
	// the length of the signature. Length is returned in slen

	if( EVP_DigestSignFinal( mdctx, NULL, &slen ) != 1 )
	{
	    e->Set( MsgSupp::InternalSSLerror ) << "EVP_DigestSignFinal";
	    return false;
	}

	if( !( sig = (unsigned char*)
	        OPENSSL_malloc( sizeof(unsigned char) *slen ) ) )
	{
	    e->Set( MsgSupp::InternalSSLerror ) << "OPENSSL_malloc";
	    return false;
	}

	if( EVP_DigestSignFinal( mdctx, sig, &slen ) != 1 )
	{
	    e->Set( MsgSupp::InternalSSLerror ) << "EVP_DigestSignFinal";
	    return false;
	}

	sig64 = Base64Encode( sig, slen ).get();

	return true;
}

bool
ExtSignData::BuildSigFile( const char *sigFile,
	                   const char *certFile,
	                   const char *privKey,
	                   Error *e )
{
	StrBuf tosign, sig64, p4sig;

	BuildSigList( nullptr, &tosign, e );

	if( e->Test() )
	    return false;

	SignData( certFile, privKey, &tosign, sig64, e );

	if( e->Test() )
	    return false;

	BuildSigList( &sig64, &p4sig, e );

	if( e->Test() )
	    return false;

	auto p4sigf = FileSys::CreateUPtr( FST_TEXT );
	p4sigf->Set( sigFile );
	p4sigf->Perms( FPM_RW );
	p4sigf->Open( FOM_WRITE, e );

	if( e->Test() )
	    return false;

	p4sigf->WriteFile( &p4sig, e );

	if( e->Test() )
	    return false;

	p4sigf->Close( e );

	if( e->Test() )
	    return false;

	return true;
}

StrBuf
ExtSignData::HashFile( const char *fname, Error *e ) const
{
	StrBuf buffer, hash;
	Sha256Digester digester;
	int l = 0;
	buffer.Alloc( 8192 );
	FileSysUPtr f = FileSys::CreateUPtr( FST_BINARY );

	f->Set( fname );
	f->Open( FOM_READ, e );

	if( e->Test() )
	    return hash;

	while( ( l = f->Read( buffer.Text(), 8192, e ) ) && !e->Test() )
	{
	    buffer.SetLength( l );
	    digester.Update( buffer );
	}

	digester.Final( hash );

	return hash;
}

bool
ExtSignData::VerifyData( const char *certFile,
	                 const StrBuf &signedData,
	                 StrBuf &sig64, Error *e ) const
{
	EVP_MD_CTX *mdctx = nullptr;
	EVP_PKEY *publickey = nullptr;
	BIO *certbio = nullptr;
	X509 *cert = nullptr;
	certbio = BIO_new( BIO_s_file() );
	unsigned int fpbin_size = 0;
	unsigned char fpbin[ EVP_MAX_MD_SIZE ];
	size_t sigMsgLen = 0;
	std::unique_ptr< unsigned char[] > sigMsg;

	std::unique_ptr< char[], std::function< void( char[] ) > >
	    Cleanup( new char[ 1 ], [&]( char* x )

	{
	    delete[] x;
	    EVP_MD_CTX_destroy( mdctx );
	    X509_free( cert );
	    EVP_PKEY_free( publickey );
	    cert = nullptr;
	    BIO_free_all( certbio );
	} );

	// Sets certbio to read from file, but not reading yet
	if( BIO_read_filename( certbio, certFile ) != 1 )
	{
	    StrRef reason( "BIO_read_filename" );
	    e->Set( MsgSupp::InternalSSLerror ) << reason;
	    return false;
	}

	// cert file is in PEM format ( base64 )
	cert = X509_new();
	if( ! ( PEM_read_bio_X509( certbio, &cert, 0, nullptr ) ) )
	{
	    StrRef reason( "PEM_read_bio_X509" );
	    e->Set( MsgSupp::InternalSSLerror ) << reason;
	    return false;
	}
	
	if( !X509_digest( cert, EVP_sha1(), fpbin, &fpbin_size ) )
	{
	    StrRef reason( "x509_digest" );
	    e->Set( MsgSupp::InternalSSLerror ) << reason;
	    return false;
	}

	if( ( publickey = X509_get_pubkey( cert ) ) == nullptr )
        {
	    StrRef reason( "x509_get_pubkey" );
	    e->Set( MsgSupp::InternalSSLerror ) << reason;
	    return false;
	}

	Base64Decode( sig64.Text(), sigMsg, &sigMsgLen );

	mdctx = EVP_MD_CTX_create();
	EVP_MD_CTX_init( mdctx );

	if( EVP_DigestVerifyInit( mdctx, nullptr, EVP_sha256(), nullptr, 
	                          publickey ) != 1 )
	{
	    StrRef reason( "EVP_DigestVerifyInit" );
	    e->Set( MsgSupp::InternalSSLerror ) << reason;
	    return false;
	}

	if( EVP_DigestVerifyUpdate( mdctx, 
	                            signedData.Text(),
	                            signedData.Length() ) != 1 )
	{
	    StrRef reason( "EVP_DigestVerifyUpdate" );
	    e->Set( MsgSupp::InternalSSLerror ) << reason;
	    return false;
	}

	if( EVP_DigestVerifyFinal( mdctx, sigMsg.get(), sigMsgLen ) != 1 )
	{
	    e->Set( MsgSupp::ExtSignatureFailure );
	    return false;
	}

	return true;
}

// Given the provided .p4-signatures.json file and certificate file
// 1) Extract list of files and corresponding sha256
//    encoded in json format in the .p4-signatures.json
// 2) Verify the sha256 matches the sha256 in the directory

bool
ExtSignData::VerifySigFile( const char *pkgSigfile,
	                    const char *certFile,
	                    Error *e )
{
	// This function assumes the caller verified the certificate file 
	// is one that we trust
	
	auto sigFile = FileSys::CreateUPtr( FST_TEXT );
	sigFile->Set( pkgSigfile );
	sigFile->Open( FOM_READ, e );

	if( e->Test() )
	    return false;

	StrBuf buf;
	json sigs;

	sigFile->ReadWhole( &buf, e );

	try
	{
	sigs = json::parse( buf.Text() );

	auto const sig = sigs.find( "signature" );

	if( sig == sigs.end() )
	{
	    e->Set( MsgSupp::InvalidFormatSigFile );
	    return false;
	}

	StrBuf sig64( (*sig).get< std::string >().c_str() );

	auto flist = sigs.find( "filelist" ); 

	if( flist == sigs.end() )
	{
	    e->Set( MsgSupp::InvalidFormatSigFile );
	    return false;
	}

	StrBuf sigList( flist->dump( 4 ).c_str() ), diskList;

	BuildSigList( nullptr, &diskList, e );

	if( e->Test() )
	    return false;

	if( sigList != diskList )
	{
	    e->Set( MsgSupp::ExtSignatureFailure );
	    return false;
	}

	return VerifyData( certFile, diskList, sig64, e );

	}
	catch( const std::exception& err )
	{
	    e->Set( MsgSupp::ManifestParseError ) << err.what();
	    return false;
	}

	return false;
}

void ExtSignData::AddFile( const StrBuf &pkgDir, const char *file, Error *e )
{
	std::unordered_map< std::string, std::string > data;

	auto& name = data[ "name" ] = file + pkgDir.Length() + 1;
	// Normalize to slashes so the signature is cross-platform.
	std::replace( name.begin(), name.end(), '\\', '/' );

	data[ "hash" ] = HashFile( file, e ).Text();

	files.emplace_back( std::move( data ) );
}

std::vector< std::unordered_map< std::string, std::string > >&
ExtSignData::GetFiles()
{
	return files;
}

# endif // HAS_CPP11 !HAS_BROKEN_CPP11 USE_SSL

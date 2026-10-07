/*
 * Copyright 1995, 2024 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# ifdef USE_CDC

# include <error.h>
# include <strbuf.h>
# include <filesys.h>
# include <debug.h>
# include <strops.h>
# include <json.hpp>
# include <md5.h>
# include <msgdm2.h>
# include <vartree.h>

# include <string>
# include <cstdint>
# include <inttypes.h>

# include <tunable.h>
# include <intarray.h>
# include "blake3digester.h"
# include "fastcdc.h"
# include "chunkmap.h"

# include <vartree.h>
# include <vararray.h>
# include <errorlog.h>

class ChunkVTree : public VVarTree
{
    public:
	ChunkVTree() {}
	virtual ~ChunkVTree()
	{
	    Clear();
	}

	virtual int Compare( const void *a, const void *b ) const
	{
	    const ChunkMap::Chunk *ca = (const ChunkMap::Chunk *)a;
	    const ChunkMap::Chunk *cb = (const ChunkMap::Chunk *)b;
	    return ca->hash.XCompare( cb->hash );
	}

	virtual void *Copy( const void *src ) const
	{
	    return new ChunkMap::Chunk( *(const ChunkMap::Chunk *)src );
	}

	virtual void Delete( void *a ) const
	{
	    delete (ChunkMap::Chunk *)a;
	}

	virtual void Dump( void *a, StrBuf &buf ) const
	{
	    buf = ((ChunkMap::Chunk *)a)->hash;
	}
} ;

class ChunkVArray : public VVarArray
{
    public:
	ChunkVArray() {}
	virtual ~ChunkVArray()
	{
	    int n = Count();
	    for( int i = 0; i < n; i++ )
	    {
	        Destroy( Get( i ) );
	    }
	}

	virtual int Compare( const void *a, const void *b ) const
	{
	    const ChunkMap::Chunk *ca = (const ChunkMap::Chunk *)a;
	    const ChunkMap::Chunk *cb = (const ChunkMap::Chunk *)b;
	    return ca->hash.XCompare( cb->hash );
	}

	virtual void Destroy( void *a ) const
	{
	    delete (ChunkMap::Chunk *)a;
	}
} ;

ChunkMap::Chunk::Chunk()
{
}

ChunkMap::Chunk::Chunk( const Chunk& rhs )
{
	hash   = rhs.hash;
	size   = rhs.size;
	offset = rhs.offset;
}

ChunkMap::Chunk&
ChunkMap::Chunk::operator =( ChunkMap::Chunk&& rhs )
{
	hash   = std::move( rhs.hash );
	size   = rhs.size;
	offset = rhs.offset;
	return *this;
}

bool
ChunkMap::Chunk::operator <( const Chunk& rhs ) const
{
	return hash < rhs.hash;
}

bool
ChunkMap::Chunk::operator ==( const Chunk& rhs ) const
{
	return hash == rhs.hash;
}

ChunkMap::ChunkMap()
{
}

void
ChunkMap::SetBuf( StrPtr* map, Error* e )
{
	if( ownBuf )
	    delete (StrBuf*)mapBuf;
	ownBuf = false;
	mapBuf = map;
	Parse( "SetBuf", "", e );
}

void
ChunkMap::CopyBuf( StrPtr* map, Error* e )
{
	if( ownBuf )
	    delete (StrBuf*)mapBuf;

	ownBuf = true;
	mapBuf = new StrBuf( *map );
	Parse( "CopyBuf", "", e );
}

ChunkMap::ChunkMap( StrPtr* map, Error* e )
{
	CopyBuf( map, e );
}

ChunkMap::~ChunkMap()
{
	// It may be owned by an RPC buffer.

	if( ownBuf )
	    delete (StrBuf*)mapBuf;
}

ChunkMap& ChunkMap::operator=( ChunkMap&& other )
{
	this->version = other.version;

	if( ownBuf )
	    delete (StrBuf*)mapBuf;

	mapBuf = other.mapBuf;
	other.mapBuf = nullptr;

	memcpy( verify_hash, other.verify_hash, sizeof( verify_hash ) );
	this->ownBuf = other.ownBuf;
	this->numChunks = other.numChunks;

	return *this;
}

int
ChunkMap::GetVersion() const
{
	return version;
}

const uint8_t *
ChunkMap::GetVerifyHash() const
{
	return verify_hash;
}

size_t
ChunkMap::ChunkCount() const
{
	return numChunks;
}

const StrPtr* const
ChunkMap::GetBuf() const
{
	return mapBuf;
};

void
ChunkMap::ResetIterator()
{
	offset = 0;
	pos = nullptr;
}

void
ChunkMap::GetVerifyHash( StrBuf& digestBuf ) const
{
	StrOps::OtoX( GetVerifyHash(), BLAKE3_BUFFLEN, digestBuf );
}

void
ChunkMap::DumpJSON( StrBuf& out )
{
	ResetIterator();
	using json = nlohmann::json;

	json doc, cs;

	Chunk c;

	while( GetNextChunk( c ) )
	{
	    json a;
	    a[ "offset" ] = c.offset;
	    a[ "size"   ] = c.size;
	    a[ "hash"   ] = c.hash.Text();
	    cs.emplace_back( a );
	}

	if( numChunks == 0 )
	    cs = json::array();

	StrBuf digestBuf;
	StrOps::OtoX( GetVerifyHash(), BLAKE3_BUFFLEN, digestBuf );

	doc[ "version" ] = GetVersion();
	doc[ "verify_hash" ] = digestBuf.Text();
	doc[ "chunks" ] = cs;

	out << doc.dump( 4 ).c_str() << "\n";

	ResetIterator();
}

size_t
ChunkMap::GetCDCMinSize()
{
	return cdc_min_size;
}

size_t
ChunkMap::GetCDCAvgSize()
{
	return cdc_avg_size;
}

size_t
ChunkMap::GetCDCMaxSize()
{
	return cdc_max_size;
}


// Note that our BLAKE3 class isn't used here since the interface isn't
// convenient for these datatypes.

void
ChunkMap::Create( const StrPtr* fromFile, Error* e )
{
	auto in = FileSys::CreateUPtr( FST_BINARY );

	in->Set( *fromFile );
	in->Open( FOM_READ, e );

	if( e->Test() )
	    return;

	Create( in.get(), e, nullptr );

	in->Close( e );

	return;
}

void
ChunkMap::Create( FileSys* in, Error* e, MD5* md5 )
{
	// Be nice to callers that might not expect the file
	// position to change out of underneath them.

	const offL_t origPos = in->Tell();

	in->Seek( 0, e );

	if( e->Test() )
	    return;

	// Set up a checksum for the whole structure.

	BLAKE3 b3;

	// Version goes first.

	const char* version_cast = reinterpret_cast< const char* >( &version );
	b3.Update( version_cast, sizeof( version ) );
	((StrBuf*)mapBuf)->Append( version_cast, sizeof( version ) );

	// Fixed-size buffer to copy into while processing.  Limits the
	// amount of memory copies into the final buffer. ???

	const int max_chunks = 10000;
	StrBuf out_buf;
	out_buf.BlockAlloc( ChunkMap::DiskChunkSize() * max_chunks );

	// Helper to move the temp buffer data into the final buffer.
	class ChunkHandler : public cdc_ft::fastcdc::Chunker
	{
	public:
	    ChunkHandler( const cdc_ft::fastcdc::Config& cfg,
	        BLAKE3& b3, MD5 *md5,
	        StrBuf& buf_in, StrBuf& buf_out ) :
	        cdc_ft::fastcdc::Chunker( cfg ),
	        b3( b3 ), md5( md5 ),
	        mapBuf( buf_in ), out_buf( buf_out ),
	        nChunks( 0 ), nTotalChunks( 0 )
	    {
	    }

	    BLAKE3 &b3;
	    MD5 *md5;
	    StrBuf &mapBuf;
	    StrBuf &out_buf;
	    
	    // Number of accumulated chunks in the write buffer.
	    size_t nChunks;
	    size_t nTotalChunks;

	    void write_buf()
	    {
	        const size_t wsz = nChunks * ChunkMap::DiskChunkSize();
	        mapBuf.Append( out_buf.Text(), wsz );
	        b3.Update( out_buf.Text(), wsz );

	        nTotalChunks += nChunks;
	        nChunks = 0;
	    }

	    // Callback for the CDC hasher, called once per chunk.

	    virtual void HandleOneChunk(const uint8_t* data, size_t len)
	    {
	        if( nChunks == max_chunks - 1 )
	            write_buf();

	        const size_t pos = nChunks * ChunkMap::DiskChunkSize();
	        // todo: LE vs BE
	        // todo: add a static assert showing that there are no higher-order bits
	        memcpy( out_buf.Text() + pos, &len, sizeof( chunkSize_t ) );
	        BLAKE3::Digest( (const char*)data, len,
	                        (unsigned char *)out_buf.Text() + pos + sizeof( chunkSize_t ) );
	        nChunks++;
	        if( md5 )
	            md5->Update( data, len );
	    }

	    void Finish()
	    {
	        if( nChunks )
	            write_buf();
	    }
	};

	// File to truffle-shuffle.

	const cdc_ft::fastcdc::Config cdc_cfg( cdc_min_size, cdc_avg_size,
	                                       cdc_max_size );
	ChunkHandler chunker( cdc_cfg, b3, md5, *(StrBuf*)mapBuf, out_buf );

	// I/O buffer.

	StrBuf in_buf;
	in_buf.BlockAlloc( 32768 * 2 );
	size_t cnt = 0;

	while( ( cnt = in->Read( in_buf.Text(), in_buf.Length(), e ) )
	       && !e->Test() )
	{
	    chunker.Process( (const uint8_t *)in_buf.Text(), cnt );
	}

	chunker.Finalize();

	// Finish a partially-filled buffer.

	chunker.Finish();
	numChunks = chunker.nTotalChunks;

	StrBuf final_hash;
	final_hash.BlockAlloc( BLAKE3_BUFFLEN );
	b3.Final( (unsigned char*)final_hash.Text() );

	((StrBuf*)mapBuf)->Append( &final_hash );

	in->Seek( origPos, e );
	Parse( "Create", "", e );
}

void
ChunkMap::Write( const StrPtr* toFile, Error* e ) const
{
	auto out = FileSys::CreateUPtr( FST_BINARY );

	out->Set( *toFile );

	// Write to a temp file, then rename
	auto tmp = FileSys::CreateUPtr( FST_BINARY );
	tmp->MakeLocalTemp( out->Name() );
	tmp->SetDeleteOnClose();
	tmp->Open( FOM_WRITE, e );
	if( e->Test() )
	    return;

	tmp->Write( mapBuf, e );
	tmp->Perms( FPM_RW );
	tmp->Close( e );
	if( e->Test() )
	    return;

	tmp->Rename( out.get(), e );
	if( e->Test() )
	{
	    // Let rename fail silently because sometimes
	    // the problem is a collision with 
	    // another process renaming it's tmp file.
	    AssertLog.Report( e );
	    e->Clear();
	}
	else
	{
	    out->Chmod( FPM_RO, e );
	}
}

bool
ChunkMap::Parse( const char* source, const char* name, Error* e )
{
	// The first check is to see if the size of the map is big enough
	// to fit the smallest-possible ChunkMap.

	const P4INT64 mapSize = mapBuf->Length();

	if( mapSize < DiskMapMininumSize() )
	{
	    e->Set( MsgDm2::ChunkMapFormat ) << source << name << "minimum size";
	    return false;
	}

	// The next check is to see if the size of the chunk data fits in a
	// multiple of a single chunk size.  If not, then it's clearly busted.

	char* pos = mapBuf->Text();

	// Region where chunks live;  total size - header version and trailing hash
	const size_t chunkSpaceSize = mapBuf->Length() - sizeof( version ) - BLAKE3_BUFFLEN;
	const size_t badFit = chunkSpaceSize % DiskChunkSize();

	if( badFit )
	{
	    e->Set( MsgDm2::ChunkMapFormat ) << source << name << "bad chunk space";
	    return false;
	}

	// We may be reading something from the future, so don't pretend to understand it.

	mapVersion_t v;
	memcpy( (void*) &v, pos, sizeof( version ) );

	if( v != version )
	{
	    e->Set( MsgDm2::ChunkMapFormat ) << source << name << "unsupported version!";
	    return false;
	}

	// Now compute the digest the file and compare that to what's in the file.
	//
	// This verification could be made optional to save some CPU, but the
	// risk of a bad map causing corrupted new archives is not appealing.

	pos += sizeof( version );

	// Total file size minus the leading version and the trailing
	// verification hash = region for data.

	const unsigned char* dgo = (const unsigned char*) mapBuf->Text() +
	                           ( mapSize - sizeof( verify_hash ) );

	memcpy( verify_hash, dgo, sizeof( verify_hash ) );

	uint8_t in_verify_hash[ BLAKE3_BUFFLEN ];
	BLAKE3::Digest( mapBuf->Text(), mapSize - sizeof( verify_hash ),
	                in_verify_hash );

	if( memcmp( in_verify_hash, verify_hash, sizeof( Chunk::hash ) ) )
	{
	    e->Set( MsgDm2::ChunkMapFormat ) << source << name << "digest mismatch!";
	    return false;
	}

	// Verification is now done.

	numChunks = chunkSpaceSize / DiskChunkSize();
	return true;
}

bool
ChunkMap::Validate( const size_t expectedSize, Error* e )
{
	Chunk c;
	offL_t prevOff = 0;
	size_t prevSize = 0;
	size_t totalSize = 0;
	P4INT64 n = 0, nChunks = ChunkCount();

	while( GetNextChunk( c ) )
	{
	    // If the size of the max chunk changes, pre-existing maps, or
	    // maps created with earlier versions of the code will run
	    // afoul of this.  Note that the last chunk in the file can be
	    // smaller than the minimum size.

	    if( c.size > GetCDCMaxSize() ||
	        ( n != ( nChunks - 1 ) && ( c.size < GetCDCMinSize() ) ) )
	    {
	        e->Set( MsgDm2::ChunkMapFormat ) << "" << "" << "bad chunk size";
	        return false;
	    }

	    // Make sure chunks are contiguous with each other.

	    if( totalSize && ( prevOff + prevSize ) != c.offset )
	    {
	        e->Set( MsgDm2::ChunkMapFormat ) << "" << "" << "chunk out of order";
	        return false;
	    }

	    // The hash is left unverified, as this is a map-only check and
	    // the file data isn't necessarily available yet when this is called.

	    prevOff = c.offset;
	    prevSize = c.size;
	    totalSize += c.size;
	    n++;
	}

	if( totalSize != expectedSize )
	{
	    e->Set( MsgDm2::ChunkMapFormat ) << "" << ""
	        << "map size does not match expected size";
	    return false;
	}

	ResetIterator();
	return true;
}

void
ChunkMap::Read( const StrPtr* fromFile, Error* e )
{
	auto in = FileSys::CreateUPtr( FST_BINARY );

	in->Set( *fromFile );
	in->Open( FOM_READ, e );

	if( e->Test() )
	    return;

	size_t cnt = 0;

	// Reads the entire map file into memory at once.

	const offL_t mapSize = in->GetSize();

	((StrBuf*)mapBuf)->BlockAlloc( mapSize );

	cnt = in->Read( mapBuf->Text(), mapSize, e );

	if( e->Test() )
	    goto END;

	if( cnt < mapSize )
	{
	    e->Sys( "ChunkMap::Read", "short" );
	    goto END;
	}

	if( !Parse( "disk", fromFile->Text(), e ) )
	    goto END;

END:
	in->Close( e );
	return;
}

bool
ChunkMap::GetNextChunk( Chunk& chunk )
{
	if( mapBuf->Length() == 0 )
	    return false;

	// Not that it's likely for there to be any contention we care about
	// here, but we could dole-out more than one chunk at a time if needed.

# ifdef PARALLEL_CHUNKING
	mtx.lock();
# endif
	if( !pos )
	    pos = mapBuf->Text() + sizeof( version );

	const char* endOfChunks = mapBuf->Text() +
	                          mapBuf->Length() - sizeof( verify_hash );

	if( pos == endOfChunks )
	{
# ifdef PARALLEL_CHUNKING
	    mtx.unlock();
# endif
	    return false;
	}

	const char* const i = pos;
	pos += DiskChunkSize();

	chunkSize_t sTmp = 0;
	memcpy( (void*) &sTmp, i, sizeof( chunkSize_t ) );
	chunk.size = sTmp;
	chunk.offset = offset;
	offset += chunk.size;

	// The offset was the last thing that needed protection,
	// so we can do the rest unlocked.

# ifdef PARALLEL_CHUNKING
	mtx.unlock();
# endif
	if( !chunk.hash.Length() )
	    chunk.hash.BlockAlloc( BLAKE3_BUFFLEN );
	chunk.hash.Clear();

	if( ( i + sizeof( chunkSize_t ) ) >= ( mapBuf->Text() + mapBuf->Length() ) )
	    fprintf(stderr,"past buffer!\n");

	StrOps::OtoX( (const unsigned char*)i + sizeof( chunkSize_t ),
	              BLAKE3_BUFFLEN, chunk.hash );

	return true;
}

VVarTree *
ChunkMap::AsVTree( Error* e )
{
	Chunk c;
	ChunkVTree *tree = new ChunkVTree();

	ResetIterator();

	while( GetNextChunk( c ) )
	    tree->Put( &c, e );

	ResetIterator();

	return tree;
}

P4INT64
ChunkMap::GetFileSize()
{
	P4INT64 size = 0;
	Chunk c;
	ResetIterator();
	while( GetNextChunk( c ) )
	{
	    size += c.size;
	}
	ResetIterator();

	return size;
}

VVarArray *
ChunkMap::Diff( ChunkMap& other, Error* e )
{
	// Note that there's no check that the version of ChunkMaps being
	// compared are the same.  Chunk maps with differing versions may
	// be compatible.

	// Unpack the map buffers, populating the offsets, etc.

	VVarTree *b = other.AsVTree( e );
	VVarArray *d = new ChunkVArray();

	// The following accomplishes the same as this:
	//
	//	std::set_difference( a.begin(), a.end(),
	//	                     b.begin(), b.end(),
	//	                     std::inserter( d, d.begin() ) );

	Chunk c;
	ResetIterator();
	while( GetNextChunk( c ) )
	{
	    if( !b->Get( &c ) )
	    {
	        b->Put( &c, e );
	        d->Put( new Chunk( c ) );
	    }
	}
	ResetIterator();

	delete b;
	return d;
}

// Determine if the number of chunks to send will be below the threshold,
// where "below the threshold" is defined as 
//     ( chunksToSend / numChunks ) * 100 < threshold
int
ChunkMap::BelowThreshold( P4INT64 chunksToSend, StrBuf *msg, bool isCltSvr )
{
	const P4INT64 threshold = isCltSvr ?

	    // Delta Transfer is between client and server
	    p4tunable.Get( P4TUNE_NET_DELTA_TRANSFER_THRESHOLD ) :

	    // Delta Transfer is between upstream and downstream servers
	    p4tunable.Get( P4TUNE_NET_DELTA_RPL_THRESHOLD );

	if( !threshold )
	{
	    if( msg )
	        *msg = "net.delta.transfer.threshold=0";

	    return 0;
	}

	// Only perform delta transfer if the ratio of nChunksToSend
	// over nTotalChunks is under the threshold to avoid further
	// overhead when the saving on transfer is small. Set threshold
	// to 100 to always perform delta transfer and 0 to disable it.

	if( (chunksToSend * 100 ) > ( threshold * numChunks ) )
	{
	    if( msg )
	    {
	        P4INT64 pct = ( chunksToSend * 100 ) / numChunks ;
	        *msg << "net.delta.transfer.threshold set/actual " <<
	                threshold << "/" << pct;
	    }
	    return 0;
	}

	return 1;
}
# if defined( P4_FUZZ_CHUNKMAP )

extern "C" int LLVMFuzzerTestOneInput( const uint8_t* data, size_t size )
{
	Error e;
	StrRef sp( (char*) data, size );
	StrPtr* p = &sp;
	ChunkMap cm( p, &e );

	if( e.Test() )
	    return 0;

	size_t s = 0;
	ChunkMap::Chunk c;

	while( cm.GetNextChunk( c ) )
	{
	    // Try to keep this loop from being optimized out.
	    s = c.size;
	}

	if( false )
	    printf( "size: %zu\n", s );

	return 0;
}

# endif // libfuzzer

# endif // HAS_CPP11

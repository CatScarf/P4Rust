/*
 * Copyright 1995, 2024 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# ifdef HAS_CPP11

# ifdef PARALLEL_CHUNKING
# include <mutex>
# endif

# include <stdint.h>

# ifdef P4_BIG_ENDIAN
# error "ChunkMap needs porting to support big-endian platforms."
// All data is saved in the byte-order of the host machine, which
// is expected to be little-endian.  A big-endian box would need
// to convert the data before use. 
# endif

/*

A chunk map is a series of size/hash pairs, where the size is a region of
a file at a specific offset, and the hash is the digest of that regions
content.

Chunk map properties:

- Small storage space, to reduce network overhead.
- Should not change often because the format is interpreted by the client
- Maps become incompatible if the CDC chunking chunk size parameters change

Chunk map file format details:

# All data is little-endian.

# Field 0 is the version number of the map format

uint8_t : version

# Fields 1-n are size/hash pairs

# Note that this chunk size wastes a bit of space being a uint32_t
# because the maximum size of a chunk is only going to be about a
# uint24_t.  The size difference isn't a whole lot though.
4 byte unsigned integer (i.e. uint32_t) : chunk size
32 bytes : BLAKE3 hash value

# The final field is a trailing verification checksum
# for the prior content of the map file.

32 bytes : BLAKE3 hash value

Note that the originating file offset for each chunk is not stored,
and must be calculated on access.  Same for the total file size and
hash of the whole file.

*/

class VVarArray;
class VVarTree;

class ChunkMap
{
	public:

	    // ChunkMap::Chunk is an unpacked section of the larger chunk map.
	    //
	    // Note that the members of a Chunk are made for convenient
	    // export via RPC and do not reflect the actual structure of
	    // the stored chunkmap.

	    struct Chunk
	    {
	        using chunkSizeMem_t = uint32_t;

	        StrBuf hash;
	        chunkSizeMem_t size = 0;
	        // This isn't part of the on-disk or in-memory chunkmap data, but
	        // is calculated on-demand when a ChunkMap::Chunk is created.
	        P4INT64 offset = 0;

	        Chunk();
	        Chunk( const Chunk& other );
	        Chunk& operator =( Chunk&& rhs );

	        // These operators are necessary for ChunkMap::Diff().
	        bool operator < ( const Chunk& rhs ) const;
	        bool operator ==( const Chunk& rhs ) const;
	    };

	    ChunkMap();
	    // If 'map' is non-NULL, it is expected to point to a complete
	    // caller-owned ChunkMap buffer, e.g. as an RPC variable.
	    ChunkMap( StrPtr* map, Error* e );

	    ~ChunkMap();
	    ChunkMap( const ChunkMap& other );

	    ChunkMap& operator=( const ChunkMap& other ) = delete;
	    ChunkMap& operator=( ChunkMap&& other );

	    // Chunk the given file and populate the class's in-memory data.
	    // Also optionally calculate the MD5 of the file, if we're feeling
	    // like dragging our feet during this operation.
	    void Create( const StrPtr* fromFile, Error* e );
	    void Create( FileSys* fs, Error* e, MD5* md5 = nullptr );

	    // Export the chunk map to disk.
	    void Write( const StrPtr* toFile, Error* e ) const;

	    // Import an existing chunkmap file.
	    void Read( const StrPtr* fromFile, Error* e );

	    // Perform some extra checking on top of what Parse() does,
	    // at the time the client sends us this data.
	    bool Validate( const size_t expectedSize, Error* e );

	    void GetVerifyHash( StrBuf& digestBuf ) const;

	    P4INT64 GetFileSize();

	    // Return the unique Chunks with hash/size pairs that are not in 'other'.
	    // Can use ~3x the size of the 'other' map's memory.
	    // This does not return a ChunkMap, because the offset is
	    // implicit in the ChunkMap map buffer, and callers need the offset
	    // to be able to locate the chunk in the file.  Callers needing a
	    // diff don't need to serialize the data.
	    VVarArray* Diff( ChunkMap& other, Error* e );

	    // Number of chunks in the map.  If the map was created
	    // from a file as a source, it's the number of chunks in
	    // the file.  If the map was created via diffing two maps,
	    // it's the number of chunks in the diff.
	    size_t ChunkCount() const;

	    // This is a thread-safe iterator over the chunks in the map.
	    // It returns a Chunk rather than data, allowing for OS-specific
	    // optimizations such as zero-copy copies.  Returns false when
	    // there are no more chunks.  Chunks are not in any specific order.
	    // Note that there is no reset() function to do more than one loop.
	    bool GetNextChunk( Chunk& chunk );

	    // Expose the in-memory complete chunkmap so a caller can
	    // do things like send it as an RPC message.
	    const StrPtr* const GetBuf() const;

	    // Return the unique set of chunks across the whole chunk map.
	    VVarTree* AsVTree( Error *e );

	    // Run after GetNextChunk() to reset it.
	    void ResetIterator();

	    void DumpJSON( StrBuf& out );

	    // Do delta transfer only if nChunksToSend / nChunksTotl < threshold
	    int BelowThreshold( P4INT64 chunksToSend, StrBuf *msg=0,
	                        bool isCltSvr=true );

	    // CDC algorithm chunk size parameters.
	    static size_t GetCDCMinSize();
	    static size_t GetCDCAvgSize();
	    static size_t GetCDCMaxSize();

	private:

	    // Version of the chunk map file format.
	    using mapVersion_t = uint8_t;
	    // Maximum size of a chunk (see GetCDCMaxSize()).
	    using  chunkSize_t = uint32_t;

	    // Storage size of a single size/hash pair in the list of chunks.
	    constexpr static size_t DiskChunkSize()
	    {
	        return sizeof( chunkSize_t ) + BLAKE3_BUFFLEN;
	    }

	    // Size of a minimum viable map.  Used as a sanity check to
	    // prevent runtime errors due to file corruption.  This is the
	    // header+trailer and no chunks (i.e. an empty lbr file)

	    constexpr static size_t DiskMapMininumSize()
	    {
	        return sizeof( mapVersion_t ) + BLAKE3_BUFFLEN;
	    }

	    void SetBuf( StrPtr* map, Error* e );
	    void CopyBuf( StrPtr* map, Error* e );

	    // Read the serialized chunk map buffer, populating the class
	    // variables.  Does a variety of error checking.
	    bool Parse( const char* source, const char* name, Error* e );

	    // Return the version of the chunk map represented here.
	    int GetVersion() const;

	    // Get the hash of the whole chunk map file.
	    const uint8_t *GetVerifyHash() const;

	    // Version of the format of this chunk map data.
	    mapVersion_t version = 0;

	    // Number of chunks in the map.
	    size_t numChunks = 0;

	    // The whole chunk map - version,chunks,verification_hash
	    StrPtr* mapBuf = new StrBuf;

	    // Whether the chunk map came from a caller, or us.
	    bool ownBuf = true;

	    // Hash of the entire chunk map file.
	    uint8_t verify_hash[ BLAKE3_BUFFLEN ] = {};

	    // Iterator - offset into the mapBuf.
	    char* pos = nullptr;
	    size_t offset = 0;
# ifdef PARALLEL_CHUNKING
	    std::mutex mtx;
# endif
	    // These CDC algorithm constants control the size of chunks that
	    // are created.  Changes in chunk size change the 'identity' of content
	    // since the hashes of chunks are the basis for the chunk map diff
	    // operation, and if a change in chunk size changes a lot of the hashes,
	    // diffing is less effective.  Chunk identity is a cross-version
	    // compatibility concern.  Note also that the max size is also used
	    // in some verification routines, so it if is increased, compatibility
	    // between product releases may be affected.
# if 0
	    static const size_t cdc_min_size =  8000u,
	                        cdc_avg_size = 16000u,
	                        cdc_max_size = 24000u;
# else
	    static const size_t cdc_min_size = 128000u,
	                        cdc_avg_size = 256000u,
	                        cdc_max_size = 1024000u;
# endif
} ;

class P4INT64Array;

class ChunkOffsetTree : public VVarTree
{
    public:
	class ChunkOffsets
	{
	    public:
	        ChunkOffsets() : count( 0 ) {}
	        ChunkOffsets( const char *h ) : count( 0 ), hash( h ) {}
	        ~ChunkOffsets() {}

	        int count;
	        P4INT64Array offsets;
	        StrBuf hash;

	        void
	        Put( P4INT64 offset )
	        {
	            offsets[ count++ ] = offset;
	        }
	} ;

	ChunkOffsetTree() {}
	virtual ~ChunkOffsetTree()
	{
	    Clear();
	}

	virtual int Compare( const void *a, const void *b ) const
	{
	    const ChunkOffsets *ca = (const ChunkOffsets *)a;
	    const ChunkOffsets *cb = (const ChunkOffsets *)b;
	    return ca->hash.XCompare( cb->hash );
	    
	}
	
	virtual void *Copy( const void *src ) const
	{
	    ChunkOffsets* c = new ChunkOffsets;
	    c->hash = ( ( ChunkOffsets* )src )->hash;
	    return c;
	}
	
	virtual void Delete( void *a ) const
	{
	    delete ( ChunkOffsets* )a;
	}

	virtual void Dump( void *a, StrBuf &buf ) const
	{
	}

	ChunkOffsets *Get( const char* hash )
	{
	    ChunkOffsets o( hash );
	    return ( ChunkOffsets* )VVarTree::Get( &o );
	}

	ChunkOffsets *Put( const char* hash, Error *e )
	{
	    ChunkOffsets o( hash );
	    return ( ChunkOffsets* )VVarTree::Put( &o, e );
	}
} ;

# endif // HAS_CPP11

/*
 * Copyright 1995, 2023 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * PipeIo -- Connect to a NamedPipe and perform 2-way IO
 *
 * Methods:
 *
 *	PipeIo() - specify the NamedPipe (no validation on the name is done)
 *	Open() - connect to the named pipe
 *	Write() - send the specified number of bytes over the pipe
 *	Read() - read any pending bytes from the pipe (BLOCKING)
 *	Peek() - non-blocking check of pending bytes to read from the pipe
 *	Close() - close the connection to the pipe
 *
 *	ReadLine() - helper method for reading up to a newline from the pipe
 *
 */
 
class FileSys;

class PipeIo
{
    public:
			PipeIo( StrPtr path );
			~PipeIo();

	void		Open( Error *e);
	void		Write( const StrPtr &in, Error *e );
	p4size_t	Read( StrBuf &out, Error *e );
	p4size_t	Peek( Error *e );
	void		Close( Error *e );

	p4size_t	ReadLine( StrBuf &out, StrBuf &buf, Error *e );

    private:
	FileSys *	pipe;
	bool		open;
} ;

/*
 * Copyright 1995, 2021 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * serverid.cc - Read/Write the serverid to the server.id file.
 *
 */

# include <stdhdrs.h>
# include <strbuf.h>
# include <strdict.h>
# include <vararray.h>
# include <error.h>
# include <debug.h>
# include <filesys.h>
# include <pathsys.h>
# include <strops.h>
# include <msgdm.h>
# include <tunable.h>
# include <charman.h>
# include "serverid.h"

int
ServerIDAccess::HasServerID()
{
	FileSys *f = FileSys::Create( FST_TEXT );
	f->Set( StrRef( serverIDFile ) );
	int result = ( f->Stat() & FSF_EXISTS ) != 0;
	delete f;
	return result;
}

void
ServerIDAccess::ReadServerID( Error *e )
{
	FileSys *f = FileSys::Create( FST_TEXT );

	f->Set( StrRef( serverIDFile ) );
	if( f->Stat() & FSF_EXISTS )
	{
	    f->Open( FOM_READ, e );
	    if( !e->Test() )
	    {
	        f->ReadLine( &serverID, e );
	        if( !e->Test() )
	            f->ReadLine( &sIDMessage, e );
	        f->Close( e );
	    }
	    StrOps::StripNewline( serverID );
	    StrOps::StripNewline( sIDMessage );
	}
	delete f;
}

void
ServerIDAccess::WriteServerID( const StrPtr &id, Error *e )
{
	if( e->Test() )
	    return;

	FileSys *f = FileSys::Create( FST_TEXT );
	f->Set( StrRef( serverIDFile ) );
	FileSys *t = FileSys::Create( FST_TEXT );
	t->MakeLocalTemp( f->Name() );
	t->SetDeleteOnClose();
	t->Open( FOM_WRITE, e );

	if( !e->Test() )
	{
	    t->Write( id.Text(), id.Length(), e );
	    if( sIDMessage.Length() ) 
	    {
	        StrBuf buffer;
	        buffer.Set( "\n" );
	        buffer.Append( &sIDMessage );
	        t->Write( buffer.Text(), buffer.Length(), e );
	    }
	    t->Perms( FPM_RW );
	    t->Close( e );
	    if( !e->Test() )
	    {
	        t->Rename( f, e );
	        f->Chmod( FPM_RO, e );
	    }
	}
	delete t;
	delete f;
}

void
ServerIDAccess::SetSIDMessage( StrPtr *msg )
{
	if( msg && msg->Length() )
	    sIDMessage.Set( msg );
	else
	    sIDMessage.Clear();
}

/* Currently this validation module is used by the Broker if it does not have
 * access to the Perforce Database. Later we can extend the appropriate usage
 * of this even at server end */
void
ValidateSvrID( const StrPtr &id, Error *e )
{
	// check for length
	if( id.Length() > p4tunable.Get( P4TUNE_DM_MAXKEY ) )
	{
	    e->Set( MsgDm::IdTooLong );
	    return;
	}

	// Check for illegals
	char *s = id.Text();
	char *d = s;
	char allnumeric = 1;

	for( ; *d; ++d )
	{
	    if( allnumeric && !isAdigit( d ) ) 
	    {
		allnumeric = 0;
	    }

	    if( !isAprint( d ) )
	    {
		e->Set( MsgDm::IdNonPrint ) << s;
		return;
	    }
	    else if( isAspace( d ) )
	    {
	        *d = '_';
	    }
	    else if( *d == '@' || *d == '#' )
	    {
	        e->Set( MsgDm::IdHasRev ) << s;
	        return;
	    }
	    else if( *d == '/' )
	    {
	        continue;
	    }
	    else if( d[0] == '%' && d[1] == '%' )
	    {
	    	e->Set( MsgDm::IdWild ) << s;
	    	return;
	    }
	    else if( *d == ',' )
	    {
		e->Set( MsgDm::IdHasComma ) << s;
		return;
	    }
	}
}

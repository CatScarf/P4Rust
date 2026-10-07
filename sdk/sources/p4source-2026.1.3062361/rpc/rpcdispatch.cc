/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>

# include <debug.h>
# include <trienode.h>
# include <strbuf.h>
# include <strdict.h>
# include <error.h>
# include <vararray.h>

# include "rpc.h"
# include "rpcdispatch.h"
# include "rpcdebug.h"

RpcDispatcher::RpcDispatcher( void )
{
	dispatches = new VarArray;
	altDispatcher = 0;
	altindex = -1;
}

RpcDispatcher::~RpcDispatcher( void )
{
	delete altDispatcher;
	delete dispatches;
}

void
RpcDispatcher::Add( const RpcDispatch *dispatch )
{
	dispatches->Put( (void *)dispatch );
}

const RpcDispatch *
RpcDispatcher::Find( const char *func ) const
{
	for( int i = dispatches->Count(); i--; )
	{
	    // If we have dispatch items pushed on
	    // later (via an Add), then search those first.
	    // If we reach where the altdispatcher
	    // was built from, pass this request on to it.
	    if( altDispatcher && ( i == altindex ) )
	        return altDispatcher->Find( func );

	    const RpcDispatch *disp = (RpcDispatch *)(dispatches->Get(i));

	    // Look up function name in dispatch table.
	    while( disp->opName && strcmp( func, disp->opName ) )
	        disp++;

	    if( disp->opName )
	        return disp;
	}
	return 0;
}

void
RpcDispatcher::AddAltDispatcher()
{
	delete altDispatcher;
	AltDispatcher *alt = new AltDispatcher;
	for( int i = 0; i < dispatches->Count(); i++ )
	    alt->Add( (RpcDispatch *) dispatches->Get(i) );
	alt->Final();
	altindex = dispatches->Count() - 1;
	altDispatcher = alt;
}

void
RpcDispatcher::ClearAltDispatcher()
{
	delete altDispatcher;
	altDispatcher = 0;
	altindex = -1;
}

AltDispatcher::AltDispatcher()
{
	final = 0;
	trieNode = new Tnode();
}

AltDispatcher::~AltDispatcher()
{
	delete trieNode;
}

void
AltDispatcher::Add( const RpcDispatch *disp )
{

	while( disp->opName )
	{
	    trieNode->insert( disp->opName, disp );
	    disp++;
	}
}


const RpcDispatch *
AltDispatcher::Find( const char *func ) const
{
	return (RpcDispatch *) trieNode->trimsearch( func );
}

void
AltDispatcher::Final()
{
	trieNode->Trim();
	final = 1;
}

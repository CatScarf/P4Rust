/*
 * Copyright 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

//
//
// C++ implementation of search and insert
// operations on Trie

# include <stdhdrs.h>
# include <strbuf.h>
# include <strdict.h>
# include <vararray.h>
# include <error.h>
# include <debug.h>
# include "trienode.h"

int Tnode::memuse = 0;
int Tnode::nodes = 0;

static const int MAX_CHARS = 256;

Tnode::Tnode() : EndOfCmdInvokes(0)
{
	children = new Tnode*[MAX_CHARS];
	memuse += sizeof( Tnode );
	memuse += MAX_CHARS * sizeof( Tnode * );
	for(int i =0; i<MAX_CHARS; ++i)
	    children[i] = 0;
	offset = 0;
	nofkids = MAX_CHARS;
	nodes++;
}

Tnode::~Tnode()
{
	for( int i = 0; i < nofkids; i++ )
	    delete children[i];
	delete []children;

	memuse -= nofkids * sizeof( Tnode *);
	memuse -= sizeof( Tnode );
	nodes--;
}

// If not present, inserts key into trie
// If the key is prefix of trie node, just
// marks leaf node

void
Tnode::insert( const char *key, const void *item )
{
	Tnode *currentNode = this;

	int len = strlen( key );
	for( int i = 0 ; i < len ; i++ )
	{
	    int index = key[i];
	    //int index = key[i] - 'a';
	    if ( ! currentNode->children[ index ] )
	        currentNode->children[ index ] = new Tnode;

	    currentNode = currentNode->children[ index ];
	}

	// mark last node as leaf

	// We have 'protocol' specified twice for the server.
	// and by default we overwrite a previous entry.
	// This works correctly for us as in the original
	// dispatcher code the 'dispatch' arrays are searched
	// in last pushed order, i.e. backwards.
	// As we are called in 'forward' order we
	// must overwrite dups to mantain the same semantics.
	currentNode->EndOfCmdInvokes = item;
}
// Returns true if key presents in trie, else
// false
const void *
Tnode::search( const char *key ) const
{
	const Tnode *currentNode = this;
	int len = strlen( key );

	for (int i = 0; i < len; i++)
	{
	    int index = key[ i ];
	    if ( ! currentNode || ! currentNode->children ||
	         ! currentNode->children[ index ] )
	        return 0;

	    currentNode = currentNode->children[ index ];
	}
	return currentNode->EndOfCmdInvokes;
}


const void *
Tnode::trimsearch( const char *key ) const
{
	const Tnode *currentNode = this;
	int len = strlen( key );
	for( int i = 0 ; i < len ; i++ )
	{
	    int index = key[ i ];
	    if( index < currentNode->offset ||
	        index >= currentNode->offset + currentNode->nofkids )
	        return 0;
	    index -= currentNode->offset;
	    if ( ! currentNode->children[ index ] )
	        return 0;

	    currentNode = currentNode->children[ index ];
	}
	return currentNode->EndOfCmdInvokes;
}

// Walk every node, resizing the children array.
void
Tnode::Trim()
{
	VarArray worklist;
	worklist.Put( this );

	while( worklist.Count() > 0 )
	{
	    Tnode *wnode = (Tnode *)worklist.Get( 0 );
	    worklist.Remove( 0 );
	    for( int i = 0; i < wnode->nofkids; i++ )
	    {
	        if( wnode->children[ i ] )
	        {
	            worklist.Put( wnode->children[ i ] );
	        }
	    }
	    TrimNode( wnode );
	}
}

void
Tnode::TrimNode( Tnode *wnode )
{
	int bot, top;

	for( bot = 0; bot < wnode->nofkids; bot++ )
	{
	    if( wnode->children[ bot ] )
	        break;
	}

	for( top = wnode->offset + wnode->nofkids - 1; top >=0 ; top-- )
	{
	    if( wnode->children[ top ] )
	        break;
	}
	int nofnewkids;
	Tnode **newkids = 0;

	nofnewkids = top - bot + 1;
	if( nofnewkids > 0 )
	{
	    newkids = new Tnode *[ nofnewkids ];
	    memuse += nofnewkids * sizeof( Tnode *);

	    int ind = 0;
	    for( int i = bot; i <= top; i++ )
	    {
	        newkids[ ind ] = wnode->children[ i ];
	        ind++;
	    }
	}
	delete []wnode->children;
	memuse -= wnode->nofkids * sizeof( Tnode *);

	if( nofnewkids > 0 )
	{
	    wnode->children = newkids;
	    wnode->offset = bot;
	    wnode->nofkids = nofnewkids;
	}
	else
	{
	    wnode->children = 0;
	    wnode->offset = 0;
	    wnode->nofkids = 0;
	}
}

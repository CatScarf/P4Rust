/*
 * Copyright 1995, 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <new>

# include <stdhdrs.h>

# include <debug.h>
# include <error.h>

# include "strbuf.h"
# include "vartree.h"

# include <msgdb.h>


VVarTree::VVarTree()
{
	root = iterNode = 0;
	count = 0;
}

/*
 *This MUST be implemented in all derived classes
 *
VVarTree::~VVarTree()
{
	Clear();
}
*/

void
VVarTree::Clear()
{
	delete root;
	root = iterNode = 0;
}

void * 
VVarTree::Get( const void* key ) const
{
	VarTreeNode* get = GetNode( key );

	return get ? get->k : 0;
}

void
VVarTree::Position( const void *key )
{
	iterNode = key ? Find( key ) : FirstNode();
	if( !iterNode || !key )
	    return;
	// Position ourselves at first node >= key.
	if( Compare( key, iterNode->k ) > 0 )
	    iterNode = iterNode->Next();
}

void *
VVarTree::Fetch()
{
	void *rec = iterNode ? iterNode->Value() : 0;
	iterNode = iterNode ? iterNode->Next() : 0;
	return rec;
}

void *
VVarTree::Shift()
{
	VarTreeNode* n = FirstNode();
	if( !n )
	    return 0;

	while ( n->l )
	    n = n->l;

	void *record = Copy( n->Value() );
	RemoveNode( n );

	return record;
}

int
VVarTree::Remove( const void* key )
{
	VarTreeNode* node = GetNode( key );
	return RemoveNode( node );
}

int
VVarTree::RemoveNode( VarTreeNode* node )
{
	// See: http://www.mathcs.emory.edu/~cheung/Courses/323/Syllabus/Trees/AVL-delete.html

	if( !node )
	    return 0;

	VarTreeNode* p = node->p;
	VarTreeNode* r = node->r;
	VarTreeNode* l = node->l;
	
	VarTreeNode *t = r;
	VarTreeNode *d = p;

	int isRoot = !p;
	int left = p && p->l == node;

	if( r && l )
	{
	    // One step right, then follow left
	    while( t->l )
	        t = t->l;

	    // Remove t from tree
	    VarTreeNode* tp = t->p;
	    VarTreeNode* tr = t->r;
	    if( tp->l == t )
	        tp->l = tr;
	    else
	        tp->r = tr;
	    if( tr )
	        tr->p = tp;

	    if( tp == node ) // then r = t
	        r = tr;

	    // replace node with t
	    if( isRoot )
	        root = t;
	    else if( left )
	        p->l = t;
	    else
	        p->r = t;
	    t->p = p;

	    t->l = l;
	    if( l )
	        l->p = t;
	    t->r = r;
	    if( r )
	        r->p = t;

	    d = tp;
	    if( tp == node )
	        d = t;
	}
	else if( r || l )
	{
	    if( l )
	        r = l;

	    //Easy
	    if( isRoot )
	    {
	        root = r;
	        r->p = 0;
	    }
	    else if( left )
	    {
	        p->l = r;
	        r->p = p;
	    }
	    else
	    {
	        p->r = r;
	        r->p = p;
	    }
	}
	else
	{
	    if( isRoot )
	        root = 0;
	    else if( left )
	        p->l = 0;
	    else
	        p->r = 0;
	}

	node->p = 0;
	node->r = 0;
	node->l = 0;
	delete node;

	if( !d )
	    return 1;

	if( ( d = CheckBalance( d ) ) )
	    Balance( d );

	return 1;
}

VarTreeNode *
VVarTree::GetNode( const void* key ) const
{
	VarTreeNode* find = Find( key );
	if ( find && !Compare( find->k, key ) )
	    return find;
	
	return 0;
}

void *
VVarTree::Put( void* key, Error* e, PutMode mode )
{
	if( !CanPut( e ) )
	    return 0;

	VarTreeNode* p = Find( key );
	if( p && !Compare( p->k, key ) )
	{
	    // Existing record
	    if( mode == Insert )
	        return 0;

	    // Do update
	    if( p->k != key)
	        p->k = CopyOver( p->k, key );
	    return p->k;
	}

	if( mode == Update )
	    return 0;

	// Do insert
	VarTreeNode* n = NULL;

	try
	{
	    n = new VarTreeNode( key, p, this );
	} catch( const std::bad_alloc& err )
	{
	    e->Set( MsgDb::MaxMemory );
	    return 0;
	}

	if( !p )
	{
	    root = n;
	    return n->k;
	}

	if( Compare( key, p->k ) < 0 )
	    p->l = n;
	else
	    p->r = n;

	if( ( p = CheckBalance( p ) ) )
	    Balance( p );

	return n->k;
}

VarTreeNode *
VVarTree::FirstNode() const
{
	if( !root )
	    return 0;

	VarTreeNode* n = root;

	while( n->l )
	    n = n->l;

	return n;
}

VarTreeNode *
VVarTree::LastNode() const
{
	if( !root )
	    return 0;

	VarTreeNode* n = root;

	while( n->r )
	    n = n->r;

	return n;
}

void *
VVarTree::CopyOver( void *tgt, const void *src ) const
{
	// Subclasses dealing with fixed-size objects may be able
	// to implement this more efficiently by reusing tgt's memory.
	Delete( tgt );
	return Copy( src );
}

void
VVarTree::Dump( void *val )
{
	StrBuf buf;
	Dump( val, buf );
	p4debug.printf( "%s\n", buf.Text() );
}

// AVL trees stay height-balanced by tracking the height
// of each node's subtree.  After each insert, we
// recalculate the heights and balances.  If any node is
// "unbalanced" by having a height difference of more than
// 1 between its two child trees, we need to balance it.

VarTreeNode *
VVarTree::CheckBalance( VarTreeNode* n )
{
	if( !n )
	    return 0;

	n->h = 1;
	if( n->l && n->l->h > n->h )
	    n->h = n->l->h;
	if( n->r && n->r->h > n->h )
	    n->h = n->r->h;
	if( n->l || n->r )
	    n->h++;

	int b = ( n->l ? n->l->h : 0 ) - ( n->r ? n->r->h : 0 );
	
	if( b == n->b ) 
	    return 0;
	else
	    n->b = b;

	if( n->b < -1 || n->b > 1 )
	    return n;

	return CheckBalance( n->p );
}

void
VVarTree::Balance( VarTreeNode* p )
{
	VarTreeNode* t;

	if( p->b < -1 ) // heavy on the right
	{
	    if( p->r->b > 0 ) // right child heavy on the left?
	    {
	        // if so, do a right-rotate first

	        t = p->r; 
	        p->r = t->l;
	        p->r->p = p;

	        t->l = p->r->r;
	        if( t->l )
	            t->l->p = t;

	        p->r->r = t;
	        t->p = p->r;

	        CheckBalance( t );
	    }

	    // now left-rotate

	    t = p->r;
	    p->r = t->l;
	    if( p->r )
	        p->r->p = p;

	    t->l = p;
	    t->p = p->p;
	    p->p = t;

	    if( !t->p ) 
	        root = t;
	    else if( t->p->r == p )
	        t->p->r = t;
	    else if( t->p->l == p )
	        t->p->l = t;
	}
	else if( p->b > 1 ) // heavy on the left
	{
	    if( p->l->b < 0 ) // left child heavy on the right?
	    {
	        // if so, do a left-rotate first

	        t = p->l;
	        p->l = t->r;
	        p->l->p = p;

	        t->r = p->l->l;
	        if( t->r )
	            t->r->p = t;

	        p->l->l = t;
	        t->p = p->l;

	        CheckBalance( t );
	    }

	    // now right-rotate

	    t = p->l;
	    p->l = t->r;
	    if( p->l )
	        p->l->p = p;

	    t->r = p;
	    t->p = p->p;
	    p->p = t;

	    if( !t->p )
	        root = t;
	    else if( t->p->r == p )
	        t->p->r = t;
	    else if( t->p->l == p )
	        t->p->l = t;
	}

	CheckBalance( p );
}

VarTreeNode *
VVarTree::Find( const void* key ) const
{
	// Find the node with a matching key if it exists.
	// If not, return the closest match we found while traversing
	// the tree, which will be either immediately before OR after
	// the spot where the key would go.
	VarTreeNode* look = root;
	VarTreeNode* last = root;
	while( look )
	{
	    last = look;

	    int c = Compare( key, look->k );
	    if( !c )
	        return look;
	    if( c < 0 ) 
	        look = look->l;
	    else 
	        look = look->r;
	}

	return last;
}


VarTreeNode::VarTreeNode( void *key, VarTreeNode *parent, VVarTree *tree )
{
	t = tree;
	k = t->Copy( key );
	p = parent;
	l = r = 0;
	b = 0;
	h = 1;

	t->count++;
}

VarTreeNode::~VarTreeNode()
{
	t->Delete( k );
	delete l;
	delete r;

	t->count--;
}

VarTreeNode *
VarTreeNode::Next()
{
	VarTreeNode *n;

	// Leftmost child of right subtree?

	for( n = r; n; n = n->l )
	    if( !n->l ) 
	        return n;

	// Nearest ancestor we're in a left subtree of?

	for( n = this; n->p; n = n->p )
	    if( n == n->p->l )
	        return n->p;

	// Nothing!

	return 0;
}

VarTreeNode *
VarTreeNode::Prev()
{
	// Same as Next() but backwards.

	VarTreeNode *n;

	for( n = l; n; n = n->r )
	    if( !n->r )
	        return n;

	for( n = this; n->p; n = n->p )
	    if( n == n->p->r )
	        return n->p;

	return 0;
}

void
VarTreeNode::Dump( int level ) const
{
	if( r )
	    r->Dump( level + 1 );

	if( r && r->p != this )
	    p4debug.printf( "ORPHANED NODE!\n" );

	for( int i = 0; i < level; i++ )
	    p4debug.printf( "  " );

	t->Dump( k );

	if( l && l->p != this )
	    p4debug.printf( "ORPHANED NODE!\n" );

	if( l )
	    l->Dump( level + 1 );	
}

void
VVarTree::DumpTree()
{
	if( root )
	    root->Dump( 0 );
}

int
VVarTree::VerifyTree()
{
	int c = 0;
	int errCount = 0;
	VarTreeNode* a = FirstNode();
	VarTreeNode* b = 0;
	while( a )
	{
	    c++;
	    b = a ? a->Next() : 0;
	    if( b && Compare( a->Value(), b->Value() ) >= 0 )
	    {
	        p4debug.printf( "Nodes are out of order!\n" );
	        Dump( a );
	        Dump( b );
	        errCount++;
	    }
	    a = b;
	}

	if( c != count )
	{
	    p4debug.printf( "Expected %d nodes, but only saw %d!\n",
	                    count, c );
	    DumpTree();
	    errCount++;
	}
	return errCount;
}

/*
 * Copyright 1995, 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <debug.h>
# include <errorlog.h>
# include <msgclient.h>

# include "error.h"
# include "vartree.h"

# include "strbuf.h"
# include "strdict.h"
# include "strtree.h"
# include "vararray.h"


struct VarVal {
	StrPtr *varp;
	StrPtr *valp;
} ;

struct StrArrTreeItem {
	StrBuf var;
	VarArray vals;  // array of StrPtr*
} ;

// StrArrVTree allows multiple values for each item in the tree
class StrArrVTree : public VVarTree {

    public:
			StrArrVTree() : VVarTree() {}
			~StrArrVTree() { Clear(); }

	int		Compare( const void *a, const void *b ) const;
	void		*Copy( const void *src ) const;
	void		Delete( void *a ) const;
	void		Dump( void *a, StrBuf &buf ) const;

	StrArrTreeItem	*GetItem( const StrRef &var );
	StrArrTreeItem	*PutItem( const StrRef &var, Error *e );
	void		RemoveItem( const StrRef &var );
} ;


int
StrArrVTree::Compare( const void *a, const void *b ) const
{
	return ( (StrArrTreeItem *)a )->var.XCompare(
	    ( (StrArrTreeItem *)b )->var);
}

void
StrArrVTree::Delete( void *a ) const
{
	for( int i = 0; i < ( (StrArrTreeItem *)a )->vals.Count(); i++ )
	{
	    StrBuf *p = (StrBuf *)( (StrArrTreeItem *)a )->vals.Get( i );
	    delete p;
	}

	( (StrArrTreeItem *)a )->vals.Clear();

	delete (StrArrTreeItem *)a;
}

void
StrArrVTree::Dump( void *a, StrBuf &buf ) const
{
	buf = ( (StrArrTreeItem *)a )->var;
}

void *
StrArrVTree::Copy( const void *src ) const
{
	StrArrTreeItem *ret = new StrArrTreeItem();
	ret->var = ((StrArrTreeItem *)src)->var;

	for( int i = 0; i < ( (StrArrTreeItem *)src )->vals.Count(); i++ )
	    ret->vals.Put( ( (StrArrTreeItem *)src )->vals.Get( i ) );

	return ret;
}

StrArrTreeItem *
StrArrVTree::GetItem( const StrRef &var )
{
	StrArrTreeItem key;
	key.var = var;

	return (StrArrTreeItem *)Get( &key );
}

StrArrTreeItem *
StrArrVTree::PutItem( const StrRef &var, Error *e )
{
	StrArrTreeItem *a = GetItem( var );

	if( !a )
	{
	    StrArrTreeItem key;
	    key.var = var;
	    a = (StrArrTreeItem *)Put( &key, e );
	}

	return a;
}

void
StrArrVTree::RemoveItem( const StrRef &var )
{
	StrArrTreeItem key;
	key.var = var;

	Remove( &key );
}

struct StrBufTreeItem {
	StrBuf	var;
	StrBuf	val;
} ;

StrBufTree::StrBufTree()
{
	tree = new StrArrVTree();
}

StrBufTree::StrBufTree( StrDict &dict )
{
	tree = new StrArrVTree();
	CopyVars( dict );
}

StrBufTree &
StrBufTree::operator =( StrDict & dict )
{
	CopyVars( dict );
	return *this;
}

StrBufTree::~StrBufTree()
{
	delete tree;
}

StrPtr *
StrBufTree::VGetVar( const StrPtr &var )
{
	StrArrTreeItem *item = tree->GetItem( var );
	return item ? (StrBuf *)((item->vals).Get( 0 )) : 0;
}


int
StrBufTree::VGetVarX( int x, StrRef &var, StrRef &val )
{
	if( x >= tree->Count() )
	    return 0;

	VarTreeNode *node = tree->FirstNode();
	while( x-- && node )
	    node = node->Next();

	if( !node )
	    return 0;

	StrArrTreeItem *item = (StrArrTreeItem *)node->Value();
	var = item->var;
	val = *(StrBuf *)((item->vals).Get( 0 )); // get the first value

	return 1;
}

void
StrBufTree::VSetVar( const StrPtr &var, const StrPtr &val )
{
	Error e;
	StrArrTreeItem *item = tree->PutItem( var, &e );

	if( item )
	{
	    StrBuf *valPtr = new StrBuf( val );
	    ( item->vals ).Put( valPtr );
	}
}

void
StrBufTree::VRemoveVar( const StrPtr &var )
{
	tree->RemoveItem( var );
}

void
StrBufTree::VClear()
{
	tree->Clear();
}

int
StrBufTree::VGetCount()
{
	return tree->Count();
}


class StrTreeIterator : public StrDictIterator {
    public:
	StrTreeIterator( StrBufTree *tree )
	{
	    this->tree = tree;
	    node = tree->tree->FirstNode();
	    i = 0;
	}

	~StrTreeIterator()
	{
	}

	virtual int Get(StrRef &var, StrRef &val)
	{
	    if( !node )
	        return 0;
	    
	    var = ( (StrArrTreeItem *)node->Value() )->var;
	    val = *(StrPtr *)( (StrArrTreeItem *)node->Value() )->vals.Get( i );
	    return 1;
	}

	virtual void Next()
	{
	    if( i + 1 < ( (StrArrTreeItem *)node->Value() )->vals.Count() )
	        ++i;
	    else
	    {
	        node = node ? node->Next() : 0;
	        i = 0;
	    }
	}

	virtual void Reset()
	{
	    node = tree->tree->FirstNode();
	    i = 0;
	}

    private:
	VarTreeNode	*node;
	StrBufTree	*tree;
	int		i;
} ;

StrDictIterator *
StrBufTree::GetIterator()
{
	if( !iterator )
	    iterator = new StrTreeIterator( this );
	else
	    iterator->Reset();
	return iterator;
}


StrBufTreeIdx::StrBufTreeIdx()
{
	keyArray = new VarArray();
}


StrBufTreeIdx::~StrBufTreeIdx()
{
	for( int i = 0; i < keyArray->Count(); i++ )
	{
	    VarVal* varValp = ( VarVal* )keyArray->Get( i );
	    ((StrBuf*)(varValp->valp))->Reset();
	    delete varValp;
	}

	delete keyArray;
}

int
StrBufTreeIdx::VGetVarX( int x, StrRef &var, StrRef &val )
{
	if( x >= keyArray->Count() )
	    return 0;

	VarVal* varValp = ( VarVal* )keyArray->Get( x );

	if( !varValp || !varValp->varp  || !varValp->valp )
	    return 0;

	var = *varValp->varp;
	val = *varValp->valp;
	return 1;
}

void
StrBufTreeIdx::VSetVar( const StrPtr &var, const StrPtr &val )
{
	StrBufTree::VSetVar( var, val );
	StrArrTreeItem *item = tree->GetItem( var );

	if( item )
	{
	    VarVal *kvp = new VarVal;
	    kvp->varp = &(item->var);

	    // look-up the just created last value in item->vals:
	    kvp->valp = (StrPtr*)(item->vals).Get( (item->vals).Count() - 1 );
	    keyArray->Put( kvp );
	}
}

void
StrBufTreeIdx::VRemoveVar( const StrPtr &var )
{
	StrPtr * a = VGetVar( var );

	if( a )
	{
	    for( int i = 0; i < keyArray->Count(); i++ )
	    {
	        VarVal *varVal = (VarVal *)keyArray->Get( i );

	        // remove all keyArray entries that match:
	        if( !varVal->varp->Compare( var ) )
	        {
	            delete varVal;
	            keyArray->Remove( i );
	            i--; // stay at the index that just got removed
	        }
	    }

	    StrBufTree::VRemoveVar( var );
	}
}

void
StrBufTreeIdx::VClear()
{
	for( int i = 0; i < keyArray->Count(); i++ )
	    delete (VarVal *)keyArray->Get( i );
	StrBufTree::VClear();
	keyArray->Clear();
}

StrDictIterator *
StrBufTreeIdx::GetIterator()
{
	if( !iterator )
	    iterator = new StrDictBasicIterator( this );
	else
	    iterator->Reset();
	return iterator;
}

int
StrBufTreeIdx::VGetCount()
{
	return keyArray ? keyArray->Count() : 0;
}

/*
 * Copyright 1995, 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */
 # ifndef VVARTREE
 # define VVARTREE

class KeepAlive;
class VarTreeNode;

class VVarTree {

    public:
	enum PutMode {
			Insert,  // no-op if key exists
			Update,  // no-op if key doesn't exist
			Upsert	 // update if key exists, insert if not
	};

			VVarTree();
	
	// Derived classes MUST call Clear() in their destructors
	// It cannot be called in the base class, because it calls the pure
	// virtual Delete() method.
	//		~VVarTree() { Clear(); }
	virtual		~VVarTree() {}

	virtual int	Compare( const void *a, const void *b ) const = 0;
	virtual void *	Copy( const void *src ) const = 0;
	virtual void	Delete( void *a ) const = 0;
	virtual void	Dump( void *a, StrBuf &buf ) const = 0;

	void		Clear();

	int		Count() const { return count; }
	void *		Get( const void *keyRecord ) const;
	void		Position( const void *key = 0 );
	void*		Fetch();

	void *		Put( void *record, Error *e, PutMode mode = Upsert );
	VarTreeNode *	Find( const void *key ) const;

	// n.b. Replace and Add will return 0 if they result in a no-op.
	void*		Replace( void *record, Error *e )
				{ return Put( record, e, Update ); }
	void*		Add( void *record, Error *e )
				{ return Put( record, e, Insert ); }

	void *		Shift();
	int		Remove( const void* key );

	VarTreeNode *	FirstNode() const;
	VarTreeNode *	LastNode() const;
	VarTreeNode *	GetNode( const void *keyRecord ) const;
	int		RemoveNode( VarTreeNode* node );

	void		DumpTree();
	int		VerifyTree();

    protected:
	// Derived classes: if returning 0, set an Error to say why.
	virtual int	CanPut( Error *e ) { return 1; }
	virtual void *	CopyOver( void *tgt, const void *src ) const;
	virtual void	Dump( void *val );  // to log

    private:

	friend class VarTreeNode;

	VarTreeNode *	CheckBalance( VarTreeNode* n );
	void		Balance( VarTreeNode* n );

	VarTreeNode*	root;
	VarTreeNode*	iterNode;
	int		count;
} ;

class VarTreeNode
{
public:

	void *		Value() const { return k; }

	VarTreeNode *	Next();
	VarTreeNode *	Prev();

private:

	friend class VVarTree;

			VarTreeNode( void *key, VarTreeNode *parent,
			             VVarTree *tree );
			~VarTreeNode();

	void		Dump( int level ) const;

	void *		k;	// key/record

	VarTreeNode *	p;	// parent
	VarTreeNode *	l;	// left child
	VarTreeNode *	r;	// right child

	int		b;	// balance
	int		h;	// height

	VVarTree *	t;
};

# endif

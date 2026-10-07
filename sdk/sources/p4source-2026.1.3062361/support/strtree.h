/*
 * Copyright 1995, 2022 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */


/*
 * Strtree.h - a set/get dictionary interface based on a tree structure
 *
 * Classes:
 *
 *	StrBufTree    - a StrDict for maintaining key/value pairs that provides
 *                      efficient inserts, deletes and access by key. If only
 *                      access by key is required, it is a better choice.
 *                      The underlying tree allows multiple values per key.
 *
 *	StrBufTreeIdx - a StrBufTree that also has a VarArray of pointer to
 *                      keys, providing efficient access by index. Use it if
 *                      both access by index and access by key are needed.
 *
 */

class VarArray;
class StrArrVTree;

class StrBufTree : public StrDict {

    public:
			StrBufTree();
			StrBufTree( StrDict & dict );
			StrBufTree & operator =( StrDict & dict );
			~StrBufTree();

	// virtuals of StrDict

	StrPtr *	VGetVar( const StrPtr &var );
	void		VSetVar( const StrPtr &var, const StrPtr &val );
	void		VRemoveVar( const StrPtr &var );
	int		VGetVarX( int x, StrRef &var, StrRef &val );
	int		VGetCount();
	void		VClear();

	StrDictIterator *GetIterator();

    friend class StrTreeIterator;

    protected:
	
	StrArrVTree	*tree;

} ;

class StrBufTreeIdx : public StrBufTree {

    public:
			StrBufTreeIdx();
			~StrBufTreeIdx();

	// virtuals of StrDict

	void		VSetVar( const StrPtr &var, const StrPtr &val );
	void		VRemoveVar( const StrPtr &var );
	int		VGetVarX( int x, StrRef &var, StrRef &val );
	int		VGetCount();
	void		VClear();
	StrDictIterator *GetIterator();

    private:
	
	VarArray	*keyArray;
};

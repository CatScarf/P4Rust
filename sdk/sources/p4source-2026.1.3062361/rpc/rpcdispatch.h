/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class VarArray ;
class AltDispatcher;
class Tnode;

class RpcDispatcher {
    public:

			RpcDispatcher( void );
			~RpcDispatcher( void );

	void			Add( const RpcDispatch *dispatch );
	void			AddAltDispatcher();
	void			ClearAltDispatcher();
	const RpcDispatch	*Find( const char *func ) const;

    private:

	VarArray	*dispatches;
	AltDispatcher	*altDispatcher;
	int		altindex;

} ;

// An alternative dispatcher build on the Trie
// algorithm. See support/trienode.cc

class AltDispatcher {
    public:
	AltDispatcher();
	~AltDispatcher();
	void Add( const RpcDispatch *dispatch );
	void Final();
	const RpcDispatch *Find( const char *func ) const;
    private:
	Tnode *trieNode;
	int final;
} ;

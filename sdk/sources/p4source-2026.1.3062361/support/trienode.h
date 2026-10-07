/*
 * Copyright 2020 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */


class Tnode
{
	Tnode **children;
	int nofkids;
	int offset;
	// isEndOfCommand is true if the node represents
	// end of a word
	//bool isEndOfCommand;
	const void *EndOfCmdInvokes;

    public:
	Tnode();
	~Tnode();
	void insert( const char *key, const void *item );
	const void * search( const char *key ) const;
	const void * trimsearch( const char *key ) const;
	void Trim();
	void TrimNode( Tnode *wnode );
	// Track memory use.
	static int memuse;
	static int nodes;
} ;


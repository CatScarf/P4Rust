/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

namespace P4Lua {
	
class ClientProgressP4Lua : public ClientProgress {
    public:
		ClientProgressP4Lua( void * prog, int t );
		virtual ~ClientProgressP4Lua();

    public:
    
	void	Description( const StrPtr *d, int u );
	void	Total( long t );
	int	Update( long update );
	void	Done( int f );

    private:
	void*	progress;
};

}
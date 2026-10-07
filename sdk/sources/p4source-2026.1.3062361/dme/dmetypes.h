/*
 * Copyright 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * DmtExtName - Extension name formatting
 */

enum DmtExtType {
	DET_PREFIX = 1,
	DET_NAME = 2,
	DET_FULLNAME = 3,
} ;

class DmtExtName : public StrBuf {

    public:
	        DmtExtName() {};
	        DmtExtName( const StrPtr &name, Error *e ) 
	        { ParseExt( name, DET_FULLNAME, e ); }
	bool	ParseExt( const StrPtr &name, DmtExtType t, Error *e );
} ;


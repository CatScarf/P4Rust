/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * ClientUserDebug -- version of output that always to stdout
 */

class ClientUserDebug : public ClientUser {

    public:

	ClientUserDebug( int apiVersion ) : ClientUser( 0, apiVersion  )
	{
	    outputTaggedWithErrorLevel = 1;
	}

	void		OutputTag( const char *tag, 
				const char *data, int length );

	virtual void	OutputInfo( char level, const char *data );
	virtual void 	OutputError( const char *errBuf );
	virtual void 	OutputText( const char *data, int length );

	virtual void 	HandleError( Error *err );

} ;

/*
 * ClientUserDebugMsg -- dump Error info for debugging
 */

class ClientUserDebugMsg : public ClientUserDebug {

    public:

	ClientUserDebugMsg( int apiVersion ) : ClientUserDebug( apiVersion ) {}

	virtual void	Message( Error* err );
	virtual void	Prompt( Error *err, StrBuf &rsp, int noEcho,Error *e );

} ;

/*
 * ClientUserFmt -- format output with user-specified string
 */

class ClientUserFmt : public ClientUser {

    public:

	ClientUserFmt( const StrPtr *fmt, int apiVersion = -1 )
	    : ClientUser( 0 , apiVersion ) { this->fmt = fmt; };

	void		Message( Error *err );
	void		OutputStat( StrDict *dict );

    private:

	const StrPtr *fmt;
} ;

/*
 * ClientUserMunge -- modify spec output with user-specified strings
 */

class ClientUserMunge : public ClientUser {

    public:

	ClientUserMunge( Options &opts, int autoLoginPrompt = 0,
	                 int apiVersion = -1 );

	void		OutputStat( StrDict *dict );

	static void	Munge( StrDict *spec, StrPtrArray *fields, 
			       ClientUser *ui );

    private:

	StrPtrArray fields;
	int done;
};

/*
 * ClientUserDebugProgress -- progress debug
 *
 * When built with a C++11 capable toolchain supports handles
 * progress from parallel sync/submit/etc.
 */

# ifdef HAS_CPP11
# include <mutex>
# endif

class ClientUserDebugProgress : public ClientUserProgress {

    public:

	ClientUserDebugProgress( int autoLoginPrompt, int apiVersion )
	    : ClientUserProgress( autoLoginPrompt, apiVersion ) {}

	virtual ClientProgress *CreateProgress( int, P4INT64 );
	virtual ClientProgress *CreateProgress( int );

# ifdef HAS_CPP11
	virtual void	HandleError( Error *err );
	virtual int	CanParallelProgress() { return 1; }

	std::mutex output_mutex;
# endif
} ;

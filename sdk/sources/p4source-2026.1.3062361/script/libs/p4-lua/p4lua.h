/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

class ClientUserP4Lua;
class ClientApi;
class Enviro;

namespace P4Lua {

class P4Lua
{
    friend ClientUserP4Lua;

    public:
	
	P4Lua();
	~P4Lua();

	static void doBindings( void* luaV, void* nsV,
	std::vector< std::function< void( ClientApi& ) > >* ClientApiBindCfgs );

	void SwapClientApi( ClientApi *ca ) { delete client; client = ca; }

	// Tagged mode - can be enabled/disabled on a per-command basis
	void SetTagged( bool enable );
	bool GetTagged()		{ return IsTag();			}

	// Set track mode - track usage of this command on the server
	bool SetTrack( bool enable, sol::this_state s );
	bool GetTrack()			{ return IsTrackMode() != 0;		}

	// Set streams mode
	void SetStreams( bool enable );
	bool GetStreams() 		{ return IsStreamsMode() != 0;		}

	// Set graph mode
	void SetGraph( bool enable );
	bool GetGraph()			{ return IsGraphMode() != 0;		}

	// Returns bool, but may raise exception
	int  SetCharset( const char *c );

	// Set API level for backwards compatibility
	void SetApiLevel( int level );

	void SetClient( const char *c )	{ client->SetClient( c );		}
	void SetCwd( const char *c );
	void SetEnviroFile( const char *c );
	void SetHost( const char *h )	{ client->SetHost( h );			}
	void SetIgnoreFile( const char *f ){ client->SetIgnoreFile( f );	}
	void SetMaxResults( int v )	{ maxResults = v;			}
	void SetMaxScanRows( int v )	{ maxScanRows = v;			}
	void SetMaxLockTime( int v )	{ maxLockTime = v;			}
	bool SetEnv( const char *var, const char *val, sol::this_state s );
	void SetLanguage( const char *l ){ client->SetLanguage( l );		}
	void SetPassword( const char *p ){ client->SetPassword( p );		}
	void SetPort( const char *p )	{ client->SetPort( p );			}
	void SetProg( const char *p )	{ prog = p;				}
	void SetProtocol( const char *var );
	void SetProtocol( const char *var, const char *val );
	void SetTicketFile( const char *p );
	void SetTrustFile( const char *p );
	void SetUser( const char *u )	{ client->SetUser( u );			}
	void SetVersion( const char *v ){ version = v;				}

	int GetApiLevel()		{ return apiLevel;			}
	const char *GetCharset()	{ return client->GetCharset().Text();	}
	const char *GetClient()		{ return client->GetClient().Text();	}
	const char *GetConfig()		{ return client->GetConfig().Text();	}
	const char *GetCwd()		{ return client->GetCwd().Text();	}
	const char *GetEnv( const char *v );
	const char *GetEnviroFile();
	const char *GetHost()		{ return client->GetHost().Text();	}
	const char *GetIgnoreFile()	{ return client->GetIgnoreFile().Text();}
	const char *GetLanguage()	{ return client->GetLanguage().Text();	}
	const char *GetPassword()	{ return client->GetPassword().Text();	}
	const char *GetPort()		{ return client->GetPort().Text();	}
	const char *GetProg()		{ return prog.Text();			}
	const char *GetTicketFile()	{ return ticketFile.Text();		}
	const char *GetTrustFile()	{ return trustFile.Text();		}
	const char *GetUser()		{ return client->GetUser().Text();	}
	const char *GetVersion()	{ return version.Text();		}

	bool IsIgnored( const char* path );

	long GetMaxResults()		{ return maxResults;			}
	long GetMaxScanRows()		{ return maxScanRows;			}
	long GetMaxLockTime()		{ return maxLockTime;			}
	sol::object GetServerLevel( sol::this_state s );
	bool ServerCaseSensitive( sol::this_state s );
	bool ServerUnicode( sol::this_state s );
	
	bool Connect( sol::this_state s );
	bool Connected( sol::this_state s );
	bool Disconnect( sol::this_state s );
	void Reset();

	bool SetInput( sol::object, sol::this_state s );

	sol::object
	Run( const char* cmd, const sol::variadic_args& args, sol::this_state s );


	// Result handling
	sol::table GetErrors( sol::this_state s )	{ return ui->GetResults().GetErrors(s);}
	sol::table GetWarnings( sol::this_state s )	{ return ui->GetResults().GetWarnings(s);}
	sol::table GetMessages( sol::this_state s )	{ return ui->GetResults().GetMessages(s);}
	sol::table GetTrackOutput( sol::this_state s )	{ return ui->GetResults().GetTrack(s);}

	// Spec parsing
	sol::object ParseSpec( const char * type, const char *form, sol::this_state luaState );
	sol::object FormatSpec( const char *type, sol::table hash, sol::this_state luaState );
	sol::object SpecFields( const char * type, sol::this_state luaState );
	
	// Exception levels:
	//
	// 		0 - No exceptions raised
	// 		1 - Exceptions raised for errors
	// 		2 - Exceptions raised for errors and warnings
	//
	void  SetExceptionLevel( int i )	{ exceptionLevel = i; 	}
	int   GetExceptionLevel()		{ return exceptionLevel; }

	//
	// Debugging support. Debug levels are:
	//
	//     1:	Debugs commands being executed
	//     2:	Debug UI method calls
	//     3:	Show garbage collection
	//
	int   GetDebug() { return debug; }
	void  SetDebug( int d );

	// Handler support

	void SetHandler( void* handler );
	void* GetHandler() { return ui->GetHandler(); }

	// Progress API support
	void SetProgress( void* progress );
	void* GetProgress() { return ui->GetProgress(); }
	
    private:

	void RunCmd(const char *cmd, int argc, char * const *argv);

	bool ConnectOrReconnect( sol::this_state s );	// internal connect method

	enum {
	    S_TAGGED 		= 0x0001,
	    S_CONNECTED		= 0x0002,
	    S_CMDRUN		= 0x0004,
	    S_UNICODE		= 0x0008,
	    S_CASEFOLDING	= 0x0010,
	    S_TRACK		= 0x0020,
	    S_STREAMS		= 0x0040,
	    S_GRAPH		= 0x0080,

	    // Streams, Graph, and Tagged enabled by default
	    S_INITIAL_STATE	= 0x00C1,
	    S_RESET_MASK	= 0x001E,
	};

	void	InitFlags()		{ flags = S_INITIAL_STATE;	}
	void	ResetFlags()		{ flags &= ~S_RESET_MASK;	}

	void	SetTag()		{ flags |= S_TAGGED;		}
	void	ClearTag()		{ flags &= ~S_TAGGED;		}
	bool	IsTag()			{ return flags & S_TAGGED;	}

	void	SetConnected()		{ flags |= S_CONNECTED;		}
	void	ClearConnected() 	{ flags &= ~S_CONNECTED;	}
	bool	IsConnected()		{ return flags & S_CONNECTED;	}

	void	SetCmdRun()		{ flags |= S_CMDRUN;		}
	void	ClearCmdRun() 		{ flags &= ~S_CMDRUN;		}
	bool	IsCmdRun()		{ return flags & S_CMDRUN;	}

	void	SetUnicode()		{ flags |= S_UNICODE;		}
	void	ClearUnicode() 		{ flags &= ~S_UNICODE;		}
	bool	IsUnicode()		{ return flags & S_UNICODE;	}

	void	SetCaseFold()		{ flags |= S_CASEFOLDING;	}
	void	ClearCaseFold()		{ flags &= ~S_CASEFOLDING;	}
	bool	IsCaseFold()		{ return flags & S_CASEFOLDING;	}

	void	SetTrackMode()		{ flags |= S_TRACK;		}
	void	ClearTrackMode()	{ flags &= ~S_TRACK;		}
	bool	IsTrackMode()		{ return flags & S_TRACK;	}

	void	SetStreamsMode()	{ flags |= S_STREAMS;		}
	void	ClearStreamsMode()	{ flags &= ~S_STREAMS;		}
	bool	IsStreamsMode()		{ return flags & S_STREAMS;	}

	void	SetGraphMode()		{ flags |= S_GRAPH;		}
	void	ClearGraphMode()	{ flags &= ~S_GRAPH;		}
	bool	IsGraphMode()		{ return flags & S_GRAPH;	}

    private:

	ClientUserP4Lua *ui;
	ClientApi *client;
	SpecMgrP4Lua *specMgr;
	Enviro *enviro;
	StrBuf prog;
	StrBuf version;
	StrBuf ticketFile;
	StrBuf trustFile;
	int	depth;
	int	debug;
	int	exceptionLevel;
	int	apiLevel;
	int	server2;
	int	flags;
	long	maxResults;
	long	maxScanRows;
	long	maxLockTime;
};

}
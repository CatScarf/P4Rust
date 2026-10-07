/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

#ifndef __CLIENTSERVICE__
#define __CLIENTSERVICE__

class Client;
class ClientFile;
extern const RpcDispatch clientDispatch[];

void clientBailoutFile( void *arg );
void clientOpenFile( Client *client, Error *e );
void clientWriteFile( Client *client, Error *e );
void clientCloseFile( Client *client, Error *e );
void clientDeleteFile( Client *client, Error *e );
void clientChmodFile( Client *client, Error *e );
void clientConvertFile( Client *client, Error *e );
void clientCheckFile( Client *client, Error *e );
void clientReconcileEdit( Client *client, Error *e );
void clientReconcileAdd( Client *client, Error *e );
void clientReconcileFlush( Client *client, Error *e );
void clientRenameFile( Client *client, Error *e );
void clientActionResolve( Client *client, Error *e );
void clientBailoutMerge( void *arg );
void clientOpenMerge( Client *client, Error *e );
void clientWriteMerge( Client *client, Error *e );
void clientCloseMerge( Client *client, Error *e );
void clientSendFile( Client *client, Error *e );
void clientEditData( Client *client, Error *e );
void clientInputData( Client *client, Error *e );
void clientErrorPause( Client *client, Error *e );
void clientOutputError( Client *client, Error *e );
void clientOutputInfo( Client *client, Error *e );
void clientOutputText( Client *client, Error *e );
void clientFstatInfo( Client *client, Error *e );
void clientAck( Client *client, Error *e );
void clientCheckCharSet( Client *client, Error *e );

void clientReconcileEdit( Client *client, Error *e );
void clientReconcileAdd( Client *client, Error *e );
void clientReconcileFlush( Client *client, Error *e );
void clientExactMatch( Client *client, Error *e );
void clientOpenMatch( Client *client, ClientFile *f, Error *e );
void clientCloseMatch( Client *client, ClientFile *f, Error *e );
void clientAckMatch( Client *client, Error *e );

const char * clientCheckFileType( FileSys *f, FileSysType t,
				  int clientProtocolFiles,
				  int checkSize, int scan, StrPtr *wildType,
				  StrPtr *forceType, StrPtr *msgType,
				  Error *e );

int AltSyncCheckFile( Client *client, StrPtr *confirm,
                      const char *status, const char *ntype, Error *e );

void clientStats( Client* client, Error* e );

enum XDir { FromServer, FromClient };

class ClientSvc
{
    public:
	static FileSys		*File( Client *client, Error *e );
	static FileSys		*FileFromPath( Client *client,
				               const char *vName, Error *e );
	static FileSys		*FileFromPath( Client *client,
				               const char *vName,
				               const char *vType, Error *e );
	static FileSys		*FileFromPathNoVar( Client *client,
				               const StrPtr *clientPath,
				               const FileSysType type,
				               const StrPtr *utf8bom, Error *e );
	
	static int		CheckFilePath( Client *client, FileSys *f,
				               Error *e, int skipClientPath = 0 );

	static CharSetCvt	*XCharset( Client *client, XDir d );

	// True when the on-disk byte count equals the LF-equivalent count the
	// server records, so GetSize() answers without reading the file.

	static bool		SizeMatchesDepot( FileSysType t,
				                  CharSetCvt *cvt );
};

/*
 * ClientFile - handle client interaction with files
 */

# ifdef USE_CDC
class StrIntsTree;
# endif

class ProgressReport;
class ClientFile : public LastChance {

    public:
			ClientFile( FileSys *fs = 0 );
			~ClientFile();

    public:

	FileSys		*file;
	FileSys		*indirectFile;

	int		isDiff;

	StrBuf		diffName;
	StrBuf		diffFlags;

	StrBuf		symTarget;

	StrBuf		serverDigest;
	MD5		*checksum;

	StrBufDict	*matchDict;

	ProgressReport	*progress;

# ifdef USE_CDC
	StrIntsTree	*chunkOffsetTree;
# endif
} ;

#endif // __CLIENTSERVICE__

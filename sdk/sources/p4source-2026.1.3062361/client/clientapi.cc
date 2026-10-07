/*
 * Copyright 1995, 1996 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * clientapi.cc -- a Client wrapper that isn't subclassed from Rpc
 */

# include "clientapi.h"

# include <rpc.h>

# include "client.h"

	ClientApi::ClientApi() { client = new Client; ui = 0; ourClient = 1; }
	ClientApi::ClientApi( Enviro *env ) { client = new Client( env ); ui = 0; ourClient = 1; }
	ClientApi::ClientApi( ClientUser *i ) { client = new Client; ui = i; ourClient = 1; }
	ClientApi::ClientApi( Client *c ) { client = c; ui = 0; ourClient = 0; }
	ClientApi::~ClientApi() { if( ourClient ) delete client; }

void	ClientApi::SetTrans( int output, int content, int fnames, int dialog )
		{ client->SetTrans(output, content, fnames, dialog); }

void 	ClientApi::SetUi( ClientUser *i ) { ui = i; }

void 	ClientApi::Init( Error *e ) { client->Init( e ); }
void 	ClientApi::Run( const char *func ) { client->Run( func, ui ); }
void 	ClientApi::Run( const char *func, ClientUser *i ) { client->Run( func, i ); }
int 	ClientApi::Final( Error *e ) { return client->Final( e ); }
int 	ClientApi::Dropped() { return client->Dropped(); }
int 	ClientApi::GetErrors() { return client->GetErrors(); }
int 	ClientApi::GetFatals() { return client->GetFatals(); }
int	ClientApi::GetTrans() { return client->output_charset; }
int	ClientApi::IsUnicode() { return client->IsUnicode(); }

void	ClientApi::RunTag( const char *f, ClientUser *u ) { client->RunTag( f, u ); }
void	ClientApi::WaitTag( ClientUser *u ) { client->WaitTag( u ); }

StrPtr	*ClientApi::VGetVar( const StrPtr &var ) { return client->translated->GetVar( var ); }
void	ClientApi::VSetVar( const StrPtr &var, const StrPtr &val ) { client->translated->SetVar( var, val ); }
void	ClientApi::VSetVarV( const char *var ) { client->SetVarV( var ); }
int	ClientApi::VGetCount() { return client->translated->GetCount(); }
StrPtr	*ClientApi::GetEVar( const StrPtr &var ) { return client->GetEVar( &var ); }
void	ClientApi::SetEVar( const StrPtr &var, const StrPtr &val ) { client->SetEVar( &var, &val ); }

void 	ClientApi::SetArgv( int ac, char *const *av ) { client->SetArgv( ac, av ); }
void 	ClientApi::SetCharset( const char *c ) { client->SetCharset( c ); }
void 	ClientApi::SetClient( const char *c ) { client->SetClient( c ); }
void 	ClientApi::SetCwd( const char *c ) { client->SetCwd( c ); }
void 	ClientApi::SetCwdNoReload( const char *c ) { client->SetCwdNoReload( c ); }
void 	ClientApi::SetHost( const char *c ) { client->SetHost( c ); }
void	ClientApi::SetIgnoreFile( const char *c ) { client->SetIgnoreFile( c ); }
void 	ClientApi::SetLanguage( const char *c ) { client->SetLanguage( c ); }
void 	ClientApi::SetPassword( const char *c ) { client->SetPassword( c ); }
void 	ClientApi::SetPort( const char *c ) { client->SetPort( c ); }
void 	ClientApi::SetUser( const char *c ) { client->SetUser( c ); }
void 	ClientApi::SetProg( const char *c ) { client->SetProg( c ); }
void	ClientApi::SetVersion( const char *c ) { client->SetVersion( c ); }
void 	ClientApi::SetTicketFile( const char *c ) { client->SetTicketFile( c ); }
void 	ClientApi::SetTrustFile( const char *c ) { client->SetTrustFile( c ); }
void 	ClientApi::SetEnviroFile( const char *c ) { client->SetEnviroFile( c ); }

void 	ClientApi::SetCharset( const StrPtr *c ) { client->SetCharset( c ); }
void 	ClientApi::SetClient( const StrPtr *c ) { client->SetClient( c ); }
void 	ClientApi::SetCwd( const StrPtr *c ) { client->SetCwd( c ); }
void 	ClientApi::SetCwdNoReload( const StrPtr *c ) { client->SetCwdNoReload( c ); }
void 	ClientApi::SetExecutable( const StrPtr *c ) { client->SetExecutable( c ); }
void 	ClientApi::SetHost( const StrPtr *c ) { client->SetHost( c ); }
void	ClientApi::SetIgnoreFile( const StrPtr *c ) { client->SetIgnoreFile( c ); }
void 	ClientApi::SetLanguage( const StrPtr *c ) { client->SetLanguage( c ); }
void 	ClientApi::SetPassword( const StrPtr *c ) { client->SetPassword( c ); }
void 	ClientApi::SetPort( const StrPtr *c ) { client->SetPort( c ); }
void 	ClientApi::SetUser( const StrPtr *c ) { client->SetUser( c ); }
void 	ClientApi::SetProg( const StrPtr *c ) { client->SetProg( c ); }
void	ClientApi::SetVersion( const StrPtr *c ) { client->SetVersion( c ); }
void 	ClientApi::SetTicketFile( const StrPtr *c ) { client->SetTicketFile( c ); }
void 	ClientApi::SetTrustFile( const StrPtr *c ) { client->SetTrustFile( c ); }
void 	ClientApi::SetEnviroFile( const StrPtr *c ) { client->SetEnviroFile( c ); }

void	ClientApi::SetBreak( KeepAlive *k ) { client->SetBreak( k ); }
KeepAlive *	ClientApi::GetBreak() { return client->GetBreak(); }

void 	ClientApi::DefineCharset( const char *c, Error *e ) { client->DefineCharset( c, e ); }
void 	ClientApi::DefineClient( const char *c, Error *e ) { client->DefineClient( c, e ); }
void 	ClientApi::DefineHost( const char *c, Error *e ) { client->DefineHost( c, e ); }
void 	ClientApi::DefineLanguage( const char *c, Error *e ) { client->DefineLanguage( c, e ); }
void 	ClientApi::DefinePassword( const char *c, Error *e ) { client->DefinePassword( c, e ); }
void 	ClientApi::DefinePort( const char *c, Error *e ) { client->DefinePort( c, e ); }
void 	ClientApi::DefineUser( const char *c, Error *e ) { client->DefineUser( c, e ); }

const int &ClientApi::GetAPI() { return client->GetAPI(); }
const StrPtr & ClientApi::GetCharset() { return client->GetCharset(); }
const StrPtr & ClientApi::GetClient() { return client->GetClient(); }
const StrPtr & ClientApi::GetClientNoHost() { return client->GetClientNoHost(); }
const StrPtr & ClientApi::GetCwd() { return client->GetCwd(); }
const StrPtr & ClientApi::GetExecutable() { return client->GetExecutable(); }
const StrPtr & ClientApi::GetHost() { return client->GetHost(); }
const StrPtr & ClientApi::GetIgnoreFile() { return client->GetIgnoreFile(); }
const StrPtr & ClientApi::GetLanguage() { return client->GetLanguage(); }
const StrPtr & ClientApi::GetOs() { return client->GetOs(); }
const StrPtr & ClientApi::GetPassword() { return client->GetPassword(); }
const StrPtr & ClientApi::GetPassword2() { return client->GetPassword2(); }
const StrPtr & ClientApi::GetPort() { return client->GetPort(); }
const StrPtr & ClientApi::GetProg() { return client->GetProg(); }
const StrPtr & ClientApi::GetUser() { return client->GetUser(); }
const StrPtr & ClientApi::GetConfig() { return client->GetConfig(); }
const StrArray* ClientApi::GetConfigs() { return client->GetConfigs(); }
const StrPtr & ClientApi::GetBuild() { return client->GetBuild(); }
const StrPtr & ClientApi::GetVersion() { return client->GetVersion(); }
Error * ClientApi::GetTransError() { return client->GetTransError(); }

Enviro* ClientApi::GetEnviro() { return client->GetEnviro(); }
Ignore *ClientApi::GetIgnore() { return client->GetIgnore(); }

void	ClientApi::SetIgnorePassword() { client->SetIgnorePassword(); }

void 	ClientApi::SetProtocol( const char *p, const char *v ) { client->SetProtocol( p, v ); }
void 	ClientApi::SetProtocol( const char *p ) { client->SetProtocol( p ); }
void 	ClientApi::SetProtocolV( const char *p ) { client->SetProtocolV( p ); }
StrPtr	*ClientApi::GetProtocol( const char *v ) { return client->GetProtocol( StrRef( v ) ); }

void 	ClientApi::EnableExtensions( Error* e ) { client->EnableExtensions( e ); }
void 	ClientApi::DisableExtensions() { client->DisableExtensions(); }
bool 	ClientApi::ExtensionsEnabled() { return client->ExtensionsEnabled(); }
void	ClientApi::SetExtension( ClientScript* cs, Error* e, const bool callerOwns ) { client->SetExtension( cs, e, callerOwns ); }
void	ClientApi::EnableDebugHooks() { client->EnableDebugHooks(); }

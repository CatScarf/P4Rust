/*
 * Copyright 2018 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * msgserver.cc - definitions of errors for scripting subsystem.
 *
 * Note:
 *
 * Never re-use an error code value,  these may have already been 
 * translated, so using it for a different error is not OK.
 *
 * ErrorIds which are no longer used should be moved to the bottom
 * of the list, with a trailing comment like this: // DEPRECATED.
 * We keep these to maintain compatibility between newer api clients
 * and older servers which send old ErrorIds.
 *
 * Its okay to add a message in the middle of the file.
 *
 * When adding a new error make sure its greater than the current high
 * value and update the following number:
 *
 * Current high value for a MsgServer error code is: 24
 */
# include <stdhdrs.h>
# include <error.h>
# include <errornum.h>
# include "msgscript.h"

ErrorId MsgScript::ScriptRuntimeError  = { ErrorOf( ES_SCRIPT, 1, E_FATAL, EV_FAULT, 2 ), "%lang% runtime error:  %error%." } ;
ErrorId MsgScript::ScriptMaxRunErr     = { ErrorOf( ES_SCRIPT, 2, E_FATAL, EV_FAULT, 2 ), "Script ran over %type% limit of %limit%[ %suffix%|]." } ;
ErrorId MsgScript::DoNotBlameTheScript = { ErrorOf( ES_SCRIPT, 3, E_FATAL, EV_FAULT, 0 ), "Error object passed to scripting already set with an error." } ;

ErrorId MsgScript::ExtAddChangeDesc    = { ErrorOf( ES_SCRIPT, 4, E_INFO , EV_NONE , 2 ), "Installing extension: %extName%\nVersion: %extVersion%" } ;
ErrorId MsgScript::ExtEditChangeDesc   = { ErrorOf( ES_SCRIPT, 5, E_INFO , EV_NONE , 3 ), "Updating extension: %extName%\nVersion: %toExtVersion%\nPrevious version: %fromExtVersion%" } ;
ErrorId MsgScript::ExtOverChangeDesc   = { ErrorOf( ES_SCRIPT, 15, E_INFO , EV_NONE , 2 ), "Overwriting extension: %extName%\nVersion: %toExtVersion%" } ;
ErrorId MsgScript::ExtDelChangeDesc    = { ErrorOf( ES_SCRIPT, 20, E_INFO , EV_NONE , 2 ), "Deleting extension: %extName%%optVersion%" } ;
ErrorId MsgScript::ExtCertAddChangeDesc= { ErrorOf( ES_SCRIPT, 22, E_INFO , EV_NONE , 2 ), "Installing Extension certificate for %orgCommonName%\n%desc%." } ;
ErrorId MsgScript::ExtCertDelChangeDesc= { ErrorOf( ES_SCRIPT, 23, E_INFO , EV_NONE , 2 ), "Deleting Extension certificate for %orgCommonName%\n%desc%." } ;
// Note:  extProds is a list, which ought to be separate arguments
// for proper translation, but we have to hardcode the number of arguments here.
ErrorId MsgScript::ExtWrongProduct     = { ErrorOf( ES_SCRIPT, 18, E_FATAL , EV_NONE , 2 ), "Extension is incompatible with '%prod%', supports '%extProds%'." };

ErrorId MsgScript::ExtLoadErr          = { ErrorOf( ES_SCRIPT, 6, E_FATAL, EV_FAULT, 2 ), "Could not load extension '%extName%', rev '%extRev%'." } ;
ErrorId MsgScript::ExtDisabled         = { ErrorOf( ES_SCRIPT, 7, E_FATAL, EV_FAULT, 3 ), "Extension '%name%:%rev%', config '%cfg%' not enabled in its configuration." } ;
ErrorId MsgScript::ExtCodingErr        = { ErrorOf( ES_SCRIPT, 8, E_FATAL, EV_FAULT, 3 ), "Extension '%name%:%rev%' has a coding error:  %err%" } ;
ErrorId MsgScript::ExtCodingGenErr     = { ErrorOf( ES_SCRIPT, 18, E_FATAL, EV_FAULT, 1 ), "Extension coding error: %err%" } ;
ErrorId MsgScript::ExtResourceErr      = { ErrorOf( ES_SCRIPT, 15, E_FATAL, EV_FAULT, 2 ), "Error loading resource '%resource%': %error%" } ;

ErrorId MsgScript::DevErr              = { ErrorOf( ES_SCRIPT, 9, E_FATAL, EV_FAULT, 1 ), "Programming error:  %err%" };
ErrorId MsgScript::ScriptLangUnknown   = { ErrorOf( ES_SCRIPT, 16, E_FATAL, EV_FAULT, 1 ), "Unknown script language '%lang%'." };
ErrorId MsgScript::ScriptLangVerUnknown= { ErrorOf( ES_SCRIPT, 17, E_FATAL, EV_FAULT, 1 ), "Unknown script language '%lang%' version '%ver%'." };
ErrorId MsgScript::OsExitRealError     = { ErrorOf( ES_SCRIPT, 21, E_FATAL, EV_NONE, 0 ), "The Lua os.execute() function was called." };
ErrorId MsgScript::GenericFatal        = { ErrorOf( ES_SCRIPT, 24, E_FATAL, EV_NONE, 1 ), "Script error:  %err%." };

// Client-side Extensions //////////////////////////////////////////////////////

ErrorId MsgScript::ExtClientMsg        = { ErrorOf( ES_SCRIPT, 10, E_INFO, EV_NONE, 2 ), "Client Extension message (%extName%): %message%" };
ErrorId MsgScript::ExtClientError      = { ErrorOf( ES_SCRIPT, 11, E_FATAL, EV_NONE, 2 ), "Client Extension message (%extName%): %message%" };
ErrorId MsgScript::ExtClientPrompt     = { ErrorOf( ES_SCRIPT, 12, E_INFO, EV_NONE, 2 ), "Client Extension prompt (%extName%): %question%" };
ErrorId MsgScript::ExtClientCmdRejected= { ErrorOf( ES_SCRIPT, 13, E_FATAL, EV_NONE, 1 ), "Command rejected in '%callback%' by client Extension (%extName%)." };
ErrorId MsgScript::ExtClientRuntimeFail= { ErrorOf( ES_SCRIPT, 14, E_FATAL, EV_NONE, 2 ), "Client Extension (%extName%) failed:  %error%" };
ErrorId MsgScript::ExtScriptNotInBuild = { ErrorOf( ES_SCRIPT, 19, E_FATAL, EV_FAULT, 0 ), "Script/Extension support not present in this build." };

// ErrorId graveyard: retired/deprecated ErrorIds. 

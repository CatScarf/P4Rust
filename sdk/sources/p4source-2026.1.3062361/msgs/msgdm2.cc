/*
 * Copyright 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * msgdm2.cc - definitions of errors for overflow data manager core subsystem.
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
 * Current high value for a MsgDm2 error code is:
 *                                175 (MultiplePartitionedRevSpec)
 *                                               Max code is 1023!!!
 */
# include <stdhdrs.h>
# include <error.h>
# include <errornum.h>
# include <msgdm2.h>

ErrorId MsgDm2::ExistingStorage         = { ErrorOf( ES_DM2, 1, E_FAILED, EV_EMPTY, 1 ), "Bad zero count storage (%type%) record detected after file transfer." } ;
ErrorId MsgDm2::ConfigHistData          = { ErrorOf( ES_DM2, 2, E_INFO, EV_EMPTY, 9 ), "%sname%#%name% changed from '%ovalue%' to '%nvalue%' (iteration %version%) by '%user%' on %date% local to server %server%[ with comment '%comment%']" };
ErrorId MsgDm2::LbrScanBadState         = { ErrorOf( ES_DM2, 3, E_FAILED, EV_NONE, 2 ), "Lbrscan cannot change state from '%oldstate%' to '%newstate%'." } ;
ErrorId MsgDm2::LbrScanCtlNotFound      = { ErrorOf( ES_DM2, 4, E_FAILED, EV_NONE, 1 ), "No current scan for path (%record%)" } ;
ErrorId MsgDm2::RequiresAutoIdCode      = { ErrorOf( ES_DM2, 5, E_FAILED, EV_FAULT, 0 ), "New field code in '%type%' spec must be entered as 'NNN'" } ;
ErrorId MsgDm2::SpecMissingBuiltin      = { ErrorOf( ES_DM2, 6, E_FAILED, EV_FAULT, 1 ), "Spec is missing builtin field: '%field%'" } ;
ErrorId MsgDm2::RequiresAutoIdOrPosCode = { ErrorOf( ES_DM2, 7, E_FAILED, EV_FAULT, 0 ), "New field code in '%type%' spec must be entered as 'NNN' or positive int." } ;
ErrorId MsgDm2::CannotRecreateDeleteField = { ErrorOf( ES_DM2, 8, E_FAILED, EV_FAULT, 0 ), "New field code '%code% %tag%' was previously deleted and may not be recreated." } ;
ErrorId MsgDm2::SpecRepairDisallowNNN = { ErrorOf( ES_DM2, 18, E_FAILED, EV_FAULT, 1 ), "'NNN' fields are disallowd during 'p4 spec --repair' :  field '%tag%'." } ;
ErrorId MsgDm2::SpecRepairNoCustomSpec = { ErrorOf( ES_DM2, 19, E_FAILED, EV_FAULT, 0 ), "--repair disallowed on default spec." } ;

ErrorId MsgDm2::IntegIntoReadOnlyOverlay = { ErrorOf( ES_DM2, 108, E_INFO, EV_NONE, 2 ), "%depotFile% - can't %action% into file that is overlaid in client's View" } ;
ErrorId MsgDm2::IntegRequiresStream      = { ErrorOf( ES_DM2, 140, E_FAILED, EV_CONTEXT, 2 ), "Must use a stream view to %action% into %stream%."} ;
ErrorId MsgDm2::IntegStreamSyntaxHint    = { ErrorOf( ES_DM2, 141, E_INFO, EV_USAGE, 3 ), "Try running: 'p4 %cmd%[ --from %fromStream%][ %toFile%]'"} ;

ErrorId MsgDm2::OpenReadOnlyOverlay      = { ErrorOf( ES_DM2, 109, E_INFO, EV_NONE, 2 ), "%depotFile% - can't %action% file that is overlaid in client's View" } ;
ErrorId MsgDm2::OpenWarnOverlay          = { ErrorOf( ES_DM2, 110, E_INFO, EV_USAGE, 2 ), "%depotFile% - warning: cannot submit file that is overlaid in client's View" } ;
ErrorId MsgDm2::OpenUndoConflict        = { ErrorOf( ES_DM2, 114, E_FAILED, EV_NOTYET, 6), "%depotFile% - can't %action% (undo @%startChange%,@%endChange%) and %action2% at the same time[ (try undoing changes after @%moveChange% first?)]" } ;

ErrorId MsgDm2::UnshelveBadOverlay       = { ErrorOf( ES_DM2, 111, E_FAILED, EV_NOTYET, 2 ), "%depotFile% - can't unshelve from revision at change %change% (overlaid in client's View)" } ;
ErrorId MsgDm2::UnshelveStreamResolve    = { ErrorOf( ES_DM2, 9, E_INFO, EV_USAGE, 3 ), "%streamSpec% - must resolve shelved stream spec %streamSpec%@%change% before submitting" } ;

ErrorId MsgDm2::StreamSpecIntegOkay     = { ErrorOf( ES_DM2, 10, E_INFO, EV_NONE, 8 ), "Stream spec %targetStreamSpec%@%targetChange% - %action% field %field% from %sourceStreamSpec%@%fromChange%[ using base %baseStreamSpec%][@%baseChange%]" } ;
ErrorId MsgDm2::CheckFailedNoDB         = { ErrorOf( ES_DM2, 11, E_FAILED, EV_NONE, 1 ), "%table% not found in default/specified server root" } ;
ErrorId MsgDm2::NoStreamSpecPermsWarn   = { ErrorOf( ES_DM2, 12, E_INFO, EV_PROTECT, 0 ), "You don't have streamspec permission for this operation." } ;
ErrorId MsgDm2::StreamSpecProtectsNotCompatible   = { ErrorOf( ES_DM2, 13, E_FAILED, EV_ADMIN, 0 ), "P4Admin tool version is not compatible with streamSpec permissions.\nThe protection table currently contains streamspec permissions.\nYou must use P4Admin with release >= 2020.1 or p4 to administer the protection table." } ;
ErrorId MsgDm2::StreamOpenedByUser      = { ErrorOf( ES_DM2, 14, E_INFO, EV_NONE, 5 ), "Stream %stream%[@%haveChange%] - %action% stream spec %change% by %user%" } ;
ErrorId MsgDm2::StreamOpenReOpen        = { ErrorOf( ES_DM2, 15, E_INFO, EV_NONE, 3 ), "%stream%[@%haveChange%] - reopened for %action%" } ;
ErrorId MsgDm2::RemoteLabelOpenFailed   = { ErrorOf( ES_DM2, 16, E_FAILED, EV_FAULT, 1 ), "Failed to get global label %domainName% on commit server." } ;
ErrorId MsgDm2::RemoteLabelUpdateFailed = { ErrorOf( ES_DM2, 17, E_FAILED, EV_FAULT, 1 ), "Failed to update global label %domainName% on commit server." } ;
ErrorId MsgDm2::RemoteStreamUpdateFailed = { ErrorOf( ES_DM2, 20, E_FAILED, EV_FAULT, 1 ), "Failed to update stream %streamName% on commit server." } ;
ErrorId MsgDm2::StreamParentViewNoChange = { ErrorOf( ES_DM2, 21, E_FAILED, EV_FAULT, 1 ), "Failed to change the ParentView field to %value%.\nThe ParentView property of a stream specification can only be changed using the command\n  'p4 stream parentview'\nSee 'p4 help stream' for details." } ;
ErrorId MsgDm2::LbrRevVerOutOfRange     = { ErrorOf( ES_DM2, 22, E_FAILED, EV_NONE, 1 ), "LbrRev suffix has reached the Max limit %oname%. The shelf Suffix cannot be incremented." } ;
ErrorId MsgDm2::GblLockIndexMismatch    = { ErrorOf( ES_DM2, 23, E_FAILED, EV_FAULT, 2 ), "Index mismatch! Wrong depotFile: expected '%file1%', got '%file2%'" } ;
ErrorId MsgDm2::GblLockIndexMissing     = { ErrorOf( ES_DM2, 24, E_FAILED, EV_FAULT, 1 ), "Index missing! No index found for depotFile'%dfile%'" } ;
ErrorId MsgDm2::GblLockMissing          = { ErrorOf( ES_DM2, 25, E_FAILED, EV_FAULT, 1 ), "Commit server didn't report back on locking '%dfile%'" } ;
ErrorId MsgDm2::GblUnlockMissing        = { ErrorOf(ES_DM2, 94, E_FAILED, EV_FAULT, 1), "Commit server didn't report back on unlocking '%dfile%'" };
ErrorId MsgDm2::StreamlogInteg          = { ErrorOf( ES_DM2, 26, E_INFO, EV_UNKNOWN, 4 ), "%how% %fromFile%%fromRev% '%field%' " } ;
ErrorId MsgDm2::RemoteAutoGenSpecFailed = { ErrorOf( ES_DM2, 27, E_FAILED, EV_FAULT, 1 ), "Failed to autogen field id in %specName% spec on commit server." } ;
ErrorId MsgDm2::StreamParentViewMustBeOpen = { ErrorOf( ES_DM2, 28, E_FAILED, EV_FAULT, 1 ), "Stream spec %streamname% must be open in the current client to change the Parent View field." } ;
ErrorId MsgDm2::StreamPVSourceComment = { ErrorOf( ES_DM2, 29, E_INFO, EV_EMPTY, 2 ), " %how% %streamname%@%change%" } ;
ErrorId MsgDm2::BeginUpgradeStep        = { ErrorOf( ES_DM2, 30, E_INFO, EV_NONE, 2 ), "Upgrade step \"%description%\" beginning on pid %pid%." } ;
ErrorId MsgDm2::EndUpgradeStep          = { ErrorOf( ES_DM2, 31, E_INFO, EV_NONE, 2 ), "Upgrade step \"%description%\" ended with status \"%status%\"." } ;
ErrorId MsgDm2::StreamNoCmtClientBadSave= { ErrorOf( ES_DM2, 32, E_FAILED, EV_UPGRADE, 1 ), "The stream specification for %stream% contains comments.\nThis client program is too old to update this stream spec.\nThe client program must be fully compatible with the 2020.2 server release in order to update a stream specification with comments." } ;
ErrorId MsgDm2::ConnNeedsFwdCrypto      = { ErrorOf( ES_DM2, 33, E_FAILED, EV_UNKNOWN, 0 ), "Connection to upstream is already established without required authentication parameters!" } ;
ErrorId MsgDm2::NoStreamTypeChangePV    = { ErrorOf( ES_DM2, 34, E_FAILED, EV_NOTYET, 2 ), "Failed to change stream %stream% to type %streamType% because it has a noinherit ParentView.  Change the ParentView to inherit with the command 'p4 stream parentview --inherit' first, then change the stream type.\nSee 'p4 help stream' for details." } ;
ErrorId MsgDm2::PurgeTaskStream         = { ErrorOf( ES_DM2, 35, E_FAILED, EV_ILLEGAL, 1 ), "One or more task stream paths will be obliterated.\nThe '-T' option is required for arguments which include task stream paths: %taskpath%" } ;
ErrorId MsgDm2::PurgeCheckWldDelIgn     = { ErrorOf( ES_DM2, 36, E_INFO, EV_NONE, 7 ), "Would delete [%onHave% client ][%onLabel% label ][%onInteg% integration ][%onWorking% opened ][%onRev% revision ]ignoring [%onRevArchive% archived-revision ][and add %synInteg% integration ]record(s)." } ;
ErrorId MsgDm2::PurgeCheckWldDel        = { ErrorOf( ES_DM2, 37, E_INFO, EV_NONE, 7 ), "Would delete [%onHave% client ][%onLabel% label ][%onInteg% integration ][%onWorking% opened ][%onRev% revision ][%onRevArchive% archived-revision ][and add %synInteg% integration ]record(s)." } ;
ErrorId MsgDm2::PurgeCheckIgn           = { ErrorOf( ES_DM2, 38, E_INFO, EV_NONE, 7 ), "[%onHave% client ][%onLabel% label ][%onInteg% integration ][%onWorking% opened ][%onRev% revision ]Ignoring [%onRevArchive% archived-revision ][and add %synInteg% integration ]record(s)." } ;
ErrorId MsgDm2::PurgePurgeCheckWldDelIgn = { ErrorOf( ES_DM2, 39, E_INFO, EV_NONE, 7 ), "Would purge [%onHave% client ][%onLabel% label ][%onInteg% integration ][%onWorking% opened ][%onRev% revision ]ignoring [%onRevArchive% archived-revision ][and add %synInteg% integration ]record(s)." } ;
ErrorId MsgDm2::PurgePurgeCheckWldDel   = { ErrorOf( ES_DM2, 40, E_INFO, EV_NONE, 7 ), "Would purge [%onHave% client ][%onLabel% label ][%onInteg% integration ][%onWorking% opened ][%onRev% revision ][%onRevArchive% archived-revision ][and add %synInteg% integration ]record(s)." } ;
ErrorId MsgDm2::PurgePurgeCheckIgn      = { ErrorOf( ES_DM2, 41, E_INFO, EV_NONE, 7 ), "[%onHave% client ][%onLabel% label ][%onInteg% integration ][%onWorking% opened ][%onRev% revision ]Ignoring [%onRevArchive% archived-revision ][and add %synInteg% integration ]record(s)." } ;
ErrorId MsgDm2::StreamAtChangeDeleted   = { ErrorOf( ES_DM2, 42, E_FAILED, EV_CONTEXT, 2 ), "Stream '%stream%' was deleted after change %change%" } ;
ErrorId MsgDm2::StreamNotOpenInChange   = { ErrorOf( ES_DM2, 43, E_FAILED, EV_CONTEXT, 2 ), "Stream '%stream%' not open in change %change%" } ;
ErrorId MsgDm2::IdHasWhitespace         = { ErrorOf( ES_DM2, 44, E_FAILED, EV_USAGE, 1 ), "Whitespace characters not allowed in '%id%'." } ;
ErrorId MsgDm2::IdHasEquals             = { ErrorOf( ES_DM2, 45, E_FAILED, EV_USAGE, 1 ), "Equals character not allowed in '%id%'." } ;
ErrorId MsgDm2::RmtAddTopologyFailed    = { ErrorOf( ES_DM2, 46, E_FAILED, EV_FAULT, 5 ), "Server access failed while trying to add topology record (SrcAddr:'%address%' DestAddr:'%destaddress%' and SvrID:'%serverID%') from SrcServer:'%address%' to TargetServer:'%targetaddress%'." } ;
ErrorId MsgDm2::RmtTopologyExists       = { ErrorOf( ES_DM2, 47, E_INFO, EV_NONE, 2 ), "Topology [SrcAddr:'%addr%' and DestAddr:'%daddr%'] already exists in this installation." } ;
ErrorId MsgDm2::ImportDittoGraph        = { ErrorOf( ES_DM2, 48, E_FAILED, EV_USAGE, 0 ), "Import& Path is not allowed with any graph depot Path." } ;
ErrorId MsgDm2::ReopenHasMoved          = { ErrorOf( ES_DM2, 49, E_INFO, EV_NONE, 2 ), "%depotFile%%haveRev% - has been moved, not reopened" } ;
ErrorId MsgDm2::TopologyData            = { ErrorOf( ES_DM2, 50, E_INFO, EV_NONE, 12 ), "%address% [%destaddress% ][%serverID% ]%date% [%type% ][%encryption% ][%svcUser% ][%lsDate% ][%svrRecType% ][%tAddr% ][%tDAddr% ][%tSvrID% ]" } ;
ErrorId MsgDm2::StreamViewMatchData     = { ErrorOf( ES_DM2, 51, E_INFO, EV_NONE, 6 ), "Stream %stream% %pathtype% %viewPath% %depotPath%[@%depotRev%][ (%status%)]" } ;
ErrorId MsgDm2::NoTopologyRecord        = { ErrorOf( ES_DM2, 52, E_WARN, EV_ADMIN, 3 ), "No entries made in db.topology for server address: '%address%', dest address: '%destaddress%' and serverID: '%svrId%'." } ;
ErrorId MsgDm2::NoServerIDSet           = { ErrorOf( ES_DM2, 53, E_WARN, EV_ADMIN, 0 ), "ServerID for the server should be set and a server restart is required." } ;
ErrorId MsgDm2::TopologyRecDeleted      = { ErrorOf( ES_DM2, 55, E_INFO, EV_NONE, 4 ), "Deleted Topology Record: Addr-'%address%' DestAddr-'%destaddress%' SvrID-'%serverID%' Date-'%date%'" } ;
ErrorId MsgDm2::TopologyRecNotFound     = { ErrorOf( ES_DM2, 56, E_FAILED, EV_FAULT, 3 ), "Specified topology record[ with address '%addr%'][ target address '%taddr%'][ and server id '%svrID%'] not found" } ;
ErrorId MsgDm2::LockNameNull            = { ErrorOf( ES_DM2, 57, E_FAILED, EV_ADMIN, 1 ), "ServerLock name is null. Lock type is %locktype%." } ;
ErrorId MsgDm2::WorkRecNotFound         = { ErrorOf( ES_DM2, 58, E_FAILED, EV_ADMIN, 1 ), "The working record for %clientfile% not found." } ;
ErrorId MsgDm2::StreamDeletedInChange   = { ErrorOf( ES_DM2, 59, E_FAILED, EV_CONTEXT, 2 ), "Stream '%stream%' was deleted in change %change%" } ;
ErrorId MsgDm2::DomainObliterate        = { ErrorOf( ES_DM2, 60, E_INFO, EV_NONE, 2 ), "%domainType% %domainName% obliterated." } ;
ErrorId MsgDm2::StreamNotModifiedAtChange = { ErrorOf( ES_DM2, 61, E_FAILED, EV_CONTEXT, 2 ), "Stream '%stream%' was neither edited nor deleted in change %change%." } ;
ErrorId MsgDm2::PurgeStreamSpec          = { ErrorOf( ES_DM2, 62, E_INFO, EV_NONE, 7 ), "Would delete %count% stream revision(s)." } ;
ErrorId MsgDm2::CannotDeleteShelvedStream= { ErrorOf( ES_DM2, 63, E_FAILED, EV_CONTEXT, 2 ), "Cannot delete/obliterate shelved stream '%stream%' (shelf %shelf%)." } ;
ErrorId MsgDm2::RmtArchiveDeleteFailed      = { ErrorOf( ES_DM2, 64, E_FAILED, EV_FAULT, 3 ), "Failed to delete archive on the target server - lbrfile: %lbrFile% lbrRev: %lbrRev% lbrType: %lbrType%." } ;
ErrorId MsgDm2::RmtDeleteEdgeArchiveFailed  = { ErrorOf( ES_DM2, 65, E_FAILED, EV_FAULT, 0 ), "Failed to delete remote archive." } ;
ErrorId MsgDm2::ComponentStreamInvalid  = { ErrorOf( ES_DM2, 66, E_FAILED, EV_UNKNOWN, 1 ), "The component stream '%stream%' is not valid." } ;
ErrorId MsgDm2::ComponentTypeNotAvailable = { ErrorOf( ES_DM2, 67, E_FAILED, EV_UNKNOWN, 1 ), "The component type '%type%' is not available." } ;
ErrorId MsgDm2::TopologyDelPreview        = { ErrorOf( ES_DM2, 68, E_INFO, EV_NONE, 0 ), "This is in preview mode. For actual [%op%|deletion], use '-y'." } ;
ErrorId MsgDm2::StreamHasComponentsDelete = { ErrorOf( ES_DM2, 69, E_FAILED, EV_NOTYET, 2 ), "Stream '%stream%' is a defined as a component in consuming stream '%consumer%'; cannot delete until the component definition is removed." } ;
ErrorId MsgDm2::StreamHasComponentsOblit = { ErrorOf( ES_DM2, 70, E_FAILED, EV_NOTYET, 2 ), "Stream '%stream%' is a defined as a component in consuming stream '%consumer%'; cannot obliterate until the component definition is removed." } ;
ErrorId MsgDm2::ComponentInvalidIsStream = { ErrorOf( ES_DM2, 71, E_FAILED, EV_USAGE, 1 ), "Stream '%stream%' cannot be defined as a component of itself." } ;
ErrorId MsgDm2::ComponentInvalidIsConsumer = { ErrorOf( ES_DM2, 72, E_FAILED, EV_USAGE, 2 ), "Stream '%component%' is a consumer of this stream, %stream%, and cannot be defined as a component." } ;
ErrorId MsgDm2::ComponentInvalidIsRelative = { ErrorOf( ES_DM2, 73, E_FAILED, EV_USAGE, 2 ), "Stream '%component%' is a relative of this stream, %stream%, and cannot be defined as a component." } ;
ErrorId MsgDm2::ReparentFailedParentIsComponent = { ErrorOf( ES_DM2, 74, E_FAILED, EV_USAGE, 2 ), "Cannot change Parent to '%parent%' since it is a component of this stream, %stream%." } ;
ErrorId MsgDm2::ReparentFailedParentIsCompOfChild = { ErrorOf( ES_DM2, 75, E_FAILED, EV_USAGE, 3 ), "Cannot change Parent to '%parent%' since it is a component of a descendent, %descendent%, of this stream, %stream%." } ;
ErrorId MsgDm2::ReparentFailedFamilyIsComponent = { ErrorOf( ES_DM2, 76, E_FAILED, EV_USAGE, 3 ), "Cannot change Parent to '%parent%' since a stream family member, %member%, is a component of this stream, %stream%." } ;
ErrorId MsgDm2::ReparentFailedFamilyIsCompOfChild = { ErrorOf( ES_DM2, 77, E_FAILED, EV_USAGE, 4 ), "Cannot change Parent to '%parent%' since a member of the stream's family, %member%, is a component of a descendent, %descendent%, of this stream, %stream%." } ;
ErrorId MsgDm2::ReparentFailedParentHasComponent = { ErrorOf( ES_DM2, 78, E_FAILED, EV_USAGE, 2 ), "Cannot change Parent to '%parent%' since it defines this stream, %stream%, as a component." } ;
ErrorId MsgDm2::ReparentFailedFamilyHasComponent = { ErrorOf( ES_DM2, 79, E_FAILED, EV_USAGE, 3 ), "Cannot change Parent to '%parent%' since a stream family member, %member%, defines this stream, %stream%, as a component." } ;
ErrorId MsgDm2::StreamDeletedInChangeWarn  = { ErrorOf( ES_DM2, 80, E_WARN, EV_CONTEXT, 2 ), "Stream '%stream%' was deleted in change %change%" } ;
ErrorId MsgDm2::StreamLoopFound            = { ErrorOf( ES_DM2, 81, E_FAILED, EV_USAGE, 5 ), "Stream '%dependent%' is a %relation1% of %independent1% and a %relation2% of %independent2%." } ;
ErrorId MsgDm2::ComponentInvalidIsDependent= { ErrorOf( ES_DM2, 82, E_FAILED, EV_USAGE, 4 ), "Stream '%dependent%' cannot be defined as a component.  It is a %relation% of %independent%, which is a relative or consumer of this stream %stream%." } ;
ErrorId MsgDm2::TopologyThresholdOutOfRange       = { ErrorOf( ES_DM2, 83, E_FAILED, EV_USAGE, 0 ), "Threshold value provided has exceeded the maximum range (LLONG_MAX/86400)." } ;
ErrorId MsgDm2::ProtectsMismatch                  = { ErrorOf( ES_DM2, 84, E_FAILED, EV_ILLEGAL, 2 ), "Path '%spath%' doesn't match command argument '%apath%'." };
ErrorId MsgDm2::DirsDataStreamViews               = { ErrorOf( ES_DM2, 85, E_INFO, EV_NONE, 2 ), "%streamDirNam% (mapped to %dirName%)" } ;
ErrorId MsgDm2::FilesDataStreamViews              = { ErrorOf( ES_DM2, 86, E_INFO, EV_NONE, 6 ), "%streamFile%%depotRev% (mapped to %depotFile%) - %action% %change% (%type%)" } ;
ErrorId MsgDm2::FilesDataTrait                    = { ErrorOf( ES_DM2, 148, E_INFO, EV_NONE, 6 ), "%depotFile%%depotRev% - %action% %change% (%type%) %trait%" } ;
ErrorId MsgDm2::MaxMem                            = { ErrorOf( ES_DM2, 87, E_FAILED, EV_ADMIN, 1 ), "Too much memory used (over %maxMem%); see '%'p4 help maxmemory'%'." } ;
ErrorId MsgDm2::GroupsDataVerbose222              = { ErrorOf( ES_DM2, 88, E_INFO, EV_NONE, 8 ), "%group% %maxresults% %maxscanrows% %maxtimeout% %maxopenfiles% %maxmem% %timeout% %passtimeout%[ %idleTimeout%]" };
ErrorId MsgDm2::TopologyDelRecMarker              = { ErrorOf( ES_DM2, 89, E_INFO, EV_NONE, 0 ), "The server(s) marked as deleted" } ;
ErrorId MsgDm2::TopologyAmbiguity                 = { ErrorOf( ES_DM2, 90, E_WARN, EV_NONE, 1 ), "Ambiguity found for the topology record with server address '%address%'" } ;
ErrorId MsgDm2::TopologyTargetDeleted             = { ErrorOf( ES_DM2, 91, E_WARN, EV_NONE, 2 ), "The target server '%targetAddr%' is marked as deleted for the server '%address%'" } ;
ErrorId MsgDm2::TopologyRecAlreadyDeleted         = { ErrorOf( ES_DM2, 92, E_WARN, EV_NONE, 3 ), "The server with address '%addr%', destaddress '%daddr%' and serverid '%svrID%' is already marked as deleted." } ;
ErrorId MsgDm2::ComponentWritableNoChange         = { ErrorOf( ES_DM2, 93, E_FAILED, EV_USAGE, 0 ), "A writable component cannot be given a change specifier." } ;
ErrorId MsgDm2::NoChangeOnDistribution            = { ErrorOf( ES_DM2, 95, E_FAILED, EV_USAGE, 0 ), "Cannot create new changelist on distribution server." } ;
ErrorId MsgDm2::TopologyRecAlreadyMoved           = { ErrorOf( ES_DM2, 96, E_WARN, EV_NONE, 6 ), "The server with address '%addr%', destaddress '%daddr%' and serverid '%svrID%' is already moved to address '%taddr%', destaddress '%tdaddr%' and serverid '%tsvrID%'." } ;
ErrorId MsgDm2::TopologyMoveRecMarker             = { ErrorOf( ES_DM2, 97, E_INFO, EV_NONE, 0 ), "The server marked as moved." } ;
ErrorId MsgDm2::TopologyRecMoved                  = { ErrorOf( ES_DM2, 98, E_INFO, EV_NONE, 4 ), "Moved Topology Record: Addr '%address%' DestAddr '%destaddress%' SvrID '%serverID%' Date '%date%'" } ;
ErrorId MsgDm2::TopologyMoveSame                  = { ErrorOf( ES_DM2, 99, E_FAILED, EV_USAGE, 0 ), "Server cannot be set as moved to itself." } ;
ErrorId MsgDm2::TopologyFailedToMark              = { ErrorOf( ES_DM2, 100, E_FAILED, EV_NONE, 3 ), "Failed to mark the record : address '%addr%', destaddress '%daddr%', serverid '%svrID%' as 'movedFrom'." } ;
ErrorId MsgDm2::TopologyMarkedMoveTo              = { ErrorOf( ES_DM2, 101, E_INFO, EV_NONE, 3 ), "Marked the record : address '%addr%', destaddress '%daddr%', serverid '%svrID%' as 'movedTo'." } ;
ErrorId MsgDm2::NoAltSyncChangeWithHave           = { ErrorOf( ES_DM2, 112, E_FAILED, EV_USAGE, 0 ), "Cannot change client '%'altSync'%' option when have list is not empty." } ;
ErrorId MsgDm2::MaxMemOS                          = { ErrorOf( ES_DM2, 113, E_FAILED, EV_ADMIN, 0 ), "Not enough OS memory available (past 'sys.pressure.memory.high' threshold)." } ;
ErrorId MsgDm2::BadLazyPipeCount                  = { ErrorOf( ES_DM2, 115, E_FAILED, EV_USAGE, 2 ), "The search for lazy copies has not found the expected number of revisions. Expect: '%exp%'. Seen: '%seen%'." } ;
ErrorId MsgDm2::StreamViewGenAtChangeSkip         = { ErrorOf( ES_DM2, 117, E_WARN, EV_NONE, 2 ), "Skipping stream view generation for %stream% at change %change%." } ;
ErrorId MsgDm2::StreamFieldValueError             = { ErrorOf( ES_DM2, 120, E_FAILED, EV_FAULT, 3 ), "A stream of type '%streamType%' cannot have a %fieldName% of '%fieldValue%'." } ;
ErrorId MsgDm2::DepotTraitDup                     = { ErrorOf( ES_DM2, 121, E_FAILED, EV_CONTEXT, 1 ), "There is already a %'trait'% depot called '%depot%'." } ;
ErrorId MsgDm2::NoTraitDepot                      = { ErrorOf( ES_DM2, 129, E_FAILED, EV_CONTEXT, 0 ), "No trait depot was found on this server." } ;
ErrorId MsgDm2::NonServiceNotAllowed              = { ErrorOf( ES_DM2, 173, E_FAILED, EV_PROTECT, 2 ), "Non-service user %userName% may not perform operation %funcName%" } ;
ErrorId MsgDm2::ServiceRmtDistNotAllowed          = { ErrorOf( ES_DM2, 174, E_FAILED, EV_ADMIN, 0 ), "Remote depot access of distribution servers by service users is not allowed." } ;
ErrorId MsgDm2::NoTraitName                       = { ErrorOf( ES_DM2, 163, E_FAILED, EV_USAGE, 1 ), "Attribute '%trait%' does not exist: attributes were not cleared." } ;

ErrorId MsgDm2::ComponentStreamInvalidSparse     = { ErrorOf( ES_DM2, 107, E_FAILED, EV_NONE, 2 ), "The stream '%stream%' is a sparse stream of type '%type%' and cannot be defined as a component." } ;
ErrorId MsgDm2::NoReparentSparse                 = { ErrorOf( ES_DM2, 105, E_FAILED, EV_USAGE, 1 ), "Cannot change parent of stream '%stream%' because it is a sparse stream." } ;
ErrorId MsgDm2::NoSparseChildren                 = { ErrorOf( ES_DM2, 106, E_FAILED, EV_USAGE, 2 ), "Stream '%parent%' cannot parent '%stream%' because it is a sparse stream." } ;
ErrorId MsgDm2::NoStreamTypeChangeToNonSparse    = { ErrorOf( ES_DM2, 104, E_FAILED, EV_NOTYET, 2 ), "Failed to change sparse stream '%stream%' to non-sparse type '%streamType%'." } ;
ErrorId MsgDm2::NoStreamTypeChangeToSparse       = { ErrorOf( ES_DM2, 103, E_FAILED, EV_NOTYET, 2 ), "Failed to change non-sparse stream '%stream%' to sparse type '%streamType%'." } ;
ErrorId MsgDm2::NoVirtualParentSparseChild       = { ErrorOf( ES_DM2, 136, E_FAILED, EV_USAGE, 2 ), "Stream '%parent%' is virtual and stream '%stream%' sparse. Virtual streams may not parent sparse streams." } ;
ErrorId MsgDm2::SparseStreamCmdChangeSpecifierOnly = { ErrorOf( ES_DM2, 124, E_FAILED, EV_USAGE, 2 ), "Source revision for %cmd% into sparse stream %stream% must be a changelist." };
ErrorId MsgDm2::SparseStreamCmdMustIncludeAll    = { ErrorOf( ES_DM2, 123, E_FAILED, EV_USAGE, 3 ), "Must include all files for %cmd% into sparse stream %stream%[ (view is limited by %map%)]." };
ErrorId MsgDm2::SparseStreamCmdMustIncludeAll2   = { ErrorOf( ES_DM2, 128, E_FAILED, EV_USAGE, 2 ), "Must include files along with stream spec for %cmd% into sparse stream %stream%." };
ErrorId MsgDm2::SparseStreamCmdNoRevRange        = { ErrorOf( ES_DM2, 127, E_FAILED, EV_USAGE, 1 ), "Revision range not allowed for %cmd% into sparse stream." } ;
ErrorId MsgDm2::SparseStreamCmdWrongStream       = { ErrorOf( ES_DM2, 145, E_FAILED, EV_USAGE, 3 ), "Not able to use view of %otherStream% to %cmd% into sparse stream %yourStream%." };
ErrorId MsgDm2::SparseStreamCopyLatestOnly       = { ErrorOf( ES_DM2, 126, E_FAILED, EV_USAGE, 1 ), "May only copy from latest revision of sparse stream %sparseStream%." };
ErrorId MsgDm2::SparseStreamCmdOutOfDate          = { ErrorOf( ES_DM2, 125, E_FAILED, EV_USAGE, 2 ), "Sparse stream %sparseStream% must be fully up-to-date with parent before %command%." };
ErrorId MsgDm2::SparseStreamNoPin                = { ErrorOf( ES_DM2, 119, E_FAILED, EV_ILLEGAL, 1 ), "Stream of type '%streamType%' must have one or more 'share' paths with the same @change." };
ErrorId MsgDm2::SparseStreamNotSupported         = { ErrorOf( ES_DM2, 116, E_FAILED, EV_USAGE, 0 ), "This operation is not supported for sparse streams." };
ErrorId MsgDm2::SparseStreamOpNotAllowed         = { ErrorOf( ES_DM2, 122, E_FAILED, EV_USAGE, 2 ), "This operation is not allowed between sparse stream %sparseStream% and non-parent %otherStream%." };
ErrorId MsgDm2::SparseStreamPinChangeDown        = { ErrorOf( ES_DM2, 102, E_FAILED, EV_USAGE, 3 ), "The files in %depotPath% are associated with 'share' path %oldChange%.  Use 'p4 copy -F %newChange%' to overwrite." } ;
ErrorId MsgDm2::SparseStreamPinChangeUp          = { ErrorOf( ES_DM2, 118, E_FAILED, EV_USAGE, 3 ), "The files in %depotPath% are associated with 'share' path %oldChange%.  Use 'p4 merge %newChange%' to update." } ;
ErrorId MsgDm2::SparseStreamPinUpdate            = { ErrorOf( ES_DM2, 139, E_INFO, EV_NONE, 3 ), "%stream% - update from %parentStream%%fromChange%" } ;
ErrorId MsgDm2::VirtChildSparseParentOpNotAllowed= { ErrorOf( ES_DM2, 137, E_FAILED, EV_USAGE, 2 ), "This operation is not allowed for virtual child '%virtChild%' of sparse stream '%sparseStream%'." };
ErrorId MsgDm2::UnsubmitNoStreamSpec             = { ErrorOf( ES_DM2, 143, E_FAILED, EV_USAGE, 2 ), "Cannot unsubmit change %change% because it affected the stream specification %streamName%." };

ErrorId MsgDm2::OnlyOneClientReload              = { ErrorOf( ES_DM2, 130, E_FAILED, EV_ADMIN, 0 ), "Cannot reload multiple clients from the remote server." } ;
ErrorId MsgDm2::UnknownParam                     = { ErrorOf( ES_DM2, 131, E_FAILED, EV_USAGE, 1 ), "Unknown parameter '%param%'!" } ;
ErrorId MsgDm2::BadS3Mode                        = { ErrorOf( ES_DM2, 132, E_FAILED, EV_USAGE, 1 ), "Invalid S3 mode[ '%mode%']!" } ;
ErrorId MsgDm2::BadS3AuthKeys                    = { ErrorOf( ES_DM2, 158, E_FAILED, EV_USAGE, 0 ), "Both accessKey and secretKey must be set!" } ;
ErrorId MsgDm2::BadS3AuthToken                   = { ErrorOf( ES_DM2, 159, E_FAILED, EV_USAGE, 0 ), "A token can only be provided when accessKey and secretKey are also set!" } ;
ErrorId MsgDm2::BadS3AuthMultiple                = { ErrorOf( ES_DM2, 160, E_FAILED, EV_USAGE, 0 ), "Only 1 authentication source may be provided (accessKey and secretKey, profile or role)!" } ;
ErrorId MsgDm2::DepotBadAddress                  = { ErrorOf( ES_DM2, 133, E_FAILED, EV_USAGE, 2 ), "Depots of type '%depotType%' require 'Address' to [either be unset or set to a valid external storage scheme (currently %schemes%)|be unset]." } ;
ErrorId MsgDm2::DepotBadRemoteAddress            = { ErrorOf( ES_DM2, 134, E_FAILED, EV_USAGE, 0 ), "Remote depots require 'Address' to be a valid P4PORT" } ;
ErrorId MsgDm2::NoConfigHistory                  = { ErrorOf( ES_DM2, 135, E_FAILED, EV_USAGE, 2 ), "Config history [iteration %iteration% ]for '%name%' not found" } ;
ErrorId MsgDm2::SyncToRemove                     = { ErrorOf( ES_DM2, 138, E_INFO, EV_USAGE, 1 ), "Sync %clientFile% to remove it from your workspace." } ;
ErrorId MsgDm2::DomainNoViewCmtClientBadSave= { ErrorOf( ES_DM2, 142, E_FAILED, EV_UPGRADE, 2 ), "The %domainType% specification for %domainName% contains comments.\nThis client program is too old to update this spec.\nThe client program must be fully compatible with the 2024.1 server release in order to update this specification with comments." } ;
ErrorId MsgDm2::ConfigNotFound                   = { ErrorOf( ES_DM2, 144, E_FAILED, EV_FAULT, 2 ), "Failed to read the configurable variable '%cfgCName%' with server name '%cfgSName%' from the db.config table." } ;
ErrorId MsgDm2::ChunkMapFormat                   = { ErrorOf( ES_DM2, 146, E_FATAL, EV_ADMIN, 3 ), "Chunk map format error: '%source%', '%file%', '%error%'" }; // NOTRANS
ErrorId MsgDm2::SparseStreamNoCreateMaxCommit0   = { ErrorOf( ES_DM2, 147, E_FAILED, EV_USAGE, 0 ), "Can't create sparse stream if maxCommitChange is 0." }; 
ErrorId MsgDm2::SparseStreamNoUnshelvePinLower = { ErrorOf( ES_DM2, 149, E_FAILED, EV_USAGE, 3 ), "The stream spec in shelf %shelf% is associated with 'share' path %oldChange%. The current stream spec is associated with 'share' path %newChange%. Unshelving to a lower sparse stream parent changeview is not allowed." } ;
ErrorId MsgDm2::UpgradeToCreatePJnlClient        = { ErrorOf( ES_DM2, 150, E_FAILED, EV_USAGE, 0 ), "Cannot set client type to '%'partitioned-jnl'%' until all upstream servers have been upgraded to at least 2024.1 server version." };
ErrorId MsgDm2::BadHotFilePattern                = { ErrorOf( ES_DM2, 151, E_FAILED, EV_USAGE, 1 ), "Invalid hotfile pattern '%type%'; see '%'p4 help hotfiles'%'." };
ErrorId MsgDm2::NoTopologyActiveRec              = { ErrorOf( ES_DM2, 152, E_WARN, EV_USAGE, 2 ), "No active topology record found with the source address '%saddr%' and the target address '%taddr%'." };
ErrorId MsgDm2::DbBodTextCxEntryMissing          = { ErrorOf( ES_DM2, 153, E_FAILED, EV_FAULT, 2 ), "No entry for stream '%stream%' at change '%change%' found in db.bodtexcx. Stream spec integration analysis cannot continue." };
ErrorId MsgDm2::StreamSpecIntegrationNoFrmChange = { ErrorOf( ES_DM2, 154, E_FAILED, EV_FAULT, 2 ), "Can't find stream spec rev for stream '%fromstream%' at change '%change%'." };

ErrorId MsgDm2::ExVIEWC                          = { ErrorOf( ES_DM2, 155, E_WARN, EV_EMPTY, 1 ), "[%argc% - file(s)|File(s)] not in command limit view." } ;
ErrorId MsgDm2::ExLIMITVIEW                      = { ErrorOf( ES_DM2, 162, E_WARN, EV_EMPTY, 0 ), "No paths mapped after applying client limit view." } ;
ErrorId MsgDm2::LimitViewMixed                   = { ErrorOf( ES_DM2, 170, E_FAILED, EV_EMPTY, 0 ), "LimitView must be either depot syntax or client syntax, not a mix of both." } ;

ErrorId MsgDm2::SaltFail                          = { ErrorOf( ES_DM2, 156, E_FAILED, EV_EMPTY, 0 ), "Failed to generate random salt." } ;
ErrorId MsgDm2::HashFail                          = { ErrorOf( ES_DM2, 157, E_FAILED, EV_FAULT, 0 ), "Failed to generate password hash." } ;
ErrorId MsgDm2::NoMatchingMoves                   = { ErrorOf( ES_DM2, 161, E_INFO, EV_EMPTY, 0 ), "No matching moves found." } ;
ErrorId MsgDm2::Translate2Utf8                    = { ErrorOf( ES_DM2, 164, E_FAILED, EV_FAULT, 2 ), "Unable to convert to UTF8: '%input%' from '%message%'." } ;
ErrorId MsgDm2::TopologyNoRoot                    = { ErrorOf( ES_DM2, 165, E_INFO, EV_EMPTY, 0 ), "No root node in the topology records." } ;
ErrorId MsgDm2::TopologyCycle                     = { ErrorOf( ES_DM2, 166, E_INFO, EV_EMPTY, 1 ), "Cycle in topology graph for %serverid%." } ;
ErrorId MsgDm2::TopologyBadTerm                   = { ErrorOf( ES_DM2, 167, E_INFO, EV_EMPTY, 2 ), "Topology record: %serverid% not terminated by the root node: %root%." } ;
ErrorId MsgDm2::TopologyNoTerm                    = { ErrorOf( ES_DM2, 168, E_INFO, EV_EMPTY, 1 ), "Unable to build terminated topology graph for %serverid%." } ;

ErrorId MsgDm2::UserNotExistInfo                  = { ErrorOf( ES_DM2, 169, E_INFO, EV_UNKNOWN, 2 ), "User %user% doesn't exist (Extension owner:%owner%)." } ;
ErrorId MsgDm2::SparseStreamResolveMoveErr        = { ErrorOf( ES_DM2, 171, E_FAILED, EV_USAGE, 4 ), "%clientFile% is in a pending resolved state, but %sourceFile% has been moved. Integration cannot occur until %clientFile% has been submitted or reverted. Check changelist %changeList% for similar resolved files with moved sources." };
ErrorId MsgDm2::No2faWithIdp                      = { ErrorOf( ES_DM2, 172, E_FAILED, EV_CONFIG, 2 ), "2FA cannot be used with IdP authentication." } ;
ErrorId MsgDm2::MultiplePartitionedRevSpec        = { ErrorOf( ES_DM2, 175, E_FAILED, EV_USAGE, 1 ), "Client %clientName% cannot be used as a revision specifier together with another partitioned client." } ;


// ErrorId graveyard: retired/deprecated ErrorIds. 

ErrorId MsgDm2::NoPartitionedToReadonly    = { ErrorOf( ES_DM2, 54, E_FAILED, EV_USAGE, 1 ), "Cannot change client type from '[%from%|partitioned]' to 'readonly'." } ; //NOTRANS

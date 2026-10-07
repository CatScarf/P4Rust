/*
 * Copyright 1995, 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * msgserver2.cc - definitions of errors for server subsystem.
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
 * Current high value for a MsgServer2 error code is: 369
 *                                                   Max code is 1023!!!
 *
 * The MsgServer2 class contains overflow messages from MsgServer.
 *
 */
# include <stdhdrs.h>
# include <error.h>
# include <errornum.h>
# include "msgserver2.h"

ErrorId MsgServer2::ExtensionDeletePreview = { ErrorOf( ES_SERVER2, 1, E_INFO, EV_NONE, 1 ), "Would delete Extension '%delargs%'." } ;
ErrorId MsgServer2::ExtensionInstallPreview = { ErrorOf( ES_SERVER2, 27, E_INFO, EV_NONE, 8 ), "Would install Extension '%extfullname%'\n\nVersion: '%Version%'\nUUID: %uuid%\nDeveloper: %developer%\nDescription: %description%\nLicense: %license%\nHomepage URL: %homepageurl%\nCompatible products: %CompatProds%\n" } ;
ErrorId MsgServer2::WarnPreviewMode        = { ErrorOf( ES_SERVER2, 2, E_INFO, EV_NONE, 0 ), "This was report mode. Use -y to perform the operation." } ;
ErrorId MsgServer2::UseReopen2             = { ErrorOf( ES_SERVER2, 3, E_FAILED, EV_USAGE, 0 ), "Usage: %'reopen -c changelist# -So'%" } ;
ErrorId MsgServer2::UseReopen3             = { ErrorOf( ES_SERVER2, 34, E_FAILED, EV_USAGE, 0 ), "Usage: %'reopen -c changelist# [ -So | -Si files... ]'%" } ;
ErrorId MsgServer2::UseResolve2            = { ErrorOf( ES_SERVER2, 7, E_FAILED, EV_USAGE, 0 ), "Usage: %'resolve [ -af -am -as -at -ay -n -o ] -So'%" } ;
ErrorId MsgServer2::UseOpened3             = { ErrorOf( ES_SERVER2, 8, E_FAILED, EV_USAGE, 0 ), "Usage: %'opened [ -c changelist# ] -So'%" } ;
ErrorId MsgServer2::StorageUpgradeInProgress = { ErrorOf( ES_SERVER2, 9, E_FAILED, EV_NONE, 0 ), "A storage upgrade is already in progress" } ;
ErrorId MsgServer2::StorageEdgeFailure     = { ErrorOf( ES_SERVER2, 10, E_FAILED, EV_FAULT, 2 ), "Upgrade storage for Edge server failed, file %file% change %change%." } ;
ErrorId MsgServer2::UseStreamlog           = { ErrorOf( ES_SERVER2, 11, E_FAILED, EV_USAGE, 0 ), "Usage: %'streamlog [ -c changelist# -h -i -l -L -t -m maxRevs ] stream...'%" } ;
ErrorId MsgServer2::SubmitNoBgXferTarget   = { ErrorOf( ES_SERVER2, 12, E_FAILED, EV_NOTYET, 0 ), "Background archive transfer requires 'ExternalAddress' field in edge server spec to be set --  cannot submit."} ;
ErrorId MsgServer2::SubmitBgXferNoConfig   = { ErrorOf( ES_SERVER2, 13, E_FAILED, EV_NOTYET, 0 ), "Background archive transfer requires submit.allowbgtransfer configurable to be set --  cannot submit."} ;
ErrorId MsgServer2::SubmitBgNotEdge        = { ErrorOf( ES_SERVER2, 129, E_INFO, EV_NOTYET, 0 ), "Background archive transfer is supported only from edge servers.\nProceeding with default submit archive transfer."} ;
ErrorId MsgServer2::SubmitBgNotConfigured  = { ErrorOf( ES_SERVER2, 130, E_INFO, EV_NOTYET, 0 ), "Background archive transfer requires submit.allowbgtransfer configurable to be set.\nProceeding with default submit archive transfer."} ;
ErrorId MsgServer2::UsePullt               = { ErrorOf( ES_SERVER2, 14, E_FAILED, EV_USAGE, 0 ), "Usage: %'pull -u -t target [ -b <N> -i <N> ]'%" } ;
ErrorId MsgServer2::SubmitNoBackgroundThreads = { ErrorOf( ES_SERVER2, 15, E_FAILED, EV_NOTYET, 1 ), "Unable to launch background threads for change %change% --  cannot submit."} ;
ErrorId MsgServer2::StorageNoUpgrade       = { ErrorOf( ES_SERVER2, 16, E_FAILED, EV_NONE, 0 ), "There is no storage upgrade in progress to restart." } ;
ErrorId MsgServer2::FailoverForced         = { ErrorOf( ES_SERVER2, 17, E_WARN, EV_ADMIN, 2 ), "Attempting unsupported forced %failover%; attempting to continue through any errors encountered. This server might not be as expected after the forced %failover%." } ;
ErrorId MsgServer2::FailoverWriteServerID  = { ErrorOf( ES_SERVER2, 49, E_INFO, EV_NONE, 2 ), "Failover from %serverID% %address%" } ;
ErrorId MsgServer2::FailoverDetails        = { ErrorOf( ES_SERVER2, 131, E_INFO, EV_NONE, 6 ), "%action% from source: %serverID% (%serverType% %sourcePort%) to target: %targetID% (%targetPort%)." } ;
ErrorId MsgServer2::StorageRestoreDigest   = { ErrorOf( ES_SERVER2, 18, E_INFO, EV_NONE, 2 ), "Warning: A file with a different digest value already exists for %file% %rev% - skipping archive." } ;
ErrorId MsgServer2::xuUpstream             = { ErrorOf( ES_SERVER2, 25, E_INFO, EV_ADMIN, 0 ), "Upgrades will be applied from '%'-xu'%' at upstream server." } ;
ErrorId MsgServer2::xuAtStart              = { ErrorOf( ES_SERVER2, 63, E_INFO, EV_ADMIN, 0 ), "Upgrades will be applied at server startup." } ;
ErrorId MsgServer2::xuUpstream2            = { ErrorOf( ES_SERVER2, 64, E_INFO, EV_ADMIN, 0 ), "Additional upgrades will be applied from '%'-xu'%' at upstream server." } ;
ErrorId MsgServer2::xuAtStart2             = { ErrorOf( ES_SERVER2, 65, E_INFO, EV_ADMIN, 0 ), "Additional upgrades will be applied at server startup." } ;
ErrorId MsgServer2::JournalRequired        = { ErrorOf( ES_SERVER2, 19, E_FAILED, EV_ADMIN, 0 ), "Cannot perform upgrades: journal is disabled! Upgrades produce data that must be written to the journal to be replicated!" } ;
ErrorId MsgServer2::ShelvedStreamDeleted   = { ErrorOf( ES_SERVER2, 20, E_INFO, EV_NONE, 2 ), "Stream %stream% removed from shelf %change%." } ;
ErrorId MsgServer2::NoShelvedStreamDelete  = { ErrorOf( ES_SERVER2, 21, E_FAILED, EV_EMPTY, 0 ), "No shelved stream in changelist to remove." } ;
ErrorId MsgServer2::DescribeShelvedStream  = { ErrorOf( ES_SERVER2, 22, E_INFO, EV_NONE, 0 ), "Shelved stream specification: %stream%\n" } ;
ErrorId MsgServer2::ShelveDeleteJustFiles  = { ErrorOf( ES_SERVER2, 23, E_INFO, EV_NONE, 2 ), "Shelved change %change% partially deleted, still contains stream %stream%." } ;
ErrorId MsgServer2::ShelveCompleteStream   = { ErrorOf( ES_SERVER2, 35, E_INFO, EV_NONE, 1 ), "%change% stream shelved." } ;
ErrorId MsgServer2::ShelveCompleteBoth     = { ErrorOf( ES_SERVER2, 36, E_INFO, EV_NONE, 1 ), "%change% files and stream shelved." } ;
ErrorId MsgServer2::StorageWaitComplete    = { ErrorOf( ES_SERVER2, 24, E_INFO, EV_NONE, 0 ), "The storage upgrade process is complete." } ;
ErrorId MsgServer2::ExtensionNameCfgUniq   = { ErrorOf( ES_SERVER2, 26, E_FAILED, EV_USAGE, 0 ), "The Extension '--name' and '--configure' values may not be the same." } ;
ErrorId MsgServer2::UpgradeWarning         = { ErrorOf( ES_SERVER2, 28, E_INFO, EV_ADMIN, 2 ), "The storage upgrade phase %num% has detected a problem. View the file \"%errorfile%\" in the server root for details." } ;
ErrorId MsgServer2::BadUpLbr               = { ErrorOf( ES_SERVER2, 29, E_FATAL, EV_ADMIN, 3 ), "Revision %file%#%rev% from table %tbl% has at least one invalid lbr field." } ;
ErrorId MsgServer2::MissingLbr             = { ErrorOf( ES_SERVER2, 30, E_FATAL, EV_ADMIN, 2 ), "Unable to find librarian file for revision %file%#%rev%." } ;
ErrorId MsgServer2::SkippedKeyed           = { ErrorOf( ES_SERVER2, 96, E_INFO, EV_ADMIN, 2 ), "Skipped generating digest for %file%#%rev%." } ;
ErrorId MsgServer2::NoStreamFieldsResolve  = { ErrorOf( ES_SERVER2, 31, E_WARN, EV_EMPTY, 0 ), "No stream fields to resolve." } ;
ErrorId MsgServer2::UseDiffA               = { ErrorOf( ES_SERVER2, 32, E_FAILED, EV_USAGE, 0 ), "Usage: %'diff [ -d<flags> ] -As [ streamname[ @change ] ]'%" } ;
ErrorId MsgServer2::UseDiff2A              = { ErrorOf( ES_SERVER2, 33, E_FAILED, EV_USAGE, 0 ), "Usage: %'diff2 [ -d<flags> ] -As streamname1[ @change1 ] streamname2[ @change2 ]'%" } ;
ErrorId MsgServer2::NoStreamDefaultShelve  = { ErrorOf( ES_SERVER2, 37, E_FAILED, EV_EMPTY, 0 ), "No stream spec to shelve from the default changelist." } ;
ErrorId MsgServer2::NoStreamShelve         = { ErrorOf( ES_SERVER2, 38, E_FAILED, EV_EMPTY, 0 ), "No stream spec to shelve." } ;
ErrorId MsgServer2::ShelveStreamBegin      = { ErrorOf( ES_SERVER2, 39, E_INFO, EV_NONE, 1 ), "Shelving stream spec for change %change%." } ;
ErrorId MsgServer2::StreamShelfOccupied    = { ErrorOf( ES_SERVER2, 40, E_FAILED, EV_NOTYET, 3 ), "Change %change% already contains the stream spec for %shelvedstream%.\n%curstream% cannot be shelved with this change.\nRemove %shelvedstream% from shelf and try again." } ;
ErrorId MsgServer2::ServiceNotSupported    = { ErrorOf( ES_SERVER2, 41, E_WARN, EV_EMPTY, 2 ), "The server is configured with Service %service% which is not supported, change it with 'p4 server %serverid%' command." } ;
ErrorId MsgServer2::NoRplMissingMandatory  = { ErrorOf( ES_SERVER2, 42, E_FAILED, EV_ADMIN, 0 ), "Replication to this server is stalled because one or more mandatory standby servers of this server's upstream server are not alive." } ;
ErrorId MsgServer2::UnexpectedRotJnlChange = { ErrorOf( ES_SERVER2, 62, E_FATAL, EV_FAULT, 4 ), "Unexpected change of rotated journal '%jfile%' seen while scanning for journal trailer. rd->Tell(): %rdTell%; currentTell: %currentTell%; currentSize: %currentSize%" } ;
ErrorId MsgServer2::RunExtErrorWarning     = { ErrorOf( ES_SERVER2, 43, E_WARN, EV_NONE, 1 ), "%exterrorwarn%." } ;
ErrorId MsgServer2::RunExtErrorFailed      = { ErrorOf( ES_SERVER2, 44, E_FAILED, EV_NONE, 1 ), "%exterrorfailed%." } ;
ErrorId MsgServer2::RunExtErrorFatal       = { ErrorOf( ES_SERVER2, 45, E_FATAL, EV_NONE, 1 ), "%exterrorfatal%." } ;
ErrorId MsgServer2::StorageCleanupWarn     = { ErrorOf( ES_SERVER2, 46, E_FATAL, EV_NONE, 0 ), "The clean up via a storage record has failed." } ;
ErrorId MsgServer2::VerifyDataProblem      = { ErrorOf( ES_SERVER2, 47, E_FAILED, EV_NONE, 6 ), "%depotFile% %depotRev% - %rcount% (%type%) %digest% %status%" } ; // NOTRANS
ErrorId MsgServer2::VerifyData             = { ErrorOf( ES_SERVER2, 48, E_INFO, EV_NONE, 6 ), "%depotFile% %depotRev% - %rcount% (%type%) %digest%[ %status%]" } ; // NOTRANS
ErrorId MsgServer2::FailoverServerIDBad    = { ErrorOf( ES_SERVER2, 50, E_FAILED, EV_ADMIN, 0 ), "Failover has occurred from this server, and the server ID must be changed. Use '%'p4d -xD'%' to change the server ID." } ;
ErrorId MsgServer2::FailoverMasterTooOld   = { ErrorOf( ES_SERVER2, 83, E_FAILED, EV_USAGE, 0 ), "Server from which failover is to occur must be at version 2018.2 or above to participate in failover." } ;
ErrorId MsgServer2::FailoverFBackOldWarn   = { ErrorOf( ES_SERVER2, 175, E_WARN, EV_ADMIN, 0 ), "The server from which failover is to occur must be at version 2022.1 or above in order to run failback after this failover.\nThat server must be upgraded, and '%'failbackconfigs'%' must be copied to its %'$P4ROOT'% as '%'failoverconfigs'%' prior to running '%'p4d -Fm'%' to prepare it for failback." } ;
ErrorId MsgServer2::FailoverFConfigsBad    = { ErrorOf( ES_SERVER2, 178, E_FAILED, EV_ADMIN, 0 ), "The '%'failoverconfigs'%' file in the %'$P4ROOT'% directory has been corrupted." } ;
ErrorId MsgServer2::FailoverCfgCommit      = { ErrorOf( ES_SERVER2, 115, E_INFO, EV_NONE, 1 ), "Propagating configuration of the %failed% server ..." } ;
ErrorId MsgServer2::FailoverUnCfgCommit    = { ErrorOf( ES_SERVER2, 116, E_INFO, EV_NONE, 1 ), "Undoing propagation of configuration for the %failed% server ..." } ;
ErrorId MsgServer2::FailoverWriteFConfigs  = { ErrorOf( ES_SERVER2, 176, E_INFO, EV_NONE, 0 ), "Writing %'failoverconfigs'% file ..." } ;
ErrorId MsgServer2::FailoverDeleteFConfigs = { ErrorOf( ES_SERVER2, 177, E_INFO, EV_NONE, 0 ), "Removing %'failoverconfigs'% file ..." } ;
ErrorId MsgServer2::FailoverNeedYOK        = { ErrorOf( ES_SERVER2, 156, E_INFO, EV_NONE, 1 ), "No errors reported; use %'--yes or -y'% to execute the %failover%." } ;
ErrorId MsgServer2::FailbackStandbyRestrict= { ErrorOf( ES_SERVER2, 185, E_INFO, EV_NONE, 0 ), "Standby in restricted mode." } ;
ErrorId MsgServer2::FailbackWriteServerID  = { ErrorOf( ES_SERVER2, 195, E_INFO, EV_NONE, 2 ), "Failback from %serverID% %address%" } ;
ErrorId MsgServer2::UseFailback            = { ErrorOf( ES_SERVER2, 196, E_FAILED, EV_USAGE, 0 ), "Usage: %'failback [ -y ] [ -w <quiesce wait> ] [ -v <verification time> ] [ <failback message> ]'%" } ;
ErrorId MsgServer2::FailbackMasterTooOld   = { ErrorOf( ES_SERVER2, 197, E_FAILED, EV_USAGE, 0 ), "Server from which failback is to occur must be at version 22.1 or above to participate in failback." } ;
ErrorId MsgServer2::FailbackFConfigsMissing= { ErrorOf( ES_SERVER2, 198, E_FAILED, EV_ADMIN, 1 ), "The required '%failoverconfigs%' file generated by failover is missing from the $P4ROOT directory." } ;
ErrorId MsgServer2::UseFailoverB           = { ErrorOf( ES_SERVER2, 199, E_FAILED, EV_USAGE, 0 ), "Usage: %'failover -B [ -y ] [ -w <quiesce wait> ] [ -v <verification time> ] [ <failback message> ]'%" } ;
ErrorId MsgServer2::FailbackStandbyNotRestricted = { ErrorOf( ES_SERVER2, 200, E_FAILED, EV_ADMIN, 0 ), "Failback must be run from a restricted standby server. Use the 'p4d -Fm' command to create a restricted standby that will be ready for failback." } ;
ErrorId MsgServer2::FailbackStandbyBad     = { ErrorOf( ES_SERVER2, 245, E_FAILED, EV_ADMIN, 0 ), "Failover or failback has occurred from this server. It cannot not be determined which occurred since both 'failoverconfigs' and 'failbackconfigs' were found. Use '%'p4d -Fm'%' to convert to a restricted standby if this is a failed over server, '%'p4d -Fs'%' command to convert to a standby if this is a failed back server, or '%'p4d -xD'%' to change the server ID and bypass the failback process." } ;
ErrorId MsgServer2::FailoverRunFailback    = { ErrorOf( ES_SERVER2, 246, E_FAILED, EV_ADMIN, 0 ), "Failback, not failover, should be run from this server. Run '%'p4 failback'%' instead." } ;
ErrorId MsgServer2::FailbackNeedsFm        = { ErrorOf( ES_SERVER2, 243, E_FAILED, EV_ADMIN, 0 ), "Failover has occurred from this server. Use the '%'p4d -Fm'%' command to convert this server into a restricted standby that will be ready for failback." } ;
ErrorId MsgServer2::FailbackNeedsFs        = { ErrorOf( ES_SERVER2, 244, E_FAILED, EV_ADMIN, 0 ), "Failback has occurred from this server. Use the '%'p4d -Fs'%' command to convert this server into a standby for the failed back server." } ;
ErrorId MsgServer2::ServerIDReused         = { ErrorOf( ES_SERVER2, 51, E_FAILED, EV_ADMIN, 1 ), "Cannot reuse server ID '%serverID%' after failover." } ;
ErrorId MsgServer2::ExtensionPostInstallMsg ={ ErrorOf( ES_SERVER2, 52, E_INFO, EV_NONE, 4 ), "Perform the following steps to turn on the Extension:\n\n# Create a global configuration if one doesn't already exist.\np4 extension --configure %extension%\n\n# Create an instance configuration to enable the Extension.\np4 extension --configure %extension% --name %extension%-instanceName\n\nFor more information, visit:\nhttps://www.perforce.com/manuals/v%serverversion%/extensions/Content/Extensions/Home-extensions.html\n" } ;
ErrorId MsgServer2::StreamShelfReadOnly    = { ErrorOf( ES_SERVER2, 53, E_FAILED, EV_NOTYET, 4 ), "Stream spec %stream% is shelved in change %change% and is not open.\nThe stream field in this change spec is read only and cannot be\nchanged until the stream is removed from the shelf.\nSwitch your client to %stream%, then use 'p4 shelve -d -As %change%'\nto remove the stream from shelf." } ;
ErrorId MsgServer2::UseStreamSpec          = { ErrorOf( ES_SERVER2, 54, E_FAILED, EV_USAGE, 0 ), "Usage: %'streamspec [ -i -o ]'%" } ;
ErrorId MsgServer2::UseLbrScan             = { ErrorOf( ES_SERVER2, 55, E_FAILED, EV_USAGE, 0 ), "Usage: %'p4 storage -l { start [--nonlbr [-t target]] | pause | restart | cancel | status [--nonlbr] | pri } //depotpath/...'%" } ;
ErrorId MsgServer2::LbrScanBusy            = { ErrorOf( ES_SERVER2, 56, E_FAILED, EV_NONE, 0 ), "lbrscan busy." } ;
ErrorId MsgServer2::LbrScanBadDepot        = { ErrorOf( ES_SERVER2, 57, E_FAILED, EV_NONE, 0 ), "The depot path argument must start with \"//\" and end with \"/...\"" } ;
ErrorId MsgServer2::LbrScanPathInUse       = { ErrorOf( ES_SERVER2, 58, E_FAILED, EV_NONE, 1 ), "A scan is already active on that librarian path (%lpath%)." } ;
ErrorId MsgServer2::LbrScanUnderPath       = { ErrorOf( ES_SERVER2, 59, E_FAILED, EV_NONE, 2 ), "A lbrscan request overlaps an existing lbrscan request ('%short%' '%long%')." } ;
ErrorId MsgServer2::LbrScanNotFound        = { ErrorOf( ES_SERVER2, 60, E_FAILED, EV_NONE, 1 ), "Lbrscan request '%request%' not found." } ;
ErrorId MsgServer2::LbrScanBadPath         = { ErrorOf( ES_SERVER2, 61, E_FAILED, EV_NONE, 1 ), "Lbrscan request '%request%' is not a local depot." } ;
ErrorId MsgServer2::StorageZeroRefClean    = { ErrorOf( ES_SERVER2, 66, E_INFO, EV_NONE, 2 ), "Storage record with a reference count of zero %file% %rev%." } ;
ErrorId MsgServer2::StorageZeroCount       = { ErrorOf( ES_SERVER2, 67, E_INFO, EV_NONE, 2 ), "Deleted storage revisions: %count%. Deleted librarian revisions: %lcount%." } ;
ErrorId MsgServer2::StorageDupZero         = { ErrorOf( ES_SERVER2, 68, E_FATAL, EV_NONE, 2 ), "Found multiple zero reference file during deletion for %lbrfile% %lbrrev%." } ;
ErrorId MsgServer2::StorageShareRep        = { ErrorOf( ES_SERVER2, 69, E_FAILED, EV_NONE, 0 ), "Running the orphan scanner on a shared archive replica is not permitted." } ;
ErrorId MsgServer2::StorageSingle          = { ErrorOf( ES_SERVER2, 70, E_FAILED, EV_NONE, 0 ), "Running the orphan scanner is not allowed in single-threaded mode." } ;
ErrorId MsgServer2::StorageSymlink         = { ErrorOf( ES_SERVER2, 71, E_FAILED, EV_NONE, 1 ), "The orphan scanner has detected a symlink (%path%). Using symlinks with the orphan scanner may cause data loss. After review, symlinks can be enabled by setting lbr.storage.allowsymlink to 1." } ;
ErrorId MsgServer2::ExtMissingCfg          = { ErrorOf( ES_SERVER2, 72, E_FAILED, EV_USAGE, 1 ), "Extension config '%cfg%' not found." } ;
ErrorId MsgServer2::ExtMissingCfgEvent     = { ErrorOf( ES_SERVER2, 73, E_FAILED, EV_USAGE, 2 ), "Extension config '%cfg%' does not have event '%event%'." } ;
ErrorId MsgServer2::ExtensionRunFunction   = { ErrorOf( ES_SERVER2, 85, E_FAILED, EV_NONE, 1 ), "This function '%function%' can only be called from extension --run." } ;
ErrorId MsgServer2::StringTooLarge         = { ErrorOf( ES_SERVER2, 86, E_FAILED, EV_NONE, 1 ), "The string is too large, must be smaller than %maxsize%." } ;
ErrorId MsgServer2::ExtensionNonUTF8Data   = { ErrorOf( ES_SERVER2, 87, E_FAILED, EV_NONE, 1 ), "Data received by %function% contains non-UTF8 character." } ;
ErrorId MsgServer2::MissingMovedFilesHeader= { ErrorOf( ES_SERVER2, 75, E_FATAL, EV_NONE, 0 ), "Files are missing as a result of one or more move operations.  The files are:" } ;
ErrorId MsgServer2::MissingMovedFile       = { ErrorOf( ES_SERVER2, 76, E_FATAL, EV_NONE, 2 ), "  %missingFile%%missingFileRev%" } ;
ErrorId MsgServer2::UpdatedLbrType         = { ErrorOf( ES_SERVER2, 77, E_INFO, EV_NONE, 0 ), "The lbrType is updated successfully" } ;
ErrorId MsgServer2::InvalidExtName         = { ErrorOf( ES_SERVER2, 78, E_FAILED, EV_NONE, 1 ), "Invalid character in Extension name '%extname%'." } ;
ErrorId MsgServer2::DigestFail             = { ErrorOf( ES_SERVER2, 81, E_FATAL, EV_ADMIN, 2 ), "Failed to create digest for revision %file%#%rev%." } ;
ErrorId MsgServer2::NoFilesInSvrRtForVal   = { ErrorOf( ES_SERVER2, 82, E_FAILED, EV_NONE, 1 ), "No Database files found in the default/specified server root '%root%'." } ;
ErrorId MsgServer2::EndOfStorePhase1       = { ErrorOf( ES_SERVER2, 84, E_INFO, EV_NONE, 0 ), "Phase 1 of the storage upgrade has finished." };
ErrorId MsgServer2::UseHeartbeat           = { ErrorOf( ES_SERVER2, 88, E_FAILED, EV_USAGE, 0 ), "Usage: %'heartbeat [ -t<target> ] [ -i<interval> ] [ -w<wait> ] [ -m<missingInterval> ] [ -r<missingWait> ] [ -c<missingCount> ]'%" } ;
ErrorId MsgServer2::HeartbeatNoTarget      = { ErrorOf( ES_SERVER2, 89, E_FAILED, EV_USAGE, 0 ), "Target must be specified by either '-t<target>' or P4TARGET must be set." } ;
ErrorId MsgServer2::HeartbeatExiting       = { ErrorOf( ES_SERVER2, 90, E_FAILED, EV_ADMIN, 1 ), "The heartbeat command monitoring %target% is exiting because it detected that the target is not responding." } ;
ErrorId MsgServer2::UseHeartbeatWait       = { ErrorOf( ES_SERVER2, 91, E_FAILED, EV_ADMIN, 1 ), "A heartbeat wait time cannot be less than %minWait% ms." } ;
ErrorId MsgServer2::UseHeartbeatInterval   = { ErrorOf( ES_SERVER2, 92, E_FAILED, EV_ADMIN, 1 ), "A heartbeat interval cannot be less than %minInterval% ms." } ;
ErrorId MsgServer2::UseHeartbeatCount      = { ErrorOf( ES_SERVER2, 93, E_FAILED, EV_ADMIN, 1 ), "The heartbeat count cannot be less than %minCount%." } ;
ErrorId MsgServer2::HeartbeatAccessFailed  = { ErrorOf( ES_SERVER2, 94, E_FAILED, EV_COMM, 1 ), "Heartbeat missing: access to %target% server failed." } ;
ErrorId MsgServer2::HeartbeatMaxWait       = { ErrorOf( ES_SERVER2, 74, E_FAILED, EV_COMM, 1 ), "Heartbeat response exceeded maximum wait duration of %ms% milliseconds." } ;
ErrorId MsgServer2::HeartbeatTargetTooOld  = { ErrorOf( ES_SERVER2, 112, E_FAILED, EV_USAGE, 1 ), "Heartbeat target server %target% must be at version 2020.1 or above." } ;
ErrorId MsgServer2::DigestFail2            = { ErrorOf( ES_SERVER2, 95, E_FATAL, EV_ADMIN, 4 ), "Failed to create digest for revision %file%#%rev%. Archive %lbrfile% %lbrrev%" } ;
ErrorId MsgServer2::DuplicateCertificate   = { ErrorOf( ES_SERVER2, 97, E_FAILED, EV_NONE, 2 ), "Certificate for '%orgcommon%' with fingerprint '%fp%' already installed in trusted list." } ;
ErrorId MsgServer2::ExtensionCertInstallSuccess= { ErrorOf( ES_SERVER2, 98, E_INFO, EV_NONE, 2 ), "Certificate for '%orgcommon%' with fingerprint '%fp%' successfully installed and added to trusted list." } ;
ErrorId MsgServer2::ExtensionCertInstallPreview= { ErrorOf( ES_SERVER2, 99, E_INFO, EV_NONE, 2 ), "Would install certificate for '%orgcommon%' with fingerprint '%fp%'." } ;
ErrorId MsgServer2::ExtensionCertDelSuccess= { ErrorOf( ES_SERVER2, 100, E_INFO, EV_NONE, 2 ), "Certificate for '%orgcommon%' with fingerprint '%fp%' successfully deleted from trusted list." } ;
ErrorId MsgServer2::ExtensionCertDelPreview= { ErrorOf( ES_SERVER2, 101, E_INFO, EV_NONE, 2 ), "Would delete certificate for '%orgcommon%' with fingerprint '%fp%'." } ;
ErrorId MsgServer2::ExtensionCertMissing   = { ErrorOf( ES_SERVER2, 102, E_FAILED, EV_NONE, 1 ), "Certificate with fingerprint '%fp%' not found in trusted list." } ;
ErrorId MsgServer2::ExtensionNotSigned     = { ErrorOf( ES_SERVER2, 103, E_FAILED, EV_NONE, 0 ), "Installation failure: extension package must be signed but is missing required signature file or certificate file to validate authenticity; alternatively use the --allow-unsigned flag." } ;
ErrorId MsgServer2::ExtensionSignUntrusted = { ErrorOf( ES_SERVER2, 104, E_FAILED, EV_NONE, 0 ), "Installation failure: extension package is signed by untrusted certificate." } ;
ErrorId MsgServer2::ClientTooOldToPackage  = { ErrorOf( ES_SERVER2, 105, E_FAILED, EV_USAGE, 0 ), "Extension packaging requires a 2020.1+ command-line client." } ;
ErrorId MsgServer2::StreamSpecIntegPend    = { ErrorOf( ES_SERVER2, 106, E_WARN, EV_EMPTY, 0 ), "All stream spec revision(s) already integrated in pending changelist." } ;
ErrorId MsgServer2::StreamSpecInteg        = { ErrorOf( ES_SERVER2, 107, E_WARN, EV_EMPTY, 0 ), "All stream spec revision(s) already integrated." } ;
ErrorId MsgServer2::BadExternalAddr        = { ErrorOf( ES_SERVER2, 108, E_FAILED, EV_ADMIN, 3 ), "ExternalAddress %extAddr% set incorrectly for %serverId%: found server %realServerId%" } ;
ErrorId MsgServer2::ShelvePromotedStream   = { ErrorOf( ES_SERVER2, 111, E_INFO, EV_NONE, 1 ), "%change% stream promoted." } ;
ErrorId MsgServer2::ShelvePromotedBoth     = { ErrorOf( ES_SERVER2, 110, E_INFO, EV_NONE, 1 ), "%change% files and stream promoted." } ;
ErrorId MsgServer2::StreamSpecPermsDisabled = { ErrorOf( ES_SERVER2, 109, E_INFO, EV_NONE, 0 ), "Stream spec permissions currently disabled. Set %'dm.protects.streamspec=1'% to enable." } ;
ErrorId MsgServer2::UseDbSchema            = { ErrorOf( ES_SERVER2, 113, E_FAILED, EV_USAGE, 0 ), "Usage: %'dbschema [ [ -A [ dbtable ] ] | [ dbtable[:version] ... ] ]'%" } ;
ErrorId MsgServer2::UseStreamSpecParentView= { ErrorOf( ES_SERVER2, 114, E_FAILED, EV_USAGE, 0 ), "Usage: %'stream parentview  [ -c changelist# -n -o --source-comments ] { --inherit | --noinherit }'%" } ;
ErrorId MsgServer2::StgOrphanIndex         = { ErrorOf( ES_SERVER2, 115, E_INFO, EV_NONE, 3 ), "Missing db.storagesh record for storagesx key %lbrfile%#%lbrrev% %lbrtype%." } ;
ErrorId MsgServer2::StgIndexMismatch       = { ErrorOf( ES_SERVER2, 116, E_INFO, EV_NONE, 7 ), "Incorrect storage digest for key %lbrfile%#%lbrrev% %lbrtype%. Storagesx: %idigest% %isize% Storagesh: %sdigest% %ssize%" } ;
ErrorId MsgServer2::StgOrphanStart         = { ErrorOf( ES_SERVER2, 117, E_INFO, EV_NONE, 1 ), "Scan started on %path%" } ;
ErrorId MsgServer2::StgOrphanPause         = { ErrorOf( ES_SERVER2, 118, E_INFO, EV_NONE, 1 ), "Scan paused on %path%" } ;
ErrorId MsgServer2::StgOrphanRestart       = { ErrorOf( ES_SERVER2, 119, E_INFO, EV_NONE, 1 ), "Scan restarted on %path%" } ;
ErrorId MsgServer2::StgOrphanCancelled     = { ErrorOf( ES_SERVER2, 120, E_INFO, EV_NONE, 1 ), "Scan cancelled on %path%" } ;
ErrorId MsgServer2::StgOrphanWait          = { ErrorOf( ES_SERVER2, 121, E_INFO, EV_NONE, 1 ), "Scan process on %path% has finished." } ;
ErrorId MsgServer2::StgScanHeader          = { ErrorOf( ES_SERVER2, 122, E_INFO, EV_NONE, 1 ), "Status for scan %path%:" } ;
ErrorId MsgServer2::StgNoScans             = { ErrorOf( ES_SERVER2, 123, E_FAILED, EV_NONE, 0 ), "No scans are active." } ;
ErrorId MsgServer2::StgScanTargetMissing   = { ErrorOf( ES_SERVER2, 314, E_FAILED, EV_NONE, 1 ), "The target directory (%path%) must exist" } ;
ErrorId MsgServer2::StgScanTargetNoDir     = { ErrorOf( ES_SERVER2, 315, E_FAILED, EV_NONE, 1 ), "The target path (%path%) must be a directory" } ;
ErrorId MsgServer2::UpgradeInfo            = { ErrorOf( ES_SERVER2, 124, E_INFO, EV_NONE, 2 ), "Upgrade step %name% has status %state%" } ;
ErrorId MsgServer2::UpgradeComplete        = { ErrorOf( ES_SERVER2, 125, E_INFO, EV_NONE, 1 ), "Upgrade step %name% has been run." } ;
ErrorId MsgServer2::UpgradeNeeded          = { ErrorOf( ES_SERVER2, 126, E_WARN, EV_NONE, 1 ), "Upgrade step %name% has not completed on all servers." } ;
ErrorId MsgServer2::UpgradeRplUnknown      = { ErrorOf( ES_SERVER2, 127, E_FAILED, EV_NONE, 0 ), "Upstream servers must be upgraded to at least 2020.2 to report this information." } ;
ErrorId MsgServer2::UseUpgrades            = { ErrorOf( ES_SERVER2, 128, E_FAILED, EV_NONE, 0 ), "Usage: %'upgrades [-g upgrade-step]'%" } ;
ErrorId MsgServer2::BadPRoot               = { ErrorOf( ES_SERVER2, 132, E_FAILED, EV_ADMIN, 1 ), "Proxy Root directory %path% (set with %'$P4PROOT'% or %'-R'% flag) invalid." };
ErrorId MsgServer2::FailedToUpdUnExpKtextDigest = { ErrorOf( ES_SERVER2, 133, E_FAILED, EV_NONE, 3 ), "Failed to update an unexpanded ktext digest in db.storagesh on the Commit Server for %lbrfile%#%lbrrev% %lbrtype%." } ;
ErrorId MsgServer2::StreamHasParentView    = { ErrorOf( ES_SERVER2, 134, E_INFO, EV_NONE, 2 ), "Stream spec %stream% already has a ParentView property value of %parentView%." } ;
ErrorId MsgServer2::StreamParentViewChanged= { ErrorOf( ES_SERVER2, 135, E_INFO, EV_NONE, 2 ), "Converted stream spec %stream% ParentView property to %parentView%." } ;
ErrorId MsgServer2::StreamPVSpecOpen       = { ErrorOf( ES_SERVER2, 136, E_FAILED, EV_USAGE, 1 ), "Stream spec %stream% is open.  To convert the ParentView property the stream spec must not be open." } ;
ErrorId MsgServer2::UpdateDigestReport     = { ErrorOf( ES_SERVER2, 137, E_INFO, EV_NONE, 2 ), "Storage digest update complete. Records read: %reads% Records updated: %updates%." } ;
ErrorId MsgServer2::UpdateDigestProgress   = { ErrorOf( ES_SERVER2, 138, E_INFO, EV_NONE, 2 ), "Storage digest update. Records read: %reads% Records updated: %updates%." } ;
ErrorId MsgServer2::StreamPVVirtualOnlyInh = { ErrorOf( ES_SERVER2, 139, E_FAILED, EV_USAGE, 0 ), "Virtual streams may only have inherit ParentViews." } ;
ErrorId MsgServer2::SSInhPVIntegNotDone    = { ErrorOf( ES_SERVER2, 140, E_FAILED, EV_ADMIN, 1 ), "The stream spec %stream% has an inherit ParentView and cannot be integrated.  The configurable dm.integ.streamspec must be 2 to integrate a stream spec with an inherit ParentView." } ;
ErrorId MsgServer2::StreamPVTaskOnlyInh    = { ErrorOf( ES_SERVER2, 141, E_FAILED, EV_USAGE, 0 ), "Task streams may only have inherit ParentViews." } ;
ErrorId MsgServer2::SSIntegNotCurStream    = { ErrorOf( ES_SERVER2, 142, E_FAILED, EV_USAGE, 2 ), "The target stream of a stream spec integration must be the stream associated with the current client. The specified target stream is %targetStream%, but the current stream is %currentStream%.  Stream spec integration failed." } ;
ErrorId MsgServer2::ExtCfgMissing          = { ErrorOf( ES_SERVER2, 143, E_FAILED, EV_UNKNOWN, 2 ), "Missing Extension config for %name%, %uuid%." } ;
ErrorId MsgServer2::NoUnshelveVirtIntoNoInh= { ErrorOf( ES_SERVER2, 144, E_FAILED, EV_ILLEGAL, 0 ), "Can't unshelve a virtual stream spec into a stream spec with a noinherit ParentView." } ;
ErrorId MsgServer2::NoUnshelveNoInhIntoVirt= { ErrorOf( ES_SERVER2, 145, E_FAILED, EV_ILLEGAL, 0 ), "Can't unshelve a stream spec with a noinherit ParentView into a virtual stream spec." } ;
ErrorId MsgServer2::ReplicaSharedConfig    = { ErrorOf( ES_SERVER2, 146, E_FATAL, EV_ADMIN, 1 ), "Replica cannot run! This replica server does not appear to share archive storage with its master server for depotname=%depot%. However, the replica was configured with lbr.replication=shared. Setting lbr.replication=shared without sharing archive storage can result in transfer errors between servers. Map paths for all depots can be verified with '%'p4 depots -a'%'. Please reconfigure this replica or contact Perforce Technical Support for assistance." } ;
ErrorId MsgServer2::RtMonitorDisabled      = { ErrorOf( ES_SERVER2, 147, E_FAILED, EV_ADMIN, 0 ), "Realtime monitoring not currently enabled." } ;
ErrorId MsgServer2::SwitchStreamUnrelated  = { ErrorOf( ES_SERVER2, 148, E_FAILED, EV_ILLEGAL, 0 ), "Use the '--allow-unrelated' option to switch to a different stream hierarchy." } ;
ErrorId MsgServer2::PurgeReportArchive     = { ErrorOf( ES_SERVER2, 149, E_WARN, EV_NONE, 0 ), "This was report mode.  Use %'-y'% to remove files. Add %'-A'% to include archived revisions." } ;
ErrorId MsgServer2::ReplicaLag             = { ErrorOf( ES_SERVER2, 150, E_INFO, EV_NONE, 2 ), "Replica is %byteLag% bytes of journal behind the upstream server (%journals% journal rotations)" } ;
ErrorId MsgServer2::InfoProxyServerID      = { ErrorOf( ES_SERVER2, 151, E_INFO, EV_NONE, 1 ), "Proxy serverID: %svrId%" } ;
ErrorId MsgServer2::MoveReaddIntegConflictResolveWarn= { ErrorOf( ES_SERVER2, 152, E_WARN, EV_NONE, 6 ), "Warning: %fromFile%%endFromRev% was added after a move of the previous version.\nThe move resolve of %clientFile% was ignored.\nAccepting branch resolve (at) from %fromFile%%endFromRev% will overwrite workspace file %clientFile%." } ;
ErrorId MsgServer2::ShelveArchiveInUse     = { ErrorOf( ES_SERVER2, 153, E_FAILED, EV_UNKNOWN, 2 ), "Archive for shelved file %depotFile%@%change% is already in use and differs!" } ;
ErrorId MsgServer2::ShelveSuffixBumpFailed = { ErrorOf( ES_SERVER2, 358, E_FAILED, EV_UNKNOWN, 2 ), "Failed to increment suffixes during promote of shelf %change%, triggered by in-use archive for %depotFile%." } ;
ErrorId MsgServer2::ShelveDupDiff          = { ErrorOf( ES_SERVER2, 154, E_FAILED, EV_UNKNOWN, 10 ), "Shelved archive file %lbrFile1%/%lbrRev1%/%lbrType1% (%size1% %hash1%) differs from %lbrFile2%/%lbrRev2%/%lbrType2% (%size2% %hash2%). Skipping any further cleanup." } ; 
ErrorId MsgServer2::ShelveNotPromoted      = { ErrorOf( ES_SERVER2, 155, E_FAILED, EV_UNKNOWN, 1 ), "%change% does not contain promoted shelved files." } ;
ErrorId MsgServer2::UseMonitorRT           = { ErrorOf( ES_SERVER2, 157, E_FAILED, EV_USAGE, 0 ), "Usage: %'monitor realtime [ -F filter -T fields ]'%" } ;
ErrorId MsgServer2::VerifyRepairKtext      = { ErrorOf( ES_SERVER2, 158, E_WARN, EV_UNKNOWN, 0 ), "Cannot find shelved archives for ktext files by digest. Skipping." } ;
ErrorId MsgServer2::VerifyRepairNone       = { ErrorOf( ES_SERVER2, 159, E_WARN, EV_UNKNOWN, 0 ), "No suitable shelved archive found." } ;
ErrorId MsgServer2::VerifyRepairConflict   = { ErrorOf( ES_SERVER2, 160, E_WARN, EV_UNKNOWN, 2 ), "Cannot copy shelved archive %foundLbr%. Target file %targetLbr% already exists." } ;
ErrorId MsgServer2::VerifyRepairSnapped    = { ErrorOf( ES_SERVER2, 161, E_INFO, EV_NONE, 2 ), "Snapping revision from using archive %oldLbr% to use archive %targetLbr%." } ;
ErrorId MsgServer2::VerifyRepairCopied     = { ErrorOf( ES_SERVER2, 162, E_INFO, EV_NONE, 2 ), "Copying shelved archive %foundLbr% to %targetLbr%." } ;
ErrorId MsgServer2::UseVerifyR             = { ErrorOf( ES_SERVER2, 163, E_FAILED, EV_USAGE, 0 ), "Usage: %'verify -R [ -m maxRevs ] [ -q -s ] [ -v | --only BAD|MISSING ] ] [ -X ] [ -b batch ] files...'%" } ;
ErrorId MsgServer2::InfoCommitServer       = { ErrorOf( ES_SERVER2, 164, E_INFO, EV_NONE, 1 ), "Commit server ID: %commitServerId%" };
ErrorId MsgServer2::InfoEdgeServer         = { ErrorOf( ES_SERVER2, 165, E_INFO, EV_NONE, 1 ), "Upstream edge server ID: %edgeServerId%" };
ErrorId MsgServer2::MovePairSplit          = { ErrorOf( ES_SERVER2, 166, E_FAILED, EV_NOTYET, 0 ), "Cannot submit half of a moved file pair." } ;
ErrorId MsgServer2::UseTopology            = { ErrorOf( ES_SERVER2, 167, E_FAILED, EV_USAGE, 0 ), "Usage: %'topology [ [ -a | -t numOfDays ] [ -F filter ] [ -T field,... ] ] | [ -D date [ -y ] [ -e ] [ -s serveraddress ] [ -i serverID ] [ -p targetaddress ] ] | [ -d [ -y ] [ -s serveraddress -i serverID [ -p targetaddress ] ] | [ -c date [ -e ] ] | [ -l lastSeenDate [ -e ] ] ] | [ -m [ -y ] -s serveraddress -i serverID [ -p targetaddress ] -S newserveraddress -I newserverID [ -P newtargetaddress ] ]'%" } ;
ErrorId MsgServer2::TopologyOnCurrentSvr   = { ErrorOf( ES_SERVER2, 168, E_INFO, EV_NONE, 1 ), "Topology command was run on server - %address%" } ;
ErrorId MsgServer2::FileNoMatchStgDigest   = { ErrorOf( ES_SERVER2, 169, E_FAILED, EV_ADMIN, 2 ), "The archive does not match the storage digest %depotpath%#%rev%." } ;
ErrorId MsgServer2::FileNoMatchStgSize     = { ErrorOf( ES_SERVER2, 170, E_FAILED, EV_ADMIN, 2 ), "The archive does not match the storage size %depotpath%#%rev%." } ;
ErrorId MsgServer2::UseStreams2            = { ErrorOf( ES_SERVER2, 171, E_FAILED, EV_USAGE, 0 ), "Usage: %'streams --viewmatch //depotPath1 [ [--viewmatch //depotPathN] ... ] '%" } ;
ErrorId MsgServer2::UnknownContext         = { ErrorOf( ES_SERVER2, 172, E_FATAL, EV_ILLEGAL, 1 ), "No %field% was found for this operation." } ;
ErrorId MsgServer2::RplTooBig              = { ErrorOf( ES_SERVER2, 173, E_FAILED, EV_ADMIN, 3 ), "Replication request too large for available memory on '%serverID%'. Consider adding memory or decreasing 'rpl.jnl.batch.size' to less than %batchSize% for '%replicaID%'." } ;
ErrorId MsgServer2::RplReduced             = { ErrorOf( ES_SERVER2, 174, E_WARN, EV_ADMIN, 2 ), "Replication successful using reduced batch size of %batchSize%. Consider adjusting 'rpl.jnl.batch.size' for '%replicaID%'." } ;
ErrorId MsgServer2::UseP4dF                = { ErrorOf( ES_SERVER2, 179, E_FAILED, EV_USAGE, 0 ), "Usage: %'p4d -Fm | -Fs -r p4root [ -y ] masterServerID standbyServerID'%" } ;
ErrorId MsgServer2::P4dFBadMaster          = { ErrorOf( ES_SERVER2, 180, E_FAILED, EV_ADMIN, 3 ), "The serverID '%id%' in the root directory '%s%' does not match the master argument '%masterId%' used in this command." } ;
ErrorId MsgServer2::P4dFRefuseMissing      = { ErrorOf( ES_SERVER2, 181, E_FAILED, EV_ADMIN, 4 ), "The server '%id%' in the root directory '%root%' cannot be converted to a standby after %action%. It cannot be verified that this server participated in %action% with master participation." } ;
ErrorId MsgServer2::P4dFFConfigsMissing    = { ErrorOf( ES_SERVER2, 182, E_FAILED, EV_ADMIN, 5 ), "The server '%id%' in the root directory '%root%' cannot be converted to a standby after %action%. The required '%failoverconfigs%' file generated by %action% is missing." } ;
ErrorId MsgServer2::P4dFStandbyNotStandby  = { ErrorOf( ES_SERVER2, 183, E_FAILED, EV_ADMIN, 1 ), "The standby server argument '%standbyArg%' is not a standby server." } ;
ErrorId MsgServer2::P4dFBadRplFrom         = { ErrorOf( ES_SERVER2, 184, E_FAILED, EV_ADMIN, 2 ), "The '%'ReplicatingFrom'%' server field of the server '%standby%' does not match the master server '%master%'" } ;
ErrorId MsgServer2::P4dFPreview            = { ErrorOf( ES_SERVER2, 186, E_INFO, EV_NONE, 2 ), "Checking if reconfiguring '%master%' as '%standby%' might be possible..." } ;
ErrorId MsgServer2::P4dFStarting           = { ErrorOf( ES_SERVER2, 187, E_INFO, EV_NONE, 2 ), "Reconfiguring '%master%' as '%standby%'..." } ;
ErrorId MsgServer2::P4dFOK                 = { ErrorOf( ES_SERVER2, 188, E_INFO, EV_NONE, 0 ), "No errors reported; use -y to execute the command." } ;
ErrorId MsgServer2::P4dFSuccess            = { ErrorOf( ES_SERVER2, 189, E_INFO, EV_NONE, 2 ), "Server '%master%' has been reconfigured as restricted standby '%standby%'." } ;
ErrorId MsgServer2::P4dFFailbackNotRun     = { ErrorOf( ES_SERVER2, 190, E_FAILED, EV_ADMIN, 0 ), "Perhaps '%'p4 failback'%' has not yet been run?" } ;
ErrorId MsgServer2::P4dFRestrictedStart    = { ErrorOf( ES_SERVER2, 191, E_INFO, EV_NONE, 0 ), "Standby replica starting in restricted mode..." } ;
ErrorId MsgServer2::IntegTaskNoDirect      = { ErrorOf( ES_SERVER2, 192, E_FAILED, EV_USAGE, 0 ), "The direct integration option cannot be used with a target task stream." } ;
ErrorId MsgServer2::ExtraPxcIDUsage        = { ErrorOf( ES_SERVER2, 193, E_FAILED, EV_USAGE, 0 ), "Usage: %'p4p -xD [ serverID ]'%" }; 
ErrorId MsgServer2::BadPxcExtraFlag        = { ErrorOf( ES_SERVER2, 194, E_FAILED, EV_USAGE, 0 ), "Unknown %'-x'% operation.  Try '%'D'%'." } ;
ErrorId MsgServer2::LbrDeletionFailed      = { ErrorOf( ES_SERVER2, 201, E_FAILED, EV_ADMIN, 3 ), "Unable to delete archive file - lbrFile-%lbrFile% lbrRev-%lbrRev% lbrType-%lbrType%." } ;
ErrorId MsgServer2::ThreadMustBeNumeric    = { ErrorOf( ES_SERVER2, 202, E_FAILED, EV_USAGE, 1 ), "Non-numeric value not allowed for thread argument '%arg%'." } ;
ErrorId MsgServer2::ThreadBiggerMinusTwo   = { ErrorOf( ES_SERVER2, 203, E_FAILED, EV_USAGE, 1 ), "Thread count must be zero or greater. '%arg%'." } ;
ErrorId MsgServer2::NoStdoutPDump          = { ErrorOf( ES_SERVER2, 204, E_FAILED, EV_ADMIN, 0 ), "Parallel database dumping is not allowed to stdout." } ;
ErrorId MsgServer2::NoDirPDump             = { ErrorOf( ES_SERVER2, 205, E_FAILED, EV_ADMIN, 1 ), "The target directory %dir% must not pre-exist." } ;
ErrorId MsgServer2::BadJournalSubOpt       = { ErrorOf( ES_SERVER2, 206, E_FAILED, EV_ADMIN, 0 ), "Unsupported suboption to -j. Must be -jc[s|p|pm], -jd[s|p|ps|pm], -jr[p|F|c|pF|pc], -jv[v|p|pv]." } ;
ErrorId MsgServer2::LicenseExpiryWarning   = { ErrorOf( ES_SERVER2, 207, E_WARN, EV_ADMIN, 1 ), "Your license is due to expire on %expiry_date%; please contact sales@perforce.com to obtain an updated license file to avoid downtime." } ;
ErrorId MsgServer2::InfoProxyCacheRoot     = { ErrorOf( ES_SERVER2, 208, E_INFO, EV_NONE, 1 ), "Proxy cacheRoot: %root%" } ;
ErrorId MsgServer2::InfoProxyRoot          = { ErrorOf( ES_SERVER2, 209, E_INFO, EV_NONE, 1 ), "Proxy root: %root%" } ;
ErrorId MsgServer2::UpgradeFeatureUnknown  = { ErrorOf( ES_SERVER2, 210, E_FAILED, EV_NONE, 1 ), "Upstream servers must be upgraded to at least %version% to use this feature." } ;
ErrorId MsgServer2::NoParallelMflag        = { ErrorOf( ES_SERVER2, 211, E_FAILED, EV_ADMIN, 0 ), "The -m option is not supported for a parallel recovery" } ;
ErrorId MsgServer2::NoParallelCflag        = { ErrorOf( ES_SERVER2, 212, E_FAILED, EV_ADMIN, 0 ), "The 'c' suboption is not supported for a parallel recovery" } ;
ErrorId MsgServer2::NoParallelSflag        = { ErrorOf( ES_SERVER2, 213, E_FAILED, EV_ADMIN, 0 ), "The -s option is not supported for a parallel recovery" } ;
ErrorId MsgServer2::BadRecoverDir          = { ErrorOf( ES_SERVER2, 214, E_FAILED, EV_ADMIN, 1 ), "Bad recovery directory %dir%" } ;
ErrorId MsgServer2::BadRecoverFileName     = { ErrorOf( ES_SERVER2, 215, E_FAILED, EV_ADMIN, 1 ), "Bad recovery filename %file%" } ;
ErrorId MsgServer2::NoStorageMflag         = { ErrorOf( ES_SERVER2, 216, E_FAILED, EV_ADMIN, 0 ), "The -S option cannot be combined with the -m option" } ;
ErrorId MsgServer2::NoParallelSSflag       = { ErrorOf( ES_SERVER2, 217, E_FAILED, EV_ADMIN, 0 ), "The -S option is not supported for a parallel recovery" } ;
ErrorId MsgServer2::SyncWStreamViewChange  = { ErrorOf( ES_SERVER2, 218, E_INFO, EV_NONE, 2 ), "Stream client view derived from stream spec version at or before change %change% for stream %stream%." } ;
ErrorId MsgServer2::SyncWStreamViewHeadChange = { ErrorOf( ES_SERVER2, 219, E_INFO, EV_NONE, 1 ), "Stream client view derived from current stream spec for %stream%." } ;
ErrorId MsgServer2::StreamAtChangeVsStreamViewChange = { ErrorOf( ES_SERVER2, 220, E_INFO, EV_NONE, 0 ), "StreamAtChange defined in the client spec overrules any --use-stream-change value." } ;
ErrorId MsgServer2::UseRenameClient        = { ErrorOf( ES_SERVER2, 221, E_FAILED, EV_USAGE, 0 ), "Usage: %'renameclient --from=old --to=new'%" } ;
ErrorId MsgServer2::FromToSame             = { ErrorOf( ES_SERVER2, 222, E_FAILED, EV_USAGE, 1 ), "Client %client% cannot be renamed to itself." } ;
ErrorId MsgServer2::ClientNotExist         = { ErrorOf( ES_SERVER2, 223, E_FAILED, EV_UNKNOWN, 1 ), "Client %client% doesn't exist." } ;
ErrorId MsgServer2::NewClientExists        = { ErrorOf( ES_SERVER2, 224, E_FAILED, EV_USAGE, 1 ), "Client %client% already exists." } ;
ErrorId MsgServer2::RenameClientUnloaded   = { ErrorOf( ES_SERVER2, 225, E_FAILED, EV_USAGE, 0 ), "Unloaded client cannot be renamed." } ;
ErrorId MsgServer2::RenameClientPartitioned= { ErrorOf( ES_SERVER2, 226, E_FAILED, EV_USAGE, 0 ), "Partitioned or readonly clients cannot be renamed." } ;
ErrorId MsgServer2::ClientOpenStream       = { ErrorOf( ES_SERVER2, 227, E_FAILED, EV_USAGE, 0 ), "Client with open stream cannot be renamed." } ;
ErrorId MsgServer2::RenameNeedsMaster      = { ErrorOf( ES_SERVER2, 228, E_FAILED, EV_USAGE, 2 ), "The master server must be accessible to rename a client on a build server." } ;
ErrorId MsgServer2::RenameNeedsCommit      = { ErrorOf( ES_SERVER2, 229, E_FAILED, EV_USAGE, 2 ), "The commit server must be accessible to rename a client on an edge server." } ;
ErrorId MsgServer2::RenameNoPromoted       = { ErrorOf( ES_SERVER2, 230, E_FAILED, EV_USAGE, 0 ), "Client with promoted shelved changes cannot be renamed." } ;
ErrorId MsgServer2::ClientRenamed          = { ErrorOf( ES_SERVER2, 231, E_INFO, EV_NONE, 2 ), "Client %oldClient% renamed to %newClient%." } ;
ErrorId MsgServer2::ClientNeedsRecover     = { ErrorOf( ES_SERVER2, 232, E_FAILED, EV_ADMIN, 0 ), "Unexpected db error encountered during client rename. Run 'p4d -xx' to check for database inconsistencies." } ;
ErrorId MsgServer2::ClientRenameFailed     = { ErrorOf( ES_SERVER2, 233, E_FAILED, EV_ADMIN, 0 ), "Client rename failed." } ;
ErrorId MsgServer2::ClientNotLocal         = { ErrorOf( ES_SERVER2, 234, E_FAILED, EV_USAGE, 0 ), "Client rename must be run on local server." } ;
ErrorId MsgServer2::RenameCommitOld        = { ErrorOf( ES_SERVER2, 235, E_FAILED, EV_ADMIN, 0 ), "Master or commit server must be upgraded to at least 2022.2 to use this feature." } ;
ErrorId MsgServer2::UseRenameWorkspace     = { ErrorOf( ES_SERVER2, 236, E_FAILED, EV_USAGE, 0 ), "Usage: %'renameworkspace --from=old --to=new'%" } ;
ErrorId MsgServer2::MultiFilePara          = { ErrorOf( ES_SERVER2, 237, E_FAILED, EV_ADMIN, 0 ), "The multifile suboption can only be specified with the parallel suboption" } ;
ErrorId MsgServer2::PasswordChangeSU       = { ErrorOf( ES_SERVER2, 238, E_FAILED, EV_PROTECT, 0 ), "Initial passwords can only be set by a super-user whose password is set." } ;
ErrorId MsgServer2::RenameClientNotAllowed = { ErrorOf( ES_SERVER2, 239, E_FAILED, EV_ADMIN, 0 ), "Client renames are not allowed." } ;
ErrorId MsgServer2::RenameClientAdminSuper = { ErrorOf( ES_SERVER2, 240, E_FAILED, EV_PROTECT, 0 ), "Admin or super permission is required to rename a client." } ;
ErrorId MsgServer2::RenameClientSuper      = { ErrorOf( ES_SERVER2, 241, E_FAILED, EV_PROTECT, 0 ), "Super permission is required to rename a client." } ;
ErrorId MsgServer2::RenameClientNotOwner   = { ErrorOf( ES_SERVER2, 242, E_FAILED, EV_ILLEGAL, 1 ), "The '%oldClient%' client is owned by another user; admin or super permission is required to rename another user's client." } ;
ErrorId MsgServer2::PreserveChangeNumberConflict = { ErrorOf( ES_SERVER2, 247, E_INFO, EV_NONE, 0), "Cannot fetch changes because change numbers can not be preserved" } ;
ErrorId MsgServer2::DistributionBlockSubmit      = { ErrorOf( ES_SERVER2, 248, E_FAILED, EV_ILLEGAL, 0 ), "Cannot submit on distribution server." } ;
ErrorId MsgServer2::DistributionServerOverrides  = { ErrorOf( ES_SERVER2, 249, E_FAILED, EV_ADMIN, 0 ), "This server is configured as a type of distribution. P4TARGET/db.replication/lbr.replication must not be set." } ;
ErrorId MsgServer2::MissingParallelFile    = { ErrorOf( ES_SERVER2, 250, E_FAILED, EV_ADMIN, 3 ), "Missing checksum file(s) in checkpoint directory '%dir%', expecting %ex% found %seen%." } ;
ErrorId MsgServer2::AltSyncNoSupport       = { ErrorOf( ES_SERVER2, 251, E_FAILED, EV_CLIENT, 0 ), "Client does not support P4ALTSYNC, required for this workspace." } ;
ErrorId MsgServer2::AltSyncNotConfigured   = { ErrorOf( ES_SERVER2, 252, E_FAILED, EV_CLIENT, 0 ), "Client requires P4ALTSYNC configuration for this workspace. Verify P4ALTSYNC is set and/or VFS application is running." } ;
ErrorId MsgServer2::AltSyncNoVersion       = { ErrorOf( ES_SERVER2, 253, E_FAILED, EV_CLIENT, 0 ), "AltSync agent did not send an altSync protocol version." };
ErrorId MsgServer2::AltSyncBadVersion      = { ErrorOf( ES_SERVER2, 254, E_FAILED, EV_CLIENT, 1 ), "AltSync agent using unsupported altSync protocol version: %version%" } ;
ErrorId MsgServer2::AltSyncNoProg          = { ErrorOf( ES_SERVER2, 326, E_FAILED, EV_CLIENT, 0 ), "AltSync agent did not send an altSync agent program." };
ErrorId MsgServer2::AltSyncActive          = { ErrorOf( ES_SERVER2, 255, E_INFO, EV_CLIENT, 2 ), "AltSync agent '[%prog%|unknown]' in use using altSync protocol version: %version%" } ;
ErrorId MsgServer2::UpgradeAuthDown        = { ErrorOf( ES_SERVER2, 256, E_FAILED, EV_ADMIN, 0 ), "Server upgrade cannot proceed because the authentication server is not accessible." } ;
ErrorId MsgServer2::UpgradeAuth            = { ErrorOf( ES_SERVER2, 257, E_FAILED, EV_ADMIN, 0 ), "The authentication server must be upgraded before this server can upgrade." } ;
ErrorId MsgServer2::UpgradeAuthNoLicense   = { ErrorOf( ES_SERVER2, 258, E_FAILED, EV_ADMIN, 0 ), "The authentication server must have a valid license before this server can upgrade." } ;
ErrorId MsgServer2::UpgradeAuthMaintenance = { ErrorOf( ES_SERVER2, 259, E_FAILED, EV_ADMIN, 0 ), "Server upgrade cannot proceed because the authentication server is running in maintenance mode." } ;
ErrorId MsgServer2::UpgradeAuthRestricted  = { ErrorOf( ES_SERVER2, 260, E_FAILED, EV_ADMIN, 0 ), "Server upgrade cannot proceed because the authentication server is running in restricted mode." } ;
ErrorId MsgServer2::LowResourceTerm        = { ErrorOf( ES_SERVER2, 261, E_FATAL, EV_TOOBIG, 1 ), "Server low on resources (%type%), command terminated." } ;
ErrorId MsgServer2::ImpatientPauseTerm     = { ErrorOf( ES_SERVER2, 262, E_FATAL, EV_TOOBIG, 1 ), "Waited too long (%time%s) while command paused;  terminated." } ;
ErrorId MsgServer2::TooManyPausedTerm      = { ErrorOf( ES_SERVER2, 263, E_FATAL, EV_TOOBIG, 0 ), "Too many commands paused;  terminated." } ;
ErrorId MsgServer2::TooMuchResourceMonitor = { ErrorOf( ES_SERVER2, 264, E_FATAL, EV_TOOBIG, 0 ), "There is already a 'resource-monitor' background process running." } ;
ErrorId MsgServer2::UseAdminResourceMonitor= { ErrorOf( ES_SERVER2, 265, E_FAILED, EV_USAGE, 0 ), "Usage: %'admin resource-monitor'%" } ;
ErrorId MsgServer2::StartupCapabilities    = { ErrorOf( ES_SERVER2, 266, E_INFO, EV_NONE, 5 ), "Perforce Server operating system capabilities (%date%, PID %pid%, version %ver%):  OOM protection (%oom%), OS-supplied resource pressure (%psi%)." };
ErrorId MsgServer2::BadPressureThresholds  = { ErrorOf( ES_SERVER2, 267, E_WARN, EV_ADMIN, 0 ), "Resource monitoring medium/high threshold configuration invalid - using defaults.\n" } ;
ErrorId MsgServer2::PopulateSparseStreamDesc = { ErrorOf( ES_SERVER2, 268, E_INFO, EV_NONE, 2 ), "Populate stream %targetStream% from %sourceStream%." };
ErrorId MsgServer2::NonResidentOpenMustSync = { ErrorOf( ES_SERVER2, 269, E_FAILED, EV_USAGE, 2 ), "%clientFile% - must sync before opening for %action% in sparse stream." };
ErrorId MsgServer2::BadJField              = { ErrorOf( ES_SERVER2, 270, E_FAILED, EV_USAGE, 2 ), "Bad jfield option specified '%opt%'. Must be one of %optlist%" } ;
ErrorId MsgServer2::TooManySparseStreamFiles = { ErrorOf( ES_SERVER2, 271, E_FAILED, EV_USAGE, 4 ), "Can't %action% files in sparse stream %stream% (%branchMax% or more depot files already in %depotPath%)" };
ErrorId MsgServer2::TraitDepotNotConfigured= { ErrorOf( ES_SERVER2, 272, E_FAILED, EV_CONTEXT, 0 ), "Trait depot storage has not been configured by '%'trait.storagedepot.min'%'. This value must be greater than 0 to enable trait depot storage." } ;
ErrorId MsgServer2::TraitDepotNotForCommit = { ErrorOf( ES_SERVER2, 273, E_FAILED, EV_CONTEXT, 0 ), "Trait depot storage is not supported on this version of the commit server. Commit server must be upgraded to use this feature." } ;
ErrorId MsgServer2::NoValidIPOrMACAddresses = { ErrorOf( ES_SERVER2, 274, E_FAILED, EV_ADMIN, 0), "No valid IP or MAC Addresses available for License use."} ;
ErrorId MsgServer2::ValidIPv4Address       = { ErrorOf( ES_SERVER2, 275, E_INFO, EV_NONE, 2), "IPv4 Address (Interface %int%): %address%"} ;
ErrorId MsgServer2::ValidIPv6Address       = { ErrorOf( ES_SERVER2, 276, E_INFO, EV_NONE, 2 ), "IPv6 Address (Interface %int%): %address%" };
ErrorId MsgServer2::ValidMACAddress        = { ErrorOf( ES_SERVER2, 277, E_INFO, EV_NONE, 2 ), "MAC Address (Interface %int%): %address%" };
ErrorId MsgServer2::NoTraitValueInDepot    = { ErrorOf( ES_SERVER2, 278, E_FAILED, EV_ADMIN, 2 ), "The value for trait '%name%' was not found in the trait depot at %path%." };
ErrorId MsgServer2::AttributeNoWild        = { ErrorOf( ES_SERVER2, 294, E_FAILED, EV_USAGE, 0 ), "Attribute name must not include wildcards." };
ErrorId MsgServer2::AttributeNotFound      = { ErrorOf( ES_SERVER2, 295, E_FAILED, EV_USAGE, 3 ), "%depotFile%#%depotRev% - no such attribute[ '%attr%'|s] found." };
ErrorId MsgServer2::AttributeFileEmpty     = { ErrorOf( ES_SERVER2, 296, E_FAILED, EV_USAGE, 1 ), "File '%traitFile%' is empty: cannot set attribute value." };
ErrorId MsgServer2::UseTraitI              = { ErrorOf( ES_SERVER2, 297, E_FAILED, EV_USAGE, 0 ), "Usage: %'attribute -I filename [-f -p [-T0|-T1]] -n name file'%" } ;
ErrorId MsgServer2::Types64Warn            = { ErrorOf( ES_SERVER2, 279, E_WARN, EV_ADMIN, 2 ), "Server requires a client that supports 64bit changelist numbers. Client version: %cv%. Required version %rv%." } ;
ErrorId MsgServer2::Types64Err             = { ErrorOf( ES_SERVER2, 280, E_FAILED, EV_ADMIN, 2 ), "Server requires a client that supports 64bit changelist numbers. Client version: %cv%. Required version %rv%." } ;
ErrorId MsgServer2::ConfigureSetComment    = { ErrorOf( ES_SERVER2, 281, E_INFO, EV_NONE, 5 ), "For server '%serverName%', configuration variable '%variableName%' comment set to '%comment%'[ overwriting previous comment '%oldComment%'] at iteration %iteration%." } ;
ErrorId MsgServer2::ProxyClearCacheNotSet  = { ErrorOf( ES_SERVER2, 282, E_INFO, EV_NOTYET, 0 ), "Proxy cache-purge needs configurable 'proxy.clearcachethresh'. Please set it and try again."} ;
ErrorId MsgServer2::StreamMustBeSparse     = { ErrorOf( ES_SERVER2, 283, E_FAILED, EV_USAGE, 0 ), "This operation is only allowed on sparse streams." } ;
ErrorId MsgServer2::InvalidDestStreamType  = { ErrorOf( ES_SERVER2, 284, E_FAILED, EV_USAGE, 0 ), "Invalid stream type. Stream type must be release or development." };
ErrorId MsgServer2::UseStreamConvertSparse = { ErrorOf( ES_SERVER2, 285, E_FAILED, EV_USAGE, 0 ), "Usage: %'stream convertsparse [ -q ]'%" };
ErrorId MsgServer2::ConfigureMandatoryComment = { ErrorOf( ES_SERVER2, 286, E_FAILED, EV_USAGE, 0), "Cannot set/unset configurable without comment.\nSee '%'p4 help configure'%'." } ;
ErrorId MsgServer2::NoJournalRotateWarning = { ErrorOf( ES_SERVER2, 287, E_INFO, EV_ADMIN, 1), "No journal rotation in the past %days% days." } ;
ErrorId MsgServer2::CommitVerifyNoExternalAddress = { ErrorOf( ES_SERVER2, 288, E_FAILED, EV_ADMIN, 1 ), "Commit verify requires 'ExternalAddress' field in %serverID% server spec to be set" };
ErrorId MsgServer2::SwitchStreamFailedReconcile = { ErrorOf( ES_SERVER2, 289, E_FAILED, EV_NOTYET, 0 ), "Failed to switch streams. Could not reconcile all files and not all local changes could be shelved. Please resolve issues shown by 'p4 reconcile' and try again." };
ErrorId MsgServer2::MissingConfigDbFile = { ErrorOf( ES_SERVER2, 290, E_FAILED, EV_ADMIN, 1 ), "Recovery directory %dir% contains partitioned clients but no db.config replay file." } ;
ErrorId MsgServer2::MissingPcdir = { ErrorOf( ES_SERVER2, 291, E_FAILED, EV_ADMIN, 1 ), "The Recovery directory %dir% contains partitioned clients but the db.config replay file does not contain a clientReadonlyDir setting." } ;
ErrorId MsgServer2::NoStreamSpecEditStreamAtChangeClient = { ErrorOf( ES_SERVER2, 292, E_FAILED, EV_USAGE, 0 ), "Stream spec cannot be opened for edit in StreamAtChange client." } ;
ErrorId MsgServer2::NoStreamSpecUnshelveStreamAtChangeClient = { ErrorOf( ES_SERVER2, 293, E_FAILED, EV_USAGE, 0 ), "Stream spec cannot be unshelved in StreamAtChange client." } ;
ErrorId MsgServer2::SparseStreamOperationNotAllowed = { ErrorOf( ES_SERVER2, 298, E_FAILED, EV_USAGE, 0 ), "This operation is not allowed on sparse streams." } ;
ErrorId MsgServer2::UseHotFiles            = { ErrorOf( ES_SERVER2, 299, E_FAILED, EV_USAGE, 0 ), "Usage: %'hotfiles [ -i | -o ]'%" } ;
ErrorId MsgServer2::UseDiags               = { ErrorOf( ES_SERVER2, 300, E_FAILED, EV_USAGE, 0 ), "Usage: %'diagnostics [ -D ] [ -j ] [ -J [ -L limit ] ] | [ [ -l ] [ -s [--strace-runtime secs] [ -t ] [ -p ] [ -h ] ] [ -a [ -J [ -L limit ] ] ]'%" } ;
ErrorId MsgServer2::FilelimitSuf           = { ErrorOf( ES_SERVER2, 301, E_FAILED, EV_USAGE, 0 ), "The filelimit suffix is invalid, valid suffixes: one of tTgGmMkK." } ;
ErrorId MsgServer2::Filelimit              = { ErrorOf( ES_SERVER2, 309, E_FAILED, EV_USAGE, 0 ), "The filelimit string is invalid." } ;
ErrorId MsgServer2::DiagsNotSupported      = { ErrorOf( ES_SERVER2, 302, E_FAILED, EV_USAGE, 1 ), "The %cmd% suboption is not supported on this platform" } ;
ErrorId MsgServer2::DiagsDone              = { ErrorOf( ES_SERVER2, 313, E_INFO, EV_NONE, 3 ), "The diagnostics command has created the '%path%'[%type%directory| compressed tar file] containing: %file%." } ;
ErrorId MsgServer2::FileTarlimit           = { ErrorOf( ES_SERVER2, 325, E_FAILED, EV_NONE, 0 ), "The file limit must be < 8589934591 when creating a .tgz file" } ;
ErrorId MsgServer2::ShelveCantUpdateSpec   = { ErrorOf( ES_SERVER2, 303, E_FAILED, EV_USAGE , 1 ), "Stream spec for %stream% - already shelved, use %'-f'% to update." };
ErrorId MsgServer2::LogExportSummary       = { ErrorOf( ES_SERVER2, 307, E_INFO, EV_NONE, 4 ), "Logfile %logfile% %state% state is:\tLogfile number %lognum%,\toffset %logoffset%." } ;
ErrorId MsgServer2::CurrentServerTime      = { ErrorOf( ES_SERVER2, 304, E_INFO, EV_NONE, 2 ), "The current server time is currently:\t%serverDate% %serverTimeZone%" } ;
ErrorId MsgServer2::UseLogExport           = { ErrorOf( ES_SERVER2, 305, E_FAILED, EV_USAGE, 0 ), "Usage: %'logexport [ -l | -e endpoint [ -h headers... ] [ -I interval ] ] log'%" } ;
ErrorId MsgServer2::NoLogStatefile         = { ErrorOf( ES_SERVER2, 306, E_FAILED, EV_ADMIN, 1 ), "No export statefile found for log %logfile%" } ;
ErrorId MsgServer2::DataOutOfBounds        = { ErrorOf( ES_SERVER2, 308, E_FAILED, EV_FAULT, 0 ), "Data received out of boundaries." } ;
ErrorId MsgServer2::RExcludeList           = { ErrorOf( ES_SERVER2, 310, E_INFO, EV_FAULT, 1 ), "List of excluded tables for %type%: %list%" } ;
ErrorId MsgServer2::ExcludeListUsage       = { ErrorOf( ES_SERVER2, 311, E_FAILED, EV_FAULT, 0 ), "The %'-l'% option may only be used with %'-jd or -jr and -R servertype'%." } ;
ErrorId MsgServer2::PrehashedPassword      = { ErrorOf( ES_SERVER2, 312, E_FAILED, EV_USAGE, 0 ), "Prehashed passwords are no longer supported." } ;
ErrorId MsgServer2::UseMoveM               = { ErrorOf( ES_SERVER2, 316, E_FAILED, EV_USAGE, 0 ), "Usage: %'move -M [ -c changelist# ] [ -n ] [--parallel=N]'%" } ;
ErrorId MsgServer2::StraceRuntime          = { ErrorOf( ES_SERVER2, 317, E_FAILED, EV_USAGE, 0 ), "Bad strace runtime parameter" } ;
ErrorId MsgServer2::StraceBusy             = { ErrorOf( ES_SERVER2, 318, E_FAILED, EV_USAGE, 0 ), "A strace request is currently active" } ;
ErrorId MsgServer2::IdentFailed            = { ErrorOf( ES_SERVER2, 319, E_FAILED, EV_FAULT, 2 ), "Ident: %ident%/%group%" };
ErrorId MsgServer2::PullTransferVSummary   = { ErrorOf( ES_SERVER2, 320, E_INFO, EV_NONE, 8 ), "Active/queued/failed/retry: %transfersVActive%/%transfersVQueued%/%transfersVFailed%/%transfersVRetry%, bytes: %bytesVActive%/%bytesVQueued%/%bytesVFailed%/%bytesVRetry%" } ;
ErrorId MsgServer2::UseSyncst              = { ErrorOf( ES_SERVER2, 321, E_FAILED, EV_USAGE, 0 ), "Usage: %'sync -k --sync-time=N [files...]'%" };
ErrorId MsgServer2::DeltaTransferRplErr    = { ErrorOf( ES_SERVER2, 322, E_INFO, EV_FAULT, 3 ), "Delta Transfer failed for file = %file%, lbrrev = %lbrrev%. Falling back to full file transfer.\nDelta Transfer Error = %error%\n" } ;
ErrorId MsgServer2::ObliterateExtension    = { ErrorOf( ES_SERVER2, 323, E_FAILED, EV_ADMIN, 1 ), "Obliterate cannot delete extension file(s) %file%\nSee '%'p4 help extension'%' to delete extensions." };
ErrorId MsgServer2::LoginIdled             = { ErrorOf( ES_SERVER2, 324, E_FAILED, EV_ILLEGAL, 0 ), "Your session has expired due to inactivity, please %'login'% again." } ;
ErrorId MsgServer2::P4authNeeds20251       = { ErrorOf( ES_SERVER2, 327, E_FAILED, EV_NOTYET, 0 ), "The P4AUTH server requires the connecting server to be upgraded to 2025.1 or above." } ;
ErrorId MsgServer2::UseWebServer           = { ErrorOf( ES_SERVER2, 335, E_FAILED, EV_USAGE, 0 ), "Usage: %'webserver { start | stop | show }'%" };
ErrorId MsgServer2::UseWebServerStart      = { ErrorOf( ES_SERVER2, 328, E_FAILED, EV_USAGE, 0 ), "Usage: %'webserver start -p [tcp:][ip:]port'%" };
ErrorId MsgServer2::UseWebServerStop       = { ErrorOf( ES_SERVER2, 334, E_FAILED, EV_USAGE, 0 ), "Usage: %'webserver stop -p [ip:]port'%" };
ErrorId MsgServer2::WebServerStarted       = { ErrorOf( ES_SERVER2, 329, E_INFO, EV_NONE, 2 ), "Web server started on [%ip%:]%port%." };
ErrorId MsgServer2::WebServerStopped       = { ErrorOf( ES_SERVER2, 330, E_INFO, EV_NONE, 0 ), "Web server stopped." };
ErrorId MsgServer2::WebServerAlreadyRunning = { ErrorOf( ES_SERVER2, 331, E_FAILED, EV_USAGE, 2 ), "Web server already running on [%ip%:]%port%." };
ErrorId MsgServer2::WebServerStartFailed   = { ErrorOf( ES_SERVER2, 332, E_FAILED, EV_FAULT, 0 ), "Failed to start web server." };
ErrorId MsgServer2::WebServerStopFailed    = { ErrorOf( ES_SERVER2, 333, E_FAILED, EV_USAGE, 0 ), "Failed to stop web server." };
ErrorId MsgServer2::WebServerError         = { ErrorOf( ES_SERVER2, 340, E_FAILED, EV_FAULT, 0 ), "Web server encountered an exception while running the request: %exceptionMsg%." } ;
ErrorId MsgServer2::RplFilterRestrict      = { ErrorOf( ES_SERVER2, 337, E_INFO, EV_NONE, 1 ), "Replica filter restrict processing %db%." };
ErrorId MsgServer2::RplFilterExpand        = { ErrorOf( ES_SERVER2, 347, E_INFO, EV_NONE, 1 ), "Replica filter expand processing %db%." };
ErrorId MsgServer2::TooMuchRplFilter       = { ErrorOf( ES_SERVER2, 338, E_FAILED, EV_ADMIN, 1 ), "There is already a '%cmd%' process running." };
ErrorId MsgServer2::RplFilterReport        = { ErrorOf( ES_SERVER2, 339, E_INFO, EV_NONE, 3 ), "Records scanned: %scan%, records filtered: %filtered% from %db%." };
ErrorId MsgServer2::ExtensionConfigExists  = { ErrorOf( ES_SERVER2, 341, E_FAILED, EV_ADMIN, 2 ), "Could not create new configuration instance as this extension only allows a single configuration instance.\nConfiguration instance '%instance%' already exists for '%extension%'." };
ErrorId MsgServer2::RmtOpFailedInconsistent = { ErrorOf( ES_SERVER2, 342, E_FAILED, EV_FAULT, 0 ), "Operation failed on remote server because of data inconsistency, possibly due to stalled or slow replication." };
ErrorId MsgServer2::MustSetPasswordRequireSU = { ErrorOf( ES_SERVER2, 343, E_FAILED, EV_USAGE, 0 ), "Password must be set by super user before access can be granted." };
ErrorId MsgServer2::ServiceAuthErr         = { ErrorOf( ES_SERVER2, 363, E_FAILED, EV_ADMIN, 2 ), "Service user '%svc%' failed authentication check on server '%server%'" };
ErrorId MsgServer2::InfoServerCertExpire   = { ErrorOf( ES_SERVER2, 344, E_INFO, EV_NONE, 1 ), "Server cert expires: %serverCertExpire%" } ;
ErrorId MsgServer2::InfoProxyCertExpire    = { ErrorOf( ES_SERVER2, 345, E_INFO, EV_NONE, 1 ), "Proxy cert expires: %proxyCertExpire%" } ;
ErrorId MsgServer2::WebServerRuntime       = { ErrorOf( ES_SERVER2, 346, E_FAILED, EV_FAULT, 1 ), "Web server runtime startup failed: %message%" };
ErrorId MsgServer2::ExtensionNotFound      = { ErrorOf( ES_SERVER2, 348, E_FAILED, EV_NONE, 1 ), "Extension with UUID '%uuid%' not found." };
ErrorId MsgServer2::ExtensionDisabled      = { ErrorOf( ES_SERVER2, 349, E_INFO, EV_NONE, 1 ), "Extension with UUID '%uuid%' has been disabled." };
ErrorId MsgServer2::DisableExtensionUsage  = { ErrorOf( ES_SERVER2, 350, E_FAILED, EV_USAGE, 0 ), "Usage: '-xf disable.extension uuid'" };
ErrorId MsgServer2::StgRevMiss             = { ErrorOf( ES_SERVER2, 351, E_INFO, EV_NONE, 3 ), "Missing db.rev record. db.storage count %stg% db.rev count %rev% key %key.%" } ;
ErrorId MsgServer2::StgRevShMiss           = { ErrorOf( ES_SERVER2, 352, E_INFO, EV_NONE, 3 ), "Missing db.revsh record. db.storagesh count %stg% db.revsh count %rev% key %key%." } ;
ErrorId MsgServer2::StgStorageMiss         = { ErrorOf( ES_SERVER2, 353, E_INFO, EV_NONE, 3 ), "Missing db.storage record. db.storage count %stg% db.rev count %rev% key %key%." } ;
ErrorId MsgServer2::StgStorageShMiss       = { ErrorOf( ES_SERVER2, 354, E_INFO, EV_NONE, 3 ), "Missing db.storagesh record. db.storagesh count %stg% db.revsh count %rev% key %key%." } ;
ErrorId MsgServer2::StgBadRefCount         = { ErrorOf( ES_SERVER2, 355, E_INFO, EV_NONE, 3 ), "Incorrect db.storage refCount. db.storage count %stg% db.rev count %rev% key %key%." } ;
ErrorId MsgServer2::StgBadShRefCount       = { ErrorOf( ES_SERVER2, 356, E_INFO, EV_NONE, 3 ), "Incorrect db.storagesh refCount. db.storagesh count %stg% db.revsh count %rev% key %key%." } ;
ErrorId MsgServer2::RESTAPISetVarErr       = { ErrorOf( ES_SERVER2, 357, E_FATAL, EV_FAULT, 2 ), "Encountered an exception while transferring data to the web server for the variable '%varName%'. %errMsg%'" };
ErrorId MsgServer2::ReplicaFilterNotReplica = { ErrorOf( ES_SERVER2, 359, E_FAILED, EV_USAGE, 0 ), "Replica-filter-reconcile only allowed on replica servers." } ;
ErrorId MsgServer2::UserNotOfType          = { ErrorOf( ES_SERVER2, 360, E_FAILED, EV_USAGE, 1 ), "User must be of type %type%." } ;
ErrorId MsgServer2::NotTheAuthServer       = { ErrorOf( ES_SERVER2, 361, E_FAILED, EV_USAGE, 0 ), "This command must be run on the P4AUTH server." } ;
ErrorId MsgServer2::InfoStats              = { ErrorOf( ES_SERVER2, 362, E_INFO, EV_NONE, 2 ), "Server Statistics: %stats% encrypted(%statsEnc%)" } ;
ErrorId MsgServer2::WebServerShow          = { ErrorOf( ES_SERVER2, 364, E_INFO, EV_NONE, 3 ), "Protocol: %proto% Interface: %iface% Port: %port% Uptime %uptime%[%units%HH:MM:SS| Days] User %user%." } ;
ErrorId MsgServer2::ConfigureReadonlyDirUnsetWarn = { ErrorOf( ES_SERVER2, 365, E_WARN, EV_NONE, 0 ), "When 'client.readonly.dir' is unset, any partitioned clients will be inaccessible until 'client.readonly.dir' is reconfigured." } ;
ErrorId MsgServer2::ExtPkgPathTraversal    = { ErrorOf( ES_SERVER2, 369, E_FAILED, EV_FAULT, 1 ), "Path traversal in extension archive: '%filename%' resolves outside the extraction sandbox." } ;


/* Deprecated */
ErrorId MsgServer2::StgKeyMiss             = { ErrorOf( ES_SERVER2, 4, E_INFO, EV_NONE, 1 ), "Can't find key %key% in the checkmap." } ;
ErrorId MsgServer2::StgBadCount            = { ErrorOf( ES_SERVER2, 5, E_INFO, EV_NONE, 3 ), "Bad count for key %key% Storage rec: %rec% Scan value: %scan%." } ;
ErrorId MsgServer2::StgOrphan              = { ErrorOf( ES_SERVER2, 6, E_INFO, EV_NONE, 1 ), "Orphan storage record for key %key%." } ;
ErrorId MsgServer2::StoreCount2            = { ErrorOf( ES_SERVER2, 336, E_INFO, EV_ADMIN, 3 ), "Storage %scount% Rev %rcount% Key %key%" } ;

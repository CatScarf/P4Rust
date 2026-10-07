/*
 * Copyright 2023 Perforce Software. All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * msgconfig.cc - definitions of help messages for config variables.
 *
 * Note:
 *
 * Never re-use an error code value, these may have already been
 * translated, so using it for a different error is not OK.
 *
 * ErrorIds which are no longer used should be moved to the bottom
 * of the list, with a trailing comment like this: // DEPRECATED.
 * We keep these to maintain compatibility between newer api clients
 * and older servers which send old ErrorIds.
 *
 * Its okay to add a message in the middle of the file.
 *
 * When adding a new error make sure it's greater than the current high
 * value and update the following number:
 *
 * Current high value is: 536
 */

//
// Debug levels
//

ErrorId MsgConfig::Db = { ErrorOf( ES_CONFIG, 435, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the database.
)"
};

ErrorId MsgConfig::Diff = { ErrorOf( ES_CONFIG, 436, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the diff/merge subsystem.
)"
};

ErrorId MsgConfig::Dm = { ErrorOf( ES_CONFIG, 437, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the data manager subsystem.
)"
};

ErrorId MsgConfig::Dmc = { ErrorOf( ES_CONFIG, 438, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the data manager collections subsystem.
)"
};

ErrorId MsgConfig::Ftp = { ErrorOf( ES_CONFIG, 439, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::Handle = { ErrorOf( ES_CONFIG, 440, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the connection scoped state handler subsystem.
)"
};

ErrorId MsgConfig::Lbr = { ErrorOf( ES_CONFIG, 441, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the librarian subsystem (archive management).
)"
};

ErrorId MsgConfig::Map = { ErrorOf( ES_CONFIG, 442, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the map subsystem.
)"
};

ErrorId MsgConfig::Net = { ErrorOf( ES_CONFIG, 443, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the network subsystem.
)"
};

ErrorId MsgConfig::Options = { ErrorOf( ES_CONFIG, 444, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::Peek = { ErrorOf( ES_CONFIG, 445, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging related to lockless database reads.
)"
};

ErrorId MsgConfig::Rcs = { ErrorOf( ES_CONFIG, 446, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the RCS subsystem.
)"
};

ErrorId MsgConfig::Records = { ErrorOf( ES_CONFIG, 447, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of array deletion when >= 4 and extensions when >= 5.
)"
};

ErrorId MsgConfig::Rpc = { ErrorOf( ES_CONFIG, 448, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the RPC subsystem.
)"
};

ErrorId MsgConfig::Server = { ErrorOf( ES_CONFIG, 449, E_INFO, EV_NONE, 0 ),
R"(Enables server logging of commands (levels are additive):
	0: Fatal errors and some system events will be logged
	1: All command starts and track information on long commands are logged
	2: Track information is logged
	3: All ends are logged
	4: All warnings and details related server locks and network addresses
	   are logged
	5: Database locking, monitor and resource pressure information is logged
)"
};

ErrorId MsgConfig::Spec = { ErrorOf( ES_CONFIG, 450, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the spec parser/formatter subsystem.
)"
};

ErrorId MsgConfig::Track = { ErrorOf( ES_CONFIG, 451, E_INFO, EV_NONE, 0 ),
R"(Controls the detail of performance tracking relative to the number of users:
	-1: Default based on number of licensed users
	0: Off
	1: Maximum detail regardless of user count
	>=2: Increasing thresholds before track information is logged
)"
};

ErrorId MsgConfig::Ob = { ErrorOf( ES_CONFIG, 452, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::Viewgen = { ErrorOf( ES_CONFIG, 453, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the streams view generator subsystem.
)"
};

ErrorId MsgConfig::Rpl = { ErrorOf( ES_CONFIG, 454, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the replication subsystem.
)"
};

ErrorId MsgConfig::Ssl = { ErrorOf( ES_CONFIG, 455, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of SSL operations.
)"
};

ErrorId MsgConfig::Time = { ErrorOf( ES_CONFIG, 456, E_INFO, EV_NONE, 0 ),
R"(When enabled, debug logging information will be prefixed with timestamps.
)"
};

ErrorId MsgConfig::Cluster = { ErrorOf( ES_CONFIG, 457, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::Zk = { ErrorOf( ES_CONFIG, 458, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::Ldap = { ErrorOf( ES_CONFIG, 459, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the LDAP subsystem.
)"
};

ErrorId MsgConfig::Dvcs = { ErrorOf( ES_CONFIG, 460, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the DVCS subsystem.
)"
};

ErrorId MsgConfig::Graph = { ErrorOf( ES_CONFIG, 461, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the graph depot subsystem.
)"
};

ErrorId MsgConfig::Gconn = { ErrorOf( ES_CONFIG, 462, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging for git-connector (must be set on the git-connector).
)"
};

ErrorId MsgConfig::Fovr = { ErrorOf( ES_CONFIG, 463, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the failover subsystem.
)"
};

ErrorId MsgConfig::Script = { ErrorOf( ES_CONFIG, 464, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of usage of the embedded Lua interpreter.
)"
};

ErrorId MsgConfig::Stg = { ErrorOf( ES_CONFIG, 465, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging from the storage subsystem (see '%'p4 help storage'%').
)"
};

ErrorId MsgConfig::Thread = { ErrorOf( ES_CONFIG, 466, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the thread manager subsystem.
)"
};

ErrorId MsgConfig::Exts = { ErrorOf( ES_CONFIG, 467, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the extensions subsystem.
)"
};

ErrorId MsgConfig::Protect = { ErrorOf( ES_CONFIG, 468, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the protections subsystem.
)"
};

ErrorId MsgConfig::Heartbeat = { ErrorOf( ES_CONFIG, 469, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the heartbeat subsystem.
)"
};

ErrorId MsgConfig::Shelve = { ErrorOf( ES_CONFIG, 470, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the shelving subsystem.
)"
};

ErrorId MsgConfig::Sqw = { ErrorOf( ES_CONFIG, 471, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the stream view queue worker subsystem.
)"
};

ErrorId MsgConfig::Stm = { ErrorOf( ES_CONFIG, 472, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the stream materialisation subsystem.
See '%'p4 files|fstat|dirs --streamviews'%'
)"
};

ErrorId MsgConfig::Pcheck = { ErrorOf( ES_CONFIG, 473, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of parallel checkpoints.
)"
};

ErrorId MsgConfig::Topology = { ErrorOf( ES_CONFIG, 474, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the topology subsystem.
)"
};

ErrorId MsgConfig::Resource = { ErrorOf( ES_CONFIG, 475, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the resource pressure subsystem.
)"
};

ErrorId MsgConfig::S3 = { ErrorOf( ES_CONFIG, 476, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the S3 object storage subsystem.
)"
};

ErrorId MsgConfig::Suptool = { ErrorOf( ES_CONFIG, 483, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the Support Tool (perfmerge, perfsplit, p4migrate)
subsystem.
)"
};

ErrorId MsgConfig::Elog = { ErrorOf( ES_CONFIG, 490, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the exported logging system.
)"
};

ErrorId MsgConfig::Dltxfer = { ErrorOf( ES_CONFIG, 495, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the delta transfer.
)"
};

ErrorId MsgConfig::Perf = { ErrorOf( ES_CONFIG, 501, E_INFO, EV_NONE, 0 ),
R"(Enables display of performance stats.
)"
};

ErrorId MsgConfig::WebSrv = { ErrorOf( ES_CONFIG, 512, E_INFO, EV_NONE, 0 ),
R"(Enables debug logging of the webserver subsystem.
)"
};

//
// Numeric Tunables
//

ErrorId MsgConfig::ClusterJournalShared = { ErrorOf( ES_CONFIG, 1, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::DbCheckpointBufsize = { ErrorOf( ES_CONFIG, 2, E_INFO, EV_NONE, 0 ),
R"(Used in conjunction with '%'dbjournal.bufsize'%' to determine the write size used
when creating checkpoint or dump files. The write size used is the greater
of these two configurables.
)"
};

ErrorId MsgConfig::DbCheckpointThreads = { ErrorOf( ES_CONFIG, 3, E_INFO, EV_NONE, 0 ),
R"(The number of threads to use in checkpoint, dump and recovery operations.
)"
};

ErrorId MsgConfig::DbCheckpointWorklevel = { ErrorOf( ES_CONFIG, 4, E_INFO, EV_NONE, 0 ),
R"(The keys at this depth in a database file are used to split a checkpoint or
dump operation into multiple outputs.
)"
};

ErrorId MsgConfig::DbCheckpointReqlevel = { ErrorOf( ES_CONFIG, 5, E_INFO, EV_NONE, 0 ),
R"(Only database files that have a depth greater that this configurable will be
split into multiple outputs.
)"
};

ErrorId MsgConfig::DbCheckpointNumfiles = { ErrorOf( ES_CONFIG, 6, E_INFO, EV_NONE, 0 ),
R"(Used to determine the number of files produced from a database file during a
checkpoint or dump operation.
)"
};

ErrorId MsgConfig::DbExperimentalLogging = { ErrorOf( ES_CONFIG, 7, E_INFO, EV_NONE, 0 ),
R"(Enables the asynchronous lock tracking infrastructure, logging which tables a
blocked process is trying to acquire locks on.
)"
};

ErrorId MsgConfig::DbInternalRepair = { ErrorOf( ES_CONFIG, 8, E_INFO, EV_NONE, 0 ),
R"(If set to 1, will repair/recover space in internal nodes if it will avoid a page
split. This should only be set for old databases which can not or have not been
rebuilt from checkpoint since the 2022.2 release.
)"
};

ErrorId MsgConfig::DbIsalive = { ErrorOf( ES_CONFIG, 9, E_INFO, EV_NONE, 0 ),
R"(Number of database rows to scan in between '%'maxLockTime'%' checks.
)"
};

ErrorId MsgConfig::DbJnlackShared = { ErrorOf( ES_CONFIG, 10, E_INFO, EV_NONE, 0 ),
R"(Number of shared-memory pages to use for the '%'db.jnlack'%' table.
A value of 0 disables it.
)"
};

ErrorId MsgConfig::DbJournalrotateWarnthresh = { ErrorOf( ES_CONFIG, 488, E_INFO, EV_NONE, 0 ),
R"(Number of days prior to another journal rotation being needed and
providing the warning if '%'db.journalrotate.warnthresh'%' is more than zero.
)"
};

ErrorId MsgConfig::DbMonitorAddthresh = { ErrorOf( ES_CONFIG, 11, E_INFO, EV_NONE, 0 ),
R"(Milliseconds before adding a command or connection at monitor level 1 or 2.
)"
};

ErrorId MsgConfig::DbMonitorInterval = { ErrorOf( ES_CONFIG, 12, E_INFO, EV_NONE, 0 ),
R"(Minimum number of seconds between when a command checks to see if it has been
terminated via '%'p4 monitor terminate'%'.
)"
};

ErrorId MsgConfig::DbMonitorMinruntime = { ErrorOf( ES_CONFIG, 13, E_INFO, EV_NONE, 0 ),
R"(Minimum of seconds before a monitor table entry's state can be changed by
'%'p4 monitor terminate | pause | resume'%'.
)"
};

ErrorId MsgConfig::DbMonitorTermallow = { ErrorOf( ES_CONFIG, 14, E_INFO, EV_NONE, 0 ),
R"(When set to 1 this configurable allows users to terminate their own processes
using '%'p4 monitor terminate'%'.
)"
};

ErrorId MsgConfig::DbMonitorShared = { ErrorOf( ES_CONFIG, 15, E_INFO, EV_NONE, 0 ),
R"(The number of 8K pages of shared memory to store monitor data used by the
'%'p4 monitor'%' command. Values can be set between 0 and 4096 on Windows, 32768
on all other operating systems. A setting of 0 disables use of shared memory
and writes to the '%'db.monitor'%' table in '%'P4ROOT'%'.
)"
};

ErrorId MsgConfig::DbPageMigrate = { ErrorOf( ES_CONFIG, 16, E_INFO, EV_NONE, 0 ),
R"(If a new page is the to be allocated at a position beyond this threshold, in
percent, into the database file, it will be allocated as close to the start of
file as possible instead.
)"
};

ErrorId MsgConfig::DbPartitionVerify = { ErrorOf( ES_CONFIG, 485, E_INFO, EV_NONE, 0 ),
R"(Enables the verification of partitioned tables.
)"
};

ErrorId MsgConfig::DbPartitionDrop = { ErrorOf( ES_CONFIG, 486, E_INFO, EV_NONE, 0 ),
R"(Enabling the deletion of the partitioned have table instead of deleting the
records in the table.
)"
};

ErrorId MsgConfig::DbPeeking = { ErrorOf( ES_CONFIG, 17, E_INFO, EV_NONE, 0 ),
R"(Controls the behavior of lockless database reads:
	0: Disabled - Unsupported deprecated behavior. DO NOT USE.
	1: Lockless reads enabled but not used
	2: Lockless reads used where generally applicable
	3: Some commands will use lockless reads to reduce overall contention at
	   the cost of some performance to the individual commands.
)"
};

ErrorId MsgConfig::DbPeekingUsemaxlock = { ErrorOf( ES_CONFIG, 18, E_INFO, EV_NONE, 0 ),
R"(If enabled, long peek locks (lockless reads) will be contribute to the overall
lock time that '%'maxLockTime'%' check.
)"
};

ErrorId MsgConfig::DbReorgDisable = { ErrorOf( ES_CONFIG, 19, E_INFO, EV_NONE, 0 ),
R"(If set to 1, disables database page reorganisation on internal page creation.
Reorganisation of the database historically improved consecutive read
performance on spinning disks; this optimisation doesn't give the same benefits
on solid state storage.
)"
};

ErrorId MsgConfig::DbReorgMisorder = { ErrorOf( ES_CONFIG, 20, E_INFO, EV_NONE, 0 ),
R"(The percentage of non consecutive child pages to trigger a database page
reorganisation on internal page creation.
)"
};

ErrorId MsgConfig::DbReorgOccup = { ErrorOf( ES_CONFIG, 21, E_INFO, EV_NONE, 0 ),
R"(The percentage occupancy of a page to consider child database page
reorganisation on internal page creation.
)"
};

ErrorId MsgConfig::DbRtIo = { ErrorOf( ES_CONFIG, 22, E_INFO, EV_NONE, 0 ),
R"(When enabled, the '%'rtv.db.io.records'%' realtime counter will count database
records read, written and deleted.
)"
};

ErrorId MsgConfig::DbTrylock = { ErrorOf( ES_CONFIG, 23, E_INFO, EV_NONE, 0 ),
R"(Number of attempts to take database locks without blocking.
)"
};

ErrorId MsgConfig::DbarrayPutcheck = { ErrorOf( ES_CONFIG, 24, E_INFO, EV_NONE, 0 ),
R"(Interval at which internal db record arrays are checked for memory limits
(measured in number of records).
)"
};

ErrorId MsgConfig::DbarrayReserve = { ErrorOf( ES_CONFIG, 25, E_INFO, EV_NONE, 0 ),
R"(Currently unused.
)"
};

ErrorId MsgConfig::DbjournalBufsize = { ErrorOf( ES_CONFIG, 26, E_INFO, EV_NONE, 0 ),
R"(Number of bytes to be read from/written to journals and checkpoints each IO.
For checkpoints, if '%'db.checkpoint.bufsize'%' is larger, that is used instead.
)"
};

ErrorId MsgConfig::DbjournalWordsize = { ErrorOf( ES_CONFIG, 27, E_INFO, EV_NONE, 0 ),
R"(Number of bytes to be read from journals and checkpoints each IO for unbuffered
use cases.
)"
};

ErrorId MsgConfig::DbopenCache = { ErrorOf( ES_CONFIG, 28, E_INFO, EV_NONE, 0 ),
R"(Maximum number of database pages to cache.
)"
};

ErrorId MsgConfig::DbopenCacheWide = { ErrorOf( ES_CONFIG, 29, E_INFO, EV_NONE, 0 ),
R"(Maximum number of wide database pages to cache.
)"
};

ErrorId MsgConfig::DbopenFreepct = { ErrorOf( ES_CONFIG, 30, E_INFO, EV_NONE, 0 ),
R"(Percentage of page to leave empty when inserting consecutive records.
)"
};

ErrorId MsgConfig::DbopenMismatchDelay = { ErrorOf( ES_CONFIG, 31, E_INFO, EV_NONE, 0 ),
R"(Milliseconds to wait between inconsistent lockless page re-reads.
)"
};

ErrorId MsgConfig::DbopenNofsync = { ErrorOf( ES_CONFIG, 32, E_INFO, EV_NONE, 0 ),
R"(When enabled, disables fsync on database files for performance.
)"
};

ErrorId MsgConfig::DbopenPagesize = { ErrorOf( ES_CONFIG, 33, E_INFO, EV_NONE, 0 ),
R"(Size of database pages. Do not change this once the database has been created.
)"
};

ErrorId MsgConfig::DbopenRetry = { ErrorOf( ES_CONFIG, 34, E_INFO, EV_NONE, 0 ),
R"(Number of attempts to locklessly read a database page.
)"
};

ErrorId MsgConfig::DiffBinaryRcs = { ErrorOf( ES_CONFIG, 35, E_INFO, EV_NONE, 0 ),
R"(Treat '%'binary+D'%' type files as text when diffing.
)"
};

ErrorId MsgConfig::DiffSlimit1 = { ErrorOf( ES_CONFIG, 36, E_INFO, EV_NONE, 0 ),
R"(Maximum length of diff "snake" (number of lines searched for matching chunk)
for files with > '%'diff.sthresh'%' lines.
)"
};

ErrorId MsgConfig::DiffSlimit2 = { ErrorOf( ES_CONFIG, 37, E_INFO, EV_NONE, 0 ),
R"(Maximum length of diff "snake" (number of lines searched for matching chunk)
for files with <= '%'diff.sthresh'%' lines.
)"
};

ErrorId MsgConfig::DiffSthresh = { ErrorOf( ES_CONFIG, 38, E_INFO, EV_NONE, 0 ),
R"(Files with <= this number of lines use '%'diff.slimit2'%' instead of '%'diff.slimit1'%'.
)"
};

ErrorId MsgConfig::DmAltsyncEnforce = { ErrorOf( ES_CONFIG, 39, E_INFO, EV_NONE, 0 ),
R"(When enabled, clients with the '%'altsysnc'%' option set can only be used from
clients that support '%'P4ALTSYNC'%' and have a properly configured agent.
)"
};

ErrorId MsgConfig::DmAnnotateMaxsize = { ErrorOf( ES_CONFIG, 40, E_INFO, EV_NONE, 0 ),
R"(Maximum size a file revision allowed by '%'p4 annotate'%'.
)"
};

ErrorId MsgConfig::DmBatchDomains = { ErrorOf( ES_CONFIG, 41, E_INFO, EV_NONE, 0 ),
R"(Number of labels to process per lock on '%'db.label'%' when running '%'p4 labels'%'
with filespec arguments. Values less than 1000 disable batching.
)"
};

ErrorId MsgConfig::DmBatchNet = { ErrorOf( ES_CONFIG, 42, E_INFO, EV_NONE, 0 ),
R"(Number of global label record updates to send from the commit server per RPC
message from an edge server. Applies to the edge server.
)"
};

ErrorId MsgConfig::DmChangeRestrictPending = { ErrorOf( ES_CONFIG, 43, E_INFO, EV_NONE, 0 ),
R"(Description visibility for restricted pending changes:
	0: Only visible by owner of change.
	1: Visible by anyone with list access to file(s).
)"
};

ErrorId MsgConfig::DmChangeSkipkeyed = { ErrorOf( ES_CONFIG, 44, E_INFO, EV_NONE, 0 ),
R"(When set to 1, updates to a submitted changelist will drop the checksum and
filesize for '%'+k'%' type files instead of recalculating them. These may be
recalculated later with '%'p4 verify -u'%'.
)"
};

ErrorId MsgConfig::DmChangesThresh1 = { ErrorOf( ES_CONFIG, 45, E_INFO, EV_NONE, 0 ),
R"(Wide path optimization for '%'p4 changes'%' without a revision specifier: the
maximum number of non-matching rows scanned against '%'db.rev'%' before using
'%'db.revcx'%'.
)"
};

ErrorId MsgConfig::DmChangesThresh2 = { ErrorOf( ES_CONFIG, 46, E_INFO, EV_NONE, 0 ),
R"(Wide path optimization for '%'p4 changes'%' without a revision specifier: the
maximum number of matching rows scanned against '%'db.rev'%' before using
'%'db.revcx'%'. This must be less than or equal to '%'dm.changes.thresh1'%'.
)"
};

ErrorId MsgConfig::DmChangeviewOpenable = { ErrorOf( ES_CONFIG, 47, E_INFO, EV_NONE, 0 ),
R"(When set to 1, files restricted by a client's ChangeView may still be opened for
add/edit/delete/etc but may not be submitted.
)"
};

ErrorId MsgConfig::DmClientInitroot = { ErrorOf( ES_CONFIG, 48, E_INFO, EV_NONE, 0 ),
R"(Enables the use of '%'P4INITROOT'%' to outside of DVCS to automatically resolve
the client root.
)"
};

ErrorId MsgConfig::DmClientLimitprotects = { ErrorOf( ES_CONFIG, 49, E_INFO, EV_NONE, 0 ),
R"(If enabled, restricts the user's effective protections by their client view
when running commands that would be limited to the scope of the client view.
)"
};

ErrorId MsgConfig::DmConfigureCommentMandatory = { ErrorOf(ES_CONFIG, 481, E_INFO, EV_NONE, 0),
R"(If set to 1, changing configurable values with '%'p4 configure'%' requires
a comment.
)"
};

ErrorId MsgConfig::DmCopyAttributes = { ErrorOf( ES_CONFIG, 518, E_INFO, EV_NONE, 0 ),
R"(If set to 0, '%'p4 copy'%' will ignore files that differ only by attributes.
)"
};

ErrorId MsgConfig::DmCopyMovewarn = { ErrorOf( ES_CONFIG, 50, E_INFO, EV_NONE, 0 ),
R"(If set to 1, '%'p4 copy'%' displays a warning when both halves of a moved file
are not represented in the copy result, and suggests an earlier changelist that
may be used to perform the copy in multiple stages to ensure that all move
actions are reflected. This can be used to automatically create branches that
may be arbitrarily reparented without losing the ability to merge refactoring
changes.
)"
};

ErrorId MsgConfig::DmDomainAccessupdate = { ErrorOf( ES_CONFIG, 51, E_INFO, EV_NONE, 0 ),
R"(Time interval to update access time on use of a client/label/stream/branch,
if a lock can be taken.
)"
};

ErrorId MsgConfig::DmDomainAccessforce = { ErrorOf( ES_CONFIG, 52, E_INFO, EV_NONE, 0 ),
R"(Time interval to force the client/label/stream/branch access time update.
)"
};

ErrorId MsgConfig::DmFetchPreservechangenumbers = { ErrorOf( ES_CONFIG, 53, E_INFO, EV_NONE, 0 ),
R"(If enabled, preserve original changelist numbers on changes transferred by
'%'p4 fetch'%' or '%'p4 push'%' to this server.
This makes the '%'--preserve-change-numbers'%' flag implicit.
)"
};

ErrorId MsgConfig::DmFlushforce = { ErrorOf( ES_CONFIG, 54, E_INFO, EV_NONE, 0 ),
R"(Maximum batch size of files being synced/opened/reverted before waiting for a
lock to persist.
)"
};

ErrorId MsgConfig::DmFlushtry = { ErrorOf( ES_CONFIG, 55, E_INFO, EV_NONE, 0 ),
R"(Batch size interval of files being synced/opened/reverted before trying to
take a lock to persist; if the lock could be taken, the batch will grow to the
next multiple of this interval before trying again, unless the size reaches
'%'dm.flush.force'%'.
)"
};

ErrorId MsgConfig::DmFstatOutputthreshold = { ErrorOf( ES_CONFIG, 480, E_INFO, EV_NONE, 0 ),
R"(Threshold on number of files that fstat will process before sending output
to the client.
)"
};

ErrorId MsgConfig::DmFstatMaxcontent = { ErrorOf( ES_CONFIG, 56, E_INFO, EV_NONE, 0 ),
R"(The maximum number of bytes of an file revision that '%' p4 fstat -g'%' will
display.
)"
};

ErrorId MsgConfig::DmGraphEnabled = { ErrorOf( ES_CONFIG, 57, E_INFO, EV_NONE, 0 ),
R"(If set to 0, disables all graph depot functionality.
)"
};

ErrorId MsgConfig::DmGrepMaxlinelength = { ErrorOf( ES_CONFIG, 58, E_INFO, EV_NONE, 0 ),
R"(The maximum length of a line to be considered by '%'p4 grep'%'.
)"
};

ErrorId MsgConfig::DmGrepMaxrevs = { ErrorOf( ES_CONFIG, 59, E_INFO, EV_NONE, 0 ),
R"(The maximum number of file revisions '%'p4 grep'%' may process in one command.
)"
};

ErrorId MsgConfig::DmGrepMaxcontext = { ErrorOf( ES_CONFIG, 60, E_INFO, EV_NONE, 0 ),
R"(The maximum size of a file to be considered by '%'p4 grep'%'.
)"
};

ErrorId MsgConfig::DmInfoHide = { ErrorOf( ES_CONFIG, 61, E_INFO, EV_NONE, 0 ),
R"(When set, the '%'p4 info'%' command hides the sensitive info such as server name,
address, license information, etc.
)"
};

ErrorId MsgConfig::DmIntegEngine = { ErrorOf( ES_CONFIG, 62, E_INFO, EV_NONE, 0 ),
R"(The default algorithm to be used by '%'p4 integ'%' and '%'p4 merge'%'.
)"
};

ErrorId MsgConfig::DmIntegMaxact = { ErrorOf( ES_CONFIG, 63, E_INFO, EV_NONE, 0 ),
R"(Maximum number of individual integration records that will be considered during
base selection (per file being integrated).
)"
};

ErrorId MsgConfig::DmIntegMaxbranch = { ErrorOf( ES_CONFIG, 64, E_INFO, EV_NONE, 0 ),
R"(Maximum number of unique file paths that will be considered during base
selection (per file being integrated).
)"
};

ErrorId MsgConfig::DmIntegStreamAllowDepotFilespec = { ErrorOf( ES_CONFIG, 492, E_INFO, EV_NONE, 0 ),
R"(If set to 1, removes requirement for force flag (-F) when integrating
between streams using filespecs with depot syntax. The default is 0.
)"
};
ErrorId MsgConfig::DmIntegStreamspec = { ErrorOf( ES_CONFIG, 65, E_INFO, EV_NONE, 0 ),
R"(Controls the behavior of stream spec integration:
	0: Stream spec integration is not allowed
	1: Allow stream spec integration only for target streams with
	   '%'ParentViews'%' set to '%'noinherit'%'.
	2: Allow stream spec integration for all streams
)"
};

ErrorId MsgConfig::DmIntegTweaks = { ErrorOf( ES_CONFIG, 66, E_INFO, EV_NONE, 0 ),
R"(Combinable bitwise flags.

Bits 1-8 modify the behavior of the '%'p4 integrate'%' base selection engine in
ways that are not generally recommended, but which have been specifically
proposed or requested by individual customers. Refer to
'%'http://workshop.perforce.com/files/guest/sam_stafford/doc/Base%20Picking.pdf'%'
for necessary context on the base selection engine's data model.

	1: All '%'copy from'%' records are treated as '%'merge from'%' records.
	2: All '%'ignored'%' records are treated as '%'merge from'%' records.
	4: '%'copy from'%' records are treated as if they had been convergent
	   merges (setting the data model of the target of the copy to be the
	   union of the source and target, rather than reflecting the actual
	   content, which is a copy of the source).
	8: The base selection engine will always attempt to produce a
	   convergent merge (creating a union of the source and target) rather
	   than preserving divergence introduced by prior '%'ignored'%'
	   records.

Bit 16 does not modify base selection, but prevents '%'p4 integrate'%' from
scheduling any resolves other than content resolves. This allows the use of
the standard base selection engine while disabling the ability to resolve
changes other than content (e.g. branch-vs-edit, edit-vs-delete, moves,
filetypes, etc).

Bit 32 enables safeguards that prevent '%'move'%' actions from being partially
overwritten/corrupted as a side effect of a '%'p4 integrate'%' when move
actions collide with other actions in the same change. In release 2020.2 and
earlier these safeguards were always enabled; in release 2021.1 and later,
collisions are allowed by default.
)"
};

ErrorId MsgConfig::DmIntegUndo = { ErrorOf( ES_CONFIG, 67, E_INFO, EV_NONE, 0 ),
R"(If set to 1, modifies the '%'p4 integrate'%' base selection engine to treat
changes that were undone (via '%'p4 undo'%') as never having occurred, i.e.
excluding the associated integration records from consideration and allowing
the undone change to be re-integrated. By default, the result of an '%'undo'%'
is treated like any other change.
)"
};

ErrorId MsgConfig::DmIntegMultipleStreams = { ErrorOf( ES_CONFIG, 68, E_INFO, EV_NONE, 0 ),
R"(When set to 1, '%'p4 integ'%' may open files for integ across multiple streams,
excluding task streams.
)"
};

ErrorId MsgConfig::DmIsalive = { ErrorOf( ES_CONFIG, 69, E_INFO, EV_NONE, 0 ),
R"(Loop count interval for checking keepalives.
)"
};

ErrorId MsgConfig::DmKeysHide = { ErrorOf( ES_CONFIG, 70, E_INFO, EV_NONE, 0 ),
R"(Controls the '%'p4 key'%' and '%'p4 keys'%' access permissions:
	0: Default.
	1: '%'p4 keys'%' requires '%'admin'%' permissions rather than '%'list'%'.
	2: '%'p4 keys'%' and '%'p4 key'%' requires '%'admin'%' permissions.
)"
};

ErrorId MsgConfig::DmLabelsSearchAutoreload = { ErrorOf( ES_CONFIG, 71, E_INFO, EV_NONE, 0 ),
R"(Controls the behavior of '%'p4 labels filespec...'%' in respect to '%'autoreload'%'
labels:
	0: '%'autoreload'%' labels are not searched for the specified files.
	1: '%'autoreload'%' labels are only searched if the '%'-f'%' if provided.
	2: '%'autoreload'%' labels are always searched.
)"
};

ErrorId MsgConfig::DmLockBatch = { ErrorOf( ES_CONFIG, 72, E_INFO, EV_NONE, 0 ),
R"(Maximum number of files to be locked per journal transaction.
)"
};

ErrorId MsgConfig::DmLocksExclBatchNet = { ErrorOf( ES_CONFIG, 73, E_INFO, EV_NONE, 0 ),
R"(Batch size of files being exclusively locked/unlocked on an edge server to be
sent to the commit server.
)"
};

ErrorId MsgConfig::DmLocksGlobalBatchNet = { ErrorOf( ES_CONFIG, 74, E_INFO, EV_NONE, 0 ),
R"(Batch size of files being globally locked/unlocked on an edge server to be sent
to the commit server.
)"
};

ErrorId MsgConfig::DmLocksGlobalResultBatchNet = { ErrorOf( ES_CONFIG, 75, E_INFO, EV_NONE, 0 ),
R"(Batch size of results of files being globally locked/unlocked on an edge server
to be sent back to the edge server from the commit server.
)"
};

ErrorId MsgConfig::DmMaxkey = { ErrorOf( ES_CONFIG, 76, E_INFO, EV_NONE, 0 ),
R"(Maximum length of a database key value.
)"
};

ErrorId MsgConfig::DmObliterateSkiphave = { ErrorOf( ES_CONFIG, 527, E_INFO, EV_NONE, 0 ),
R"(Skip the removal of have records by default when obliterating files. This can
be overridden with the '%'-H'%' flag.
)"
};

ErrorId MsgConfig::DmObliterateSkiplabel = { ErrorOf( ES_CONFIG, 528, E_INFO, EV_NONE, 0 ),
R"(Skip the removal of label records by default when obliterating files. This can
be overridden with the '%'-L'%' flag.
)"
};

ErrorId MsgConfig::DmOpenShowGloballocks = { ErrorOf( ES_CONFIG, 77, E_INFO, EV_NONE, 0 ),
R"(If enabled, edge servers will report other instances of a file being open on the
commit server when a file is opened on the edge server. This requires the edge
server to query the commit server and may affect performance.
)"
};

ErrorId MsgConfig::DmPasswordMinlength = { ErrorOf( ES_CONFIG, 78, E_INFO, EV_NONE, 0 ),
R"(The minimum password length required at security level 2 and greater.
)"
};

ErrorId MsgConfig::DmPopulateSkipkeyed = { ErrorOf( ES_CONFIG, 79, E_INFO, EV_NONE, 0 ),
R"(When set to 1, '%'p4 populate'%' will skip the recalculation of digests for '%'+k'%'
type files. These may be recalculated later with '%'p4 verify -u'%'.
)"
};

ErrorId MsgConfig::DmProtectsAllowAdmin = { ErrorOf( ES_CONFIG, 80, E_INFO, EV_NONE, 0 ),
R"(If enabled, users with '%'admin'%' protections may run '%'p4 protects -a|-u|-g|-s'%'.
)"
};

ErrorId MsgConfig::DmProtectsExclusioncheck = { ErrorOf( ES_CONFIG, 81, E_INFO, EV_NONE, 0 ),
R"(When enabled, users with protections that have been entirely removed by
exclusions will be treated as if they had no protections in the first place.
)"
};

ErrorId MsgConfig::DmProtectsHide = { ErrorOf( ES_CONFIG, 82, E_INFO, EV_NONE, 0 ),
R"(When enabled, do not show exclusions in the output of '%'p4 protects'%' for the
current user.
)"
};

ErrorId MsgConfig::DmProtectsStreamspec = { ErrorOf( ES_CONFIG, 83, E_INFO, EV_NONE, 0 ),
R"(When enabled, access to stream specs may be controlled with the '%'list'%',
'%'readstreamspec'%', '%'openstreamspec'%' and '%'writestreamspec'%' permissions in the
protections table.
)"
};

ErrorId MsgConfig::DmProxyProtects = { ErrorOf( ES_CONFIG, 84, E_INFO, EV_NONE, 0 ),
R"(When set to 1, the host field in the protections table may explicitly match
connections via intermediary servers by prefixing the IP with '%'proxy-'%'.
)"
};

ErrorId MsgConfig::DmQuickClients = { ErrorOf( ES_CONFIG, 85, E_INFO, EV_NONE, 0 ),
R"(Maximum number of clients domain records to cache when looking up working/have.
)"
};

ErrorId MsgConfig::DmQuickDomains = { ErrorOf( ES_CONFIG, 86, E_INFO, EV_NONE, 0 ),
R"(Maximum number of label domain records to cache when running '%'p4 labels'%' with
file spec arguments.
)"
};

ErrorId MsgConfig::DmQuickHave = { ErrorOf( ES_CONFIG, 87, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::DmQuickInteg = { ErrorOf( ES_CONFIG, 88, E_INFO, EV_NONE, 0 ),
R"(Maximum number of rev records to cache when running '%'p4 integ'%' without lockless.
)"
};

ErrorId MsgConfig::DmQuickResolve = { ErrorOf( ES_CONFIG, 89, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::DmQuickRev = { ErrorOf( ES_CONFIG, 90, E_INFO, EV_NONE, 0 ),
R"(Maximum number of rev records to cache when running '%'p4 labels'%' with file spec
arguments.
)"
};

ErrorId MsgConfig::DmQuickWorking = { ErrorOf( ES_CONFIG, 91, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::DmRenameuserScanthresh = { ErrorOf( ES_CONFIG, 482, E_INFO, EV_NONE, 0 ),
R"('%'p4 renameuser'%' will perform a direct scan of db.rev if the user being
renamed submitted more than this percentage of submitted changes on the server.
Otherwise, an optimized lookup via db.revcx will be used.
)"
};

ErrorId MsgConfig::DmReopenBatch = { ErrorOf( ES_CONFIG, 526, E_INFO, EV_NONE, 0 ),
R"(Maximum number of files to be reopened per journal transaction.
)"
};

ErrorId MsgConfig::DmRepoNoautocreate = { ErrorOf( ES_CONFIG, 92, E_INFO, EV_NONE, 0 ),
R"(If set to 1, pushing a non-existent repo into graph depot will be blocked.
)"
};

ErrorId MsgConfig::DmRepoUnpack = { ErrorOf( ES_CONFIG, 93, E_INFO, EV_NONE, 0 ),
R"(Controls graph depot unpacking of pack files into loose objects:
	0: Don't unpack pack files
	1: Unpack pack files except for the initial pack file pushed to a repo
	2: Unpack all pack files
)"
};

ErrorId MsgConfig::DmResolveAttrib = { ErrorOf( ES_CONFIG, 94, E_INFO, EV_NONE, 0 ),
R"(When set to 1, '%'p4 integ'%' will schedule the resolve of attributes as well as
file content.
)"
};

ErrorId MsgConfig::DmResolveIgnoredeleted = { ErrorOf( ES_CONFIG, 95, E_INFO, EV_NONE, 0 ),
R"(When set to 1, integrations into deleted files will be resolved as '%'ignored'%'
by '%'p4 resolve -as'%'.
)"
};

ErrorId MsgConfig::DmRevScanThresh = { ErrorOf( ES_CONFIG, 96, E_INFO, EV_NONE, 0 ),
R"(Number of successive revisions of the same file to scan in '%'db.rev'%' before
repositioning to the next file.
)"
};

ErrorId MsgConfig::DmRevcxThresh1 = { ErrorOf( ES_CONFIG, 97, E_INFO, EV_NONE, 0 ),
R"(Wide path optimisation when using @change rev specifiers: number of non-matching
records scanned from '%'db.revcx'%' before just using '%'db.rev'%' instead,
unless '%'dm.revcx.scan.thresh2'%' matches are found first.
)"
};

ErrorId MsgConfig::DmRevcxThresh2 = { ErrorOf( ES_CONFIG, 98, E_INFO, EV_NONE, 0 ),
R"(Wide path optimisation when using @change rev specifiers: number of matching
records scanned from '%'db.revcx'%' before disabling the fallback to '%'db.rev'%'.
)"
};

ErrorId MsgConfig::DmRevertBatch = { ErrorOf( ES_CONFIG, 99, E_INFO, EV_NONE, 0 ),
R"(Maximum number of files to be reverted per journal transaction.
)"
};

ErrorId MsgConfig::DmRotatelogwithjnl = { ErrorOf( ES_CONFIG, 100, E_INFO, EV_NONE, 0 ),
R"(When enabled, structured logs are rotated when the journal is rotated.
)"
};

ErrorId MsgConfig::DmHavePreload = { ErrorOf( ES_CONFIG, 508, E_INFO, EV_NONE, 0 ),
R"(When disabled, reduce memory usage on syncs.
)"
};

ErrorId MsgConfig::DmShelveAccessupdate = { ErrorOf( ES_CONFIG, 101, E_INFO, EV_NONE, 0 ),
R"(Time interval to update the shelve access time.
)"
};

ErrorId MsgConfig::DmShelveMaxfiles = { ErrorOf( ES_CONFIG, 102, E_INFO, EV_NONE, 0 ),
R"(Maximum number of files that can be shelved with 'p4 shelve'.
)"
};

ErrorId MsgConfig::DmShelveMaxsize = { ErrorOf( ES_CONFIG, 103, E_INFO, EV_NONE, 0 ),
R"(Limit size of a file that can be shelved.
)"
};

ErrorId MsgConfig::DmShelvePromote = { ErrorOf( ES_CONFIG, 104, E_INFO, EV_NONE, 0 ),
R"(Automatically promote shelved files to the commit server from an edge server.
)"
};

ErrorId MsgConfig::DmStatusMatchlines = { ErrorOf( ES_CONFIG, 105, E_INFO, EV_NONE, 0 ),
R"(Minimum percentage of lines that must match for '%'p4 reconcile'%' or
'%'p4 status'%' to consider deleted+added file pairs to be a single '%'move'%'
action. If this is set to 100, digest matching is used rather than a textual
diff.
)"
};

ErrorId MsgConfig::DmStatusMatchsize = { ErrorOf( ES_CONFIG, 106, E_INFO, EV_NONE, 0 ),
R"(Approximate maximum percentage difference in filesize for deleted+added file
pairs to be compared as possible '%'move'%' candidates according to
'%'dm.status.matchlines'%'. (There is an additional minor allowance to account
for possible line ending differences.)
)"
};

ErrorId MsgConfig::DmStreamParentview = { ErrorOf( ES_CONFIG, 107, E_INFO, EV_NONE, 0 ),
R"(Controls the default ParentView value for new streams:
	0: Always '%'inherit'%'
	1: Use '%'noinherit'%' for all stream types that support that mode
	2: Use '%'noinherit'%' only for '%'release'%' streams
)"
};

ErrorId MsgConfig::DmStreamComponents = { ErrorOf( ES_CONFIG, 108, E_INFO, EV_NONE, 0 ),
R"(Enables stream components functionality.
)"
};

ErrorId MsgConfig::DmStreamSparseBranchMax = { ErrorOf( ES_CONFIG, 434, E_INFO, EV_NONE, 0 ),
R"(Maximum number of depot files in the branch view of a sparse stream.
	0: No limit
	N: Limit the number of files to N
)"
};

ErrorId MsgConfig::DmSyncStreamChange = { ErrorOf( ES_CONFIG, 109, E_INFO, EV_NONE, 0 ),
R"(If enabled, '%'p4 sync --use-stream-change'%' is implicit: syncing a stream client
to a specific changelist will also apply the stream's view at that change.
)"
};

ErrorId MsgConfig::DmSubprotectsGrantAdmin = { ErrorOf( ES_CONFIG, 110, E_INFO, EV_NONE, 0 ),
R"(When enabled, users granted '%'owner'%' permission in the protections table
are implicitly granted '%'admin'%' permission on the same path.
)"
};

ErrorId MsgConfig::DmTopologyLastseenupdate = { ErrorOf( ES_CONFIG, 111, E_INFO, EV_NONE, 0 ),
R"(Time interval to update the '%'lastSeenDate'%' field of the topology record.
)"
};

ErrorId MsgConfig::DmUserAccessupdate = { ErrorOf( ES_CONFIG, 112, E_INFO, EV_NONE, 0 ),
R"(Time interval to update user access time, if a lock can be taken.
)"
};

ErrorId MsgConfig::DmUserAccessforce = { ErrorOf( ES_CONFIG, 113, E_INFO, EV_NONE, 0 ),
R"(Time interval to force the user access time update.
)"
};

ErrorId MsgConfig::DmUserAccessmode = { ErrorOf( ES_CONFIG, 505, E_INFO, EV_NONE, 0),
R"(Conditions for updating user access times:
	0: Access times are only updated on command start
	1: Access times for users with IdleTimeouts are updated periodically
	   during and at the end of a command
	2: All user's access times are periodically updated
	3: As above, but including service type users too
)"
};

ErrorId MsgConfig::DmUserAllowselfupdate = { ErrorOf( ES_CONFIG, 114, E_INFO, EV_NONE, 0 ),
R"(Allowing the Users to update their email and fullname.
)"
};

ErrorId MsgConfig::DmUserAutoprotections = { ErrorOf( ES_CONFIG, 533, E_INFO, EV_NONE, 0 ),
R"(The first user automatically populates the protections table with the defaults.
)"
};

ErrorId MsgConfig::DmUserHideinvalid = { ErrorOf(ES_CONFIG, 478, E_INFO, EV_NONE, 0),
R"(Hide invalid user error on authentication failure.
)"
};

ErrorId MsgConfig::DmUserInsecurelogin = { ErrorOf( ES_CONFIG, 115, E_INFO, EV_NONE, 0 ),
R"(Issue login tickets that work on all hosts.
)"
};

ErrorId MsgConfig::DmUserLoginattempts = { ErrorOf( ES_CONFIG, 116, E_INFO, EV_NONE, 0 ),
R"(A delay is added for a password attempt when the number of consecutive failed
login attempts equal this value.
)"
};

ErrorId MsgConfig::DmUserNoautocreate = { ErrorOf( ES_CONFIG, 117, E_INFO, EV_NONE, 0 ),
R"(Control the behavior of automatic user creation:
	0: User created by running a command updating depot or metadata
	1: User created by the '%'p4 user'%' command
	2: User created by a super user running '%'p4 user -f'%' command
)"
};

ErrorId MsgConfig::DmUserNumeric = { ErrorOf( ES_CONFIG, 118, E_INFO, EV_NONE, 0 ),
R"(When enabled, allows purely numeric usernames.
)"
};

ErrorId MsgConfig::DmUserResetpassword = { ErrorOf( ES_CONFIG, 119, E_INFO, EV_NONE, 0 ),
R"(New user requires password reset.
)"
};

ErrorId MsgConfig::DmUserSetinitialpasswd = { ErrorOf( ES_CONFIG, 120, E_INFO, EV_NONE, 0 ),
R"(If set to 0, only super users can set user's initial password instead of the
user being prompted to set their password before their first authentication.
)"
};

ErrorId MsgConfig::FilesysAtomicRename = { ErrorOf( ES_CONFIG, 121, E_INFO, EV_NONE, 0 ),
R"(Enables atomic rename semantics for the journal file on Windows.
)"
};

ErrorId MsgConfig::FilesysBinaryscan = { ErrorOf( ES_CONFIG, 122, E_INFO, EV_NONE, 0 ),
R"(The maximum number of bytes of a file to check for non-textual content when
performing filetype detection.
)"
};

ErrorId MsgConfig::FilesysBufsize = { ErrorOf( ES_CONFIG, 123, E_INFO, EV_NONE, 0 ),
R"(Default buffer size for file IO.
)"
};

ErrorId MsgConfig::FilesysCachehint = { ErrorOf( ES_CONFIG, 124, E_INFO, EV_NONE, 0 ),
R"(If set to 1 on Linux, attempt to free file cache on file close.
)"
};

ErrorId MsgConfig::FilesysChecklinks = { ErrorOf( ES_CONFIG, 125, E_INFO, EV_NONE, 0 ),
R"(Symlink checking mode:
	0: No checking
	1: Don't allow adds of symlinks to directories or
	   files inside a symlink that's within the client workspace
	2: No checking if the force flag is passed
	3: No checking if the force flag is passed
	4: Don't allow adds of files inside a symlink that's within the client
	   workspace
)"
};

ErrorId MsgConfig::FilesysDetectunicode = { ErrorOf( ES_CONFIG, 126, E_INFO, EV_NONE, 0 ),
R"(If enabled and a file has high ASCII characters try and detect as unicode
)"
};

ErrorId MsgConfig::FilesysDetectutf8 = { ErrorOf( ES_CONFIG, 127, E_INFO, EV_NONE, 0 ),
R"(UTF8 filetype detection:
	0: Off
	1: If file has high ASCII characters and is valid UTF8, detect as '%'utf8'%'
	   before falling back to P4CHARSET to detect as unicode
	2: Only detect as '%'utf8'%' if the file includes a UTF8 BOM
)"
};

ErrorId MsgConfig::FilesysLockdelay = { ErrorOf( ES_CONFIG, 128, E_INFO, EV_NONE, 0 ),
R"(If a .lck file was last modified longer ago that this number of milliseconds
attempt to delete it in case the owner has gone but left the file.
)"
};

ErrorId MsgConfig::FilesysLocktimeout = { ErrorOf( ES_CONFIG, 129, E_INFO, EV_NONE, 0 ),
R"(Lock acquisition timeout before invoking long lock functionality.
)"
};

ErrorId MsgConfig::FilesysLocktry = { ErrorOf( ES_CONFIG, 130, E_INFO, EV_NONE, 0 ),
R"(Number of attempts to create a .lck file.
)"
};

ErrorId MsgConfig::FilesysMaketmp = { ErrorOf( ES_CONFIG, 131, E_INFO, EV_NONE, 0 ),
R"(Maximum number of attempts to create a unique temp file.
)"
};

ErrorId MsgConfig::FilesysMaxmap = { ErrorOf( ES_CONFIG, 132, E_INFO, EV_NONE, 0 ),
R"(Maximum size of file to read directly into memory via memory mapping.
)"
};

ErrorId MsgConfig::FilesysMaxsymlink = { ErrorOf( ES_CONFIG, 133, E_INFO, EV_NONE, 0 ),
R"(Maximum length of a symlink target.
)"
};

ErrorId MsgConfig::FilesysMaxtmp = { ErrorOf( ES_CONFIG, 134, E_INFO, EV_NONE, 0 ),
R"(Maximum number to append to a temporary file's filename.
)"
};

ErrorId MsgConfig::FilesysRestrictsymlinks = { ErrorOf( ES_CONFIG, 135, E_INFO, EV_NONE, 0 ),
R"(When enabled, and '%'P4CLIENTROOT'%' or DVCS are in use, symlinks to locations
outside the client root cannot be created by the client.
)"
};

ErrorId MsgConfig::FilesysSymlinkTypemap = { ErrorOf( ES_CONFIG, 536, E_INFO, EV_NONE, 0 ),
R"(Controls whether typemap entries can override symlink file type detection
during 'p4 add':
	0: Symlinks added as 'symlink' regardless of typemap (default)
	1: Typemap entries can override symlink type detection (legacy behavior)
)"
};

ErrorId MsgConfig::FilesysUtf8Bom = { ErrorOf( ES_CONFIG, 136, E_INFO, EV_NONE, 0 ),
R"(Controls whether '%'utf8'%' type files should be sync'ed with a BOM:
	0: No BOM
	1: Always write the BOM
	2: Only write the BOM on Windows
)"
};

ErrorId MsgConfig::FilesysExtendlowmark = { ErrorOf( ES_CONFIG, 137, E_INFO, EV_NONE, 0 ),
R"(The mimimum filesize for the server to instruct the client to preallocate space
for a file, avoiding defragmentation on some filesystems.
)"
};

ErrorId MsgConfig::FilesysWindowsLfn = { ErrorOf( ES_CONFIG, 138, E_INFO, EV_NONE, 0 ),
R"(Enables long filename support on Windows:
	0: Off
	1: Paths longer than 255 characters use LFN
	10: All paths use LFN
)"
};

ErrorId MsgConfig::FilesysClientNullsync = { ErrorOf( ES_CONFIG, 139, E_INFO, EV_NONE, 0 ),
R"(When set to 1, the client will discard file content sent from the server.
)"
};

ErrorId MsgConfig::IndexDomainOwner = { ErrorOf( ES_CONFIG, 140, E_INFO, EV_NONE, 0 ),
R"(When enabled, the owner of clients/branches/labels/streams are indexed for
faster lookup by owner.
)"
};

ErrorId MsgConfig::LbrAutocompress = { ErrorOf( ES_CONFIG, 141, E_INFO, EV_NONE, 0 ),
R"(Specifically all newly added text files defaults to text+C (compressed) type
rather than text+D (delta) type.
)"
};

ErrorId MsgConfig::LbrBufsize = { ErrorOf( ES_CONFIG, 142, E_INFO, EV_NONE, 0 ),
R"(Archive file I/O buffer size.
)"
};

ErrorId MsgConfig::LbrFabricate = { ErrorOf( ES_CONFIG, 143, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::LbrProxyCase = { ErrorOf( ES_CONFIG, 144, E_INFO, EV_NONE, 0 ),
R"(The case sensitivity of the proxy server's cache:
	1: Case insensitive
	2: Use the upstream server's case sensitivity
	3: Case sensitive
)"
};

ErrorId MsgConfig::LbrRcsExistcheck = { ErrorOf( ES_CONFIG, 145, E_INFO, EV_NONE, 0 ),
R"(Perform RCS validation check during submit.
)"
};

ErrorId MsgConfig::LbrRcsMaxlen = { ErrorOf( ES_CONFIG, 146, E_INFO, EV_NONE, 0 ),
R"(Maximum line length in a '%'+k'%' type file to search for keyword placeholders.
)"
};

ErrorId MsgConfig::LbrReplicaNotransfer = { ErrorOf( ES_CONFIG, 147, E_INFO, EV_NONE, 0 ),
R"(Suppress on-demand file fetch.
)"
};

ErrorId MsgConfig::LbrRetryMax = { ErrorOf( ES_CONFIG, 148, E_INFO, EV_NONE, 0 ),
R"(Number of times a replica should retry a failed archive transfer.
)"
};

ErrorId MsgConfig::LbrS3UploadRetries = { ErrorOf( ES_CONFIG, 494, E_INFO, EV_NONE, 0 ),
R"(Number of times an upload to S3 will be retried.
)"
};

ErrorId MsgConfig::LbrStatInterval = { ErrorOf( ES_CONFIG, 149, E_INFO, EV_NONE, 0 ),
R"(Proxy file status interval. See '%'p4p -h'%'.
)"
};

ErrorId MsgConfig::LbrVerifyIn = { ErrorOf( ES_CONFIG, 150, E_INFO, EV_NONE, 0 ),
R"(If set to 1, the digest of file content sent from the client to server is
checked on transfer.
)"
};

ErrorId MsgConfig::LbrVerifyOut = { ErrorOf( ES_CONFIG, 151, E_INFO, EV_NONE, 0 ),
R"(If set to 1, the digest of file content sent from the server to client is
checked on transfer.
)"
};

ErrorId MsgConfig::LbrVerifyScriptOut = { ErrorOf( ES_CONFIG, 152, E_INFO, EV_NONE, 0 ),
R"(If set to 1, the digest of file content for files of try '%'+X'%' sent from the
server to client is checked on transfer.
)"
};

ErrorId MsgConfig::LbrStorageDelay = { ErrorOf( ES_CONFIG, 153, E_INFO, EV_NONE, 0 ),
R"(The required age of a zero reference storage record that permits the record's
archive to be removed.
)"
};

ErrorId MsgConfig::LbrStorageAllowsymlink = { ErrorOf( ES_CONFIG, 154, E_INFO, EV_NONE, 0 ),
R"(Controls if symbolic links are permitted in a depot by the orphan scanner:
	0: symbolic links are not permitted and will stop the orphan scanner
	   with an error
	1: symbolic links are permitted and followed
)"
};

ErrorId MsgConfig::LbrStorageSkipkeyed = { ErrorOf( ES_CONFIG, 155, E_INFO, EV_NONE, 0 ),
R"(Controls the action of the storage upgrade process when it detects a keyed
revision:
	0: Compute the digest of the revision
	1: Skip the computation of the digest and log the revision that
	   was skipped
	2: Skip the computation of the digest and do not log the revision
)"
};

ErrorId MsgConfig::LbrStorageThreads = { ErrorOf( ES_CONFIG, 156, E_INFO, EV_NONE, 0 ),
R"(Controls the number of threads to use in a '%'p4 storage -U'%' request.
)"
};

ErrorId MsgConfig::LbrRcsLocking = { ErrorOf( ES_CONFIG, 157, E_INFO, EV_NONE, 0 ),
R"(When enabled, create lockfiles for RCS archives to serialise updates.
)"
};

ErrorId MsgConfig::LbrUnloaddepotCompress = { ErrorOf( ES_CONFIG, 513, E_INFO, EV_NONE, 0 ),
R"(When enabled, write unload-depot in compressed mode.
)"
};

ErrorId MsgConfig::LogGroupMaxlen = { ErrorOf( ES_CONFIG, 158, E_INFO, EV_NONE, 0 ),
R"(Maximum length of command group to be recorded in the structured logs.
)"
};

ErrorId MsgConfig::LogOriginhost = { ErrorOf( ES_CONFIG, 159, E_INFO, EV_NONE, 0 ),
R"(When enabled, the IP of the first intermediary is logged in addition to the
client's IP address.
)"
};

ErrorId MsgConfig::MapJoinmax1 = { ErrorOf( ES_CONFIG, 160, E_INFO, EV_NONE, 0 ),
R"(The maximum number of lines by which a joined map may exceed the sum of its
input maps. Increasing this threshold beyond its default may result in server
instability.
)"
};

ErrorId MsgConfig::MapJoinmax2 = { ErrorOf( ES_CONFIG, 161, E_INFO, EV_NONE, 0 ),
R"(The absolute maximum number of lines in a joined map. Increasing this threshold
beyond its default may result in server instability.
)"
};

ErrorId MsgConfig::MapLimitMaxlookback = { ErrorOf( ES_CONFIG, 477, E_INFO, EV_NONE, 0 ),
R"(The maximum number of limit-map entries to check when searching for a duplicate.
)"
};

ErrorId MsgConfig::MapMaxwild = { ErrorOf( ES_CONFIG, 162, E_INFO, EV_NONE, 0 ),
R"(The maximum number of wildcards within a single mapping expression. Increasing
this threshold beyond its default may result in server instability.
)"
};

ErrorId MsgConfig::MapOverlayLegacy = { ErrorOf( ES_CONFIG, 163, E_INFO, EV_NONE, 0 ),
R"(If set to 1, revert overlay client views to their 'legacy' pre-2023.1 behavior.
The pre-2023.1 behavior caused a number of confusing and often wrong results,
so setting this configurable to a non-zero value is not recommended.
Non-zero values for map.overlay.legacy are also incompatible with the
Sparse Stream feature.
)"
};

ErrorId MsgConfig::MergeDlEndeol = { ErrorOf( ES_CONFIG, 164, E_INFO, EV_NONE, 0 ),
R"(Control merge handling of EOF in -dl mode:
	0: the last two lines of the merge result may be joined, but a conflict
	   will not arise.
	1: the last two lines of the merge result will not be joined, but a
	   conflict may arise.
)"
};

ErrorId MsgConfig::NetAutotune = { ErrorOf( ES_CONFIG, 165, E_INFO, EV_NONE, 0 ),
R"(Let operating system determine optimal socket buffer sizes rather than
setting them explicitly with value of '%'net.tcpsize'%'.
)"
};

ErrorId MsgConfig::NetBufsize = { ErrorOf( ES_CONFIG, 166, E_INFO, EV_NONE, 0 ),
R"(Network buffer size for sends.
)"
};

ErrorId MsgConfig::NetDeltaRplMinsize = { ErrorOf( ES_CONFIG, 506, E_INFO, EV_NONE, 0 ),
R"(Minimum file size to perform replica delta transfer. Must be non-zero on
both upstream and downstream servers to activate delta transfer. Upstream
server values are used for replica delta transfer.
)"
};

ErrorId MsgConfig::NetDeltaRplThreshold = { ErrorOf( ES_CONFIG, 507, E_INFO, EV_NONE, 0 ),
R"(Maximum percentage of file size to perform replica delta transfer before
reverting to streaming the complete file. Must be non-zero on both upstream
and downstream servers to activate delta transfer. Upstream server values are
used for replica delta transfer.
)"
};

ErrorId MsgConfig::NetDeltaTransferMinsize = { ErrorOf( ES_CONFIG, 487, E_INFO, EV_NONE, 0 ),
R"(Minimum file size to perform a delta transfer between server and client.
A value of 0 disables delta transfers.
)"
};

ErrorId MsgConfig::NetDeltaTransferThreshold = { ErrorOf( ES_CONFIG, 489, E_INFO, EV_NONE, 0 ),
R"(Maximum percentage of file size to perform a delta transfer between
server and client before reverting to streaming the complete file. A value
of 0 disables delta transfers.
)"
};

ErrorId MsgConfig::NetKeepaliveDisable = { ErrorOf( ES_CONFIG, 167, E_INFO, EV_NONE, 0 ),
R"(If 0 and keepalive functionality is supported by the OS, keepalives are enabled
on the socket. If 1, keepalives are disabled on the socket.
)"
};

ErrorId MsgConfig::NetKeepaliveIdle = { ErrorOf( ES_CONFIG, 168, E_INFO, EV_NONE, 0 ),
R"(Idle time (in seconds) before starting to send keepalives.
)"
};

ErrorId MsgConfig::NetKeepaliveInterval = { ErrorOf( ES_CONFIG, 169, E_INFO, EV_NONE, 0 ),
R"(Interval (in seconds) between sending keepalive packets.
)"
};

ErrorId MsgConfig::NetKeepaliveCount = { ErrorOf( ES_CONFIG, 170, E_INFO, EV_NONE, 0 ),
R"(Number of unacknowledged keepalives before failure.
)"
};

ErrorId MsgConfig::NetHeartbeatInterval = { ErrorOf( ES_CONFIG, 171, E_INFO, EV_NONE, 0 ),
R"(Interval in milliseconds used by '%'p4 heartbeat'%' to send heartbeats.
)"
};

ErrorId MsgConfig::NetHeartbeatWait = { ErrorOf( ES_CONFIG, 172, E_INFO, EV_NONE, 0 ),
R"(Wait time in milliseconds for response from '%'p4 heartbeat'%' request.
)"
};

ErrorId MsgConfig::NetHeartbeatMissingInterval = { ErrorOf( ES_CONFIG, 173, E_INFO, EV_NONE, 0 ),
R"(Wait time in milliseconds before resuming heartbeats after '%'p4 heartbeat'%'
has detected a missing response.
)"
};

ErrorId MsgConfig::NetHeartbeatMissingWait = { ErrorOf( ES_CONFIG, 174, E_INFO, EV_NONE, 0 ),
R"(Wait time in milliseconds for response from '%'p4 heartbeat'%' after previous
'%'p4 heartbeat'%' request has detected a missed response.
)"
};

ErrorId MsgConfig::NetHeartbeatMissingCount = { ErrorOf( ES_CONFIG, 175, E_INFO, EV_NONE, 0 ),
R"(Number of consecutive missing heartbeats detected by '%'p4 heartbeat'%' to
determine that the target is dead.
)"
};

ErrorId MsgConfig::NetMaxFaultPub = { ErrorOf( ES_CONFIG, 176, E_INFO, EV_NONE, 0 ),
R"(A value in megabytes that controls the proxy's cache faulting behavior.
A single '%'p4 sync'%' will not publish more than '%'net.maxfaultpub'%'
megabytes of faults into '%'pdb.lbr'%'.
)"
};

ErrorId MsgConfig::NetMaxclosewait = { ErrorOf( ES_CONFIG, 177, E_INFO, EV_NONE, 0 ),
R"(Milliseconds to wait for a network close.
)"
};

ErrorId MsgConfig::NetMaxwait = { ErrorOf( ES_CONFIG, 178, E_INFO, EV_NONE, 0 ),
R"(Time, in seconds, before a network connection times out and is closed.
)"
};

ErrorId MsgConfig::NetNagle = { ErrorOf( ES_CONFIG, 509, E_INFO, EV_NONE, 0 ),
R"(Enable the Nagle algorithm (ie, clear %'TCP_NODELAY'% socket option) if
'%'net.nagle=1'%'.
)"
};

ErrorId MsgConfig::NetParallelMax = { ErrorOf( ES_CONFIG, 179, E_INFO, EV_NONE, 0 ),
R"(The maximum number of concurrent threads for '%'p4 sync'%', '%'p4 shelve'%',
and '%'p4 submit'%'. A value greater than 1 enables parallel processing up to
the specified number of threads.
)"
};

ErrorId MsgConfig::NetParallelThreads = { ErrorOf( ES_CONFIG, 180, E_INFO, EV_NONE, 0 ),
R"(The default number of parallel threads for transferring files concurrently.
)"
};

ErrorId MsgConfig::NetParallelBatch = { ErrorOf( ES_CONFIG, 181, E_INFO, EV_NONE, 0 ),
R"(The number of files in a parallel thread batch.
)"
};

ErrorId MsgConfig::NetParallelBatchsize = { ErrorOf( ES_CONFIG, 182, E_INFO, EV_NONE, 0 ),
R"(The number of bytes in a parallel thread batch.
)"
};

ErrorId MsgConfig::NetParallelCheckAlive = { ErrorOf( ES_CONFIG, 532, E_INFO, EV_NONE, 0 ),
R"(Verify the controlling sync or submit operation behind a parallel transfer
has not exited.
)"
};

ErrorId MsgConfig::NetParallelMin = { ErrorOf( ES_CONFIG, 183, E_INFO, EV_NONE, 0 ),
R"(The minimum number of files in a parallel sync. A sync of fewer files
does not initiate parallel file transfers.
)"
};

ErrorId MsgConfig::NetParallelMinsize = { ErrorOf( ES_CONFIG, 184, E_INFO, EV_NONE, 0 ),
R"(The minimum number of bytes in a parallel sync. A sync that is too small
does not initiate parallel file transfers.
)"
};

ErrorId MsgConfig::NetParallelShelveThreads = { ErrorOf( ES_CONFIG, 185, E_INFO, EV_NONE, 0 ),
R"(Enables automatic parallel file transfer for '%'p4 shelve'%' and sets the
number of threads to this value.
)"
};

ErrorId MsgConfig::NetParallelShelveBatch = { ErrorOf( ES_CONFIG, 186, E_INFO, EV_NONE, 0 ),
R"(Number of files in a batch for automatic parallel file transfer for
'%'p4 shelve'%'.
)"
};

ErrorId MsgConfig::NetParallelShelveMin = { ErrorOf( ES_CONFIG, 187, E_INFO, EV_NONE, 0 ),
R"(Minimum number of files in a change to enable parallel file transfer for
'%'p4 shelve'%'.
)"
};

ErrorId MsgConfig::NetParallelSubmitThreads = { ErrorOf( ES_CONFIG, 188, E_INFO, EV_NONE, 0 ),
R"(Enables automatic parallel file transfer for '%'p4 submit'%' and sets the
number of threads to this value.
)"
};

ErrorId MsgConfig::NetParallelSubmitBatch = { ErrorOf( ES_CONFIG, 189, E_INFO, EV_NONE, 0 ),
R"(Number of files in a batch for automatic parallel file transfer for
'%'p4 submit'%'.
)"
};

ErrorId MsgConfig::NetParallelSubmitMin = { ErrorOf( ES_CONFIG, 190, E_INFO, EV_NONE, 0 ),
R"(Minimum number of files in a change to enable parallel file transfer for
'%'p4 submit'%'.
)"
};

ErrorId MsgConfig::NetParallelSyncSvrthreads = { ErrorOf( ES_CONFIG, 191, E_INFO, EV_NONE, 0 ),
R"(Server-wide parallel sync transmit thread limit.
)"
};

ErrorId MsgConfig::NetQuickAck = { ErrorOf( ES_CONFIG, 510, E_INFO, EV_NONE, 0 ),
R"(Don't delay ACKs (ie, set %'TCP_QUICKACK'% socket option) if
'%'net.quickack=1'%'.
)"
};

ErrorId MsgConfig::NetRcvbuflowmark = { ErrorOf( ES_CONFIG, 192, E_INFO, EV_NONE, 0 ),
R"(Used to manage dynamic '%'net.rcvbufsize'%' growth when autotuning.
)"
};

ErrorId MsgConfig::NetRcvbufmaxsize = { ErrorOf( ES_CONFIG, 193, E_INFO, EV_NONE, 0 ),
R"(Maximum buffer size for receives.
)"
};

ErrorId MsgConfig::NetRcvbufsize = { ErrorOf( ES_CONFIG, 194, E_INFO, EV_NONE, 0 ),
R"(Network buffer size for receives.
)"
};

ErrorId MsgConfig::NetReuseport = { ErrorOf( ES_CONFIG, 195, E_INFO, EV_NONE, 0 ),
R"(Set %'SO_REUSEPORT'% for listening socket.
)"
};

ErrorId MsgConfig::NetRfc3484 = { ErrorOf( ES_CONFIG, 196, E_INFO, EV_NONE, 0 ),
R"(Abide by RFC 3484 when neither IPv4 nor IPv6 explicitly specified.
)"
};

ErrorId MsgConfig::NetSendlimit = { ErrorOf( ES_CONFIG, 197, E_INFO, EV_NONE, 0 ),
R"(Data size limit used to help determine when to send.
)"
};

ErrorId MsgConfig::NetTcpsize = { ErrorOf( ES_CONFIG, 198, E_INFO, EV_NONE, 0 ),
R"(Set %'SO_SNDBUF'% and %'SO_RCVBUF'% socket options explicitly if '%'net.autotune=0'%'.
)"
};

ErrorId MsgConfig::NetBacklog = { ErrorOf( ES_CONFIG, 199, E_INFO, EV_NONE, 0 ),
R"(Maximum length of queue for pending connections. Sets '%'backlog'%' parameter
to the '%'listen()'%' system call.
)"
};

ErrorId MsgConfig::NetX3Minsize = { ErrorOf( ES_CONFIG, 200, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::ProxyDeliverFix = { ErrorOf( ES_CONFIG, 201, E_INFO, EV_NONE, 0 ),
R"(Enables a round-trip when delivering file content via proxies to avoid hangs
but at the cost of some performance.
)"
};

ErrorId MsgConfig::ProxyMonitorInterval = { ErrorOf( ES_CONFIG, 202, E_INFO, EV_NONE, 0 ),
R"(Proxy monitoring interval. See '%'p4p -h'%'.
)"
};

ErrorId MsgConfig::ProxyMonitorLevel = { ErrorOf( ES_CONFIG, 203, E_INFO, EV_NONE, 0 ),
R"(Proxy monitoring level. See '%'p4p -h'%'.
)"
};

ErrorId MsgConfig::ProxyClearcachethresh = { ErrorOf( ES_CONFIG, 204, E_INFO, EV_NONE, 0 ),
R"(The threshold in number of days for Proxy cache clearing, used by the command
'%'p4p --cache-purge'%'. A value of 0 means cache clearing is turned off.
)"
};

ErrorId MsgConfig::ProxySuppresswarnings = { ErrorOf( ES_CONFIG, 491, E_INFO, EV_NONE, 0 ),
R"(Used to suppress Proxy warning level errors to the client. A value of 0 means
errors are passed to the client. This does not impact error logging.
)"
};

ErrorId MsgConfig::RcsMaxinsert = { ErrorOf( ES_CONFIG, 205, E_INFO, EV_NONE, 0 ),
R"(Maximum number of lines that can be inserted to an RCS archive chunk.
)"
};

ErrorId MsgConfig::RcsNofsync = { ErrorOf( ES_CONFIG, 206, E_INFO, EV_NONE, 0 ),
R"(When set to 1, RCS files will be closed without a call to fsync.
)"
};

ErrorId MsgConfig::RmtAllowRemoteUser = { ErrorOf( ES_CONFIG, 511, E_INFO, EV_NONE, 0 ),
R"(When set to 1 and at security levels below 4, the 'remote' user may be used
for server to server connections (for example, remote depots).
)"
};

ErrorId MsgConfig::RmtEnforceServiceusertype = { ErrorOf( ES_CONFIG, 530, E_INFO, EV_NONE, 0 ),
R"(When set to 1, server to server commands that are authenticated by the
'serviceUser' check that the user is of type 'service'.
)"
};

ErrorId MsgConfig::RpcDelay = { ErrorOf( ES_CONFIG, 207, E_INFO, EV_NONE, 0 ),
R"(Artificial delay to add to each RPC send and receive in milliseconds.
)"
};

ErrorId MsgConfig::RpcDurablewait = { ErrorOf( ES_CONFIG, 208, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::RpcHimark = { ErrorOf( ES_CONFIG, 209, E_INFO, EV_NONE, 0 ),
R"(Initial RPC himark value - the size of the server's send and receive buffers
for this connection. If set, prevents automatic himark setting based on the
OS's reporting of the TCP stack's receive buffer.
)"
};

ErrorId MsgConfig::RpcLowmark = { ErrorOf( ES_CONFIG, 210, E_INFO, EV_NONE, 0 ),
R"(Initial RPC lomark value - the number of bytes sent before an RPC flush.
)"
};

ErrorId MsgConfig::RpcIpaddrMismatch = { ErrorOf( ES_CONFIG, 211, E_INFO, EV_NONE, 0 ),
R"(When set to 1, rejects commands that were forwarded by an intermediary but
differ in the end user's IP address.
)"
};

ErrorId MsgConfig::RplArchiveGraph = { ErrorOf( ES_CONFIG, 212, E_INFO, EV_NONE, 0 ),
R"(Controls replication of graph depot archives:
	0: Graph depot archives are not replicated
	1: Pack files and loose objects are scheduled for transfer
	2: As above and includes LFS archives
)"
};

ErrorId MsgConfig::RplAwaitjnlCount = { ErrorOf( ES_CONFIG, 213, E_INFO, EV_NONE, 0 ),
R"(Maximum number of times to check for journal growth on an upstream server before
returning no changes to a replica.
)"
};

ErrorId MsgConfig::RplAwaitjnlInterval = { ErrorOf( ES_CONFIG, 214, E_INFO, EV_NONE, 0 ),
R"(Interval between checking for journal growth on an upstream server.
)"
};

ErrorId MsgConfig::RplBufferRelease = { ErrorOf( ES_CONFIG, 215, E_INFO, EV_NONE, 0 ),
R"(Maximum size of buffered journal before the buffer's memory should be freed
after the journal transaction is completed.
)"
};

ErrorId MsgConfig::RplChecksumAuto = { ErrorOf( ES_CONFIG, 216, E_INFO, EV_NONE, 0 ),
R"(Controls which tables' checksums are noted in the journal on journal rotation:
	0: None
	1: Only tables that are always expected to be identical
	2: Above and some tables that may contain differences due to timestamps
	3: Above and tables that may have local only data on edge servers
)"
};

ErrorId MsgConfig::RplChecksumChange = { ErrorOf( ES_CONFIG, 217, E_INFO, EV_NONE, 0 ),
R"(Controls change-by-change checksum behavior:
	0: Off
	1: Submits write checksums are journal notes
	2: On replay of above note, verify the checksum and log if invalid
	3: On replay of above note, verify the checksum and log result
)"
};

ErrorId MsgConfig::RplChecksumTable = { ErrorOf( ES_CONFIG, 218, E_INFO, EV_NONE, 0 ),
R"(Controls table checksum behavior:
	0: Off
	1: Table checksums in the journal are checked and logged if they differ.
	2: Table checksums in the journal are checked and the result logged.
)"
};

ErrorId MsgConfig::RplCompress = { ErrorOf( ES_CONFIG, 219, E_INFO, EV_NONE, 0 ),
R"(Enabled replica upstream network compression:
	0: No compression
	1: Compress only archive transfers
	2: Compress metadata and archive transfers
	3: Compress metadata and archive transfers including data streams for
	   forwarded commands
	4: Compress only metadata transfers
)"
};

ErrorId MsgConfig::RplCounterHook = { ErrorOf( ES_CONFIG, 220, E_INFO, EV_NONE, 0 ),
R"(When set to 0, prevents certain counters from being decremented by replication.
)"
};

ErrorId MsgConfig::RplDeferredSends = { ErrorOf( ES_CONFIG, 221, E_INFO, EV_NONE, 0 ),
R"(Allow '%'pull -u'%' to fetch missing archives on edge and build servers scheduled
by '%'p4 sync'%'. If disabled, the archives will be fetched synchronously by the
sync command itself.
)"
};

ErrorId MsgConfig::RplFilterExpandBatch = { ErrorOf( ES_CONFIG, 523, E_INFO, EV_NONE, 0 ),
R"(Block size used for the '%'admin replica-filter-reconcile'%' expand operation.
)"
};

ErrorId MsgConfig::RplFilterExpand = { ErrorOf( ES_CONFIG, 524, E_INFO, EV_NONE, 0 ),
R"(Allow '%'pull'%' to automatically run the '%'admin replica-filter-reconcile'%'
expand operation when a server filter change is detected.
)"
};

ErrorId MsgConfig::RplFilterExpandThreads = { ErrorOf( ES_CONFIG, 525, E_INFO, EV_NONE, 0 ),
R"(The number of threads to use in the replica filter expand operation.
)"
};

ErrorId MsgConfig::RplFilterRestrictBatch = { ErrorOf( ES_CONFIG, 515, E_INFO, EV_NONE, 0 ),
R"(Number of records to batch process per database table in the
'%'admin replica-filter-reconcile'%' restrict operation.
)"
};

ErrorId MsgConfig::RplFilterRestrict = { ErrorOf( ES_CONFIG, 516, E_INFO, EV_NONE, 0 ),
R"(Allow '%'pull'%' to automatically run the '%'admin replica-filter-reconcile'%'
restrict operation when a server filter change is detected.
)"
};

ErrorId MsgConfig::RplFilterRestrictThreads = { ErrorOf( ES_CONFIG, 517, E_INFO, EV_NONE, 0 ),
R"(The number of threads to use in the replica filter restrict operation.
)"
};

ErrorId MsgConfig::RplJnlBatchSize = { ErrorOf( ES_CONFIG, 222, E_INFO, EV_NONE, 0 ),
R"(Maximum size in bytes of a single journal transfer.
)"
};

ErrorId MsgConfig::RplJnlwaitAdjust = { ErrorOf( ES_CONFIG, 223, E_INFO, EV_NONE, 0 ),
R"(The additional time in milliseconds to add to the wait before checking for
journal updates after no updates were seen. The total will not exceed the value
specified by '%'rpl.jnlwait.max'%'.
)"
};

ErrorId MsgConfig::RplJnlwaitInterval = { ErrorOf( ES_CONFIG, 224, E_INFO, EV_NONE, 0 ),
R"(The longest time in milliseconds to wait before checking for journal updates.
Incremented by '%'rpl.jnlwait.adjust'%' each time no updates are seen.
)"
};

ErrorId MsgConfig::RplJnlwaitMax = { ErrorOf( ES_CONFIG, 225, E_INFO, EV_NONE, 0 ),
R"(The longest time in milliseconds to wait before checking for journal updates.
)"
};

ErrorId MsgConfig::RplJournalAck = { ErrorOf( ES_CONFIG, 226, E_INFO, EV_NONE, 0 ),
R"(Number of acknowledgments required to progress the server's logical
end of journal file for replicas to replicate up to. Will be reduced to the
number of standby replicas active.
)"
};

ErrorId MsgConfig::RplJournalAckMin = { ErrorOf( ES_CONFIG, 227, E_INFO, EV_NONE, 0 ),
R"(Minimum number of acknowledgments required to progress the server's logical
end of journal file for replicas to replicate up to. Should be equal to or less
than the number standby replicas.
)"
};

ErrorId MsgConfig::RplJournalcopyLocation = { ErrorOf( ES_CONFIG, 228, E_INFO, EV_NONE, 0 ),
R"(Current journalcopy'd journal location:
	0: written directly to '%'journalPrefix'%'
	1: rotated to '%'journalPrefix'%'
)"
};

ErrorId MsgConfig::RplLabelsGlobal = { ErrorOf( ES_CONFIG, 229, E_INFO, EV_NONE, 0 ),
R"(If enabled, all labels created on edge servers are automatically global.
)"
};

ErrorId MsgConfig::RplReplayUserrp = { ErrorOf( ES_CONFIG, 230, E_INFO, EV_NONE, 0 ),
R"(When enabled, the '%'db.user.rp'%' and '%'db.ticket.rp'%' tables will be replicated
from the upstream replica. This will replicate the tickets and access time for
users logged into the upstream server.
)"
};

ErrorId MsgConfig::RplTrackBehind = { ErrorOf( ES_CONFIG, 231, E_INFO, EV_NONE, 0 ),
R"(Controls the tracking of replication lag:
	0: No tracking is done
	1: The '%'rtv.rpl.behind.*'%' realtime counters are updated by replication
	   based on the interval defined by '%'rpl.track.behind.interval'%'.
	   Also logged when '%'rpl'%' >= 1.
	2: '%'pull -l -j'%' will log the replication lag when '%'rpl'%' >= 1.
)"
};

ErrorId MsgConfig::RplTrackBehindInterval = { ErrorOf( ES_CONFIG, 232, E_INFO, EV_NONE, 0 ),
R"(The interval in milliseconds that the server will send lag information to
downstream replicas.
)"
};

ErrorId MsgConfig::RplVerifyCache = { ErrorOf( ES_CONFIG, 233, E_INFO, EV_NONE, 0 ),
R"(When set to 1, verifies archive contents in the replica's cache before
delivering it to the user, potentially refreshing the archive from the upstream
server if the archive is not intact.
)"
};

ErrorId MsgConfig::RplVerifyShared = { ErrorOf( ES_CONFIG, 234, E_INFO, EV_NONE, 0 ),
R"(When set to 1, enables commit/replica shared archive storage checks.
)"
};

ErrorId MsgConfig::RplPullArchivedepots = { ErrorOf( ES_CONFIG, 235, E_INFO, EV_NONE, 0 ),
R"(When enabled, archives stored in archive depots will be replicated to this
server.
)"
};

ErrorId MsgConfig::RunClientextsAllow = { ErrorOf( ES_CONFIG, 236, E_INFO, EV_NONE, 0 ),
R"(Enables client-side extensions support.
)"
};

ErrorId MsgConfig::RunMoveAllow = { ErrorOf( ES_CONFIG, 237, E_INFO, EV_NONE, 0 ),
R"(When set to 0, blocks any '%'p4 move'%' commands.
)"
};

ErrorId MsgConfig::RunObliterateAllow = { ErrorOf( ES_CONFIG, 238, E_INFO, EV_NONE, 0 ),
R"(When set to 0, blocks any '%'p4 obliterate'%' commands.
)"
};

ErrorId MsgConfig::RunPruneAllow = { ErrorOf( ES_CONFIG, 239, E_INFO, EV_NONE, 0 ),
R"(When set to 0, blocks any '%'p4 prune'%' commands.
)"
};

ErrorId MsgConfig::RunRenameclientAllow = { ErrorOf( ES_CONFIG, 240, E_INFO, EV_NONE, 0 ),
R"(Controls access to the '%'p4 renameclient'%' command:
	0: Disable the command for all users
	1: Enable the command for client owners, admin users, and super users
	2: Enable the command for admin and super users
	3: Enable the command for only super users
)"
};

ErrorId MsgConfig::RunUnzipUserAllow = { ErrorOf( ES_CONFIG, 241, E_INFO, EV_NONE, 0 ),
R"(When set to 0, blocks '%'p4 unzip -u'%' (import by specified user).
)"
};

ErrorId MsgConfig::RunUsersAuthorize = { ErrorOf( ES_CONFIG, 242, E_INFO, EV_NONE, 0 ),
R"(When enabled, only users who are authenticated can run '%'p4 users'%'.
)"
};

ErrorId MsgConfig::ServerCommandlimits = { ErrorOf( ES_CONFIG, 243, E_INFO, EV_NONE, 0 ),
R"(Controls client provided command limits are handled:
	0: Allows command-line flags to override group settings
	1: Allows command-line flags to reduce group settings
	2: Command-line flags are ignored
)"
};

ErrorId MsgConfig::ServerCtrlcFilecleanup = { ErrorOf( ES_CONFIG, 244, E_INFO, EV_NONE, 0 ),
R"(When enabled, stopping the service by Ctrl+C (SIGINT) will cause any open files
to be closed and cleaned up gracefully.
)"
};

ErrorId MsgConfig::ServerExtsAllowAdmin = { ErrorOf( ES_CONFIG, 245, E_INFO, EV_NONE, 0 ),
R"(When enabled, users with '%'admin'%' protections can install and configure
extensions.
)"
};

ErrorId MsgConfig::ServerExtsAllowUnsigned = { ErrorOf( ES_CONFIG, 246, E_INFO, EV_NONE, 0 ),
R"(Unless enabled, extensions must be signed before they can be installed.
)"
};

ErrorId MsgConfig::ServerFilecharset = { ErrorOf( ES_CONFIG, 247, E_INFO, EV_NONE, 0 ),
R"(If set to 1, unicode files persist the charset of the file that was used on
the submitting client, and clients will use this encoding on sync.
See "Versioned character set" in '%'p4 help undoc'%'.
)"
};

ErrorId MsgConfig::ServerLocksArchive = { ErrorOf( ES_CONFIG, 248, E_INFO, EV_NONE, 0 ),
R"(By default, p4 archive and p4 restore lock the global metadata while archiving
or restoring revisions. To disable locking, set the value of this configurable
to 0.
)"
};

ErrorId MsgConfig::ServerLocksSync = { ErrorOf( ES_CONFIG, 249, E_INFO, EV_NONE, 0 ),
R"(When set, the '%'p4 sync'%' command takes a client workspace lock in shared
mode. The default value of 0 prevents sync from taking a client workspace lock.
If '%'db.peeking'%' is enabled, the '%'server.locks.dir'%' directory must exist.
The changes to locking behavior that occur when you enable '%'db.peeking'%'
obviate the need to set '%'server.locks.dir'%' to disabled, but if performance
issues arise with respect to multiple concurrent, large, and/or interrupted
'%'p4 sync'%' commands, you can obtain the old behavior for syncing by setting
'%'server.locks.sync'%' to 0.
)"
};

ErrorId MsgConfig::ServerLocksGlobal = { ErrorOf( ES_CONFIG, 250, E_INFO, EV_NONE, 0 ),
R"(Set this configurable to 1 to make '%'p4 lock'%' from an edge server take
global locks on the commit server by default.
)"
};

ErrorId MsgConfig::ServerAllowfetch = { ErrorOf( ES_CONFIG, 251, E_INFO, EV_NONE, 0 ),
R"(Determines whether changes can be fetched.
	0: Fetching to or from this server is disabled.
	1: This server can fetch from other servers.
	2: Other servers can fetch from this server.
	3: Both 1 and 2 are allowed.
)"
};

ErrorId MsgConfig::ServerAllowpush = { ErrorOf( ES_CONFIG, 252, E_INFO, EV_NONE, 0 ),
R"(Determines whether changes can be pushed.
	0: Pushing to or from this server is disabled.
	1: This server can push to other servers.
	2: Other servers can push to this server.
	3: Both 1 and 2 are allowed.
)"
};

ErrorId MsgConfig::ServerAllowremotelocking = { ErrorOf( ES_CONFIG, 253, E_INFO, EV_NONE, 0 ),
R"(DVCS configurations with files of type '%'+l'%' can use the '%'--remote'%' flag
on the '%'p4 edit'%', '%'p4 delete'%', and '%'p4 revert'%' commands.
This locks '%'+l'%' file types exclusively on the remote server.
The locks are released automatically when the modified files are pushed.

The remote server must be a commit server and this configurable must have 1,
not 0, as its value.
)"
};

ErrorId MsgConfig::ServerAllowrewrite = { ErrorOf( ES_CONFIG, 254, E_INFO, EV_NONE, 0 ),
R"(If set to a non-zero value, allows this server to run the '%'p4 unsubmit'%'
and '%'p4 fetch -t'%' commands.
)"
};

ErrorId MsgConfig::DmDvcsAllowcasemismatch = { ErrorOf( ES_CONFIG, 500, E_INFO, EV_NONE, 0 ),
R"(If set to a non-zero value, allows this server to send/receive changes
with another server whose case handling is different.
)"
};

ErrorId MsgConfig::ServerGlobalClientViews = { ErrorOf( ES_CONFIG, 255, E_INFO, EV_NONE, 0 ),
R"(Controls whether the view maps (or client maps) of a non-stream client on an
edge server are made global when a client is modified.

View maps of a client on a replica must be made global if that client is to
be used as a template on another server.

This configurable can be set globally, or individually for each server.

Setting this configurable can make client view maps global upon the subsequent
client modification. Clearing this configurable does not delete the view maps
of any clients, but does prevent subsequent changes to a client's view map
from being propagated to other servers. If a client with global view maps is
deleted, its view maps are also deleted globally regardless of the value of
'%'server.global.client.views'%'.
)"
};

ErrorId MsgConfig::ServerOomAdjScore = { ErrorOf( ES_CONFIG, 256, E_INFO, EV_NONE, 0 ),
R"(Linux out-of-memory killer score adjustment value to be set on server processes
whilst writing to the database.
)"
};

ErrorId MsgConfig::ServerMaxcommands = { ErrorOf( ES_CONFIG, 257, E_INFO, EV_NONE, 0 ),
R"(If monitoring is enabled, this configurable is set to a nonzero value, and
the limit is exceeded:

P4 Server refuses to accept more than this many simultaneous command
requests users receive the TooManyCommands error.
)"
};

ErrorId MsgConfig::ServerMaxcommandsAllow = { ErrorOf( ES_CONFIG, 258, E_INFO, EV_NONE, 0 ),
R"(Allow super and operator users access to a subset of commands even if
'%'server.maxcommands'%' is reached.
To disable, set to 0.
You must restart the server after changing the value of this configurable.
)"
};

ErrorId MsgConfig::ServerStartUnlicensed = { ErrorOf( ES_CONFIG, 259, E_INFO, EV_NONE, 0 ),
R"(When enabled, the server may start even if the license is invalid.
User's commands will be rejected until a valid license is installed.
)"
};

ErrorId MsgConfig::ServerStartupAutoRestart = { ErrorOf( ES_CONFIG, 504, E_INFO, EV_NONE, 0 ),
R"(When enabled, the server will automatically restart startup commands.
)"
};

ErrorId MsgConfig::FiletypeBypasslock = { ErrorOf( ES_CONFIG, 260, E_INFO, EV_NONE, 0 ),
R"(Enables the '%'--bypass-exclusive-lock'%' used by Swarm to unshelve/submit
changes under review that include '%'+l'%' type files.
)"
};

ErrorId MsgConfig::FiletypeMaxtextsize = { ErrorOf( ES_CONFIG, 261, E_INFO, EV_NONE, 0 ),
R"(Maximum file size for text type detection.
)"
};

ErrorId MsgConfig::SpecHashbuckets = { ErrorOf( ES_CONFIG, 262, E_INFO, EV_NONE, 0 ),
R"(Maximum number of subdirectories for hashed directory structures.
Limits the number of subdirectories into which files are hashed in the
spec depot, unload depot '%'server.locks.dir'%' directory
	99: default
	0: disables hashing
)"
};

ErrorId MsgConfig::SpecCustom = { ErrorOf( ES_CONFIG, 263, E_INFO, EV_NONE, 0 ),
R"(Enables the modification of all spec types:
	0: Only '%'job'%' and '%'stream'%' type specs can be modified.
	1: All spec types can be modified.
)"
};

ErrorId MsgConfig::StreamviewDotsLow = { ErrorOf( ES_CONFIG, 264, E_INFO, EV_NONE, 0 ),
R"(When set to 1, single '%'.'%' characters in stream views are treated as special
during sorts.
)"
};

ErrorId MsgConfig::StreamviewSortRemap = { ErrorOf( ES_CONFIG, 265, E_INFO, EV_NONE, 0 ),
R"(When set to 0, Remapped paths in a stream spec are applied without changing
the client view order.
)"
};

ErrorId MsgConfig::SubmitCollisioncheck = { ErrorOf( ES_CONFIG, 266, E_INFO, EV_NONE, 0 ),
R"(The default, 1, prevents submitting a changelist that contains what
the server perceives as a name collision between directory name and file name.

For example:
'%'$ p4 files ...'%'
'%'//depot/foo/bar/myfile#1 - add change 10 (text)'%'
'%'$ p4 add bar'%'
'%'//depot/foo/bar#1 - opened for add'%'
'%'$ p4 submit -d test Submitting change 11.'%'
'%'Locking 1 files ...'%'
'%'Cannot add file '//depot/foo/bar', filename collides with an existing
                                      directory path in the depot.'%'

To remove the check, set this configurable to 0 and be aware of the
risk of accidentally overwriting file content.
)"
};

ErrorId MsgConfig::SubmitForcenoretransfer = { ErrorOf( ES_CONFIG, 267, E_INFO, EV_NONE, 0 ),
R"(Enables unsupported and undocumented '%'p4 submit --forcenotransfer'%' command.
)"
};

ErrorId MsgConfig::SubmitNoretransfer = { ErrorOf( ES_CONFIG, 268, E_INFO, EV_NONE, 0 ),
R"(Always re-transfer files after a failed submit.
Set to 1 if you want the server to check whether files are already in
the expected archive location and not re-transfer such files.

You can override the set behavior by using the '%'--noretransfer'%' option
to the p4 submit command.
)"
};

ErrorId MsgConfig::SubmitAllowbgtransfer = { ErrorOf( ES_CONFIG, 269, E_INFO, EV_NONE, 0 ),
R"(To enable background archive transfer, set to 1 on those edge servers,
if any, that you want to have this feature. See '%'submit.autobgtransfer'%'.
)"
};

ErrorId MsgConfig::SubmitAutobgtransfer = { ErrorOf( ES_CONFIG, 270, E_INFO, EV_NONE, 0 ),
R"(If you have set '%'submit.allowbgtransfer'%' to 1 on one or more of the
edge servers that participate in your replicated environment, to make
'%'p4 submit'%' function as if it were an alias for '%'p4 submit -b'%' on
those edge servers, also set '%'submit.autobgtransfer'%' to 1 on those
same edge servers.
)"
};

ErrorId MsgConfig::SubmitUnlockLocked = { ErrorOf( ES_CONFIG, 271, E_INFO, EV_NONE, 0 ),
R"(When set, open files that users have locked (with the '%'p4 lock'%' command)
are automatically unlocked after a failed '%'p4 submit'%'.
)"
};

ErrorId MsgConfig::SubmitStoragefields = { ErrorOf( ES_CONFIG, 272, E_INFO, EV_NONE, 0 ),
R"(Controls the action of a submit request with regards to the '%'compCksum'%' field in
a storage record:
	0: Do not compute the digest of the incoming archive files on the server
	   during a submit
	1: Compute the digest of the incoming archive files on the server during
	   a submit
)"
};

ErrorId MsgConfig::TraitStoredepot = { ErrorOf( ES_CONFIG, 273, E_INFO, EV_NONE, 0 ),
R"(Controls trait value storage:
	0:  Trait value is always stored in the db.traits table
	>0: Trait value is stored in the traits depot when its size meets or
	    exceeds this value
)"
};

ErrorId MsgConfig::SwitchStreamUnrelated = { ErrorOf( ES_CONFIG, 274, E_INFO, EV_NONE, 0 ),
R"(Enables the use of '%'p4 switch'%' to switch between unrelated streams without
requiring the '%'--allow-unrelated'%' flag.
)"
};

ErrorId MsgConfig::PushUnlockLocked = { ErrorOf( ES_CONFIG, 275, E_INFO, EV_NONE, 0 ),
R"(When set, unlock locked files if '%'p4 push'%' fails.
)"
};

ErrorId MsgConfig::SysMemoryPoolfree = { ErrorOf( ES_CONFIG, 276, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::SysMemoryProcfree = { ErrorOf( ES_CONFIG, 277, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::SysMemoryPoolgrowinc = { ErrorOf( ES_CONFIG, 278, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::SysMemoryProcgrowinc = { ErrorOf( ES_CONFIG, 279, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::SysMemorySubpools = { ErrorOf( ES_CONFIG, 280, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::SysMemoryLimit = { ErrorOf( ES_CONFIG, 281, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::SysMemoryDebug = { ErrorOf( ES_CONFIG, 282, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::CmdMemoryPoolfree = { ErrorOf( ES_CONFIG, 283, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::CmdMemoryProcfree = { ErrorOf( ES_CONFIG, 284, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::CmdMemoryLimit = { ErrorOf( ES_CONFIG, 285, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::CmdMemoryFlushpool = { ErrorOf( ES_CONFIG, 286, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::CmdMemoryListpools = { ErrorOf( ES_CONFIG, 287, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::CmdMemoryChkpt = { ErrorOf( ES_CONFIG, 288, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::SysMemoryMiShowerrors = { ErrorOf( ES_CONFIG, 289, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
When enabled, outputs memory manager errors and warnings.
)"
};

ErrorId MsgConfig::SysMemoryMiShowstats = { ErrorOf( ES_CONFIG, 290, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
When enabled, shows memory stats on server process termination.
)"
};

ErrorId MsgConfig::SysMemoryMiVerbose = { ErrorOf( ES_CONFIG, 291, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
When enabled, outputs verbose memory manager information.
)"
};

ErrorId MsgConfig::SysMemoryMiEagercommit = { ErrorOf( ES_CONFIG, 292, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
commit per segment directly (4MiB)  (but see also '%'eager_commit_delay'%')
)"
};

// Rename from eager_region_commit to arena_eager_commit
ErrorId MsgConfig::SysMemoryMiArenaeagercommit = { ErrorOf( ES_CONFIG, 293, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
eager commit arena's? 2 is used to enable this only on an OS that has overcommit
(i.e. linux)
)"
};

// Rename from reset_decommits to purge_decommits
ErrorId MsgConfig::SysMemoryMiPurgedecommits = { ErrorOf( ES_CONFIG, 294, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
purge decommits memory (instead of reset) (note: on linux this uses
MADV_DONTNEED for decommit)
)"
};

// Rename from large_os_pages to allow_large_os_pages
ErrorId MsgConfig::SysMemoryMiAllowlargeospages = { ErrorOf( ES_CONFIG, 295, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
use large OS pages, use only with eager commit to prevent fragmentation of VMA's
)"
};

ErrorId MsgConfig::SysMemoryMiReservehugeospages = { ErrorOf( ES_CONFIG, 296, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
per 1GiB huge pages
)"
};

ErrorId MsgConfig::SysMemoryMiReservehugeospagesat = { ErrorOf( ES_CONFIG, 297, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
reserve huge pages at node N
)"
};

ErrorId MsgConfig::SysMemoryMiReserveosmemory = { ErrorOf( ES_CONFIG, 298, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
reserve os memory
)"
};

ErrorId MsgConfig::SysMemoryMiSegmentcache = { ErrorOf( ES_CONFIG, 299, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
cache N segments per thread
)"
};

ErrorId MsgConfig::SysMemoryMiDeprecatedpagereset = { ErrorOf( ES_CONFIG, 300, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
reset page memory on free
)"
};

// Rename from abandoned_page_reset to abandoned_page_purge
ErrorId MsgConfig::SysMemoryMiAbandonedpagepurge = { ErrorOf( ES_CONFIG, 301, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
reset free page memory when a thread terminates
)"
};

ErrorId MsgConfig::SysMemoryMiDeprecatedsegmentreset = { ErrorOf( ES_CONFIG, 302, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
reset segment memory on free (needs eager commit)
)"
};

ErrorId MsgConfig::SysMemoryMiEagercommitdelay = { ErrorOf( ES_CONFIG, 303, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
the first N segments per thread are not eagerly committed (but per page in the
segment on demand)
)"
};

// Rename from reset_delay to purge_delay
ErrorId MsgConfig::SysMemoryMiPurgeDelay = { ErrorOf( ES_CONFIG, 304, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
purge delay in milli-seconds
)"
};

ErrorId MsgConfig::SysMemoryMiUsenumanodes = { ErrorOf( ES_CONFIG, 305, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
0 = use available numa nodes, otherwise use at most N nodes.
)"
};

ErrorId MsgConfig::SysMemoryMiLimitosalloc = { ErrorOf( ES_CONFIG, 306, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
1 = do not use OS memory for allocation (but only reserved arenas)
)"
};

ErrorId MsgConfig::SysMemoryMiOstag = { ErrorOf( ES_CONFIG, 307, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
only apple specific for now but might serve more or less related purpose
)"
};

ErrorId MsgConfig::SysMemoryMiMaxerrors = { ErrorOf( ES_CONFIG, 308, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
maximum errors that are output
)"
};

ErrorId MsgConfig::SysMemoryMiMaxwarnings = { ErrorOf( ES_CONFIG, 309, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
maximum warnings that are output
)"
};

ErrorId MsgConfig::SysMemoryMiMaxsegmentreclaim = { ErrorOf( ES_CONFIG, 310, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
max. number of segment reclaims from the abandoned segments per try
)"
};

// Deprecated
ErrorId MsgConfig::SysMemoryMiAllowdecommit = { ErrorOf( ES_CONFIG, 311, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
decommit slices when no longer used (after decommit_delay milli-seconds)
)"
};

// Deprecated
ErrorId MsgConfig::SysMemoryMiSegmentdecommitdelay = { ErrorOf( ES_CONFIG, 312, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
decommit delay in milli-seconds for freed segments
)"
};

ErrorId MsgConfig::SysMemoryMiDestroyonexit = { ErrorOf( ES_CONFIG, 484, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
release all OS memory on process exit; careful with dangling pointer
or after-exit frees!
)"
};

ErrorId MsgConfig::SysMemoryMiArenareserve = { ErrorOf( ES_CONFIG, 502, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
reserve memory N KiB at a time
)"
};

ErrorId MsgConfig::SysMemoryMiArenapurgemult = { ErrorOf( ES_CONFIG, 503, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
purge delay multiplier for arena's
)"
};

// Rename from decommit_extend_delay to purge_extend_delay
ErrorId MsgConfig::SysMemoryMiPurgeextenddelay = { ErrorOf( ES_CONFIG, 313, E_INFO, EV_NONE, 0 ),
R"(Exposed mimalloc option - not guaranteed to take effect:
wait a tiny bit longer in case there is a series of free's
)"
};

ErrorId MsgConfig::SysMemoryStacksize = { ErrorOf( ES_CONFIG, 314, E_INFO, EV_NONE, 0 ),
R"(Value to set the stack size limit to on Linux.
)"
};

ErrorId MsgConfig::SysPressureMaxPauseTime = { ErrorOf( ES_CONFIG, 315, E_INFO, EV_NONE, 0 ),
R"(The number of seconds a command is able to spend in the paused state because of
resource pressure before the P4 Server returns an error to the client.
Setting this configurable to 0 disables pausing commands entirely.
)"
};

ErrorId MsgConfig::SysPressureMaxPaused = { ErrorOf( ES_CONFIG, 316, E_INFO, EV_NONE, 0 ),
R"(The maximum number of concurrently resource pressure paused client commands on
the P4 Server (0 is unlimited). New incoming commands above this
threshold will be rejected with an error.
)"
};

ErrorId MsgConfig::SysPressureMemHigh = { ErrorOf( ES_CONFIG, 317, E_INFO, EV_NONE, 0 ),
R"(While the server is above this threshold, commands might be cancelled and return
an error to the client.
Setting this configurable to 0 disables this memory threshold check.
)"
};

ErrorId MsgConfig::SysPressureMemHighDuration = { ErrorOf( ES_CONFIG, 318, E_INFO, EV_NONE, 0 ),
R"(Number of milliseconds for averaging '%'sys.pressure.mem.high'%'.
)"
};

ErrorId MsgConfig::SysPressureMemMedium = { ErrorOf( ES_CONFIG, 319, E_INFO, EV_NONE, 0 ),
R"(Percentage of processes stalled for memory.
)"
};

ErrorId MsgConfig::SysPressureMemMediumDuration = { ErrorOf( ES_CONFIG, 320, E_INFO, EV_NONE, 0 ),
R"(Number of milliseconds for averaging '%'sys.pressure.mem.medium'%'.
)"
};

ErrorId MsgConfig::SysPressureOsCpuHigh = { ErrorOf( ES_CONFIG, 321, E_INFO, EV_NONE, 0 ),
//TODO: @jgibson must have a better explaination of this
R"((Linux) CPU monitoring is only available if cgroups v2 support is configured.
This configurable represents the amount of time some processes on the system
are spending stalled waiting for CPU time. For Linux cgroup support,
only the system-wide '%'/proc/pressure/*'%' files are considered.
Setting this configurable to 0 disables this check on stalled processes.
)"
};

ErrorId MsgConfig::SysPressureOsCpuHighDuration = { ErrorOf( ES_CONFIG, 322, E_INFO, EV_NONE, 0 ),
R"(Number of milliseconds for averaging '%'sys.pressure.os.cpu.high'%'.
)"
};

ErrorId MsgConfig::SysPressureOsMemHigh = { ErrorOf( ES_CONFIG, 323, E_INFO, EV_NONE, 0 ),
//TODO: @jgibson must have a better explaination of this
R"((Windows and Linux) Amount of time some processes on the system are spending
stalled waiting for the memory. New incoming commands received by the server
while at this threshold are rejected. Existing commands that request more
memory while the server is above this threshold might be canceled and return
an error to the client. When the P4 Server limits its work, it does
not distinguish between memory used by other processes on the operating system
and its own. For example, if a large external process comes and consumes a
large amount of memory, the P4 Server can throttle itself in response.
Setting this configurable to 0 disables this threshold checking.
)"
};

ErrorId MsgConfig::SysPressureOsMemHighDuration = { ErrorOf( ES_CONFIG, 324, E_INFO, EV_NONE, 0 ),
R"(Number of milliseconds for averaging '%'sys.pressure.os.mem.high'%'.
)"
};

ErrorId MsgConfig::SysPressureOsMemMedium = { ErrorOf( ES_CONFIG, 325, E_INFO, EV_NONE, 0 ),
//TODO: @jgibson must have a better explaination of this
R"(The server tries to keep memory usage below the 'medium' level. Linux only.
)"
};

ErrorId MsgConfig::SysPressureOsMemMediumDuration = { ErrorOf( ES_CONFIG, 326, E_INFO, EV_NONE, 0 ),
R"(Number of milliseconds for averaging '%'sys.pressure.os.mem.medium'%'.
)"
};

ErrorId MsgConfig::SysRenameMax = { ErrorOf( ES_CONFIG, 327, E_INFO, EV_NONE, 0 ),
R"(Maximum number of retries if renaming a file fails.
)"
};

ErrorId MsgConfig::SysRenameWait = { ErrorOf( ES_CONFIG, 328, E_INFO, EV_NONE, 0 ),
R"(Timeout in milliseconds between file rename attempts.
)"
};

ErrorId MsgConfig::SysThreadingGroups = { ErrorOf( ES_CONFIG, 329, E_INFO, EV_NONE, 0 ),
R"(Enables utilizing multiple processor groups on Windows Server 2008 R2 or later:
	0: Disabled
	1: Enabled
)"
};

ErrorId MsgConfig::SysTypesAllow64 = { ErrorOf( ES_CONFIG, 330, E_INFO, EV_NONE, 0 ),
R"(Controls the behavior of 64-bit types:
	0: Only allow 32-bit changelists and traits
	1: Allow 64-bit changelists and traits, allow pre-23.2 clients
	2: Allow 64-bit changelists and traits, warn pre-23.2 clients
	3: Allow 64-bit changelists and traits, forbid pre-23.2 clients
)"
};

ErrorId MsgConfig::RplForwardAll = { ErrorOf( ES_CONFIG, 331, E_INFO, EV_NONE, 0 ),
R"(When set to 1 on a replica, commands that are not read-only commands are
forwarded to the upstream sever.
)"
};

ErrorId MsgConfig::RplForwardLogin = { ErrorOf( ES_CONFIG, 332, E_INFO, EV_NONE, 0 ),
R"(Set to 1 on each replica to enable single-sign-on authentication for
users in a distributed configuration. The '%'auth.id'%' configurable must also be
the same for all servers participating in a distributed configuration.
)"
};

ErrorId MsgConfig::RplPullPosition = { ErrorOf( ES_CONFIG, 333, E_INFO, EV_NONE, 0 ),
R"(Interval in milliseconds for pull position reports.
If set, the position in the %'P4TARGET'%'s journal of the last journal fragment
applied by the replica is written (that is, reported) to the replica's server
log and the replica's journal at the specified interval.
)"
};

ErrorId MsgConfig::RplPullReload = { ErrorOf( ES_CONFIG, 334, E_INFO, EV_NONE, 0 ),
R"(Interval in milliseconds for '%'p4 pull'%' to reload any configuration changes.
)"
};

ErrorId MsgConfig::SslSecondarySuite = { ErrorOf( ES_CONFIG, 335, E_INFO, EV_NONE, 0 ),
R"(Enables the alternative server TLS 1.0-1.2 cipher list selection:
	0: AES256-SHA
	1: CAMELLIA256-SHA

This may be overridden by setting '%'ssl.cipher.list'%'.
TLS 1.3 uses the OpenSSL defaults unless overridden by '%'ssl.cipher.suites'%'.
)"
};

ErrorId MsgConfig::SslClientTimeout = { ErrorOf( ES_CONFIG, 336, E_INFO, EV_NONE, 0 ),
R"(Timeout in seconds for SSL handshake negotiation on client connects.
If '%'net.maxwait'%' is longer, '%'net.maxwait'%' is used instead.
)"
};

ErrorId MsgConfig::SslClientTlsVersionMin = { ErrorOf( ES_CONFIG, 337, E_INFO, EV_NONE, 0 ),
R"(Minimum TLS version to use for client connections, including those made by
servers. Valid values are:
	10: TLS 1.0
	11: TLS 1.1
	12: TLS 1.2
	13: TLS 1.3

If '%'ssl.tls.version.min'%' is set and '%'ssl.client.tls.version.min'%' is not set,
the value of '%'ssl.tls.version.min'%' will be used rather than the default.
)"
};

ErrorId MsgConfig::SslClientTlsVersionMax = { ErrorOf( ES_CONFIG, 338, E_INFO, EV_NONE, 0 ),
R"(Maximum TLS version to use for client connections, including those made by
servers. Valid values are:
	10: TLS 1.0
	11: TLS 1.1
	12: TLS 1.2
	13: TLS 1.3

If '%'ssl.tls.version.max'%' is set and '%'ssl.client.tls.version.max'%' is not set,
the value of '%'ssl.tls.version.max'%' will be used rather than the default.
)"
};

ErrorId MsgConfig::SslClientTrustName = { ErrorOf( ES_CONFIG, 339, E_INFO, EV_NONE, 0 ),
R"(Controls how '%'p4 trust'%' records the fingerprint of a server's certificate:
	0: Only the IP address is be recorded
	1: Both the hostname and IP address may be recorded
	2: Only the hostname is be recorded

Hostnames can only be recorded is the server's certificate is not self-signed.
)"
};

ErrorId MsgConfig::SslClientCertValidate = { ErrorOf( ES_CONFIG, 340, E_INFO, EV_NONE, 0 ),
R"(Controls how SSL client connections are validated:
	0: Use P4TRUST without certificate validation
	1: Validate the certificate chain against a certificate authority (CA)
	2: Only validate that the certificate's subject matches the P4PORT
)"
};

ErrorId MsgConfig::SslTlsVersionMin = { ErrorOf( ES_CONFIG, 341, E_INFO, EV_NONE, 0 ),
R"(Minimum TLS version to use for server connections.
Valid values are:
	10: TLS 1.0
	11: TLS 1.1
	12: TLS 1.2
	13: TLS 1.3
)"
};

ErrorId MsgConfig::SslTlsVersionMax = { ErrorOf( ES_CONFIG, 342, E_INFO, EV_NONE, 0 ),
R"(Maximum TLS version to use for server connections.
Valid values are:
	10: TLS 1.0
	11: TLS 1.1
	12: TLS 1.2
	13: TLS 1.3
)"
};

ErrorId MsgConfig::SslEnableEtm = { ErrorOf( ES_CONFIG, 343, E_INFO, EV_NONE, 0 ),
R"(When set to 0, disables Encrypt-then-MAC (EtM).
)"
};

ErrorId MsgConfig::SslHandshakeBlocking = { ErrorOf( ES_CONFIG, 344, E_INFO, EV_NONE, 0 ),
R"(When set to 1, enables blocking SSL handshakes. This may be more reliable under
heavy connection load, but at the cost of disabling DoS prevention.
)"
};

ErrorId MsgConfig::TriggersIo = { ErrorOf( ES_CONFIG, 345, E_INFO, EV_NONE, 0 ),
R"(Controls how triggers are communicated with:
	0: Variables are provided to the trigger as command-line
	   arguments. Return code is used to determine pass/fail.
	1: Variables are provided as a dictionary via STDIN.
	   Dictionary response via STDOUT is used to determine pass/fail.
)"
};

ErrorId MsgConfig::IstatMimicIchanges = { ErrorOf( ES_CONFIG, 346, E_INFO, EV_NONE, 0 ),
R"(If set to 1, '%'p4 istat'%' behaves like '%'p4 ichanges'%' rather than '%'p4 merge'%'.
)"
};

ErrorId MsgConfig::InfoP4AuthUsercheck = { ErrorOf( ES_CONFIG, 347, E_INFO, EV_NONE, 0 ),
R"(Validate username against %'P4AUTH'% server
)"
};

ErrorId MsgConfig::AuthAutologinprompt = { ErrorOf( ES_CONFIG, 348, E_INFO, EV_NONE, 0 ),
R"(Enables automatic login prompting on the command-line:
	0: Disabled
	1: Enabled
)"
};

ErrorId MsgConfig::RplSubmitNocopy = { ErrorOf( ES_CONFIG, 349, E_INFO, EV_NONE, 0 ),
R"(Disable default submit archive file copy
)"
};

ErrorId MsgConfig::Auth2FaPersist = { ErrorOf( ES_CONFIG, 350, E_INFO, EV_NONE, 0 ),
R"(Enables 2FA persistence:
	0: Disabled
	1: Enabled, users must use '%'p4 login -p'%'
	2: Enabled, '%'p4 login2 -p'%' is implicit
)"
};

ErrorId MsgConfig::AuthTicketsNounlocked = { ErrorOf( ES_CONFIG, 351, E_INFO, EV_NONE, 0 ),
R"(If set to 1 or 2, prevents '%'p4 login -a'%' from issuing host
unlocked tickets. In other words, 1 or 2 enforce host locked
tickets, which are restricted to the one host with the correct IP address.
	0: Default.
	1: Means the -a flag is silently ignored and the users are
	   always issued host locked tickets.
	2: Means the -a flag is explicitly disabled and users get
	   an error if they try to use it.

If either value is set, the tagged output from p4 info '%'p4 -ztag info'%'
shows that unlockedTickets is disabled:
'%'... unlockedTickets disabled'%'
)"
};

ErrorId MsgConfig::AuthLicenseexpiryWarn = { ErrorOf( ES_CONFIG, 352, E_INFO, EV_NONE, 0 ),
R"(Enables license expiry warning:
	0: No license expiry warning
	1: Warn users with permission level admin or higher when they log in
	2: Warn all users when they log in
)"
};

ErrorId MsgConfig::AuthLicenseexpiryWarnthreshold = { ErrorOf( ES_CONFIG, 353, E_INFO, EV_NONE, 0 ),
R"(Number of days prior to the expiration of the P4 license to start
providing the warning if '%'auth.licenseexpiry.warn'%' is set.
)"
};

ErrorId MsgConfig::AuthSsoAllowPasswd = { ErrorOf( ES_CONFIG, 354, E_INFO, EV_NONE, 0 ),
R"(Allows users who's '%'AuthMethod'%' is '%'perforce'%' to fall back to password
authentication when an '%'auth-check-sso'%' trigger is in use:
	0: The user will be required to authenticate using a client-side
	   %'P4LOGINSSO'% script
	1: The user will be prompted to authenticate by password if the
	   %'P4LOGINSSO'% is not configured in their environment.

If LDAP is enabled, see the '%'auth.sso.nonldap'%' configurable.
)"
};

ErrorId MsgConfig::AuthSsoNonldap = { ErrorOf( ES_CONFIG, 355, E_INFO, EV_NONE, 0 ),
R"(Controls when '%'auth-check-sso'%' triggers are used for users whose
'%'AuthMethod'%' is '%'perforce'%' when LDAP authentication is enabled:
	0: The user will be prompted to authenticate by password.
	1: The user will be required to authenticate using a client-side
	   '%P4LOGINSSO%' script, unless '%'auth.sso.allow.passwd'%' is
	   also set to 1 allowing the user to authenticate by
	   password if the '%'P4LOGINSSO'%' is not configured in their
	   environment.
)"
};

ErrorId MsgConfig::ZlibCompressionLevel = { ErrorOf( ES_CONFIG, 356, E_INFO, EV_NONE, 0 ),
R"(Adjusts the level of compression used by Zlib to optimize file sizes to
potentially reduce CPU load at the expense of longer transfer times during
'%'p4 submit'%', '%'p4 sync'%' and similar commands. In many cases the time
to transfer plus compression time can be lower by adjusting this value from
the default value:
	-1: Use Zlib default setting
	 0: Lowest amount of compression (larger file size)
	 9: Highest amount of compression (smaller file size)
)"
};

ErrorId MsgConfig::ZlibDisableOptim = { ErrorOf( ES_CONFIG, 357, E_INFO, EV_NONE, 0 ),
R"(The Zlib used by the P4API can utilize SSE3/SSE42/PCLMULQDQ CPU instructions.
Some CPUs do not correctly implement these instructions, which can result in
forks failing with 'illegal instruction' faults; disabling this optimization
may address this:
	0: Zlib optimizations enabled
	1: Zlib optimizations disabled
)"
};

ErrorId MsgConfig::SysTypesTextStorageBehavior = { ErrorOf( ES_CONFIG, 496, E_INFO, EV_NONE, 0 ),
R"(When enabled decide the implicit storage Modifier for input of basetype
rcs based on the current lbr.autocompress and always show the storage modifier
for basetype text in command outputs:
	0: Disabled - old behavior
	1: Enabled
)"
};

//
// String Tunables
//

ErrorId MsgConfig::SecurityDigest = { ErrorOf( ES_CONFIG, 493, E_INFO, EV_NONE, 0 ),
R"(The digest algorithm used in the round trip tamper checking.
If unset or set to an unknown algorithm, the default sha256 algorithm will be
used; in the case of invalid algorithms or algorithms now considered insecure,
reports will be written to the server log.
)"
};

ErrorId MsgConfig::SSLClientCAPath = { ErrorOf( ES_CONFIG, 358, E_INFO, EV_NONE, 0 ),
R"(The path of a PEM-formatted Certificate Authority (CA) file to validate server
certificates, for use if the CA is not present in the system's CA store or if
validation against a specific CA is required.

This configurable applies to any client connection, including when a server
connects to another server.
)"
};

ErrorId MsgConfig::SslClientCipherList = { ErrorOf( ES_CONFIG, 497, E_INFO, EV_NONE, 0 ),
R"(The list of OpenSSL ciphers the client will allow when establishing a TLS 1.2
or below connection, overriding the default, or 'ssl.cipher.list' if set.
)"
};

ErrorId MsgConfig::SslClientCipherSuites = { ErrorOf( ES_CONFIG, 498, E_INFO, EV_NONE, 0 ),
R"(The list of OpenSSL cipher suites the client will allow when establishing a
TLS 1.3 connection, overriding the OpenSSL defaults, or 'ssl.cipher.suites'
if set.
)"
};

ErrorId MsgConfig::SslCipherList = { ErrorOf( ES_CONFIG, 431, E_INFO, EV_NONE, 0 ),
R"(The list of OpenSSL ciphers the server will allow when establishing a TLS 1.2
or below connection, overriding the default.
This will be used by clients too unless 'ssl.client.cipher.list' is also set.
)"
};

ErrorId MsgConfig::SslCipherSuites = { ErrorOf( ES_CONFIG, 432, E_INFO, EV_NONE, 0 ),
R"(The list of OpenSSL cipher suites the server will allow when establishing a
TLS 1.3 connection, overriding the OpenSSL defaults.
This will be used by clients too unless 'ssl.client.cipher.suites' is also set.
)"
};

ErrorId MsgConfig::SSLKeylogFile = { ErrorOf( ES_CONFIG, 479, E_INFO, EV_NONE, 0 ),
R"(The file to record the ssl key log to. This is useful for debugging SSL with
tools like wireshark: search for SSLKEYLOGFILE for examples.
)"
};


//
// Configurables
//

ErrorId MsgConfig::P4ROOT = { ErrorOf( ES_CONFIG, 359, E_INFO, EV_NONE, 0 ),
R"(The location of the server's database. This can only be specified in the
environment or with the '%'-r'%' flag when starting the server.
)"
};

ErrorId MsgConfig::P4PORT = { ErrorOf( ES_CONFIG, 360, E_INFO, EV_NONE, 0 ),
R"(The port number and optionally protocol and network interface that the server
should listen for connections on.
)"
};

ErrorId MsgConfig::P4JOURNAL = { ErrorOf( ES_CONFIG, 361, E_INFO, EV_NONE, 0 ),
R"(The name of the database journal file. This should be set to a location on
separate fast disks to the '%'P4ROOT'%'.
)"
};

ErrorId MsgConfig::P4NAME = { ErrorOf( ES_CONFIG, 362, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and should no longer be used.
See '%'p4 help serverid'%'.
)"
};

ErrorId MsgConfig::P4DEBUG = { ErrorOf( ES_CONFIG, 363, E_INFO, EV_NONE, 0 ),
R"(A comma separated list of configurables and values to be used.
)"
};

ErrorId MsgConfig::P4DESCRIPTION = { ErrorOf( ES_CONFIG, 364, E_INFO, EV_NONE, 0 ),
R"(The server description displayed in '%'p4 info'%'.
)"
};

ErrorId MsgConfig::P4LOG = { ErrorOf( ES_CONFIG, 365, E_INFO, EV_NONE, 0 ),
R"(The name of the '%'serverlog'%'. Most of the events are available via structured
logging, with the main exception being debug logging.
)"
};

ErrorId MsgConfig::P4AUDIT = { ErrorOf( ES_CONFIG, 366, E_INFO, EV_NONE, 0 ),
R"(The name of the '%'audit'%' log, which tracks user access to file revision content
for compliance purposes.
The same information is available via structured logging.
)"
};

ErrorId MsgConfig::P4AUTH = { ErrorOf( ES_CONFIG, 367, E_INFO, EV_NONE, 0 ),
R"(If set, this specifies the '%'P4PORT'%' of another server which will be responsible
for user management: this includes users, groups, licensing and authentication.
)"
};

ErrorId MsgConfig::P4CHANGE = { ErrorOf( ES_CONFIG, 368, E_INFO, EV_NONE, 0 ),
R"(If set, specifies the '%'P4PORT'%' of another server which will be responsible
for allocating new changelist numbers. For edge servers, this is implicitly
set to the commit server.
)"
};

ErrorId MsgConfig::P4TICKETS = { ErrorOf( ES_CONFIG, 369, E_INFO, EV_NONE, 0 ),
R"(The file used to store user authenticated user's tickets.
)"
};

ErrorId MsgConfig::P4TRUST = { ErrorOf( ES_CONFIG, 370, E_INFO, EV_NONE, 0 ),
R"(The file used to store trusted SSL server fingerprints for verifying client
connections.
)"
};

ErrorId MsgConfig::ServiceUser = { ErrorOf( ES_CONFIG, 371, E_INFO, EV_NONE, 0 ),
R"(The service user a server (or proxy) uses to authenticate against a master
server in a replication/proxy configuration, or against a remote server in
the context of remote depots.
)"
};

ErrorId MsgConfig::Threadingmode = { ErrorOf( ES_CONFIG, 372, E_INFO, EV_NONE, 0 ),
R"(The threading mode the server is running under:
	'%'multi'%': The server uses threads on Windows or fork elsewhere
	'%'single'%': The server is running in single threaded mode
	'%'daemon'%': On posix platforms, fork to run in the background before
	          running in '%'multi'%' mode.
	'%'daemonsafe'%': As above, but closes STDIO
)"
};

ErrorId MsgConfig::P4TARGET = { ErrorOf( ES_CONFIG, 373, E_INFO, EV_NONE, 0 ),
R"('%'P4PORT'%' of upstream server for replicas and proxies.
)"
};

ErrorId MsgConfig::Security = { ErrorOf( ES_CONFIG, 374, E_INFO, EV_NONE, 0 ),
R"(Server security level:
	0: Legacy support: passwords not required, strength
	   requirements unenforced.
	1: Strong passwords required, existing passwords not reset,
	   compatible with pre-2003.2 client software.
	2: Strong passwords required, existing passwords reset,
	   requires 2003.2 or higher client software.
	3: Passwords must be strong, and ticket-based authentication
	   (%'p4 login'%) is required.
	4: All of the above restrictions. Also, authenticated service
	   users must be used for all replica server and remote depot
	   connections to this server.
	5: Requires that any intermediary (such as a proxy or broker)
	   has a valid authenticated service user.
	6: Requires each intermediary to have a valid server spec,
	   where the service user must match the user named in the
	   %'User'% field of the spec.
)"
};

ErrorId MsgConfig::Unicode = { ErrorOf( ES_CONFIG, 375, E_INFO, EV_NONE, 0 ),
R"(If set, the server will assume all data is encoded in UTF8 and enable file
content translation for files of type '%'unicode'%'. Clients must specify a valid
'%'P4CHARSET'%' when communicating with the server.
Unicode mode cannot be disabled and should only be enabled via 'p4d -xi'.
)"
};

ErrorId MsgConfig::Monitor = { ErrorOf( ES_CONFIG, 376, E_INFO, EV_NONE, 0 ),
R"(Valid values for the monitor configurable are:
	0: Server process monitoring off. (Default)
	1: monitor active commands
	2: active commands and idle connections
	3: same as 2, but also includes connections that failed to
	   initialize (stuck at the Init() phase)
	5: same as 2, but also includes a list of the files locked
	   by the command for more than one second
	10: same as 5, but also includes lock wait times
	25: same as 10, except that the list of files locked by the
	    command includes files locked for any duration
)"
};

ErrorId MsgConfig::MinClient = { ErrorOf( ES_CONFIG, 377, E_INFO, EV_NONE, 0 ),
R"(Lowest version of client software permitted to connect to this server.
The minClient version can be set to a release or client protocol level less or
equal to the server's release version.
)"
};

ErrorId MsgConfig::MinClientMessage = { ErrorOf( ES_CONFIG, 378, E_INFO, EV_NONE, 0 ),
R"(Message to issue if client software is too old.
)"
};

ErrorId MsgConfig::NetMimcheck = { ErrorOf( ES_CONFIG, 379, E_INFO, EV_NONE, 0 ),
R"(Man-in-the-middle network security level: This enables checks for possible
interception or modification of data such as using an SSH tunnel or other
TCP forwarder for users with passwords set.

	0: Disable MitM checks.
	1: Check proxy/broker connections in legacy contexts.
	2: Connections from clients are checked for TCP forwarding.
	3: Connections from clients, proxies, and brokers are checked
	   for TCP forwarding.
	4: All connections are checked; client software older than
	   release 2010.1 cannot connect.
	5: Requires that proxies, brokers, and all Perforce intermediate
	   servers have valid logged-in service users associated with
	   them. This allows administrators to prevent unauthorized
	   proxies and services from being used.
)"
};

ErrorId MsgConfig::StartupN = { ErrorOf( ES_CONFIG, 380, E_INFO, EV_NONE, 0 ),
R"(For replica servers, set '%'startup.1'%' through '%'startup.N'%' to be
'%'p4 pull'%' threads to be spawned at startup.

All servers with a configured ServerID can set '%'startup.1'%' through
'%'startup.N'%' to be background tasks to be spawned at startup.

The '%'startup.N'%' configurables are processed sequentially. Processing
stops at the first gap in the numerical sequence. Any commands after a
gap are ignored.
)"
};

ErrorId MsgConfig::DbReplication = { ErrorOf( ES_CONFIG, 381, E_INFO, EV_NONE, 0 ),
R"(Control behavior of commands that access metadata (%'db.*'% files) on the P4 server:
	'%'readonly'%': User commands that read metadata are accepted;
	                commands that modify metadata are rejected.

Equivalent to starting a replica with the '%'p4d -M readonly'%' option.
This configurable cannot be set globally. You must specify a server id.
)"
};

ErrorId MsgConfig::LbrReplication = { ErrorOf( ES_CONFIG, 382, E_INFO, EV_NONE, 0 ),
R"(Controls the replication behavior archive files on replica servers:
	'%'readonly'%': Replicates archives when files are updated on
	                the upstream server.
	'%'shared'%': The archives must be shared with the commit
	          server and so no replication is required.
	'%'ondemand'%': Replicates archives only when when they are
	            needed and are missing on the replica.
	            May be shared with commit.
	'%'cache'%': Replicates archives only when when they are
	         needed and are missing on the replica.
	         Cannot be shared with commit server.
	         Supports '%'p4 cachepurge`%`.
	'%'none'%': The server performs no replication of archives.

This configurable cannot be set globally; you must specify a server id.
)"
};

ErrorId MsgConfig::Statefile = { ErrorOf( ES_CONFIG, 383, E_INFO, EV_NONE, 0 ),
R"(For replica servers, the file used by the server to track the current journal
position.
)"
};

ErrorId MsgConfig::JournalPrefix = { ErrorOf( ES_CONFIG, 384, E_INFO, EV_NONE, 0 ),
R"(Prefix including path for rotated journals and checkpoints.
For rotated journals, this value will be appended with '%'.N.jnl'%' where N
is the journal number.
)"
};

ErrorId MsgConfig::JournalPrefixBackup = { ErrorOf( ES_CONFIG, 385, E_INFO, EV_NONE, 0 ),
R"(Secondary location for journalPrefix.
)"
};

ErrorId MsgConfig::FilesysP4ROOTMin = { ErrorOf( ES_CONFIG, 386, E_INFO, EV_NONE, 0 ),
R"(Minimum disk space required on server root filesystem before server rejects
commands.

To specify size, use the following binary abbreviations, which are slightly
different from the more familiar decimal abbreviations:
	'%'t'%' or '%'T'%' for terabytes
	'%'g'%' or '%'G'%' for gigabytes
	'%'m'%' or '%'M'%' for megabytes
	'%'k'%' or '%'K'%' for kilobytes

You can also use a percentage to specify the relative amount of free disk
space required.
)"
};

ErrorId MsgConfig::FilesysP4JOURNALMin = { ErrorOf( ES_CONFIG, 387, E_INFO, EV_NONE, 0 ),
R"(Minimum disk space required on server journal filesystem before server rejects
commands.

To specify size, use the following binary abbreviations, which are slightly
different from the more familiar decimal abbreviations:
	'%'t'%' or '%'T'%' for terabytes
	'%'g'%' or '%'G'%' for gigabytes
	'%'m'%' or '%'M'%' for megabytes
	'%'k'%' or '%'K'%' for kilobytes

You can also use a percentage to specify the relative amount of free disk
space required.
)"
};

ErrorId MsgConfig::FilesysP4LOGMin = { ErrorOf( ES_CONFIG, 388, E_INFO, EV_NONE, 0 ),
R"(Minimum disk space required on server log filesystem before server rejects
commands.

To specify size, use the following binary abbreviations, which are slightly
different from the more familiar decimal abbreviations:
	'%'t'%' or '%'T'%' for terabytes
	'%'g'%' or '%'G'%' for gigabytes
	'%'m'%' or '%'M'%' for megabytes
	'%'k'%' or '%'K'%' for kilobytes

You can also use a percentage to specify the relative amount of free disk
space required.
)"
};

ErrorId MsgConfig::FilesysTEMPMin = { ErrorOf( ES_CONFIG, 433, E_INFO, EV_NONE, 0 ),
R"(Minimum disk space required on server temp filesystem before server rejects
commands.

To specify size, use the following binary abbreviations, which are slightly
different from the more familiar decimal abbreviations:
	'%'t'%' or '%'T'%' for terabytes
	'%'g'%' or '%'G'%' for gigabytes
	'%'m'%' or '%'M'%' for megabytes
	'%'k'%' or '%'K'%' for kilobytes

You can also use a percentage to specify the relative amount of free disk
space required.
)"
};

ErrorId MsgConfig::FilesysDepotMin = { ErrorOf( ES_CONFIG, 389, E_INFO, EV_NONE, 0 ),
R"(Minimum disk space required for each depot. The server rejects commands from
those depots, if any, that fail to meet this minimum.

To specify size, use the following binary abbreviations, which are slightly
different from the more familiar decimal abbreviations:
	'%'t'%' or '%'T'%' for terabytes
	'%'g'%' or '%'G'%' for gigabytes
	'%'m'%' or '%'M'%' for megabytes
	'%'k'%' or '%'K'%' for kilobytes

You can also use a percentage to specify the relative amount of free disk
space required.
)"
};

ErrorId MsgConfig::ServerDepotRoot = { ErrorOf( ES_CONFIG, 390, E_INFO, EV_NONE, 0 ),
R"(The filesystem location, an absolute path, with respect to which a relative
address given in the %'Map:'% field of a depot form is evaluated.
If it is not set, the %'Map:'% field relative address is evaluated with
respect to the value stored in %'P4ROOT'%.
)"
};

ErrorId MsgConfig::ServerExtensionsDir = { ErrorOf( ES_CONFIG, 391, E_INFO, EV_NONE, 0 ),
R"(Directory for Extension-owned storage.
)"
};

ErrorId MsgConfig::ServerLocksDir = { ErrorOf( ES_CONFIG, 392, E_INFO, EV_NONE, 0 ),
R"(Directory for server locks, specified relative to '%'P4ROOT'%'. To disable
server locking, set this configurable to '%'disabled'%'. (If '%'db.peeking'%'
is nonzero (enabled), '%'server.locks'%' cannot be disabled; you can disable
locking by setting '%'server.locks.sync'%' to 0.)
)"
};

ErrorId MsgConfig::ServerlogCounterN = { ErrorOf( ES_CONFIG, 393, E_INFO, EV_NONE, 0 ),
R"(The optional name of the counter to use to hold the current rotation number for
the structured log file; if unset, the last rotated log's number is found and
incremented.
This applies to the structured log file designated with the same %'.N'% value.
)"
};

ErrorId MsgConfig::ServerlogFileN = { ErrorOf( ES_CONFIG, 394, E_INFO, EV_NONE, 0 ),
R"(Server log file name associated with each structured log file.
See '%'p4 logparse'%' for a list of valid filenames.
N may not exceed 500.
)"
};

ErrorId MsgConfig::ServerlogEventsN = { ErrorOf( ES_CONFIG, 395, E_INFO, EV_NONE, 0 ),
R"(The optional explicit list of events to include in the structured log file.
This applies to the structured log file designated with the same %'.N'% value.
)"
};

ErrorId MsgConfig::ServerlogFormatN = { ErrorOf( ES_CONFIG, 396, E_INFO, EV_NONE, 0 ),
R"(The optional format of the structured log file. Only '%'csv'%' is supported.
This applies to the structured log file designated with the same %'.N'% value.
)"
};

ErrorId MsgConfig::ServerlogMaxmbN = { ErrorOf( ES_CONFIG, 397, E_INFO, EV_NONE, 0 ),
R"(The optional size threshold in megabytes to trigger the structured log file
rotation; structured logs are also rotated on journal rotation.
This applies to the structured log file designated with the same %'.N'% value.
)"
};

ErrorId MsgConfig::ServerlogNameN = { ErrorOf( ES_CONFIG, 398, E_INFO, EV_NONE, 0 ),
R"(The optional alternative name for a structured log file, which can be used with
'%'p4 logparse'%', '%'p4 logrotate'%', '%'p4 logstat'%' and '%'p4 logtail'%'.
This applies to the structured log file designated with the same %'.N'% value.
)"
};

ErrorId MsgConfig::ServerlogRetainN = { ErrorOf( ES_CONFIG, 399, E_INFO, EV_NONE, 0 ),
R"(The optional retention count for rotated structured log files: if a log is
rotated more than this number, the earlier rotated logs beyond this limit are
deleted.
This applies to the structured log file designated with the same %'.N'% value.
)"
};

ErrorId MsgConfig::ServerlogVersionN = { ErrorOf( ES_CONFIG, 400, E_INFO, EV_NONE, 0 ),
R"(The optional version to limit a structured log file to: if set, events will be
downgraded or dropped if their format has changed beyond the specified version.
This applies to the structured log file designated with the same %'.N'% value.
)"
};

ErrorId MsgConfig::ServerlogBufszN = { ErrorOf( ES_CONFIG, 401, E_INFO, EV_NONE, 0 ),
R"(The optional in-memory buffer size for a structured log file.
If set, the maximum number of bytes in the buffer for the log file.
By default, there is no buffer. A buffer improves performance because the
server only writes to the log file when the command has completed or the
buffer has been filled. Be aware that data loss is possible if the server
crashes before the buffer's content is written to the log file.
This applies to the structured log file designated with the same %'.N'% value.
)"
};

ErrorId MsgConfig::ServerRolechecks = { ErrorOf( ES_CONFIG, 402, E_INFO, EV_NONE, 0 ),
R"(Should server enforce '%'p4 server'%' settings.
)"
};

ErrorId MsgConfig::SubmitIdentity = { ErrorOf( ES_CONFIG, 403, E_INFO, EV_NONE, 0 ),
R"(Enable the generation of global changelist ids. This is relevant for users of
the P4 Server's distributed versioning (DVCS) features.
	'%'uuid'%': generates the id in uuid format.
	'%'checksum'%': generates the id in checksum format.
	'%'serverid'%': generates the id in serverid+change format.
)"
};

ErrorId MsgConfig::TemplateClient = { ErrorOf( ES_CONFIG, 404, E_INFO, EV_NONE, 0 ),
R"(Specifies the default client to be used as a template if the user omits
the '%'-t'%' option on the '%'p4 client'%' command.
)"
};

ErrorId MsgConfig::TemplateLabel = { ErrorOf( ES_CONFIG, 405, E_INFO, EV_NONE, 0 ),
R"(Specifies the default label to be used as a template if the user omits the
'%'-t'%' option on the p4 label command.
)"
};

ErrorId MsgConfig::DefaultChangeType = { ErrorOf( ES_CONFIG, 406, E_INFO, EV_NONE, 0 ),
R"(Default type for new changelists: either '%'public'%' or '%'restricted'%'.
If unset, new changelists are '%'public'%'.
)"
};

ErrorId MsgConfig::ZerosyncPrefix = { ErrorOf( ES_CONFIG, 407, E_INFO, EV_NONE, 0 ),
R"(If set, changes default behavior of '%'p4 sync'%' such that if a client
workspace begins with this prefix, all sync operations to affected workspaces
assume '%'p4 sync -k'%', and do not alter contents of the workspace.
)"
};

ErrorId MsgConfig::RejectList = { ErrorOf( ES_CONFIG, 408, E_INFO, EV_NONE, 0 ),
R"(Specifies one or more clients whose requests should be blocked.
For more information, see "Blocking Clients" in P4 Server Administrator
Guide.
)"
};

ErrorId MsgConfig::ClusterId = { ErrorOf( ES_CONFIG, 409, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and should not be used.

Configurables set to scope of the '%'cluster.id'%' will be used instead of the
'%'any'%' scope, unless set specifically to the scope of this server's serverId.
)"
};

ErrorId MsgConfig::P4UtilsDir = { ErrorOf( ES_CONFIG, 410, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::ZkHostPortPairs = { ErrorOf( ES_CONFIG, 411, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::P4zkLogFile = { ErrorOf( ES_CONFIG, 412, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::MonitorLsof = { ErrorOf( ES_CONFIG, 413, E_INFO, EV_NONE, 0 ),
R"(When set to the path of the Linux '%'lsof'%' program (e.g. '%'/usr/bin/lsof'%'),
enables additional details in the '%'p4 monitor -L'%' output.
)"
};

ErrorId MsgConfig::AuthDefaultMethod = { ErrorOf( ES_CONFIG, 414, E_INFO, EV_NONE, 0 ),
R"(The default method to use for authenticating new users:
	'%'perforce'%': specifies that the user is to be authenticated by
	            password against either the P4 database or by
	            authentication triggers.
	'%'ldap'%':     specifies the user be authenticated directly against
	            an AD/LDAP server.
)"
};

ErrorId MsgConfig::AuthId = { ErrorOf( ES_CONFIG, 415, E_INFO, EV_NONE, 0 ),
R"(The identifier used to lookup authentication ticket for this server.

When '%'rpl.forward.login'%' is set to 1 on a replica, '%'auth.id'%' must be set to the
same value on the commit server and the replica (and all other replicas also
participating in forwarding logins).
)"
};

ErrorId MsgConfig::AuthLdapCafile = { ErrorOf( ES_CONFIG, 416, E_INFO, EV_NONE, 0 ),
R"(The path of a PEM-formatted Certificate Authority (CA) file to validate the
server certificates of the LDAP server when '%'auth.ldap.ssllevel'%' is set to
at least 1.
)"
};

ErrorId MsgConfig::AuthLdapOrderN = { ErrorOf( ES_CONFIG, 417, E_INFO, EV_NONE, 0 ),
R"(The ordered list of LDAP configurations to use when authenticating users who
have '%'AuthMethod'%' set to '%ldap'%'.
)"
};

ErrorId MsgConfig::AuthLdapSslciphers = { ErrorOf( ES_CONFIG, 418, E_INFO, EV_NONE, 0 ),
R"(OpenSSL cipher list to be used of LDAPS or LDAP with StartTLS connections.
)"
};

ErrorId MsgConfig::AuthLdapPagesize = { ErrorOf( ES_CONFIG, 419, E_INFO, EV_NONE, 0 ),
R"(The maximum number of LDAP records to request per-page. If 0, all results will
be returned in one response.
)"
};

ErrorId MsgConfig::AuthLdapSsllevel = { ErrorOf( ES_CONFIG, 420, E_INFO, EV_NONE, 0 ),
R"(Controls SSL certificate validation behavior:
0: no validation (default)
1: cert must be valid, but CN not checked
2: cert must be valid and CN must match
)"
};

ErrorId MsgConfig::AuthLdapTimeout = { ErrorOf( ES_CONFIG, 421, E_INFO, EV_NONE, 0 ),
R"(Number of seconds to wait for an LDAP authentication request to complete.
)"
};

ErrorId MsgConfig::AuthLdapUserautocreate = { ErrorOf( ES_CONFIG, 422, E_INFO, EV_NONE, 0 ),
R"(When set to non-zero, automatically creates a user account on '%'p4 login'%'.
Requires '%'auth.default.method'%' to be set to '%'ldap'%'.
)"
};

ErrorId MsgConfig::AuthSsoArgs = { ErrorOf( ES_CONFIG, 423, E_INFO, EV_NONE, 0 ),
R"(A string value to provide to the client-side P4LOGINSSO triggers as '%'ssoArgs'%'
when using an '%'auth-check-sso'%' trigger.
)"
};

ErrorId MsgConfig::ZkConnectTimeout = { ErrorOf( ES_CONFIG, 424, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::ClientReadonlyDir = { ErrorOf( ES_CONFIG, 425, E_INFO, EV_NONE, 0 ),
R"(Directory for the server to store the have database files for the workspaces of
type '%'readonly'%', '%'partitioned'%' and '%'partitioned-jnl'%'. A relative path
to the server's root directory is recommended.
)"
};

ErrorId MsgConfig::ClientSendqDir = { ErrorOf( ES_CONFIG, 426, E_INFO, EV_NONE, 0 ),
R"(Directory for the server to store the partitioned database files backing
'%'p4 sync --parallel'%'.
)"
};

ErrorId MsgConfig::ClientBackupInterval = { ErrorOf( ES_CONFIG, 427, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::IdleUnloadInterval = { ErrorOf( ES_CONFIG, 428, E_INFO, EV_NONE, 0 ),
R"(This configurable has been deprecated and is no longer used.
)"
};

ErrorId MsgConfig::RtMonitorfile = { ErrorOf( ES_CONFIG, 429, E_INFO, EV_NONE, 0 ),
R"(Path to the file used to store realtime performance counters, relative to
P4ROOT. Must be set to enable realtime monitoring features. Set to 'off'
to disable.
)"
};

ErrorId MsgConfig::PullTriggerDir = { ErrorOf( ES_CONFIG, 430, E_INFO, EV_NONE, 0 ),
R"(The location that the '%'p4 pull -u --trigger'%' command writes the temporary file
passed as '%'%archiveList%'%' to the '%'pull-archive'%' trigger.
)"
};
ErrorId MsgConfig::LbrScanIgnore = { ErrorOf( ES_CONFIG, 499, E_INFO, EV_NONE, 0 ),
R"(A JSON format array of regular expressions. This list is examined during
a storage scan with the --nonlbr flag specified. Any filename that matcheds
any one of the patterns in this list will not be removed or moved.
)"
};

static const char shared_retry[] = 
R"(A JSON format array of http status codes and AWS error code strings.
These values are combined to build an object used to detect an AWS S3
error and attempt to retry it or re-fetch the credentials and retry it.
Such an object needs an http status code and an AWS error code string. The
special empty string ("") is used for the AWS error code string
when the S3 operation does not supply a response body, such as HEAD
requests. So you can retry those requests based only on matching the http
status code. While parsing the JSON array, when an http status code is
seen, it is stored and applied to any following string, until the next
http status code.

The two configurables will both retry an S3 attempt that fails with
a configured error, However "s3.credentialerror.retry" differs in that it
will first clear and reload the current credentials before the retry.

Specifing a configurable discards all its default settings.

You set these configurables if your S3 system is generating
an error you need to retry or an error that you know will never succeed
on a retry that is not covered by the defaults. There maybe
different errors raised depending on the authentication method in use.

The default values used for "s3.error.retry" are:

500, "InternalError"
307, "Redirect"
307, "TemporaryRedirect"
503, "SlowDown"
503, "503 SlowDown"
503, "ServiceUnavailable"

This would be expressed as a JSON array as:
[ 500, "InternalError", 503, "SlowDown", "503 SlowDown",
  "ServiceUnavailable", 307, "Redirect", "TemporaryRedirect" ]

The default values used for "s3.credentialerror.retry" are:
400, "ExpiredToken"
400, "InvalidSessionException"
400, "TokenRefreshRequired"
400, ""

This would be expressed as a JSON array as:
[ 400, "ExpiredToken", "InvalidSessionException", "TokenRefreshRequired",
  "" ]

Example:
The default setting in "s3.credentialerror.retry" of a http status code
of 400 with a code of "InvalidSessionException" is always failing, even
when retried, thus causing a delay before the transaction is failed.

In that case you would set "s3.credentialerror.retry" to:
[ 400, "ExpiredToken", "TokenRefreshRequired",  "" ]
)";
ErrorId MsgConfig::S3ErrorRetry = { ErrorOf( ES_CONFIG, 519, E_INFO, EV_NONE, 0 ), shared_retry};

ErrorId MsgConfig::S3CredErrorRetry = { ErrorOf( ES_CONFIG, 520, E_INFO, EV_NONE, 0 ), shared_retry};

ErrorId MsgConfig::LbrS3MultiPart = { ErrorOf( ES_CONFIG, 521, E_INFO, EV_NONE, 0 ),
R"(Only files greater than this value will use the MultiPart protocol for S3
transfers to split and send this file. This value is specified in megabytes.
The default value is 4096 (4GB)
)"
};

ErrorId MsgConfig::LbrS3MultiPartSz = { ErrorOf( ES_CONFIG, 522, E_INFO, EV_NONE, 0 ),
R"(S3 transfers using the MultiPart protocol will split the transfer into
chunks of this size. This value is specified in megabytes. The default value
is 1024 (1Gb)
)"
};

ErrorId MsgConfig::WebServerLogPath = { ErrorOf( ES_CONFIG, 529, E_INFO, EV_NONE, 0 ),
R"(Set the directory path to store the P4 Web Server log files. If not set,
 log files are stored in P4ROOT.
)"
};

ErrorId MsgConfig::WebServerLogRetain = { ErrorOf( ES_CONFIG, 531, E_INFO, EV_NONE, 0 ),
R"(Set the maximum number of old log files to retain for each P4 Web Server log
type on each web server process. When the limit is reached, the oldest files
are removed.
)"
};

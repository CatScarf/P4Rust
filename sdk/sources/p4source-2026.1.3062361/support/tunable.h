/*
 * Copyright 2008 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

enum P4TunableName {

	P4TUNE_CLUSTER_JOURNAL_SHARED = DT_LAST,// see db/dbjournal2.cc
	P4TUNE_DB_CHECKPOINT_BUFSIZE,
	P4TUNE_DB_CHECKPOINT_THREADS,
	P4TUNE_DB_CHECKPOINT_WORKLEVEL,
	P4TUNE_DB_CHECKPOINT_REQLEVEL,
	P4TUNE_DB_CHECKPOINT_NUMFILES,
	P4TUNE_DB_EXPERIMENTAL_LOGGING,
	P4TUNE_DB_INTERNAL_REPAIR,
	P4TUNE_DB_ISALIVE,
	P4TUNE_DB_JNLACK_SHARED,
	P4TUNE_DB_JOURNALROTATE_WARNTHRESH,
	P4TUNE_DB_MONITOR_ADDTHRESH,
	P4TUNE_DB_MONITOR_INTERVAL,
	P4TUNE_DB_MONITOR_MINRUNTIME,
	P4TUNE_DB_MONITOR_TERMALLOW,
	P4TUNE_DB_MONITOR_SHARED,
	P4TUNE_DB_PAGE_MIGRATE,
	P4TUNE_DB_PARTITION_JUST_DROPTABLE,
	P4TUNE_DB_PARTITION_VERIFY,
	P4TUNE_DB_PEEKING,
	P4TUNE_DB_PEEKING_USEMAXLOCK,
	P4TUNE_DB_REORG_DISABLE,
	P4TUNE_DB_REORG_MISORDER,
	P4TUNE_DB_REORG_OCCUP,
	P4TUNE_DB_RT_IO,			// see dbopen.cc
	P4TUNE_DB_TRYLOCK,			// see dbschema.cc
	P4TUNE_DBARRAY_PUTCHECK,		// see dbarray.cc
	P4TUNE_DBARRAY_RESERVE,			// see dbarray.cc
	P4TUNE_DBJOURNAL_BUFSIZE,		// see dbjournal2.cc
	P4TUNE_DBJOURNAL_WORDSIZE,		// see dbjournal2.cc
	P4TUNE_DBOPEN_CACHE,			// see dbopen.cc
	P4TUNE_DBOPEN_CACHE_WIDE,		// see dbopen.cc
	P4TUNE_DBOPEN_FREEPCT,			// see dbopen.cc
	P4TUNE_DBOPEN_MISMATCH_DELAY,		// see bt_fio.cc
	P4TUNE_DBOPEN_NOFSYNC,			// see btree.cc
	P4TUNE_DBOPEN_PAGESIZE,			// see dbopen.cc
	P4TUNE_DBOPEN_RETRY,			// see bt_fio.cc
	P4TUNE_DIFF_BINARY_RCS,			// see dmtypes.cc
	P4TUNE_DIFF_SLIMIT1,
	P4TUNE_DIFF_SLIMIT2,
	P4TUNE_DIFF_STHRESH,
	P4TUNE_DM_ALTSYNC_ENFORCE,
	P4TUNE_DM_ANNOTATE_MAXSIZE,
	P4TUNE_DM_BATCH_DOMAINS,		// see dmdomains.cc
	P4TUNE_DM_BATCH_NET,			// see dmrprobe.cc
	P4TUNE_DM_CHANGE_RESTRICT_PENDING,
	P4TUNE_DM_CHANGE_SKIPKEYED,		// see dmschange.cc
	P4TUNE_DM_CHANGES_THRESH1,		// set dmchanges.cc
	P4TUNE_DM_CHANGES_THRESH2,		// set dmchanges.cc
	P4TUNE_DM_CHANGEVIEW_OPENABLE,		// see dmlimit.cc
	P4TUNE_DM_CLIENT_INITROOT,		// see rhservice.cc
	P4TUNE_DM_CLIENT_LIMITPROTECTS,		// see dmlimit.cc
	P4TUNE_DM_CONFIGURE_COMMENT_MANDATORY,	// see userconfig.cc
	P4TUNE_DM_COPY_ATTRIBUTES,		// see dmicopy.cc
	P4TUNE_DM_COPY_MOVEWARN,
	P4TUNE_DM_DOMAIN_ACCESSUPDATE,		// see dmadomain.cc
	P4TUNE_DM_DOMAIN_ACCESSFORCE,		// see dmadomain.cc
	P4TUNE_DM_DVCS_ALLOWCASEMISMATCH,	// DVCS feature enabling
	P4TUNE_DM_FETCH_PRESERVECHANGENUMBERS,
	P4TUNE_DM_FLUSHFORCE,
	P4TUNE_DM_FLUSHTRY,
	P4TUNE_DM_FSTAT_OUTPUT_THRESHOLD,	// see userfstat.cc
	P4TUNE_DM_FSTAT_MAXCONTENT,		// see userfstat.cc
	P4TUNE_DM_GRAPH_ENABLED,		// see rhservice.cc
	P4TUNE_DM_GREP_MAXLINELENGTH,
	P4TUNE_DM_GREP_MAXREVS,
	P4TUNE_DM_GREP_MAXCONTEXT,
	P4TUNE_DM_HAVE_PRELOAD,			// see dmsync.cc
	P4TUNE_DM_INFO_HIDE,
	P4TUNE_DM_INTEG_ENGINE,
	P4TUNE_DM_INTEG_MAXACT,
	P4TUNE_DM_INTEG_MAXBRANCH,
	P4TUNE_DM_INTEG_MULTIPLE_STREAMS,	// see dminteg.cc
	P4TUNE_DM_INTEG_STREAM_ALLOWDEPOTFILESPEC,// see dminteg.cc
	P4TUNE_DM_INTEG_STREAMSPEC,
	P4TUNE_DM_INTEG_TWEAKS,
	P4TUNE_DM_INTEG_UNDO,
	P4TUNE_DM_ISALIVE,			// set dmcaller.cc
	P4TUNE_DM_KEYS_HIDE,			// see dmcounters.cc
	P4TUNE_DM_LABELS_SEARCH_AUTORELOAD,	// see userdomains.cc
	P4TUNE_DM_LOCK_BATCH,			// see userlock.cc
	P4TUNE_DM_LOCKS_EXCL_BATCH_NET,	// see dmrprobe.cc
	P4TUNE_DM_LOCKS_GLOBAL_BATCH_NET,	// see dmrprobe.cc
	P4TUNE_DM_LOCKS_GLOBAL_RESULT_BATCH_NET,	// see rmtservice.cc
	P4TUNE_DM_MAXKEY,			// see dmtypes.cc
	P4TUNE_DM_OBLITERATE_SKIPHAVE,
	P4TUNE_DM_OBLITERATE_SKIPLABEL,
	P4TUNE_DM_OPEN_SHOW_GLOBALLOCKS,
	P4TUNE_DM_PASSWORD_MINLENGTH,
	P4TUNE_DM_POPULATE_SKIPKEYED,           // see userpop.cc
	P4TUNE_DM_PROTECTS_ALLOW_ADMIN,
	P4TUNE_DM_PROTECTS_EXCLUSIONCHECK,	// see dmlimit.cc
	P4TUNE_DM_PROTECTS_HIDE,
	P4TUNE_DM_PROTECTS_STREAMSPEC,		// see dmaprotect.cc
	P4TUNE_DM_PROXY_PROTECTS,
	P4TUNE_DM_QUICK_CLIENTS,
	P4TUNE_DM_QUICK_DOMAINS,
	P4TUNE_DM_QUICK_HAVE,
	P4TUNE_DM_QUICK_INTEG,
	P4TUNE_DM_QUICK_RESOLVE,
	P4TUNE_DM_QUICK_REV,
	P4TUNE_DM_QUICK_WORKING,
	P4TUNE_DM_RENAMEUSER_SCANTHRESH,	// see userrenameuser.cc
	P4TUNE_DM_REOPEN_BATCH,			// see userreopen.cc
	P4TUNE_DM_REPO_NOAUTOCREATE,		// see dmgraph.cc
	P4TUNE_DM_REPO_UNPACK,			// see dmgraph.cc
	P4TUNE_DM_RESOLVE_ATTRIB,
	P4TUNE_DM_RESOLVE_IGNOREDELETED,
	P4TUNE_DM_REV_SCAN_THRESH,		// see dmpipes.cc
	P4TUNE_DM_REVCX_THRESH1,
	P4TUNE_DM_REVCX_THRESH2,
	P4TUNE_DM_REVERT_BATCH,			// see rhfiledm.cc
	P4TUNE_DM_ROTATELOGWITHJNL,
	P4TUNE_DM_SHELVE_ACCESSUPDATE,		// see dmachange.cc
	P4TUNE_DM_SHELVE_MAXFILES,
	P4TUNE_DM_SHELVE_MAXSIZE,
	P4TUNE_DM_SHELVE_PROMOTE,
	P4TUNE_DM_STATUS_MATCHLINES,		// see userreconcile.cc
	P4TUNE_DM_STATUS_MATCHSIZE,		// see dmreconcile.cc
	P4TUNE_DM_STREAM_PARENTVIEW,
	P4TUNE_DM_STREAM_COMPONENTS,
	P4TUNE_DM_STREAM_SPARSE_BRANCHMAX,
	P4TUNE_DM_SYNC_STREAM_CHANGE,		// see usersync.cc
	P4TUNE_DM_SUBPROTECTS_GRANT_ADMIN,
	P4TUNE_DM_TOPOLOGY_LASTSEENUPDATE,	// see dmatopology.cc
	P4TUNE_DM_USER_ACCESSUPDATE,		// see dmauser.cc dmcaller.cc
	P4TUNE_DM_USER_ACCESSFORCE,		// see dmauser.cc dmcaller.cc
	P4TUNE_DM_USER_ACCESSMODE,		// see dmauser.cc dmcaller.cc
	P4TUNE_DM_USER_ALLOWSELFUPDATE,		// see dmsuser.cc
	P4TUNE_DM_USER_AUTOPROTECTIONS,		// see dmaprotect.cc
	P4TUNE_DM_USER_HIDEINVALID,		// see userlogin.cc
	P4TUNE_DM_USER_INSECURELOGIN,		// see userlogin.cc
	P4TUNE_DM_USER_LOGINATTEMPTS,		// see dmauser.cc
	P4TUNE_DM_USER_NOAUTOCREATE,		// see dmauser.cc
	P4TUNE_DM_USER_NUMERIC,		// see dmauser.cc
	P4TUNE_DM_USER_RESETPASSWORD,		// see dmauser.cc
	P4TUNE_DM_USER_SETINITIALPASSWD,	// see userpasswd.cc
	P4TUNE_FILESYS_ATOMIC_RENAME,		// see filesys.cc
	P4TUNE_FILESYS_BINARYSCAN,		// see filecheck.cc
	P4TUNE_FILESYS_BUFSIZE,			// see filesys.h
	P4TUNE_FILESYS_CACHEHINT,		// see filesys.cc
	P4TUNE_FILESYS_CHECKLINKS,		// see clientservice.cc and useropen.cc
	P4TUNE_FILESYS_DETECTUNICODE,		// see filesys.cc
	P4TUNE_FILESYS_DETECTUTF8,		// see filecheck.cc
	P4TUNE_FILESYS_LOCKDELAY,		// see filetmp.cc
	P4TUNE_FILESYS_LOCKTIMEOUT,			// see fdutil.cc
	P4TUNE_FILESYS_LOCKTRY,			// see filetmp.cc
	P4TUNE_FILESYS_MAKETMP,
	P4TUNE_FILESYS_MAXMAP,			// see readfile.cc
	P4TUNE_FILESYS_MAXSYMLINK,		// see fileiosym.cc
	P4TUNE_FILESYS_MAXTMP,
	P4TUNE_FILESYS_RESTRICTSYMLINKS,	// see clientservice.cc
	P4TUNE_FILESYS_SYMLINK_TYPEMAP,		// see clientservice.cc
	P4TUNE_FILESYS_UTF8BOM,			// see fileiouni.cc
	P4TUNE_FILESYS_EXTENDLOWMARK,
	P4TUNE_FILESYS_WINDOWS_LFN,		// see filesys.cc
	P4TUNE_FILESYS_CLIENT_NULLSYNC,		// see clientservice.cc
	P4TUNE_INDEX_DOMAIN_OWNER,              // see dmdomains.cc
	P4TUNE_LBR_AUTOCOMPRESS,		// see submit
	P4TUNE_LBR_BUFSIZE,			// see lbr.h
	P4TUNE_LBR_FABRICATE,			// see lbrservice.h
	P4TUNE_LBR_PROXY_CASE,			// see pxclient.cc
	P4TUNE_LBR_RCS_EXISTCHECK,		// see usersubmit.cc
	P4TUNE_LBR_RCS_MAXLEN,			// see lbrkeyed.cc
	P4TUNE_LBR_REPLICA_NOTRANSFER,		// see rhservice.cc
	P4TUNE_LBR_RETRY_MAX,			// see lbrstatus.cc
	P4TUNE_LBR_STAT_INTERVAL,		// see pxclient.cc and lbrstatus.cc
	P4TUNE_LBR_S3_UPLOADRETRIES,		// see lbr/s3client.cc
	P4TUNE_LBR_S3_MULTIPART,		// see lbr/s3client.cc
	P4TUNE_LBR_S3_MULTIPARTSZ,		// see lbr/s3client.cc
	P4TUNE_LBR_VERIFY_IN,			// see lbrservice.cc
	P4TUNE_LBR_VERIFY_OUT,			// see rhservice.cc
	P4TUNE_LBR_VERIFY_SCRIPT_OUT,		// see rhservice.cc
	P4TUNE_LBR_STORAGE_DELAY,               // see userstorage.cc
	P4TUNE_LBR_STORAGE_ALLOWSYMLINK,        // see userstorage.cc
	P4TUNE_LBR_STORAGE_SKIPKEYED,           // see dmstorageupgrade.cc
	P4TUNE_LBR_STORAGE_THREADS,             // see userstorage.cc
	P4TUNE_LBR_RCS_LOCKING,                 // see dmgrcslock.cc
	P4TUNE_LBR_UNLOADDEPOT_COMPRESS,        // see dmsdomain.cc
	P4TUNE_LOG_GROUP_MAXLEN,		// see rhservice.cc
	P4TUNE_LOG_ORIGINHOST,			// see rhloggable.cc
	P4TUNE_MAP_JOINMAX1,
	P4TUNE_MAP_JOINMAX2,
	P4TUNE_MAP_LIMIT_MAXLOOKBACK,
	P4TUNE_MAP_MAXWILD,
	P4TUNE_MAP_OVERLAY_LEGACY,
	P4TUNE_MERGE_DL_ENDEOL,
	P4TUNE_NET_AUTOTUNE,
	P4TUNE_NET_BUFSIZE,			// see netbuffer.h
	P4TUNE_NET_DELTA_RPL_MINSIZE,		// see rpl.cc, rmtservice.cc
	P4TUNE_NET_DELTA_RPL_THRESHOLD,		// see rpl.cc, rmtservice.cc
	P4TUNE_NET_DELTA_TRANSFER_MINSIZE,	// see clientservice.cc/usersubmit.cc
	P4TUNE_NET_DELTA_TRANSFER_THRESHOLD,	// see clientservice.cc
	P4TUNE_NET_KEEPALIVE_DISABLE,		// see nettcptransport.cc
	P4TUNE_NET_KEEPALIVE_IDLE,		// see nettcptransport.cc
	P4TUNE_NET_KEEPALIVE_INTERVAL,		// see nettcptransport.cc
	P4TUNE_NET_KEEPALIVE_COUNT,		// see nettcptransport.cc
	P4TUNE_NET_HEARTBEAT_INTERVAL,		// see userheartbeat.cc
	P4TUNE_NET_HEARTBEAT_WAIT,		// see userheartbeat.cc
	P4TUNE_NET_HEARTBEAT_MISSING_INTERVAL,	// see userheartbeat.cc
	P4TUNE_NET_HEARTBEAT_MISSING_WAIT,	// see userheartbeat.cc
	P4TUNE_NET_HEARTBEAT_MISSING_COUNT,	// see userheartbeat.cc
	P4TUNE_NET_MAX_FAULT_PUB,		// see proxy/pxclient.cc
	P4TUNE_NET_MAXCLOSEWAIT,		// see net{ssl,tcp}transport.cc
	P4TUNE_NET_MAXWAIT,			// see net{ssl,tcp}transport.cc
	P4TUNE_NET_NAGLE,			// see nettcpendpoint.cc
	P4TUNE_NET_PARALLEL_MAX,		// see dmparsend.cc
	P4TUNE_NET_PARALLEL_THREADS,		// see usersync.cc
	P4TUNE_NET_PARALLEL_BATCH,		// see usersync.cc
	P4TUNE_NET_PARALLEL_BATCHSIZE,		// see usersync.cc
	P4TUNE_NET_PARALLEL_CHECKALIVE,		// see usertransmit.cc
	P4TUNE_NET_PARALLEL_MIN,		// see usersync.cc
	P4TUNE_NET_PARALLEL_MINSIZE,		// see usersync.cc
	P4TUNE_NET_PARALLEL_SHELVE_THREADS,	// see usershelve.cc
	P4TUNE_NET_PARALLEL_SHELVE_BATCH,	// see usershelve.cc
	P4TUNE_NET_PARALLEL_SHELVE_MIN,		// see usershelve.cc
	P4TUNE_NET_PARALLEL_SUBMIT_THREADS,	// see usersubmit.cc
	P4TUNE_NET_PARALLEL_SUBMIT_BATCH,	// see usersubmit.cc
	P4TUNE_NET_PARALLEL_SUBMIT_MIN,		// see usersubmit.cc
	P4TUNE_NET_PARALLEL_SYNC_SVRTHREADS,	// see usersync.cc
	P4TUNE_NET_QUICKACK,			// see nettcp{endpoint,transport}.cc
	P4TUNE_NET_RCVBUFLOWMARK,		// see netbuffer.cc
	P4TUNE_NET_RCVBUFMAXSIZE,		// see netbuffer.cc
	P4TUNE_NET_RCVBUFSIZE,			// see netbuffer.h
	P4TUNE_NET_REUSEPORT,			// see nettcpendpoint.cc
	P4TUNE_NET_RFC3484,			// see nettcpendpoint.cc
	P4TUNE_NET_SENDLIMIT,			// set netbuffer.cc
	P4TUNE_NET_TCPSIZE,			// set nettcp.cc
	P4TUNE_NET_BACKLOG,			// see nettcp.cc
	P4TUNE_NET_X3_MINSIZE,			// see rmtservice.cc
	P4TUNE_PROXY_DELIVER_FIX,
	P4TUNE_PROXY_MONITOR_INTERVAL,		// see pxmonitor.cc
	P4TUNE_PROXY_MONITOR_LEVEL,		// see pxmonitor.cc
	P4TUNE_PROXY_CLEARCACHETHRESH,
	P4TUNE_PROXY_SUPPRESSWARNINGS,		// see pxservice.cc
	P4TUNE_RCS_MAXINSERT,
	P4TUNE_RCS_NOFSYNC,			// see rcsvfile.cc
	P4TUNE_RMT_ALLOW_REMOTEUSER,		// see rmtservice.cc
	P4TUNE_RMT_ENFORCE_SERVICEUSERTYPE,	// see rhservice.cc
	P4TUNE_RPC_DELAY,			// see rpc.cc
	P4TUNE_RPC_DURABLEWAIT,			// see rhservice.cc
	P4TUNE_RPC_HIMARK,
	P4TUNE_RPC_LOWMARK,
	P4TUNE_RPC_IPADDR_MISMATCH,		// see rhservice.cc, rpcfwd.cc
	P4TUNE_RPL_ARCHIVE_GRAPH,		// see server / rpl.cc
	P4TUNE_RPL_AWAITJNL_COUNT,		// See server / rmtservice.cc
	P4TUNE_RPL_AWAITJNL_INTERVAL,		// See server / rmtservice.cc
	P4TUNE_RPL_BUFFER_RELEASE,		// See server / rpl.cc
	P4TUNE_RPL_CHECKSUM_AUTO,
	P4TUNE_RPL_CHECKSUM_CHANGE,
	P4TUNE_RPL_CHECKSUM_TABLE,
	P4TUNE_RPL_COMPRESS,			// See server / rpl.cc
	P4TUNE_RPL_COUNTER_HOOK,		// See server / userpull.cc
	P4TUNE_RPL_DEFERRED_SENDS,		// See server / usersync.cc
	P4TUNE_RPL_FILTER_EXPAND,		// See server / userpull.cc
	P4TUNE_RPL_FILTER_EXPAND_BATCH,		// See server / useradmin.cc
	P4TUNE_RPL_FILTER_EXPAND_THREADS,	// See server / useradmin.cc
	P4TUNE_RPL_FILTER_RESTRICT,		// See server / userpull.cc
	P4TUNE_RPL_FILTER_RESTRICT_BATCH,	// See server / useradmin.cc
	P4TUNE_RPL_FILTER_RESTRICT_THREADS,	// See server / useradmin.cc
	P4TUNE_RPL_JNL_BATCH_SIZE,		// See server / rpl.cc
	P4TUNE_RPL_JNLWAIT_ADJUST,		// See server / rhservice.cc
	P4TUNE_RPL_JNLWAIT_INTERVAL,		// See server / rhservice.cc
	P4TUNE_RPL_JNLWAIT_MAX,			// See server / rhservice.cc
	P4TUNE_RPL_JOURNAL_ACK,			// See server / rmtservice.cc
	P4TUNE_RPL_JOURNAL_ACK_MIN,		// See server / rmtservice.cc
	P4TUNE_RPL_JOURNALCOPY_LOCATION,	// See server / rpl.cc
	P4TUNE_RPL_LABELS_GLOBAL,		// See userlabel/userlabsync
	P4TUNE_RPL_REPLAY_USERRP,		// See server / rpl.cc
	P4TUNE_RPL_TRACK_BEHIND,		// See server / rpl.cc
	P4TUNE_RPL_TRACK_BEHIND_INTERVAL,	// See server / rmtservice.cc
	P4TUNE_RPL_VERIFY_CACHE,		// See rhservice.cc
	P4TUNE_RPL_VERIFY_SHARED,		// See userpull.cc
	P4TUNE_RPL_PULL_ARCHIVEDEPOTS,		// See userpull.cc
	P4TUNE_RUN_CLIENTEXTS_ALLOW,		// see rhservice.cc
	P4TUNE_RUN_MOVE_ALLOW,			// see usermove.cc
	P4TUNE_RUN_OBLITERATE_ALLOW,		// see userpurge.cc
	P4TUNE_RUN_PRUNE_ALLOW,			// see userpurge.cc
	P4TUNE_RUN_RENAMECLIENT_ALLOW,		// see userrenameclient.cc
	P4TUNE_RUN_UNZIP_USER_ALLOW,		// see userzip.cc
	P4TUNE_RUN_USERS_AUTHORIZE,		// see userusers.cc
	P4TUNE_SERVER_COMMANDLIMITS,		// see dmcaller.cc
	P4TUNE_SERVER_CTRLC_FILECLEANUP,	// see rhmain.cc
	P4TUNE_SERVER_EXTS_ALLOW_ADMIN,	        // see userextension.cc
	P4TUNE_SERVER_EXTS_ALLOW_UNSIGNED,	// see userextension.cc
	P4TUNE_SERVER_FILECHARSET,
	P4TUNE_SERVER_LOCKS_ARCHIVE,		// see userarchive.cc
	P4TUNE_SERVER_LOCKS_SYNC,		// see usersync.cc
	P4TUNE_SERVER_LOCKS_GLOBAL,		// see userlock.cc
	P4TUNE_SERVER_ALLOWFETCH,		// DVCS feature enabling
	P4TUNE_SERVER_ALLOWPUSH,		// DVCS feature enabling
	P4TUNE_SERVER_ALLOWREMOTELOCKING,	// DVCS feature enabling
	P4TUNE_SERVER_ALLOWREWRITE,		// DVCS feature enabling
	P4TUNE_SERVER_GLOBAL_CLIENT_VIEWS,	// see dmr/dmrprobe.cc
	P4TUNE_SERVER_OOM_ADJ_SCORE,		// see rhmain.cc
	P4TUNE_SERVER_MAXCOMMANDS,		// see rhmain.cc
	P4TUNE_SERVER_MAXCOMMANDS_ALLOW,	// see rhmain.cc
	P4TUNE_SERVER_START_UNLICENSED,		// see rhmain.cc
	P4TUNE_FILETYPE_BYPASSLOCK,		// see useropen.cc
	P4TUNE_FILETYPE_MAXTEXTSIZE,		// set useropen.cc
	P4TUNE_SPEC_HASHBUCKETS,		// see rhspecmgr.cc
	P4TUNE_SPEC_CUSTOM,			// see userspec.cc
	P4TUNE_STREAMVIEW_DOTS_LOW,		// see maptable.cc
	P4TUNE_STREAMVIEW_SORT_REMAP,		// see viewgen.cc
	P4TUNE_SUBMIT_COLLISIONCHECK,		// see usersubmit.cc
	P4TUNE_SUBMIT_FORCENORETRANSFER,	// see usersubmit.cc
	P4TUNE_SUBMIT_NORETRANSFER,		// see usersubmit.cc
	P4TUNE_SUBMIT_ALLOWBGTRANSFER,		// see usersubmit.cc
	P4TUNE_SUBMIT_AUTOBGTRANSFER,		// see usersubmit.cc
	P4TUNE_SUBMIT_UNLOCK_LOCKED,		// see usersubmit.cc
	P4TUNE_SUBMIT_STORAGEFIELDS,            // see userstorage.cc
	P4TUNE_TRAIT_STOREDEPOT,                // see usertrait.cc
	P4TUNE_SWITCH_STREAM_UNRELATED,         // see userswitch.cc
	P4TUNE_PUSH_UNLOCK_LOCKED,		// see userpush.cc
	// vv Smart Heap tunables must be a continuous group vv
	P4TUNE_SYS_MEMORY_POOLFREE,		// see shhandler.cc
	P4TUNE_SYS_MEMORY_PROCFREE,		// see shhandler.cc
	P4TUNE_SYS_MEMORY_POOLGROWINC,		// see shhandler.cc
	P4TUNE_SYS_MEMORY_PROCGROWINC,		// see shhandler.cc
	P4TUNE_SYS_MEMORY_SUBPOOLS,		// see shhandler.cc
	P4TUNE_SYS_MEMORY_LIMIT,		// see shhandler.cc
	P4TUNE_SYS_MEMORY_DEBUG,		// see shhandler.cc/mihandler.cc
	P4TUNE_CMD_MEMORY_POOLFREE,		// see shhandler.cc
	P4TUNE_CMD_MEMORY_PROCFREE,		// see shhandler.cc
	P4TUNE_CMD_MEMORY_LIMIT,		// see shhandler.cc
	P4TUNE_CMD_MEMORY_FLUSHPOOL,		// see shhandler.cc
	P4TUNE_CMD_MEMORY_LISTPOOLS,		// see shhandler.cc
	P4TUNE_CMD_MEMORY_CHKPT,		// see shhandler.cc
	// ^^ Smart Heap tunables must be a continuous group ^^
	// vv mimalloc tunables must be a continuous group vv"
	P4TUNE_SYS_MEMORY_MI_SHOWERRORS,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_SHOWSTATS,		// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_VERBOSE,		// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_EAGERCOMMIT,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_ARENAEAGERCOMMIT,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_PURGEDECOMMITS,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_ALLOWLARGEOSPAGES,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_RESERVEHUGEOSPAGES,// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_RESERVEHUGEOSPAGESAT,// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_RESERVEOSMEMORY,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_DEPRECATEDSEGMENTCACHE, // see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_DEPRECATEDPAGERESET, // see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_ABANDONEDPAGEPURGE,// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_DEPRECATEDSEGMENTRESET, // see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_EAGERCOMMITDELAY,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_PURGE_DELAY,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_USENUMANODES,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_LIMITOSALLOC,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_OSTAG,		// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_MAXERRORS,		// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_MAXWARNINGS,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_MAXSEGMENTRECLAIM,// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_DESTROYONEXIT,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_ARENARESERVE,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_ARENAPURGEMULT,	// see mihandler.cc
	P4TUNE_SYS_MEMORY_MI_PURGEEXTENDDELAY,	// see mihandler.cc
	// ^^ mimalloc tunables must be a continuous group ^^
	P4TUNE_SYS_MEMORY_STACKSIZE,
	P4TUNE_SYS_PRESSURE_MAX_PAUSE_TIME,
	P4TUNE_SYS_PRESSURE_MAX_PAUSED,
	P4TUNE_SYS_PRESSURE_MEM_HIGH,
	P4TUNE_SYS_PRESSURE_MEM_HIGH_DURATION,
	P4TUNE_SYS_PRESSURE_MEM_MEDIUM,
	P4TUNE_SYS_PRESSURE_MEM_MEDIUM_DURATION,
	P4TUNE_SYS_PRESSURE_OS_CPU_HIGH,
	P4TUNE_SYS_PRESSURE_OS_CPU_HIGH_DURATION,
	P4TUNE_SYS_PRESSURE_OS_MEM_HIGH,
	P4TUNE_SYS_PRESSURE_OS_MEM_HIGH_DURATION,
	P4TUNE_SYS_PRESSURE_OS_MEM_MEDIUM,
	P4TUNE_SYS_PRESSURE_OS_MEM_MEDIUM_DURATION,
	P4TUNE_SYS_RENAME_MAX,			// see fileiont.cc
	P4TUNE_SYS_RENAME_WAIT,			// see fileiont.cc
	P4TUNE_SYS_THREADING_GROUPS,		// see threading.cc
	P4TUNE_SYS_TYPES_ALLOW64,		// allow 64bit fields
	P4TUNE_SYS_TYPES_TEXT_STORAGE_BEHAVIOR,
	P4TUNE_RPL_FORWARD_ALL,
	P4TUNE_RPL_FORWARD_LOGIN,		// see rhmain.cc
	P4TUNE_RPL_PULL_POSITION,		// see userpull.cc
	P4TUNE_RPL_PULL_RELOAD,			// see userpull.cc
	P4TUNE_SSL_SECONDARY_SUITE,             // see netssltransport.cc
	P4TUNE_SSL_CLIENT_TIMEOUT,		// see netssltransport.cc
	P4TUNE_SSL_CLIENT_TLS_VERSION_MIN,	// see netssltransport.cc
	P4TUNE_SSL_CLIENT_TLS_VERSION_MAX,	// see netssltransport.cc
	P4TUNE_SSL_CLIENT_TRUST_NAME,		// see clienttrust.cc
	P4TUNE_SSL_CLIENT_CERT_VALIDATE,	// see netssltransport.cc
	P4TUNE_SSL_TLS_VERSION_MIN,		// see netssltransport.cc
	P4TUNE_SSL_TLS_VERSION_MAX,		// see netssltransport.cc
	P4TUNE_SSL_ENABLE_ETM,			// see netssltransport.cc
	P4TUNE_SSL_HANDSHAKE_BLOCKING,		// see netssltransport.cc
	P4TUNE_TRIGGERS_IO,			// see rhtrigger.cc
	P4TUNE_ISTAT_MIMIC_ICHANGES,		// see dmistat.cc & DIOR_SPLIT
	P4TUNE_INFO_P4AUTH_USERCHECK,		// see userinfo.cc
	P4TUNE_AUTH_AUTOLOGIN,			// see rhservice.cc
	P4TUNE_RPL_SUBMIT_NOCOPY,		// see usersubmit.cc
	P4TUNE_AUTH_2FA_PERSIST,		// see login2.cc
	P4TUNE_AUTH_TICKETS_NOUNLOCKED,		// see login.cc
	P4TUNE_AUTH_LICENSEEXPIRY_WARN,		// see login.cc
	P4TUNE_AUTH_LICENSEEXPIRY_WARNTHRESHOLD,// see login.cc
	P4TUNE_AUTH_SSO_ALLOW_PASSWD,		// see login.cc
	P4TUNE_AUTH_SSO_NONLDAP,		// see login.cc
	P4TUNE_ZLIB_COMPRESSION_LEVEL,
	P4TUNE_ZLIB_DISABLE_OPTIM,

	P4TUNE_LAST,

	// BEGIN STRINGS
	P4TUNE_LBR_SCAN_IGNORE_PATTERN,		// see userstorage.cc
	P4TUNE_SECURITY_DIGEST,			// see digester.cc, rhservice.cc
	P4TUNE_SSL_CLIENT_CA_PATH,		// see netssltransport.cc
	P4TUNE_SSL_CLIENT_CIPHER_LIST,		// see netssltransport.cc
	P4TUNE_SSL_CLIENT_CIPHER_SUITES,	// see netssltransport.cc
	P4TUNE_SSL_CIPHER_LIST,			// see netssltransport.cc
	P4TUNE_SSL_CIPHER_SUITES,		// see netssltransport.cc
	P4TUNE_SSL_KEYLOG_FILE,			// see netssltransport.cc
	P4TUNE_S3_ERROR_RETRY,		// see lbr/s3client.cc
	P4TUNE_S3_CREDERROR_RETRY,		// see lbr/s3client.cc
	P4TUNE_LAST_STR

}  ;


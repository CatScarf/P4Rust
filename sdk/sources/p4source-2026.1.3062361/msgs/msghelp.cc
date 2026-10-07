/*
 * Copyright 1995-2010 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * msghelp.cc - definitions of errors for help subsystem.
 *
 * Note:
 *
 * Never re-use an error code value,  these may have already been
 * translated and while deleting the error is OKAY using it for a
 * different error is not.
 *
 * Its okay to add a message in the middle of the file.
 *
 * When adding a new error make sure its greater than the current high
 * value and update the following number:
 *
 * Current high value for a MsgHelp error code is: 281
 */

ErrorId MsgHelp::NoHelp = { ErrorOf( ES_HELP, 1, E_FAILED, EV_USAGE, 1 ),
"No help for %command%."
} ;

ErrorId MsgHelp::NoGraphHelp = { ErrorOf( ES_HELP, 235, E_FAILED, EV_USAGE, 1 ),
"No graph specific help for %command%."
} ;

ErrorId MsgHelp::HelpPerforce = { ErrorOf( ES_HELP, 11, E_INFO, EV_NONE, 2 ),
R"(
    Perforce P4 -- Scalable Version Control for Any Digital Asset

    p4 is the P4 command-line client.  Try:

	p4 help simple               list most common commands
	p4 help commands             list all standard commands
	p4 help command              help on a specific command

	p4 help administration       help on specialized administration topics
	p4 help charset              help on character set translation
	p4 help configurables        list server configuration variables
	p4 help environment          list environment and registry variables
	p4 help serverextensionintro introduction to server Extensions
	p4 help clientextensionintro introduction to client Extensions
	p4 help filetypes            list supported file types
	p4 help jobview              help on jobview syntax
	p4 help networkaddress       help on network address syntax
	p4 help revisions            help on specifying file revisions
	p4 help streamintro          introduction to streams
	p4 help usage                generic command line arguments
	p4 help views                help on view syntax
	p4 help replication          help on specialized replication topics
	p4 help dvcs                 help on decentralized P4 configs

	p4 help legal                legal and license information

	p4 -h                        show p4 usage details
	p4d -h                       show p4d usage details

    The full user manual is available at
    https://www.perforce.com/support/self-service-resources/documentation

    Server %release_id%/%patch_id%.
)"
};

ErrorId MsgHelp::HelpUsage = { ErrorOf( ES_HELP, 12, E_INFO, EV_NONE, 0 ),
R"(
    P4 command-line client usage:

    p4 [options] command [--explain] [arg ...]
    p4 -V

	Many command flags have both short-form and long-form syntax. Specify
	the --explain flag to display additional information about the flags
	accepted by this command. For example, 'p4 sync --explain' to display
	all the options supported by sync, or 'p4 edit --explain -k' to
	display information about the -k flag to edit.

	options:
	    -b batchsize -c client -C charset -d dir -H host
	    -I -G -L language -p port -P pass -q -r retries -s
	    -Q charset -u user -x file -z tag

	The -b batchsize specifies a batch size (number of arguments) to
	use when processing a command from a file '-x' (default 128).

	The -c flag specifies the client name, overriding the value of
	$P4CLIENT in the environment and the default (the hostname).

	The -C flag specifies the client's character set, overriding the
	value of $P4CHARSET in the environment.  See 'p4 help charset'
	for more information.

	The -d flag specifies the current directory, overriding the value of
	$PWD in the environment and the default (the current directory).

	The -G flag formats all output (and batch input for form commands
	with -i) as marshalled Python dictionary objects.

	The -Mj flag formats all output (and batch input for form commands
	with -i) as strict line-delimited JSON.  Non-UTF8 data will be
	converted to the Unicode replacement character.

	The -ztag flag is required for all marshalled output flags.

	The -H flag specifies the host name, overriding the value of
	$P4HOST in the environment and the default (the hostname).

	The -I flag specifies that progress indicators, if available,
	are desired.  Progress indicators are available for the commands:
	'p4 -I submit', 'p4 -I sync -q', 'p4 -I switch', 'p4 clone',
	'p4 -I fetch', 'p4 -I push', 'p4 -I zip', and 'p4 -I unzip'.
	This flag is not compatible with the -s, -e, -G options.

	The -L flag specifies the language for text messages from the
	server, overriding the value of $P4LANGUAGE in the environment.
	Setting this option only works if the administrator has loaded
	support for non-English messages into the server database.

	The -p flag specifies the server's listen address, overriding the
	value of $P4PORT in the environment and the default (perforce:1666).

	The -P flag specifies the password, overriding the value of
	$P4PASSWD in the environment.

	The -q flag suppresses all info messages.

	The -Q flag specifies the client's command character set,
	overriding the value of $P4COMMANDCHARSET in the environment.
	See 'p4 help charset' for more information.

	The -r flag specifies the number of times a sync command should be
	retried if the network times out (takes longer than N seconds to
	respond to a single I/O operation) during sync command execution.
	For example:
           p4 -r4 -vnet.maxwait=300 sync //...
	specifies to retry this 'sync' up to 4 times, should any single
	network operation take longer than 5 minutes to complete. The -r
	flag should not be used on a command which reads from stdin.

	The -s flag causes the p4 command line client program to prefix
	each line of output with a tag (error, warning, info, text, exit)
	to make it easier to use for scripting.

	The -u flag specifies the user name, overriding the value of
	$P4USER, $USER, and $USERNAME in the environment.

	The -x flag instructs p4 to read arguments, one per line, from the
	specified file.  If you specify '-', standard input is read.

	The -V flag displays the version of the p4 client command and exits.

	The -z tag option returns output of reporting commands in the
	format returned by 'p4 fstat'.

	The --no-script flag disables client-side Extensions.
)"
};

ErrorId MsgHelp::HelpSimple = { ErrorOf( ES_HELP, 13, E_INFO, EV_NONE, 0 ),
R"(
    Most common P4 client commands:

	client     Create or edit a client workspace specification and view

	add        Open files for adding to the depot
	edit       Open existing files for editing
	delete     Open existing files for removal from the depot
	opened     List open files

	update     Update files in the current client workspace
	resolve    Resolve updates to open workspace files

	submit     Submit open files to the depot
	revert     Revert open files and restore originals to workspace
)"
};

ErrorId MsgHelp::HelpCheckPermission = { ErrorOf( ES_HELP, 200, E_INFO, EV_NONE, 0 ),
R"(
    check-permission -- check access permission granted to a repo

    p4 check-permission -n //repo/name -u user [-r ref] -p permission

	'p4 check-permission' is used by an administrator or a super user
	to check permissions on a specified repo. An administrator is the
	owner of the graph depot, the owner of the repo, or a user that has
	been added as an administrator for the repo specified. In addition
	any user can check another users permission even if they are not an
	administrator, provided that they have been granted the permission
	that they are checking for.

	The -n //repo/name flag checks the permission for the repo specified.

	The -u user flag checks permission for the user specified.

	The -r ref flag specifies an optional reference (ref) for the
	permission (branch or tag).

	The -p permission flag checks the permission specified.
)"
};

ErrorId MsgHelp::HelpCommands = { ErrorOf( ES_HELP, 14, E_INFO, EV_NONE, 0 ),
R"(
    P4 client commands:

	add          Open a new file to add it to the depot
	aliases      Display the content of the P4ALIASES file
	annotate     Print file lines along with their revisions
	attribute    Set per-revision attributes on revisions
	branch       Create or edit a branch specification
	branches     Display list of branches
	change       Create or edit a changelist description
	changes      Display list of pending and submitted changelists
	changelist   Create or edit a changelist description
	changelists  Display list of pending and submitted changelists
	clean        Delete or refresh local files to match depot state
	client       Create or edit a client specification and its view
	clients      Display list of known clients
	copy         Schedule copy of latest rev from one file to another
	counter      Display, set, or delete a counter
	counters     Display list of known counters
	cstat        Dump change/sync status for current client
	delete       Open an existing file to delete it from the depot
	depot        Create or edit a depot specification
	depots       Display list of depots
	describe     Display a changelist description
	diff         Display diff of client file with depot file
	diff2        Display diff of two depot files
	dirs         List subdirectories of a given depot directory
	edit         Open an existing file for edit
	filelog      List revision history of files
	files        List files in the depot
	fix          Mark jobs as being fixed by named changelists
	fixes        List what changelists fix what job
	flush        Fake a 'p4 sync' by not moving files
	fstat        Dump file info
	grep         Print lines from text files matching a pattern
	group        Change members of a user group
	groups       List groups (of users)
	have         List revisions last synced
	help         Print the requested help message
	ignores      List P4IGNORE mappings
	info         Print out client/server information
	integrate    Schedule integration from one file to another
	integrated   Show integrations that have been submitted
	interchanges Report changes that have not yet been integrated
	istat        Show integrations needed for a stream
	job          Create or edit a job (defect) specification
	jobs         Display list of jobs
	key          Display, set, or delete a key/value pair
	keys         Display list of known keys and their values
	label        Create or edit a label specification and its view
	labels       Display list of labels
	labelsync    Synchronize label with the current client contents
	list         Create an in-memory (label) list of depot files
	lock         Lock an opened file against changelist submission
	logger       Report what jobs and changelists have changed
	login        Login to P4 by obtaining a session ticket
	login2       Perform a multi factor authentication
	logout       Logout of P4 by removing or invalidating a ticket
	merge        Schedule merge (integration) from one file to another
	move         Moves files from one location to another
	opened       Display list of files opened for pending changelist
	passwd       Set the user's password on the server (and Windows client)
	populate     Populate a branch or stream with files
	print        Retrieve a depot file to the standard output
	protect      Modify protections in the server namespace
	protects     Display protections in place for a given user/path
	prune        Remove unmodified branched files from a stream
	rec          Reconcile client to offline workspace changes
	reconcile    Reconcile client to offline workspace changes
	rename       Moves files from one location to another
	reopen       Change the type or changelist number of an opened file
	repo         Create, edit or delete a repo specification
	repos        Display a list of repos (depots of type graph)
	reshelve     Copy shelved files to a new or existing shelf
	resolve      Merge open files with other revisions or files
	resolved     Show files that have been merged but not submitted
	revert       Discard changes from an opened file
	review       List and track changelists (for the review daemon)
	reviews      Show what users are subscribed to review files
	set          Set variables in the registry (Windows only)
	shelve       Store files from a pending changelist into the depot
	status       Preview reconcile of client to offline workspace changes
	sizes        Display size information for files in the depot
	stream       Create or edit a stream specification
	streamlog    List revision history of streams
	streams      Display list of streams
	streamspec   Edit the stream template
	submit       Submit open files to the depot
	switch       Switch to a different stream, or create a new stream.
	sync         Synchronize the client with its view of the depot
	tag          Tag files with a label
	tickets      Display list of session tickets for this user
	trust        Establish trust of an SSL connection
	undo         Undo a range of revisions
	unlock       Release a locked file but leave it open
	unshelve     Restore shelved files from a pending changelist
	update       Update the client with its view of the depot
	user         Create or edit a user specification
	users        Display list of known users
	where        Show how file names map through the client view
	workspace    Create or edit a client specification and its view
	workspaces   Display list of known clients

    See 'p4 help administration' for more information about additional
    commands and issues of interest to those who are administering and
    operating the server.

    See 'p4 help dvcs' for more information about additional commands and
    topics of interest to those who use using P4 with decentralized workflows.

    See 'p4 help serverextensionintro' and 'p4 help clientextensionintro'
    for more information about client/server Extensions.

)"
};

ErrorId MsgHelp::HelpUndoc = { ErrorOf( ES_HELP, 15, E_INFO, EV_NONE, 0 ),
R"(
    Unsupported or obsolete P4 commands and options:

    p4 admin dump
	Outputs a checkpoint of the server through the client.  Important:
	read-locks the database during the entire operation!  Normally
	database locks are not held during network I/O to the client.
	This command requires 'super' access.

    p4 admin import [-l] [-b batchsz] [-f]
	Imports a journal of the server through the client.  The journal
	data is read from the standard input.  When the data has been
	transfered to the server, it is processed in batches with the
	batch size of 5000 records or the number specified with the '-b'
	flag.  Each batch locks only the tables that will change.
	The -l flag instructs import to write-lock all database tables
	and processes all data until it comes to the end of the file or
	encounters an error.  When applying records to the database, it
	ignores failures to delete records,  unlike 'p4d -jr', which
	normally terminates if it can't delete a record as indicated by
	the journal.  Avoid using this command to import large journal
	files, because the entire file has to fit in memory.
	The -f flag forces a replica server to run this command.
	This command requires 'super' access.

    p4 branch -F stream [ -P parent ] -o name
	The -F flag used with -o will output the full branch spec generated
	using the named stream view that includes imported paths. The
	optional -P option allows a user to specify a different parent
	stream for the branch source.

    p4 client -of
	An administrator may combine the -o and -f flags in order to display
	information about the client view havemaps. Havemaps are created when
	a client view is modified.

    p4 clone -A
	The -A flag excludes archive content for the new revisions. This flag
	also prevents the automatic sync normally performed by clone.

    p4 configure help undoc
	Displays all the undocumented configurables.

    p4 configure help all
	Displays both the documented and the undocumented configurables.

    p4 configure show env
	Displays the command-line flags and environment variables that were
	in effect when this server was started.

    p4 configure show undoc
	Displays a list of all the server tunable parameters and their
	current values. Consult Perforce Technical Support before
	changing values.

    p4 copy -e
	Only existing source files are considered for copying to the target.
	Target files with no corresponding source files are ignored.

    p4 copy -E
	This has the same effect as setting dm.integ.tweaks to 32 for
	the current command. It disables a readded/moved file integration
	algorithm.

    p4 cstat
	Dump change/sync status for current client.
	See 'p4 help cstat'.

    p4 dbpack
	Reorder database index pages.
	See 'p4 help dbpack'

    p4 diff2 -F stream [-P parent] [[file1] file2]
	The -F flag causes diff2 to use the full stream branch, including
	imported paths, to determine the pairs of files to compare. The files
	are compared against the parent stream. The optional -P flag allows
	a user to specify a different parent stream to diff against. If file
	arguments are also present, they can further limit the files and
	specify the revisions for comparison.

    p4 diff/diff2/describe -dh
	The -dh compares files on a word-by-word basis (using whitespace
	as a separator rather than just newline), and marks differences
	with HTML tags that switch the font to red or blue for words that
	are only in the first or second file, respectively.

    p4 diff/diff2/describe -dv
	The -dv compares files on a word-by-word basis (using character
	classes rather than just newline), and marks differences with
	HTML tags that switch the font to red or blue for words only
	in the first or second file, respectively.

    p4 discover
	This variant of 'p4 info' is used to discover if a server is
	in unicode mode or not.  It's important to allow this through
	a broker.

    p4 duplicate
	Duplicate revisions and their integration history.
	See 'p4 help duplicate'.

    p4 edit/delete -L [file ...]
	The -L flag can be used with multiple file arguments in order to
	process them altogether.  This flag can help performance when dealing
	with many +l files and issuing the command from an edge server.

    p4 fetch [-p target -A -I -u]
	The -p flag specifies the address of the target server, overriding
	the address from the remote spec specified by the -r flag.

	The -I flag excludes integration records for the new revisions.

	The -A flag excludes archive content for the new revisions. This flag
	also prevents the automatic sync normally performed by fetch.

	The -u flag specifies that conflicting changes should be unsubmitted,
	not relocated to a tangent. The -u flag requires the -r flag, and
	also requires that an administrator has set server.allowrewrite=1.

    p4 filelog -1
	This flag prevents filelog from following any renames resulting from
	use of the 'p4 move' command. By default, filelog follows moved files.

    p4 fstat [-Dcdh]
	The -D flag effectively runs 'p4 dirs' on the same path that was
	passed to 'p4 fstat', appending 'dirs'-like output to the normal
	'fstat' output.  The -cdh subflags correspond to 'p4 dirs -CDH'.
	Use a placeholder (e.g. -Dx) for default 'p4 dirs'-like behavior.

    p4 fstat [-OcChiz]
	The -Oc flag outputs the path, revision and type of the server
	archive file (requires 'admin' privilege). The -OC flag is the same
	as -Oc, but includes archived revisions. The -Oz flag outputs the
	path and revision of any lazy copies (requires 'admin' privilege).
	The -Oh flag outputs the size, digest and any selected attributes
	of the synced revision rather than the head revision. The -Oi flag
	outputs extra (debugging) information about the attributes.

    p4 graph cat-file
	Display object contents.
	See 'p4 help-graph cat-file'.

    p4 graph cherry-pick
	Replay one commit onto another.
	See 'p4 help-graph cherry-pick'.

    p4 graph lfs-fetch
	Fetch lfs file from repo.
	See 'p4 help-graph lfs-fetch'.

    p4 graph lfs-push
	Push lfs file into repo.
	See 'p4 help-graph lfs-push'.

    p4 graph lfs-stat
	Report data about LFS objects.
	See 'p4 help-graph lfs-stat'.

    p4 graph ls-tree
	List tree contents.
	See 'p4 help-graph ls-tree'.

    p4 graph pack-objects
	Export git data to a packfile.
	See 'p4 help-graph pack-objects'.

    p4 graph receive-pack
	Import git objects and/or references.
	See 'p4 help-graph receive-pack'.

    p4 graph ref-hist
	Display reference history.
	See 'p4 help-graph ref-hist'.

    p4 graph rev-list
	List commit objects in reverse chronological order.
	See 'p4 help-graph rev-list'.

    p4 graph submodule
	Manage submodule configuration data.
	See 'p4 help-graph submodule'.

    p4 help commandments
	Values we work by.

    p4 help credits
	Lists server contributors.

)"
R"(
    p4 index
	Add words to the jobs index.
	See 'p4 help index'.

    p4 info -w
	In tagged (-ztag) output, adds a 'threadCount' field carrying the
	server's current internal thread count as seen by the process handling
	the command. In a forked child the value reflects the count at fork
	time, not the live count. Has no effect in non-tagged output.

    p4 integrate -1 -2 -C changelist# -Rlou -Znnn
	The flag '-1' can be supplied to the 'p4 integrate' command to
	force consideration of direct integration history only.  Note that -1
	is not allowed if the integration target is a task stream.  The -C
	changelist# flag considers only	integration history from changelists
	at or below the given number, allowing you to ignore credit from
	subsequent integrations.  The -2 flag uses the old 'second generation'
	integrate engine (2006.1).  The -Rl flag causes only edited revisions
	to require integration.  The -Ro flag suppresses the scheduling of
	non-content resolves.  The -Ru flag causes resolves to be scheduled
	for 'undone' changes.  The -Z flag overrides the dm.integ.tweaks
	configurable setting.

    p4 integrate -Dst -d -i -I -o -t
	These flags are deprecated but preserved for backward compatibility.
		-Dt     If the target file has been deleted and the source
			file has changed, re-branch the source file on top
			of the target file instead of scheduling a resolve.
		-Ds     If the source file has been deleted and the target
			file has changed, delete the target file instead of
			scheduling a resolve.
		-d	Synonym for '-Dist'.
		-i/-I	Use the first revision of the source as the base for
			baseless merges, rather than an empty file.
		-o	Synonym for '-Ob'.
		-t	Force filetype and attributes to be copied from the
			source file rather than scheduling resolves.

    p4 jobspec presets
	Preset values for job non-optional fields that undergo variable
	expansion, in addition to the simple $now, $blank, and $user
	substitution.  The variables supported include: %%old%% (previous
	contents of field), %%action%% (what caused the update of the form),
	%%n%% (a newline), %%date%% (date and time), %%day%% (date only),
	and %%user%%.

    p4 jobspec codes over 10000
	Jobspec field codes are required to be in the range 100-199 or over
	10000.

    p4 merge [options] -b branch [-r] [toFile[revRange] ...]
    p4 merge [options] -b branch -s fromFile[revRange] [toFile ...]
    p4 merge [options] -S stream [-P parent] [-F] [-r] [toFile[revRange]...]
	The typical use of 'p4 merge' is to merge into the current stream
	from its parent stream (or the stream specified by --from). More
	advanced usages of 'p4 merge' are possible, using the flags as
	described by 'p4 help integ'.

    p4 merge3
	Three-way file merge.
	See 'p4 help merge3'.
	Undoc '-d' flags for 'resolve' (below) apply to 'merge3' as well.

    p4 obliterate [ -iI ]
	Tells obliterate to synthesize new integration records to attempt
	to bridge the gaps left by the obliteration of intermediate branches.
	If '-i' is used, only the initial branch records are considered; if
	'-I' is used, all records are considered.  Note that it is possible
	to end up with more integration records than you started with.

    p4 pull -I
	The '-I' flag specifies the pull interval in milliseconds.

    p4 push [-p target -I]
	The -p flag specifies the address of the target server, overriding
	the address from the remote spec specified by the -r flag.

	The -I flag excludes integration records for the new revisions.

    p4 resolve -dg
	The'-dg' flag makes the default merge algorithm more guarded about
	merging adjacent chunks.  When this option is specified,
	more conflicts are returned,  especially where changes in both
	'yours' and 'theirs' are identical but then diverge.

    p4 resolve -dt
	Enables two-way merging, similar to integrate between unrelated files.
	The base is ignored, and all diffs are treated as conflicts.

    p4 resolve -dx
	The '-dx' flag makes the default merge algorithm less likely to
	remove duplicate lines when merging adjacent chunks.  When this
	option is specified, blocks that contain identical lines in both
	'yours' and 'theirs' but are not totally the same will be fully
	inserted. This behavior tends to perform similar to 'diff3'.

    p4 resolve -e
	Translates all 'ignored' results to 'edit from', leaving the contents
	unchanged.  The resulting revision is a candidate for integration back
	to its source, which has the effect of undoing the 'ignored' change.

    p4 retype
	Change revision or server archive type.
	See 'p4 help retype'.

    p4 search
	Search index.
	See 'p4 help search'.

    p4 snap
	Snap archive lazy copies.
	See 'p4 help snap'.

    p4 spec
	Edit spec definitions.
	See 'p4 help spec'.

    p4 storage [ -R [ -f | -S ] | -r | -s ]
	These options to 'p4 storage' all have significant performance
	impacts on a running server and extreme caution must be exercised
	before considering invoking one of them. They all require 'super'
	access.

	The -R flag discards and rebuilds both storage tables. The discard
	actions are journalled. This in effect reruns the storage upgrade
	process performed when migrating to server release 19.1. The upgrade
	process has two phases. During phase one all commands that access any
	of the storage or revision tables will be blocked. Once the upgrade
	process enters phase two, then only commands that remove revisions will
	be prohibited. When this option is invoked on a commit server, the
	upgrade request will be transmitted to any downstream edge servers,
	triggering them to instigate a storage upgrade. Rather than
	using this option, the recommended method of rebuilding the storage
	tables is to shutdown the server and use:
	'p4d -r $P4ROOT -xU BuildStorage191'.
	During the execution of a 'p4 storage -R' command, it can be terminated
	once it enters phase 2 of the storage upgrade process. When it is
	terminated in this way, the server will terminate in an orderly
	manner, allowing the upgrade process to be restarted by using the
	'-r' option to p4 storage. If instead, a full restart is required,
	then the '-f' needs to be used to force the command to discard
	the existing state of the upgrade process. e.g.
	'p4 storage -R -f'

	The -S option can be provided with -R to only discard and rebuild
	the shelf storage table (db.storagesh). As it only deals with the
	shelf revisions and they are handled in phase 1 of the storage
	upgrade process, no phase 2 of the upgrade process is performed.
	Similar to just the -R option, the command will block all user
	requests attempting access to any revision table while active.
	This option will trigger a storage upgrade of any downstream edge.

	The -r option to 'p4 storage' restarts a previously interrupted
	storage upgrade process initiated by a call to 'p4 storage -R'.
	The storage upgrade process will be continued from where it
	had reached when it was terminated. The storage upgrade process
	is also resumed when the server is restarted.

	The -s flag requests a consistency check of the storage and revision
	tables. It checks that the reference count in the storage records
	correctly numbers the active revision records. This command share
	locks all the revision and storage tables and so blocks all commands
	that wish to update any of those tables whilst this command is active.
	The command builds an in memory structure to track the reference
	counts and is therefore limited by the amount of physical memory
	on the server. Any inconsistancies are reported in the output
	of the command.

	You are strongly encouraged to only use these p4 storage options
	after discussion with Perforce Support.

    p4 streamlog -H
	The documented '-h' follows both 'branch' and content.
	This undocumented '-H' follows only content.
	See 'p4 help streamlog'.

    p4 submit -c <change> --forcenoretransfer
	This option instructs submit to bypass the client file transfer if
	the archive is found to be already present on the server. If the
	archive is found, submit will bypass the client file transfer and
	commit the archive content even if it differs from the client file
	content. This option is only allowed if the undocumented tunable
	value 'submit.forcenoretransfer' is set to 1 or higher. If the
	tunable is set to 2, 'submit -c N' defaults to --forcenoretransfer.
	Files submitted without a file transfer are recorded with no digest
	value; you may set the digest later if desired, for example by
	running verify or by other means. NOTE: bypassing file transfer
	is a very advanced feature requiring specific workflows to be
	used and with specific restrictions to its operation.

    p4 submit -t
	Tamper-checks branch/sync files and files resolved with 'at'
	(copy from/lazy-copied files). This check can significantly slow
	submit.
	Imported files are never tamper-checked.

    p4 unload -o localFile
	The -o localFile flag redirects the output to the specified file on
	the client filesystem. The client, label, or stream is not actually
	unloaded; instead, the data that would have been written to the
	unload depot is written to the local file.

    p4 unsubmit -c N
	Unsubmits the specified changelist by changelist number. Each
	changelist is unsubmitted independently, but if you specify
	multiple interdependent changelists, you must specify them in
	descending changelist order.

    p4 -ztzoffset=N verify
	Operating a replica in a timezone other than the master's timezone
	can cause files of type +k which use $Date$ or $DateTime$ keywords
	to verify as BAD! Specify the master's timezone offset to cause the
	replica to use that timezone for keyed file verification. This value
	does not, however, affect the time zone name shown by $DateTimeTZ$.

    Overlay mapping legacy behavior
	The map.overlay.legacy configurable can be set in order to revert
	overlay mapping behavior to its pre-2023.1 state. The pre-2023.1
	behavior caused a number of confusing and often wrong results,
	so setting this configurable to a non-zero value is not recommended.
	Non-zero values for map.overlay.legacy are also incompatible with the
	Sparse Stream feature.

    Predefined Trigger Actions
	The predefined trigger actions 'pass' and 'fail' are processed
	by the server without calling any external scripts. 'pass' will
	always succeed. 'fail' will always fail and return the expanded
	argument list as the error message.

    Relative revisions @<, @<=, @>, @>=, @=
	            and #<, #<=, #>, #>=, #=
	Anywhere a revision range can be specified, the revisions can be
	modified with relative operators.  Relative operators on #head
	(other than = and <=) aren't supported.  A maximum of 4 revision
	specifications (separated by ,s) are allowed: the implied operator
	on the first is >=; thereafter <=.  You must quote < and > for the
	command-line shell.  See 'p4 help revisions' for a description of
	revision specifications.

    Revision actions #add, #edit, #delete, #branch, #integrate, #import
	Selects all revisions with the specified action.  Actions are not
	relative, which means that #action cannot be used in a revision range.
	For many commands, when multiple revisions of the same
	file are selected, the highest revision is used.

    FileType modifier +T
	The +T modifier stores the revisions in the server in a database
	file called tiny.db, which is useful for lots of very small
	files.  The types ttext and tbinary are equivalent to text+T and
	binary+T.  tiny.db is not backed up with a normal checkpoint.
	Backups are done via 'p4d -xf 857 > tiny.ckp' and restores with
	'p4d -xf 857 tiny.ckp'

    Versioned character set
	Set the server.filecharset configurable to 1 in order to version
	the charset of individual unicode files along with the filetype.
	With this configurable enabled, each unicode file has a charset
	associated with it that will be used instead of the local P4CHARSET
	when reading and writing the file in the client workspace.  This
	versioned charset is set to the local P4CHARSET on add by default
	and may be changed explicitly by using the -Q flag on add, edit, and
	reopen.  The 'p4 reopen -X -Q charset' command will convert the client
	file's contents into the new charset; otherwise it is assumed that the
	client file already matches the newly specified charset.  Note that
	this feature requires client-side support; if not all client
	applications in use support this feature, confusing behavior may
	result.

    Jobspec format
	The 'p4 jobspec' form can have an additional field 'Formats:' to
	help P4 visual clients display forms properly.  Each row of the Format
	field consists of three words: field, order, and format.  The order is
	an integer (starting at 1) indicating the display order (the display
	order is otherwise undefined).
	The format is: normal, L, R, or I; for full line, left half,
	right half, indented.  If a right field follows a left, they
	are displayed on the same line.

    JobStatus field on 'p4 change', 'p4 submit'
	'p4 change' and 'p4 submit' forms have a 'JobStatus' field that is not
	normally displayed.  This field sets the fix status for all jobs,
	overriding the default (which is 'closed').  A job's fix status can be
	set using the 'p4 change -s' and 'p4 submit -s' commands.

    p4 -zmaxLockTime=n command
    p4 -zmaxOpenFiles=n command
    p4 -zmaxResults=n command
    p4 -zmaxScanRows=n command
    p4 -zmaxMemory=n command

	Overrides the user's maxLockTime/maxOpenFiles/maxResults/maxScanRows
	for the command. Note that the server.commandlimits configurable
	affects the operation of these command-level overrides.

    p4 -e
	Extended version of the '-s' output that displays more detailed
	information about each error/info message received, including the
	formatting strings, unique error codes, and severity values that
	are available to API users.

    p4 -E P4VAR=value
	Override the value of an arbitrary environment/config variable,
	as in a P4CONFIG file.  Multiple instances of this flag can be used.
	Useful for variables without dedicated override flags (such as P4DIFF).

    p4 -F formatstring
	Override the formatting string of server messages, allowing output
	to be customized for easier scripting.  Use the '-e' global option
	to see examples of built-in message strings and the dictionary of
	variables associated with each message.  The '-F' option also works
	with the dictionary provided by tagged (-Ztag) output mode.

    p4 --field Field=replacement_text
    p4 --field Field+=extra_text
	Modify the contents of a spec returned by p4 [specname] -o.  The
	modified spec can be piped directly to another command that expects
	a formatted spec, e.g. p4 [specname] -i.)"
R"(

    p4 -R
	Formats all output (and batch input for form commands with -i)
	as marshalled Ruby hash objects.

    p4 -x cmdfile run
	Runs the commands listed in cmdfile one after the next, using a
	single connection to the server.

    p4d -Cn
	Forces the server to operate in case-sensitive mode (n=0) on Windows
	or case-insensitive mode (n=1) on Unix.  n=2 is an experimental
	'hybrid' order that folds case while preserving uniqueness.
	Attempting to change modes after a database is created produces an
	error.

    p4d -In Name
	Directs the server to identify itself with a unique name (P4NAME).
	Enclose the name in quotes if spaces are included.
	Both 'p4d -In name' and P4NAME are deprecated in favour of using
	the 'p4 serverid' command (see 'p4 help serverid').

    p4d -jds dumpfile
	Dumps the server metadata to the specified file. Unlike p4d -jd,
	ignores the table indexes and reads in file system order rather than
	data order.  This option can be useful for recovery if the btree
	index structure becomes corrupt.  This recovery procedure is not
	meant for normal production operation.  Restoring the resulting
	dump file does not compact the btrees as effectively as restoring
	from regular checkpoints does, and such restores take longer to
	complete.

    p4d -xf bugno
	Updates the server data to fix problems due to the
	specified bug number.  Valid values of bugno are as follows:

		857 -- checkpoint/restore tiny.db
		925 -- working and locks table out of sync
		3104 -- early 99.2 resolve bug
		4164 -- early 2000.1 submit bug
		12904 -- replace / with _ in client names
		18362 -- 2005.2 replace 'check' trigger command
		43361 -- 2012.2 - recreate db.have from db.working
		71819 -- 2013.2 - clears unneeded/invalid charset data
		71917 -- 2014.2 - promoted shelved changes on commit server
		103829 -- 2020.2 - disable all authentication extensions
		123929 -- fix db.bodtextcx inconsistencies

    p4d -xf update.change.identity //path/to/files/...
	Retrospectively applies the server's configured submit.identity
	algorithm to generate identities for changes that may have touched
	the specified files and have not yet been assigned an identity.

    p4d -xf disable.extension uuid
	Disables the specified extension and all its configurations, this is
	primarily used if a broken extension has taken control of 'pre-user-*'
	preventing the use of 'p4 extension --delete'.

    p4d -xU upgrade
	Re-runs the named upgrade step that is normally done automatically by
	p4d -xu.  To display a list of steps, issue the 'p4d -xU help' command.
	This option can be used to rebuild certain tables (like db.storage).

    p4d -xi[n] [-o output]
	Produces a jnl.invalid-utf8 file if any invalid entries found.
	Use -o flag to specify a filename other than jnl.invalid-utf8.
	Specify 'n' to only validate database without switching to Unicode.

    p4d -xr
	Does a low-level verification and repair of database files.
	Use only as a last resort if checkpoint and journal are not
	available.

    p4d -xrU
	Resets the UnlockCount of database files, but makes no other repairs.
	Use only as a last resort if checkpoint and journal are not
	available.

    p4d -xrc
	Consolidates leaf pages and recovers lost internal nodes in the btree.

    p4d -p <serverlevel> -jd
	The -p flag directs the server to dump metadata in a form compatible
	with an older server version.  The earliest version that might work
	is 2005.1, which is server level 19.

    p4d -m rcsfiles...
	Dumps RCS metadata from files to a format suitable for the
	CVS-to-Perforce conversion script.

    p4d -M readonly [options]
	The -M readonly option, used only with replica servers, directs
	the replica server to only accept readonly metadata commands.
	This overrides the db.replication configurable.

    p4d -D readonly|shared|ondemand|cache|none [options]
	The -D flag is used only with replica servers to specify the
	approach for handling depot file content. A replica running
	with '-D readonly' automatically transfers copies of the target
	server's files. A replica running with '-D shared' or '-D ondemand'
	either shares the same physical archives as the target or uses
	external archive synchronization from the target. A replica running
	with '-D cache' only requests file transfer as needed by user
	commands. A replica running with '-D none' rejects any command that
	requires access to depot files. This overrides the lbr.replication
	configurable.

    p4p -k ...
	Start the P4 Proxy without cache timestamp checking.
	The P4 Proxy normally uses a timestamp on cached revisions to check
	that the right revision is delivered if an obliterate deletes a
	revision that is subsequently replaced with an identically-numbered
	revision.  Using this option causes problems if head revisions are
	obliterated.  However, the proxy cache can be pre-loaded with the
	server's archive files by specifying this flag when you start the
	proxy.

    p4p -w ...
	Start the P4 Proxy in read-only cache mode.  In this mode the proxy
	never updates its cache, which is useful if the cache is updated
	through a WAN NFS or other system-level remote filesystem method.

    P4LOG=syslog
	Uses syslog instead of the named error log file for server error
	logging.  Must be set in server's environment.

    P4DEBUG=server=2
	Logs server commands to the file configured by P4LOG.  The server=2
	option logs command completion. Must be set in server's environment.
	To disable all command logging, set server=0.

    P4 Server configurables (unsupported)
    P4DEBUG=configurables
	For information about supported server configuration variables, see
	'p4 help configurables'.

	The following server parameters can be tuned using 'p4 configure set'
	or by setting P4DEBUG to a list of variable=value pairs separated by
	commas. Change these values only as directed by Technical Support.

	Values that are too large or small are silently corrected.

	Name               Default Use
	----               ------- ---
	db.internal.repair       0 Runtime BTree consolidation and recovery
	db.isalive             10K Rows scanned before maxLockTime check
	db.page.migrate		 0 Avoid allocating pages at end of btree
	db.reorg.misorder	80 BTree reorg percent out of order trigger
	db.reorg.occup		 8 BTree reorg percent of page free remaining
	db.trylock               3 Attempts to avoid locks that block
	dbarray.putcheck        4K Every this many rows...
	dbjournal.wordsize      4K Journal word processing size
	dbopen.cache            96 Pages of cache for db.* files
	dbopen.cache.wide      192 Pages of cache for db.integed
	dbopen.freepct           0 Free page space on p4d -jr
	dbopen.pagesize         8K Page size of db.* files
	diff.binary.rcs          0 Diff binary+D (RCS) files as text
	diff.slimit1           10M Longest diff snake; smaller is faster
	diff.slimit2          100M Longest diff snake for smaller files
	diff.sthresh           50K Use slimit2 if lines to diff < sthresh
	dm.batch.domains         0 'labels path' scan in label intervals
	dm.change.restrict.pending
	                         0 Description for pending restricted changes
	                           0: Only visible by owner of change
	                           1: Visible with list access to file(s)
	dm.changes.thresh1     50K 'changes -mx path' uses db.revcx if...
	dm.changes.thresh2     10K ...if < thresh2 of thresh1 db.rev match
	dm.changeview.openable   0 Legacy (pre-16.1) readonly behavior
	dm.copy.movewarn         0 verbose 'p4 copy' warnings for moved files
	dm.dvcs.allowcasemismatch 0 Allow case-handling mismatch for push/fetch
	dm.fstat.outputthreshold
	                       10K Output threshold for fstat
	dm.fstat.maxcontent     4M Maximum number of bytes of an file revision
	                           that 'p4 fstat -g'%' will display
	dm.grep.maxlinelength 4096 Maximum line length that can be searched
	dm.grep.maxcontext      1K Maximum size of context allowed for grep
	dm.integ.engine          3 'p4 integrate' engine version
	                           2: 2006.1 integration engine
	                           3: 2013.2 integration engine
	dm.integ.maxact       100K Max db.integed edges considered for a file
	dm.integ.maxbranch     100 Max branches explored for base selection
	dm.integ.tweaks          0 Modify integrate behavior (engine=3 only):
	                           1: Treat all 'copy' records as 'merge'
	                           2: Treat all 'ignore' records as 'edit'
	                           4: Retain credit for copied-over edits
	                           8: Force convergent merge of all edits
	                          16: Legacy (pre-2011.1) resolve behavior
	                          32: Readded files not integrated with moved
	                              files
	dm.integ.undo            0 Enable re-integration of undone changes
	dm.isalive             50K Rows scanned before client connection check
	dm.maxkey               1K Longest identifier (client, label, etc)
	dm.protects.allow.admin  0 Allow admin to use -agu flags (protects)
	dm.protects.hide         0 Hide exclusionary protections (protects)
	dm.resolve.ignoredeleted 0 Treat auto-resolve changes to deleted files
	                           as ignored
	dm.quick.clients       10M Release lock if all needed rows buffered
	dm.quick.domains        1M Release lock if all needed rows buffered
	dm.quick.have           1M Release lock if all needed rows buffered
	dm.quick.integ          1M Release lock if all needed rows buffered
	dm.quick.resolve        1K Release lock if all needed rows buffered
	dm.quick.rev          100K Release lock if all needed rows buffered
	dm.quick.working        1K Release lock if all needed rows buffered
	dm.status.matchlines    80 Min %% matching lines to match moves
	dm.status.matchsize     10 Max size %% difference to match moves
	dm.revcx.thresh1        4K Path@change uses db.revcx if thresh2 ...
	dm.revcx.thresh2        1K ...of thresh1+thresh2 rows match path
	dm.user.insecurelogin    0 issue login tickets that work on all hosts
	filesys.cachehint        0 preserve buffer cache for db files (linux)
	filesys.maketmp         10 Max attempts to find unused temp name
	filesys.maxmap       1000M Use read rather than mmapping big files
	filesys.maxsymlink      1K Symlink maximum content length
	filesys.maxtmp          1M Rollover for creating temp file names
	filesys.windows.lfn      1 Enable Windows filename > 260 characters
	map.joinmax1           10K Produce at most map1+map2+joinmax1
	map.joinmax2            1M Produce at most joinmax2
	map.maxwild             10 Maximum number of wildcards per line
	map.overlay.legacy       0 See 'Overlay mapping legacy behavior' above
	net.bufsize             4K Network I/O buffer size
	proxy.deliver.fix	 1 Enable fix for proxy hang
	rcs.maxinsert           1G Max lines in RCS archive file
	rpc.himark            2000 Max outstanding data between server/client
	rpc.lowmark            700 Interval for checking outstanding data
	rpc.ipaddr.mismatch      1 Check for client address mismatch
	rpl.awaitjnl.count     100 Max count of waits for journal data (-i 0)
	rpl.awaitjnl.interval   50 Millisecs to wait for journal data (-i 0)
	rpl.journal.ack          0 In DCS, number of ACKs requested
	rpl.journal.ack.min      0 In DCS, number of ACKs required
	rpl.pull.reload      60000 Interval in ms for pull to reload config
	server.filecharset       0 Enable per-file charset storage
	serverlog.name.N      none Alias name for this log file
	serverlog.events.N    none Events that should write to this log
	spec.custom              0 If > 0 allow modifications to spec forms.
	submit.forcenoretransfer 0 Allow submit --forcenoretransfer option
	switch.stream.unrelated  0 If 1 can switch to another stream hierarchy
	sys.rename.max          10 Limit for retrying a failed file rename
	sys.rename.wait       1000 Timeout in ms between file rename attempts)"
# ifdef HAVE_SMARTHEAP
# ifdef OS_NTX86
R"(
	sys.memory.poolfree     1M Free memory retained per heap, 1K blocks
	sys.memory.procfree     1M Free memory retained in process, 1K blocks)"
# else
R"(
	sys.memory.poolfree     4M Free memory retained per heap, 1K blocks
	sys.memory.procfree  undef Free memory retained in process, 1K blocks)"
# endif
R"(
	cmd.memory.poolfree      0 Same as sys.memory.poolfree, not persistent
	cmd.memory.procfree      0 Same as sys.memory.procfree, not persistent)"
# endif
R"(

    P4 client configuration (unsupported)
	May be set in P4CONFIG or P4ENVIRO files or via 'p4 set'

	filesys.detectunicode	 1 Set to 0 to prevent unicode file detection
	filesys.detectutf8	 2 Set to 0 to prevent utf8 file detection
				   Set to 1 for historic behavior
				   Set to 2 to only detect utf8 with a BOM
	filesys.utf8bom		 1 Set to 0 to prevent writing utf8 files BOM
				   Set to 1 to write utf8 files with a BOM
				   Set to 2 to write utf8 BOM only on Windows

    P4CHARSET
	Some P4CHARSET values exist which are undoc/unsupported for now

	winoem		Windows Codepage 437 (US Command Window)
)"
};

ErrorId MsgHelp::HelpEnvironment = { ErrorOf( ES_HELP, 16, E_INFO, EV_NONE, 0 ),
R"(
    Environment variables used by P4:

    Variable         Defines                         More information
    --------         -------                         ------------------------
    P4ALIASES        Name of aliases file            p4 help aliases
    P4ALTSYNC        Alternative sync agent          P4 Command Reference
    P4CHARSET        Client's local character set    p4 help charset
    P4COMMANDCHARSET Client's local character set
                     (for command line operations)   p4 help charset
    P4CLIENT         Name of client workspace        p4 help client
    P4CLIENTPATH     Directories client can access   P4 Command Reference
    P4CONFIG         Name of configuration file      P4 Command Reference
    P4DIFF           Diff program to use on client   p4 help diff
    P4DIFFUNICODE    Diff program to use on client   p4 help diff
    P4EDITOR         Editor invoked by p4 commands   p4 help change, etc
    P4ENVIRO         Name of environment file        P4 Command Reference
    P4EXTENSIONS     Name of client Extension file   P4 Command Reference
    P4HOST           Name of host computer           p4 help usage
    P4IGNORE         Name of ignore file             P4 Command Reference
    P4LANGUAGE       Locale for server messages      p4 help usage
    P4LOGINSSO       Client side credentials script  p4 help triggers
    P4MERGE          Merge program to use on client  p4 help resolve
    P4MERGEUNICODE   Merge program to use on client  p4 help resolve
    P4PAGER          Pager for 'p4 resolve' output   p4 help resolve
    P4PASSWD         User password passed to server  p4 help passwd
    P4PORT           Port to which client connects   p4 help info
    P4SSLDIR         SSL server credential directory P4 Command Reference
    P4TICKETS        Location of tickets file        P4 Command Reference
    P4TRUST          Location of SSL trust file      P4 Command Reference
    P4USER           P4 user name                    p4 help usage
    PWD              Current working directory       p4 help usage
    TMP, TEMP        Directory for temporary files   P4 Command Reference

    For details about configuring Windows settings, issue the 'p4 help set'
    command.  The syntax for setting an environment variable depends on the
    OS/shell.  Many shells permit you to set shell variables separately from
    environment variables, but P4 cannot access the shell variable, only the
    environment variable.

    Variables of primary interest to typical users are:
    $P4CLIENT, $P4PORT and $P4PASSWD.

    Environment variables used by the P4 Server:

    Variable        Defines                          More information
    --------        -------                          ------------------------
    P4AUDIT         Name of server audit file        p4d -h
    P4AUTH          Authentication server address    P4 Command Reference
    P4CHANGE        Global changelist server address P4 Command Reference
    P4DEBUG         Server diagnostic trace flags    P4 Command Reference
    P4DESCRIPTION   Text description for server      P4 Command Reference
    P4JOURNAL       Name of server journal file      p4d -h
    P4LOG           Name of server log file          p4d -h
    P4NAME          Unique server name               P4 Command Reference
    P4PORT          Port on which server listens     p4d -h
    P4ROOT          Server root directory            p4d -h
    P4PROOT         Proxy Server metadata directory  p4p -h
    P4PCACHE        Proxy Server archives directory  p4p -h
    P4SSLDIR        Server SSL credentials directory P4 Command Reference
    P4TARGET        Target server for proxy/replica  P4 Command Reference
    P4TICKETS       Location of tickets file         P4 Command Reference
    P4TRUST         Location of SSL trust file       P4 Command Reference
    TMP, TEMP       Directory for temporary files    P4 Command Reference

    To set server environment variables persistently, use the 'p4 configure'
    command.  Note that P4ROOT and TMP/TEMP cannot be set using 'p4 configure'.
    Also note that server environment variables are not searched for in
    configuration or environment files, i.e. P4CONFIG and P4ENVIRO are ignored.
)"
};

ErrorId MsgHelp::HelpFiletypes = { ErrorOf( ES_HELP, 17, E_INFO, EV_NONE, 0 ),
R"(
    File types supported by P4:

	P4 file type determines how the file is handled on both the client and
	the server.

	A filetype can be specified as 'type', with modifiers as 'type+mods'
	or as just the modifiers '+mods'.

	The following are the base P4 filetypes:

	    Type        Client Use              Server Storage
	    ----        ----------              --------------
	    text        newlines translated     deltas in RCS format
	    binary      raw bytes               compressed binary
	    symlink     symbolic link           deltas in RCS format
	    apple       Mac resource + data     compressed AppleSingle
	    resource    Mac resource fork       compressed binary
	    unicode     newlines translated     deltas in RCS format
	                                        stored as UTF-8
	    utf8	newlines translated     deltas in RCS format
	                client file UTF-8  	stored as UTF-8
	    utf16	newlines translated     deltas in RCS format
	                client file UTF-16      stored as UTF-8

	                Files of type utf16 are stored in the depot in UTF-8.
	                These files are in utf16 in the client workspace.
	                The automatic type detection requires a BOM be present
	                at the start of the file.  Files without a BOM are
	                assumed to be in client byte order.  When utf16 files
	                are written to a client, they are written with a BOM
	                in client byte order.

	The following are the modifiers:

	    Modifier    Meaning
	    --------    -------
	    +m		always set modtime on client
			(overrides client's nomodtime)
	    +w          always writable on client
	    +x          exec bit set on client

	    +k		$Keyword$ expansion of Id, Header, Author
			Date, DateUTC, DateTime, DateTimeUTC, DateTimeTZ
			Change, File, Revision
	    +ko         $Keyword$ expansion of ID, Header only
	    +l		exclusive open: disallow multiple opens

	    +C          server stores compressed file per revision
	    +D          server stores deltas in RCS format
	    +F          server stores full file per revision, uncompressed
	    +S          server stores only single head revision
	    +S<n>       server stores <n> number of revisions, where <n>
	                is a number 1-10 or 16,32,64,128,256,512.
	    +X		server runs archive trigger to access files

	The following aliases for filetypes are supported for backwards
	compatibility:

	    Type        Is Base Type         Plus Modifiers
	    --------    ------------         --------------
	    ctempobj    binary               +Sw
	    ctext       text                 +C
	    cxtext      text                 +Cx
	    ktext       text                 +k
	    kxtext      text                 +kx
	    ltext       text                 +F
	    tempobj     binary               +FSw
	    ubinary     binary               +F
	    uresource   resource             +F
	    uxbinary    binary               +Fx
	    xbinary     binary               +x
	    xltext      text                 +Fx
	    xtempobj    binary               +Swx
	    xtext       text                 +x
	    xunicode    unicode              +x
	    xutf16	utf16                +x

	'p4 add', 'p4 edit', and 'p4 reopen' accept the '-t filetype'
	flag to specify the filetype.  If you omit the -t flag, 'p4 add'
	determines filetype using its own logic and the name-to-type
	mapping table managed by 'p4 typemap', if configured.
	'p4 edit -t auto' will determine the filetype by the above mentioned
	logic.

	By default, 'p4 edit' and 'p4 reopen' reuse the current file
	filetype, and 'p4 add' determines the filetype by examining the
	file's contents and its execute permission bits.

	If a filetype is specified using only '+mods, then that filetype
	is combined with the default.  Most modifiers are simply added to
	the default type, but the +C, +D, and +F storage modifiers replace
	the storage method.  To remove a previously assigned modifier, the
	whole filetype must be specified.
)"
};

ErrorId MsgHelp::HelpJobView = { ErrorOf( ES_HELP, 18, E_INFO, EV_NONE, 0 ),
R"(
   P4 job views:

	A job view is an expression that selects jobs according to word
	and date matches.  Job views are used by the 'p4 jobs' -e flag
	to select the jobs to be displayed.  The 'p4 user' form contains
	a JobView field, which selects the jobs that are added to newly-
	created changelists.

	'p4 job' indexes all whitespace-separated words and all punctuation-
	separated words within those words.  So 'sub-par' is entered into
	the index as 'sub', 'par', and 'sub-par'.  Case is not considered.
	'p4 job' separately indexes all date fields in a way that enables
	you to search for a range of dates.

	In its simplest form, a job view is a list of words separated by
	spaces, used to select jobs. For a match to occur, all the specified
	words must appear somewhere in the job, except for date fields:

		JobView: GUI redrawing bug
		p4 jobs -e 'GUI redrawing bug'

	To match the contents of a particular field, use the 'field=word'
	syntax:

		JobView: GUI redrawing status=open

	Logical operators & (and), | (or), ^ (not), and () (grouping) can
	also be used. Spaces are treated as a low-precedence 'and' operator:

		JobView: redrawing (type=bug|type=sir) status=open

	The ^ (not) operator can be used only in conjunction with the & (and)
	or space (and) operators:

		JobView: type=bug & ^status=closed

	The comparative operators >, >=, <, <=, and = succeed if any
	word in the field matches, so only the = operator
	is useful for matching fields that contain blocks of text:

		JobView: priority<=b description=gui

	Text searches can use the * wildcard, which matches anything:

		JobView: redraw* type=bug

	To match operator characters, escape them with \. To match the
	backslash character, escape it with an additional backslash (\\).

	Date fields can be searched using comparative operators.  Specify
	dates using the format yyyy/mm/dd or yyyy/mm/dd:hh:mm:ss.  If you
	omit the time, the equality operators (=, <=, >=) match the whole day:

		JobView: reported_date>=1998/01/01 status=closed

	Text field comparisons are done alphabetically. Date field
	comparisons are done chronologically.
)"
};

ErrorId MsgHelp::HelpRevisions = { ErrorOf( ES_HELP, 19, E_INFO, EV_NONE, 0 ),
R"(
    Specifying file revisions and revision ranges:

	You can limit the effect of some commands to a single file revision
	or a range of revisions, by appending one of the following revision
	specifiers to the file specification:

	    #rev - A revision number or one of the following keywords:
	    #none - A nonexistent revision (also #0).
	    #head - The current head revision (also @now).
	    #have - The revision on the current client.
	    @change - A change number: the revision as of that change.
	    @client - A client name: the revision on the client.
	    @label - A label name: the revision in the label.
	    @date - A date or date/time: the revision as of that time.
		    Either yyyy/mm/dd or yyyy/mm/dd:hh:mm:ss
		    Note that yyyy/mm/dd means yyyy/mm/dd:00:00:00.
		    To include all events on a day, specify the next day.

	Only matching revisions are processed. If you use a label to refer to
	a file that the label doesn't contain, the file is not processed.

	If you omit the revision specification, the default is usually #head.

	If you specify a revision or range and omit the file specification,
	the command affects all relevant files. For example, #head refers
	to the head revision of all files in the depot, and @label refers
	to the revisions of all files in the specified label.

	Files that have been shelved can be accessed by the 'p4 diff',
	'p4 diff2', 'p4 files' and 'p4 print' commands using the revision
	specification '@=change', where 'change' is the pending changelist
	number.

	Some commands accept a range of revisions, specified as the start
	and end revisions separated by a comma. If a single revision is
	specified where a revision range is expected, the revision specifies
	the end revision, and the start defaults to revision #1.

	'p4 obliterate' is an exception: a single revision means only
	that revision.

	If you omit the revision specifier, the default is all revisions.
)"
};

ErrorId MsgHelp::HelpViews = { ErrorOf( ES_HELP, 20, E_INFO, EV_NONE, 0 ),
R"(
    P4 views:

	A P4 view maps file names from the depot to the client workspace
	(client view) or to another part of the depot (branch view), or selects
	a subset of the depot (label view).

	A view is one or more mappings, and each mapping is a pair of file
	names on a line.  The left side always refers to the depot namespace;
	the right side refers to the client or depot namespace.  Each name
	is in P4 syntax: it begins with //, followed by the client or depot
	name, then the file name. If the path contains whitespace, it must be
	double-quoted.  P4 syntax uses forward slashes (/) as directory
	separators.  Example: "//depot/a dir/file.c"

	Mappings can contain wildcards.  Wildcards on the right and left sides
	of a mapping must match. Valid wildcards are:

		*			Matches any characters except /
		... 			Matches any characters, including /
		%%%%1 to %%%%9 		Like *, but used to reorder the path

	If more than one mapping line refers to the same files, the later
	mapping line overrides the earlier one.

	A mapping line that begins with a + overlays the later mapping on
	an earlier one: if files match both the earlier and later mappings,
	then the file matching the later mapping is used. Overlay mappings
	are supported only for client views, and enable you to map multiple
	server directories to the same client directory.  When multiple
	directories are overlaid in this way, only the last one is writable.

	A mapping line that begins with a & does not override earlier mappings
	for the same depot path, allowing multiple 'ditto' versions of the
	same depot file to be synced to one workspace.  Ditto mappings are
	supported only for client views, and files mapped in this way are
	read-only.

	To exclude matching files, precede the mapping with a minus sign (-).
)"
};

ErrorId MsgHelp::HelpMaxResults = { ErrorOf( ES_HELP, 21, E_INFO, EV_NONE, 0 ),
R"(
    Limiting data access:

	To avoid swamping servers with requests that result in large datasets,
	the P4 super-user can limit the amount of data that the server handles
	for client operations.

	The following limits can be set: MaxResults, MaxScanRows, MaxLockTime
	MaxOpenFiles, and MaxMemory.  If an operation exceeds any of these
	limits, the whole operation fails.

	MaxResults limits the number of rows of result data buffered and
	prevents the server from using excessive memory.

	MaxScanRows limits the number of rows of data considered and prevents
	the server from making large-scale scans.

	MaxLockTime limits the amount of time spent during data scans to
	prevent the server from locking tables for too long.  MaxLockTime
	starts counting when the first table is locked, and includes time
	spent waiting for any subsequent table locks.

	MaxOpenFiles limits the number of files that can be opened by a single
	operation and prevents users from accidentally opening an excessive
	portion of the repository.

	MaxMemory limits the number of megabytes of memory that a command may
	use.  Note that this is not a hard limit, but best-effort.
	This applies to P4 servers running on Linux or Windows, but is
	not supported for MacOS.

	To set limits for groups of users, issue the 'p4 group' command.

	You may wish to additionally set server.commandlimits=2 in order to
	disable overriding these settings on a per-command basis.

	Each group has MaxResults, MaxScanRows, MaxLockTime, MaxOpenFiles and
	MaxMemory fields, which limit the resources committed to operations
	performed by members of the group.  For these fields, 'unlimited'
	or 'unset' means no limit for that group.  An individual user's
	limit is the highest of any group with a limit to which he belongs,
	unlimited if any of his groups has 'unlimited' for that field, or
	unlimited if he belongs to no group with a limit.

	For naive users, set MaxResults to a value that is larger than the
	number of files likely to reside in any of their client workspaces.
	Set MaxScanRows to MaxResults multiplied by the average number of
	revisions per file in the repository.  As a rule, neither need to
	be smaller than 10000 or 50000 (respectively), and values of 50000
	and 250000 are probably sufficient to prevent accidental misuse
	without interfering with normal use.  Set MaxLockTime to a value
	in the 1000-30000 (1-30 second) range.  Set MaxOpenFiles to a value
	that is smaller than MaxResults, but larger than the number of files
	likely to exist in a single project.

	Most commands which enforce entity limits count either files or
	revisions of files. A single file may have many revisions, so the
	number of revisions of a set of files is always at least as large
	as the number of files in that set. Some commands track other types
	of entities, as explained below.

	The following table lists the commands that are controlled by these
	limits, the entity that is counted, and tells you how to adjust the
	operation to run within the limit.

	Command                 Counting        Reduce output with:
	-------                 --------        -------------------
	p4 add                  files * **      see (1)
	p4 annotate             revisions       see (2)
	p4 branches             domains *       see (6)
	p4 clients              domains *       see (6)
	p4 changes              changes         -m maxChanges
	p4 changes files        revisions       see (2)
	p4 copy                 files **        see (1)
	p4 delete               files * **      see (1)
	p4 describe             files *         see (3)
	p4 diff                 files *         see (1)
	p4 diff2                files           see (1)
	p4 dirs                 files *         see (1)
	p4 edit                 files * **      see (1)
	p4 fetch                revisions       see (2)
	p4 filelog              revisions       see (2)
	p4 files                files           see (1)
	p4 fixes                fixes           -j job, -c change
	p4 fixes files          revisions       see (2)
	p4 fstat                files           see (1)
	p4 integrate            files **        see (1)
	p4 integrated           revisions       see (2)
	p4 jobs                 jobs            -e query
	p4 jobs files           revisions       see (2)
	p4 jobs -e              index terms     see (7)
	p4 have                 files *         see (1)
	p4 labels               domains *       see (6)
	p4 labels files         files *         see (1)
	p4 labelsync            files           see (4)
	p4 list                 files           see (1)
	p4 lock                 files *         see (1)
	p4 merge                files **        see (1)
	p4 opened               files *         see (1)
	p4 print                files           see (1)
	p4 push                 revisions       see (2)
	p4 reconcile            files **        see (1)
	p4 reload               files *         see (5)
	p4 reopen               files *         see (1)
	p4 resolve              files *         see (1)
	p4 resolved             revisions *     see (2)
	p4 reviews -c           revisions *     see (2)
	p4 sizes                files           see (1)
	p4 sync                 files           see (1)
	p4 unload               files *         see (5)
	p4 verify               revisions       -m maxRevs
	p4 zip                  revisions       see (2)

	*  Subject to MaxScanRows/MaxLockTime but not MaxResults.
	** Subject to MaxOpenFiles.

	(1) These commands limit files.  Use more restrictive wildcards.

	(2) These commands limit individual revisions of each file,
	which can significantly exceed the number of actual files.
	Restrict the operation by using more restrictive wildcards on
	the command line to exclude more files, or by using a
	revision range (if permitted).

	(3) If the data required to describe a change exceeds the limit,
	'describe' cannot be run.

	(4) 'labelsync' limits files.  To build up the label in pieces,
	use restrictive wildcards with 'p4 labelsync -a'.

	(5) To 'unload' a client, you must be able to scan all the files
	in the client's have list; to 'unload' a label, you must be able
	to scan all the files listed in the label. If you are trying to
	unload or reload your own client or label, but are resource limited,
	you will need to ask an administrator with higher limits to unload
	or reload the client or label for you.

	(6) Domains include clients, labels, and branches. If the combined
	number of these objects in the server exceeds MaxScanRows, the
	command cannot be run.

	(7) If the number of index terms in the jobs database exceeds the
	limit, 'jobs -e jobview' cannot be run.
)"
};

ErrorId MsgHelp::HelpCharset = { ErrorOf( ES_HELP, 23, E_INFO, EV_NONE, 0 ),
R"(
	The P4 clients and server have an optional mode of operation in which
	all metadata and some file content are stored in the server in the UTF8
	Unicode character set and are translated into another character set on
	the client.

	The environment variable P4CHARSET specifies the client character
	set.  Valid settings for P4CHARSET are:

		auto		(Guess a P4CHARSET based on client OS params
		none		(same as unsetting P4CHARSET)
		eucjp
		iso8859-1
		iso8859-2
		iso8859-5
		iso8859-7
		iso8859-15
		macosroman
		shiftjis
		koi8-r
		utf8 		(UTF-8)
		utf8-bom	(UTF-8 with Byte-Order-Mark)
		utf16		(UTF-16 with client's byte ordering
					 and Byte-OrderMark)
		utf16-nobom	(UTF-16 client's byte ordering
					 without Byte-Order-Mark)
		utf16le		(UTF-16 with little endian byte ordering)
		utf16le-bom	(UTF-16 with little endian Byte-Order-Mark)
		utf16be		(UTF-16 with big endian byte ordering)
		utf16be-bom	(UTF-16 with big endian Byte-Order-Mark)
		utf32		(UTF-32 with client's byte ordering
					 and Byte-OrderMark)
		utf32-nobom	(UTF-32 client's byte ordering
					 without Byte-Order-Mark)
		utf32le		(UTF-32 with little endian byte ordering)
		utf32le-bom	(UTF-32 with little endian Byte-Order-Mark)
		utf32be		(UTF-32 with big endian byte ordering)
		utf32be-bom	(UTF-32 with big endian Byte-Order-Mark)
		cp850		(Windows code page 850)
		cp852		(Windows code page 852)
		cp858		(Windows code page 858)
		cp936		(Windows code page 936 - Simplified Chinese)
		cp949		(Windows code page 949 - Korean)
		cp950		(Windows code page 950 - Traditional Chinese)
		cp1250		(Windows code page 1250 - Central European)
		cp1251		(Windows code page 1251 - Cyrillic)
		winansi		(Windows code page 1252)
		cp1253		(Windows code page 1253 - Greek)

	If (and only if) P4CHARSET is set for the client, the server
	must also be operating in Unicode mode.  This mode is switched
	on (and never off) by invoking 'p4d -xi' on the server.  This
	operation verifies that all existing metadata is valid UTF8,
	and then sets the protected counter 'unicode' to indicate that
	the server now runs in Unicode mode.

	If P4CHARSET is not set explicitly when connecting to a Unicode mode
	server, a default charset will be chosen based on the client's
	platform and/or code page.

	In Unicode mode, all non-file data (identifiers, descriptions,
	etc) and the content of files of type 'unicode' are translated
	between the character set configured on the client by P4CHARSET
	and UTF8 in the server.

	P4CHARSET is supported by 2001.2 and later clients.

	When P4CHARSET is set to one of the UTF-16 values, P4COMMANDCHARSET
	must be set to a non-utf-16 value for the p4 command-line client to
	operate correctly.  This configuration enables P4CHARSET to control
	the translation of file revision contents while using a command
	line system that is not utf-16 based (because most command lines
	are not utf-16 compatible).  All UTF-16 values honor a file's Byte-
	Order-Mark when a file is read. When a file is written, the specified
	UTF-16 format is observed.
)"
};

ErrorId MsgHelp::HelpCommandments = { ErrorOf( ES_HELP, 128, E_INFO, EV_NONE, 0 ),
R"(
	P4 Commandments -- Values we work by

	* We have high standards.
	* We are straightforward.
	* We rise to responsibility.
	* We like work we can be proud of.
	* We like to hear what we've done.
	* We value both people skills and job skills.
	* We treat each other with dignity and respect.
	* We are one team. We are not in competition with each other.
	* We talk and listen. We like feedback.
	* We appreciate creative and practical solutions. There might be
	  a better way.
	* We appreciate people for who they are.
	* Fun is always an option. It is not mandatory.
	* These are the best years of our lives.
)"
};

ErrorId MsgHelp::HelpCredits = { ErrorOf( ES_HELP, 24, E_INFO, EV_NONE, 0 ),
R"(
	P4, The fast versioning engine from Perforce Software.
	Contributions from Iustin Amihaesei, Jeff Anton, Michael Bishop,
	Ksenia Burlachenko, Brian Campbell, Phil Champagne, Isaac Chou,
	Geri Clucas, Cal Collier, Scott Common, Robert Cowham, Ed Daraki,
	Jake Dickson, Jonathan Dyer, Gary Gibbons, Jason Gibson,
	Paul Haffenden, John Halbig, Wendy Heffner, Sven Erik Knop,
	Joel Kovisto, Peter Kreps, Dave Lewak, Fred Malouf, Mark Mears,
	Michael Alyn Miller, Adam Morriss, Norman Morse, Alex Neumann,
	Bryan Pendleton, Nick Poole, Giles Rainy Brown, Mike Schonberg,
	Christopher Seiwald, Andy Shebanow, Michael Shields, David Sielaff,
	Lakshmi Sistla, Tony Smith, Sam Stafford, James Strickland,
	Brett Taylor, Rob Taylor-Davis, Patrycja Tomiak, Alan Teague,
	Dulan Wettasinghe and Mark Wittenberg.
)"
};

ErrorId MsgHelp::HelpAdd = { ErrorOf( ES_HELP, 25, E_INFO, EV_NONE, 0 ),
R"(
    add -- Open a new file to add it to the depot

    p4 add [-c changelist#] [-d -f -I -n] [-t filetype] file ...

	Open a file for adding to the depot.  If the file exists on the
	client, it is read to determine if it is text or binary. If it does
	not exist, it is assumed to be text.  To be added, the file must not
	already reside in the depot, or it must be deleted at the current
	head revision.  Files can be deleted and re-added.

	A 2012.1 client will ignore files that were to be added, if they
	match an exclusion line specified in a P4IGNORE file.

	To associate the open files with a specific pending changelist, use
	the -c flag; if you omit the -c flag, the open files are associated
	with the default changelist.

	As a shortcut to reverting and re-adding, you can use the -d
	flag to reopen currently-open files for add (downgrade) under
	the following circumstances:

	    A file that is 'opened for edit' and is synced to the head
	    revision, and the head revision has been deleted (or moved).

	    A file that is 'opened for move/add' can be downgraded to add,
	    which is useful when the source of the move has been deleted
	    or moved.  Typically, under these circumstances, your only
	    alternative is to revert.  In this case, breaking the move
	    connection enables you to preserve any content changes in the
	    new file and safely revert the source file (of the move).

	To specify file type, use the -t flag.  By default, 'p4 add'
	determines file type using the name-to-type mapping table managed
	by 'p4 typemap' and by examining the file's contents and execute
	permission bit. If the file type specified by -t or configured in
	the typemap table is a partial filetype, the resulting modifier is
	applied to the file type that is determined by 'p4 add'. For more
	details, see 'p4 help filetypes'.

	To add files with filenames that contain wildcard characters, specify
	the -f flag. Filenames that contain the special characters '@', '#',
	'%%' or '*' are reformatted to encode the characters using ASCII
	hexadecimal representation.  After the files are added, you must
	refer to them using the reformatted file name, because P4 does not
	recognize the local filesystem name.

	The -I flag informs the client that it should not perform any ignore
	checking configured by P4IGNORE.

	The -n flag displays a preview of the specified add operation without
	changing any files or metadata.

	See 'p4 help-graph add' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpServerid = { ErrorOf( ES_HELP, 139, E_INFO, EV_NONE, 0 ),
R"(
    p4 serverid [serverID]

	'p4 serverid' gets or sets the unique id of the server. Use
	this command after you have first described the server using the
	'p4 server' command.

	Unless a P4NAME value has been specified for the server, the server
	uses the serverid to determine the appropriate configuration settings
	(see 'p4 help configure').

	The recommended technique for configuring servers in a multi-server
	installation is to give each server its own serverid, and specify
	the server configuration for that serverid; specifying a separate
	P4NAME for the server is generally not necessary.
)"
};

ErrorId MsgHelp::HelpAdmin = { ErrorOf( ES_HELP, 26, E_INFO, EV_NONE, 0 ),
R"(
    admin -- Perform administrative operations on the server

    p4 admin checkpoint [-z | -Z] [-p [-m]] [-N threads] [prefix]
    p4 admin journal [-z] [prefix]
    p4 admin stop
    p4 admin restart
    p4 admin updatespecdepot [-a | -s type]
    p4 admin resetpassword -a | -u user
    p4 admin setldapusers
    p4 admin end-journal
    p4 admin sysinfo
    p4 admin resource-monitor
    p4 admin replica-filter-reconcile [--restrict-only|--expand-only]
                                      [table ...]

	'p4 admin updatespecdepot', 'p4 admin resetpassword', and
	'p4 admin end-journal' require 'super' access.  The others
	require that the user be an operator or have 'super' access.

	'p4 admin checkpoint' causes the server to take a checkpoint and
	to copy the journal to a numbered journal file.  This command is
	equivalent to 'p4d -jc'. The '-p' option requests a parallel
	checkpoint. The '-m' option requests a multifile checkpoint and
	may only be specified with the '-p' option.
	The '-N threads' specifies the number of threads to
	use during the parallel request.

	'p4 admin journal' causes the server to save the journal to a
	numbered journal file and then truncate it.  This command is
	equivalent to 'p4d -jj'.

	The -z flag saves the checkpoint and journal in compressed (gzip)
	format, with the '.gz' extension added to the file names.

	The -Z flag saves the checkpoint in compressed (gzip) format,
	with the '.gz' extension added to the file name, but saves
	the journal as plain text (without an appended '.gz' extension).
	This allows a replica to update from the uncompressed journal
	without requiring the replica to stop and restart, while still
	saving disk space by compressing the presumably large checkpoint.

	If a prefix is specified, the files are named prefix.ckp.n and
	prefix.jnl.n respectively, where n is a sequence number. Without
	a prefix, the default filenames are checkpoint.n and journal.n.

	'p4 admin stop' stops the server, terminating any requests currently
	running.  It locks the database to ensure that no updates are taking
	place, but otherwise does not wait for users to finish what they are
	doing. (Windows NT: this command can be used regardless of whether
	you are running the server as a program or a service.)

	'p4 admin restart' restarts the server, terminating any requests
	currently running.  It locks the database to ensure that no updates
	are taking place, but otherwise does not wait for users to finish what
	they are doing. (Windows NT: this command can be used regardless of
	whether you are running the server as a program or a service.) The
	restarted server will pick up the latest configurable settings.

	'p4 admin updatespecdepot' archives forms (client, depot, repo, branch,
	label, ldap, typemap, group, user, job, stream, triggers, protect,
	server, license, and jobspec forms) into the spec depot.  To archive
	all types of form specifications specify the -a flag. To archive
	a single type of spec, specify the '-s' flag. Only forms that have
	not yet been archived are created.

	'p4 admin resetpassword' forces specified users with existing
	passwords to change their passwords before they can run another
	command. To force password reset of all users with passwords, specify
	the '-a' flag. To force password reset of a single user, specify the
	'-u' flag.

	'p4 admin setldapusers' bulk updates users' AuthMethod field to 'ldap'.
	This affects all non-super users of standard type.

	'p4 admin end-journal' ends at the last consistency point the
	journal replicated by the 'journalcopy' thread in a standby
	server. The 'journalcopy' thread is stopped.

	'p4 admin sysinfo' displays the output from operating system reporting
	commands run on the P4 server's host. This is intended as a support
	tool to gather information about the environment the P4 server is
	running in.

	'p4 admin resource-monitor' - see 'p4 help admin-resource-monitor'

	'p4 admin replica-filter-reconcile' purge database records that should
	be excluded due to replica filter changes, and fetch database records
	that were previously excluded.
)"
};

ErrorId MsgHelp::HelpAdminResourceMonitor = { ErrorOf( ES_HELP, 276, E_INFO,
	                                               EV_NONE, 0 ),
R"(
    p4 admin resource-monitor -- Monitor system resources

	This command monitors resources on the host operating system that the
	P4 server is running on.  This is the command that provides the
	information the P4 server uses to throttle incoming client
	commands in order to keep itself responsive and from overloading the
	operating system.  This command is intended to be run as a server
	startup command, one per server instance.  Setting the
	sys.pressure.os.[mem|cpu].high configurables to zero will disable
	checking for the given resource.

	See 'p4 help server-resources' for more information on server
	handling of operating system resource pressure.

	This command requires 'super' access.
)"
};


ErrorId MsgHelp::HelpJournaldbchecksums = { ErrorOf( ES_HELP, 135, E_INFO, EV_NONE, 0 ),
R"(
    journaldbchecksums -- write journal notes with table checksums

    p4 journaldbchecksums [-t tableincludelist | -T tableexcludelist] [-l N]
    p4 journaldbchecksums -u filename -t tablename [-v N] [-z]
    p4 journaldbchecksums -s -t tablename [ -b blocksize ][-v N]
    p4 journaldbchecksums -c change

	'p4 journaldbchecksums' provides a set of tools for ensuring data
	integrity across a multi-server installation.

	This command enables administrators to compare database contents
	between the master server and its replicas. Important information
	about the results of the command is logged to structured server logs
	as events of type 'integrity', so using this command effectively
	includes defining a log such as 'integrity.csv' to capture these log
	events and then reviewing the log files regularly.

	Effective replica monitoring must be performed regularly, so when
	using the 'p4 journaldbchecksums' command, you should ensure
	that the rpl.checksum.change and rpl.checksum.table configurables
	have been configured to enable routine integrity verification, and
	you should ensure that you regularly inspect your integrity logs for
	indications of problems.

	The rpl.checksum.change configurable can be set to 0,1,2,3; it
	defaults to 0. The levels operate as follows:
	    0: don't do any changelist checksumming
	    1: server writes journal note at end of submit, populate,
	       push, fetch, and unzip commands
	    2: replica should verify the changelist summary, and should
	       write to integrity.csv if the changelist does not match
	    3: replica should write the results to integrity.csv even
	       if the changelist checksum does match

	The rpl.checksum.table configurable can be set to 0,1,2; it
	defaults to 0. The levels operate as follows:
	    0: table-level checksumming only
	    1: journal notes for table-unload and table-scan are processed
	       by the replica, and are logged if the check fails
	    2: results of journal note processing in the replica are
	       logged even if the results match.

	The first form of the 'p4 journaldbchecksums' command causes the
	server to write journal notes containing table checksum information.
	Replica servers will automatically verify the table checksums when
	processing these notes.

	The -t and -T flags specify a list of database tables that are,
	respectively, included or excluded from the set of tables for which
	journal notes are written.  To specify multiple tables, double-quote
	the list and separate the table names with spaces. The table names
	must start with "db.". Table names can also be separated by commas.
	Please note that the specific partitioned tables (For example,
	"db.have.pt%pjnlclient") cannot be included in the set of tables.

	When the server rotates its journal as part of the 'admin checkpoint'
	or 'admin journal' commands, it automatically performs the
	'journaldbchecksums' command; the set of tables included in the
	command is controlled by the rpl.checksum.auto configurable. To
	specify the same set of tables explicitly in the 'journaldbchecksums'
	command, specify the table set level using the -l flag.

	The table set levels for this release are:
	    1: the most important system and revision tables. Checksum errors
	       in these tables should be investigated immediately.
	    2: all tables that should not vary between unfiltered replicas
	    3: all tables, including tables which will vary on build-server
	       and edge-server replicas

	Note that if data filtering is used for a replica, both the table
	and changelist checksums are affected and can be expected to differ.

	The second form of the 'p4 journaldbchecksums' command causes the
	server to unload the database table specified with -t to the file
	specified with -u. The command also writes a journal note describing
	this action, and replica servers will automatically unload the same
	table to the same file when processing these notes. This allows you to
	compare the contents of the table in a time-consistent fashion. This
	command is recommended only for tables that are small. In the case
	of partitioned-jnl have tables, the respective table is unloaded to
	the file where its name is customised accordingly. For example,
	'p4 journaldbchecksums -u unloadhavept -t db.have.pt' command results
	in unloading the table 'db.have.pt%pwsjnl1' to the file
	'unloadhavept_db_pwsjnl1'. The -z flag causes the file specified with
	-u to be saved in compressed format.

	The third form of the 'p4 journaldbchecksums' command causes the
	server to scan the specified database table. The table is scanned
	in blocks; the number of records in a block is specified by the -b
	flag, which defaults to 5,000. For each block, the server computes
	a block checksum and writes it as a journal note. Replica servers
	will automatically verify these blocks when processing these notes.
	This command can be used with large tables on a production system
	as the table is unlocked between each block. Inspecting the results
	of the block verifications will reveal the location of damage which
	affects only part of a database table.

	When unloading or scanning a table, the -v flag specifies the server
	version number to use. If it is not specified, the current server
	version is used.

	The fourth form of the 'p4 journaldbchecksums' command causes the
	server to compute a checksum of an individual submitted changelist.
	This checksum is written as a journal note, and replica servers will
	automatically verify the checksum of the change when processing
	these notes.

	'p4 journaldbchecksums' requires that the user be an operator or
	have 'super' access.
)"
};

ErrorId MsgHelp::HelpJournalcopy = { ErrorOf( ES_HELP, 171, E_INFO, EV_NONE, 0 ),
R"(
    journalcopy -- Copy journal data from master to local filesystem

    p4 journalcopy -i N [-b N] [--durable-only] [--non-acknowledging]
    p4 journalcopy -l

	'p4 journalcopy' is used with a standby replica that can take
	over as the master server in the event of a failover.

	Complete configuration of a standby replica involves several steps.
	Please consult the P4 documentation set for the full details.

	The -i flag causes the command to automatically repeat its action
	every N seconds. If -i is not specified, the command runs once,
	then exits.

	The -b flag specifies a wait time before retrying a journalcopy after
	a failed journalcopy attempt.  This defaults to 60 seconds.

	The --durable-only flag limits the results to durable journal records.

	The --non-acknowledging flag means that this request does not mark
	previous journal records as durable.

	The -l flag reports the current standby journal position.

	This command requires 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpBranch = { ErrorOf( ES_HELP, 27, E_INFO, EV_NONE, 0 ),
R"(
    branch -- Create, modify, or delete a branch view specification

    p4 branch [-f] name
    p4 branch -d [-f] name
    p4 branch [ -S stream ] [ -P parent ] -o name
    p4 branch -i [-f]

	A branch specification ('spec') is a named, user-defined mapping of
	depot files to depot files. It can be used with most of the commands
	that operate on two sets of files ('copy', 'merge', 'integrate',
	'diff2', etc.)

	Creating a branch spec does not branch files.  To branch files, use
	'p4 copy', with or without a branch spec.

	The 'branch' command puts the branch spec into a temporary file and
	invokes the editor configured by the environment variable $P4EDITOR.
	Saving the file creates or modifies the branch spec.

	The branch spec contains the following fields:

	Branch:      The branch spec name (read only).

	Owner:       The user who created this branch spec. Can be changed.

	Update:      The date this branch spec was last modified.

	Access:      The date of the last command used with this spec.

	Description: A description of the branch spec (optional).

	Options:     Flags to change the branch spec behavior. The defaults
		     are marked with *.

		locked   	Permits only the owner to change the spec.
		unlocked *	Prevents the branch spec from being deleted.

	View:        Lines mapping of one view of depot files to another.
		     Both the left and right-hand sides of the mappings refer
		     to the depot namespace.  See 'p4 help views' for more on
		     view syntax.

	New branch specs are created with a default view that maps all depot
	files to themselves.  This view must be changed before the branch
	spec can be saved.

	The -d flag deletes the named branch spec.

	The -o flag writes the branch spec to standard output. The user's
	editor is not invoked.

	The -i flag causes a branch spec to be read from the standard input.
	The user's editor is not invoked.

	The -f flag enables a user with 'admin' privilege to delete the spec
	or set the 'last modified' date.  By default, specs can be deleted
	only by their owner.

	A branch spec can also be used to expose the internally generated
	mapping of a stream to its parent. (See 'p4 help stream' and 'p4
	help streamintro'.)

	The -S stream flag will expose the internally generated mapping.
	The -P flag may be used with -S to treat the stream as if it were a
	child of a different parent. The -o flag is required with -S.
)"
};

ErrorId MsgHelp::HelpAnnotate = { ErrorOf( ES_HELP, 87, E_INFO, EV_NONE, 0 ),
R"(
    annotate -- Print file lines and their revisions

    p4 annotate [-aciIqtTu -d<flags>] file[revRange] ...

	Prints all lines of the specified files, indicating the revision that
	introduced each line into the file.

	If the file argument includes a revision, then only revisions up to
	the specified revision are displayed.  If the file argument has a
	revision range, only revisions within that range are displayed. For
	details about specifying revisions, see 'p4 help revisions'.

	The -a flag includes both deleted files and lines no longer present
	at the head revision. In the latter case, both the starting and ending
	revision for each line is displayed.

	The -c flag directs the annotate command to output changelist numbers
	rather than revision numbers for each line.

	The -d<flags> change the way whitespace and/or line endings are
	treated: -db (ignore whitespace changes), -dw (ignore whitespace),
	-dl (ignore line endings).

	The -i flag follows branches.  If a file was created by branching,
	'p4 annotate' includes the revisions of the source file up to the
	branch point, just as 'p4 filelog -i' does.  If a file has history
	prior to being created by branching (such as a file that was branched
	on top of a deleted file), -i ignores those prior revisions and
	follows the source.  -i implies -c.

	The -I flag follows all integrations into the file.  If a line was
	introduced into the file by a merge, the source of the merge is
	displayed as the changelist that introduced the line. If the source
	itself was the result of an integration, that source is used instead,
	and so on.  -I implies -c and may not be combined with -i.

	The -q flag suppresses the one-line header that is displayed by
	default for each file.

	The -t flag forces 'p4 annotate' to display binary files.

	The -T flag aligns output to a tab stop of 8.  A different tab stop
	may be specified via '--tab=N'.

	The -u flag directs the annotate command to output the user who
	modified the line in the file and the date it was modified.
)"
};

ErrorId MsgHelp::HelpArchive = { ErrorOf( ES_HELP, 125, E_INFO, EV_NONE, 0 ),
R"(
    archive -- Archive obsolete revisions to an archive depot.

    p4 archive [-n -h -p -q -t -z] -D <depot> file[revRange] ...

	Moves the archives for the specified revisions into the specified
	depot, which must be an archive depot. The revision's action is
	changed to 'archive' so that all commands that access revision
	content will skip the revision. Only revisions that meet the
	following criteria are archived:

	1. stored in full (+F) or compressed (+C) format
	2. located in a local depot
	3. not copied or branched from another revision
	4. not copied or branched to another revision

	The storage for the archive depot must be mounted before running
	this command (unless you are previewing results using the -n flag).

	The -h flag prevents the head revision of files from being archived.

	The -n flag displays a preview of the results of the archive
	operation.

	The -p flag purges any archives of the specified files in the
	specified archive depot. On completion, the action for affected
	revisions is set to 'purge'.

	The -q flag suppresses messages about skipped revisions.

	The -t flag instructs 'p4 archive' to archive revisions stored
	in delta format (binary+D or text revisions). By default, these
	revisions are excluded due to the high cost of manipulating
	their storage.

	The -z flag includes in the archive those revisions that have lazy
	copies or are lazy copies. With this flag, only criteria 1 and 2
	must be met. Unless all copies are archived, the original file
	remains in the depot.

	'p4 archive' requires 'admin' access granted by 'p4 protect'.

	To restore archived revisions, use 'p4 restore'; for details, see
	'p4 help restore'.
)"
};

ErrorId MsgHelp::HelpAttribute = { ErrorOf( ES_HELP, 95, E_INFO, EV_NONE, 0 ),
R"(
    attribute -- Set per-revision attributes on revisions

    p4 attribute [-e -f -p] -n name [-v value [-T0|-T1]] files ...
    p4 attribute [-e -f -p [-T0|-T1]] -i -n name file
    p4 attribute [-f -p [-T0|-T1]] -I filename -n name file

	Sets a user-managed attribute 'name' to a value on opened files.
	If -v, -i, or -I are omitted, the attribute is cleared (if set).

	To display attributes use 'p4 fstat -Oa' or 'p4 print -T'.

	The -e flag indicates that the 'value' is specified in hex.

	The -f flag sets the attribute on submitted files.  If a propagating
	trait is set on a submitted file, a revision specification may not
	be used, and the file must not be currently open on any client.
	The -f flag requires 'admin' access granted by 'p4 protect'.

	The -p flag creates an attribute with a value that is propagated
	when the file is opened with 'p4 copy', 'p4 delete', 'p4 edit',
	'p4 integrate', 'p4 reconcile', 'p4 resolve', 'p4 submit', 'p4 shelve',
	or 'p4 unshelve'.

	The -i flag causes the attribute value to be read from the standard
	input. This option supports both text and binary input. Only one
	file argument is allowed when using this option.

	The -I flag causes the attribute value to be read from a file.
	This option supports both text and binary files. This is
	recommended for attribute data that is greater than 250MB in size,
	which might cause the command to fail with the 'Rpc buffer too big'
	error. The '-e' option to specify the value as hex cannot be used
	with this option. Only one file argument is allowed when using this
	option. Also, only one trait value may be set using this option.
	To display attributes set with this option, the 'p4 print -T'
	command is recommended instead of the 'p4 fstat -Oa' command because
	'p4 print -T' can handle larger text data as well as larger
	non-encoded binary data.

	Multiple attributes can be set or cleared by specifying multiple
	'-n name' flags and an equal number of corresponding '-v value'
	flags (to set) or no '-v' flags (to clear). Note that '-v value'
	only supports text data input.

	When an attribute is created, the configurable 'trait.storagedepot.min'
	determines how the attribute is stored. By default, attribute values
	are stored in the db.traits table. The 'trait.storagedepot.min'
	value is used to enable depot storage for new attribute values, and
	also to specify the minimum size in bytes for storing a value in the
	trait depot. When 'trait.storagedepot.min' is set to a value greater
	than 0, the value of a newly created attribute will be stored in the
	trait depot if its size matches or exceeds the trait.storagedepot.min
	value. When 'trait.storagedepot.min' is unset or set to 0, trait values
	are stored in the 'db.traits' table instead.

	The -T option may be used when creating a new attribute to override
	the configurable setting only when the 'trait.storagedepot.min'
	configurable is set to a value greater than 0. The -T0 setting causes
	the value to be stored in the db.traits table. When trait depot
	storage is allowed, the -T1 option specifies that the value will be
	stored in the trait depot even if it does not meet the
	trait.storagedepot.min size.
)"
};

ErrorId MsgHelp::HelpBackup = { ErrorOf( ES_HELP, 190, E_INFO, EV_NONE, 0 ),
R"(
    backup -- cause a workspace-server to automatically backup its workspaces

    p4 backup"

	'p4 backup' instructs this workspace server to launch a process to
	periodically backup all workspaces on the server. This command is
	valid only when used on p4d servers functioning as cluster
	workspace-servers. The behavior of the backup process is managed
	via two configurables: the client.backup.interval and the
	idle.unload.interval.

	The configurable client.backup.interval causes the command to
	automatically repeat its action every N seconds. If the
	client.backup.interval is not set or set to 0 then
	the command runs once, then exits.

	When set, the idle.unload.interval configurable causes the command
	to police the last used time all workspaces participating in backup*.
	Each of the workspaces that have a last used time that exceeds the
	specified N number of seconds will be automatically unloaded. If this
	configurable is not set, or set to 0, the auto-unload feature is
	turned off.

	*A user can opt out of having one or more of their workspaces backed
	up by setting the Backup field to 'disable' in that workspace's
	client spec. By default all non-readonly workspaces have backup
	enabled unless this option is set. Setting this option to
	'disable' is recommended for build farm workspaces.

)"
};

ErrorId MsgHelp::HelpBranches = { ErrorOf( ES_HELP, 28, E_INFO, EV_NONE, 0 ),
R"(
    branches -- Display list of branch specifications

    p4 branches [-t] [-u user [--user-case-insensitive]] [[-e|-E] nameFilter]
	     [-m max]

	Lists branch specifications. (See 'p4 help branch'.)

	The -t flag displays the time as well as the date.

	The -u user flag lists branch specs owned by the specified user.
	This can include wildcards to form a search pattern. If wildcards
	are used enclose the search pattern in double quotes. You can also
	add a --user-case-insensitive flag which will indicate that the user
	value is a case-insensitive search pattern.

	The -e nameFilter flag lists branch specs with a name that matches
	the nameFilter pattern, for example: -e 'svr-dev-rel*'. The -e flag
	uses the server's normal case-sensitivity rules. The -E flag makes
	the matching case-insensitive, even on a case-sensitive server.

	The -m max flag limits output to the specified number of branch specs.
)"
};

ErrorId MsgHelp::HelpBroker = { ErrorOf( ES_HELP, 149, E_INFO, EV_NONE, 0 ),
R"(
    broker -- Display Broker connection information

    p4 broker

	'p4 broker' is only understood by the P4 Broker.

	'p4 broker' lists information about the broker which the client is
	connected to.  Items displayed include the client's network address,
	the broker's network address, and the broker's version information.
)"
};

ErrorId MsgHelp::HelpChange = { ErrorOf( ES_HELP, 29, E_INFO, EV_NONE, 0 ),
R"(
    change -- Create or edit a changelist description
    changelist -- synonym for 'change'

    p4 change [-s] [-f | -u] [[-O|-I] changelist#]
    p4 change -d [-f -s -O] changelist#
    p4 change -o [-s] [-f] [[-O|-I] changelist#]
    p4 change -i [-s] [-f | -u]
    p4 change -t restricted | public [-U user] [-f|-u|-O|-I] changelist#
    p4 change -U user [-t restricted | public] [-f] changelist#
    p4 change -d -f --serverid=X changelist#

	'p4 change' creates and edits changelists and their descriptions.
	With no argument, 'p4 change' creates a new changelist.  If a
	changelist number is specified, 'p4 change' edits an existing
	pending changelist.  In both cases, the changelist specification
	is placed into a form and the user's editor is invoked.

	The -d flag deletes a pending changelist, if it has no opened files
	(or stream spec) and no pending fixes associated with it.  Use
	'p4 opened -a' to report on opened files and 'p4 reopen' to move them
	to another changelist.  Use 'p4 fixes -c changelist#' to report on
	pending fixes and 'p4 fix -d -c changelist# jobs...' to delete pending
	fixes. The changelist can be deleted only by the user and client
	who created it, or by a user with 'admin' privilege using the -f
	flag.

	The -o flag writes the changelist specification to the standard
	output.  The user's editor is not invoked.

	The -i flag reads a changelist specification from the standard
	input.  The user's editor is not invoked.

	The -f flag forces operations on pending and submitted changelists
	and requires 'admin' access granted by 'p4 protect'. The -f and -u
	flags are mutually exclusive. By default, submitted changelists cannot
	be changed. The -f flag forces:

	    - Update or deletion of other users' pending changelists.
	    - Changing the 'User' of an empty pending change via -U.
	    - Deletion of submitted changelists after they have been
	      emptied of files using 'p4 obliterate' and of streams
	      using 'p4 stream --obliterate'.
	    - Update of the 'Description', 'Date', and 'User' fields of
	      submitted changelists.
	    - Display of the 'Description' field in a restricted changelist.

	The -u flag can force the update of a submitted change by the owner
	of the change. Only the Jobs, Type, and Description fields can be
	changed	using the -u flag. The -f and -u flags cannot be used in
	the same change command.

	The -s flag extends the list of jobs to include the fix status
	for each job.  On new changelists, the fix status begins as the
	special status 'ignore', which, if left unchanged simply excludes
	the job from those being fixed.  Otherwise, the fix status, like
	that applied with 'p4 fix -s', becomes the job's status when
	the changelist is committed.  Note that this option exists
	to support integration with external defect trackers.

	The -O flag specifies that the changelist number is the original
	number of a changelist which was renamed on submit.

	The -I flag specifies that the changelist number is the Identity
	field of a changelist.

	The -U flag changes the 'User' of an empty pending change to the
	specified user. The user field can only be changed using this flag
	by the user who created the change, or by a user with 'admin'
	privilege using the -f flag. This option is useful for running
	in a trigger or script.

	The -t flag changes the 'Type' of the change to 'restricted'
	or 'public' without displaying the change form. This option is
	useful for running in a trigger or script.

	The 'Type' field can be used to hide the change or its description
	from users. Valid values for this field are 'public' (default), and
	'restricted'. A shelved or committed change that is 'restricted' is
	accessible only to users who own the change or have 'list' permission
	to at least one file in the change.  A pending (not shelved)
	restricted change is only accessible to its owner.  Public changes
	are accessible to all users. This setting affects the output of the
	'p4 change', 'p4 changes', and 'p4 describe' commands. Note that
	the '-S' flag is required with 'p4 describe' for the command to
	enforce shelved	rather than pending restricted changelist rules.

	If a user is not permitted to have access to a restricted change,
	The 'Description' text is replaced with a 'no permission' message
	(see 'Type' field). Users with admin permission can override the
	restriction using the -f flag.

	In order to delete a pending changelist which is owned by a client
	on a different server, specify 'change -d -f --serverid=X', providing
	the serverid of the owning server. This variant of the change command
	must be issued directly to the Commit Server.
)"
};

ErrorId MsgHelp::HelpChanges = { ErrorOf( ES_HELP, 30, E_INFO, EV_NONE, 0 ),
R"(
    changes -- Display list of pending and submitted changelists
    changelists -- synonym for 'changes'

    p4 changes [-i -t -l -L -f -r] [-c client [-E]] [ -e changelist# ]
	    [-m max] [-s status] [-u user [-E]] [file[revRange] ...]
	    [--stream|--nostream]

	Returns a list of all pending and submitted changelists currently
	stored in the server.

	If files are specified, 'p4 changes' lists only submitted
	changelists that affect those files at the head revision.  If
	the file specification includes a revision range, 'p4 changes'
	lists only submitted changelists that affect the specified
	revisions.  See 'p4 help revisions' for details.

	If files are not specified, 'p4 changes' limits its report
	according to each change's type ('public' or 'restricted').
	If a submitted or shelved change is restricted, the change is
	not reported unless the user owns the change or has list
	permission for at least one file in the change. Only the owner
	of a restricted and pending (not shelved) change is permitted
	to see it.

	The -i flag also includes any changelists integrated into the
	specified files.

	The -t flag displays the time as well as the date.

	The -l flag displays the full text of the changelist
	descriptions.

	The -L flag displays the changelist descriptions, truncated to 250
	characters if longer.

	The -f flag enables admin users to view restricted changes.

	The -c client flag displays only changes owned by the specified client.
	You can repeat this option to filter the result for multiple clients.
	Each client can include wildcards to form a search pattern. This search
	pattern can start with '-' to exclude changes by those client(s) from
	the result. Adding a -E (or --client-case-insensitive) flag makes the
	search pattern case-insensitive even on a case-sensitive server.

	The -e changelist# flag displays only changes that are above and
	including the specified changelist number.

	The -m max flag limits changes to the 'max' most recent.

	The -r flag sorts the output in reverse order.

	The -s status flag limits the output to changelists with the specified
	status. Specify '-s pending', '-s shelved', or '-s submitted'.

	The -u user flag displays only changes owned by the specified user.
	You can repeat this option to filter the result for multiple users.
	Each user can include wildcards to form a search pattern. This search
	pattern can start with '-' to exclude changes by those user(s) from
	the result. Adding a -E (or --user-case-insensitive) flag makes the
	search pattern case-insensitive even on a case-sensitive server.

	The --stream flag displays only changes that contain a stream spec.

	The --nostream flag displays only changes that do not contain a
	stream spec.

	For additional details on a single changelist, see 'p4 help describe'.
)"
};

ErrorId MsgHelp::HelpClient = { ErrorOf( ES_HELP, 31, E_INFO, EV_NONE, 0 ),
R"(
    client -- Create or edit a client workspace specification and its view
    workspace -- Synonym for 'client'

    p4 client [-f] [-t template] [-T type] [name]
    p4 client -d [-f [-Fs|-Fd]] name
    p4 client -o [-t template] [-T type] [name]
    p4 client -S stream [[-c change] -o] [name]
    p4 client -s [-f] -S stream [name]
    p4 client -s [-f] -t template [name]
    p4 client -i [-f]
    p4 client -d -f --serverid=X [-Fs] name

	Creates a new client specification ('spec') or edits an existing
	spec.  A client spec is a named mapping of depot files to workspace
	files.

	The 'p4 client' command puts the client spec into a temporary file
	and invokes the editor configured by the environment variable
	$P4EDITOR.  For new workspaces, the client name defaults to the
	$P4CLIENT environment variable, if set, or to the current host name.
	Saving the file creates or modifies the client spec.

	The client spec contains the following fields:

	Client:      The client name.

	Host:        If set, restricts access to the named host.
		     If unset, access is allowed from any host.

	Owner:       The user who created this client.

	Update:      The date that this spec was last modified.

	Access:      The date that this client was last used in any way.

	Description: A short description of the workspace.

	Root:        The root directory of the workspace (specified in local
		     file system syntax), under which all versioned files
		     will be placed. If you change this setting, you must
		     physically relocate any files that currently reside
		     there.  On Windows client machines, you can specify the
		     root as "null" to enable you to map files to multiple
		     drives.

	AltRoots:    Up to two optional alternate client workspace roots.
		     The first of the main and alternate roots to match the
		     client program's current working directory is used. If
		     none match, the main root is used. 'p4 info' displays
		     the root that is being used.

	Options:     Flags to configure the client behavior. Defaults
		     are marked with *.

		allwrite	Leaves all files writable on the client;
		noallwrite *	by default, only files opened by 'p4 edit'
				are writable. If set, files might be clobbered
				as a result of ignoring the clobber option
				(see below).

		clobber		Permits 'p4 sync' to overwrite writable
		noclobber *	files on the client.  noclobber is ignored if
				allwrite is set.

		compress 	Compresses data sent between the client
		nocompress *	and server to speed up slow connections.

		locked   	Allows only the client owner to use or change
		unlocked *	the client spec.  Prevents the client spec from
				being deleted or unloaded.

		modtime  	Causes 'p4 sync' and 'p4 submit' to preserve
		nomodtime *	file modification time, as with files with the
				+m type modifier. (See 'p4 help filetypes'.)
				With nomodtime, file timestamps are updated by
				sync and submit operations.

		rmdir		Makes 'p4 sync' attempt to delete a workspace
		normdir *	directory when all files in it are removed.

		altsync		Instructs the client to use an alternative sync
		noaltsync *	agent specified with P4ALTSYNC when performing
				file operations in the workspace. This option
				can only be changed when the client's have list
				is empty.

	SubmitOptions:  Flags to change submit behavior.

		submitunchanged     All open files are submitted (default).

		revertunchanged     Files that have content, type, or resolved
		                    changes are submitted. Unchanged files are
		                    reverted.

		leaveunchanged      Files that have content, type, or resolved
		                    changes are submitted. Unchanged files are
		                    moved to the default changelist.

		        +reopen     Can be appended to the submit option flag
		                    to cause submitted files to be reopened in
		                    the default changelist.
		                    Example: submitunchanged+reopen

	LineEnd:    Set line-ending character(s) for client text files.

		local		mode that is native to the client (default).
		unix		linefeed: UNIX style.
		mac		carriage return: Macintosh style.
		win		carriage return-linefeed: Windows style.
		share		hybrid: writes UNIX style but reads UNIX,
				Mac or Windows style.

	View:        Maps files in the depot to files in your client
		     workspace.  Defines the files that you want in your
		     client workspace and specifies where you want them
		     to reside.  The default view maps all depot files
		     onto the client.  See 'p4 help views' for view syntax.
		     A new view takes effect on the next 'p4 sync'.

	ChangeView:  Restricts depot paths to a particular point in time.
	             Example: //depot/path/...@1000
	             Revisions submitted to the path after the specified
	             changelist will not be visible.  Files matching a
	             ChangeView path may not be submitted.

	             Automatic labels may also used to specify a particular
	             point in time: the label's 'Revision' field must be set
	             to a changelist number or a date.  If the label has a
	             view, only files that match the intersection of the
	             ChangeViewpath and the label's view will be restricted to
	             the point in time specified by the 'Revision' field; any
	             other files matching the ChangeView path may sync to
	             #head, but cannot be submitted.

	LimitView:   Restricts the paths mapped by the View. This is helpful
	             for streams clients where the View cannot be modified in
	             the client spec.
	             The mappings may be specified in either client or depot
	             syntax, but not a combination of both.
	             Example:
	                 View:
	                     //depot1/... //ws/depot1/...
	                     //depot2/... //ws/depot2/...
	                 LimitView:
	                     //ws/depot1/path/...
	                     -//ws/....exe
	             Will limit the client's view to just files under
	             //depot1/path excluding files with the .exe extension.

	Stream:      The stream to which this client's view will be dedicated.
		     (Files in stream paths can be submitted only by dedicated
		     stream clients.) When this optional field is set, the
		     View field will be automatically replaced by a stream
		     view as the client spec is saved.

	StreamAtChange:
		     Stream meta-data (for view creation) is versioned.
		     When StreamAtChange is set, the stream hierarchy is
		     constructed back-in-time to reproduce the view that was
		     available to clients in the past. With this field set
		     changes cannot be be submitted from the client.
		     StreamAtChange is optional; Stream must be set as well.

	ServerID:    If set, restricts usage to the named server.
		     If unset, usage is allowed on the master server and on
		     any replicas of the master other than Edge Servers.

	Type:        By default all clients are 'writeable', certain clients
		     are short lived and perform long sync and build cycles.
		     Over time these build clients can fragment the 'db.have'
		     table which is used to track what files a client has
		     synced.  Setting a type of 'readonly' gives the client
		     its own personal 'db.have' database table.  A 'readonly'
		     client cannot 'edit' or 'submit' files, however for build
		     automation this is not usually a requirement and the
		     performance tradeoff is worth considering if your build
		     automation is causing issues with the 'db.have' table.
		     This option requires that an administrator has first
		     configured the 'client.readonly.dir' setting.  If it is
		     necessary to submit changes as part of your build,
		     you may specify a 'partitioned' client: like a 'readonly'
		     client, this type also has a separate 'db.have' table
		     under the 'client.readonly.dir' directory, but allows
		     journalled 'edit' and 'submit' of files.

	Note: changing the client root does not actually move the client
	files; you must relocate them manually.  Similarly, changing
	the 'LineEnd' option does not actually update the client files;
	you can refresh them with 'p4 sync -f'.

	The -d flag deletes the specified spec, as long as the client
	workspace has no opened files or pending changes.  (See 'p4 help
	opened'.) The -f flag forces the delete. The -Fs option used with -f
	forces client deletion even when the user has shelved changes. In
	this case, the shelved changes are left intact. The -Fd option with
	-f forces deletion of the client and all its shelved changes.

	The -o flag writes the named client spec to the standard output.
	The user's editor is not invoked.

	The -i flag reads a client spec from the standard input.  The
	user's editor is not invoked.

	The -t template flag, where 'template' is the name of another client
	spec, causes the View and Options fields to be replaced by those of
	the template.

	The -T type flag lets you set the type of client, this can only be
	set when first creating the client. For classic P4 clients the default
	is 'writeable', set this value to 'readonly' for build automation or
	clients that do not need to update content.

	The -f flag can force the updating of locked clients; normally
	locked clients can only be modified by their owner.  -f also allows
	the last modified date to be set.  The -f flag requires 'admin'
	access granted by 'p4 protect'.

	The -s flag is used to switch an existing client spec's view without
	invoking the editor.  It can be used with -S to switch to a stream
	view, or with -t to switch to a view defined in another client spec.
	Switching views is not allowed in a client that has opened files.
	The -f flag can be used with -s to force switching with opened files.
	View switching has no effect on files in a client workspace until
	'p4 sync' is run.

	Without -s, the '-S stream' flag can be used to create a new client
	spec dedicated to a stream. If the client spec already exists, and
	-S is used without -s, an error occurs when the client is bound to
	a different, or no stream.  Using -S sets the new client's
	Stream field.  The special syntax '-S //a/stream@changelist' can
	be used to set both Stream and StreamAtChange at the same time.

	The '-S stream' flag can be used with '-o -c change' to inspect an
	old stream client view. It yields the client spec that would have
	been created for the stream at the moment the change was recorded.

	A server of type 'build-server' (see 'p4 help server') is a replica
	which supports build farm integration, and the 'p4 client' command
	may be used to create or edit client workspaces on a build-server.
	Such workspaces may issue the 'p4 sync' command, in addition to any
	read-only command supported by the replica. For more information, run
	'p4 help buildserver'.

	When creating or editing a client workspace for a build-server,
	the client specified by the optional 'name' argument, as well as the
	client specified by the $P4CLIENT environment variable or via the
	global '-c client' argument must not exist, or must be restricted
	to this server; this command may not be used to create or edit a
	workspace that is not restricted to this build-server.

	In order to delete a workspace which is bound to a different server,
	specify 'client -d -f --serverid=X', providing the serverid of the
	owning server. This variant of the client command must be issued
	directly to the Commit Server.

	See 'p4 help-graph client' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpClients = { ErrorOf( ES_HELP, 32, E_INFO, EV_NONE, 0 ),
R"(
    clients -- Display list of clients
    workspaces -- synonym for 'clients'

    p4 clients [-t] [-u user [--user-case-insensitive]] [[-e|-E] nameFilter]
               [-m max] [-S stream] [-a|-s serverID]
    p4 clients -U

	Lists all client workspaces currently defined in the server.

	The -t flag displays the time as well as the date.

	The -u user flag lists client workspaces that are owned by the
	specified user. This can include wildcards to form a search pattern.
	If wildcards are used enclose the search pattern in double quotes.
	You can also add a --user-case-insensitive flag which will indicate
	that the user value is a case-insensitive search pattern.

	The -e nameFilter flag lists workspaces with a name that matches
	the nameFilter pattern, for example: -e 'svr-dev-rel*'. The -e flag
	uses the server's normal case-sensitivity rules. The -E flag makes
	the matching case-insensitive, even on a case-sensitive server.

	The -m max flag limits output to the specified number of workspaces.

	The -S stream flag limits output to the client workspaces dedicated
	to the stream(s).  A wildcard pattern may be used to match multiple
	streams, or an empty string to match clients with no stream.

	The -U flag lists unloaded clients (see 'p4 help unload').

	The -a flag specifies that all clients should be displayed, not just
	those that are bound to this server.

	The -s serverID flag specifies that only those clients bound to the
	specified serverID should be displayed. On an Edge Server, the -s
	flag defaults to the Edge Server's serverID.

)"
};

ErrorId MsgHelp::HelpCluster = { ErrorOf( ES_HELP, 164, E_INFO, EV_NONE, 0 ),
R"(
    cluster -- Administer a server with failover (P4 cluster servers only)

    p4 cluster new-master previous-master-server-id
    p4 cluster master-changed new-master-address
    p4 cluster members-set server-type list-of-cluster-members
    p4 cluster end-journal

	The preceding 'p4 cluster' commands are issued automatically by the
	P4 cluster management infrastructure when a cluster server fails.


	The following 'p4 cluster' commands are invoked by the cluster manager
	during the implementation of its command to replace a workspace server.

    p4 cluster to-workspace
    p4 cluster restore-clients old-server-id

	The following 'p4 cluster' command can be issued to reset the cluster
	master generation number stored in Zookeeper.

    p4 cluster set-gen-number new-master-gen-number

	All 'p4 cluster' commands require that the user be an operator
	or have 'super' access.

)"
};

ErrorId MsgHelp::HelpStream = { ErrorOf( ES_HELP, 110, E_INFO, EV_NONE, 0 ),
R"(
    stream -- Create, delete, or modify a stream specification

    p4 stream [-f] [-d] [-P parent] [-t type]
              [--parentview inherit|noinherit] [name]
    p4 stream [-o [-v]] [-P parent] [-t type]
              [--parentview inherit|noinherit] [name[@change]]
    p4 stream [-f] --obliterate [-y] name[@change]
    p4 stream [-f] [-d] name
    p4 stream -i [-f]
    p4 stream edit
    p4 stream parentview [-c changelist# -n -o --source-comments]
                         {--inherit|--noinherit}
    p4 stream convertsparse [-q]
    p4 stream resolve [-a<flag>] [-n] [-o]
    p4 stream revert

	A stream specification ('spec') names a path in a stream depot to be
	treated as a stream.  (See 'p4 help streamintro'.)  The spec also
	defines the stream's lineage, its view, and its expected flow of
	change.

	The 'p4 stream' command puts the stream spec into a temporary file and
	invokes the editor configured by the environment variable $P4EDITOR.
	When creating a stream, the type of the stream must be specified with
	the '-t' flag.  Saving the file creates or modifies the stream spec.

	By default updates to the stream spec are automatically visible to
	all clients; see 'p4 help streamcmds' for information on 'opening' a
	stream spec to stage updates on one client before submitting them.

	Creating a stream spec does not branch a new stream.  To branch a
	stream, use 'p4 copy -r -S stream', where 'stream' is the name of a
	stream spec.

	The stream spec contains the following fields:

	Stream:   The stream's path in a stream depot, of the form
	          //depotname/streamname. This is both the name of the stream
	          spec and the permanent, unique identifier of the stream.
	          This stream path may extend beyond the traditional form by
	          updating the 'StreamDepth' field in the stream depot when
	          creating a new stream depot.  Here, depth relates to the
	          number of single slashes in stream names following the
	          depot name (so traditional form above reflects StreamDepth
	          of 1.  All streams within a stream depot must adhere to
	          this same format.

	Update:   The date the specification was last changed.

	Access:   The date the specification was originally created.

	Owner:    The stream's owner. A stream can be owned by a user, or
	          owned by a group. Can be changed.

	Name:     An alternate name of the stream, for use in display outputs.
	          Defaults to the final path in the 'streamname' portion in
	          the stream. Can be changed.

	Parent:   The parent of this stream. Can be 'none' if the stream type
	          is 'mainline',  otherwise must be set to an existing stream
	          identifier, of the form //depotname/streamname.
	          Can be changed.

	Type:     'mainline', 'development', 'release', 'sparsedev',
	          'sparserel', 'task', or 'virtual'. Defines the role of a
	          stream.

	          A 'mainline' has no parent.

	          The 'development', 'release', 'sparsedev', and 'sparserel'
	          streams have controlled flow. This can be changed in the
	          stream Options.

	          'sparsedev' and 'sparserel' are lightweight streams which do
	          not need to be populated when they are created and instead
	          automatically branch files as they are edited. They use the
	          same flow control as 'development' and 'release' streams,
	          respectively.

	          A 'task' stream is meant to be a short-lived stream that only
	          promotes edited files to the repository. It is a best
	          practice to delete or unload unused 'task' streams to avoid
	          unnecessary metadata that otherwise would slow performance.

	          A 'virtual' stream is not a separate set of files, but
	          instead a filtered view of its parent stream

	          Flow control is provided by 'p4 copy -S' and 'p4 merge -S'.
	          These commands restrict the flow of change as follows:

	          Stream Type   Direction of flow     Allowed with
	          -----------   -----------------     ------------
	          development   to parent stream      'p4 copy'
	          sparsedev     to parent stream      'p4 copy'
	          task          to parent stream      'p4 copy'
	          release       to parent stream      'p4 merge'
	          sparserel     to parent stream      'p4 merge'
	          development   from parent stream    'p4 merge'
	          sparsedev     from parent stream    'p4 merge'
	          release       from parent stream    'p4 copy'
	          sparserel     from parent stream    'p4 copy'

	Description: An optional description of the stream.

	Options:  Flags to configure stream behavior. Defaults are marked *:

	          unlocked *      Indicates whether the stream spec is locked
	          locked          against modifications. If locked, the spec
	                          may not be deleted, and only its owner or
	                          group users can modify it.

	          allsubmit *     Indicates whether all users or only the
	          ownersubmit     owner (or group users) of the stream may
	                          submit changes to the stream path.

	          toparent *      Indicates if controlled flow from the
	          notoparent      stream to its parent is expected to occur.

	          fromparent *    Indicates if controlled flow to the stream
	          nofromparent    from its parent is expected to occur.

	          mergedown *     Indicates if merge flow is restricted or
	          mergeany        merge is permitted against the expected flow.

	          The [no]fromparent and [no]toparent options determine if
	          'p4 copy -S' and 'p4 merge -S' allow change to flow between
	          a stream and its parent. A 'virtual' stream must have its
	          flow options set as 'notoparent' and 'nofromparent'. Flow
	          options are ignored for 'mainline' streams.

	ParentView: 'inherit' or 'noinherit'.  Defines whether the stream
	          inherits a view from its parent.

	          If a ParentView is inherit, the Paths, Remapped, and Ignored
	          fields will be affected.  The view created from each field is
	          composed of the stream's fields and the set of fields
	          "inherited" from each of the stream's ancestors. The
	          inheritance is implicit, so the inherited Paths, Remapped,
	          and Ignored values will not be displayed with the current
	          stream specification.

	          If a ParentView is noinherit, the Paths, Remapped, and
	          Ignored fields are not affected by the stream's ancestors.
	          The child views are exactly what is specified in the Paths,
	          Remapped, and Ignored fields.

	          Task streams, virtual streams, and sparse streams can only
	          have inherit ParentViews. All other stream types can have
	          inherit or noinherit ParentViews.

	          When a development, release, or mainline stream is first
	          created, the ParentView field may be set with the
	          '--parentview' parameter. The parameter may be set with
	          values 'inherit' or 'noinherit'.

	          When creating a development, release, or mainline stream
	          without the '--parentview' parameter, the configurable
	          dm.stream.parentview will determine the default value for
	          the ParentView field.  The configurable has the following
	          values:

	              0: all new streams have an inherit ParentView.
	              1: all new mainline, development and release streams have
	                 a noinherit ParentView, all new task and virtual
	                 streams have an inherit ParentView.
	              2: all new release streams have a noinherit ParentView,
	                 all new non-release streams have an inherit ParentView
	                 value.

	          The default value for dm.stream.parentview is 0.

	          After a stream has been created, the ParentView field cannot
	          be changed by editing the stream specification.  Instead, use
	          the command 'p4 stream parentview' to change stream's
	          parentview. See 'p4 help streamcmds' for more details.

)"
R"(
	Components:  One or more lines that include other streams' views within
	          the current stream.  Each line is of the form:

	              <component_type> <component_folder> <stream>[@change]
	
	          where

	              <component_type> is a single keyword.

	              <component_folder> a directory path relative to the
	              root of the client workspace.

	              <stream> is the stream path and name of an existing
	              stream.  An optional '@change' or '@automatic_label' can
	              be specified to get the streamviews at or before the
	              given change or automatic label.

	          For example:

	              readonly dirA //stream/mainA@37

	          A component is a stream that contributes its views to the
	          the stream that defines the component. A stream that defines
	          one or more other streams as components is called a consuming
	          stream.

	          If a component is defined to be at a given change, that
	          component's views come from the last time the stream was
	          edited, either at the specified change or earlier. Any file
	          sync'd from a component@change's views will be at a revision
	          at or before the specified change.

	          The component types are:

	              readonly -- all views that originate from the component
	                  are readonly;  files can be sync'd but not edited and
	                  submitted.  Note that if the component has an import+
	                  path, that path is readonly for the consuming stream.

	              writeimport+ -- all views that originate from import+
	                  paths in the component can be opened for edit and
	                  submitted (writable);  all other views that
	                  originate from the component are readonly.

	              writeall -- all views that originate from share, isolate,
	                  or import+ paths in the component can be opened for
	                  edit and submitted (writable);  all other views that
	                  originate from the component are readonly.

	          Note that component stream views will be included when the
	          consuming stream is itself defined as a component for some
	          other stream.  For example:

	              Suppose there are streams //stream/mainA, //stream/mainB,
	              and main //stream/mainC.

	              Suppose that //stream/mainC defines a Path

	                  import libXYZ/lib1.a //libs/libXYZ/lib1.a

	              Then //stream/mainB defines a component

	                  readonly dirC //stream/mainC

	              //stream/mainA defines a component

	                  readonly dirB //stream/mainB

	              Then a workspace ws, which is associated with
	              //stream/mainA, will have views

	                  //stream/mainA/... //ws/...
	                  //stream/mainB/... //ws/dirB/...
	                  //stream/mainC/... //ws/dirB/dirC/...
	                  //libs/libXYZ/lib1.a //ws/dirB/dirC/libXYZ/lib1.a

	          Note that in a chain of components of different types, the
	          most restrictive type becomes the effective component type
	          for views originating from the end component in the chain.

	          For example, if component B is defined as writeall in A, and
	          B has a readonly component C, then the views coming from
	          paths in C are readonly.  Similarly, if B is defined as
	          writeimport+ in A and C is defined as writeall in B, then the
	          views in A which originate from share or isolate paths in C
	          are readonly, while import+ paths in C are writable.

	          The configurable, dm.stream.components, can be used to enable
	          or disable the inclusion of stream components in the
	          workspace view.  Use values

	              0 (default): disables stream components in the workspace
	                  view.

	              1: enables stream components in the workspace view.


	Paths:    One or more lines that define file paths in the stream view.
	          Each line is of the form:

	              <path_type> <view_path> [<depot_path>]

	          where <path_type> is a single keyword, <view_path> is a file
	          path with no leading slashes, and the optional <depot_path>
	          is a file path beginning with '//'.  Both <view_path> and
	          <depot_path> may contain trailing wildcards, but no leading
	          wildcards.  Embedded wildcards are restricted to the final
	          expression of the path, with no further slashes.
	          Lines in the Paths field may appear in any order.
	          A duplicated <view_path> overrides its preceding entry.

	          For example:

	              share   src/...
	              import  lib/abc/...  //over/there/abc/...
	              import  module2/*.txt //demo3/main/module2/*.txt
	              isolate bin/*
	              isolate bin/...dll

	          Default is:

	              share   ...

	          The <path_type> keyword must be one of:

	          share:  <view_path> will be included in client views and
	                  in branch views. Files in this path are accessible
	                  to workspaces, can be submitted to the stream, and
	                  can be integrated with the parent stream.

	          isolate: <view_path> will be included in client views but
	                   not in branch views. Files in this path are
	                   accessible to workspaces, can be submitted to the
	                   stream, but are not integratable with the parent
	                   stream.

	          import: <view_path> will be included in client views but
	                  not in branch views. Files in this path are mapped
	                  as in the parent stream's view (the default) or to
	                  <depot_path> (optional); they are accessible to
	                  workspaces, but can not be submitted or integrated
	                  to the stream.  If <depot_path> is used it may
	                  include a changelist or automatic label specifier;
	                  clients of that stream will be limited to seeing
	                  revisions at that point in time or lower within that
	                  depot path (See the explanation of 'ChangeView' in
	                  'p4 help client' for more details)

	          import+: <view_path> same as 'import' except that files can
	                   be submitted to the import path.  Note that an
	                   import+ path that references a path in a stream
	                   depot will not respect the views or flow rules of
	                   any other streams that depend on that depot path,
	                   so undesirable interactions are possible.

	          import&: <view_path> <depot_path> same as 'import' except
	                   that multiple import& paths can map the
	                   same <depot_path> to multiple <view_path>s.
	                   Files marked this way are readonly.
	                   <depot_path> is required.

	          exclude: <view_path> will be excluded from client views
	                   and branch views. Files in this path are not
	                   accessible to workspaces, and can't be submitted
	                   or integrated to the stream.

	          Paths are inherited by child stream views. A child stream's
	          paths can downgrade the inherited view, but not upgrade it.
	          (For instance, a child stream can downgrade a shared path to
	          an isolated path, but it can't upgrade an isolated path to a
	          shared path.) Note that <depot_path> is relevant only when
	          <path_type> is 'import'.

	Remapped: Optional; one or more lines that define how stream view paths
	          are to be remapped in client views.  Each line is of the
	          form:

	              <view_path_1> <view_path_2>

	          where <view_path_1> and <view_path_2> are P4 view paths with
	          no leading slashes, no leading wildcards, or wildcards
	          embedded between slashes.
	          For example:

	              ...    x/...
	              y/*    y/z/*

	          Line ordering in the Remapped field is significant; if more
	          than one line remaps the same files, the later line has
	          precedence.  Remapping is inherited by child stream client
	          views.

	Ignored: Optional; a list of file or directory names to be recursively
	         excluded in branch and client views. For example:

	             /tmp      # ignores files named 'tmp'
	             /tmp/...  # ignores dirs named 'tmp'
	             .tmp      # ignores file names ending in '.tmp'

	         Lines in the Ignored field may appear in any order.  Ignored
	         names are inherited by child stream client views.  Note that
	         due to their recursive nature, large numbers of Ignored
	         entries may be inefficient; consider P4IGNORE as another way
	         to prevent specific filename patterns from being added.

	The -d flag causes the stream spec to be deleted.  A stream spec may
	not be deleted if it is referenced by child streams or stream clients.
	Deleting a stream spec does not remove stream files, but it does mean
	changes can no longer be submitted to the stream's path.
	The stream spec is marked deleted while preserving its metadata.
	Report on deleted streams using 'p4 stream -o deletedStream@change',
	'p4 streams -a', and 'p4 streamlog deletedStream'.
	( See --obliterate. )

	The --obliterate flag deletes the stream from all metadata,
	removes the stream from pending and committed change lists, and allows
	deletion of such changelists. Obliterate can be restricted to
	a changelist: p4 stream --obliterate name@change.
	By default, --obliterate displays a preview of the
	results. To execute the operation, specify the -y flag.
	--obliterate requires 'admin' access granted by 'p4 protect'.

	The -o flag causes the stream spec to be written to the standard
	output. The user's editor is not invoked. -v may be used with -o to
	expose the automatically generated client view for this stream.
	('p4 help branch' describes how to expose the branch view.)
	If an @changelist specifier is included with the stream name, the
	(submitted) stream as of that change is displayed rather than the
	latest (or open) version.

	The -P flag can be used to insert a value into the Parent field of a
	new stream spec. The flag has no effect on an existing spec.

	The -t flag is used to insert a value into the type field of a
	new stream spec and to adjust the default fromparent option
	for a new 'release' -type stream. The flag has no effect on an
	existing spec.

	The -i flag causes a stream spec to be read from the standard input.
	The user's editor is not invoked.

	The -f flag allows a user other than the owner to modify or delete a
	locked stream. It requires 'admin' access granted by 'p4 protect'.

	See 'p4 help streamcmds' for information on the 'p4 stream edit',
	'p4 stream resolve', 'p4 stream revert', 'p4 stream parentview', and
	'p4 stream convertsparse'
	commands.
)"
};

ErrorId MsgHelp::HelpStreamCmds = { ErrorOf( ES_HELP, 191, E_INFO, EV_NONE, 0 ),
R"(
    Operating on opened stream specs

	Stream specifications may be modified directly via the 'p4 stream',
	command, automatically and immediately updating all client views
	derived from that stream.  Alternatively, edits to the stream spec
	may be isolated to the editing client prior to being made available
	to other clients as part of an atomic changelist submission, in the
	same way that edits to files are made locally on a single client and
	then submitted to make them available to other clients.

    p4 stream edit
    p4 stream parentview [-c changelist# -n -o --source-comments]
                         {--inherit|--noinherit}
    p4 stream convertsparse [-q]
    p4 stream resolve [-a<flag>] [-n] [-o]
    p4 stream revert
    p4 streamlog stream ...

	'p4 stream edit' puts the client's current stream spec into the
	'opened' state, isolating any edits made to fields that affect
	view generation.  While the spec is open, those fields are marked
	with the comment '#open' to indicate that they are open and isolated
	to your client.  Changes made to these fields will affect your client
	view normally, but other clients are not affected. 'p4 edit -So' has
	the same effect as 'p4 stream edit' (see 'p4 help edit').

	'p4 stream parentview' modifies the ParentView property of the client's
	current stream spec.  This property determines whether changes to the
	current stream's parent or ancestor affects client and branch views.

		-c	Associate the stream spec with given changelist#
			when opening the stream spec for edit.
		-n	Previews the changes made.
		-o	Display the changed stream spec.
		--inherit
			Change the ParentView field to 'inherit'. Changes to
			the Paths, Remapped, and Ignored fields which occurred
			when converting the stream spec to a noinherit
			ParentView will remain.  Client and branch views for
			streams with an inherit ParentView property will be
			affected by changes in an ancestor's Paths, Remapped,
			or Ignored fields.
		--noinherit
			Change the ParentView field to 'noinherit'.  Also
			converts the Paths, Remapped and Ignored fields by
			copy or merging paths from the corresponding fields in
			the ancestor stream specifications.  Client and branch
			views for streams with an inherit ParentView property
			will not be affected by changes in an ancestor's Paths,
			Remapped, or Ignored fields.
		--source-comments
			Add inline source comments to Paths, Remapped, or
			Ignored lines which were copied or merged from an
			ancestor stream spec when changing the ParentView field
			from inherit to noinherit.  The comment will
			specify the source ancestor spec, the change number for
			head version of the ancestor spec, and whether a given
			line was copied or merged into the target stream spec.

	'p4 stream parentview' may be used only with mainline, development, and
	release streams.

	The client's current stream spec must not be opened to use 'p4 stream
	parentview'.  The command will open the stream spec and change the
	ParentView field and possibly the Paths, Remapped, or Ignored fields.
	Any changes made to the stream spec will not be made public until the
	change is submitted.  Once submitted, the ParentView change will become
	part of the stream spec's integration history.

	'p4 stream convertsparse' converts a sparsedev or sparserel stream to
	either a development or release stream. If it is not already open, the
	stream specification will be opened and any unbranched files from the
	parent stream will be opened in the workspace for branch to the stream.
		-q	Suppress normal output messages. Messages regarding
			errors or exceptional conditions are displayed.

	'p4 stream resolve' resolves changes that have been submitted to the
	stream spec since you opened it.  You may not submit changes to the
	stream spec until newer changes have been resolved.
		-as	'Safe' resolve; skip fields that need merging.
		-am	Resolve by merging; skip fields with conflicts.
		-af	Force combination of text fields with conflicts.
		-at	Force acceptance of theirs; overwrites yours.
		-ay	Force acceptance of yours; ignores theirs.
		-n	Preview which fields require resolve.
		-o	Output the base used for the merge.
	'p4 resolve -So' is an alias for 'p4 stream resolve'.

	'p4 stream revert' reverts any pending changes made to the open spec,
	returning your client to the latest submitted version of the stream.
	'p4 revert -So' has the same effect as 'p4 stream revert' (see
	'p4 help revert').

	'p4 streamlog' lists the revision history of the specified streams
	in reverse chronological order, starting from the most recent revision
	to the earliest.

	The open stream spec is included by default with any files that are
	shelved, unshelved, reshelved, or submitted.  The stream may be omitted
	from any of these operations by using the '-Af' flag to specify that
	only files should be acted upon.

	See 'p4 help openablestreamspecs' for more on openable stream specs.
)"
};

ErrorId MsgHelp::HelpStreamlog = { ErrorOf( ES_HELP, 262, E_INFO, EV_NONE, 0 ),
R"(
    streamlog -- List revision history of streams

    p4 streamlog [ -c  changelist# -h -i -l -L -t -m max ] stream ...

	List the revision history of the specified stream specs, from the most
	recent revision to the first.  If the stream was opened for edit
	and submitted, the change list information is displayed.
	Otherwise only the maximum change num at the time of edit is displayed.

	The -c flag displays the stream submitted at the specified
	changelist number.

	The -i flag includes inherited stream history. For a stream created by
	branching (using 'p4 integrate'), streamlog lists the revisions of the
	stream's ancestors up to the branch points that led to the specified
	revision.  Stream history inherited by renaming (using 'p4 move') is
	always displayed regardless of whether -i is specified.

	The -h flag displays the stream content history instead of stream name
	history.  The list includes revisions of other streams that were
	branched or copied (using 'p4 integrate' and 'p4 resolve -at') to
	the specified revision.  Revisions that were replaced by copying
	or branching are omitted, even if they are part of the history of
	the specified revision.

	The -t flag displays the time as well as the date.

	The -l flag lists the full text of the changelist descriptions.

	The -L flag lists the full text of the changelist descriptions,
	truncated to 250 characters if longer.

	The -m max displays at most 'max' revisions per stream
	argument specified.

	'p4 streamlog' reports on deleted streams.

)"
};
ErrorId MsgHelp::HelpStreamintro = { ErrorOf( ES_HELP, 132, E_INFO, EV_NONE, 0 ),
R"(
    Introduction to streams

	A P4 'stream' is (generally) a branch of files.  Each stream has a
	parent stream and any number of child streams.  The backbone of a
	stream system is the mainline, a stream with no parent.

	A stream typically contains files that are branched from its parent
	stream. Work on files occurs in the context of a stream. As files are
	modified, changes can be integrated (i.e., merged or copied) between
	the stream and its parent.

	A child stream is either a 'development', 'release', 'task' or a
	'virtual' stream.  On a scale of stability, a development stream
	is considered lower than its parent, while a release stream is
	considered higher than its parent.  Change is expected to flow down
	by merging, and up by copying.  This "merge down, copy up" practice
	assures that merging is done only when necessary, and always in the
	more forgiving of the two streams involved.

	A stream is identified by its unique depot location.  A 'spec' defines
	the stream's location, its type, its parent stream, the files in its
	view, and other configurable behaviors.  A stream can be reparented
	by changing its spec.

	A stream's 'view' refers to both the client view and the branch view
	that are generated from the spec.  Generated client views are inserted
	automatically when a user creates a stream client.  Generated branch
	views are used automatically by commands that compare or integrate
	files.

	The stream view enables you to specify which files can be submitted
	to the stream and merged or copied from its parent stream.  A parent
	stream controls the scope of a child stream's view.  The stream view
	is inherited from the parent stream (except for mainline streams,
	which have no parent).  You can refine the view of a child stream to
	make it more restrictive than its parent, but you cannot change the
	view to make it less restrictive than the view of the parent.
	Virtual streams can be used to create alternative views of real
	streams.

	Virtual streams differ from other stream types in that a virtual
	stream is not a branch of files, but instead a filtered view of
	its parent stream. A virtual stream can have child streams, and its
	child streams inherit its views.

	Task streams are lightweight short-lived branches that are great for
	bug fixing or new features that only modify a small subset of the
	branch data. Since branched (copied) files are tracked in a set of
	shadow tables which are later removed, repository metadata is kept
	to a minimum when using this type of stream.  Note: only a client
	of the task stream will see the branched data, only modified data
	that is promoted becomes visible to users with access to the stream
	namespace.

	Streams are rooted in stream depots. A mainline and all of the streams
	related to it are rooted in the same stream depot. A P4 server can host
	multiple stream depots. Although a stream depot can have multiple
	mainlines, one mainline per stream depot is recommended.

	Files can be submitted to streams only by stream clients. Each stream
	client is dedicated to a single stream.  A user may have several
	stream clients.  Stream clients can be switched from one stream to
	another. A client of a virtual stream is no different than a client
	of its underlying, real stream, except that it has a modified view
	of the underlying stream.

	To create a new mainline, first create a stream depot with 'p4 depot'.
	Then use 'p4 stream' to create a mainline stream rooted in that depot.
	To populate a new mainline, use 'p4 client' to create a client
	workspace for it. Then, in the client workspace, use 'p4 add' and 'p4
	submit' to upload files into the stream.

	To branch a stream into another, first use 'p4 stream' to create the
	new stream spec. Then use 'p4 copy' and 'p4 submit' to branch files
	into the new stream.  To merge files from one stream into another, use
	'p4 merge', 'p4 resolve', and 'p4 submit'. To promote files from one
	stream into another, use 'p4 copy' and 'p4 submit'.

	Several existing commands have been enhanced, and some new commands
	have been added, in support of streams:

	    branch       - Can expose a stream's branch view
	    client       - Can create or switch a stream client
	    clients      - Can list clients dedicated to a stream
	    copy         - Can use a stream view and enforce stream flow
	    depot        - Can create a stream depot
	    diff2        - Can use a stream view
	    dirs         - Can list dirs in a stream view
	    integrate    - Can use a stream view
	    interchanges - Can use a stream view
	    istat        - Checks/caches a stream's pending integration state
	    merge        - Can use a stream view and enforce stream flow
	    prune        - Remove unmodified branched files from a stream
	    stream       - Creates/edits a stream spec, can expose client view
	    streamlog    - List revision history of streams
	    streams      - Lists stream specs
	    streamspec   - Edit the stream template
	    switch       - Switch to a different stream or create a new stream

	See 'p4 help <command>' for additional help for these commands.

)"
};

ErrorId MsgHelp::HelpStreams = { ErrorOf( ES_HELP, 111, E_INFO, EV_NONE, 0 ),
R"MSG(
    streams -- Display list of streams

    p4 streams [options] [-U] [streamPath ...]
    p4 streams [options] --viewmatch file[revRange] [streamPath ...]

	options: -a -F filter -m max -T fields

	Reports the list of all streams currently known to the system.  If
	a 'streamPath' argument is specified, the list of streams is limited
	to those matching the supplied path. Unloaded task streams are not
	listed by default.

	For each stream, a single line of output lists the stream depot path,
	the type, the parent stream depot path, and the stream name.

	The -a flag returns existing and deleted streams.

	The -F filter flag limits the output to files satisfying the expression
	given as 'filter'.  This filter expression is similar to the one used
	by 'jobs -e jobview', except that fields are case sensitive.  E.g.:

	    p4 streams -F "Parent=//Ace/MAIN&(Type=development|Type=release)"
	    p4 streams --viewmatch foo.c -F "PathType=import\&|PathType=share"

	Note: the filtering takes place post-compute phase; there are no
	indexes to optimize performance.

	The -m max flag limits output to the first 'max' number of streams.

	The -T fields flag (used with the -ztag global option) limits the
	fields output to those specified by a list given as 'fields'.
	These field names can be separated by a space or a comma.  E.g.:

	    p4 streams -T "Stream,Owner"

	The -U flag lists unloaded task streams (see 'p4 help unload').

	The --viewmatch flag returns streams whose views match the specified
	file(s).  The output includes the matching path entry for each stream.
	If a wildcard is specified, all overlapping path entries are returned.
)MSG"
};

ErrorId MsgHelp::HelpStreamSpec = { ErrorOf( ES_HELP, 263, E_INFO, EV_NONE, 0 ),
R"(
    streamspec -- Edit the stream template

    p4 streamspec
    p4 streamspec -o
    p4 streamspec -i

	streamspec edits the template that specifies the format of streams.
	This format is used by 'p4 stream' when streams are entered or updated,
	and by 'p4 streams'.

	streamspec brings up a form with the following fields:

	   Fields:  A list of the fields maintained for each stream, one
		    line per field.  Each line has five words: code, name,
		    data-type, len, and field-type.

		    'code' is a unique integer identifier for storing
			the data of the field. When adding a new field,
			one may either enter a numeric field code value
			in the 701-749 range, or as 'NNN'. In the later
			case, the next available code value will be
			automatically generated. If the number of fields
			exhausts the range, new codes are assigned unique
			values greater than or equal to 10000.

		    'name' is the name of the field for the stream.

		    'data-type' indicates the format of the field:

			word: a single word (any value)
			date: a date/time field
			select: one of a set of words
			line: a one-liner
			text: a block of text

		    'len' is the recommended character length of a
			display box for the field.  If 0, a text box is
			assumed.

		    'field-type' indicates how to handle the setting of
			the field:

			optional: no default, and not required to be present
			default: default provided, still not required
			required: default provided, value must be present
			once: set once to the default and never changed
			always: always set to the default when saving the
				form

	   Values:  A list of 'select' fields and the values that the fields
		    can have.  Each line has two words: the field name and
		    the values list, with individual values separated by
		    '/' (no spaces).

	   Presets: A list of fields and their default values, for fields
		    whose 'setting' flag is other than 'optional'.  Each
		    line has two words: the field name and the default
		    value.  If the value has spaces, it must be enclosed
		    in double quotes.  The following special defaults are
		    recognized:

			$user: the user entering the stream
			$now: the current date
			$blank: the words '<enter description here>'

	   Openable: A list of versioning options when editing a stream spec.
		    Each line has two words: the field name and the value.
		    Values are:
		        'none' : may not be changed in a stream edit
		        'isolate' : may be changed only in a stream edit
		        'propagate' : may be changed in a stream edit or integ


	   Comments: text to be included at the top of each stream spec,
		    to help the user fill out the form. Each line must begin
		    with the comment character '#'.

	The following field codes are reserved:

	    code 701, required: the stream name
	    code 705, optional: the date of last modification
	    code 706, optional: the date of creation
	    code 704, optional: the user who created the stream
	    code 703, required: a short title which may be updated
	    code 702, required: parent or 'none' if Type is mainline
	    code 708, required: one of mainline, release,
	        development(default), virtual, task
	    code 709, optional: a short description
	    code 707, optional:
	        allsubmit/ownersubmit,unlocked/locked
	        toparent/notoparent,fromparent/nofromparent"
	        mergedown/mergeany
	    code 710, required:
	        identify paths in the stream and how they are to be
	        generated in resulting clients of this stream
	        path types are: share/isolate/import/import+/import&/exclude
	    code 711, optional: remap a stream path in the resulting generated
	                        client view
	    code 712, optional: ignore a stream path in the resulting generated
	                        client view


	If you change the streamspec after streams have been created, note the
	following limitations:

	    Data is stored according to its code.  Fields can be renamed
	    by keeping the same code. Removing a code can cause the
	    associated data stored for the code to become abandoned.

	    Changing the definition of a code (for example, from 'text' to
	    'word') can require users to correct streams for the new format
	    during editing.

	The -o flag writes the stream template to the standard output.  The
	user's editor is not invoked.

	The -i flag reads a stream template from the standard input.  The
	user's editor is not invoked.

	'p4 streamspec' requires 'admin' access, which is granted by
	'p4 protect'.
)"
};

ErrorId MsgHelp::HelpOpenableStreamSpecs = 
	                          { ErrorOf( ES_HELP, 266, E_INFO, EV_NONE, 0 ),
R"(
    Openable stream specs (Private editing of streams)

	Stream specifications can be edited using the command 'p4 stream'.
	By default, once the changes to the stream specification are saved,
	the changes take effect immediately, and can affect descendent streams
	as well.

	Such public changes can have far-reaching effects.  For example, if an
	import path is changed, a dependent build system may break unless
	corresponding file changes are made as well.

	As of the 2019.1 server release, stream specifications can be opened
	for edit in a client workspace, added to a changelist, shelved,
	unshelved, compared against the head version, and reverted.

	Private editing of stream specifications allows a user to modify a
	stream spec in isolation, group a stream spec change with associated
	file changes in one changelist, and shelve stream spec changes for
	review.

	The following commands have been modified to work with openable stream
	specifications:

    p4 change
    p4 copy
    p4 edit
    p4 describe
    p4 diff
    p4 diff2
    p4 integ
    p4 merge
    p4 opened
    p4 reopen
    p4 revert
    p4 shelve
    p4 stream resolve
    p4 streamlog
    p4 submit
    p4 unshelve

	'p4 change' specifications now allow an optional 'Stream:' field to
	add an opened stream spec to a pending changelist.  The stream spec
	must be opened to the current workspace prior to adding it to the
	changelist. (see 'p4 help change').

	'p4 edit -So' can be used with '-c changelist#' to open the client's
	stream spec for edit. (see 'p4 help edit').

	'p4 describe' of a changelist will list an affected stream spec before
	listing the affected files. (see 'p4 help describe').
	
	'p4 diff -As' can be used to diff an opened stream spec against either
	the head version of the stream spec (the default), or against the
	specified stream spec at either the head or the specified change
	number. To diff a shelved version of the specified stream spec, use
	the revision specification '@=change', where 'change' is the shelf
	changelist number. (see 'p4 help diff').

	'p4 diff2 -As' can be used to diff two different stream specifications.
	By default, the head version of a given stream spec is used.  If a
	changelist number is given, the stream spec version associated with
	the changelist will be used instead. If '@=change' is given, the
	shelved stream spec version associated with the changelist will be
	used instead. (see 'p4 help diff2').

	'p4 opened -So' can be used with '-c change' to list the stream spec
	that is currently opened for edit. By default, if -So is omitted,
	the opened stream spec will be listed before the list of files.
	If no stream spec is opened for edit, a stream spec will not be
	listed. (see 'p4 help opened').

	'p4 reopen -So' can be used with '-c change' to move the client's open
	stream spec to a different changelist.  'p4 reopen -Si [file ...]' can
	be used with a file list to include an open stream spec when also
	reopening files.  For example, to move an open stream spec to the
	default changelist, use 'p4 reopen -c default -So'. (see 'p4 help
	reopen').

	'p4 revert -So' can be used with '-c change' to revert the client's
	open stream spec.  'p4 revert -Si [file ...]' can be used with a file
	list to include an open stream spec when reverting files.   If a stream
	spec is open and neither -Si nor -So is specified, the stream spec will
	be excluded from the reversion. (see 'p4 help revert').

	'p4 shelve -c changelist# -As' specifies that only an opened stream
	specification should be shelved with the changelist.
	'p4 shelve -c changelist# -Af [file ...]' specifies that only files
	should be shelved with the changelist.  By default, if the stream spec
	is open and neither -Af nor -As is given, the stream specification will
	also be included with any shelved files. (see 'p4 help shelve').

	'p4 stream resolve' resolves opened stream spec fields.
	See 'p4 help streamcmds'.

	'p4 streamlog' lists the revision history of the specified streams
	in reverse chronological order, starting from the most recent revision
	to the earliest.

	'p4 submit' can submit an opened stream specification. If the opened
	stream spec is in a numbered changelist in the current workspace, use
	'p4 submit -c changelist#' to submit the changelist.  If the opened
	stream spec is shelved to a numbered changelist, use
	'p4 submit -e shelf#' to submit the shelved changelist.

	The 'p4 submit' flags -So and -Sx can be used to control whether an
	open stream spec in the default changelist will be submitted.  Use -So
	with no file list to submit only the open stream spec in the default
	changelist.  Use -Sx with a file list to exclude the open stream spec
	when submitting files in the default changelist.  Use -Sx with no file
	list to submit all files without submitting the open stream spec.

	By default, if both 'p4 submit' flags -So and -Sx flags are omitted,
	then an open stream spec will be submitted along with any files in the
	given filelist. (see 'p4 help submit').

	'p4 unshelve -s changelist# -As' specifies that only the stream spec
	associated with the shelved numbered changelist should be unshelved to
	the current client. Unshelving a stream spec requires that the client
	is set to that stream, and will open the stream spec for edit.
	'p4 unshelve -s changelist# -Af [file ...]' specifies that only files
	should be unshelved.  By default, the unshelve command acts on both
	the files (-Af) and the stream spec (-As). (see 'p4 help unshelve').

	'p4 copy -S stream' will copy the source stream spec propagatable
	fields into the target stream spec and the target stream spec will be
	opened for edit, along with stream files.  Use '-As' to copy the
	stream spec only, and '-Af' to copy files only.
	See 'p4 help streamspecinteg' for more details.

	'p4 integ -S stream' will integrate the source stream spec propagatable
	fields into the target stream spec and the target stream spec will be
	opened for edit, along with stream files.  Use '-As' to integrate the
	stream spec only, and '-Af' to integrate files only.
	See 'p4 help streamspecinteg' for more details.

	'p4 merge -S stream' will merge the source stream spec propagatable
	fields into the target stream spec and the target stream spec will be
	opened for edit, along with stream files.  Use '-As' to merge the
	stream spec only, and '-Af' to merge files only.
	See 'p4 help streamspecinteg' for more details.

)"
};

ErrorId MsgHelp::HelpStreamSpecInteg = { ErrorOf( ES_HELP, 265, E_INFO, EV_NONE, 0 ),
R"(
    Integrating stream specs

	Stream specifications may be integrated from one stream to another.

	When a stream specification is integrated/merged/copied, any field in
	the source specification which has an Openable property value of
	'propagate' will be integrated/merged/copied to the target
	specification.  See 'p4 help streamspec' for more information about the
	stream specification properties.

    p4 copy [options] [-As | -Af] -S stream [-r] [-P parent]
            [file[revRange] ...]
    p4 integrate [options] [-As | -Af] -S stream [-r] [-P parent]
                 [file[revRange] ...]
    p4 merge [options] [-As | -Af] -S stream [-r] [-P parent]
             [file[revRange] ...]

	'p4 copy', 'p4 integrate', and 'p4 merge' now have options -Af and -As.

		-Af	Perform copy/integrate/merge with files only.
		-As	Perform copy/integrate/merge with stream specs only.

	If neither -Af nor -As is given, the copy/integrate/merge is done with
	both files and stream specs, if applicable.

	If '-r' is not given, then the stream specification given with '-S' is
	the source specification, and the parent is the target.

	If '-r' is given, then the stream specification given with '-S' is
	the target specification, and the parent is the source.

	In order to integrate a stream specification, the current client must
	be set to the target stream.

	The copy/integrate/merge commands will open the target stream
	specification for integrate; this can be seen using 'p4 open'.

	'p4 stream resolve' must be used to merge stream specification fields.
	'p4 submit' commits integrated stream specifications to the depot.
	Unresolved stream specifications may not be submitted.  Integrations
	can be shelved with 'p4 shelve -As', unshelved with 'p4 unshelve -As',
	and abandoned with 'p4 revert -So'.

	When 'p4 integrate -As' schedules a stream specification to be
	resolved, the specification is read-only.  For other pre-submit
	changes, 'p4 edit -So' must be used to make the specification writable.

	Use the configurable dm.integ.streamspec to enable/disable stream
	specification integration.  The configurable has 3 possible values:

	    0:  Stream spec integration is not allowed.
	    1:  Allow stream spec integration only for target streams with
	        noinherit ParentViews.  See 'p4 help streams' and 'p4 help
	        streamcmds' to read about the stream ParentView property.
	    2:  Allow stream spec integration for all streams.

	The default value for dm.integ.streamspec is 1.

    p4 istat [-As | -Af] [-a | -r] [-c | -C] [ -s ] stream

	'p4 istat' will report the combined integration status for both stream
	files and the stream spec of the specified stream.

	Use the -Af or -As option to limit the report of the status to only
	files or only the stream spec:

		-Af	Report integration status for stream files only.
		-As	Report integration status for the stream spec only.

	If the cache is cleared, the integration status for both files and
	stream spec are discovered, even if the requested report is only for
	files or only for the stream spec.

	The reporting and discovery of the stream status is dependent on the
	dm.integ.streamspec configurable:

	    0: Stream spec status is not discovered and not reported.
	    1: Stream spec status is discovered and reported only if the
	       stream has a noinherit ParentView property.
	    2: Stream spec status is discovered and reported regardless of
	       the ParentView property value.

    p4 streamlog [ -c  changelist# -h -i -l -L -t -m max ] stream ...

	'p4 streamlog' lists the revision and integration history of the
	specified stream specs.  See 'p4 help streamlog' for more details.

)"
};

ErrorId MsgHelp::HelpBGTask = { ErrorOf( ES_HELP, 231, E_INFO, EV_NONE, 0 ),
R"(
    bgtask -- run background commands/triggers on the server

    p4 bgtask [ -b retries -d -i interval -m runcount -w seconds]
              { -e command | -t triggername }

	Server-side background tasks.  This command runs a
	user-specified program at periodic intervals and reports the
	output in the server log file.  Commands may be defined in the
	triggers table or as strings given on the command line.

	-b Maximum number of execution errors before giving up.
	   The default is 1.
	-d Detach the client and continue task execution server-side.
	   Not allowed when used in a startup.N configurable.
	-e Command string to execute.
	-i Seconds between command invocations.  The default is 1 second.
	   The maximum is 31 days.
	-m Maximum number of times the command is run.  The default is 1.
	-t Name of the background trigger in the triggers table to execute.
	-w Seconds to pause after execution error.  The default is 5.

	Examples:

    	    # Run a background task trigger
    	    p4 bgtask -t daily_verify
    	    # Sample the server load.
    	    p4 bgtask -e "top -b -n 1"
    	    # See what errors have been logged.
    	    p4 bgtask -e "powershell -Command Import-Csv C:\p4root\errors.csv"

	'p4 bgtask' requires 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpCopy = { ErrorOf( ES_HELP, 127, E_INFO, EV_NONE, 0 ),
R"(
    copy -- Copy one set of files into another set of files, and/or copy
	one stream spec into another stream spec

    p4 copy [options] fromFile[rev] toFile
    p4 copy [options] -b branch [-r] [toFile[rev] ...]
    p4 copy [options] -b branch -s fromFile[rev] [toFile ...]
    p4 copy [options] [-As | -Af] -S stream [-P parent] [-F] [-r]
                      [toFile[rev] ...]

	options: -c changelist# -f -K -n -v -m max -q

	'p4 copy' copies one set of files (the 'source') into another (the
	'target').

	Using the client workspace as a staging area, 'p4 copy' makes the
	target identical to the source by branching, replacing, or deleting
	files.  'p4 submit' submits copied files to the depot. 'p4 revert'
	can be used to revert copied files instead of submitting them.  The
	history of copied files can be shown with 'p4 filelog' or 'p4
	integrated'.

	Target files that are already identical to the source, or that are
	outside of the client view, are not affected by 'p4 copy'. Opened,
	non-identical target files cause 'p4 copy' to exit with a warning.
	When 'p4 copy' creates or modifies files in the workspace, it leaves
	them read-only; 'p4 edit' can make them writable.  Files opened by
	'p4 copy' do not need to be resolved.

	Source and target files (fromFile and toFile) can be specified on
	the 'p4 copy' command line or through a branch view. On the command
	line, fromFile is the source file set and toFile is the target file
	set.  With a branch view, one or more toFile arguments can be given
	to limit the scope of the target file set.

	A revision specifier can be used to select the revision to copy; by
	default, the head revision is copied. The revision specifier can be
	used on fromFile, or on toFile, but not on both.  When used on toFile,
	it refers to source revisions, not to target revisions.  A range may
	not be used as a revision specifier.  For revision syntax, see 'p4
	help revisions'.

	The -S flag makes 'p4 copy' copy from a stream to its parent.
	Use -r with -S to reverse the copy direction.  Note that to submit
	copied stream files, the current client must be switched to the
	target stream, or to a virtual child stream of the target stream.
	The -S flag causes 'p4 copy' to use a generated branch view that
	maps the stream to its parent.  -P can be used to generate the
	branch view using a parent stream other than the stream's actual
	parent.  The -S flag also makes 'p4 copy' respect a stream's flow.
	By default, copying between streams requires the use of a stream view.

	The -F flag can be used to force copying against a stream's expected
	flow, or to use an arbitrarily specified view to copy into a stream.

	Stream specifications may also be copied along with stream files.
	Any field in the source specification which has an Openable property of
	'propagate' will be copied to the target specification. The -Af and
	-As flags control whether stream files or the stream spec is copied.

		-Af	Perform copy with files only.
		-As	Perform copy with the stream spec only.

	Both stream files and the stream spec will be copied if neither -Af
	nor -As is specified.  See 'p4 help streamspecinteg' for more details.

	The -b flag makes 'p4 copy' use a user-defined branch view.  (See
	'p4 help branch'.) The source is the left side of the branch view
	and the target is the right side. With -r, the direction is reversed.

	The -s flag can be used with -b to cause fromFile to be treated as
	the source, and both sides of the user-defined branch view to be
	treated as the target, per the branch view mapping.  Optional toFile
	arguments may be given to further restrict the scope of the target
	file set. -r is ignored when -s is used.

	The -c changelist# flag opens files in the designated (numbered)
	pending changelist instead of the default changelist.

	The -f flag forces the creation of extra revisions in order to
	explicitly record that files have been copied.  Deleted source files
	will be copied if they do not exist in the target, and files that are
	already identical will be copied if they are not connected by existing
	integration records.

	The -K flag suppresses keyword expansion when updating ktext files on
	the client.

	The -n flag displays a preview of the copy, without actually doing
	anything.

	The -m flag limits the actions to the first 'max' number of files.

	The -q flag suppresses normal output messages. Messages regarding
	errors or exceptional conditions are displayed.

	The -v flag causes a 'virtual' copy that does not modify client
	workspace files.  After submitting a virtual integration, 'p4 sync'
	can be used to update the workspace.
)"
};

ErrorId MsgHelp::HelpCounter = { ErrorOf( ES_HELP, 33, E_INFO, EV_NONE, 0 ),
R"(
     counter -- Display, set, or delete a counter

     p4 counter name
     p4 counter [-f -v] name value
     p4 counter [-f] -d name
     p4 counter [-f -v] -i name
     p4 counter [-f] -m [ pair list ]
     p4 counter --from=oldValue --to=newValue name

	The first form displays the value of the specified counter.

	The second form sets the counter to the specified value.

	The third form deletes the counter.  This option usually has the
	same effect as setting the counter to 0.

	The -f flag sets or deletes counters used by P4, which are listed by
	'p4 help counters'. Important: Never set the 'change' counter to a
	value that is lower than its current value.

	The -i flag increments a counter by 1 and returns the new value.
	This option is used instead of a value argument and can only be
	used with numeric counters.

	The -v flag displays the previous value of the specified counter
	after the counter has been set or incremented.

	The -m flag in the fifth form allows multiple operations in one
	command. With this form, the list is pairs of arguments.  Each pair
	is either counter value or '-' counter.  To set a counter use a name
	and value. To delete a counter use a '-' followed by the name.

	The sixth form sets a counter to the newValue only if the current
	value of the counter is oldValue. Note that a counter that has never
	been set (or has been deleted) cannot be set using this form.

	Counters can be assigned textual values as well as numeric ones,
	despite the name 'counter'.

	'p4 counter' requires 'review' access granted by 'p4 protect'.
	The -f flag requires that the user be an operator or have 'super'
	access.
)"
};

ErrorId MsgHelp::HelpCounters = { ErrorOf( ES_HELP, 34, E_INFO, EV_NONE, 0 ),
R"(
    counters -- Display list of known counters

    p4 counters [-e nameFilter -m max]

	Lists the counters in use by the server.  The server
	uses the following counters directly:

	    change           Current change number
	    job              Current job number
	    journal          Current journal number
	    lastCheckpointAction Data about the last complete checkpoint
	    logger           Event log index used by 'p4 logger'
	    maxCommitChange  Last change number successfully submitted
	    traits           Internal trait lot number used by 'p4 attribute'
	    upgrade          Server database upgrade level

	The -e nameFilter flag lists counters with a name that matches
	the nameFilter pattern, for example: -e 'mycounter-*'.

	The -m max flag limits the output to the first 'max' counters.

	The names 'minClient', 'minClientMessage', 'monitor',
	'security', 'masterGenNumber', and 'unicode' are
	reserved names: do not use them as ordinary counters.

	For general-purpose server configuration, see 'p4 help configure'.
)"
};

ErrorId MsgHelp::HelpCstat = { ErrorOf( ES_HELP, 123, E_INFO, EV_NONE, 0 ),
R"(
    cstat -- Dump change/sync status for current client

    p4 cstat [files ...]

	Lists changes that are needed, had or partially synced in the current
	client. The output is returned in tagged format, similar to the fstat
	command.

	The fields that cstat displays are:

		change   changelist number
		status   'have', 'need' or 'partial'

)"
};

ErrorId MsgHelp::HelpDepot = { ErrorOf( ES_HELP, 35, E_INFO, EV_NONE, 0 ),
R"(
    depot -- Create or edit a depot specification

    p4 depot [-t type] name
    p4 depot -d [-f] name
    p4 depot -o name
    p4 depot -i

	Create a new depot specification or edit an existing depot
	specification. The specification form is put into a temporary file
	and the editor (configured by the environment variable $P4EDITOR)
	is invoked.

	The depot specification contains the following fields:

	Depot:       The name of the depot.  This name cannot be the same as
		     any branch, client, or label name.

	Owner:       The user who created this depot.

	Date:        The date that this specification was last modified.

	Description: A short description of the depot (optional).

	Type:        One of: 'local', 'stream', 'remote', 'spec', 'archive',
		     'unload', 'tangent', 'extension', 'graph', 'trait'.

		     A 'local' depot (the default) is managed directly by
		     the server and its files reside in the server's root
		     directory.

		     A 'stream' depot is a local depot dedicated to the
		     storage of files in a stream.

		     A 'remote' depot refers to files in another P4 server.

		     A 'spec' depot automatically archives all edited forms
		     (branch, change, client, depot, group, job, jobspec,
		     protect, triggers, typemap, and user) in special,
		     read-only files.  The files are named:
		     //depotname/formtype/name[suffix].  Updates to jobs made
		     by the 'p4 change', 'p4 fix', and 'p4 submit' commands
		     are also saved, but other automatic updates such as
		     as access times or opened files (for changes) are not.
		     A server can contain only one 'spec' depot.

		     An 'archive' depot defines a storage location to which
		     obsolete revisions may be relocated.

		     An 'unload' depot defines a storage location to which
		     database records may be unloaded and from which they
		     may be reloaded.

		     A 'tangent' depot defines a read-only location which
		     holds tangents created by the 'fetch -t' command. The
		     tangent depot named 'tangent' is automatically created
		     by 'fetch -t' if one does not already exist.

		     An 'extension' depot stores files related to P4
		     extensions.  See 'p4 help extension'.

		     A 'graph' depot defines a storage location to which
		     one or more git repositories are represented using
		     the git data model.

		     A 'trait' depot defines a storage location to which
		     huge traits may be stored. The storage location behavior
		     is determined by the 'p4 attribute' command or by the
		     setting of the 'trait.storedepot.min' configurable.
		     A server can contain only one 'trait' depot.
		     See 'p4 help attribute' and 'p4 help configurables'.
 
	Address:     For remote depots, the $P4PORT (connection address)
		     of the remote server.
		     For non remote depots, this may be used to specify an
		     S3 bucket as an alternative backing storage system.
		     See below.

	Suffix:      For spec depots, the optional suffix to be used
		     for generated paths. The default is '.p4s'.

	StreamDepth: For stream depots, the optional depth to be used
		     for stream paths in the depot, where depth equates
	             to the number of slashes following the depot name of a
	             stream.  This field is referenced when streams are
	             being created. The default is '1', matching the
	             traditional stream name.  This value may not be updated
	             once streams or archive data exist within the depot.

	Map:         Path translation information, in the form of a file
		     pattern with a single ... in it.  For local depots,
		     this path is relative to the server's root directory
		     or to server.depot.root if it has been configured
		     (Example: depot/...).  For remote depots, this path
		     refers to the remote server's namespace
		     (Example: //depot/...).

	SpecMap:     For spec depots, the optional description of which
	             specs should be saved, as one or more patterns.

	The -d flag deletes the specified depot.  If any files reside in the
	depot, they must be removed with 'p4 obliterate' before deleting the
	depot. If any archive files remain in the depot directory, they may
	be referenced by lazy copies in other depots; use 'p4 snap' to break
	those linkages. Snap lazy copies prior to obliterating the old depot
	files to allow the obliterate command to remove any unreferenced
	archives from the depot directory. If the depot directory is not
	empty, you must specify the -f flag to delete the depot.

	The -t flag is used to insert a value into the type field of a new
	depot spec.  This flag is ignored when used with existing depots.

	The -o flag writes the depot specification to standard output. The
	user's editor is not invoked.

	The -i flag reads a depot specification from standard input. The
	user's editor is not invoked.


	S3 storage of archives can be used for all depot types except 'remote'
	by specifying the bucket configuration in the 'Address' field.
	The configuration must be provided in the form of ':' separated options
	and values on a single line, separated by commas (',') and preceded
	by 's3'. E.g:
	    Address: s3,bucket:mybucket,region:us-east-1

	The available options are:
	    - bucket    - The name of the bucket (required)
	    - url       - URL to the bucket
	                  (defaults to https://bucketname.s3.amazonaws.com)
	    - region    - The region the bucket resides in
	                  (optional depending on s3 implementation)
	    - accessKey - The S3 account's access key (required)
	    - secretKey - The S3 account's secret key (required)
	    - token     - The S3 account's session token
	                  (optional depending on bucket security)
	    - profile   - Fetch the S3 account's credentials from the
	                  .aws/credentials file for the specified profile.
	    - role      - Fetch the S3 account's credentials from the
	                  specified EC2 role.

	Only one of the credential mechanisms can be specified, either
	accessKey/secretkey/token or profile or role.

	Note: Depending on several factors, including network latency and
	bandwidth, using S3 buckets for archive storage may not perform well
	enough to be practical.
)"
};

ErrorId MsgHelp::HelpDepots = { ErrorOf( ES_HELP, 36, E_INFO, EV_NONE, 0 ),
R"(
    depots -- Lists defined depots

    p4 depots [-t type] [[-e|-E] nameFilter] [-a]

	The depots command lists all depots defined in the server.

	The -t flag limits output to depots of the specified type. The
	following types are valid: 'local', 'spec', 'stream', 'remote',
	'archive', 'unload', 'graph', 'tangent', 'extension', 'trait'.

	The -e nameFilter flag lists depots with a name that matches
	the nameFilter pattern, for example: -e 'depo*'. The -e flag
	uses the server's normal case-sensitivity rules. The -E flag makes
	the matching case-insensitive, even on a case-sensitive server.

	With -a flag the command will include hidden depots in its output.
)"
};

ErrorId MsgHelp::HelpDiskspace = { ErrorOf( ES_HELP, 133, E_INFO, EV_NONE, 0 ),
R"(
    diskspace -- Display disk space information on the server.

    p4 diskspace [ P4ROOT | P4JOURNAL | P4LOG | TEMP | journalPrefix | depot ]

	Shows summary information about the current availability of disk
	space on the server.

	If no arguments are specified, disk space information for all
	relevant file systems is displayed; otherwise the output is
	restricted to the named filesystem(s).

	This command requires that the user be an operator or have 'super'
	access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpSizes = { ErrorOf( ES_HELP, 94,  E_INFO, EV_NONE, 0 ),
R"(
    sizes -- Display information about the size of the files in the depot

    p4 sizes [-a -S] [-s | -z] [-b size] [-h|-H] [-m max] file[revRange] ...
    p4 sizes -C [-K] [-Q charset] [-B utf8bom] [-L line-ending] [-a -S]
                [-h|-H] [-m max] file[revRange] ...
    p4 sizes -A [-a] [-s] [-b size] [-h|-H] [-m max] archivefile...
    p4 sizes -U unloadfile ...

	For specified file specification, displays depot file name, revision,
	file count and file size.  If you use client syntax for the file
	specification,the view mapping is used to list the corresponding
	depot files.

	The -a flag lists all revisions within the specific range, rather
	than just the highest revision in the range.

	The -b option specifies the blocksize in bytes.  When this option
	is specified, each accumulated filesize is rounded up to the nearest
	blocksize.

	The -h or -H flag prints the sizes in human-readable form, using a
	scaling factor of 1024 for -h or 1000 for -H.

	The -m flag limits sizes to the first 'max' number of files.

	The -s option calculates the sum of the file sizes for the specified
	files.

	The -S option displays size information for shelved files only. With
	this option, revision specifications are not permitted.

	The -z option omits lazy copies when calculating the file sizes for
	the specified files.

	If there is no file size information for a file revision in the depot
	metadata, the sizes command returns a size of "<n/a>".

	'p4 verify -u' updates the metadata for all revisions that require it.

	The -C flag reports the file size as expected in the workspace,
	defaulting to the client's environment, accounting for line-endings,
	charset conversion of unicode/utf8/utf16 filetypes (including the BOM
	for utf8 type files) and keyword expansion for +k type files.
	These variables can be explicitly overridden with the following flags:
	   -B utf8bom     - Override 'filesys.utf8bom' for utf8 type files
	   -K             - Collapse keywords for +k type files
	   -L line-ending - Override the client's line ending setting
	   -Q charset     - Override P4CHARSET for unicode type files

	Where client file size information would not match the server's, the
	size is calculated server-side on request and cached for future use.

	The -A flag displays files in archive depots (see 'p4 help archive').

	The -U option displays size information for unload files in the
	unload depot (see 'p4 help unload').
)"
};

ErrorId MsgHelp::HelpDelete = { ErrorOf( ES_HELP, 37, E_INFO, EV_NONE, 0 ),
R"(
    delete -- Open an existing file for deletion from the depot

    p4 delete [-c changelist#] [-n -v -k] [--remote=rmt] file ...

	Opens a depot file for deletion.
	If the file is synced in the client workspace, it is removed.  If a
	pending changelist number is specified using with the -c flag, the
	file is opened for delete in that changelist. Otherwise, it is opened
	in the default pending changelist.

	Files that are deleted generally do not appear on the have list.

	The -n flag displays a preview of the operation without changing any
	files or metadata.

	The -k flag performs the delete on the server without modifying
	client files.  Use with caution, as an incorrect delete can cause
	discrepancies between the state of the client and the corresponding
	server metadata.

	The -v flag enables you to delete files that are not synced to the
	client workspace. The files should be specified using depot syntax;
	since the files are not synced, client syntax or local syntax are
	inappropriate and could introduce ambiguities in the file list.
	Note, though, that if the files ARE synced, 'delete -v' will remove
	the files from the client in addition to opening them for delete.
	The preferred way to delete a set of files without transferring
	them to your machine is: 'sync -k file...', then 'delete -k file...'

	The --remote flag is useful for DVCS configurations in which files
	of type +l are in use. 'p4 delete --remote=origin filename' opens the
	named file for deletion in your DVCS server, and additionally, if the
	file is of type +l, takes a global exclusive lock on the origin
	server. That global exclusive lock will be retained until you push
	the deleted file to the origin server, or until you use the
	'p4 revert --remote=origin filename' command to revert the file.

	See 'p4 help-graph delete' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpDescribe = { ErrorOf( ES_HELP, 38, E_INFO, EV_NONE, 0 ),
R"(
    describe -- Display a changelist description

    p4 describe [-d<flags> -a -m -s -S -f -O -I] changelist# ...

	Display a changelist description, including the changelist number,
	user, client, date of submission, textual description, list of
	affected files and diffs of files updated, affected stream spec and
	diff of updated stream spec.  Pending changelists are indicated as
	'pending' and file diffs are not displayed.

	For restricted changelists, 'no permission' is displayed if the user
	is not permitted to view the change (see 'p4 help change'). If a
	submitted change is restricted, the description is hidden unless
	the user is the owner of the change or has list permission for
	at least one file in the change. If a shelved change is restricted,
	the description is hidden unless the user is the owner of the change,
	or has list permission to at least one file in the change and has
	used the -S flag to request the shelved change. To view restricted
	pending (not shelved) changes, the user must be the owner of the
	change.

	The -a flag will display the content of added files in addition
	to diff of updated files.

	The -d<flags> passes one or more flags to the built-in diff routine
	to modify the output: -dn (RCS), -dc[n] (context), -ds (summary),
	-du[n] (unified), -db (ignore whitespace changes), -dw (ignore
	whitespace), -dl (ignore line endings). The optional argument to
	to -dc/-du specifies number of context lines.

	The -s flag omits the diffs of files that were updated.

	The -S flag lists a stream spec and files that are shelved for the
	specified changelist and displays diffs of the files against their
	previous revision.  If the change is restricted, the description is
	displayed according to the rules for shelved restricted changes
	described above.

	The -f flag forces display of the descriptions in a restricted
	change.  The -f flag requires 'admin' access, which is granted
	using 'p4 protect'.

	The -O flag specifies that the changelist number is the original
	number of a changelist which was renamed on submit.

	The -I flag specifies that the changelist number is the Identity
	field of a changelist.

	The -m flag limits files to the first 'max' number of files.

	See 'p4 help-graph describe' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpDiff = { ErrorOf( ES_HELP, 39, E_INFO, EV_NONE, 0 ),
R"(
    diff -- Diff utility for comparing workspace content to depot content
    (supports files and stream spec). (For comparing two depot paths, see
    p4 diff2.)

    p4 diff [-d<flags> -f -m max -Od -s<flag> -t] [file[rev] ...]
    p4 diff [-d<flags>] -As [streamname[@change]]

	On the client machine, diff a client file against the corresponding
	revision in the depot. The file is compared only if the file is
	opened for edit or a revision is provided. See 'p4 help revisions'
	for details about specifying revisions.

	If the file specification is omitted, all open files are diffed.
	This option can be used to view pending changelists.

	The -As flag can be used to diff an opened stream spec with either
	the head version of the stream spec (the default), or against the
	specified stream spec at either head or the specified change number.
	To diff a shelved version of the specified stream spec, use the
	revision specification '@=change', where 'change' is the shelf
	changelist number.

	The -d<flags> modify the output as follows:
	    -dn (RCS),
	    -dc[n] (context),
	    -ds (summary),
	    -du[n] (unified),
	    -db (ignore whitespace changes),
	    -dw (ignore whitespace),
	    -dl (ignore line endings).
	The optional argument to -dc/-du specifies number of context lines.

	The -f flag diffs every file, regardless of whether they are opened
	or the client has synced the specified revision.  This option can be
	used to verify the contents of the client workspace.

	The -m max flag limits output to the first 'max' number of files,
	unless the -s flag is used, in which case it is ignored.

	The -Od flag limits output to files that differ.

	The -s options lists the files that satisfy the following criteria:

		-sa     Opened files that differ from the revision
			in the depot or are missing.

		-sb     Files that have been opened for integrate, resolved,
			and subsequently modified.

		-sd     Unopened files that are missing on the client.

		-se     Unopened files that differ from the revision
			in the depot.

		-sl	Every unopened file, along with the status of
			'same, 'diff', or 'missing' as compared to the
			corresponding revision in the depot.

		-sr     Opened files that do not differ from the revision in
			the depot.

	Note that if a revision is provided in the file specification, the -s
	options compare the file(s) regardless of whether they are opened
	or the client has synced the specified revision.

	The -t flag forces 'p4 diff' to diff binary files.

	If the environment variable $P4DIFF is set,  the specified diff
	program is launched in place of the default P4 client diff.
	The -d<flags> option can be used to pass arguments to the diff
	program.  Because the -s flag is only implemented internally, any
	-d<flags> option used with the -s<flag> is ignored. To configure a
	diff program for Unicode files, set the environment variable
	$P4DIFFUNICODE. Specify the file's character set as the first
	argument to the program.

	See 'p4 help-graph diff' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpDiff2 = { ErrorOf( ES_HELP, 40, E_INFO, EV_NONE, 0 ),
R"(
    diff2 -- Diff utility for comparing the content at two depot paths
    (supports files and stream specs). (For comparing workspace content to
    depot content, see p4 diff.)

    p4 diff2 [options] fromFile[rev] toFile[rev]
    p4 diff2 [options] -b branch [[fromFile[rev]] toFile[rev]]
    p4 diff2 [options] [-S stream] [-P parent] [[fromFile[rev]] toFile[rev]]
    p4 diff2 [-d<flags>] -As streamname1[@change1] streamname2[@change2]

    	options: -d<flags> -Od -q -t -u

	'p4 diff2' runs on the server to compare one set of depot files (the
	'source') to another (the 'target').  Source and target file sets
	can be specified on the 'p4 diff2' command line or through a branch
	view.

	With a branch view, fromFile and toFile are optional; fromFile limits
	the scope of the source file set, and toFile limits the scope of the
	target. If only one file argument is given, it is assumed to be
	toFile.

	fromFile and toFile can include revision specifiers; by default, the
	head revisions are diffed.  See 'p4 help revisions' for details
	about specifying file revisions.

	'p4 diff2' precedes each diffed file pair with a header line of the
	following form:

	    ==== source#rev (type) - target#rev (type) ==== summary

	A source or target file shown as '<none>' means there is no file
	at the specified name or revision to pair with its counterpart.
	The summary status is one of the following: 'identical' means file
	contents and types are identical, 'types' means file contents are
	identical but the types are different, and 'content' means file
	contents are different.

	The -b flag makes 'p4 diff2' use a user-defined branch view.  (See
	'p4 help branch'.) The left side of the branch view is the source
	and the right side is the target.

	The -S flag makes 'p4 diff2' use a generated branch view that maps
	a stream (or its underlying real stream) to its parent.  This defaults
	to the current client's stream, if applicable.  -P can be used to
	generate the branch view using a parent stream other than the stream's
	actual parent.

	The -As flag can be used to diff two different stream specifications.
	By default, the head version of a given stream spec is used; if a
	change list number is given, the stream spec version associated with
	the changelist will be used instead. If '@=change' is given, the
	shelved stream spec version associated with the changelist will be
	used instead.

	The -d<flags> modify the output of diffs as follows:

		-dn (RCS)
		-dc[n] (context)
		-ds (summary)
		-du[n] (unified)
		-db (ignore whitespace changes)
		-dw (ignore whitespace)
		-dl (ignore line endings).

	The optional argument to -dc/-du specifies number of context lines.

	The -Od flag limits output to files that differ.

	The -q omits files that have identical content and types and
	suppresses the actual diff for all files.

	The -t flag forces 'p4 diff2' to diff binary files.

	The -u flag uses the GNU diff -u format and displays only files
	that differ. The file names and dates are in P4 syntax, but the output
	can be used by the patch program.

	See 'p4 help-graph diff2' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpDirs = { ErrorOf( ES_HELP, 41, E_INFO, EV_NONE, 0 ),
R"(
    dirs -- List depot subdirectories

    p4 dirs [-C -D -H] [-S stream] [-i] dir[revRange] ...
    p4 dirs [-D] [ --streamviews ] dir[revRange] ...

	List directories that match the specified file pattern (dir).
	This command does not support the recursive wildcard (...).
	Use the * wildcard instead.

	P4 does not track directories individually. A path is treated as a
	directory if there are any undeleted files with that path as a prefix.

	By default, all directories containing files are listed. If the dir
	argument includes a revision range, only directories containing files
	in the range are listed. For details about specifying file revisions,
	see 'p4 help revisions'.

	The -i flag is used to ignore the case of the file pattern when
	listing directories in a case sensitive server. This flag is not
	compatible with the -C option.

	The -C flag lists only directories that fall within the current
	client view.

	The -D flag includes directories containing only deleted files.

	The -H flag lists directories containing files synced to the current
	client workspace.

	The -S flag limits output to depot directories mapped in a stream's
	client view.

	The --streamviews option reports results for streams in the context
	of the stream.

	See 'p4 help-graph dirs' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpDuplicate = { ErrorOf( ES_HELP, 104, E_INFO, EV_NONE, 0 ),
R"(
    duplicate -- duplicate revisions with integration history (unsupported)

    p4 duplicate [-n -q] from[revRange] to

	Duplicate revisions as if they always existed.  All aspects of the
	source revisions are mirrored to the target revisions, including
	changelist number, date, attributes, and contents.  The target
	revision must not already exist and the target file must not be
	opened (for any operation) on any client.

	Integration records are duplicated as well.

	'p4 duplicate' followed by a 'p4 obliterate' (of the source
	revisions) is in effect a deep rename operation, with any source
	revision in client workspace or labels forgotten.

	The -n flag displays what would be duplicated without actually
	copying anything.

	The -q flag suppresses the warning about target revisions already
	existing.

	'p4 duplicate' requires 'admin' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpEdit = { ErrorOf( ES_HELP, 42, E_INFO, EV_NONE, 0 ),
R"(
    edit -- Open an existing file or stream spec for edit

    p4 edit [-c changelist#] [-k -n] [-t filetype] [--remote=rmt] file ...
    p4 edit [-c changelist#] -So
    
	Open an existing file for edit.  The server records the fact that
	the current user has opened the file in the current workspace, and
	changes the file permission from read-only to read/write.

	If -c changelist# is included, the file opened in the specified
	pending changelist.  If changelist number is omitted, the file is
	opened in the 'default' changelist.

	If -t filetype is specified, the file is assigned that P4 filetype.
	Otherwise, the filetype of the previous revision is reused.
	If a partial filetype is specified, it is combined with the current
	filetype.  For details, see 'p4 help filetypes'.
	Using a filetype of 'auto' will cause the filetype to be chosen
	as if the file were being added, that is the typemap will be
	considered and the file contents may be examined.

	The -n flag previews the operation without changing any files or
	metadata.

	The -k flag updates metadata without transferring files to the
	workspace. This option can be used to tell the server that files in
	a client workspace are already editable, even if they are not in the
	client view. Typically this flag is used to correct the P4 server when
	it is wrong about the state of files in the client workspace, but
	incorrect use of this option can result in inaccurate file status
	information.

	The --remote flag is useful for DVCS configurations in which files
	of type +l are in use. 'p4 edit --remote=origin filename' opens the
	named file for edit in your DVCS server, and additionally, if the
	file is of type +l, takes a global exclusive lock on the origin
	server. That global exclusive lock will be retained until you push
	the updated file to the origin server, or until you use the
	'p4 revert --remote=origin filename' command to revert the file.

	The -So flag can be used with '-c change' to open the client's stream
	spec for edit.  (See 'p4 help streamcmds'.)

	See 'p4 help-graph edit' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpExtension = { ErrorOf( ES_HELP, 251, E_INFO, EV_NONE, 0 ),
R"(
    extension -- Manage the P4 Server extensibility mechanism

    p4 extension --sample extName
    p4 extension --package dir [--sign=keyDir]
    p4 extension --install extName.p4-extension [--yes]
    p4 extension --install certificateFile --cert [ --comment ]
    p4 extension --delete extFQN [ --name instName ] [ --revision=n ]
                 [ --path filespec ] [--yes]
    p4 extension --delete fingerprint --cert

    p4 extension --configure extFQN [ --revision=n ] [ -o | -i ]
    p4 extension --configure extFQN [ --revision=n ] [ -o | -i ]
                 --name instName

    p4 extension --run instName [ extArguments ]

    p4 extension --list --type type

	'p4 extension' manages the installation, versioning and configuration
	of P4 Server extensions.

	For an introduction to extensions, see 'p4 help serverextensionintro'
	and 'p4 help clientextensionintro'.  Further reference is available
	in the online documentation.

	Extensions are referred to by their fully-qualified name ('extFQN').
	This is the combination of the extension's namespace, name and optional
	depot file revision.  E.g. a namespace of 'ExampleInc' and name of
	'extName' would be referred to as 'ExampleInc::extName', or with
	a revision of '6', 'ExampleInc::extName#6'.

	By default, installation ( --install ) and deletion ( --delete ) of an
	extension displays a preview of the results. To execute the operation,
	you must also specify the --yes flag.

	The --sample flag creates a skeleton of a new extension.  The argument
	is the name of the extension to create as well as the name of the local
	directory where the files are placed.  The sample extension has
	placeholder values that should be replaced when developing the code.

	The --package flag creates a packaged extension file from the named
	directory of files, preparing it for --install on the server.  The
	resulting file has the same name as the directory, but with the
	'.p4-extension' suffix.  When used with the --sign flag, the files in
	the extension package are cryptographically signed by the key pair in
	the directory specified with --sign (containing 'privatekey.txt' and
	'certificate.txt').  Signing adds two files to the extension
	package:  a copy of the public key, renamed to '.p4-certificate.txt',
	and the signatures of the files in the extension '.p4-signatures.json'.
	Packaging is done client-side and requires a version 2020.1 or newer
	command-line client.

	The --install flag is passed the full name of a packaged extension file
	residing on the client and installs it server-side, where it can then
	be instantiated with --configure.  When --install is used with
	--cert, the fingerprint of the certificate pointed to with --install
	is stored and used to validate the signatures of signed extensions
	prior to installation.  If signing enforcement is enabled on the
	server (via the 'server.extensions.allow.unsigned' configurable), the
	signing certificate must be installed prior to the extension, via the
	'--install --cert' options; alternatively use the --allow-unsigned
	option to allow unsigned extensions on the server.  The --comment
	flag goes with --cert to provide a short piece of text describing
	the certificate.

	The --configure flag is used to supply details about an extension's
	configuration.  There are two parts to configuring an extension - the
	'global' configuration, and the 'instance' configuration.  To create
	a global configuration, use --configure with the FQN of the extension.
	Use --configure with --name together to create an instance
	configuration.  The --name flag takes the name of the configuration to
	create or modify.  More than one named instance is allowed for most
	event types that an extension may be coded to work with (e.g. the
	'change-submit' event, but not 'auth-check').

	The --revision flag specifies which depot version of the extension to
	configure.  The default is the head rev.  This flag applies to the
	--configure flag when used with or without the --name flag.

	The --list flag displays installed extensions, configurations, and
	certificate fingerprints.  The value of the --type flag determines
	which data is reported.  Values for --type are: 'extensions', 'configs'
	(further narrowed by 'global' and 'instance'), and 'certs'.  Adding
	the --path flag filters instance configuration output to instances
	whose events are path-based and match the specified filespec.

	The --delete flag deletes an extension, its configurations, or both.
	Passed the --name flag, just the named instance configuration will be
	removed.  Pass the --path flag to delete file-based instances by
	the part of the depot they affect.  When used without a --name,
	all revisions of an extension and its configurations are removed,
	unless the --revision flag is used, in which case just the
	specific revision and instances using that revision are removed.
	When used with --cert, it deletes a trusted certificate.

	The --run flag is used to call an extension-supplied command.  An
	extension can implement its own end-user commands that behave similarly
	to native P4 commands in that they can have tagged output, send
	RPC-level messages, and open forms client-side.  They are restricted
	to reporting output and may not access client-side content.  The --run
	flag takes an instance configuration as its argument, and any other
	arguments are left to the extension to interpret.

	This command requires 'super' access granted by 'p4 protect' unless
	the 'server.extensions.allow.admin' configurable is set to 1.  Users
	who are members of groups specified in the 'ExtAllowedGroups' field
	of an extension's global configuration and who are either a depot
	or repo owner may create/configure instances of file-based extensions
	to operate on the sections of the server they manage.

	Extensions do not run on metadata-only replicas.
)"
};

ErrorId MsgHelp::HelpClientExtensionIntro = { ErrorOf( ES_HELP, 261, E_INFO, EV_NONE, 0 ),
R"(
    Introduction to P4 client extensions (Technical Preview)

	Client-side extensions are a means for customizing parts of
	P4 clients with user-supplied logic.

	P4 clients run extensions natively without relying on external
	processes, providing a portable runtime and versioned programmatic API.
	Extensions are written in the Lua language, version 5.3.  There are two
	types of client-side extension: loose extensions and packaged
	extensions.  A loose extension is a bare script source file, and a
	packaged extension is a package of content - a manifest, source code,
	assets, etc all packaged up in a '.p4-extension' file.

	The P4EXTENSIONS environmental variable enables client-side
	extensions.  The value of this variable specifies a file
	pattern the client program uses when searching the client
	filesystem for extensions.  When executing a command, the
	client program load the P4EXTENSIONS variable, and like
	P4CONFIG, uses the current working directory as its starting
	point and moves its way to the root of the tree in search of
	file matches, keeping any that it encounters.  Unlike
	P4CONFIG, P4EXTENSIONS does not specify a literal file name.
	To leave room in the future for the existence of multiple
	scripting options, P4EXTENSIONS only specifies a filename
	prefix.  Of files that match the prefix, there must be a
	matching suffix in the form of '.version.language'.  E.g. if
	P4EXTENSIONS is set to 'p4script', a matching file would be
	named 'p4script-example.53.lua', where the '53' is the '5.3'
	of the bundled version of Lua.  Currently the only supported
	script type is 'lua' (case-sensitive) and the version is '53'.
	These two fields in the file name are to give the client a
	hint about what interpreter to load when running the loose
	extension since they don't have a manifest file to declare
	those details.  P4EXTENSIONS may point to an absolute path, in
	which case the client does not search outside of the specified
	directory.

	Client-side extensions can be disabled either by removing the
	P4EXTENSIONS variable from the environment, or by using the
	--no-script global flag.

	Client-side extensions are run before and after command
	execution (pre/post hooks).

	Certain commands can not be handled with extensions.  E.g. 'p4 trust'
	and 'p4 transmit'.

	The --script flag runs the named script file and exits.  The
	script is not an extension here, but rather just a normal
	program being run for its side effects.

	Refer to the online documentation for instructions on creating
	extensions.  Server-side extensions are largely the same, but
	have additional APIs.  Note that for client-side extensions,
	that the server-side help may not reflect client program
	capabilities if the versions don't match.
)"
};

ErrorId MsgHelp::HelpServerExtensionIntro = { ErrorOf( ES_HELP, 253, E_INFO, EV_NONE, 0 ),
R"(
    Introduction to P4 server extensions

	Extensions are a means for customizing parts of the P4 with
	user-supplied logic.

	Extensions are self-contained packages of third-party code and assets
	run within the P4 server, used for change submission validation, form
	validation, external authentication, external job fix integration,
	external archive integration, and command policies.
	See the list of trigger types in 'p4 help triggers' for a list of
	events that extensions may register for.

	The P4 server runs extensions natively, without relying on external
	processes, so it can provide a portable, versioned runtime with
	automatic replication and a programmatic API.

	Extensions coexist with 'p4 triggers', but offer more functionality.

	The 'p4 extension' command manages the installation and configuration
	of extensions.  See 'p4 help extension'.

	Extensions are versioned within the P4 server and are stored in a
	special 'extension' depot, named '.p4-extensions' by default.  The
	extension depot is only accessible to the super user and by read-only
	commands.  The extension depot is created automatically.  Installing
	or upgrading an extension creates a changelist with information about
	pre/post install versions.  Multiple revisions of an extension may be
	active at any given time and may be referenced by their revision
	number.

	An extension is not active until it has been configured.  An extension
	may have multiple configurations, depending on the events it registers
	for.  Each configuration of an extension may point to a different
	version of the extension.

	Refer to the online documentation for instructions on creating
	extensions.
)"
};

ErrorId MsgHelp::HelpFailback = { ErrorOf( ES_HELP, 274, E_INFO, EV_NONE, 0 ),
R"(
    failback -- Fail back to the failed server from its previous failover

    p4 failback [ -y ] [ -w <quiesce wait> ]
                [ -v <verification time> ] [ <failback message> ]

	Fail back to the original master server from a previous failover
	to this server. The original failover must have been run with master
	participation. Both server versions must be at least 22.1 to
	participate in failback.

	The failback command must be run on the server that failed over
	during the failover command. In other words, the P4ROOT directory
	of this server must be the same location as the original master.
	The server must first be prepared for failback using 'p4d -Fm'
	to change it to a restricted standby. Then the server could be
	manually changed to become a mandatory standby after replication
	has caught up to the master. After running the failback command,
	this server is restored to its status as the master server.
	The original standby server should be then restored to a standby for
	this server using 'p4d -Fs'. For a more detailed description of
	this failback process, please refer to 'p4 help failbackintro'.

	The 'p4 failback' command operates the same as 'p4 failover',
	except that it requires master participation. For a more detailed
	description of the failover process, please refer to
	'p4 help failover'.

	The '-v' argument specifies the window of time, in seconds,
	during which classic depot file content that was updated prior to
	the failback will be verified to ensure that it will be correct on
	the new master server. By default, the 'p4 failback' command verifies
	the file content that was updated during the 300 seconds prior to the
	failback. The command does not wait for this amount of time; it only
	waits as long as it takes to verify the file content that was
	updated during this window of time prior to the failback.

	The '-w' argument specifies the amount of time, in seconds, that
	the 'p4 failback' command waits for existing commands running on the
	master server to complete before stalling any remaining commands and
	then continuing with the failback. By default, the command waits
	for 60 seconds.

	The '-y' flag actually executes the failback. By default, the
	'p4 failback' command only checks that a failback might be possible.

	The <failback message> is returned when attempting to start a new
	command on the master server once the failback process has actually
	started. A default message is used if the <failback message> is not
	specified. The <failback message> must be the last argument specified
	in the 'p4 failback' command; no other arguments should be specified
	after the <failback message>. The <failback message> need not be
	enclosed in quotes.

	'p4 failback' requires 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpFailbackintro = { ErrorOf( ES_HELP, 273, E_INFO, EV_NONE, 0 ),
R"(
    Introduction to the Failback Process

	Failback is the process of restoring the original master to its
	role prior to failover. It is also possible to restore the
	original standby to its role prior to failover. This is a process
	that involves a few steps that are outlined here.

	The original 'p4 failover' command must be performed with
	master participation. Both the master and the standby servers
	must be running at server version 22.1 or above. Please refer
	to 'p4 help failover' for details regarding this command.

	In order to run 'p4 failback', the original master server prior
	to failover must be converted to a standby that will be the target
	of the later 'p4 failback'. This conversion is done via the
	'p4d -Fm' command:

	    p4d -r <p4root> -Fm [-y] <masterServerID> <standbyServerID>

	where <masterServerID> and <standbyServerID> refer to the server IDs
	of the master and standby prior to failover. The <p4root> location
	must be the P4ROOT location of the original master server. The '-y'
	option executes the command. Without '-y', this command runs in
	preview mode. The 'p4d -Fm' command prepares the server for later
	running 'p4 failback' by changing this server to become a
	restricted standby. The restricted status allows the standby to ignore
	configuration settings that would interfere with its operation as the
	target of the later 'p4 failback'.

	After this server is started and its replication has caught up
	with the master, the standby server spec can optionally be changed
	to make it a mandatory standby.

	The next step is to run 'p4 failback' from this restricted
	standby server. Please refer to 'p4 help failback' for details
	regarding this command.

	When the original master has been restored to master status
	via 'p4 failback', the original standby can then be restored
	to become the standby for the master. This procedure is done via
	the 'p4d -Fs' command:

	    p4d -r <p4root> -Fs [-y] <masterServerID> <standbyServerID>

	where <masterServerID> and <standbyServerID> refer to the server IDs
	of the master and standby prior to failover. The <p4root> location
	must be the P4ROOT location of the original standby server. The '-y'
	option executes the command. Without '-y', this command runs in
	preview mode. The 'p4d -Fs' command changes the server at that
	location to become a restricted standby. When this server discovers
	via replication that 'p4 failback' has been successfully run, it will
	function as an unrestricted standby for the restored master. After
	replication has caught up to the master, the server spec can
	optionally be changed to make it a mandatory standby.
)"
};

ErrorId MsgHelp::HelpFailover = { ErrorOf( ES_HELP, 255, E_INFO, EV_NONE, 0 ),
R"(
    failover -- Fail over to a standby server

    p4 failover [ -y ] [ -m | [ -i ] [ -s <server ID> ] ] [ -w <quiesce wait> ]
                [ -v <verification time> ] [ <failover message> ]

	Fail over to a standby server from the server that the standby
	server is journalcopy'ing. The standby server can be a server with
	either 'standby' or 'forwarding-standby' services. The server from
	which failover can occur is any server from which a standby server
	can journalcopy, such as a server with 'standard', 'commit-server',
	or 'edge-server' services. (In the context of failover, the server
	from which failover can occur is often referred to as the "master"
	server, though that server could be a server with 'edge-server'
	services.) The 'p4 failover' command must be run on a
	standby server.

	A significant factor in determining the steps executed by
	the 'p4 failover' command is whether or not the master server
	participates in the failover. The master server participates in the
	failover if it is accessible, unless the '-i' flag (ignore the master
	server) is specified. (If the master server is ignored yet still
	accessible, an undesirable "split brain scenario" could result;
	see the description of the '-i' flag below.) If the master server
	participates in the failover, the standby server coordinates with
	the master server to perform an orderly failover. This includes the
	orderly disabling of master server usage, waiting for the journalcopy
	of the remaining transactions from the master server to the standby
	server, and the standby server stopping the master server. Note that
	the master's server version must be at least 2018.2 to participate
	in its failover.

	If the master server does not participate in the failover, a check
	is made to ensure that the standby server to which failover is to
	occur has the "mandatory" option set. Without the participation
	of the master server, failing over to a mandatory standby server
	is required to ensure that the other replicas remain consistent
	with the new master server after failover. Consistency is assured
	since during production operations, metadata must be journalcopy'd by
	all mandatory standby servers before that metadata is replicated to
	the other replicas. (Since journalcopy performance of the mandatory
	standby servers can affect the production replication to the other
	replicas, deploying one or more mandatory standby servers local
	to the master server is recommended for high availability. On
	the other hand, setting the "mandatory" option on a standby server
	at a remote disaster recovery site is generally not recommended.)

	Monitoring (p4 monitor) must be enabled when the standby server
	was started prior to running the 'p4 failover' command. The standby
	server must be appropriately licensed for its new role following
	the failover.

	Following a successful failover, site-specific changes might
	be needed to use the new master server. For example, it might
	be necessary to make DNS changes so that users and replicas
	can connect to the new master server. And if any 'standby'
	or 'forwarding-standby' servers across the entire installation
	do not have a value for the 'ReplicatingFrom' field in their
	server spec, the 'statefailover' file in the 'P4ROOT' of the
	new master server should be deleted, but only after the standby
	servers journalcopy'ing from the new master server have made their
	initial journalcopy request after the failover. (The deletion
	of the 'statefailover' file will occur automatically if the
	'ReplicatingFrom' field is defined for all 'standby' and
	'forwarding-standby' servers across the entire installation.)
	The former master server cannot restart until its server ID
	is reset via 'p4d -xD', and will require additional configuration
	to make it a standby server.

	If the 'p4 failover' command is interrupted or an error is
	encountered during the failover, the command will attempt to undo,
	to the extent practical, what was done prior to the interruption or
	error. For example, if after starting to refuse new commands on the
	master server, the 'p4 failover' command is interrupted during the
	wait to allow existing commands on the master server to complete,
	then new commands will again be allowed on the master server. But
	if an error is encountered after the standby server has stopped
	the master server, the standby server will not restart the
	master server.

	The '-i' flag ignores the master server during failover. Only the
	accessibility of the master server is checked; there is no other
	participation of the master server in the failover. If the master
	server is ignored yet still accessible, failing over to a standby
	server could result in the undesirable configuration of two master
	servers, each processing divergent datasets. (This configuration
	is sometimes referred to as a "split brain scenario".) If the
	master server is still accessible, a message warning of the
	potentially undesirable configuration is reported.

	The '-m' flag requires that the master server must participate in
	the failover. If the master server cannot be accessed by the standby
	server, the failover will not occur.

	The '-s' argument specifies the master's server ID that is used if
	the master server cannot be accessed by the standby server.
	If '-s' is not specified for this scenario and this standby server
	is mandatory, the value of the ReplicatingFrom server spec field
	will be used for the master's server ID.

	The '-v' argument specifies the window of time, in seconds,
	during which classic depot file content that was updated prior to
	the failover will be verified to ensure that it will be correct on
	the new master server. By default, the 'p4 failover' command verifies
	the file content that was updated during the 300 seconds prior to the
	failover. The command does not wait for this amount of time; it only
	waits as long as it takes to verify the file content that was
	updated during this window of time prior to the failover.

	The '-w' argument specifies the amount of time, in seconds, that
	the 'p4 failover' command waits for existing commands running on the
	master server to complete before stalling any remaining commands and
	then continuing with the failover. By default, the command waits
	for 60 seconds.

	The '-y' flag actually executes the failover. By default, the
	'p4 failover' command only checks that a failover might be possible.

	The <failover message> is returned when attempting to start a new
	command on the master server once the failover process has actually
	started. A default message is used if the <failover message> is not
	specified. The <failover message> must be the last argument specified
	in the 'p4 failover' command; no other arguments should be specified
	after the <failover message>. The <failover message> need not be
	enclosed in quotes.

	'p4 failover' requires 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpFiles = { ErrorOf( ES_HELP, 43, E_INFO, EV_NONE, 0 ),
R"(
    files -- List files in the depot

    p4 files [ -a ] [ -A ] [ -e ] [-i] [ -m max ] file[revRange] ...
    p4 files [ -a ] [ -e ] [ -m max ] [ --streamviews ] file[revRange] ...
    p4 files -U unloadfile ...

	List details about specified files: depot file name, revision,
	file, type, change action and changelist number of the current
	head revision. If client syntax is used to specify the file
	argument, the client view mapping is used to determine the
	corresponding depot files.

	By default, the head revision is listed.  If the file argument
	specifies a revision, then all files at that revision are listed.
	If the file argument specifies a revision range, the highest revision
	in the range is used for each file. For details about specifying
	revisions, see 'p4 help revisions'.

	The -a flag displays all revisions within the specific range, rather
	than just the highest revision in the range.

	The -A flag displays files in archive depots.

	The -e flag displays files with an action of anything other than
	deleted, purged or archived.  Typically this revision is always
	available to sync or integrate from.

	The -i flag is used to ignore the case of the file argument when
	listing files in a case sensitive server.

	The -m flag limits files to the first 'max' number of files.

	The -U option displays files in the unload depot (see 'p4 help unload'
	for more information about the unload depot).

	The --streamviews option reports results for streams in the context
	of the stream.

	See 'p4 help-graph files' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpFilelog = { ErrorOf( ES_HELP, 44, E_INFO, EV_NONE, 0 ),
R"(
    filelog -- List revision history of files

    p4 filelog [-c changelist# -h -i -l -L -t -m max -p -s] file[revRange] ...

	List the revision history of the specified files, from the most
	recent revision to the first.  If the file specification includes
	a revision, the command lists revisions at or prior to the specified
	revision.  If the file specification includes a revision range,
	the command lists only the specified revisions.
	See 'p4 help revisions' for details.

	The -c changelist# flag displays files submitted at the specified
	changelist number.

	The -i flag includes inherited file history. If a file was created by
	branching (using 'p4 integrate'), filelog lists the revisions of the
	file's ancestors up to the branch points that led to the specified
	revision.  File history inherited by renaming (using 'p4 move') is
	always displayed regardless of whether -i is specified.

	The -h flag displays file content history instead of file name
	history.  The list includes revisions of other files that were
	branched or copied (using 'p4 integrate' and 'p4 resolve -at') to
	the specified revision.  Revisions that were replaced by copying
	or branching are omitted, even if they are part of the history of
	the specified revision.

	The -t flag displays the time as well as the date.

	The -l flag lists the full text of the changelist descriptions.

	The -L flag lists the full text of the changelist descriptions,
	truncated to 250 characters if longer.

	The -m max displays at most 'max' revisions per file of the file[rev]
	argument specified.

	The -p flag is used in conjunction with the '-h' flag to prevent
	filelog from following content of promoted task streams. This flag
	is useful when there are many child task streams branched from the
	file argument supplied.

	The -s flag displays a shortened form of filelog that omits
	non-contributory integrations.

	See 'p4 help-graph filelog' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpFix = { ErrorOf( ES_HELP, 45, E_INFO, EV_NONE, 0 ),
R"(
    fix -- Mark jobs as being fixed by the specified changelist

    p4 fix [-d] [-s status] -c changelist# jobName ...

	'p4 fix' marks each named job as being fixed by the changelist
	number specified with -c.  The changelist can be pending or
	submitted and the jobs can be open or closed (fixed by another
	changelist).

	If the changelist has already been submitted and the job is still
	open, then 'p4 fix' marks the job closed.  If the changelist has not
	been submitted and the job is still open, the job is closed when the
	changelist is submitted.  If the job is already closed, it remains
	closed.

	The -d flag deletes the specified fixes.  This operation does not
	otherwise affect the specified changelist or jobs.

	The -s flag uses the specified status instead of the default defined
	in the job specification.

	The fix's status is reported by 'p4 fixes', and is related to the
	job's status. Certain commands set the job's status to the fix's
	status for each job associated with the change. When a job is fixed
	by a submitted change, the job's status is set to match the fix
	status.  When a job is fixed by a pending change, the job's status
	is set to match the fix status when the change is submitted. If the
	fix's status is 'same', the job's status is left unchanged.
)"
};

ErrorId MsgHelp::HelpFixes = { ErrorOf( ES_HELP, 46, E_INFO, EV_NONE, 0 ),
R"(
    fixes -- List jobs with fixes and the changelists that fix them

    p4 fixes [-i -m max -c changelist# -j jobName] [file[revRange] ...]

	'p4 fixes' list fixed jobs and the number of the changelist that
	 contains the fix. Fixes are associated with changelists using the
	'p4 fix' command or by editing and submitting changelists.

	The 'p4 fixes' command lists both submitted and pending changelists.

	By default, 'p4 fixes' lists all fixes.  This list can be limited
	as follows: to list fixes for a specified job, use the -j jobName
	flag.  To list fixes for a specified changelist, use -c changelist#.
	To list fixes that affect specified files, include the file argument.
	The file pattern can include wildcards and revision specifiers. For
	details about revision specifiers, see 'p4 help revisions'

	The -i flag also includes any fixes made by changelists integrated
	into the specified files.

	The -m max flag limits output to the specified number of job
	fixes.
)"
};

ErrorId MsgHelp::HelpFstat = { ErrorOf( ES_HELP, 48, E_INFO, EV_NONE, 0 ),
R"(
    fstat -- Dump file info

    p4 fstat [-F filter -L -T fields -m max -r] [-c | -e changelist#]
	[-Ox -Rx -Sx] [-A pattern] [-U] file[rev] ...
    p4 fstat [-F filter -T fields -m max -r] [-c | -e changelist#]
	[-Ox -Rx -Sx] [-A pattern] [ --streamviews ] file[rev] ...

	fstat lists information about files, one line per field.  Fstat is
	intended for use in P4 API applications, where the output can be
	accessed as variables, but its output is also suitable for parsing
	from the client command output in scripts.

	The fields that fstat displays are:

		attr-<name>          -- attribute value for <name>
		attrProp-<name>      -- set if attribute <name> is propagating
		clientFile           -- local path (host or P4 syntax)
		depotFile            -- name in depot
		movedFile            -- name in depot of moved to/from file
		path                 -- local path (host syntax)
		isMapped             -- set if file is mapped in the client
		isMissing            -- set if the rev is known to not be in
		                        the depot
		shelved              -- set if file is shelved
		headAction           -- action at head rev, if in depot
		headChange           -- head rev changelist#, if in depot
		headRev              -- head rev #, if in depot
		headType             -- head rev type, if in depot
		headCharset          -- head charset, for unicode type
		headTime             -- head rev changelist time, if in depot
		headModTime	     -- head rev mod time, if in depot
		movedRev             -- head rev # of moved file
		haveRev              -- rev had on client, if on client
		desc                 -- change description (if -e specified)
		digest               -- MD5 digest (fingerprint)
		fileSize             -- file size
		otherOpen            -- set if someone else has it open
		otherOpen#           -- list of user@client with file opened
		otherLock            -- set if someone else has it locked
		otherLockGlobal      -- set if lock is global
		otherLockOnCommit    -- set if locked on commit server
		otherLock#           -- user@client with file locked
		otherLockGlobal#     -- user@client with file globally locked
		otherLockOnCommit#   -- user@client with lock on commit server
		otherAction#         -- open action, if opened by someone else
		otherChange#         -- changelist, if opened by someone else
		lockGlobal           -- set if globally locked
		lockOnCommit         -- set if locked on commit server
		openattr-<name>      -- attribute value for <name>
		openattrProp-<name>  -- set if attribute <name> is propagating
		totalFileCount       -- total no. of files, if sorted

	If the file is opened in the current workspace, additional fields
	that fstat displays are:

		action               -- open action, if opened
		actionOwner          -- user who opened file, if opened
		change               -- open changelist#, if opened
		charset              -- open charset, for unicode type
		ourLock              -- set if this user/client has it locked
		revtime              -- file modification time, if shelved
		type                 -- open type, if opened
		workRev              -- open revision, if opened

	If the opened file involves pending integrations, additional fields
	that fstat displays are:

		resolved             -- resolved integration records
		unresolved           -- unresolved integration records
		reresolvable         -- reresolvable integration records
		resolveAction#       -- pending integration record action
		resolveType#         -- pending integration type
		resolveBaseFile#     -- pending integration base file
		resolveBaseRev#      -- pending integration base rev
		resolveFromFile#     -- pending integration from file
		resolveStartFromRev# -- pending integration from start rev
		resolveEndFromRev#   -- pending integration from end rev

	The -A <pattern> flag restricts displayed attributes to those that
	match 'pattern'.

	The -F flag lists only files satisfying the filter expression. This
	filter syntax is similar to the one used for 'jobs -e jobview' and is
	used to evaluate the contents of the fields in the preceding list.
	Filtering is case-sensitive.

	        Example: -Ol -F "fileSize > 1000000 & headType=text"

	Note: filtering is not optimized with indexes for performance.

	The -L flag can be used with multiple file arguments that are in
	full depot syntax and include a valid revision number. When this
	flag is used the arguments are processed together by building an
	internal table similar to a label. This file list processing is
	significantly faster than having to call the internal query engine
	for each individual file argument. However, the file argument syntax
	is strict and the command will not run if an error is encountered.

	The -T fields flag returns only the specified fields. The field names
	can be specified using a comma- or space-delimited list.

	        Example: -Ol -T "depotFile, fileSize"

	The -m max flag limits output to the specified number of files.

	The -r flag sorts the output in reverse order.

	The -c changelist# flag displays files modified by the specified
	changelist or after that changelist was submitted.  This operation is
	much faster than using a revision range on the affected files.

	The -e changelist# flag lists files modified by the specified
	changelist. When used with the -Ro flag, only pending changes are
	considered, to ensure that files opened for add are included. This
	option also displays the change description.

	The -O options modify the output as follows:

	        -Oa     output attributes set by 'p4 attribute'

		-Ob	output the path, revision, type, full and relative
			local paths of the server archive file. Requires
			'admin' privilege.

	        -Od     output the digest of the attribute

	        -On     output attribute value storage locations rather than
	                the attribute values

	        -Oe     output attribute values encoded as hex

	        -Of     output all revisions for the given files (this
	                option suppresses other* and resolve* fields)

	        -Ol     output a fileSize and digest field for each revision
	                (this may be expensive to compute)

	        -Om     include other ditto mapped client files in output

	        -Op     output the local file path in both P4 syntax
	                (//client/) as 'clientFile' and host form as 'path'

	        -Or     output pending integration record information for
	                files opened on the current client, or if used with
	                '-e <change> -Rs', on the shelved change

	        -Os     exclude client-related data from output

	        -OL     when run against an edge server, output global lock
	                information from the commit server

	The -R option limits output to specific files:

	        -Rc     files mapped in the client view
	        -Rh     files synced to the client workspace
	        -Rn     files opened not at the head revision
	        -Ro     files opened
	        -Rr     files opened that have been resolved
	        -Rs     files shelved (requires -e)
	        -Ru     files opened that need resolving

	The -S option changes the order of output:

	        -Sd     sort by date
	        -Sh     sort by have revision
	        -Sr     sort by head revision
	        -Ss     sort by filesize
	        -St     sort by filetype

	Note that sorting is applied to all potential results matching
	the file argument(s), and the totalFileCount field indicates
	the number of results before the -m option is applied.

	The -U flag displays information about unload files in the unload
	depot (see 'p4 help unload').

	For compatibility, the following flags are also supported:
	-C (-Rc) -H (-Rh) -W (-Ro) -P (-Op) -l (-Ol) -s (-Os).

	The --streamviews option reports results for streams in the context
	of the stream.

	See 'p4 help-graph fstat' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpGrantPermission = { ErrorOf( ES_HELP, 197, E_INFO, EV_NONE, 0 ),
R"(
    grant-permission -- grant access permission to a repo/graph depot

    p4 grant-permission [-f] -n //repo/name -g group [-r ref] -p permission
    p4 grant-permission [-f] -n //repo/name -u user  [-r ref] -p permission
    p4 grant-permission [-f] -d graph-depot -g group [-r ref] -p permission
    p4 grant-permission [-f] -d graph-depot -u user  [-r ref] -p permission

	'p4 grant-permission' is used by an administrator or a super user
	to grant permissions on a specified repo or a collection of repos
	managed under one graph depot.  An administrator is the owner of
	the graph depot, the owner of the repo, or a user that has been
	added as an administrator for the repo or graph depot specified.

	Since permissions can be granted either to a single repo or to all
	repos in one graph depot, specify either -d or -n, but not both.

	The -f flag grants permission for repo, user, group which do not exist.

	The -d graph-depot flag grants the permission for all of the repos
	managed under the graph depot specified.  Specifying "*" will grant the
	permission for all repos in all graph depots.

	The -n //repo/name flag grants the permission for the repo specified.

	The -u user flag grants permission to the user specified.

	The -g group flag grants permission to the group specified.

	The -r ref flag specifies an optional reference (ref) for the
	permission (branch or tag).

	The -p permission flag grants the permission specified.

	  Valid permissions:

	  read           - the user/group can read the repo(s) specified
	                   by the '-d' or '-n' option.

	  write-ref      - the user/group can update the ref, (branch or tag)
	                   specified by '-r'.  Modified files are checked
	                   against the protection table and for lfs locks.
	                   Also grants 'read' permission.

	  write-all      - the user/group can update the ref, (branch or tag)
	                   and all files specified by '-r'.  Modified files
	                   are not checked against the protection table nor
	                   checked for lfs locks.  Also grants 'read'
	                   and 'write-ref' permission.

	  restricted-ref - the user/group can update the ref, (branch or tag)
	                   specified by '-r'. If set, only users with this
	                   permission can perform an update. Prevents other
	                   users with write-ref/write-all from updating the
	                   specified reference.

	  create-ref     - the user/group can create the ref, (branch or tag)
	                   specified by '-r'. Does not include write-ref
	                   permissions.

	  delete-ref     - the user/group can delete the ref, (branch or tag)
	                   specified by '-r'.

	  force-push     - the user/group can force push (overwrite) work to
	                   the reference specified by '-r'. Also grants 'read',
	                   'write-ref', 'write-all', 'create-ref' and
	                   'delete-ref' permission.

	  create-repo    - the user/group can create a repo that is managed by
	                   the graph depot specified.

	  delete-repo    - the user/group can delete a repo that is managed by
	                   the graph depot specified.

	  admin          - the user/group specified can grant and revoke
	                   permissions for the repo/depot specified. An
	                   admin user has all permissions enabled.


)"
};

ErrorId MsgHelp::HelpGrep = { ErrorOf( ES_HELP, 122, E_INFO, EV_NONE, 0 ),
R"(
    grep -- Print lines matching a pattern

    p4 grep [options] -e pattern file[revRange] ...

	options: -a -i -n -A <num> -B <num> -C <num> -t -s (-v|-l|-L) (-F|-G)

	Searches files for lines that match the specified regular expression,
	which can contain wildcards.  The parser used by the P4 server is based
	on V8 regexp and might not be compatible with later parsers, but the
	majority of functionality is available.

	By default the head revision is searched.  If the file argument
	includes a revision specification, all corresponding revisions are
	searched.  If the file argument includes a revision range, only files
	in that range are listed, and the highest revision in the range is
	searched.  For details about revision specifiers, see
	'p4 help revisions'.

	The -a flag searches all revisions within the specified range. By
	default only the highest revision in the range is searched.

	The -i flag causes the pattern matching to be case-insensitive. By
	default, matching is case-sensitive.

	The -n flag displays the matching line number after the file revision
	number. By default, matches are displayed as revision#: <text>.

	The -v flag displays files with non-matching lines.

	The -F flag is used to interpret the pattern as a fixed string.

	The -G flag is used to interpret the pattern as a regular expression,
	which is the default behavior.

	The -L flag displays the name of each selected file from which no
	output would normally have been displayed. Scanning stops on the
	first match.

	The -l flag displays the name of each selected file containing
	matching text. Scanning stops on the first match.

	The -s flag suppresses error messages that result from abandoning
	files that have a maximum number of characters in a single line that
	are greater than 4096.  By default, an error is reported when grep
	abandons such files.

	The -t flag searches binary files.  By default, only text files are
	searched.

	The -A <num> flag displays the specified number of lines of trailing
	context after matching lines.

	The -B <num> flag displays the specified number of lines of leading
	context before matching lines.

	The -C <num> flag displays the specified number of lines of output
	context.

	Regular expressions:

	A regular expression is zero or more branches, separated by `|'. It
	matches anything that matches one of the branches.

	A branch is zero or more pieces, concatenated.  It matches a match
	for the first, followed by a match for the second, etc.

	A piece is an atom possibly followed by `*', `+', or `?'.  An atom
	followed by `*' matches a sequence of 0 or more matches of the atom.
	An atom followed by `+' matches a sequence of 1 or more matches of
	the atom.  An atom followed by `?' matches a match of the atom, or
	the null string.

	An atom is a regular expression in parentheses (matching a match for
	the regular expression),  a range (see below),  `.'  (matching any
	single character),  `^' (matching the null string at the beginning
	of the input string),  `$' (matching the null string at the end of
	the input string),  a `\' followed by a single character (matching
	that character),  or a single character with no other significance
	(matching that character).

	A range is a sequence of characters enclosed in `[]'.  It normally
	matches any single character from the sequence.  If the sequence
	begins with `^',  it matches any single character not from the rest
	of the sequence.  If two characters in the sequence are separated by
	`-', this is shorthand for the full list of ASCII characters between
	them (e.g. `[0-9]' matches any decimal digit).  To include a literal
	`]' in the sequence, make it the first character (following a possible
	`^').  To include a literal `-', make it the first or last character.

	The symbols '\<' and '\>' respectively match the empty string at
	the beginning and end of a word.
)"
};

ErrorId MsgHelp::HelpGroup = { ErrorOf( ES_HELP, 49, E_INFO, EV_NONE, 0 ),
R"(
    group -- Change members of user group

    p4 group [-a|-A] name
    p4 group -d [-a | -F] name
    p4 group -o name
    p4 group -i [-a|-A]

	Create a group or modify the membership of an existing group.
	A group can contain users and other groups. The group specification
	is put into a temporary file and the editor (configured by the
	environment variable $P4EDITOR) is invoked.

	A group exists when it has any users or other groups in it, and
	ceases to exist if all users and groups in it are removed.

	Each group has MaxResults, MaxScanRows, MaxLockTime, MaxOpenFiles
	and MaxMemory fields, which limit the resources committed to
	operations performed by members of the group.  See
	'p4 help maxresults' for more information on MaxResults, MaxScanRows,
	MaxLockTime, MaxOpenFiles and MaxMemory.

	Since a user may be a member of multiple groups, the effective
	resource limits for that user take into account the resource limits
	specified by each group to which that user belongs, except for those
	groups where the limit is specified as 'unset'.

	After examining all relevant groups, an individual user's limit is
	the highest of any group with a limit to which they belong, unlimited
	if any of their groups has 'unlimited' for that field, or unlimited
	if they belong to no group with a limit.

	Each group also has a Timeout field, which specifies how long (in
	seconds) a 'p4 login' ticket remains valid.  A value of 'unset' or
	'unlimited' is equivalent to no timeout. An individual's timeout is
	the highest of any group with a limit to which they belong, unlimited
	if any of their groups has 'unlimited' for the timeout value, or
	unlimited if they belong to no group with a limit. See 'p4 help login'
	for more information.

	Each group has a IdleTimeout field, which specifies how long (in
	seconds) after the last run command a 'p4 login' ticket should be
	invalidated. A value of 'unset' or 'unlimited' disables the timeout.
	An individual's timeout is the highest of any group with a limit to
	which they belong, unlimited if any of their groups has 'unlimited'
	for the timeout value, or unlimited if they belong to no group with a
	limit. Since the idle time is based on the user's access time, if an
	IdleTimeout is set of a user, but the value is lower than that of
	'dm.user.accessupdate' or 'dm.user.accessforce' the highest value is
	used.

	Each group has a PasswordTimeout field, which determines how long a
	password remains valid for members of the group.

	A group may be synchronized with an LDAP group by setting the three
	fields: LdapConfig, LdapSearchQuery and LdapUserAttribute. This takes
	the LDAP configuration (see 'p4 ldap') specified by LdapConfig and uses
	it to execute the query stored by LdapSearchQuery. The LDAP attribute
	specified by LdapUserAttribute is taken to be user's username and is
	added to the group's user list.  If the group contains references its
	members by their DNs rather than by username, you may instruct the DN
	to be followed by setting the LdapUserDNAttribute field to the name of
	the group object's attribute that the DNs are stored in: the username
	will be found by looking for the LdapUserAttribute attribute in the
	object referred to by the DN.  At least one group owner must be set
	if these LDAP fields are used.  If the LDAP server requires login for
	read-only queries, then the LDAP configuration must contain valid bind
	credentials in the LDAP spec's SearchBindDN and SearchPasswd fields

	The -d flag deletes a group.

	The -o flag writes the group specification to standard output. The
	user's editor is not invoked.

	The -i flag reads a group specification from standard input. The
	user's editor is not invoked.  The new group specification replaces
	the previous one.

	The -a flag enables a user without 'super' access to modify the group
	if that user is an 'owner' of that group. Group owners	are specified
	in the 'Owners' field of the group spec.

	The -A flag enables a user with 'admin' access to add a new group.
	Existing groups may not be modified when this flag is used.

	The -F flag, used only with -d, forces the deletion of the
	specified group, and also removes the group from the protections
	table and from all groups.

	All commands that require access granted by 'p4 protect' consider a
	user's groups when calculating access levels.

	'p4 group' requires 'super' access granted by 'p4 protect' unless
	invoked with the '-a' or '-A' flag by a qualified user.
)"
};

ErrorId MsgHelp::HelpGroups = { ErrorOf( ES_HELP, 50, E_INFO, EV_NONE, 0 ),
R"(
    groups -- List groups (of users)

    p4 groups [-m max] [-v] [group]
    p4 groups [-m max] [-i [-v]] user | group
    p4 groups [-m max] [-g | -u | -o] name

	The first form lists all user groups defined in the server, or just
	the groups for which the specified group is a subgroup.

	The second form displays subgroup relationships. If a user argument is
	specified, only groups containing that user are displayed. If a group
	argument is specified, only groups containing the group are displayed.

	The third form is useful when there are groups and users with the
	same name, or when requesting all groups owned by a certain user.

	The -i flag also displays groups that the user or group belongs to
	indirectly by means of membership in subgroups.

	The -m max flag limits output to the specified number of groups.

	The -v flag displays the MaxResults, MaxScanRows, MaxLockTime,
	MaxOpenFiles, MaxMemory, Timeout, and PasswordTimeout values
	for each group that is displayed.

	The -g flag indicates that the 'name' argument is a group.

	The -u flag indicates that the 'name' argument is a user.

	The -o flag indicates that the 'name' argument is an owner.
)"
};

ErrorId MsgHelp::HelpHave = { ErrorOf( ES_HELP, 51, E_INFO, EV_NONE, 0 ),
R"(
    have -- List the revisions most recently synced to the current workspace

    p4 have [--graph-only] [file ...]

	List revision numbers of the currently-synced files. If file name is
	omitted, list all files synced to this client workspace.

	The format is:  depot-file#revision - client-file

	For hybrid workspaces that have both classic and graph depots, the
	--graph-only flag limits the report to graph depots only. Without
	it 'p4 have' will report on both graph and classic depots.

	See 'p4 help-graph have' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpHeartbeat = { ErrorOf( ES_HELP, 264, E_INFO, EV_NONE, 0 ),
R"(
    heartbeat -- monitor responsiveness of a target server

    p4 heartbeat [-t target] [-i interval] [-w wait] [-m missingInterval]
	[-r missingWait] [-c count ]

	Heartbeat sends requests to a target server and monitors the
	responses to determine the responsiveness of the target. Based on the
	responses, it may fire triggers to alert admins about a change
	in the target server's status. The first missed response will
	fire 'heartbeat-missing' triggers and/or Extensions. The first resumed
	response after missing responses will fire 'heartbeat-resumed' triggers
	and/or Extensions. If the number of consecutive missing responses
	reaches the maximum count, the 'heartbeat-dead' trigggers and/or
	Extensions will be fired and the command will terminate.

	The -t option specifies the target server to be monitored. If
	not specified, the command will use the value of P4TARGET. If
	neither P4TARGET nor the -t option is set, the command will fail.

	The -i option specifies the time in milliseconds to wait before
	sending another heartbeat request. If not specified, this
	interval defaults to the net.heartbeat.interval configurable.

	The -w option specifies the time in milliseconds to wait before
	the response times out. If not specified, this defaults to the
	net.heartbeat.wait configurable.

	The -m option specifies the time in milliseconds to wait before
	sending another heartbeat request after the previous response
	was missing. If not specified, this defaults to the
	net.heartbeat.missing.interval configurable.

	The -r option specifies the time in milliseconds to wait before
	the response times out after the previous response was missing.
	If not specified, this defaults to the net.heartbeat.missing.wait
	configurable.

	The -c option specifies the maximum number of consecutive missed
	heartbeats before the heartbeat is declared dead. If not specified,
	this defaults to the net.heartbeat.missing.count configurable.

	Note that 'p4 heartbeat' can increase network traffic between
	the servers.

	The 'heartbeat' command requires super or operator privileges.
	The target's server version must be at least 2020.1.
)"
};

ErrorId MsgHelp::HelpHelp = { ErrorOf( ES_HELP, 52, E_INFO, EV_NONE, 0 ),
R"(
    help -- Print help message

    p4 help [command ...]

	Print usage details about the specified command.  If the command
	name is omitted, print a general help message about P4 and list the
	available client commands.
)"
};

ErrorId MsgHelp::HelpHotFiles = { ErrorOf( ES_HELP, 279, E_INFO, EV_NONE, 0 ),
R"(
    hotfiles -- Edit the hotfiles mapping table

    p4 hotfiles
    p4 hotfiles -o
    p4 hotfiles -i

	'p4 hotfiles' edits a path-to-rule mapping table for altsync clients.

	Commands that could create placeholders with an altsync agent will
	check this mapping to decide if a placeholder is created or if the file
	is considered hot and should be immediately transferred to the client.

	The altsync agent may specify additional mappings that could override
	these global mappings.

	The hotfiles form has a single field, 'HotFiles', followed by any
	number of hotfile lines. Each hotfile line contains a depot file path
	pattern followed by an optional rule, the rule can either be a filetype
	and/or size:

	Path:       The mapping is a file pattern in depot syntax.
		    To match all files anywhere in the depot hierarchy,
		    the pattern must begin with '//...'.  To match files
		    with a specified suffix, use '//.../*.suffix' or
		    use '//....suffix' (four dots).
		    Exclusionary mapping is supported '-//...' and can
		    also be used to override previous entries.

	Filetype:   See 'p4 help filetypes' for a list of valid filetypes.

	Size:       '<' or '>' followed by numeric value in bytes, followed
		    by an optional unit specifier K/M/G/T.

	Later entries override earlier entries. If no matching entry is found
	in the table or if an exclusionary entry is encountered, the file is
	not considered hot.

	The -o flag writes the hotfiles table to standard output. The user's
	editor is not invoked.

	The -i flag reads the hotfiles table from standard input. The user's
	editor is not invoked.

	'p4 hotfiles' requires 'admin' access, which is granted by
	'p4 protect'.
)"
};

ErrorId MsgHelp::HelpIgnores = { ErrorOf( ES_HELP, 189, E_INFO, EV_NONE, 0 ),
R"(
    ignores -- List P4IGNORE mappings

    p4 ignores [-v] [path ...]
    p4 ignores [-v] -i path ...

	List the computed mappings from the rules found in P4IGNORE files
	at the specified path. The path(s) must be in local syntax. If no
	paths are specified, P4IGNORE rules from the current directory are
	reported.

	The -v flag displays additional information about the origin of the
	mapping. This includes the original rule, which file that was loaded
	from and the line number in that file.

	The -i flag allows you to test whether the path specified would be
	ignored by the rules in the P4IGNORE files. The target is listed if
	path matches one of the mappings. If the -v flag is also provided,
	the target will be reported with a message explaining if it will be
	ignored and if it matched a mapping, which P4IGNORE rule affected it.
)"
};

ErrorId MsgHelp::HelpIndex = { ErrorOf( ES_HELP, 96, E_INFO, EV_NONE, 0 ),
R"(
    index -- Add words to the jobs index (unsupported)

    p4 index [-a attrib] [-d] name

	Index is an interface to the search engine used to search jobs.
	It reads its standard input for text, breaks the text into words,
	and adds those words to the search engine's index with the
	key 'name' and the optional attribute 'attrib'.  (Job fields
	are entered with the name of the job as the name and the field
	name as the attribute.)

	If -a attrib is specified, that attribute number is used instead of 0.

	The -d option deletes entries from the index.

	See also 'p4 help search'.
)"
};

ErrorId MsgHelp::HelpInfo = { ErrorOf( ES_HELP, 53, E_INFO, EV_NONE, 0 ),
R"(
    info -- Display client/server information

    p4 info [-s]

	Info lists information about the current client (user name,
	client name, applicable client root, client current directory,
	and the client IP address) and some server information (server
	IP address, server root, date, uptime, version and license data).

	The -s option produces 'short' output that omits any information
	that requires a database lookup such as the client root).
)"
};

ErrorId MsgHelp::HelpInteg = { ErrorOf( ES_HELP, 54, E_INFO, EV_NONE, 0 ),
R"(
    integrate -- Integrate one set of files into another set of files, and/or
	integrate one stream spec into another stream spec

    p4 integrate [options] fromFile[revRange] toFile
    p4 integrate [options] -b branch [-r] [toFile[revRange] ...]
    p4 integrate [options] -b branch -s fromFile[revRange] [toFile ...]
    p4 integrate [options] [-As | -Af] -S stream [-r] [-P parent]
	                         [file[revRange] ...]

	options: -c changelist# -Di -f -F -h -K -O<flags> -n -m max
	         -R<flags> -q -v

	'p4 integrate' integrates one set of files (the 'source') into
	another (the 'target').

	(See also 'p4 merge' and 'p4 copy', variants of 'p4 integrate' that
	may be easier and more effective for the task at hand.)

	Using the client workspace as a staging area, 'p4 integrate' adds and
	deletes target files per changes in the source, and schedules all
	other affected target files to be resolved.  Target files outside of
	the current client view are not affected. Source files need not be
	within the client view.

	'p4 resolve' must be used to merge file content, and to resolve
	filename and filetype changes. 'p4 submit' commits integrated files
	to the depot.  Unresolved files may not be submitted.  Integrations
	can be shelved with 'p4 shelve' and abandoned with 'p4 revert'.  The
	commands 'p4 integrated' and 'p4 filelog' display integration history.

	When 'p4 integrate' schedules a workspace file to be resolved, it
	leaves it read-only. 'p4 resolve' can operate on a read-only file.
	For other pre-submit changes, 'p4 edit' must be used to make the
	file writable.

	Source and target files can be specified either on the 'p4 integrate'
	command line or through a branch view. On the command line, fromFile
	is the source file set and toFile is the target file set.  With a
	branch view, one or more toFile arguments can be given to limit the
	scope of the target file set.

	revRange is a revision or a revision range that limits the span of
	source history to be probed for unintegrated revisions.  revRange
	can be used on fromFile, or on toFile, but not on both.  When used on
	toFile, it refers to source revisions, not to target revisions.  For
	details about revision specifiers, see 'p4 help revisions'.

	The -S flag makes 'p4 integrate' use a generated branch view that maps
	a stream (or its underlying real stream) to its parent.  With -r, the
	direction of the mapping is reversed.  -P can be used to generate the
	branch view using a parent stream other than the stream's actual
	parent.  Note that to submit integrated stream files, the current
	client must be switched to the target stream, or to a virtual child
	stream of the target stream.  By default, integration between streams
	requires the use of a stream view.

	Stream specifications may also be integrated along with stream files.
	Any field in the source specification which has an Openable property of
	'propagate' will be integrated to the target specification. The -Af and
	-As flags control whether stream files or the stream spec is
	integrated.

		-Af	Perform integration with files only.
		-As	Perform integration with stream specs only.

	Both stream files and the stream spec will be integrated if neither -Af
	nor -As is specified.  See 'p4 help streamspecinteg' for more details.

	The -b flag makes 'p4 integrate' use a user-defined branch view.
	(See 'p4 help branch'.) The source is the left side of the branch view
	and the target is the right side. With -r, the direction is reversed.

	The -s flag can be used with -b to cause fromFile to be treated as
	the source, and both sides of the branch view to be treated as the
	target, per the branch view mapping.  Optional toFile arguments may
	be given to further restrict the scope of the target file set.  The
	-r flag is ignored when -s is used.

	Note that 'p4 integrate' automatically adusts source-to-target
	mappings for moved and renamed files.  (Adjustment occurs only if
	the 'p4 move' command was used to move/rename files.) The scope of
	source and target file sets must include both the old-named and the
	new-named files for mappings to be adjusted.  A filename resolve is
	scheduled for each remapped file to allow the target to be moved to
	match the source.

	The -f flag forces integrate to ignore integration history and treat
	all source revisions as unintegrated. It is meant to be used with
	revRange to force reintegration of specific, previously integrated
	revisions.

	The -O flags cause more information to be output for each file opened:

		-Ob	Show the base revision for the merge (if any).
		-Or	Show the resolve(s) that are being scheduled.

	The -R flags modify the way resolves are scheduled:

		-Rb	Schedules 'branch resolves' instead of branching new
			target files automatically.

		-Rd	Schedules 'delete resolves' instead of deleting
			target files automatically.

		-Rs	Skips cherry-picked revisions already integrated.
			This can improve merge results, but can also cause
			multiple resolves per file to be scheduled.

	The -Di flag modifies the way deleted revisions are treated.  If the
	source file has been deleted and re-added, revisions that precede
	the deletion will be considered to be part of the same source file.
	By default, re-added files are considered to be unrelated to the
	files of the same name that preceded them. When the source file has
	been moved or renamed, the move/add and move/delete revisions are
	propagated as branch and delete revisions.

	The -F flag permits integration between streams with an arbitrarily
	specified view.  By default, a stream view must be used to integrate
	between two streams.

	The -h flag leaves the target files at the revision currently synced
	to the client (the '#have' revision). By default, target files are
	automatically synced to the head revision by 'p4 integrate'.

	The -K flag suppresses keyword expansion when updating ktext files on
	the client.

	The -m flag limits integration to the first 'max' number of files.

	The -n flag displays a preview of integration, without actually
	doing anything.

	The -q flag suppresses normal output messages. Messages regarding
	errors or exceptional conditions are displayed.

	If -c changelist# is specified, the files are opened in the
	designated numbered pending changelist instead of the 'default'
	changelist.

	The -v flag causes a 'virtual' integration that does not modify
	client workspace files unless target files need to be resolved.
	After submitting a virtual integration, 'p4 sync' can be used to
	update the workspace.
)"
};

ErrorId MsgHelp::HelpInteg3 = { ErrorOf( ES_HELP, 160, E_INFO, EV_NONE, 0 ),
R"(
	See 'p4 help integrate'.
)"
};

ErrorId MsgHelp::HelpInteged = { ErrorOf( ES_HELP, 55, E_INFO, EV_NONE, 0 ),
R"(
    integrated -- List integrations that have been submitted

    p4 integrated [-b branch [-r]] [-s change | -m max ]
	          [--into-only] [file ...]

        The p4 integrated command lists integrations that have been submitted.
        To list unresolved integrations, use 'p4 resolve -n'.  To list
        resolved but unsubmitted integrations, use 'p4 resolved'.

        If the -b branch flag is specified, only files integrated from the
        source to target files in the branch view are listed.  Qualified
        files are listed, even if they were integrated without using the
        branch view.

        The -r flag reverses the mappings in the branch view, swapping the
        target files and source files.  The -b branch flag is required.

        The -s flag takes a change number and shows integrations from that
        change forward. This option cannot be used with -m.

        The -m flag limits integrations to the 'max' most recent. The
        output is sorted by descending change number. This option cannot
        be used with -s.

        The --into-only flag shows only integrations from this path into
        other paths, not integrations from other paths into this one,
        or changes ignored by the target path.
)"
};

ErrorId MsgHelp::HelpIstat = { ErrorOf( ES_HELP, 131, E_INFO, EV_NONE, 0 ),
R"(
    istat -- Show/cache a stream's integration status

    p4 istat [-As | -Af] [-a | -r] [-c | -C] [ -s ] stream

	'p4 istat' shows a stream's cached integration status with respect
	to its parent. If the cache is stale, either because newer changes
	have been submitted or the stream's branch view has changed, 'p4
	istat' checks for pending integrations and updates the cache before
	showing status.

	Pending integrations are shown only if they are expected by the
	stream; that is, only if they are warranted by the stream's type
	and its fromParent/toParent flow options. (See 'p4 help stream'.)

	The -r flag shows the status of integration to the stream from its
	parent. By default, status of integration in the other direction is
	shown, from the stream to its parent.

	The -a flag shows status of integration in both directions.

	The -c flag forces 'p4 istat' to assume the cache is stale; it
	causes a search for pending integrations.  Use of this flag can
	impact server performance. The -C flag performs the same actions,
	and additionally clears the stream's record of the highest merged
	changelist.

	The -s flag shows cached state without refreshing stale data.

	'p4 istat' will report the combined integration status for both stream
	files and the stream spec of the specified stream.

	Use -Af or -As flag to limit the report of the status of files only or
	the stream spec only:

		-Af	Report integration status for stream files only.
		-As	Report integration status for the stream spec only.

	If the cache is cleared, the integration status for both files and
	stream spec are discovered, even if the requested report is only for
	files or only for the stream spec.
	
	See 'p4 help streamspecinteg' for more details.

)"
};

ErrorId MsgHelp::HelpJob = { ErrorOf( ES_HELP, 56, E_INFO, EV_NONE, 0 ),
R"(
    job -- Create or edit a job (defect) specification

    p4 job [-f] [jobName]
    p4 job -d jobName
    p4 job -o [jobName]
    p4 job -i [-f]

	The 'p4 job' command creates and edits job specifications using an
	ASCII form. A job is a defect, enhancement, or other unit of
	intended work.The 'p4 fix' command associates changelists with jobs.

	With no arguments, 'p4 job' creates an empty job specification
	and invokes the user's editor.  When the specification is saved,
	a job name of the form jobNNNNNN is assigned.  If the jobName
	parameter is specified on the command line, the job is created or
	opened for editing.

	As jobs are entered or updated, all fields are indexed for searching
	Text fields are broken into individual alphanumeric words (punctuation
	and whitespace are ignored) and each word is case-folded and entered
	into the word index.  Date fields are converted to an internal
	representation (seconds since 1970/01/01 00:00:00) and entered
	into the date index.

	The fields that compose a job are defined by the 'p4 jobspec' command.
	P4 provides a default job specification that you can edit.

	The -d flag deletes the specified job. You cannot delete a job if
	it has pending or submitted fixes associated with it.

	The -o flag writes the job specification to the standard output.
	The user's editor is not invoked.

	The -i flag reads a job specification from the standard input. The
	user's editor is not invoked.

	The -f flag enables you set fields that are read-only by default.
	The -f flag requires 'admin' access, which is granted using the
	'p4 protect' command.
)"
};

ErrorId MsgHelp::HelpKey = { ErrorOf( ES_HELP, 157, E_INFO, EV_NONE, 0 ),
R"(
     key -- Display, set, or delete a key/value pair

     p4 key name
     p4 key [-v] name value
     p4 key [-d] name
     p4 key [-i -v] name
     p4 key -m [ pair list ]
     p4 key --from=oldValue --to=newValue name

	The first form displays the value of the specified key.

	The second form sets the key to the specified value.

	The third form deletes the key.

	The -i flag increments a keys value by 1 and returns the new value.
	This option is used instead of a value argument and can only be
	used with numeric key values.

	The -v flag displays the previous value of the specified key
	after the key has been set or incremented.

	The -m flag in the fifth form allows multiple operations in one
	command. With this form, the list is pairs of arguments.  Each pair
	is either key value or '-' key.  To set a key use a key
	and value. To delete a key use a '-' followed by the key.

	The sixth form sets a key to the newValue only if the current
	value of the key is oldValue. Note that a key that has never
	been set (or has been deleted) cannot be set using this form.

	'p4 key' requires 'review' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpKeys = { ErrorOf( ES_HELP, 158, E_INFO, EV_NONE, 0 ),
R"(
    keys -- Display list of known key/values

    p4 keys [-e nameFilter -m max]

	The -e nameFilter flag lists keys with a name that matches
	the nameFilter pattern, for example: -e 'mykey-*'.

	The -m max flag limits the output to the first 'max' keys.
)"
};

ErrorId MsgHelp::HelpInterchanges = { ErrorOf( ES_HELP, 97, E_INFO, EV_NONE, 0 ),
R"(
    interchanges -- Report changes not yet integrated

    p4 interchanges [options] fromFile[revRange] toFile
    p4 interchanges [options] -b branch [toFile[revRange] ...]
    p4 interchanges [options] -b branch -s fromFile[revRange] [toFile ...]
    p4 interchanges [options] -S stream [-P parent] [file[revRange] ...]

	options: -f -l -r -t -u -F

	'p4 interchanges' lists changes that have not been integrated from
	a set of source files to a set of target files.

	The -f option lists the individual files that require integration.
	For partially integrated changelists, files might be listed even if
	they have been integrated individually.

	The -l flag includes the full text of the changelist descriptions.

	The -t flag displays the time as well as the date.

	The -b flag makes 'p4 interchanges' use a user-defined branch view.
	(See 'p4 help branch'.) The source is the left side of the branch
	view and the target is the right side.  Used with the -r flag, the
	direction is reversed.

	The -s flag can be used with -b to cause fromFile to be treated as
	the source, and both sides of the branch view to be treated as the
	target, per the branch view mapping.  Optional toFile arguments may
	be given to further restrict the scope of the target file set.  The
	-r flag is ignored when -s is used.

	The -u flag limits results to those submitted by a particular user.

	The -S flag causes 'p4 interchanges' to use a generated branch view
	that maps a stream to its parent.  With -r, the direction of the
	mapping is reversed.  -P can be used to generate the branch view
	using a parent stream other than the stream's actual parent.  The -S
	flag also makes 'p4 interchanges' respect a stream's flow.

	The -F flag can be used with -S to force 'p4 interchanges' to ignore
	a stream's expected flow.  It can also force it to generate a branch
	view based on a virtual stream; the mapping itself refers to the
	underlying real stream.

)"
};

ErrorId MsgHelp::HelpJobs = { ErrorOf( ES_HELP, 57, E_INFO, EV_NONE, 0 ),
R"(
    jobs -- Display list of jobs

    p4 jobs [-e jobview -i -l -m max -r] [file[revRange] ...]
    p4 jobs -R

	Lists jobs in the server. If a file specification is included, fixes
	for submitted changelists affecting the specified files are listed.
	The file specification can include wildcards and a revision range.
	 See 'p4 help revisions' for details about specifying revisions.

	The -e flag lists jobs matching the expression specified in the
	jobview parameter. For a description of jobview syntax, see 'p4 help
	jobview'.

	The -i flag includes any fixes made by changelists integrated into
	the specified files.

	The -l flag produces long output with the full text of the job
	descriptions.

	The -m max flag limits the output to the first 'max' jobs, ordered
	by their job name.

	The -r flag sorts the jobs in reverse order (by job name).

	The -R flag rebuilds the jobs table and reindexes each job, which
	is necessary after upgrading to 98.2.  'p4 jobs -R' requires that the
	user be an operator or have 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpJobSpec = { ErrorOf( ES_HELP, 58, E_INFO, EV_NONE, 0 ),
R"(
    jobspec -- Edit the job template

    p4 jobspec
    p4 jobspec -o
    p4 jobspec -i

	Jobspec edits the template that specifies the format of jobs.
	This format is used by 'p4 job' when jobs are entered or updated,
	and by 'p4 jobs' and 'p4 describe' when jobs are displayed.

	Jobspec brings up a form with the following fields:

	   Fields:  A list of the fields maintained for each job, one
		    line per field.  Each line has five words: code, name,
		    data-type, len, and field-type.

		    'code' is a unique integer identifier for storing
			the data of the field. When adding a new field,
			one may either enter a numeric field code value
			in the 101-199 range, or as 'NNN'. In the later
			case, the next available code value will be
			automatically generated. If the number of fields
			exhausts the range, new codes are assigned unique
			values greater than or equal to 10000.

		    'name' is the name of the field for the job.

		    'data-type' indicates the format of the field:

			word: a single word (any value)
			date: a date/time field
			select: one of a set of words
			line: a one-liner
			text: a block of text
			bulk: text not indexed for 'p4 jobs -e'

		    'len' is the recommended character length of a
			display box for the field.  If 0, a text box is
			assumed.

		    'field-type' indicates how to handle the setting of
			the field:

			optional: no default, and not required to be present
			default: default provided, still not required
			required: default provided, value must be present
			once: set once to the default and never changed
			always: always set to the default when saving the
				form, adding or deleting fixes with 'p4 fix'
				or 'p4 changelist', or submitting a change
				associated with the job with 'p4 submit'.

	   Values:  A list of 'select' fields and the values that the fields
		    can have.  Each line has two words: the field name and
		    the values list, with individual values separated by
		    '/' (no spaces).

	   Presets: A list of fields and their default values, for fields
		    whose 'setting' flag is other than 'optional'.  Each
		    line has two words: the field name and the default
		    value.  If the value has spaces, it must be enclosed
		    in double quotes.  The following special defaults are
		    recognized:

			$user: the user entering the job
			$now: the current date
			$blank: the words '<enter description here>'

		    The Preset for the job status field (code 102) has
		    a special syntax for providing a default fix status
		    for 'p4 fix' and 'p4 change' along with the default
		    status for new jobs:

		    	jobStatus,fix/fixStatus

		    Otherwise the fixStatus is hardwired to 'closed'.

	   Comments: text to be included at the top of each job specification,
		    to help the user fill out the form. Each line must begin
		    with the comment character '#'.

	The following field codes are reserved:

	    code 101, required: the job name
	    code 102, optional: the job status
	    code 103, optional: the user who created the job
	    code 104, optional: the date the job was created
	    code 105, optional: the description

	    If there is a job status field (102), 'p4 fix' and 'p4 change'
	    (of a submitted changelist) and 'p4 submit' (of a pending
	    changelist) will set the job's status to the fix's status for
	    each job associated with the change.  If the fix's status is
	    the special value 'same', the job's status is left unchanged.

	    Fields 102-105 are used by 'p4 describe' and 'p4 jobs' to display
	    a job summary.  Missing fields are omitted from the summary.

	    If field 105 is present, it is treated as a description,
	    which is used by 'p4 change' and 'p4 submit' to annotate the
	    list of jobs to be fixed by the changelist being created.

	If you change the jobspec after jobs have been created, note the
	following limitations:

	    Data is stored according to its code.  Fields can be renamed
	    by keeping the same code. Removing a code can cause the
	    associated data stored for the code to become abandoned.

	    Changing the definition of a code (for example, from 'text' to
	    'word') can require users to correct jobs for the new format
	    during editing.

	The -o flag writes the job template to the standard output.  The
	user's editor is not invoked.

	The -i flag reads a job template from the standard input.  The
	user's editor is not invoked.

	'p4 jobspec' requires 'admin' access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLabel = { ErrorOf( ES_HELP, 59, E_INFO, EV_NONE, 0 ),
R"(
    label -- Create or edit a label specification

    p4 label [-f -g -t template] name
    p4 label -d [-f -g] name
    p4 label -o [-t template] name
    p4 label -i [-f -g]

	Create or edit a label. The name parameter is required. The
	specification form is put into a temporary file and the editor
	(configured by the environment variable $P4EDITOR) is invoked.

	The label specification form contains the following fields:

	Label:       The label name (read only.)

	Owner:       The user who created this label.  A label can be owned
	             by a user, or owned by a group.  Can be changed.

	Update:      The date that this specification was last modified.

	Access:      The date of the last 'labelsync' or use of '@label'
		     referencing this label.

	Description: A short description of the label (optional).

	Options:     Flags to change the label behavior.

	             locked       Prevents users other than the label owner
	             unlocked     from changing the specification. Prevents
	                          the label from being deleted. Prevents the
	                          owner from running 'p4 labelsync'. For a
	                          loaded label, prevents 'p4 unload'.

	             autoreload   For a static label, indicates where label
	             noautoreload revisions are stored. Specify 'noautoreload'
	                          to indicate that the revisions should be
	                          stored in the db.label table. Specify
	                          'autoreload' to indicate that the revisions
	                          should be stored in the unload depot.

	Revision:    An optional revision specification for an automatic
		     label.  Enclose in double quotes if it contains the
		     # (form comment) character.  An automatic label can
		     be treated as a pure alias of a single revision
		     specification (excluding @label) provided that the
		     View mapping is empty.

	View:        A mapping that selects files from the depot. The
		     default view selects all depot files. Only the left
		     side of the mapping is used for labels.  Leave this
		     field blank when creating an automatic label as
		     a pure alias. See 'p4 help views'.

	ServerID:    If set, restricts usage to the named server.
		     If unset, usage is allowed on any server.

	A label is a named collection of revisions. A label is either
	automatic or static. An automatic label refers to the revisions
	given in the View: and Revision: fields. When using automatic labels
	containing both View: and Revision: fields, use of the automatic labels
	to represent revision ranges might not produce the same results when
	using the equivalent changelist revision range. To make an automatic
	label behave exactly like its revision specifier, leave the View: field
	blank. Without this field, the automatic label is considered a pure
	alias and is processed exactly like the revision specification. A
	static label refers to the revisions that are associated with the label
	using the 'p4 tag' or 'p4 labelsync' commands. A static label cannot
	have a Revision: field. See 'p4 help revisions' for information on
	using labels as revision specifiers.

	Only the label owner can run 'p4 labelsync', and only if the label
	is unlocked. A label without an owner can be labelsync'd by any user.

	Flag -d deletes the specified label. You cannot delete a locked label.
	The -f flag forces the delete.

	The -o flag writes the label specification to standard output.  The
	user's editor is not invoked.

	The -i flag reads a label specification from standard input.  The
	user's editor is not invoked.

	The -t flag copies the view and options from the template label to
	the new label.

	The -f flag forces the deletion of a label. By default, locked labels
	can only be deleted by their owner.  The -f flag also permits the
	Last Modified date to be set.  The -f flag requires 'admin' access,
	which is granted by 'p4 protect'.

	The -g flag should be used on an Edge Server to update a global
	label. Without -g, the label definition is visible only to users
	of this Edge Server. Configuring rpl.labels.global=1 reverses this
	default and causes this flag to have the opposite meaning.
)"
};

ErrorId MsgHelp::HelpLabels = { ErrorOf( ES_HELP, 60, E_INFO, EV_NONE, 0 ),
R"(
    labels -- Display list of defined labels

    p4 labels [-U|-R] [-t] [-u user [--user-case-insensitive]]
	      [[-e|-E] nameFilter] [-m max] [file[revrange]]
    p4 labels [-U|-R] [-t] [-u user [--user-case-insensitive]]
	      [[-e|-E] nameFilter] [-m max] [-a|-s serverID]

	Lists labels defined in the server.

	If files are specified, 'p4 labels' lists the labels that contain
	those files.  If you include a file specification, automatic labels
	and labels with the 'autoreload' option set are omitted from the list.
	If the file specification includes a revision range, 'p4 labels'
	lists labels that contain the specified revisions.

	See 'p4 help revisions' for details about specifying revisions.

	The -t flag displays the time as well as the date.

	The -u user flag lists labels owned by the specified user.
	This can include wildcards to form a search pattern. If wildcards are
	used enclose the search pattern in double quotes. You can also add a
	--user-case-insensitive flag which will indicate that the user value
	is a case-insensitive search pattern.

	The -e nameFilter flag lists labels with a name that matches
	the nameFilter pattern, for example: -e 'svr-dev-rel*'. The -e flag
	uses the server's normal case-sensitivity rules. The -E flag makes
	the matching case-insensitive, even on a case-sensitive server.

	The -m max flag limits output to the first 'max' number of labels.

	The -U flag lists unloaded labels (see 'p4 help unload').
	
	The -R flag lists only labels with the 'autoreload' option set.

	The -a and -s flags are useful in a distributed server installation
	(see 'p4 help distributed') in order to see the names of local labels
	stored on other Edge Servers. These flags are not allowed if the
	command includes a file specification.

	The -a flag specifies that all labels should be displayed, not just
	those that are bound to this server.

	The -s serverID flag specifies that only those labels bound to the
	specified serverID should be displayed.

	On an Edge Server, if neither -s nor -a is specified, only those
	local labels bound to this Edge Server are displayed. Labels created
	on the Commit Server are global, and are also included in the output.
)"
};

ErrorId MsgHelp::HelpLabelsync = { ErrorOf( ES_HELP, 61, E_INFO, EV_NONE, 0 ),
R"(
    labelsync -- Apply the label to the contents of the client workspace

    p4 labelsync [-a -d -g -n -q] -l label [file[revRange] ...]

	Labelsync causes the specified label to reflect the current contents
	of the client.  It records the revision of each file currently synced.
	The label's name can subsequently be used in a revision specification
	as @label to refer to the revision of a file as stored in the label.

	Without a file argument, labelsync causes the label to reflect the
	contents of the whole client, by adding, deleting, and updating the
	label.  If a file is specified, labelsync updates the specified file.

	If the file argument includes a revision specification, that revision
	is used instead of the revision synced by the client. If the specified
	revision is a deleted revision, the label includes that deleted
	revision.  See 'p4 help revisions' for details about specifying
	revisions.

	If the file argument includes a revision range specification,
	only files selected by the revision range are updated, and the
	highest revision in the range is used.

	If multiple arguments are given, they are applied to the label
	sequentially, such that the last revision specified for a given
	depot file is the one that will be recorded in the label.

	The -a flag adds the specified file to the label.

	The -d deletes the specified file from the label, regardless of
	revision.

	The -n flag previews the operation without altering the label.

	Only the owner of a label can run labelsync on that label. A label
	that has its Options: field set to 'locked' cannot be updated. A
	label without an owner can be labelsync'd by any user.

	The -q flag suppresses normal output messages. Messages regarding
	errors or exceptional conditions are displayed.

	The -g flag should be used on an Edge Server to update a global
	label. Note that in this case, the client should be a global client.
	Configuring rpl.labels.global=1 reverses this default and causes this
	flag to have the opposite meaning.
)"
};

ErrorId MsgHelp::HelpNetworkAddress = { ErrorOf( ES_HELP, 161, E_INFO, EV_NONE, 0 ),
R"(
	When specifying the network address for a P4 connection, use the
	following syntax:

	    prefix:host:port

	The prefix indicates whether this connection should be plaintext
	or encrypted, and whether address resolution should prefer IPv4 or
	IPv6 addresses. Specify one of the following prefix values:

	    tcp,tcp4,tcp6,tcp46,tcp64,ssl,ssl4,ssl6,ssl46,ssl64

	The default prefix value is 'tcp'.

	The host portion of the address can be either a resolvable host name,
	or an IP address in either IPv4 or IPv6 format. Specify an IPv4
	address as NNN.NNN.NNN.NNN. Specify an IPv6 address in the format
	described by RFC 2373 and RFC 2372: [x:x:x:x:x:x:x:x]; note that the
	use of the brackets around the address removes the ambiguity about
	where the prefix and port portions of the P4 network address begin and
	end.

	The default host name is 'perforce'.

	The port number should be an integer between 1 and 65535. Note that
	your operating system may enforce a smaller range of values, and may
	require special permission for certain values (i.e., values less than
	1025 may require root permission on Unix)

	The default port number is 1666.

)"
};

ErrorId MsgHelp::HelpLdap = { ErrorOf( ES_HELP, 172, E_INFO, EV_NONE, 0 ),
R"(
    ldap -- Create, modify, delete or test an LDAP configuration

    p4 ldap config
    p4 ldap -d config
    p4 ldap -o config
    p4 ldap -i
    p4 ldap -t username config

	An LDAP configuration defines an Active Directory or other LDAP server
	to which the P4 server can connect in order to authenticate users who
	have AuthMethod set to 'ldap'. In addition to the host and port of the
	LDAP server, this configuration defines how the authentication should
	happen, by picking one of three bind methods:

	Simple:
	This authentication method takes a template DN and substitutes %%user%%
	placeholders with the user's userId to produce the user's DN which the
	P4 Server will then attempt to bind against, validating the user's
	password. An example of such a template could look like this:

	    uid=%%user%%,ou=users,dc=example,dc=org

	Search:
	This authentication method uses an LDAP search query to locate the
	appropriate user record. It requires a known base DN for the search
	and an LDAP search query (which should make use of the %%user%%
	placeholder). It may also require the full DN and password of a known
	read-only entity in the directory in order to perform the search.
	An example base DN and LDAP query might look like this:

	    Base DN: ou=users,dc=example,dc=org
	    LDAP query: (uid=%%user%%)

	SASL:
	This authentication method defers the user search to the LDAP server
	and does not require a DN to be discovered before a bind is attempted.
	A specific realm may be provided if the LDAP server has multiple
	realms (or domains in the case of Active Directory). Additionally, if
	'realminusername' option is enabled, the realm will be separated from
	the username if that username is in either UNC (realm\user) or
	UPN (user@realm) formats.

	SASL is generally the easiest method to configure if the LDAP server
	supports SASL DIGEST-MD5.

	To enable LDAP authentication, one or more LDAP configurations must
	exist and be assigned a priority with the 'auth.ldap.order.N'
	configurable. The server will need to be restarted before LDAP
	authentication will be enabled. Note: LDAP authentication implies at
	least security level 3; all must use ticket based authentication (see
	'p4 login') and any users authenticating against the P4 database must
	have strong passwords.

	If more than one 'auth.ldap.order.N' configurable is set to a valid
	configuration, each one is considered enabled and a login by a user
	with AuthMethod set to ldap will cause each enabled configuration
	to be checked in order until the user's credentials are validated or
	the user record is found but the credentials are rejected.

	When LDAP authentication is enabled and the 'auth.default.method'
	configurable is set to 'ldap' and the 'auth.ldap.userautocreate'
	configureable is set to 1, any user who is able to authenticate will be
	created and will consume a license, regardless of whether they are
	granted access by 'p4 protect'. Setting this configurable to 2 will
	perform an additional check that the user would have protections before
	allowing the user to authenticate. If the 'getattrs' option is enabled,
	the new user's Fullname and Email fields will be populated from the
	user's record in the directory using the values in the LDAP attributes
	specified in the 'AttributeName' and 'AttributeEmail' fields. If the
	'BindMethod' is 'SASL', this feature also requires the 'SearchBaseDN'
	and 'SearchFilter' fields to be configured.

	In addition to the bind methods, user access can be restricted only to
	those who belong to a particular group within the directory. This is
	achieved by performing an LDAP search as the user who has just been
	authenticated, and checking that there is at least one record returned
	from that search. This search requires a base DN and a LDAP query that
	contains the %%user%% placeholder. An example base DN and LDAP query to
	check that the user is in a group named 'perforce' might look like:

	    Base DN: ou=groups,dc=example,dc=org
	    LDAP query: (&(cn=perforce)(memberUid=%%user%%))


	The LDAP configuration spec contains the following fields:

	Name:          The LDAP configuration name.

	Host:          The FQDN of the directory server.

	Port:          The port number at which to connect to the directory
	               server.

	Encryption:    The encryption method to use when connecting to the
	               directory server. Options are: 'none', 'ssl' and 'tls'.

	BindMethod:    The bind method to use with this directory. Options
	               are: 'simple', 'search' and 'sasl'.

	Options:       Modifiers to the behavior of the LDAP integration,
	               specific to this configuration. Options are:
	                   [no]downcase - should 'p4 ldapsync' downcase
	                                  usernames from the directory.
	                   [no]getattrs - should the Fullname and Email fields
	                                  for users created by 'p4 login'
	                                  be populated from the directory.
	                   [no]realminusername - take the realm from the SASL
	                                         username if it is in UNC or
	                                         UPN format.

	SimplePattern: This is the DN which will be used to bind against to
	               validate the user's credentials. The %%user%% placeholder
	               will be replaced with the user's userId. This field
	               only applies to the 'simple' bind method.

	SearchBaseDN:  The DN from which to start the search for the user
	               object.

	SearchFilter:  The LDAP query filter to use to identify the user object
	               which will be used to bind against. The %%user%%
	               placeholder will be replaced with the user's userId.

	SearchScope:   The scope to use when searching for the user records.
	               Options are:
	                   baseonly - Just the BaseDN object
	                   children - The BaseDN and its direct children
	                   subtree - The BaseDN and all objects below it

	SearchBindDN:  The DN to bind against in order to search the directory.

	SearchPasswd:  The password for the BindDN record.

	SaslRealm:     The optional realm to use when authenticating the user
	               via SASL.This field applies only to the 'sasl' bind
	               method.

	GroupSearchFilter: The filter to use for the group search.

	GroupBaseDN:       The search base for performing the group search. If
	                   unset this will default to the SearchBaseDN.

	GroupSearchScope:  The scope to use when performing the group search.
	                   Options are:
	                       baseonly - Just the BaseDN object
	                       children - The BaseDN and its direct children
	                       subtree - The BaseDN and all objects below it

	AttributeUid:      The name of the attribute in the user object that
	                   contains the user's UID.

	AttributeName:     The name(s) of the attribute(s) in the user object
	                   that contains the user's full name. If multiple
	                   attributes are required to form the full name,
	                   specify each one surrounded by %% symbols such
	                   that expanding them forms the user's full name."

	AttributeEmail:    The name of the attribute in the directory's user
	                   object that contain the users' email addresses.


	The GroupSearchFilter field is only needed if users must also belong
	to a group within the directory. The other two group fields may also
	be used by the 'p4 ldapsync' command.


	The -d flag deletes the named LDAP configuration.

	The -o flag writes the LDAP configuration to standard output. The
	user's editor is not invoked.

	The -i flag causes a LDAP configuration to be read from the standard
	input. The user's editor is not invoked.

	The -t flag specifies a username to attempt authentication against
	the named LDAP configuration. This will return a success message or an
	error message that will contain more detailed information when
	available.

	'p4 ldap' requires 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLdaps = { ErrorOf( ES_HELP, 173, E_INFO, EV_NONE, 0 ),
R"(
    ldaps -- Display list of LDAP configurations

    p4 ldaps [-A]
    p4 ldaps -t username

	Lists LDAP configurations. (See 'p4 help ldap'.)

	The -A flag limits the results to a priority ordered list of
	active configurations.

	The -t flag specifies a username to attempt authentication against
	each active configuration (all active configurations will be tested).

	'p4 ldaps' requires 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLdapSync = { ErrorOf( ES_HELP, 187, E_INFO, EV_NONE, 0 ),
R"(
    ldapsync -- Synchronizes P4 group memberships with LDAP groups
                or P4 users with LDAP users.

    p4 ldapsync -g [ -n ] [ -i <N> ] [ group ... ]
    p4 ldapsync -u [ -c -U -d ] [ -n ] [ -i <N> ] [ ldap ... ]

	When run with the -g flag specified, this command updates the users
	lists in P4 groups to match the lists of members in LDAP groups. If one
	or more group names are provided, only those groups are updated; if no
	groups are provided, then all groups with LDAP configurations will be
	updated.

	When run with the -u flag specified, this command updates the P4
	users to match those in the LDAP.  This works by querying each
	LDAP server defined by the LDAP specifications passed in the arguments.
	The LDAP specification's SearchFilter is used to query the LDAP server
	with the %%user%% placeholder expanded to * in order to identify all
	LDAP users.  The three Attribute* fields are used to map LDAP result to
	the P4 user's username, full name and email address.  All provided LDAP
	specifications are queried to build a full, combined list of LDAP users
	before any changes to the P4 users are made.

	The user synchronization  has three actions that must be enabled
	separately by specifying the appropriate flags:

	    The -c flag creates any new users found in the LDAP servers that
	    do not yet exist in P4. The AuthMethod will be set to ldap and Type
	    set to standard.

	    The -U flag updates the full name and email address of any existing
	    P4 users found in the LDAP servers, provided that the user is of
	    Type standard and AuthMethod ldap and that the values differ.

	    The -d flag deletes any P4 users not found in any of the LDAP
	    servers, provided that the user is of Type standard and AuthMethod
	    ldap.

	To keep users or groups with LDAP configurations in sync with their
	LDAP counterparts, ldapsync can be set as a startup command, and will
	run in the background.

	The -n flag previews the operations without altering the users or
	groups.

	The -i flag causes the command to automatically repeat its action
	every N seconds. If -i is not specified, the command runs once,
	then exits.

	'p4 ldapsync' requires 'super' access granted by 'p4 protect'.
)"
};

# include <openssl/opensslv.h>
const char* legalText = R"more(
    p4 help [ -l --local ] legal

    The -l flag prints out client-side license info.

    For Perforce legal and license information see:
	http://www.perforce.com/purchase/license-agreements

    P4/P4API License
    -----------------------
    Copyright (c) 1995-)more" ID_Y R"more(, Perforce Software, Inc.
    All rights reserved.
 
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
 
    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.


    OpenSSL:
    -----------------------)more"
# if OPENSSL_VERSION_NUMBER < 0x30000000L
R"more(
	This product includes cryptographic software written by Eric Young
	(eay@cryptsoft.com).
	This product includes software written by Tim Hudson
	(tjh@cryptsoft.com).

    OpenSSL License
    -----------------------

     * ====================================================================
     * Copyright (c) 1998-2011 The OpenSSL Project.  All rights reserved.
     *
     * Redistribution and use in source and binary forms, with or without
     * modification, are permitted provided that the following conditions
     * are met:
     *
     * 1. Redistributions of source code must retain the above copyright
     *    notice, this list of conditions and the following disclaimer.
     *
     * 2. Redistributions in binary form must reproduce the above copyright
     *    notice, this list of conditions and the following disclaimer in
     *    the documentation and/or other materials provided with the
     *    distribution.
     *
     * 3. All advertising materials mentioning features or use of this
     *    software must display the following acknowledgment:
     *    "This product includes software developed by the OpenSSL Project
     *    for use in the OpenSSL Toolkit. (http://www.openssl.org/)
     *
     * 4. The names "OpenSSL Toolkit" and "OpenSSL Project" must not be
     *    used to endorse or promote products derived from this software
     *    without prior written permission. For written permission, please
     *    contact openssl-core@openssl.org.
     *
     * 5. Products derived from this software may not be called "OpenSSL
     *    nor may "OpenSSL" appear in their names without prior written
     *    permission of the OpenSSL Project.
     *
     * 6. Redistributions of any form whatsoever must retain the following
     *    acknowledgment:
     *    "This product includes software developed by the OpenSSL Project
     *    for use in the OpenSSL Toolkit (http://www.openssl.org/)
     *
     * THIS SOFTWARE IS PROVIDED BY THE OpenSSL PROJECT ``AS IS'' AND ANY
     * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
     * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE OpenSSL PROJECT OR
     * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
     * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
     * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
     * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
     * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
     * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
     * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
     * OF THE POSSIBILITY OF SUCH DAMAGE.
     * ====================================================================
     *
     * This product includes cryptographic software written by Eric Young
     * (eay@cryptsoft.com).  This product includes software written by Tim
     * Hudson (tjh@cryptsoft.com).
     *
     *

    Original SSLeay License
    -----------------------

     * Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
     * All rights reserved.
     *
     * This package is an SSL implementation written
     * by Eric Young (eay@cryptsoft.com).
     * The implementation was written so as to conform with Netscapes SSL.
     *
     * This library is free for commercial and non-commercial use as long as
     * the following conditions are adhered to.  The following conditions
     * apply to all code found in this distribution, be it the RC4, RSA,
     * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
     * included with this distribution is covered by the same copyright terms
     * except that the holder is Tim Hudson (tjh@cryptsoft.com).
     *
     * Copyright remains Eric Young's, and as such any Copyright notices in
     * the code are not to be removed.
     * If this package is used in a product, Eric Young should be given
     * attribution as the author of the parts of the library used.
     * This can be in the form of a textual message at program startup or
     * in documentation (online or textual) provided with the package.
     *
     * Redistribution and use in source and binary forms, with or without
     * modification, are permitted provided that the following conditions
     * are met:
     * 1. Redistributions of source code must retain the copyright
     *    notice, this list of conditions and the following disclaimer.
     * 2. Redistributions in binary form must reproduce the above copyright
     *    notice, this list of conditions and the following disclaimer in the
     *    documentation and/or other materials provided with the distribution.
     * 3. All advertising materials mentioning features or use of this
     *    software must display the following acknowledgement:
     *    "This product includes cryptographic software written by
     *     Eric Young (eay@cryptsoft.com)"
     *    The word 'cryptographic' can be left out if the routines from the
     *    library being used are not cryptographic related  :-) .
     * 4. If you include any Windows specific code (or a derivative thereof)
     *    from the apps directory (application code) you must include an
     *    acknowledgement: "This product includes software written by Tim
     *    Hudson (tjh@cryptsoft.com)"
     *
     * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
     * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
     * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS
     * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
     * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
     * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
     * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
     * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
     * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
     * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
     *
     * The licence and distribution terms for any publicly available version
     * or derivative of this code cannot be changed.  i.e. this code cannot
     * simply be copied and put under another distribution licence
     * [including the GNU Public Licence.]
     *)more"
# else
R"more(
    Copyright (c) 1998-2022 The OpenSSL Project
    Copyright (c) 1995-1998 Eric A. Young, Tim J. Hudson
    All rights reserved.

    OpenSSL License
    -----------------------

                                 Apache License
                           Version 2.0, January 2004
                        https://www.apache.org/licenses/

    TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

    1. Definitions.

       "License" shall mean the terms and conditions for use, reproduction,
       and distribution as defined by Sections 1 through 9 of this document.

       "Licensor" shall mean the copyright owner or entity authorized by
       the copyright owner that is granting the License.

       "Legal Entity" shall mean the union of the acting entity and all
       other entities that control, are controlled by, or are under common
       control with that entity. For the purposes of this definition,
       "control" means (i) the power, direct or indirect, to cause the
       direction or management of such entity, whether by contract or
       otherwise, or (ii) ownership of fifty percent (50%) or more of the
       outstanding shares, or (iii) beneficial ownership of such entity.

       "You" (or "Your") shall mean an individual or Legal Entity
       exercising permissions granted by this License.

       "Source" form shall mean the preferred form for making modifications,
       including but not limited to software source code, documentation
       source, and configuration files.

       "Object" form shall mean any form resulting from mechanical
       transformation or translation of a Source form, including but
       not limited to compiled object code, generated documentation,
       and conversions to other media types.

       "Work" shall mean the work of authorship, whether in Source or
       Object form, made available under the License, as indicated by a
       copyright notice that is included in or attached to the work
       (an example is provided in the Appendix below).

       "Derivative Works" shall mean any work, whether in Source or Object
       form, that is based on (or derived from) the Work and for which the
       editorial revisions, annotations, elaborations, or other modifications
       represent, as a whole, an original work of authorship. For the purposes
       of this License, Derivative Works shall not include works that remain
       separable from, or merely link (or bind by name) to the interfaces of,
       the Work and Derivative Works thereof.

       "Contribution" shall mean any work of authorship, including
       the original version of the Work and any modifications or additions
       to that Work or Derivative Works thereof, that is intentionally
       submitted to Licensor for inclusion in the Work by the copyright owner
       or by an individual or Legal Entity authorized to submit on behalf of
       the copyright owner. For the purposes of this definition, "submitted"
       means any form of electronic, verbal, or written communication sent
       to the Licensor or its representatives, including but not limited to
       communication on electronic mailing lists, source code control systems,
       and issue tracking systems that are managed by, or on behalf of, the
       Licensor for the purpose of discussing and improving the Work, but
       excluding communication that is conspicuously marked or otherwise
       designated in writing by the copyright owner as "Not a Contribution."

       "Contributor" shall mean Licensor and any individual or Legal Entity
       on behalf of whom a Contribution has been received by Licensor and
       subsequently incorporated within the Work.

    2. Grant of Copyright License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       copyright license to reproduce, prepare Derivative Works of,
       publicly display, publicly perform, sublicense, and distribute the
       Work and such Derivative Works in Source or Object form.

    3. Grant of Patent License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       (except as stated in this section) patent license to make, have made,
       use, offer to sell, sell, import, and otherwise transfer the Work,
       where such license applies only to those patent claims licensable
       by such Contributor that are necessarily infringed by their
       Contribution(s) alone or by combination of their Contribution(s)
       with the Work to which such Contribution(s) was submitted. If You
       institute patent litigation against any entity (including a
       cross-claim or counterclaim in a lawsuit) alleging that the Work
       or a Contribution incorporated within the Work constitutes direct
       or contributory patent infringement, then any patent licenses
       granted to You under this License for that Work shall terminate
       as of the date such litigation is filed.

    4. Redistribution. You may reproduce and distribute copies of the
       Work or Derivative Works thereof in any medium, with or without
       modifications, and in Source or Object form, provided that You
       meet the following conditions:

       (a) You must give any other recipients of the Work or
           Derivative Works a copy of this License; and

       (b) You must cause any modified files to carry prominent notices
           stating that You changed the files; and

       (c) You must retain, in the Source form of any Derivative Works
           that You distribute, all copyright, patent, trademark, and
           attribution notices from the Source form of the Work,
           excluding those notices that do not pertain to any part of
           the Derivative Works; and

       (d) If the Work includes a "NOTICE" text file as part of its
           distribution, then any Derivative Works that You distribute must
           include a readable copy of the attribution notices contained
           within such NOTICE file, excluding those notices that do not
           pertain to any part of the Derivative Works, in at least one
           of the following places: within a NOTICE text file distributed
           as part of the Derivative Works; within the Source form or
           documentation, if provided along with the Derivative Works; or,
           within a display generated by the Derivative Works, if and
           wherever such third-party notices normally appear. The contents
           of the NOTICE file are for informational purposes only and
           do not modify the License. You may add Your own attribution
           notices within Derivative Works that You distribute, alongside
           or as an addendum to the NOTICE text from the Work, provided
           that such additional attribution notices cannot be construed
           as modifying the License.

       You may add Your own copyright statement to Your modifications and
       may provide additional or different license terms and conditions
       for use, reproduction, or distribution of Your modifications, or
       for any such Derivative Works as a whole, provided Your use,
       reproduction, and distribution of the Work otherwise complies with
       the conditions stated in this License.

    5. Submission of Contributions. Unless You explicitly state otherwise,
       any Contribution intentionally submitted for inclusion in the Work
       by You to the Licensor shall be under the terms and conditions of
       this License, without any additional terms or conditions.
       Notwithstanding the above, nothing herein shall supersede or modify
       the terms of any separate license agreement you may have executed
       with Licensor regarding such Contributions.

    6. Trademarks. This License does not grant permission to use the trade
       names, trademarks, service marks, or product names of the Licensor,
       except as required for reasonable and customary use in describing the
       origin of the Work and reproducing the content of the NOTICE file.

    7. Disclaimer of Warranty. Unless required by applicable law or
       agreed to in writing, Licensor provides the Work (and each
       Contributor provides its Contributions) on an "AS IS" BASIS,
       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
       implied, including, without limitation, any warranties or conditions
       of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
       PARTICULAR PURPOSE. You are solely responsible for determining the
       appropriateness of using or redistributing the Work and assume any
       risks associated with Your exercise of permissions under this License.

    8. Limitation of Liability. In no event and under no legal theory,
       whether in tort (including negligence), contract, or otherwise,
       unless required by applicable law (such as deliberate and grossly
       negligent acts) or agreed to in writing, shall any Contributor be
       liable to You for damages, including any direct, indirect, special,
       incidental, or consequential damages of any character arising as a
       result of this License or out of the use or inability to use the
       Work (including but not limited to damages for loss of goodwill,
       work stoppage, computer failure or malfunction, or any and all
       other commercial damages or losses), even if such Contributor
       has been advised of the possibility of such damages.

    9. Accepting Warranty or Additional Liability. While redistributing
       the Work or Derivative Works thereof, You may choose to offer,
       and charge a fee for, acceptance of support, warranty, indemnity,
       or other liability obligations and/or rights consistent with this
       License. However, in accepting such obligations, You may act only
       on Your own behalf and on Your sole responsibility, not on behalf
       of any other Contributor, and only if You agree to indemnify,
       defend, and hold each Contributor harmless for any liability
       incurred by, or claims asserted against, such Contributor by reason
       of your accepting any such warranty or additional liability.

    END OF TERMS AND CONDITIONS
)more"
# endif
R"more(

    OpenLDAP:
    -----------------------

    Copyright 1998-2014 The OpenLDAP Foundation
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted only as authorized by the OpenLDAP
    Public License.

    A copy of this license is available in the file LICENSE in the
    top-level directory of the distribution or, alternatively, at
    <http://www.OpenLDAP.org/license.html>.

    OpenLDAP is a registered trademark of the OpenLDAP Foundation.

    Individual files and/or contributed packages may be copyright by
    other parties and/or subject to additional restrictions.

    This work is derived from the University of Michigan LDAP v3.3
    distribution.  Information concerning this software is available
    at <http://www.umich.edu/~dirsvcs/ldap/ldap.html>.

    This work also contains materials derived from public sources.

    Additional information about OpenLDAP can be obtained at
    <http://www.openldap.org/>.

    ---

    Portions Copyright 1998-2012 Kurt D. Zeilenga.
    Portions Copyright 1998-2006 Net Boolean Incorporated.
    Portions Copyright 2001-2006 IBM Corporation.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted only as authorized by the OpenLDAP
    Public License.

    ---

    Portions Copyright 1999-2008 Howard Y.H. Chu.
    Portions Copyright 1999-2008 Symas Corporation.
    Portions Copyright 1998-2003 Hallvard B. Furuseth.
    Portions Copyright 2007-2011 Gavin Henry.
    Portions Copyright 2007-2011 Suretec Systems Ltd.
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that this notice is preserved.
    The names of the copyright holders may not be used to endorse or
    promote products derived from this software without their specific
    prior written permission.  This software is provided ``as is''
    without express or implied warranty.

    ---

    Portions Copyright (c) 1992-1996 Regents of the University of Michigan.
    All rights reserved.

    Redistribution and use in source and binary forms are permitted
    provided that this notice is preserved and that due credit is given
    to the University of Michigan at Ann Arbor.  The name of the
    University may not be used to endorse or promote products derived
    from this software without specific prior written permission.  This
    software is provided ``as is'' without express or implied warranty.


    OpenLDAP License
    -----------------------

    The OpenLDAP Public License
       Version 2.8, 17 August 2003

    Redistribution and use of this software and associated documentation
    ("Software"), with or without modification, are permitted provided
    that the following conditions are met:

    1. Redistributions in source form must retain copyright statements
       and notices,

    2. Redistributions in binary form must reproduce applicable copyright
       statements and notices, this list of conditions, and the following
       statements and notices, this list of conditions, and the following
       disclaimer in the documentation and/or other materials provided
       with the distribution, and

    3. Redistributions must contain a verbatim copy of this document.

    The OpenLDAP Foundation may revise this license from time to time.
    Each revision is distinguished by a version number.  You may use
    this Software under terms of this license revision or under the
    terms of any subsequent revision of the license.

    THIS SOFTWARE IS PROVIDED BY THE OPENLDAP FOUNDATION AND ITS
    CONTRIBUTORS ``AS IS'' AND ANY EXPRESSED OR IMPLIED WARRANTIES,
    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
    AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT
    SHALL THE OPENLDAP FOUNDATION, ITS CONTRIBUTORS, OR THE AUTHOR(S)
    OR OWNER(S) OF THE SOFTWARE BE LIABLE FOR ANY DIRECT, INDIRECT,
    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
    BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    The names of the authors and copyright holders must not be used in
    advertising or otherwise to promote the sale, use or other dealing
    in this Software without specific, written prior permission.  Title
    to copyright in this Software shall at all times remain with copyright
    holders.

    OpenLDAP is a registered trademark of the OpenLDAP Foundation.

    Copyright 1999-2003 The OpenLDAP Foundation, Redwood City,
    California, USA.  All Rights Reserved.  Permission to copy and
    distribute verbatim copies of this document is granted.


    Cyrus SASL License
    -----------------------

    /* CMU libsasl
     * Tim Martin
     * Rob Earhart
     * Rob Siemborski
     */
    /*
     * Copyright (c) 1998-2003 Carnegie Mellon University.  All rights
     * reserved.
     *
     * Redistribution and use in source and binary forms, with or without
     * modification, are permitted provided that the following conditions
     * are met:
     *
     * 1. Redistributions of source code must retain the above copyright
     *    notice, this list of conditions and the following disclaimer.
     *
     * 2. Redistributions in binary form must reproduce the above copyright
     *    notice, this list of conditions and the following disclaimer in
     *    the documentation and/or other materials provided with the
     *    distribution.
     *
     * 3. The name "Carnegie Mellon University" must not be used to
     *    endorse or promote products derived from this software without
     *    prior written permission. For permission or any other legal
     *    details, please contact
     *      Office of Technology Transfer
     *      Carnegie Mellon University
     *      5000 Forbes Avenue
     *      Pittsburgh, PA  15213-3890
     *      (412) 268-4387, fax: (412) 268-7395
     *      tech-transfer@andrew.cmu.edu
     *
     * 4. Redistributions of any form whatsoever must retain the following
     *    acknowledgment:
     *    "This product includes software developed by Computing Services
     *     at Carnegie Mellon University (http://www.cmu.edu/computing/)."
     *
     * CARNEGIE MELLON UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO
     * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
     * AND FITNESS, IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY BE LIABLE
     * FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
     * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
     * AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING
     * OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
     */


    jsmn License
    -----------------------

    Copyright (c) 2010 Serge A. Zaitsev

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.


    sol2 License
    -----------------------

    Copyright (c) 2013-2018 Rapptz, ThePhD, and contributors

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.)more"
R"more(

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


    Lua License
    -----------------------

    Copyright (c) 1994-2018 Lua.org, PUC-Rio.

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
    LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
    OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


    cURL License
    -----------------------

    This code uses a permissive license. Please use the following
    attribution in the documentation of the open source code.

    Copyright (c) 1996 - 2018, Daniel Stenberg, daniel@haxx.se, and many
    contributors, see the THANKS file.

    All rights reserved.

    Permission to use, copy, modify, and distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF
    THIRD PARTY RIGHTS. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
    BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
    ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
    CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Except as contained in this notice, the name of a copyright holder shall
    not be used in advertising or otherwise to promote the sale, use or
    other dealings in this Software without prior written authorization of
    the copyright holder.


    Lua-cURLv3 License
    -----------------------

    Copyright (c) 2014-2018 Alexey Melnichuk

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to permit
    persons to whom the Software is furnished to do so, subject to the
    following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
    NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
    USE OR OTHER DEALINGS IN THE SOFTWARE.


    LuaSQLite3 License
    -----------------------

    Copyright (C) 2002-2016 Tiago Dionizio, Doug Currie
    All rights reserved.
    Author : Tiago Dionizio <tiago.dionizio@ist.utl.pt>
    Author : Doug Currie <doug.currie@alum.mit.edu>
    Library : lsqlite3 - an SQLite 3 database binding for Lua 5

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


    lua-cjson License
    -----------------------

    Copyright (c) 2010-2012 Mark Pulford <mark@kyne.com.au>

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to
    the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
    SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


    libc++ and libc++abi License
    -----------------------

    These libraries are dual-licensed:

    The University of Illinois/NCSA Open Source License (NCSA)
    Developed under the LLVM Project

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the
    "Software"), to deal with the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to
    permit persons to whom the Software is furnished to do so, subject to the
    following conditions:

    Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimers.
    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimers in the
    documentation and/or other materials provided with the distribution.
    Neither the names of <Name of Development Group, Name of Institution>,
    nor the names of its contributors may be used to endorse or promote
    products derived from this Software without specific prior written
    permission.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
    TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
     SOFTWARE OR THE USE OR OTHER DEALINGS WITH THE SOFTWARE.

    MIT
    Developed under the LLVM Project

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to permit
    persons to whom the Software is furnished to do so, subject to the
    following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
    OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN
    NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
    OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
    USE OR OTHER DEALINGS IN THE SOFTWARE.


    JSON for Modern C++
    -----------------------

    Copyright (c) 2013-2018 Niels Lohmann

    Permission is hereby granted, free of charge, to any person obtaining
    a copy of this software and associated documentation files (the
    "Software"), to deal in the Software without restriction, including
    without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to permit
    persons to whom the Software is furnished to do so, subject to the
    following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
    IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
    CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
    OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
    THE USE OR OTHER DEALINGS IN THE SOFTWARE.


    SafeInt
    -----------------------

    MIT License

    Copyright (c) 2018 Microsoft

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software")
    , to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.


    jemalloc
    -----------------------

    Copyright (C) 2002-present Jason Evans <jasone@canonware.com>.
    All rights reserved.
    Copyright (C) 2007-2012 Mozilla Foundation.  All rights reserved.
    Copyright (C) 2009-present Facebook, Inc.  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    1. Redistributions of source code must retain the above copyright
       notice(s), this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice(s), this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER(S) ``AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER(S) BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.


    mimalloc
    -----------------------

    MIT License

    Copyright (c) 2019 Microsoft Corporation, Daan Leijen

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.


    optional-lite and any-lite
    Martin Moene
    -----------------------

    Boost Software License - Version 1.0 - August 17th, 2003

    Permission is hereby granted, free of charge, to any person or organization
    obtaining a copy of the software and accompanying documentation covered by
    this license (the "Software") to use, reproduce, display, distribute,
    execute, and transmit the Software, and to prepare derivative works of the
    Software, and to permit third-parties to whom the Software is furnished to
    do so, all subject to the following:

    The copyright notices in the Software and this entire statement, including
    the above license grant, this restriction and the following disclaimer,
    must be included in all copies of the Software, in whole or in part, and
    all derivative works of the Software, unless such copies or derivative
    works are solely in the form of machine-executable object code generated by
    a source language processor.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
    SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
    FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
    ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
)more"
R"more(
    Zlib
    Jean-loup Gailly and Mark Adler, et all
    -----------------------

    Copyright (C) 1995-2017 Jean-loup Gailly and Mark Adler

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.


    Arduino PID Library
    Brett Beauregard
    -----------------------

    MIT License

    Copyright (c) 2017 Brett Beauregard

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.


    TinyXML-2
    -----------------------

    Copyright (c) 2019-present, Lee Thomason and contributors.

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any
    damages arising from the use of this software.

    Permission is granted to anyone to use this software for any
    purpose, including commercial applications, and to alter it and
    redistribute it freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must
    not claim that you wrote the original software. If you use this
    software in a product, an acknowledgment in the product documentation
    would be appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and
    must not be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
)more"
R"more(
    BLAKE3
    --------------------

    Copyright 2019 Jack O'Connor and Samuel Neves

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

                                 Apache License
                           Version 2.0, January 2004
                        https://www.apache.org/licenses/

    TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

    1. Definitions.

       "License" shall mean the terms and conditions for use, reproduction,
       and distribution as defined by Sections 1 through 9 of this document.

       "Licensor" shall mean the copyright owner or entity authorized by
       the copyright owner that is granting the License.

       "Legal Entity" shall mean the union of the acting entity and all
       other entities that control, are controlled by, or are under common
       control with that entity. For the purposes of this definition,
       "control" means (i) the power, direct or indirect, to cause the
       direction or management of such entity, whether by contract or
       otherwise, or (ii) ownership of fifty percent (50%) or more of the
       outstanding shares, or (iii) beneficial ownership of such entity.

       "You" (or "Your") shall mean an individual or Legal Entity
       exercising permissions granted by this License.

       "Source" form shall mean the preferred form for making modifications,
       including but not limited to software source code, documentation
       source, and configuration files.

       "Object" form shall mean any form resulting from mechanical
       transformation or translation of a Source form, including but
       not limited to compiled object code, generated documentation,
       and conversions to other media types.

       "Work" shall mean the work of authorship, whether in Source or
       Object form, made available under the License, as indicated by a
       copyright notice that is included in or attached to the work
       (an example is provided in the Appendix below).

       "Derivative Works" shall mean any work, whether in Source or Object
       form, that is based on (or derived from) the Work and for which the
       editorial revisions, annotations, elaborations, or other modifications
       represent, as a whole, an original work of authorship. For the purposes
       of this License, Derivative Works shall not include works that remain
       separable from, or merely link (or bind by name) to the interfaces of,
       the Work and Derivative Works thereof.

       "Contribution" shall mean any work of authorship, including
       the original version of the Work and any modifications or additions
       to that Work or Derivative Works thereof, that is intentionally
       submitted to Licensor for inclusion in the Work by the copyright owner
       or by an individual or Legal Entity authorized to submit on behalf of
       the copyright owner. For the purposes of this definition, "submitted"
       means any form of electronic, verbal, or written communication sent
       to the Licensor or its representatives, including but not limited to
       communication on electronic mailing lists, source code control systems,
       and issue tracking systems that are managed by, or on behalf of, the
       Licensor for the purpose of discussing and improving the Work, but
       excluding communication that is conspicuously marked or otherwise
       designated in writing by the copyright owner as "Not a Contribution."

       "Contributor" shall mean Licensor and any individual or Legal Entity
       on behalf of whom a Contribution has been received by Licensor and
       subsequently incorporated within the Work.

    2. Grant of Copyright License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       copyright license to reproduce, prepare Derivative Works of,
       publicly display, publicly perform, sublicense, and distribute the
       Work and such Derivative Works in Source or Object form.

    3. Grant of Patent License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       (except as stated in this section) patent license to make, have made,
       use, offer to sell, sell, import, and otherwise transfer the Work,
       where such license applies only to those patent claims licensable
       by such Contributor that are necessarily infringed by their
       Contribution(s) alone or by combination of their Contribution(s)
       with the Work to which such Contribution(s) was submitted. If You
       institute patent litigation against any entity (including a
       cross-claim or counterclaim in a lawsuit) alleging that the Work
       or a Contribution incorporated within the Work constitutes direct
       or contributory patent infringement, then any patent licenses
       granted to You under this License for that Work shall terminate
       as of the date such litigation is filed.

    4. Redistribution. You may reproduce and distribute copies of the
       Work or Derivative Works thereof in any medium, with or without
       modifications, and in Source or Object form, provided that You
       meet the following conditions:

       (a) You must give any other recipients of the Work or
           Derivative Works a copy of this License; and

       (b) You must cause any modified files to carry prominent notices
           stating that You changed the files; and

       (c) You must retain, in the Source form of any Derivative Works
           that You distribute, all copyright, patent, trademark, and
           attribution notices from the Source form of the Work,
           excluding those notices that do not pertain to any part of
           the Derivative Works; and

       (d) If the Work includes a "NOTICE" text file as part of its
           distribution, then any Derivative Works that You distribute must
           include a readable copy of the attribution notices contained
           within such NOTICE file, excluding those notices that do not
           pertain to any part of the Derivative Works, in at least one
           of the following places: within a NOTICE text file distributed
           as part of the Derivative Works; within the Source form or
           documentation, if provided along with the Derivative Works; or,
           within a display generated by the Derivative Works, if and
           wherever such third-party notices normally appear. The contents
           of the NOTICE file are for informational purposes only and
           do not modify the License. You may add Your own attribution
           notices within Derivative Works that You distribute, alongside
           or as an addendum to the NOTICE text from the Work, provided
           that such additional attribution notices cannot be construed
           as modifying the License.

       You may add Your own copyright statement to Your modifications and
       may provide additional or different license terms and conditions
       for use, reproduction, or distribution of Your modifications, or
       for any such Derivative Works as a whole, provided Your use,
       reproduction, and distribution of the Work otherwise complies with
       the conditions stated in this License.

    5. Submission of Contributions. Unless You explicitly state otherwise,
       any Contribution intentionally submitted for inclusion in the Work
       by You to the Licensor shall be under the terms and conditions of
       this License, without any additional terms or conditions.
       Notwithstanding the above, nothing herein shall supersede or modify
       the terms of any separate license agreement you may have executed
       with Licensor regarding such Contributions.

    6. Trademarks. This License does not grant permission to use the trade
       names, trademarks, service marks, or product names of the Licensor,
       except as required for reasonable and customary use in describing the
       origin of the Work and reproducing the content of the NOTICE file.

    7. Disclaimer of Warranty. Unless required by applicable law or
       agreed to in writing, Licensor provides the Work (and each
       Contributor provides its Contributions) on an "AS IS" BASIS,
       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
       implied, including, without limitation, any warranties or conditions
       of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
       PARTICULAR PURPOSE. You are solely responsible for determining the
       appropriateness of using or redistributing the Work and assume any
       risks associated with Your exercise of permissions under this License.

    8. Limitation of Liability. In no event and under no legal theory,
       whether in tort (including negligence), contract, or otherwise,
       unless required by applicable law (such as deliberate and grossly
       negligent acts) or agreed to in writing, shall any Contributor be
       liable to You for damages, including any direct, indirect, special,
       incidental, or consequential damages of any character arising as a
       result of this License or out of the use or inability to use the
       Work (including but not limited to damages for loss of goodwill,
       work stoppage, computer failure or malfunction, or any and all
       other commercial damages or losses), even if such Contributor
       has been advised of the possibility of such damages.

    9. Accepting Warranty or Additional Liability. While redistributing
       the Work or Derivative Works thereof, You may choose to offer,
       and charge a fee for, acceptance of support, warranty, indemnity,
       or other liability obligations and/or rights consistent with this
       License. However, in accepting such obligations, You may act only
       on Your own behalf and on Your sole responsibility, not on behalf
       of any other Contributor, and only if You agree to indemnify,
       defend, and hold each Contributor harmless for any liability
       incurred by, or claims asserted against, such Contributor by reason
       of your accepting any such warranty or additional liability.
)more"
R"more(


    CDC File Transfer
    -----------------------

    Copyright 2022 Lutz Justen

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
)more"
# ifdef USE_OTEL
R"more(


    OpenTelemetry C++
    --------------------

    Copyright The OpenTelemetry C++ Authors

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

                                 Apache License
                           Version 2.0, January 2004
                        https://www.apache.org/licenses/

    TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

    1. Definitions.

       "License" shall mean the terms and conditions for use, reproduction,
       and distribution as defined by Sections 1 through 9 of this document.

       "Licensor" shall mean the copyright owner or entity authorized by
       the copyright owner that is granting the License.

       "Legal Entity" shall mean the union of the acting entity and all
       other entities that control, are controlled by, or are under common
       control with that entity. For the purposes of this definition,
       "control" means (i) the power, direct or indirect, to cause the
       direction or management of such entity, whether by contract or
       otherwise, or (ii) ownership of fifty percent (50%) or more of the
       outstanding shares, or (iii) beneficial ownership of such entity.

       "You" (or "Your") shall mean an individual or Legal Entity
       exercising permissions granted by this License.

       "Source" form shall mean the preferred form for making modifications,
       including but not limited to software source code, documentation
       source, and configuration files.

       "Object" form shall mean any form resulting from mechanical
       transformation or translation of a Source form, including but
       not limited to compiled object code, generated documentation,
       and conversions to other media types.

       "Work" shall mean the work of authorship, whether in Source or
       Object form, made available under the License, as indicated by a
       copyright notice that is included in or attached to the work
       (an example is provided in the Appendix below).

       "Derivative Works" shall mean any work, whether in Source or Object
       form, that is based on (or derived from) the Work and for which the
       editorial revisions, annotations, elaborations, or other modifications
       represent, as a whole, an original work of authorship. For the purposes
       of this License, Derivative Works shall not include works that remain
       separable from, or merely link (or bind by name) to the interfaces of,
       the Work and Derivative Works thereof.

       "Contribution" shall mean any work of authorship, including
       the original version of the Work and any modifications or additions
       to that Work or Derivative Works thereof, that is intentionally
       submitted to Licensor for inclusion in the Work by the copyright owner
       or by an individual or Legal Entity authorized to submit on behalf of
       the copyright owner. For the purposes of this definition, "submitted"
       means any form of electronic, verbal, or written communication sent
       to the Licensor or its representatives, including but not limited to
       communication on electronic mailing lists, source code control systems,
       and issue tracking systems that are managed by, or on behalf of, the
       Licensor for the purpose of discussing and improving the Work, but
       excluding communication that is conspicuously marked or otherwise
       designated in writing by the copyright owner as "Not a Contribution."

       "Contributor" shall mean Licensor and any individual or Legal Entity
       on behalf of whom a Contribution has been received by Licensor and
       subsequently incorporated within the Work.

    2. Grant of Copyright License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       copyright license to reproduce, prepare Derivative Works of,
       publicly display, publicly perform, sublicense, and distribute the
       Work and such Derivative Works in Source or Object form.

    3. Grant of Patent License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       (except as stated in this section) patent license to make, have made,
       use, offer to sell, sell, import, and otherwise transfer the Work,
       where such license applies only to those patent claims licensable
       by such Contributor that are necessarily infringed by their
       Contribution(s) alone or by combination of their Contribution(s)
       with the Work to which such Contribution(s) was submitted. If You
       institute patent litigation against any entity (including a
       cross-claim or counterclaim in a lawsuit) alleging that the Work
       or a Contribution incorporated within the Work constitutes direct
       or contributory patent infringement, then any patent licenses
       granted to You under this License for that Work shall terminate
       as of the date such litigation is filed.

    4. Redistribution. You may reproduce and distribute copies of the
       Work or Derivative Works thereof in any medium, with or without
       modifications, and in Source or Object form, provided that You
       meet the following conditions:

       (a) You must give any other recipients of the Work or
           Derivative Works a copy of this License; and

       (b) You must cause any modified files to carry prominent notices
           stating that You changed the files; and

       (c) You must retain, in the Source form of any Derivative Works
           that You distribute, all copyright, patent, trademark, and
           attribution notices from the Source form of the Work,
           excluding those notices that do not pertain to any part of
           the Derivative Works; and

       (d) If the Work includes a "NOTICE" text file as part of its
           distribution, then any Derivative Works that You distribute must
           include a readable copy of the attribution notices contained
           within such NOTICE file, excluding those notices that do not
           pertain to any part of the Derivative Works, in at least one
           of the following places: within a NOTICE text file distributed
           as part of the Derivative Works; within the Source form or
           documentation, if provided along with the Derivative Works; or,
           within a display generated by the Derivative Works, if and
           wherever such third-party notices normally appear. The contents
           of the NOTICE file are for informational purposes only and
           do not modify the License. You may add Your own attribution
           notices within Derivative Works that You distribute, alongside
           or as an addendum to the NOTICE text from the Work, provided
           that such additional attribution notices cannot be construed
           as modifying the License.

       You may add Your own copyright statement to Your modifications and
       may provide additional or different license terms and conditions
       for use, reproduction, or distribution of Your modifications, or
       for any such Derivative Works as a whole, provided Your use,
       reproduction, and distribution of the Work otherwise complies with
       the conditions stated in this License.

    5. Submission of Contributions. Unless You explicitly state otherwise,
       any Contribution intentionally submitted for inclusion in the Work
       by You to the Licensor shall be under the terms and conditions of
       this License, without any additional terms or conditions.
       Notwithstanding the above, nothing herein shall supersede or modify
       the terms of any separate license agreement you may have executed
       with Licensor regarding such Contributions.

    6. Trademarks. This License does not grant permission to use the trade
       names, trademarks, service marks, or product names of the Licensor,
       except as required for reasonable and customary use in describing the
       origin of the Work and reproducing the content of the NOTICE file.

    7. Disclaimer of Warranty. Unless required by applicable law or
       agreed to in writing, Licensor provides the Work (and each
       Contributor provides its Contributions) on an "AS IS" BASIS,
       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
       implied, including, without limitation, any warranties or conditions
       of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
       PARTICULAR PURPOSE. You are solely responsible for determining the
       appropriateness of using or redistributing the Work and assume any
       risks associated with Your exercise of permissions under this License.

    8. Limitation of Liability. In no event and under no legal theory,
       whether in tort (including negligence), contract, or otherwise,
       unless required by applicable law (such as deliberate and grossly
       negligent acts) or agreed to in writing, shall any Contributor be
       liable to You for damages, including any direct, indirect, special,
       incidental, or consequential damages of any character arising as a
       result of this License or out of the use or inability to use the
       Work (including but not limited to damages for loss of goodwill,
       work stoppage, computer failure or malfunction, or any and all
       other commercial damages or losses), even if such Contributor
       has been advised of the possibility of such damages.

    9. Accepting Warranty or Additional Liability. While redistributing
       the Work or Derivative Works thereof, You may choose to offer,
       and charge a fee for, acceptance of support, warranty, indemnity,
       or other liability obligations and/or rights consistent with this
       License. However, in accepting such obligations, You may act only
       on Your own behalf and on Your sole responsibility, not on behalf
       of any other Contributor, and only if You agree to indemnify,
       defend, and hold each Contributor harmless for any liability
       incurred by, or claims asserted against, such Contributor by reason
       of your accepting any such warranty or additional liability.
)more"
R"more(


    OpenTelemetry Protocol (OTLP) Specification
    --------------------

    Copyright The OpenTelemetry Protocol (OTLP) Specification Authors

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

                                 Apache License
                           Version 2.0, January 2004
                        https://www.apache.org/licenses/

    TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

    1. Definitions.

       "License" shall mean the terms and conditions for use, reproduction,
       and distribution as defined by Sections 1 through 9 of this document.

       "Licensor" shall mean the copyright owner or entity authorized by
       the copyright owner that is granting the License.

       "Legal Entity" shall mean the union of the acting entity and all
       other entities that control, are controlled by, or are under common
       control with that entity. For the purposes of this definition,
       "control" means (i) the power, direct or indirect, to cause the
       direction or management of such entity, whether by contract or
       otherwise, or (ii) ownership of fifty percent (50%) or more of the
       outstanding shares, or (iii) beneficial ownership of such entity.

       "You" (or "Your") shall mean an individual or Legal Entity
       exercising permissions granted by this License.

       "Source" form shall mean the preferred form for making modifications,
       including but not limited to software source code, documentation
       source, and configuration files.

       "Object" form shall mean any form resulting from mechanical
       transformation or translation of a Source form, including but
       not limited to compiled object code, generated documentation,
       and conversions to other media types.

       "Work" shall mean the work of authorship, whether in Source or
       Object form, made available under the License, as indicated by a
       copyright notice that is included in or attached to the work
       (an example is provided in the Appendix below).

       "Derivative Works" shall mean any work, whether in Source or Object
       form, that is based on (or derived from) the Work and for which the
       editorial revisions, annotations, elaborations, or other modifications
       represent, as a whole, an original work of authorship. For the purposes
       of this License, Derivative Works shall not include works that remain
       separable from, or merely link (or bind by name) to the interfaces of,
       the Work and Derivative Works thereof.

       "Contribution" shall mean any work of authorship, including
       the original version of the Work and any modifications or additions
       to that Work or Derivative Works thereof, that is intentionally
       submitted to Licensor for inclusion in the Work by the copyright owner
       or by an individual or Legal Entity authorized to submit on behalf of
       the copyright owner. For the purposes of this definition, "submitted"
       means any form of electronic, verbal, or written communication sent
       to the Licensor or its representatives, including but not limited to
       communication on electronic mailing lists, source code control systems,
       and issue tracking systems that are managed by, or on behalf of, the
       Licensor for the purpose of discussing and improving the Work, but
       excluding communication that is conspicuously marked or otherwise
       designated in writing by the copyright owner as "Not a Contribution."

       "Contributor" shall mean Licensor and any individual or Legal Entity
       on behalf of whom a Contribution has been received by Licensor and
       subsequently incorporated within the Work.

    2. Grant of Copyright License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       copyright license to reproduce, prepare Derivative Works of,
       publicly display, publicly perform, sublicense, and distribute the
       Work and such Derivative Works in Source or Object form.

    3. Grant of Patent License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       (except as stated in this section) patent license to make, have made,
       use, offer to sell, sell, import, and otherwise transfer the Work,
       where such license applies only to those patent claims licensable
       by such Contributor that are necessarily infringed by their
       Contribution(s) alone or by combination of their Contribution(s)
       with the Work to which such Contribution(s) was submitted. If You
       institute patent litigation against any entity (including a
       cross-claim or counterclaim in a lawsuit) alleging that the Work
       or a Contribution incorporated within the Work constitutes direct
       or contributory patent infringement, then any patent licenses
       granted to You under this License for that Work shall terminate
       as of the date such litigation is filed.

    4. Redistribution. You may reproduce and distribute copies of the
       Work or Derivative Works thereof in any medium, with or without
       modifications, and in Source or Object form, provided that You
       meet the following conditions:

       (a) You must give any other recipients of the Work or
           Derivative Works a copy of this License; and

       (b) You must cause any modified files to carry prominent notices
           stating that You changed the files; and

       (c) You must retain, in the Source form of any Derivative Works
           that You distribute, all copyright, patent, trademark, and
           attribution notices from the Source form of the Work,
           excluding those notices that do not pertain to any part of
           the Derivative Works; and

       (d) If the Work includes a "NOTICE" text file as part of its
           distribution, then any Derivative Works that You distribute must
           include a readable copy of the attribution notices contained
           within such NOTICE file, excluding those notices that do not
           pertain to any part of the Derivative Works, in at least one
           of the following places: within a NOTICE text file distributed
           as part of the Derivative Works; within the Source form or
           documentation, if provided along with the Derivative Works; or,
           within a display generated by the Derivative Works, if and
           wherever such third-party notices normally appear. The contents
           of the NOTICE file are for informational purposes only and
           do not modify the License. You may add Your own attribution
           notices within Derivative Works that You distribute, alongside
           or as an addendum to the NOTICE text from the Work, provided
           that such additional attribution notices cannot be construed
           as modifying the License.

       You may add Your own copyright statement to Your modifications and
       may provide additional or different license terms and conditions
       for use, reproduction, or distribution of Your modifications, or
       for any such Derivative Works as a whole, provided Your use,
       reproduction, and distribution of the Work otherwise complies with
       the conditions stated in this License.

    5. Submission of Contributions. Unless You explicitly state otherwise,
       any Contribution intentionally submitted for inclusion in the Work
       by You to the Licensor shall be under the terms and conditions of
       this License, without any additional terms or conditions.
       Notwithstanding the above, nothing herein shall supersede or modify
       the terms of any separate license agreement you may have executed
       with Licensor regarding such Contributions.

    6. Trademarks. This License does not grant permission to use the trade
       names, trademarks, service marks, or product names of the Licensor,
       except as required for reasonable and customary use in describing the
       origin of the Work and reproducing the content of the NOTICE file.

    7. Disclaimer of Warranty. Unless required by applicable law or
       agreed to in writing, Licensor provides the Work (and each
       Contributor provides its Contributions) on an "AS IS" BASIS,
       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
       implied, including, without limitation, any warranties or conditions
       of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
       PARTICULAR PURPOSE. You are solely responsible for determining the
       appropriateness of using or redistributing the Work and assume any
       risks associated with Your exercise of permissions under this License.

    8. Limitation of Liability. In no event and under no legal theory,
       whether in tort (including negligence), contract, or otherwise,
       unless required by applicable law (such as deliberate and grossly
       negligent acts) or agreed to in writing, shall any Contributor be
       liable to You for damages, including any direct, indirect, special,
       incidental, or consequential damages of any character arising as a
       result of this License or out of the use or inability to use the
       Work (including but not limited to damages for loss of goodwill,
       work stoppage, computer failure or malfunction, or any and all
       other commercial damages or losses), even if such Contributor
       has been advised of the possibility of such damages.

    9. Accepting Warranty or Additional Liability. While redistributing
       the Work or Derivative Works thereof, You may choose to offer,
       and charge a fee for, acceptance of support, warranty, indemnity,
       or other liability obligations and/or rights consistent with this
       License. However, in accepting such obligations, You may act only
       on Your own behalf and on Your sole responsibility, not on behalf
       of any other Contributor, and only if You agree to indemnify,
       defend, and hold each Contributor harmless for any liability
       incurred by, or claims asserted against, such Contributor by reason
       of your accepting any such warranty or additional liability.
)more"
R"more(


    gRPC - An RPC library and framework
    --------------------

    Copyright 2014 gRPC authors.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

                                 Apache License
                           Version 2.0, January 2004
                        https://www.apache.org/licenses/

    TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

    1. Definitions.

       "License" shall mean the terms and conditions for use, reproduction,
       and distribution as defined by Sections 1 through 9 of this document.

       "Licensor" shall mean the copyright owner or entity authorized by
       the copyright owner that is granting the License.

       "Legal Entity" shall mean the union of the acting entity and all
       other entities that control, are controlled by, or are under common
       control with that entity. For the purposes of this definition,
       "control" means (i) the power, direct or indirect, to cause the
       direction or management of such entity, whether by contract or
       otherwise, or (ii) ownership of fifty percent (50%) or more of the
       outstanding shares, or (iii) beneficial ownership of such entity.

       "You" (or "Your") shall mean an individual or Legal Entity
       exercising permissions granted by this License.

       "Source" form shall mean the preferred form for making modifications,
       including but not limited to software source code, documentation
       source, and configuration files.

       "Object" form shall mean any form resulting from mechanical
       transformation or translation of a Source form, including but
       not limited to compiled object code, generated documentation,
       and conversions to other media types.

       "Work" shall mean the work of authorship, whether in Source or
       Object form, made available under the License, as indicated by a
       copyright notice that is included in or attached to the work
       (an example is provided in the Appendix below).

       "Derivative Works" shall mean any work, whether in Source or Object
       form, that is based on (or derived from) the Work and for which the
       editorial revisions, annotations, elaborations, or other modifications
       represent, as a whole, an original work of authorship. For the purposes
       of this License, Derivative Works shall not include works that remain
       separable from, or merely link (or bind by name) to the interfaces of,
       the Work and Derivative Works thereof.

       "Contribution" shall mean any work of authorship, including
       the original version of the Work and any modifications or additions
       to that Work or Derivative Works thereof, that is intentionally
       submitted to Licensor for inclusion in the Work by the copyright owner
       or by an individual or Legal Entity authorized to submit on behalf of
       the copyright owner. For the purposes of this definition, "submitted"
       means any form of electronic, verbal, or written communication sent
       to the Licensor or its representatives, including but not limited to
       communication on electronic mailing lists, source code control systems,
       and issue tracking systems that are managed by, or on behalf of, the
       Licensor for the purpose of discussing and improving the Work, but
       excluding communication that is conspicuously marked or otherwise
       designated in writing by the copyright owner as "Not a Contribution."

       "Contributor" shall mean Licensor and any individual or Legal Entity
       on behalf of whom a Contribution has been received by Licensor and
       subsequently incorporated within the Work.

    2. Grant of Copyright License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       copyright license to reproduce, prepare Derivative Works of,
       publicly display, publicly perform, sublicense, and distribute the
       Work and such Derivative Works in Source or Object form.

    3. Grant of Patent License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       (except as stated in this section) patent license to make, have made,
       use, offer to sell, sell, import, and otherwise transfer the Work,
       where such license applies only to those patent claims licensable
       by such Contributor that are necessarily infringed by their
       Contribution(s) alone or by combination of their Contribution(s)
       with the Work to which such Contribution(s) was submitted. If You
       institute patent litigation against any entity (including a
       cross-claim or counterclaim in a lawsuit) alleging that the Work
       or a Contribution incorporated within the Work constitutes direct
       or contributory patent infringement, then any patent licenses
       granted to You under this License for that Work shall terminate
       as of the date such litigation is filed.

    4. Redistribution. You may reproduce and distribute copies of the
       Work or Derivative Works thereof in any medium, with or without
       modifications, and in Source or Object form, provided that You
       meet the following conditions:

       (a) You must give any other recipients of the Work or
           Derivative Works a copy of this License; and

       (b) You must cause any modified files to carry prominent notices
           stating that You changed the files; and

       (c) You must retain, in the Source form of any Derivative Works
           that You distribute, all copyright, patent, trademark, and
           attribution notices from the Source form of the Work,
           excluding those notices that do not pertain to any part of
           the Derivative Works; and

       (d) If the Work includes a "NOTICE" text file as part of its
           distribution, then any Derivative Works that You distribute must
           include a readable copy of the attribution notices contained
           within such NOTICE file, excluding those notices that do not
           pertain to any part of the Derivative Works, in at least one
           of the following places: within a NOTICE text file distributed
           as part of the Derivative Works; within the Source form or
           documentation, if provided along with the Derivative Works; or,
           within a display generated by the Derivative Works, if and
           wherever such third-party notices normally appear. The contents
           of the NOTICE file are for informational purposes only and
           do not modify the License. You may add Your own attribution
           notices within Derivative Works that You distribute, alongside
           or as an addendum to the NOTICE text from the Work, provided
           that such additional attribution notices cannot be construed
           as modifying the License.

       You may add Your own copyright statement to Your modifications and
       may provide additional or different license terms and conditions
       for use, reproduction, or distribution of Your modifications, or
       for any such Derivative Works as a whole, provided Your use,
       reproduction, and distribution of the Work otherwise complies with
       the conditions stated in this License.

    5. Submission of Contributions. Unless You explicitly state otherwise,
       any Contribution intentionally submitted for inclusion in the Work
       by You to the Licensor shall be under the terms and conditions of
       this License, without any additional terms or conditions.
       Notwithstanding the above, nothing herein shall supersede or modify
       the terms of any separate license agreement you may have executed
       with Licensor regarding such Contributions.

    6. Trademarks. This License does not grant permission to use the trade
       names, trademarks, service marks, or product names of the Licensor,
       except as required for reasonable and customary use in describing the
       origin of the Work and reproducing the content of the NOTICE file.

    7. Disclaimer of Warranty. Unless required by applicable law or
       agreed to in writing, Licensor provides the Work (and each
       Contributor provides its Contributions) on an "AS IS" BASIS,
       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
       implied, including, without limitation, any warranties or conditions
       of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
       PARTICULAR PURPOSE. You are solely responsible for determining the
       appropriateness of using or redistributing the Work and assume any
       risks associated with Your exercise of permissions under this License.

    8. Limitation of Liability. In no event and under no legal theory,
       whether in tort (including negligence), contract, or otherwise,
       unless required by applicable law (such as deliberate and grossly
       negligent acts) or agreed to in writing, shall any Contributor be
       liable to You for damages, including any direct, indirect, special,
       incidental, or consequential damages of any character arising as a
       result of this License or out of the use or inability to use the
       Work (including but not limited to damages for loss of goodwill,
       work stoppage, computer failure or malfunction, or any and all
       other commercial damages or losses), even if such Contributor
       has been advised of the possibility of such damages.

    9. Accepting Warranty or Additional Liability. While redistributing
       the Work or Derivative Works thereof, You may choose to offer,
       and charge a fee for, acceptance of support, warranty, indemnity,
       or other liability obligations and/or rights consistent with this
       License. However, in accepting such obligations, You may act only
       on Your own behalf and on Your sole responsibility, not on behalf
       of any other Contributor, and only if You agree to indemnify,
       defend, and hold each Contributor harmless for any liability
       incurred by, or claims asserted against, such Contributor by reason
       of your accepting any such warranty or additional liability.
)more"
R"more(


    Protocol Buffers - Google's data interchange format
    --------------------

    Copyright 2008 Google Inc.  All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer in the
          documentation and/or other materials provided with the distribution.
        * Neither the name of Google Inc. nor the names of its contributors
          may be used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Code generated by the Protocol Buffer compiler is owned by the owner
    of the input file used when generating it.  This code is not
    standalone and requires a support library to be linked with it.  This
    support library is itself covered by the above license.
)more"
R"more(


    C-ARES
    --------------------

    MIT License

    Copyright (c) 1998 Massachusetts Institute of Technology
    Copyright (c) 2007 - 2023 Daniel Stenberg with many contributors,
    see AUTHORS file.

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice (including the next
    paragraph) shall be included in all copies or substantial portions of the
    Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
)more"
R"more(


    RE2, a regular expression library
    --------------------

    Copyright (c) 2009 The RE2 Authors. All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
          notice, this list of conditions and the following disclaimer
          in the documentation and/or other materials provided with the
          distribution.
        * Neither the name of Google Inc. nor the names of its contributors
          may be used to endorse or promote products derived from this software
          without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
)more"
R"more(


    Abseil - C++ Common Libraries
    --------------------

    Copyright The Abseil Authors

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

                                 Apache License
                           Version 2.0, January 2004
                        https://www.apache.org/licenses/

    TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

    1. Definitions.

       "License" shall mean the terms and conditions for use, reproduction,
       and distribution as defined by Sections 1 through 9 of this document.

       "Licensor" shall mean the copyright owner or entity authorized by
       the copyright owner that is granting the License.

       "Legal Entity" shall mean the union of the acting entity and all
       other entities that control, are controlled by, or are under common
       control with that entity. For the purposes of this definition,
       "control" means (i) the power, direct or indirect, to cause the
       direction or management of such entity, whether by contract or
       otherwise, or (ii) ownership of fifty percent (50%) or more of the
       outstanding shares, or (iii) beneficial ownership of such entity.

       "You" (or "Your") shall mean an individual or Legal Entity
       exercising permissions granted by this License.

       "Source" form shall mean the preferred form for making modifications,
       including but not limited to software source code, documentation
       source, and configuration files.

       "Object" form shall mean any form resulting from mechanical
       transformation or translation of a Source form, including but
       not limited to compiled object code, generated documentation,
       and conversions to other media types.

       "Work" shall mean the work of authorship, whether in Source or
       Object form, made available under the License, as indicated by a
       copyright notice that is included in or attached to the work
       (an example is provided in the Appendix below).

       "Derivative Works" shall mean any work, whether in Source or Object
       form, that is based on (or derived from) the Work and for which the
       editorial revisions, annotations, elaborations, or other modifications
       represent, as a whole, an original work of authorship. For the purposes
       of this License, Derivative Works shall not include works that remain
       separable from, or merely link (or bind by name) to the interfaces of,
       the Work and Derivative Works thereof.

       "Contribution" shall mean any work of authorship, including
       the original version of the Work and any modifications or additions
       to that Work or Derivative Works thereof, that is intentionally
       submitted to Licensor for inclusion in the Work by the copyright owner
       or by an individual or Legal Entity authorized to submit on behalf of
       the copyright owner. For the purposes of this definition, "submitted"
       means any form of electronic, verbal, or written communication sent
       to the Licensor or its representatives, including but not limited to
       communication on electronic mailing lists, source code control systems,
       and issue tracking systems that are managed by, or on behalf of, the
       Licensor for the purpose of discussing and improving the Work, but
       excluding communication that is conspicuously marked or otherwise
       designated in writing by the copyright owner as "Not a Contribution."

       "Contributor" shall mean Licensor and any individual or Legal Entity
       on behalf of whom a Contribution has been received by Licensor and
       subsequently incorporated within the Work.

    2. Grant of Copyright License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       copyright license to reproduce, prepare Derivative Works of,
       publicly display, publicly perform, sublicense, and distribute the
       Work and such Derivative Works in Source or Object form.

    3. Grant of Patent License. Subject to the terms and conditions of
       this License, each Contributor hereby grants to You a perpetual,
       worldwide, non-exclusive, no-charge, royalty-free, irrevocable
       (except as stated in this section) patent license to make, have made,
       use, offer to sell, sell, import, and otherwise transfer the Work,
       where such license applies only to those patent claims licensable
       by such Contributor that are necessarily infringed by their
       Contribution(s) alone or by combination of their Contribution(s)
       with the Work to which such Contribution(s) was submitted. If You
       institute patent litigation against any entity (including a
       cross-claim or counterclaim in a lawsuit) alleging that the Work
       or a Contribution incorporated within the Work constitutes direct
       or contributory patent infringement, then any patent licenses
       granted to You under this License for that Work shall terminate
       as of the date such litigation is filed.

    4. Redistribution. You may reproduce and distribute copies of the
       Work or Derivative Works thereof in any medium, with or without
       modifications, and in Source or Object form, provided that You
       meet the following conditions:

       (a) You must give any other recipients of the Work or
           Derivative Works a copy of this License; and

       (b) You must cause any modified files to carry prominent notices
           stating that You changed the files; and

       (c) You must retain, in the Source form of any Derivative Works
           that You distribute, all copyright, patent, trademark, and
           attribution notices from the Source form of the Work,
           excluding those notices that do not pertain to any part of
           the Derivative Works; and

       (d) If the Work includes a "NOTICE" text file as part of its
           distribution, then any Derivative Works that You distribute must
           include a readable copy of the attribution notices contained
           within such NOTICE file, excluding those notices that do not
           pertain to any part of the Derivative Works, in at least one
           of the following places: within a NOTICE text file distributed
           as part of the Derivative Works; within the Source form or
           documentation, if provided along with the Derivative Works; or,
           within a display generated by the Derivative Works, if and
           wherever such third-party notices normally appear. The contents
           of the NOTICE file are for informational purposes only and
           do not modify the License. You may add Your own attribution
           notices within Derivative Works that You distribute, alongside
           or as an addendum to the NOTICE text from the Work, provided
           that such additional attribution notices cannot be construed
           as modifying the License.

       You may add Your own copyright statement to Your modifications and
       may provide additional or different license terms and conditions
       for use, reproduction, or distribution of Your modifications, or
       for any such Derivative Works as a whole, provided Your use,
       reproduction, and distribution of the Work otherwise complies with
       the conditions stated in this License.

    5. Submission of Contributions. Unless You explicitly state otherwise,
       any Contribution intentionally submitted for inclusion in the Work
       by You to the Licensor shall be under the terms and conditions of
       this License, without any additional terms or conditions.
       Notwithstanding the above, nothing herein shall supersede or modify
       the terms of any separate license agreement you may have executed
       with Licensor regarding such Contributions.

    6. Trademarks. This License does not grant permission to use the trade
       names, trademarks, service marks, or product names of the Licensor,
       except as required for reasonable and customary use in describing the
       origin of the Work and reproducing the content of the NOTICE file.

    7. Disclaimer of Warranty. Unless required by applicable law or
       agreed to in writing, Licensor provides the Work (and each
       Contributor provides its Contributions) on an "AS IS" BASIS,
       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
       implied, including, without limitation, any warranties or conditions
       of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
       PARTICULAR PURPOSE. You are solely responsible for determining the
       appropriateness of using or redistributing the Work and assume any
       risks associated with Your exercise of permissions under this License.

    8. Limitation of Liability. In no event and under no legal theory,
       whether in tort (including negligence), contract, or otherwise,
       unless required by applicable law (such as deliberate and grossly
       negligent acts) or agreed to in writing, shall any Contributor be
       liable to You for damages, including any direct, indirect, special,
       incidental, or consequential damages of any character arising as a
       result of this License or out of the use or inability to use the
       Work (including but not limited to damages for loss of goodwill,
       work stoppage, computer failure or malfunction, or any and all
       other commercial damages or losses), even if such Contributor
       has been advised of the possibility of such damages.

    9. Accepting Warranty or Additional Liability. While redistributing
       the Work or Derivative Works thereof, You may choose to offer,
       and charge a fee for, acceptance of support, warranty, indemnity,
       or other liability obligations and/or rights consistent with this
       License. However, in accepting such obligations, You may act only
       on Your own behalf and on Your sole responsibility, not on behalf
       of any other Contributor, and only if You agree to indemnify,
       defend, and hold each Contributor harmless for any liability
       incurred by, or claims asserted against, such Contributor by reason
       of your accepting any such warranty or additional liability.
)more"
# endif // USE_OTEL
R"more(


    mINI - An INI file reader and writer for the modern age
    --------------------

    The MIT License (MIT)
    Copyright (c) 2018 Danijel Durakovic

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
)more"
# ifdef HAS_RUST
//Adding the Rust Legal
# include "rustlegal.cc"
# else
;
# endif // end HAS_RUST_

ErrorId MsgHelp::HelpLegal = { ErrorOf( ES_HELP, 151, E_INFO, EV_NONE, 0 ),
	legalText
};

ErrorId MsgHelp::HelpLicense = { ErrorOf( ES_HELP, 101, E_INFO, EV_NONE, 0 ),
R"(
    license -- Update or display the license file

    p4 license -o
    p4 license -i
    p4 license -u
    p4 license -L

	Update the P4 license file.  This command requires a valid license file
	in the P4 root directory.  Typically this command lets an administrator
	add extra licensed users to the P4 server without having to shut the
	server down and copy the license file to the server root.

	Most new license files obtained from P4 can be installed with this
	command, unless the server's IP address or port has changed.
	In that case, stop the server, copy the new license file to the root,
	and restart the server.

	The -o flag writes the license file to the standard output.

	The -i flag reads a license file from the standard input.

	The -u flag reports the license limits and how many entities are in
	use towards the limits.

	The -L flag lists valid server IP and MAC addresses to be used when
	requesting a valid license from Perforce Support.

	This command requires 'super' access (or 'admin' for '-u'),
	which is granted by 'p4 protect'.

	**********************************************************************
	When using the free version of the server (no license file) the server
	is limited to 5 users and 20 workspaces, or unlimited users and
	workspaces when the repository has less than 1,000 files
	**********************************************************************
)"
};

ErrorId MsgHelp::HelpList = { ErrorOf( ES_HELP, 147, E_INFO, EV_NONE, 0 ),
R"(
    list -- Create a temporary list of files that can be used as a label

    p4 list [ -l label ] [ -C ] [ -M ] file[revRange] ...
    p4 list -l label -d [ -M ]

	List builds an in-memory temporary list of files that can be used
	as a label in any command that can take a label. The list only lasts
	as long as the connected session, for example a temporary list created
	in one 'p4' command line request will not be available to a subsequent
	'p4' command that references the label.

	By default, the head revision is listed.  If the file argument
	specifies a revision, then all files at that revision are listed.
	If the file argument specifies a revision range, the highest revision
	in the range is used for each file. For details about specifying
	revisions, see 'p4 help revisions'.

	The -l flag specifies the name of the temporary label that should be
	used to access the list.  In the event that a label with that name
	already exists, the in-memory name has precedence over the stored one.
	Without the '-l' option the list command assigns a unique name to the
	temporary list and returns the name as output.

	The -d flag specifies that the named list should be deleted, freeing
	the memory used by that list.

	The -C flag limits any depot paths to those that can be mapped through
	the client workspace.

	The -M flag can be specified when issuing the list command against a
	forwarding replica; in this case, it indicates that the list command
	should not be processed by the replica, but should instead be
	forwarded to the master server.
)"
};

ErrorId MsgHelp::HelpLock = { ErrorOf( ES_HELP, 62, E_INFO, EV_NONE, 0 ),
R"(
    lock -- Lock an open file to prevent it from being submitted

    p4 lock [-c changelist#] [-g] [file ...]

	The specified files are locked in the depot, preventing any user
	other than the current user on the current client from submitting
	changes to the files.  If a file is already locked, the lock request
	is rejected.

	The -c flag and the file specification limit the files to be locked.
	If no file names are specified and no changelist is specified, all
	open files are locked.

	The -g flag is used to lock the files locally and globally at the
	Commit Server in a distributed environment. It may only be run from
	an Edge Server. When used to pre-lock files prior to submit, files
	should be opened in a numbered change in order to improve submit
	performance. These locks are removed by the 'unlock', 'revert',
	or any 'submit' command that operates on these files. Note that
	global locks will not be taken if any of the opened files specified
	in the command could not be opened locally.

	If a changelist is specified, only those files open in that
	changelist are locked.

	If a file specification is given, only the matching files are locked.

	If both the -c flag and a file specification are provided, only the
	matching files in the specified changelist are locked.

	See 'p4 help-graph lock' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpLogger = { ErrorOf( ES_HELP, 63, E_INFO, EV_NONE, 0 ),
R"(
    logger -- Report changed jobs and changelists

    p4 logger [-c sequence#] [-t counter]

	Dumps the event log, which notes updates to changes and jobs, for
	use with defect tracking integration.  The event log is enabled
	by setting the counter 'logger' to 0 using the 'p4 counter' command.
	Each event is assigned a sequence number.  The presence of an entry
	in the log doesn't guarantee that the named entity has changed.

	If a sequence# is specified using the -c flag, only events after that
	number are listed. If a counter is given using the -t flag, only events
	after the number of that counter are listed.  If both are specified,
	the counter is updated to the sequence number and nothing is output.
	If the update brings the counter to the highest sequence number in
	the log, the log is cleared.

	Important: 'p4 logger' is not intended for end users. It is used by
	P4DTG to help propagate information to an external defect tracking
	system.

	'p4 logger -c' requires 'review' access, which is granted by 'p4
	protect'.

	In a distributed installation, 'p4 logger' commands should be issued
	to the Commit Server, not to an Edge Server.
)"
};

ErrorId MsgHelp::HelpLogin = { ErrorOf( ES_HELP, 89, E_INFO, EV_NONE, 0 ),
R"(
    login -- Log in to P4 by obtaining a session ticket

    p4 login [-s | -p] [-a | -h host] [user]
    p4 login [-s -a] -r <remotespec> [--remote-user=X]
    p4 login [-p -a] -r <remotespec> [--remote-user=X]

	The login command enables a user to access P4 until the session expires
	or the user logs out.

	When a user logs in to P4, they are prompted for a password.  If they
	enter the correct password, they are issued a ticket.  The ticket
	expires when the default timeout value has been reached and is valid
	only for the host machine where the 'login' command was executed
	(see below for exception).

	The ticket can be used anywhere that a password can be used.

	Example: p4 -P <ticket value> changes -m1

	The -a flag causes the server to issue a ticket that is valid on all
	host machines. When used with -s, displays ticket status for all hosts.

	The -h flag causes the server to issue a ticket that is valid on the
	specified host (IP address).  This flag is typically used together
	with the -p flag to acquire a ticket that can be used on a different
	machine.

	The -p flag displays the ticket, but does not store it on the client
	machine.

	The -r flag causes the server to forward the login to the server
	referenced in the specified remote specification. The login will be for
	the user specified by the --remote-user flag, or if RemoteUser is set
	in the remote specification, the login will be for that user.
	Specifying a host or a username is not allowed when logging into a
	remote server.

	The -s flag displays the status of the current ticket (if there is
	one). Use with -a to display status for all hosts, or '-h host' to
	display status for a specfic host. Users with 'super' access can
	provide a username argument.

	Specifying a username as an argument to 'p4 login' requires 'super'
	access, which is granted by 'p4 protect'.  In this case, 'p4 login'
	does not prompt for the password (you must already be logged in).
)"
};

//// Comments removed to address job108420/P4-21425
//// From command syntax section:
//
//    p4 login2 [-p] -r <remotespec> [--remote-user=X]
//    p4 login2 [-s -a ] -r <remotespec> [--remote-user=X]
//
//// 4th paragraph from help detail:
//
//	The -r flag causes the server to forward the multi factor
//	authentication to the server referenced in the specified remote
//	specification.  The authentication will be for the user specified by
//	the --remote-user flag, or if RemoteUser is set in the remote
//	specification, the login will be for that user.  Specifying a host or
//	a username is not allowed when logging into a remote server.

ErrorId MsgHelp::HelpLogin2 = { ErrorOf( ES_HELP, 230, E_INFO, EV_NONE, 0 ),
R"(
    login2 -- Perform multi factor authentication

    p4 login2 [ -p -R ] [ -h host ] [ -S state ] [ -m method ] [ username ]
    p4 login2 -s [ -a | -h host ] [ username ]

	The login2 command enables a user requiring multi factor
	authentication to authorize access on a given host.

	The -p flag causes the multi factor authorization to persist even
	after the user's ticket has expired.  The authorization status can
	still be invalidated by running 'p4 logout'.

	The -R flag causes the multi factor authentication to be restarted
	allowing users to re-request a OTP, etc.

	The -s flag displays the multi factor authorization status for the
	user on the current host, or all hosts that the user has used if the
	-a flag is used.  To show the status for a specific host, the IP
	address  can be specified with the -h flag.

	Specifying a username as an argument to 'p4 login2' requires 'super'
	access, which is granted by 'p4 protect'.  In this case, 'p4 login2'
	skips the multi factor authentication process and immediately marks
	the user as validated for the current host (the super user must already
	be logged in and verified if necessary).  A host (IP address) can be
	specified with the -h flag to validate the user on a different host.

	For non-interactive clients, the -S flag can be used to execute each
	step of the multi factor authentication individually.  This must begin
	with the 'list-methods' state, which will report the list of available
	multi factor authentication methods for the given user.  The next
	state must be 'init-auth', and must be accompanied by the chose method
	provided to the -m flag: this will initiate the authentication with the
	multi factor authentication provider.  The final step is 'check-auth',
	which will either prompt for a OTP or request the authorization status
	from the multi factor authentication provider, depending on the type
	of authentication method selected.  The -p flag may be provided at the
	'init-auth' stage.  If a host and/or user is being specified, the
	appropriate arguments must be provided at each stage.
)"
};

ErrorId MsgHelp::HelpLogout = { ErrorOf( ES_HELP, 90, E_INFO, EV_NONE, 0 ),
R"(
    logout -- Log out from P4 by removing or invalidating a ticket.

    p4 logout [ -a | -h host ] [ -2 ] [ username ]

	The logout command removes the ticket on the client and invalidates the
	per-host ticket value on the server.  To resume using P4, the user must
	log in again.

	If you are logged in to P4 from more than one machine, you can log out
	of P4 from all machines from which you were logged in by specifying the
	-a flag.  The -a flag invalidates the ticket on the server.  All of
	your P4 tickets are invalidated and you are logged out.  To invalidate
	the ticket for a specific host, you may specify the IP address of that
	host with the -h flag.

	The -2 flag causes only the user's multi factor authentication state
	to be invalidated.

	Note:
	  Only 'p4 logout -a' will invalidate tickets created by 'p4 login -a'.

	Specifying a username as an argument to 'p4 logout' requires 'super'
	access, which is granted by 'p4 protect'.  Note that either the '-a'
	or '-h' flags must be used to effectively log out a user who has a
	valid ticket on another machine.
)"
};

ErrorId MsgHelp::HelpMerge = { ErrorOf( ES_HELP, 134, E_INFO, EV_NONE, 0 ),
R"(
    merge -- Merge one set of files into another set of files, and/or merge
	one stream spec into another stream spec

    p4 merge [options] [-F] [--from stream] [toFile][revRange]
    p4 merge [options] fromFile[revRange] toFile
    p4 merge [options] [-As | -Af] -S stream [-P parent] [-r]
	               [toFile[rev] ...]

	options: -c changelist# -F -K -m max -n -Ob -q -v

	'p4 merge' merges changes from one set of files (the 'source') into
	another (the 'target'). It is a simplified form of the 'p4 integrate'
	command, similar to 'p4 integrate -Rbd -Or'.

	Using the client workspace as a staging area, 'p4 merge' schedules all
	affected target files to be resolved per changes in the source.
	Target files outside of	the current client view are not affected.
	Source files need not be within the client view.

	'p4 resolve' must be used to resolve all changes.  'p4 submit' commits
	merged files to	the depot.  Unresolved files may not be submitted.
	Merged files can be shelved with 'p4 shelve' and abandoned with
	'p4 revert'.  The commands 'p4 integrated' and 'p4 filelog' display
	merge history.

	When 'p4 merge' schedules a workspace file to be resolved, it leaves
	it read-only. 'p4 resolve' can operate on a read-only file;  for
	other pre-submit changes, 'p4 edit' must be used to make the file
	writable.

	By default, 'p4 merge' merges changes into the current stream from its
	parent, or from another stream specified by the --from flag.
	
	If the current client is not associated with a stream, the source and
	target can be specified on the command line as a pair of file paths.
	More complex merge mappings can be specified via branchspecs as with
	'p4 integrate' (see 'p4 help integrate').

	Each file in the target is mapped to a file in the source. Mapping
	adjusts automatically for files that have been moved or renamed, as
	long as 'p4 move' was used to move/rename files.  The scope of source
	and target file sets must include both old-named and new-named files
	for mappings to be adjusted.  Moved source files may schedule moves
	to be resolved in target files.

	revRange is a revision or a revision range that limits the span of
	source history to be probed for unintegrated revisions.  For details
	about revision specifiers, see 'p4 help revisions'.

	The typical use of 'p4 merge' is to merge into the current stream
	from its parent stream (or the stream specified by --from). More
	advanced uses of 'p4 merge' are possible, using the flags as described
	by 'p4 help integ'.

	The -F flag can be used to force merging against a stream's expected
	flow, or to use an arbitrarily specified view to merge into a stream.

	Stream specifications may also be merged along with stream files.
	Any field in the source specification which has an Openable property of
	'propagate' will be merged to the target specification. The -Af and
	-As flags control whether stream files or the stream spec is merged.

		-Af	Perform merge with files only.
		-As	Perform merge with stream specs only.

	Both stream files and the stream spec will be merged if neither -Af
	nor -As is specified.  See 'p4 help streamspecinteg' for more details.

	The -Ob flag causes the base revision (if any) to be displayed along
	with each scheduled resolve.

	The -K flag suppresses keyword expansion when updating ktext files on
	the client.

	The -q flag suppresses normal output messages. Messages regarding
	errors or exceptional conditions are displayed.

	The -v flag causes a 'virtual' integration that does not modify
	client workspace files unless target files need to be resolved.
	After submitting a virtual integration, 'p4 sync' can be used to
	update the workspace.

	See 'p4 help-graph merge' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpMerge3 = { ErrorOf( ES_HELP, 98, E_INFO, EV_NONE, 0 ),
R"(
    merge3 -- three-way file merge (unsupported)

    p4 merge3 [-db -dw -dl -r -t -v] base leg1 leg2

	Performs a three-way merge of the specified files and writes the
	results to standard output.  This command is implemented on the
	client and requires file names to be specified using client syntax.
	This command is used to support P4's graphical merge tools.

	The -db, -dw, -dl, -t, and -v flags are the same as in 'p4 resolve'.

	-r formats the output from a special one to one more like
	'p4 resolve'.
)"
};

ErrorId MsgHelp::HelpMonitor = { ErrorOf( ES_HELP, 88, E_INFO, EV_NONE, 0 ),
R"(
    monitor -- Display P4 process information

    p4 monitor show [-a -l -e -L -s R|T|P|B|F|I ]
    p4 monitor terminate id
    p4 monitor clear {id | all}
    p4 monitor pause id
    p4 monitor resume id
    p4 monitor realtime [ -F -T ]

	Monitor displays running p4 processes. Monitor tracks the P4 processes
	using a dedicated table. This table is constantly updated, so there is
	a potential minor impact on server performance. To enable this command,
	set the monitor configurable as follows:

	    To monitor active commands, set the configurable to 1.

	    To monitor idle connections and active commands, set the
	    configurable to 2.

	    To monitor idle connections stuck at Init(), idle connections
	    and active commands, set the configurable to 3.

	    To monitor idle connections and active commands, including a list
	    of the tables locked by the command for more than 1 second, set
	    the configurable to 5.

	    To monitor idle connections and active commands, including a list
	    of the tables locked by the command for more than 1 second, with
	    lock wait times included in the lock information, set the
	    configurable to 10.

	    To monitor idle connections and active commands, including a list
	    of the tables locked by the command for any duration, with
	    lock wait times included in the lock information, set the
	    configurable to 25.

	Changes to the monitor configurable affect all new p4 processes that
	connect to the server; restarting the server is required unless the
	db.monitor.shared configurable is set to zero (0).

	'p4 monitor show' displays current process information. For each
	process, a single line of output lists the process ID, status,
	owner, elapsed time in HH:MM:SS format, and command. The status
	values are: [R]unning, [T]erminated, [P]aused, [B]ackground,
	[F]inish, or [I]dle. The Background and Finish statuses occur only in
	replica servers.

	The following flags require that the user be an operator or have
	'super' access:

	    The -a flag includes the command args.

	    The -e flag includes the command environment. For each process,
	    client application (if known), host address and client name are
	    displayed.

	    The -l flag displays long output, including the full username and
	    argument list.

	    The -L flag includes file locking information, and requires that
	    either the monitor configurable has been set as described above,
	    or the monitor.lsof configurable has been set to enable the
	    display. The monitor.lsof configurable can be used with a Linux
	    server, while setting the monitor configurable to 5 or more works
	    for any server platform. However, there is no value to setting
	    both configurables; either set monitor.lsof, or set monitor to
	    5 or more, but not both.

	    The -s flag displays only processes in the indicated status.

	'p4 monitor terminate id' marks the specified command for
	termination. This command requires that the user be an operator,
	have 'super' access or be the owner of the process id.

	'p4 monitor clear {id|all}' removes the specified process record from
	the monitor table. All of the records are removed if the option 'all'
	is supplied instead of an ID.  This option might be required if for
	some reason commands are ending prematurely. This command requires
	that the user be an operator or have 'super' access.

	'p4 monitor pause id' marks the specified command for
	pausing until resumed. This command requires that the user be
	an operator or have 'super' access.

	'p4 monitor resume id' marks the specified command for
	resuming. This command requires that the user be an operator or have
	'super' access.

	'p4 monitor realtime [ -F -T ]' outputs realtime performance counters
	when enabled by setting the 'rt.monitorfile' configurable. This command
	requires that the user be an operator or have 'super' access.
	See 'p4 help realtime' for realtime performance counter details.

	    The -F flag lists only performance counters satisfying the filter
	    expression. This filter syntax is similar to the one used for
	    'jobs -e jobview' and is used to evaluate the contents of the
	    fields in the preceding list. Filtering is case-sensitive.

	        Example: -F "name=rtv.svr.sessions*"

	    The -T fields flag returns only the specified fields. The field
	    names can be specified using a comma- or space-delimited list.

	        Example: -T "name,value"
)"
};

ErrorId MsgHelp::HelpMove = { ErrorOf( ES_HELP, 108, E_INFO, EV_NONE, 0 ),
R"(
    move -- move file(s) from one location to another
    rename -- synonym for 'move'

    p4 move [-c changelist#] [-f -n -k] [-t filetype] fromFile toFile
    p4 move -r [-c changelist#] [-n -k] fromFile toFile
    p4 move -M [-c changelist#] [-n] [--parallel=N]

	Move by default takes an already opened file and moves it from one
	client location to another, reopening it as a pending depot move.
	When the file is submitted with 'p4 submit', its depot file is moved
	accordingly. The 'p4 move' command is intended for moves that occur
	within the same branch and not for restructuring the depot by moving
	an entire branch.

	Wildcards in fromFile and toFile must match. The fromFile must be
	a file open for add or edit unless invoked with '-r'.

	'p4 opened' lists pending moves. 'p4 diff' can compare a moved
	client file with its depot original, 'p4 sync' can schedule an
	update of a moved file, and 'p4 resolve' can resolve the update.

	A client file can be moved many times before it is submitted.
	Moving a file back to its original location will undo a pending
	move, leaving unsubmitted content intact.  Using 'p4 revert'
	undoes the move and reverts the unsubmitted content.

	If -c changelist# is specified, the file is reopened in the
	specified pending changelist as well as being moved.

	The -f flag forces a move to an existing target file. The file
	must be synced and not opened.  The originating source file will
	no longer be synced to the client.

	If -t filetype is specified, the file is assigned that filetype.
	If the filetype is a partial filetype, the partial filetype is
	combined with the current filetype.  See 'p4 help filetypes'.

	The -n flag previews the operation without moving files.

	The -k flag performs the rename on the server without modifying
	client files. Use with caution, as an incorrect move can cause
	discrepancies between the state of the client and the corresponding
	server metadata.

	The -r flag may be used to rename existing files without altering
	content or type to a new target. This option allows the user to
	move files from the depot to a different location without opening
	the files first. Since this option performs a strict rename only,
	the '-t' and '-f' flags are not allowed, source files must not be
	already opened, and the targets must not already exist.

	The -M flag matches already opened files for add and delete in the
	default or specified changelist and reopens them as a move.

	The --parallel flag when used with the -M flag specifies the number
	of files opened for add to be compared simultaneously on the client
	to the file opened for delete to improve the performance of the move
	matching.

	The 'move' command requires a release 2009.1 or newer client. The
	'-f' flag requires a 2010.1 client.
)"
};

ErrorId MsgHelp::HelpObliterate = { ErrorOf( ES_HELP, 64, E_INFO, EV_NONE, 0 ),
R"(
    obliterate -- Remove files and their history from the depot

    p4 obliterate [-y -A -b -a -T] [-h | -H] [-l | -L] [-r alg]
	    [-p | --purged-only] [ -q ] file[revRange] ...

	Obliterate permanently removes files and their history from the server.
	(See 'p4 delete' for the non-destructive way to delete a file.)
	Obliterate retrieves the disk space used by the obliterated files
	in the archive and clears the files from the metadata that is
	maintained by the server.  Files in client workspaces are not
	physically affected, but they are no longer under P4 control.

	Obliterate is aware of lazy copies made when 'p4 integrate' creates
	a branch, and does not remove copies that are still in use. Because
	of this, obliterating files does not guarantee that the corresponding
	files in the archive will be removed.

	If the file argument has a revision, the specified revision is
	obliterated.  If the file argument has a revision range, the
	revisions in that range are obliterated.  See 'p4 help revisions'
	for help.

	By default, obliterate displays a preview of the results. To execute
	the operation, you must specify the -y flag.

	Obliterate has three flags that can improve performance (-b, -a, -h):

	The '-b' flag restricts files in the argument range to those that
	are branched, are both the first revision and the head revision, and
	are not opened on any client.  This flag is useful for removing old
	branches while keeping files of interest (files that were modified).

	The '-a' flag skips the archive search and removal phase. File
	content is not removed. This option is safe to use with the '-b'
	option.

	The '-H' flag instructs obliterate to search db.have for all matching
	records to delete.  This is most useful when obliterating branches or
	namespaces for reuse, because the old content on any client will not
	match the newly-added repository files.  Usually, db.have is one of the
	largest tables in a repository and consequently this search takes a
	long time (this impacts edge servers' replication too).
	Note that without the '-H' flag any sync'ed obliterated files will be
	removed from client workspaces when they are next synced.
	Prior to 2026.1, the behavior of '-H' was the default, and the '-h'
	flag was required to skip the db.have search; this behavior can be
	restored by setting the 'dm.obliterate.skiphave' configurable to '0'.

	The '-L' flag works similar to '-H' but for db.label. It instructs
	obliterate to search db.label for all possible matching records to
	delete.  Prior to 2026.1, the behavior of '-L' was the default, and
	the '-l' flag was required to skip the db.label search; this behavior
	can be restored by setting the 'dm.obliterate.skiplabel' configurable
	to '0'.

	The '-p' flag instructs obliterate to mark the revision as purged
	and leave the integration history intact rather than removing
	the records.

	The '-r alg' flag selects the algorithm used to purge the integration
	records. The original algorithm scans the integration table looking
	for a match with the rev table. The new algorithm scans the rev
	table looking for a match with the integration table. As searches
	using a changelist number only operate on the rev table it is
	more efficient to use the new algorithm for file arguments that
	includes a changelist number or revision. By default the algorithm
	selected is decided by examining the file arguments. This option
	forces the use of a particular algorithm. When 'alg' is set to 0,
	then the original algorithm will be used. When alg is set to 1, then
	the new algorithm will be used.

	The '-T' flag is required for obliterating task stream paths.
	Without this flag, if an argument for obliterate includes task
	stream paths, a message will be reported to use '-T'.

	The '--purged-only' flag only searches for and removes previously
	purged records created by an earlier call to 'p4 obliterate -p -y'.
	It ignores the '-b', '-h' flags and enables '-a'. It removes all
	the associated meta-data (integration, client and  work records).

	The '-q' flag supresses the output messages concerning purged or
	deleted revision records or deleted archives.

	When a revision has been archived, its action is changed to archive.
	By default, obliterate will not process a revision which has been
	archived. To include such revisions, you must specify the -A flag.
	Note that obliterate -A does not affect the metadata and content in
	the archive depot. To remove the metadata and content in the archive
	depot, see 'p4 help archive' and the corresponding '-p' flag. Note
	also that obliterating an archive revision with the -A flag means
	you can no longer restore it nor use 'archive -p', so typically you
	should use 'archive -p' instead of 'obliterate -A'.

	If you are obliterating files in order to entirely remove a depot
	from the server, and files in that depot have been integrated to
	other depots, run 'p4 snap' first to break those linkages, so that
	obliterate can remove the unreferenced archive files. If, instead,
	you specify '-a' to skip the archive removal phase, then you will
	need to specify '-f' when deleting the depot, since the presence
	of the archive files will prevent the depot deletion.

	Performance may be affected when a large number of file revisions
	are being obliterated, so it is recommended that the command be run
	during a maintenance or quiet period on the server.

	'p4 obliterate' requires 'admin' access, which is granted by 'p4
	protect'.
)"
};

ErrorId MsgHelp::HelpOpened = { ErrorOf( ES_HELP, 65, E_INFO, EV_NONE, 0 ),
R"(
    opened -- List open files or stream spec and display status.

    p4 opened [-a -c changelist# -C client -u user -m max -s -g] [file ...]
    p4 opened [-a -x -m max ] [file ...]
    p4 opened [-c changelist#] -So

	Lists files currently opened in pending changelists, or, for
	specified files, show whether they are currently opened or locked.
	If the file specification is omitted, all files open in the current
	client workspace are listed.

	Files in shelved changelists are not displayed by this command. To
	display shelved changelists, see 'p4 changes -s shelved'; to display
	the files in those shelved changelists, see 'p4 describe -s -S'.

	If the stream spec associated with this client is currently opened,
	the stream spec is displayed on the first line of the output.

	By default, files opened by the current user in the current client
	client workspace are listed. The -a, -C client, -u user, and
	-c changelist# flags alter this behavior. The -a flag lists opened
	files in all clients. The -C client flag lists files open in the
	specified client workspace. The -u user flag lists files opened by
	the specified user. The -C and -u flags may be combined; -a is
	ignored if either of the -C or -u flags is used. The -C, -u, and -a
	flags are ignored if the -c flag is used.

	The -c changelist# flag lists files opened in the specified
	changelist#.

	The -m max flag limits output to the first 'max' number of files.

	The -s option produces 'short' and optimized output when used with
	the -a (all clients) option.  For large repositories '-a' can take
	a long time when compared to '-as'.

	The -x option lists files that are opened 'exclusive'. This option
	only applies to a distributed installation where global tracking of
	these file types is necessary across servers. The -x option implies
	the -a option.

	The -g option lists files that are opened on the Commit Server
	in a distributed installation. This is useful for tracking files
	that may be globally locked on the Commit Server from any of its
	Edge Servers. Since the command query runs on the Commit Server,
	the file spec argument (if any) must be in depot syntax. The -g
	option implies the -a option.

	The -So flag can be used with '-c change' to list the stream spec
	which is currently opened for edit. By default, if -So is omitted,
	the opened stream spec will be listed before the list of files.
	If no stream spec is opened for edit, a stream spec will not be
	listed.  (See 'p4 help streamcmds'.)

	See 'p4 help-graph opened' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpPasswd = { ErrorOf( ES_HELP, 66, E_INFO, EV_NONE, 0 ),
R"(
    passwd -- Set the user's password on the server (and Windows client)

    p4 passwd [-O oldPassword -P newPassword] [user]

	'p4 passwd' sets the user's password on the server.

	'p4 passwd' prompts for both the old password and the new password
	with character echoing turned off.  To delete the password, set it to
	an empty string.

	The -O flag provides the old password, avoiding prompting. If you
	specify -O, you must also specify -P.  This is only allowed at security
	levels less than 2.

	The -P flag provides the new password, avoiding prompting.  This is
	only allowed at security levels less than 2.

	If you are using ticket-based authentication, changing your password
	automatically invalidates all of your tickets and logs you out.

	Specifying a username as an argument to 'p4 passwd' requires 'super'
	access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpPopulate = { ErrorOf( ES_HELP, 148, E_INFO, EV_NONE, 0 ),
R"(
    populate -- Branch a set of files as a one-step operation

    p4 populate [options] fromFile[rev] toFile
    p4 populate [options] -b branch [-r] [toFile[rev]]
    p4 populate [options] -b branch -s fromFile[rev] [toFile]
    p4 populate [options] -S stream [-P parent] [-r] [toFile[rev]]

	options: -d description -f -m max -n -o

	'p4 populate' branches a set of files (the 'source') into another depot
	location (the 'target') in a single step.  The new files are created
	immediately, without requiring a 'p4 submit' or a client workspace.

	The target must be completely empty of files, including deleted files.

	Source and target files (fromFile and toFile) can be specified on
	the command line or through a branch view.  On the command line,
	fromFile is the source file set and toFile is the target file set.
	With a branch view, a toFile argument can be given to limit the scope
	of the target file set.

	A revision specifier can be used to select the revision to branch; by
	default, the head revision is branched. The revision specifier can be
	used on fromFile, or on toFile, but not on both.  When used on toFile,
	it refers to source revisions, not to target revisions.  A range may
	not be used as a revision specifier.  For revision syntax, see 'p4
	help revisions'.

	Use 'p4 populate -r -S' to branch a child stream from its parent.
	The -S flag makes 'p4 populate' use a generated branch view that maps
	the stream (or its underlying real stream) to its parent.  With -r,
	the direction of the mapping is reversed.  -P can be used to generate
	the branch view using a parent stream other than the stream's actual
	parent.

	The -b flag makes 'p4 populate' use a user-defined branch view.  (See
	'p4 help branch'.) The source is the left side of the branch view
	and the target is the right side. With -r, the direction is reversed.

	The -s flag can be used with -b to cause fromFile to be treated as
	the source, and both sides of the user-defined branch view to be
	treated as the target, per the branch view mapping.  An optional
	toFile argument may be given to further restrict the scope of the
	target file set.  -r is ignored when -s is used.

	The -d flag specifies a description for the submitted changelist.

	The -f flag forces deleted files to be branched into the target.
	By default, deleted files are treated as nonexistent and simply
	skipped.

	The -n flag displays a preview of the operation, without actually
	doing anything.

	The -o flag displays a list of files created by the populate command.

	The -m flag limits the actions to the first 'max' number of files.
)"
};


ErrorId MsgHelp::HelpPrint = { ErrorOf( ES_HELP, 67, E_INFO, EV_NONE, 0 ),
R"(
    print -- Write a depot file to standard output

    p4 print [-a -A -K -o localFile -q -m max --offset offset --size size
	      -Q charset -B utf8bom -L line-ending] file[revRange] ...
    p4 print -U unloadfile ...
    p4 print -T attribute [-a -q -o localFile] file ...

	Retrieve the contents of a depot file or the value of an attribute
	of the depot file to the client's standard output.
	The file is not synced.  If file is specified using client syntax,
	P4 uses the client view to determine the corresponding depot file.

	By default, the head revision is printed.  If the file argument
	includes a revision, the specified revision is printed.  If the
	file argument has a revision range,  then only files selected by
	that revision range are printed, and the highest revision in the
	range is printed. For details about revision specifiers, see 'p4
	help revisions'.

	The -a flag prints all revisions within the specified range, rather
	than just the highest revision in the range.

	The -A flag prints files in archive depots.

	The -K flag suppresses keyword expansion (this replaced the -k flag
	in 2022.1, which is now an alias for -K for backwards compatibility).

	The -L flag allows the line ending of textual files to be explicitly
	specified as either 'unix', 'win' or 'mac'.

	The -Q flag allows the charset for unicode type files to be explicitly
	specified, overriding the connection's P4CHARSET but not override the
	charset of unicode files with versioned charsets.

	The -B flag overrides the client-side 'filesys.utf8bom' setting,
	controlling the presence of the byte-order-mark in utf8 type files.

	The -o localFile flag redirects the output to the specified file on
	the client filesystem. Multiple files may be written by using wildcards
	in the localFile argument that match wildcards in the file argument.

	The -q flag suppresses the initial line that displays the file name
	and revision.

	The -m flag limits print to the first 'max' number of files.

	The --offset flag limits print by skipping 'offset' bytes from the
	begining of the file.

	The --size flag limits print to a maximum of 'size' bytes. Users can
	combine --offset and --size flags in one command.

	The -U option prints files in the unload depot (see 'p4 help unload'
	for more information about the unload depot).

	The -T option prints the value of the specified non-encoded attribute
	of the specified file. This command, rather than 'p4 fstat -Oa',
	is recommended for very large non-encoded binary attributes whose
	sizes exceed 250MB which may cause 'p4 fstat -Oa' to fail with
	the 'Rpc buffer too big to send' error. There is no option to show
	the value of the attribute in hex (as in 'p4 fstat -Oe').

	See 'p4 help-graph print' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpProperty = { ErrorOf( ES_HELP, 159, E_INFO, EV_NONE, 0 ),
R"(
    property -- Add, delete, or list property values

    p4 property -a -n name -v value [-s sequence] [-u user | -g group]
    p4 property -d -n name [-s sequence] [-u user | -g group]
    p4 property -l [-A] [-n name [-s sequence]] [-u user | -g group]
                   [-F filter -T taglist -m max]

	The first form updates a property value in the server, or adds the
	property value to the server if it is not yet there.

	The second form deletes a property value from the server.

	The third form lists one or more property values from the server.
	Specifying the -n flag when using the -l flag substantially improves
	the performance of this command.

	The server does not directly use property values. It provides
	storage of property values for use by applications that wish to
	persistently store their configuration settings and other property
	data in the server.

	When specifying multiple property values for the same property, use
	distinct sequence numbers to specify the precedence order. A value
	with a higher sequence number is ordered before a value with a lower
	sequence number. Values with the same sequence number have an
	undefined ordering relationship.

	The -n flag specifies the name of this property.

	The -v flag specifies the value of this property.

	The -s flag specifies the sequence number of this property. If the
	sequence is not specified, it defaults to 1.

	The -u flag specifies the user to whom this property applies.

	The -g flag specifies the user group to which this property applies.

	The -m max flag limits output to the first 'max' number of properties.

	The -T flag can be used to limit the fields that are displayed.

	The -F flag limits output to properties matching the filter pattern.

	The -T and -F flags can only be used with tagged format.

	The -A flag specifies that properties for all users and groups should
	be listed, including the sequence number of each property value.
	When combined with -g or -u, the output is limited to properties set
	explicitly on the group or user; omitting the -A with either flag will
	output all properties that would apply to the specified user or a user
	of the specified group.

	The -a, -A, -d, -g and -u flags require that the user have 'admin'
	access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpProtect = { ErrorOf( ES_HELP, 68, E_INFO, EV_NONE, 0 ),
R"(
    protect -- Modify protections in the server namespace

    p4 protect [path]
    p4 protect -o [path]
    p4 protect -i [path]

	'p4 protect' edits the protections table in a text form.

	Each line in the table contains a protection mode, a group/user
	indicator, the group/user name, client host ID and a depot file
	path pattern. Users receive the highest privilege that is granted
	on any line.

	Note: remote depots are accessed using the pseudo-user 'remote'.
	To control access from other servers that define your server as
	a remote server, grant appropriate permissions to the 'remote' user.

	    Mode:    The permission level or right being granted or denied.
	             Each permission level includes all the permissions above
	             it, except for 'review' and 'owner'. Each permission only
	             includes the specific right and no lesser rights.  This
	             approach enables you to deny individual rights without
	             having to re-grant lesser rights. Modes prefixed by '='
	             are rights. All other modes are permission levels.

	             'readstreamspec', 'openstreamspec' and 'writestreamspec'
	             determine access to stream spec definitions.
	             If no streamspec permission exists for any user, then
	             'list', 'open' and 'write' permissions control
	             stream spec access for 'edit', 'resolve', 'revert',
	             'shelve', 'submit', 'streams', and 'stream' commands.
	             'list' grants 'p4 streams' access for stream spec paths.
	             Any 'open' or higher permssion for a user anywhere grants
	             stream spec 'edit' and 'write' permissions to that user
	             for all stream specs.

	             If any streamspec permissions exist for any user, then
	             'list' will continue to provide 'p4 streams' access.
	             However, read/open/write access to stream specs will be
	             controlled by 'readstreamspec', 'openstreamspec', and
	             'writestreamspec'. Thus, users without explicit streamspec
	             permissions have no stream spec read/open/write access.

	             Note: streamspec permissions are disabled by default.
	             Enable by setting the server configurable
	             dm.protects.streamspec to 1.

	             Valid modes are:

	             list    - users can see names but not contents of files;
	                       users can see all non-file related metadata
	                       (clients, users, changelists, jobs, etc.)
	                       also provides 'p4 streams' access

	             read    - users can sync, diff, and print files

	             open    - users can open files (add, edit, delete,
	                       integrate)

	             write   - users can submit open files

	             admin   - permits those administrative commands and
	                       command options that don't affect the server's
	                       security.

	             owner   - allows access to the 'p4 protect' command, for
	                       the specified path.

	             super   - access to all commands and command options.

	             review  - permits access to the 'p4 review' command;
	                       implies read access

	             =read   - if this right is denied, users can't sync,
	                       diff, or print files

	             =branch - if this right is denied, users are not
	                       permitted to use files as a source
	                       for 'p4 integrate'

	             =open   - if this right is denied, users cannot open
	                       files (add, edit, delete, integrate)

	             =write  - if this right is denied, users cannot submit
	                       open files

	             readstreamspec   - users can display a stream spec with
	                                with 'p4 stream -o'.

	             openstreamspec   - users can revert, resolve, shelve, or
	                                open for edit a stream spec

	             writestreamspec  - users can submit or modify a stream
	                                spec

	             =readstreamspec  - if this right is denied, users cannot
	                                execute 'p4 stream -o'

	             =openstreamspec  - if this right is denied, users cannot
	                                revert, resolve, shelve, or open for
	                                edit a stream spec

	             =writestreamspec - if this right is denied, users cannot
	                                submit or modify a stream spec

	    Group/User indicator: specifies the grantee is a group or user.

	    Name:    A P4 group or user name; can include wildcards.

	    Host:    The IP address of a client host. IPv6 and IPv4 addresses
	             are supported and the * wildcard can be used to refer to
	             all IP addresses. When using the * wildcard with an IPv6
	             address, enclose the entire address in square brackets.
	             Best practice is to use CIDR notation, surround IPv6
	             addresses with brackets, and to avoid the * wildcard.

	             The server can distinguish connections coming from a
	             proxy, broker, or replica. The server prepends the string
	             'proxy-' to the IP address of the true client of such
	             a connection when the server enforces the protections.

	             Specify the 'proxy-' prefix for the IP address in the
	             Host: field in the protections table to indicate the
	             protections that should thus apply.

	             For example, 'proxy-*' applies to all connections from
	             all proxies, brokers, and replicas, while
	             'proxy-10.0.0.5' identifies a client machine with an IP
	             address of 10.0.0.5 which is connecting to p4d through
	             a proxy, broker, or replica.

	             If you wish to write a single set of protections entries
	             which apply both to directly-connected clients as well
	             as to those which connect via a proxy, broker, or
	             replica, you can omit the 'proxy-' prefix and also set
	             dm.proxy.protects=0. In this case, the 'proxy-' prefix
	             is not prepended to the IP address of connections which
	             are made via a proxy, replica or broker.  Note that in
	             this scenario, all intermediate proxies, brokers, and
	             replicas should be at release 2012.1 or higher.

	    Path:    The part of the depot to which access is being granted
	             or denied.  To deny access to a depot path, preface the
	             path with a "-" character. These exclusionary mappings
	             apply to all access levels, even if only one access
	             level is specified in the first field.


	There are two additional fields in the protect specification that are
	not part of the protections table:

	    SubPath: The root path of the sub-protections table (only used when
	             editing a sub-protections table).

	    Update:  The date this specification was last modified (read-only).
	             If this date is modified, the protections table will not
	             be saved; however, ommitting this field will avoid the
	             modification check.


	Sub-protections tables:
	    It is possible to delegate management of parts of the protections
	    table to non-super users or groups by creating an entry with the
	    mode 'owner'.  These entries must have a unique path, without
	    wildcards except for a trailing '...'.  Users with 'super' or that
	    have been granted 'owner' for a path may run the 'p4 protect'
	    command specifying the granted path as an argument to access the
	    sub-protections table for that path.  Any entries in this table
	    will be appended to the effective protections table directly below
	    the 'owner' entry; if an 'owner' entry is removed, so are any
	    entries in the sub-protections table for that path.  The 'owner'
	    permission only grants access to the sub-protections table at the
	    specified path: it does not grant the specified user or group any
	    permissions on that path.  The owner may grant themselves 'admin'
	    permissions on that path.  Neither 'owner' nor 'super' entries may
	    be added to a sub-protections table, and any other entries' paths
	    must be within the scope of the sub-protections table's path.


	If a path argument is specified, and an 'owner' entry with the same
	path exists, then the sub-protections table for that path will be
	accessed instead of the main protections table.

	The -o flag writes the protection table to the standard output.
	The user's editor is not invoked.

	The -i flag reads the protection table from the standard input.
	The user's editor is not invoked.

	After protections are defined, 'p4 protect' requires 'super' or 'owner'
	access.
)"
};

ErrorId MsgHelp::HelpProtects = { ErrorOf( ES_HELP, 93, E_INFO, EV_NONE, 0 ),
R"(
    protects -- Display protections defined for a specified user and path

    p4 protects [-s spec] [-a | -g group | -u user]
                [-h host | -H] [-m] [-S | -A] [file ...]
    p4 protects -M [-g group | -u user] [-h host | -H] [file ...]

	'p4 protects' displays the lines from the protections table that
	apply to the current user.  The protections table is managed using
	the 'p4 protect' command.

	If the -a flag is specified, protection lines for all users are
	displayed.  If the -g group flag or -u user flag is specified,
	protection lines for that group or user are displayed.

	If the -h host flag is specified, the protection lines that apply
	to the specified host (IP address) are displayed.

	If the -H flag is specified, the protection lines that apply to the
	current client's host (IP address) are displayed.

	If the -m flag is given, a single word summary of the maximum
	access level is reported. Note that this summary does not take
	exclusions or the specified file path into account.

	If the -M flag is given, a single word summary of the maximum
	access level is reported. Unlike '-m' it does take into account
	exclusions and the specified file path.

	If the -s spec flag is specified and a spec depot has been defined
	then the command runs using the contents of the file in the spec
	depot rather than the current protections table. This allows the
	'super' user back-in-time access to how permission would have
	behaved in a previous version of the protections spec.

	By default 'p4 protects' reports only file permissions.
	If the -S flag is given, report only stream spec protection lines.
	If the -A flag is given, report file and stream spec protection lines.

	If the file argument is specified, protection lines that apply to
	the specified files are displayed.

	The -a/-g/-u flags require 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpProxy = { ErrorOf( ES_HELP, 145, E_INFO, EV_NONE, 0 ),
R"(
    proxy -- Display Proxy connection information

    p4 proxy

	'p4 proxy' is only understood by the P4 Proxy.

	'p4 proxy' lists information about the proxy which the client is
	connected to.  Items displayed include the client's network address,
	the proxy's network address, and the proxy's version information.
)"
};

ErrorId MsgHelp::HelpPrune = { ErrorOf( ES_HELP, 174, E_INFO, EV_NONE, 0 ),
R"(
    prune -- Remove unmodified branched files from a stream

    p4 prune [-d] [-y] -S stream

	Prune permanently removes unmodified files from a stream that is
	no longer being actively used.  Only the owner of a stream may
	'p4 prune' it.

	After a stream has been pruned, files that have been modified, i.e.
	files with more than one revision or that are currently open on any
	client, will remain in the stream so that their edit history will be
	preserved.  The unmodified files will be gone as if obliterated by an
	administrator (see 'p4 help obliterate').

	Mainline, task, and virtual streams may not be pruned.  To remove
	unmodified files from a task stream, delete or unload it (see 'p4 help
	stream' and 'p4 help unload').

	Pruned files will remain in client workspaces until the next 'p4 sync',
	which will remove them.  If pruned files have been branched to a child
	stream, new integration records will be generated to directly link the
	branched files in the child stream to the files in the parent stream
	that they were previously related to indirectly.

	If the -d flag is given, a deep prune is performed which also removes
	files whose only changes in the target stream were due to merges.

	By default, prune displays a preview of the results.  To execute the
	operation, you must specify the -y flag.

	'p4 prune' requires 'write' access, which is granted by 'p4 protect'.
	You must also be the owner of the stream in order to run 'p4 prune'.

)"
};

ErrorId MsgHelp::HelpPubKey = { ErrorOf( ES_HELP, 201, E_INFO, EV_NONE, 0 ),
R"(
    pubkey -- Add or update a SSH public key to the P4 server

    p4 pubkey -d [-u user][-s scope]
    p4 pubkey -i [-u user][-s scope][-f]

	Pubkey is used to upload or delete a generated SSH public key to a P4
	user account. This SSH key can be used to authenticate as the user
	through SSH services. To upload a key to the P4 server you must already
	have a P4 user account.

	To generate a new ssh key, use the command ssh-keygen:

	$ ssh-keygen -t rsa -b 4096 -C "email-name@example.com"

	You will be prompted for a file location, choose the default file
	location.

	Enter file in which to save the key (${HOME}/.ssh/id_rsa):

	You will then be prompted for a secure passphrase, if you add one
	then you will be prompted each time you wish to use it, however
	you can use a command called "ssh-agent" to cache it.

	After generating the SSH key pair, your public key can be found in
	"${HOME}/.ssh/id_rsa.pub" NOTE the extension ".pub"
	this is the public key and can be safely distributed, the other
	half is NOT to be copied.

	Now you can upload your SSH public key to the P4 server:

	$ p4 pubkey -i < ${HOME}/.ssh/id_rsa.pub
	Public Key updated.

	You have now uploaded a copy of your public key to your account.

	The -d flag deletes the users public key

	The -f flag allows the user to replace an existing public key.

	The -i flag causes the key to be read from the standard input.

	The -u flag allows a super user to update the public key of another
	user.

	The -s flag allows a 'scope' or 'type' of public key if more than
	one key is required depending on the SSH access route. This option
	is only usually required for more complex environments. Without this
	flag the scope is set to 'default'.
)"
};

ErrorId MsgHelp::HelpPubKeys = { ErrorOf( ES_HELP, 202, E_INFO, EV_NONE, 0 ),
R"(
    pubkeys -- Display a list of users SSH public keys

    p4 pubkeys [ [-u user][-s scope] ] | [-k key -t type]

	Pubkeys outputs a list of keys previous stored in the server by the
	the command 'p4 pubkey'.

	The -u flag limits the output of keys to the specified user.

	The -s flag limits the output of keys to the specified scope.

	The -k flag limits the output to the specified base64 ssh key.

	The -t flag is required with the -k flag to specify the ssh key type.

)"
};

ErrorId MsgHelp::HelpRealtime = { ErrorOf( ES_HELP, 268, E_INFO, EV_NONE, 0 ),
R"(
	P4 server realtime performance counters

	Various server-wide performance counters are maintained in file-backed
	shared memory. This is enabled when the 'rt.monitorfile' configurable
	is set, specifying the file that backs the shared memory (for example,
	rt.monitorfile=monfile.mem). The server must be restarted before
	changes will take effect.

	The performance counters can be read:
	  - by external tools using the P4API's MonItems class
	  - with 'p4d -r P4ROOT --show-realtime'
	  - with 'p4 monitor realtime'.
	
	The current realtime performance counters are:
	
	    rtv.db.ckp.active       - Indicates if a checkpoint is in progress.
	
	    rtv.db.ckp.records      - Records read into current checkpoint.
	
	    rtv.db.io.records       - Running count of database IO operations:
	                              reads, writes and deletes. This counter
	                              may affect performance. To enable, set
	                              the 'db.rt.io' configurable to 1.
	
	    rtv.db.lockwait         - Commands waiting for locks. Includes
	                              high mark of most commands waiting for
	                              locks at any one time.
	
	    rtv.rpl.behind.bytes    - Replication lag represented as bytes of
	                              journal that the upstream server has yet
	                              to send. This counter is enabled when the
	                              'rpl.track.behind' configurable is set to
	                              1 or greater.
	                              The 'rpl.track.behind.interval'
	                              configurable can be used to tune the
	                              frequency of updates during 'p4 pull'.
	
	    rtv.rpl.behind.journals - Replication lag represented as rotated
	                              journals that the upstream server has yet
	                              to send.
	                              See 'rtv.rpl.behind.bytes' for related
	                              configurables.
	
	    rtv.server.connection.pause.pct.cpu - The percentage of commands to
	                              pause when CPU pressure is in a high
	                              state. The highest percentage of this or
	                              the rtv.server.connection.pause.pct.mem
	                              counter takes precedence.

	    rtv.server.connection.pause.pct.mem - The percentage of commands to
	                              pause when memory pressure is in a medium
	                              or high state. The highest percentage of
	                              the rtv.server.connection.pause.pct.cpu
	                              or this counter takes precedence.

	    rtv.svr.sessions.active - Current connections from clients.
	                              Includes a high mark of most concurrent
	                              connections at any one time.

	    rtv.svr.sessions.paused - Number of server sessions that are
	                              paused.
	
	    rtv.svr.sessions.total  - Running count of client connections.
	
	    rtv.sys.mem.os.available - An estimate of how much memory is
	                              available for starting new applications
	                              without swapping.
	
	    rtv.sys.mem.os.free     - Amount of memory that is not allocated to
	                              any application or the OS.
	
	    rtv.sys.mem.os.swap.free - Amount of swap memory that is not
	                              allocated.
	
	    rtv.sys.mem.os.swap.total - Total amount of swap configured in the
	                               system.
	
	    rtv.sys.mem.os.total    - Total amount of operating system memory.
	
	    rtv.sys.pressure.lastcollecttime - Timestamp of the most recent
	                              assessment of system pressure.
	
	    rtv.sys.pressure.level.cpu - The level of CPU pressure on a
	                              scale from 0 = low, 1 = medium, 2 = high

	    rtv.sys.pressure.level.mem - The level of memory pressure on a
	                              scale from 0 = low, 1 = medium, 2 = high
)"
};

ErrorId MsgHelp::HelpRename = { ErrorOf( ES_HELP, 69, E_INFO, EV_NONE, 0 ),
R"(
    rename -- How to rename files using pre-2009.1 clients

	In release 2009.1 and higher, you can use 'p4 move' to move or rename
	files.  P4 clients prior to release 2009.1 do not support 'p4 move'.
	However, files in older clients can be renamed by branching one file to
	another and deleting the original file.  For example:

	    p4 integrate fromFile toFile
	    p4 delete fromFile
	    p4 submit

	For further information, see the help for the individual commands.

	Note: Files renamed in this way are treated as branched files
	rather than moved files in subsequent operations.
)"
};

ErrorId MsgHelp::HelpReopen = { ErrorOf( ES_HELP, 70, E_INFO, EV_NONE, 0 ),
R"(
    reopen -- Change the filetype of an open file or move it to
              another changelist (or move an opened stream spec)

    p4 reopen [-c changelist#] [-t filetype | -Si] file ...
    p4 reopen -c changelist# -So

	Reopen an open file for the current user in order to move it to a
	different changelist or change its filetype.

	The target changelist must exist; you cannot create a changelist by
	reopening a file. To move a file to the default changelist, use
	'p4 reopen -c default'.

	The '-c' flag with file arguments will not allow a move pair to
	be split. The command will fail if only the 'movedFrom' file is
	specified. The command can succeed when only the 'movedTo' file
	is specified.

	If -t filetype is specified, the file is assigned that filetype. If
	a partial filetype is specified, it is combined with the current
	filetype.  For details, see 'p4 help filetypes'.

	The -So flag can be used with '-c change' to move the client's open
	stream spec to a different changelist.  The -Si flag can be used with
	a file list to include an open stream spec when also reopening files.
	For example, to move an open stream spec to the default changelist,
	use 'p4 reopen -c default -So'.  (See 'p4 help streamcmds'.)
)"
};

ErrorId MsgHelp::HelpReconcile = { ErrorOf( ES_HELP, 146, E_INFO, EV_NONE, 0 ),
R"(
    reconcile -- Open files for add, delete, and/or edit to reconcile
                 client with workspace changes made outside of P4

    rec         -- synonym for 'reconcile'
    status      -- 'reconcile -n + opened' (output uses local paths)
    status -A   -- synonym for 'reconcile -ead' (output uses local paths)

    clean       -- synonym for 'reconcile -w'

    p4 reconcile [-c change#] [-a -e -d -M -f -I -l -m -n -t] [-w [-K]]
                 [--parallel=N] [file ...]
    p4 status [-c change#] [-A | [-a -e -d -M] | [-s]] [-f -I -m]
              [--parallel=N] [file ...]
    p4 clean [-e -a -d -I -l -K -m -n] [file ...]
    p4 reconcile -k [-l -n] [file ...]
    p4 status -k [file ...]

	'p4 reconcile' finds unopened files in a client's workspace and
	detects the following:

	1. files in depot missing from workspace, but still on have list
	2. files on workspace that are not in depot
	3. files modified in workspace that are not opened for edit

	By default, the files matching each condition above in the path
	are reconciled by opening files for delete (scenario 1), add
	(scenario 2), and/or edit (scenario 3). The -e, -a, and -d flags
	may be used to limit to a subset of these operations. If no file
	arguments are given, reconcile and status default to the current
	working directory.

	If the list of files to be opened includes both adds and deletes,
	the -M flag allows the missing and added files to be compared and
	converted to pairs of move/delete and move/add operations if they
	are similar enough.

	The --parallel flag when used with the -M flag specifies the number
	of files opened for add to be compared simultaneously on the client
	to the file opened for delete to improve the performance of the move
	matching.

	In addition to opening unopened files, reconcile will detect files
	that are currently opened for edit but missing from the workspace
	and reopen them for delete. Reconcile will also detect files opened
	for delete that are present on the workspace and reopen them for
	edit.

	The -n flag previews the operation without performing any action.
	Although metadata updates from reconcile require open permission,
	the preview commands only require read access.

	If -c changelist# is included, the files are opened in the specified
	pending changelist.

	The -e flag allows the user to reconcile files that have been modified
	outside of P4. The reconcile command will open these files for edit.

	The -a flag allows the user to reconcile files that are in the user's
	directory that are not under P4 source control. These files are opened
	for add.

	The -f flag allows the user to add files with filenames that contain
	wildcard characters. Filenames that contain the special characters
	'@', '#', '%%' or '*' are reformatted to encode the characters using
	ASCII hexadecimal representation.  After the files are added, you
	must refer to them using the reformatted file name, because P4 does not
	recognize the local filesystem name.

	The -I flag informs the client that it should not perform any ignore
	checking configured by P4IGNORE.

	The -d flag allows the user to reconcile files that have been
	removed from the user's directory but are still in the depot.
	These files will be opened for delete only if they are still on the
	user's have list.

	The -t flag allows the user to reconcile files that are in the
	user's directory that had their file type changed. The reconcile
	command will open these files for edit even if the content is
	unchanged. Reconcile detects and changes filetype when the base
	filetype or executable mode has been changed. Modifiers such as
	'+l' and '+k' will be preserved. The '+w' attribute is not
	detected or handled by -t. The '+x' modifier will be unchanged
	by -t on platforms that don't support executable file bits,
	like Windows and VMS.

	The -l flag requests output in local file syntax using relative
	paths, similar to the workspace-centric view provided by 'status'.

	The -m flag used in conjunction with -e can be used to minimize costly
	digest computation on the client by checking file modification times
	before checking digests to determine if files have been modified
	outside of P4.  If the modification times match the user's have list,
	the digest computations will be skipped.

	The -w flag forces the workspace files to be updated to match the
	depot rather than opening them so that the depot can be updated to
	match the workspace. The -a, -d, and -e flags when used with -w
	update workspace files as follows:

	  -a Files with no corresponding depot file are deleted.
	  -d Depot files not in the workspace are added.
	  -e Modified files are restored to the last version synced.

	Note the use of -w will result in the loss of any changes made
	to unopened files. Use of the -w flag requires read permission.

	The -k flag updates the have list when files in the workspace but
	not on the have list match content of corresponding files in the
	depot. In this case, the client's have list is updated to reflect
	the matching revisions. This option is used to reconcile the have
	list with the workspace.

	The -K flag (only used with 'p4 clean') suppresses keyword expansion
	when updating ktext files on the client.

	The -s flag (only used with 'p4 status') requests summarized
	output for the files to be opened for 'add'. Files in the current
	directory are listed as usual, but subdirectories containing files
	to be opened for 'add' are displayed instead of each file. This
	optimized option doesn't support move detection. Files to open
	for 'delete' and 'edit' are still listed individually.

	The status command displays preview output which includes files
	which are already opened in addition to the files that need to
	be reconciled. Opened files are not shown with options -A/-a/-e/-d.

	See 'p4 help-graph reconcile' for information on using this command
	with graph depots.
)"
};

ErrorId MsgHelp::HelpResolve = { ErrorOf( ES_HELP, 71, E_INFO, EV_NONE, 0 ),
R"(
    resolve -- Resolve integrations and updates to workspace files

    p4 resolve [options] [file ...]
    p4 resolve -So [-a<flag>] [-n] [-o]

	options: -A<flags> -a<flags> -d<flags> -f -K -n -N -o -t -v
		 -c changelist#

	'p4 resolve' resolves changes to files in the client workspace.

	'p4 resolve' works only on files that have been scheduled to be
	resolved.  The commands that can schedule resolves are: 'p4 sync',
	'p4 update', 'p4 submit', 'p4 merge', and 'p4 integrate'.  Files must
	be resolved before they can be submitted.

	'p4 resolve -So' is an alias for 'p4 stream resolve'.
	See 'p4 help streamcmds'.

	Resolving involves two sets of files, a source and a target.  The
	target is a set of depot files that maps to opened files in the
	client workspace.  When resolving an integration, the source is a
	different set of depot files than the target.  When resolving an
	update, the source is the same set of depot files as the target,
	at a different revision.

	The 'p4 resolve' file argument specifies the target.  If the file
	argument is omitted, all unresolved files are resolved.

	Resolving can modify workspace files. To back up files, use 'p4
	shelve' before using 'p4 resolve'.

	The resolve process is a classic three-way merge. The participating
	files are referred to as follows:

	  'yours'       The target file open in the client workspace
	  'theirs'      The source file in the depot
	  'base'        The common ancestor; the highest revision of the
	                source file already accounted for in the target.
	  'merged'      The merged result.

	Filenames, filetypes, and text file content can be resolved by
	accepting 'yours', 'theirs', or 'merged'.  Branching, deletion, and
	binary file content can be resolved by accepting either 'yours' or
	'theirs'.

	When resolving integrated changes, 'p4 resolve' distinguishes among
	four results: entirely yours, entirely theirs, a pure merge, or an
	edited merge.  The distinction is recorded when resolved files are
	submitted, and will be used by future commands to determine whether
	integration is needed.

	In all cases, accepting 'yours' leaves the target file in its current
	state.  The result of accepting 'theirs' is as follows:

	   Content:     The target file content is overwritten.
	   Attribute:   The target's attributes are replaced.
	   Branching:   A new target is branched.
	   Deletion:    The target file is deleted.
	   Filename:    The target file is moved or renamed.
	   Filetype:    The target file's type is changed.

	For each unresolved change, the user is prompted to accept a result.
	Content and non-content changes are resolved separately.  For content,
	'p4 resolve' places the merged result into a temporary file in the
	client workspace.  If there are any conflicts, the merged file contains
	conflict markers that must be removed by the user.

	'p4 resolve' displays a count of text diffs and conflicts, and offers
	the following prompts:

	  Accept:
	     at              Keep only changes to their file.
	     ay              Keep only changes to your file.
	   * am              Keep merged file.
	   * ae              Keep merged and edited file.
	   * a               Keep autoselected file.

	  Diff:
	   * dt              See their changes alone.
	   * dy              See your changes alone.
	   * dm              See merged changes.
	     d               Diff your file against merged file.

	  Edit:
	     et              Edit their file (read only).
	     ey              Edit your file (read/write).
	   * e               Edit merged file (read/write).

	  Misc:
	   * m               Run '$P4MERGE base theirs yours merged'.
			     (Runs '$P4MERGEUNICODE charset base theirs
			      yours merged' if set and the file is a
			      unicode file.)
	     s               Skip this file.
	     h               Print this help message.
	     ^C              Quit the resolve operation.

	Options marked (*) appear only for text files. The suggested action
	will be displayed in brackets.

	The 'merge' (m) option enables you to invoke your own merge program, if
	one is configured using the $P4MERGE environment variable.  Four files
	are passed to the program: the base, yours, theirs, and the temporary
	file. The program is expected to write merge results to the temporary
	file.

	The -A flag can be used to limit the kind of resolving that will be
	attempted; without it, everything is attempted:

	    -Aa		Resolve attributes.
	    -Ab		Resolve file branching.
	    -Ac		Resolve file content changes.
	    -Ad		Resolve file deletions.
	    -Am		Resolve moved and renamed files.
	    -At		Resolve filetype changes.
	    -AQ		Resolve charset changes.

	The -a flag puts 'p4 resolve' into automatic mode. The user is not
	prompted, and files that can't be resolved automatically are skipped:

	    -as		'Safe' resolve; skip files that need merging.
	    -am 	Resolve by merging; skip files with conflicts.
	    -af		Force acceptance of merged files with conflicts.
	    -at		Force acceptance of theirs; overwrites yours.
	    -ay		Force acceptance of yours; ignores theirs.

	The -as flag causes the workspace file to be replaced with their file
	only if theirs has changed and yours has not.

	The -am flag causes the workspace file to be replaced with the result
	of merging theirs with yours. If the merge detected conflicts, the
	file is left untouched and unresolved.

	The -af flag causes the workspace file to be replaced with the result
	of merging theirs with yours, even if there were conflicts.  This can
	leave conflict markers in workspace files.

	The -at flag resolves all files by copying theirs into yours. It
	should be used with care, as it overwrites any changes made to the
	file in the client workspace.

	The -ay flag resolves all files by accepting yours and ignoring
	theirs. It preserves the content of workspace files.

	Note that there is a subtle distinction between manually accepting a
	merge resolve ('p4 resolve' and selecting 'am') and automatic merge
	resolve ('p4 resolve -am'). When accepting a merge resolve, the user
	makes a conscious choice.  With automatic merge resolves, the choice is
	automated and the result will depend on the three way merge differences
	between the target and source files.

	With an automatic merge resolve, if there are only additions or
	deletions in the target file, the target differences will be kept (the
	same as a manual accept yours).  If there are only additions or
	deletions in the source file, the source differences will be kept (the
	same as a manual accept theirs).  If there are additions or deletions
	in both the source and target files and there are no conflicts, the
	merged result is kept.

	In other words, 'p4 resolve -am' does not guarantee that all source
	and target files will be merged, only that the choice of whether to
	accept yours, accept theirs, or accept merge will be automated based
	upon the three way merge differences described above.

	The -d flags can be used to control handling of whitespace and line
	endings when merging files:

	    -db		Ignore whitespace changes.
	    -dw		Ignore whitespace altogether.
	    -dl 	Ignores line endings.

	The -d flags are also passed to the diff options in the 'p4 resolve'
	dialog. Additional -d flags that modify the diff output but do not
	modify merge behavior include -dn (RCS), -dc (context), -ds (summary),
	and -du (unified). Note that 'p4 resolve' uses content from the depot
	file if the files differ only in whitespace.

	The -f flag enables previously resolved content to be resolved again.
	By default, after files have been resolved, 'p4 resolve' does not
	process them again. The content of the target (yours) file being
	re-resolved is the result of the previous resolve, not the content of
	the original file. To preserve the option of using the original file,
	revert the resolved file.

	The -K flag suppresses keyword expansion when updating ktext files on
	the client.

	The -n flag previews the operation without altering files.

	The -N flag previews the operation with additional information about
	any non-content resolve actions that are scheduled.

	The -o flag displays the base file name and revision to be used
	during the merge.

	The -t flag forces 'p4 resolve' to attempt a textual merge, even for
	files with non-text (binary) types.

	The -v flag causes 'p4 resolve' to insert markers for all changes,
	not just conflicts.

	The -c flag limits 'p4 resolve' to the files in changelist#.

	See 'p4 help-graph resolve' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpResolved = { ErrorOf( ES_HELP, 72, E_INFO, EV_NONE, 0 ),
R"(
    resolved -- Show files that have been resolved but not submitted

    p4 resolved [-o] [file ...]

	'p4 resolved' lists file updates and integrations that have been
	resolved but not yet submitted.  To see unresolved integrations,
	use 'p4 resolve -n'.  To see already submitted integrations, use
	'p4 integrated'.

	If a client file path is specified, the output lists resolves for
	'yours' files that match the specified path.  If a depot file path is
	specified, the output lists resolves for both 'theirs' and 'yours'
	files that match the specified path.

	The -o flag reports the revision used as the base during the
	resolve.
)"
};

ErrorId MsgHelp::HelpRestore = { ErrorOf( ES_HELP, 126, E_INFO, EV_NONE, 0 ),
R"(
    restore -- Restore revisions from an archive depot.

    p4 restore [-n] -D <depot> file[revRange] ...

	Moves previously archived revisions from the specified archive depot
	back to their original locations. The revision's action is set to
	its original value (add/edit/etc.).

	The -n flag displays a preview of the operation.

	The storage for the archive depot must be mounted before running this
	command (except for preview).

	'p4 restore' requires 'admin' access, which is granted using 'p4
	protect'.
)"
};

ErrorId MsgHelp::HelpRetype = { ErrorOf( ES_HELP, 103, E_INFO, EV_NONE, 0 ),
R"(
    retype -- Change rev type or archive (storage) type (unsupported)

    p4 retype [-f -l -n] -t filetype file[revRange]

	Retype changes the filetype of each revision of the named files
	to the new specified filetype. 'filetype' may be a full or partial
	filetype.  See 'p4 help filetypes'.

	The -l flag causes retype to change the archive (storage) type
	of the revisions instead.  This involves actually moving the
	revision contents in the server archive.  If there are any lazy
	copies of the revisions, they are updated as well.  Note that
	changing the archive type to purge old revisions using the +S
	modifier or xtempobj is not allowed, as this would result in
	purging server archive files.  The effects of retype -l can be
	viewed with 'p4 fstat -Oc' (undocumented).  Note that to change
	both the filetype and the storage type, 'p4 retype' must be run
	twice: once without and once with the -l flag.

	The -n flag previews the operation.

	The -f flag forces the change of the archive (storage) type regardless
	of whether the archive is present.

	'p4 retype' ignores files from remote depots.

	'p4 retype' does not purge older revisions when a filetype is
	retyped to a '+Sn' type. Subsequent edits cause revisions to be
	purged.

	After changing the type of a file, any client workspaces which
	currently have this file should be refreshed by issuing
	'p4 sync file#none' followed by 'p4 sync file', to ensure that the
	client copy of the file reflects the new filetype.

	'p4 retype' requires 'admin' access, which is granted by 'p4 protect'.

	'p4 retype' is unsupported, because it cannot safely run concurrently
	with other 'p4 retype', 'p4 snap', or 'p4 obliterate' commands.
)"
};

ErrorId MsgHelp::HelpRevert = { ErrorOf( ES_HELP, 73, E_INFO, EV_NONE, 0 ),
R"(
    revert -- Discard changes from an opened file or stream spec.

    p4 revert [-a -n -k -K -w -c cl# -C client [-Si]] [--remote=rmt] file ...
    p4 revert -So [-c changelist# -C client]

	Revert an open file to the revision that was synced from the depot,
	discarding any edits or integrations that have been made.  You must
	explicitly specify the files to be reverted.  Files are removed from
	the changelist in which they are open.  Locked files are unlocked.

	The -a flag reverts only files that are open for edit, add, or
	integrate and are unchanged or missing. Files with pending
	integration records are left open. The file arguments are optional
	when -a is specified.

	The -n flag displays a preview of the operation.

	The -k flag marks the file as reverted in server metadata without
	altering files in the client workspace.

	The -K flag suppresses keyword expansion when updating ktext files on
	the client.

	The -w flag causes files that are open for add to be deleted from the
	workspace when they are reverted.

	The -c flag reverts files that are open in the specified changelist.

	The -C flag allows a user to specify the workspace that has the file
	opened rather than defaulting to the current client workspace. When
	this option is used, the '-k' flag is also enabled and the check for
	matching user is disabled. The -C flag requires 'admin' access, which
	is granted by 'p4 protect'.

	The -So flag can be used with '-c change' to revert the client's open
	stream spec.  The -Si flag can be used with a file list to include an
	open stream spec when reverting files.   If a stream spec is open and
	neither -Si nor -So is specified, then the stream spec will excluded
	from the reversion.  (See 'p4 help streamcmds'.)

	The --remote flag is useful for DVCS configurations in which files
	of type +l are in use. 'p4 revert --remote=origin filename' reverts
	the named file in your DVCS server, and additionally, if the file is
	of type +l, releases the global exclusive lock on the file in the
	origin server.

	See 'p4 help-graph revert' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpReview = { ErrorOf( ES_HELP, 74, E_INFO, EV_NONE, 0 ),
R"(
    review -- List and track changelists (for the review daemon)

    p4 review [-c changelist#] [-t counter]

	'p4 review' lists changelists that have not been reviewed before,
	as tracked by the specified counter.  If the counter is omitted,
	'p4 review' lists all changelists.  (If a changelist# and counter
	are specified, 'p4 review' sets the counter to that changelist# and
	produces no output.  This functionality has been superceded by the
	'p4 counter' command.)

	'p4 review' is not meant as an end-user command.  It exists to
	support an automated change review daemon.
)"
};

ErrorId MsgHelp::HelpReviews = { ErrorOf( ES_HELP, 75, E_INFO, EV_NONE, 0 ),
R"(
    reviews -- List the users who are subscribed to review files

    p4 reviews [-C client] [-c changelist#] [file ...]

	'p4 reviews' lists all users who have subscribed to review the
	specified files.

	The -c flag limits the files to the submitted changelist.

	The -C flag limits the files to those opened in the specified clients
	workspace,  when used with the -c flag limits the workspace to files
	opened in the specified changelist.

	To subscribe to review files, issue the 'p4 user' command and edit
	the 'Reviews field'.
)"
};

ErrorId MsgHelp::HelpRevokePermission = { ErrorOf( ES_HELP, 198, E_INFO, EV_NONE, 0 ),
R"(
    revoke-permission -- revoke access permission to a repo/graph depot

    p4 revoke-permission [-f] -n //repo/name -g group [-r ref] -p permission
    p4 revoke-permission [-f] -n //repo/name -u user  [-r ref] -p permission
    p4 revoke-permission [-f] -d graph-depot -g group [-r ref] -p permission
    p4 revoke-permission [-f] -d graph-depot -u user  [-r ref] -p permission

	'p4 revoke-permission' is used by an administrator or a super user
	to revoke permissions on a specified repo or a collection of repos
	managed under one graph depot.  An administrator is the owner of
	the graph depot, the owner of the repo, or a user that has been
	added as an administrator for the repo or graph depot specified.
	To revoke a permission all the options must exactly match, only
	one permission entry can be revoked with this command.

	The -f flag revokes permission for repo, user, group which do not
	exist.

	The -d graph-depot flag revokes the permission for all of the repos
	managed under the graph depot specified. To revoke permissions granted
	by "*" for all graph depots, "*" must be specified.

	The -n //repo/name flag revokes the permission for the repo specified.

	The -u user flag revokes permission to the user specified.

	The -g group flag revokes permission to the group specified.

	The -r ref flag specifies an optional reference (ref) for the
	permission (branch or tag) to revoke. For permission 'restricted-ref',
	-r ref is required.

	The -p permission flag revokes the permission specified.

	    Valid permissions:

	    admin          - the user/group specified can grant and revoke
	                     permissions for the repo/depot specified. An
	                     admin user has all permissions enabled.

	    read           - the user/group can read the repo(s) specified
	                     by the '-d' or '-n' option.

	    restricted-ref - the user/group can update the ref, (branch or tag)
	                     specified by '-r'. If set, only users with this
	                     permission can perform an update. Prevents other
	                     users with write-ref/write-all from updating the
	                     specified reference.

	    create-ref     - the user/group can update the ref, (branch or tag)
	                     specified by '-r'.

	    delete-ref     - the user/group can delete the ref, (branch or tag)
	                     specified by '-r'.

	    write-ref      - the user/group can update the ref, (branch or tag)
	                     specified by '-r'.

	    write-all      - the user/group can update the ref, (branch or tag)
	                     and all files specified by '-r'.

	    create-repo    - the user/group can create a repo that is managed
	                     by the graph depot specified.

	    delete-repo    - the user/group can delete a repo that is managed
	                     by the graph depot specified.

	    force-push     - the user/group can force push (overwrite) work to
	                     the reference specified by '-r'.


)"
};

ErrorId MsgHelp::HelpSearch = { ErrorOf( ES_HELP, 100, E_INFO, EV_NONE, 0 ),
R"(
    search -- Search index (unsupported)

    p4 search [-m max] words

	Search is an interface to the search engine that searches jobs.
	This command lists any index keys that contain the specified words.

	The -m flag limits search to the first 'max' number of jobs.

	See also 'p4 help index'.
)"
};

ErrorId MsgHelp::HelpShowPermission = { ErrorOf( ES_HELP, 199, E_INFO, EV_NONE, 0 ),
R"(
    show-permission -- show access permission granted to a repo/graph depot

    p4 show-permission -n //repo/name [-g group|-u user][-r ref][-p perm]
    p4 show-permission -d graph-depot [-g group|-u user][-r ref][-p perm]

	'p4 show-permission' is used by an administrator or a super user
	to show permissions on a specified repo or a collection of repos
	managed under one graph depot.  An administrator is the owner of
	the graph depot, the owner of the repo, or a user that has been
	added as an administrator for the repo or graph depot specified.

	The -d graph-depot flag restricts output to the graph depot specified.

	The -n //repo/name flag restricts output to the repo specified.

	The -p permission flag restricts output to the permission specified.

	The -u user flag restricts output to the user specified.

	The -g group flag restricts output to the group specified.

	The -r ref flag restricts output to the branch or tag specified.

	See 'show-permissions' to show access permissions granted to a user.
)"
};

ErrorId MsgHelp::HelpShowPermissions = { ErrorOf( ES_HELP, 240, E_INFO, EV_NONE, 0 ),
R"(
    show-permissions -- show access permissions granted to a user

    p4 show-permissions [-d graph-depot | -n //repo/name] [-u user | -g group]

	'p4 show-permissions' is used to show a user or group's permissions
	granted for graph depots.  This may be run by the specified user, an
	administrator or super user.  An administrator is the owner of the
	specified graph depot, the owner of the specified repo, or a user that
	has been added as an administrator for the repo or graph depot
	specified; if no repo or graph depot is specified, only the specified
	user or a super user may run this command.

	The -u user flag specifies the user to output the permissions for.

	The -g group flag specifies the group to output the permissions for.

	The -d graph-depot flag restricts output to the graph depot specified.

	The -n //repo/name flag restricts output to the repo specified.

	See 'show-permission' to show access permission granted to a
	repo/graph depot.
)"
};

ErrorId MsgHelp::HelpSnap = { ErrorOf( ES_HELP, 102, E_INFO, EV_NONE, 0 ),
R"(
    snap -- Snap (undo) archive lazy copies (unsupported)

    p4 snap [-n] file[revRange] ... [archivePath]

	Snap scans the specified file revisions for any revision that has
	an archive file that is a lazy copy.  For each of these files the
	contents are copied into their own archive file, and the file is
	updated to reflect that it is no longer a lazy copy.

	This operation increases the amount of space required for the archive
	but ensures that (manually) deleting any other part of the archive
	does not affect the specified revisions.

	If the archivePath argument is specified, only those file revisions
	that have lazy copies of archives in that archive path are copied.
	The archive path of a particular file revision can be viewed with
	'p4 fstat -Oc' (undocumented).

	For example, if you intend to (manually) delete the archives for
	'//olddepot', but you know that some number of integrations were
	performed from '//olddepot' to '//depot2', you might do

		'p4 snap //depot2/... //olddepot/...'

	Since 'p4 snap' does not remove the archives from '//olddepot', but
	only creates the new archives in '//depot2', you will need to run
	'p4 obliterate' on the files in '//olddepot' after you have run
	'p4 snap' in order to clean up the '//olddepot' contents.

	The -n flag previews the operation without copying files or changing
	metadata.

	'p4 snap' requires 'admin' access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpSet = { ErrorOf( ES_HELP, 76, E_INFO, EV_NONE, 0 ),
R"(
    set -- Set or display P4 variables

    p4 set [-q] [-s | -S service] [var[=[value]] ...]

	'p4 set' can set P4 variables in persistent storage.

	On Windows, 'p4 set' sets P4 variables in the Windows registry to the
	specified values.  On other platforms, 'p4 set' sets P4 variables in
	the P4ENVIRO file.

	The 'p4 set' command can be used to set multiple variables in a
	single invocation, as well as to unset variables, or to display the
	current variable value. To display a variable's value, omit the
	'=value' and specify only the variable's name; to unset a variable,
	specify 'var=' but omit the value (as in: 'p4 set P4LOG=')

	With no arguments, 'p4 set' lists all variable settings, and list
	which persistent storage mechanism contained the variable's value.

	The -q flag removes the origins of the settings from the list output.

	The -s flag sets variables for the whole system rather than for the
	user.  This option requires NT administrator privilege.

	The -S service flag sets variables for the specified service. This
	option requires NT administrator privilege.

	Registry/P4ENVIRO variable entries can be overridden by environment
	variables and (in some cases) flags on the command line. See 'p4 help
	environment' for a list of environment variables used by P4.

	Certain P4 variables can also be set persistently by using
	configuration files named by the P4CONFIG variable. These settings
	have higher precedence than environment/registry variables, but lower
	precedence than command line flags. P4CONFIG file variable settings
	only affect P4 client programs; servers do not use them.

	Since P4 variables can be set in multiple locations, you should examine
	the output of 'p4 set' carefully; a variable set in one location may be
	overriding a variable set in a lower-precedence location.
)"
};

ErrorId MsgHelp::HelpShelve = { ErrorOf( ES_HELP, 119, E_INFO, EV_NONE, 0 ),
R"(
    shelve -- Store files (or an open stream spec) from a pending changelist
    into the depot

    p4 shelve [-p] [--parallel=threads=N[,batch=N][,min=N]]
              [-As | [-Af] file ...]
    p4 shelve [-a option] [-p] -i [-f | -r] [-As | -Af]
              [--parallel=threads=N[,batch=N][,min=N]]
    p4 shelve [-a option] [-p] -r -c changelist# [-As | -Af]
              [--parallel=threads=N[,batch=N][,min=N]]
    p4 shelve [-a option] [-p] -c changelist# [-f]
              [--parallel=threads=N[,batch=N][,min=N]]
              [-As | [-Af] file ...]
    p4 shelve -d -c changelist# [-f] [-As | [-Af] file ...]

	'p4 shelve' creates, modifies or deletes shelved files in a pending
	changelist. Shelved files remain in the depot until they are deleted
	(using 'p4 shelve -d') or replaced by subsequent shelve commands.
	After 'p4 shelve', the user can revert the files and restore them
	later using 'p4 unshelve'.  Other users can 'p4 unshelve' the stored
	files into their own workspaces.

	Files that have been shelved can be accessed by the 'p4 diff',
	'p4 diff2', 'p4 files' and 'p4 print' commands using the revision
	specification '@=change', where 'change' is the pending changelist
	number.

	By default, 'p4 shelve' creates a changelist, adds files from the
	user's default changelist, then shelves those files in the depot.
	The user is presented with a text changelist form displayed using
	the editor configured using the $P4EDITOR environment variable.

	If a file pattern is specified, 'p4 shelve' shelves the files that
	match the pattern. When files are opened in the default changelist,
	only a single file pattern can be specified when creating a shelf;
	a file pattern containing wildcards can be used to match and shelve
	multiple files.

	The -Af flag specifies that only files should be shelved with this
	changelist.  The -As flag specifies that only an opened stream
	specification should be shelved with this change list.  By default,
	if the stream spec is open and neither -Af nor -As is given, the
	stream specification will also be included with any shelved
	files.  (See 'p4 help streamcmds'.)

	The -i flag reads the pending changelist specification with shelved
	files from the standard input.  The user's editor is not invoked.
	To modify an existing changelist with shelved files, specify the
	changelist number using the -c flag.

	The -c flag specifies the pending changelist that contains shelved
	files to be created, deleted, or modified. Only the user and client
	of the pending changelist can add or modify its shelved files. Any
	files specified by the file pattern must already be opened in the
	specified changelist; use 'p4 reopen' to move an opened file from
	one changelist to another.

	The -f (force) flag must be used with the -c or -i flag to overwrite
	any existing shelved files in a pending changelist.

	The -r flag (used with -c or -i) enables you to replace all shelved
	files in that changelist with the files opened in your own workspace
	at that changelist number.  Previously shelved files will be deleted.
	Only the user and client workspace of the pending changelist can
	replace its shelved files.

	The -a flag enables you to handle unchanged files similarly to some
	client submit options, namely 'submitunchanged' and 'leaveunchanged'.
	The default behavior of shelving all files corresponds to the
	'submitunchanged' option. The 'leaveunchanged' option only shelves
	changed files, and then leaves the files opened in the pending
	changelist on the client.

	The -d flag (used with -c) deletes the shelved files in the specified
	changelist so that they can no longer be unshelved.  By default, only
	the user and client of the pending changelist can delete its shelved
	files. A user with 'admin' access can delete shelved files by including
	the -f flag to force the operation.

	If the shelved changelist includes a stream spec, by default it is
	deleted when all files have been deleted; the -As flag forces the
	the stream spec to be deleted even if files remain.

	The -p flag promotes a shelved change from an edge server to a commit
	server where it can be accessed by other edge servers participating
	in the distributed configuration.  Once a shelved change has been
	promoted, all subsequent local modifications to the shelf are also
	pushed to the commit server and remain until the shelf is deleted.
	Once a shelf has been created, the combination of flags '-p -c' will
	promote the shelf without modification unless '-f' or '-r' are also
	used to update the shelved files before promotion.

	The --parallel flag specifies options for parallel file transfer. If
	parallel file transfer has been enabled by setting the
	net.parallel.max configurable, and if there are sufficient resources
	across the system, a shelve command may execute more rapidly by
	transferring multiple files in parallel. Specify threads=N to request
	files be sent concurrently, using N independent network connections.
	The N threads grab work in batches: specify batch=N to control the
	number of files in a batch. A shelve that is too small will not
	initiate parallel file transfers: specify min=N to control the
	minimum number of files in a parallel shelve. Promoted shelves
	require an additional file transfer from the Edge to Commit Server.
	Parallel pull threads for this transfer are only used if the
	ExternalAddress field is set in its Edge Server spec and pull threads
	can be used on the Commit Server. This transfer using pull threads
	is currently not supported on Windows platforms.

	Auto parallel shelve may be enabled by setting the
	net.parallel.shelve.threads configurable to the desired number
	of threads to be used for all shelve commands. This value must be
	less than or equal to the value of net.parallel.max. Other
	net.parallel.shelve.* configurables may be specified as well, but
	are not required. See 'p4 help configurables' to see the options
	and their defaults. Auto parallel shelve is turned off by unsetting
	the net.parallel.shelve.threads configurable. A user may override
	the configured auto parallel shelve options on the command line,
	or may disable it via 'p4 shelve --parallel=0'.
)"
};

ErrorId MsgHelp::HelpSubmit = { ErrorOf( ES_HELP, 77, E_INFO, EV_NONE, 0 ),
R"(
    submit -- Submit open files and/or open stream spec to the depot

    p4 submit [-r -s -f option -K [-b | --noretransfer 0|1]]
              [--parallel=threads=N[,batch=N][,min=N]]
    p4 submit [-r -s -f option -K -b]
              [--parallel=threads=N[,batch=N][,min=N]]
              file ...
    p4 submit [-r -f option -K -b] [-So|-Sx] -d description
              [--parallel=threads=N[,batch=N][,min=N]]
    p4 submit [-r -f option -K -b] [-Sx] -d description
              [--parallel=threads=N[,batch=N][,min=N]]
              file ...
    p4 submit [-r -f option -K [-b | --noretransfer 0|1]] -c changelist#
              [--parallel=threads=N[,batch=N][,min=N]]
    p4 submit -e shelvedChange# [-b]
              [--parallel=threads=N[,batch=N][,min=N]]
    p4 submit -i [-r -s -f option -K -b]
              [--parallel=threads=N[,batch=N][,min=N]]

	'p4 submit' commits a pending changelist and the files it contains to
	the depot, and/or submit an open stream spec.

	By default, 'p4 submit' attempts to submit all files in the 'default'
	changelist.  Submit displays a dialog where you enter a description
	of the change and, optionally, delete files from the list of files
	to be checked in.

	To add files to a changelist before submitting, use any of the
	commands that open client workspace files: 'p4 add', 'p4 edit',
	etc.

	If the file parameter is specified, only files in the default
	changelist that match the pattern are submitted.

	Files in a stream path can be submitted only by client workspaces
	dedicated to the stream. See 'p4 help client'.

	Before committing a changelist, 'p4 submit' locks all the files being
	submitted. If any file cannot be locked or submitted, the files are
	left open in a numbered pending changelist. By default, the files in
	a failed submit operation are left locked unless the
	submit.unlocklocked configurable is set. Files are unlocked even if
	they were manually locked prior to submit if submit fails when
	submit.unlocklocked is set. 'p4 opened' shows unsubmitted files
	and their changelists.

	Submit is atomic: if the operation succeeds, all files are updated
	in the depot. If the submit fails, no depot files are updated.

	If submit fails, some or all of the files may have been copied to
	the server. By default, retrying a failed submit will transfer all of
	the files again unless the submit.noretransfer configurable is set.
	If submit.noretransfer is set to 1, submit uses digest comparisons to
	to detect if the files have already been transferred in order to
	avoid file re-transfer when retrying a failed submit.

	The --noretransfer flag is used to override the submit.noretransfer
	configurable so the user can choose the preferred re-transfer
	behavior during the current submit operation.

	The -So and -Sx flags can be used to control whether an open stream
	spec in the default changelist will be submitted.  Use -So with no file
	list to submit only the open stream spec in the default changelist.
	Use -Sx with a file list to exclude the open stream spec when
	submitting files in the default changelist; use -Sx with no file list
	to submit all files without submitting the open stream spec.

	By default, if both the -So and -Sx flags are omitted, then an open
	stream spec will be submitted along with any files in the given
	filelist.  (See 'p4 help streamcmds'.)

	The -c flag submits the specified pending changelist instead of the
	default changelist. Additional changelists can be created manually,
	using the 'p4 change' command, or automatically as the result of a
	failed attempt to submit the default changelist.

	The -e flag submits a shelved changelist without transferring files
	or modifying the workspace. The shelved change must be owned by
	the person submitting the change, but the client may be different.
	However, files shelved to a stream target may only be submitted by
	a stream client that is mapped to the target stream. In addition,
	files shelved to a non-stream target cannot be submitted by a stream
	client. To submit a shelved change, all files in the shelved change
	must be up to date and resolved. No files may be open in any workspace
	at the same change number. Client submit options (ie revertUnchanged,
	etc) will be ignored. If the submit is successful, the shelved change
	and files are cleaned up, and are no longer available to be unshelved
	or submitted.

	The -d flag passes a description into the specified changelist rather
	than displaying the changelist dialog for manual editing. This option
	is useful for scripting, but does not allow you to add jobs or modify
	the default changelist.

	The -f flag enables you to override submit options that are configured
	for the client that is submitting the changelist.  This flag overrides
	the -r (reopen)flag, if it is specified.  See 'p4 help client' for
	details about submit options.

	The -K flag suppresses keyword expansion when updating ktext files on
	the client after submit.

	The -i flag reads a changelist specification from the standard input.
	The user's editor is not invoked.

	The -r flag reopens submitted files in the default changelist after
	submission.

	The -s flag extends the list of jobs to include the fix status
	for each job, which becomes the job's status when the changelist
	is committed.  See 'p4 help change' for details.

	The --parallel flag specifies options for parallel file transfer. If
	parallel file transfer has been enabled by setting the
	net.parallel.max configurable, and if there are sufficient resources
	across the system, a submit command may execute more rapidly by
	transferring multiple files in parallel. Specify threads=N to request
	files be sent concurrently, using N independent network connections.
	The N threads grab work in batches; specify batch=N to control the
	number of files in a batch. A submit that is too small will not
	initiate parallel file transfers; specify min=N to control the
	minimum number of files in a parallel submit.

	Using --parallel from an edge server allows parallel file transfer
	from the edge server to the commit server. This uses standard pull
	threads to transfer the files. Note that an administrator must ensure
	that pull threads can be run on the commit server. The edge server ip
 	address and port number	used by the commit server to connect to the
	edge server must be specified in the ExternalAddress field of the
	edge server spec. Note that parallel submit is not supported from an
	edge server if the commit server is on a Windows platform. In this
	case --parallel	or auto	parallel submit will be silently ignored, and
	the submit will run without using parallel threads.

	Auto parallel submit may be enabled by setting the
	net.parallel.submit.threads configurable to the desired number
	of threads to be used for all submit commands. This value must be
	less than or equal to the value of net.parallel.max. Other
	net.parallel.submit.* configurables may be specified as well, but
	are not required. See 'p4 help configurables' to see the options
	and their defaults. Auto parallel submit is turned off by unsetting
	the net.parallel.submit.threads configurable. A user may override
	the configured auto parallel submit options on the command line,
	or may disable it via 'p4 submit --parallel=0`.

	An edge-content trigger can be used to transfer archive files from
	an Edge to its Commit Server using an alternative file transfer
	mechanism. This option is only used in a distributed environment and
	is not supported for RCS storage. The configurable pull.trigger.dir
	must be set to a location to write temporary files. The configurable
	rpl.submit.nocopy must be set to 1 to suppress default P4 archive file
	transfer to the Commit Server. The trigger must use 'fstat -Ob' to get
	the list of files to transfer, and it must contain the file transfer
	commands.

	The -b flag can be used on an edge server to enable archive transfer
	from the edge to the commit server to be done in background. The
	submit process on the commit server will schedule the transfers
	and complete the submit without waiting for transfers to finish.
	The archives on the originating edge server will be transferred to
	the commit server via pull threads initiated by submit on the
	commit server. Archives of ktext files will be transferred
	serially as usual rather than in background since archive content
	depends on commit server work. Also note that submitting pending
	(not shelved) changes using this feature will cause any
	change-submit triggers to be fired before edge-content triggers. An
	administrator must ensure that pull threads can be run on the commit
	server. The edge server IP address and port number used by the
	commit server to connect to the edge server must be specified in the
	ExternalAddress field of the edge server spec. The configurables
	submit.allowbgtransfer and lbr.autocompress must be set for all
	servers using this feature, including the commit server. This
	feature is not supported for graph depots. This option will be
	ignored if alternative file transfer or lbr.replication=shared is
	configured. This option is also ignored when submit is not run
	from an edge server, or when submit.allowbgtransfer is not set.
	If noretransfer is configured, noretransfer will be ignored.
	If archives failed to transfer to the commit server, they may be
	fetched on the commit server using 'p4 pull -u -t' to pull archives
	from the edge address specified by -t. The failed transfers are visible
	on the commit server via 'p4 pull -l' which also shows the target
	address for each failed transfer. Please see 'p4 help pull' for
	more details.

	Auto background archive transfer may be enabled by setting the
	submit.autobgtransfer configurable to 1 on the edge servers which
	require this functionality. When this is set in addition to the
	required settings for using -b, all submits from that edge will
	use background archive transfer without requiring -b on the command
	line.

	See 'p4 help-graph submit' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpSpec = { ErrorOf( ES_HELP, 99, E_INFO, EV_NONE, 0 ),
R"(
    spec -- Edit spec comments and formatting hints (unsupported)

    p4 spec [ -d | -i | -o ] [ --repair ] type

	Edit any type of specification: branch, change, client, depot,
	group, job, label, spec, stream, triggers, typemap, or user. Only
	the comments and the formatting hints can be changed. Any fields
	that you add during editing are discarded when the spec is saved.

	'p4 jobspec' is equivalent to 'p4 spec job'.
	'p4 streamspec' is equivalent to 'p4 spec stream'.
	Any custom spec (including the job/stream specs) can be deleted with
	'p4 spec -d type'.

	The --repair option permits the correction of a customized spec causing
	errors.  The --repair cannot be used with non-customized specs.
	Errors may arise in a customized spec after upgrading from a pre 2019.2
	to a 2019.2 or later server where a previous spec customization
	contains incompatible or missing data.  Using --repair will suppress
	errors and attempt to update the customized spec to the current server
	release.  The customized failing spec and the current default spec
	are merged, naming conflicts noted with suffixes, conflicting codes
	reassigned.  If adding yet another new field during the --repair,
	the use of 'NNN' as a field code is not allowed.
	Renaming fields is allowed.  If an error is reported after
	saving the updated spec, quit and try again.  The --repair option
	does not support the error/correct cycle.

	If the pre 2019.2 customized spec is merely a subset of the
	new server spec fields with no differences in any of its fields
	with the server's spec fields,  then the best solution is
	to delete the customized spec with 'p4 spec -d --repair <specname>'
	and the server will then use the new default spec fields.
)"
};

ErrorId MsgHelp::HelpSync = { ErrorOf( ES_HELP, 78, E_INFO, EV_NONE, 0 ),
R"(
    sync -- Synchronize the client with its view of the depot
    flush -- synonym for 'sync -k'
    update -- synonym for 'sync -s'

    p4 sync [-E -f -L -n -N -k -K -q -r] [-m max]
            [--use-stream-change=N]
            [--parallel=threads=N[,batch=N][,batchsize=N][,min=N][,minsize=N]]
            [file[revRange] ...]
    p4 sync [-E -L -n -N -K -q -s] [-m max]
            [--use-stream-change=N]
            [--parallel=threads=N[,batch=N][,batchsize=N][,min=N][,minsize=N]]
            [file[revRange] ...]
    p4 sync [-E -L -n -N -K -p -q] [-m max]
            [--use-stream-change=N]
            [--parallel=threads=N[,batch=N][,batchsize=N][,min=N][,minsize=N]]
            [file[revRange] ...]
    p4 sync -k --sync-time=N [file[revRange] ...]

	Sync updates the client workspace to reflect its current view (if
	it has changed) and the current contents of the depot (if it has
	changed). The client view maps client and depot file names and
	locations.

	Sync adds files that are in the client view and have not been
	retrieved before.  Sync deletes previously retrieved files that
	are no longer in the client view or have been deleted from the
	depot.  Sync updates files that are still in the client view and
	have been updated in the depot.

	By default, sync affects all files in the client workspace. If file
	arguments are given, sync limits its operation to those files.
	The file arguments can contain wildcards.

	If the file argument includes a revision specifier, then the given
	revision is retrieved.  Normally, the head revision is retrieved.

	If the file argument includes a revision range specification,
	only files selected by the revision range are updated, and the
	highest revision in the range is used.

	See 'p4 help revisions' for help specifying revisions or ranges.

	Normally, sync does not overwrite workspace files that the user has
	manually made writable.  Setting the 'clobber' option in the
	client specification disables this safety check.

	The -f flag forces resynchronization even if the client already
	has the file, and overwriting any writable files.  This flag doesn't
	affect open files.

	The -L flag can be used with multiple file arguments that are in
	full depot syntax and include a valid revision number. When this
	flag is used the arguments are processed together by building an
	internal table similar to a label. This file list processing is
	significantly faster than having to call the internal query engine
	for each individual file argument. However, the file argument syntax
	is strict and the command will not run if an error is encountered.

	The -n flag previews the operation without updating the workspace.

	The -N flag also previews the operation without updating the
	workspace, but reports only a summary of the work the sync would do.

	The -k flag updates server metadata without syncing files. It is
	intended to enable you to ensure that the server correctly reflects
	the state of files in the workspace while avoiding a large data
	transfer. Caution: an erroneous update can cause the server to
	incorrectly reflect the state of the workspace.

	The -K flag suppresses keyword expansion.

	The -p flag populates the client workspace, but does not update the
	server to reflect those updates.  Any file that is already synced or
	opened will be bypassed with a warning message.  This option is very
	useful for build clients or when publishing content without the
	need to track the state of the client workspace.

	The -q flag suppresses normal output messages. Messages regarding
	errors or exceptional conditions are not suppressed.

	The -r flag allows open files which are mapped to new locations
	in the depot to be reopened in the new location.  By default, open
	workspace files remain associated with the depot files that they were
	originally opened as.

	The -s flag adds a safety check before sending content to the client
	workspace.  This check uses MD5 digests to compare the content on the
	clients workspace against content that was last synced.  If the file
	has been modified outside of P4's control then an error message is
	displayed and the file is not overwritten.  This check adds some extra
	processing which will affect the performance of the operation.  Clients
	with 'allwrite' and 'noclobber' set do this check by default.

	The -m flag limits sync to the first 'max' number of files. This
	option is useful in conjunction with tagged output and the '-n'
	flag, to preview how many files will be synced without transferring
	all the file data.

	The -E flag verifies that any changelists specified in the revRange
	are existing, submitted changes before performing the sync.

	The --parallel flag specifies options for parallel file transfer. If
	your administrator has enabled parallel file transfer by setting the
	net.parallel.max configurable, and if there are sufficient resources
	across the system, a sync command may execute more rapidly by
	transferring multiple files in parallel. Specify threads=N to request
	files be sent concurrently, using N independent network connections.
	The N threads grab work in batches; specify batch=N to control the
	number of files in a batch and/or batchsize=N to control the number of
	bytes in a batch. A sync that is too small will not initiate parallel
	file transfers; specify min=N to control the minimum number of files
	in a parallel sync and/or minsize=N to control the minimum number of
	bytes in a parallel sync.

	Auto parallel sync may be enabled by setting the net.parallel.threads
	configurable to the desired number of threads to be used by all sync
	commands. This value must be less than or equal to the value of
	net.parallel.max. Other net.parallel.* configurables may be specified
	as well, but are not required. See 'p4 help configurables' to see
	the options and their defaults. Auto parallel sync is turned off by
	unsetting the net.parallel.threads configurable. A user may override
	the configured auto parallel sync options on the command line, or may
	disable it via 'p4 sync --parallel=0'.

	The --use-stream-change flag specifies options for syncing stream
	clients using the stream view at or before change N.  If N=0, the
	current head version of the stream spec view is used.  If the
	StreamAtChange field is set in the client specification, then that
	value overrules any --use-stream-change value.

	If the configurable dm.sync.streamchange is set 1, then the
	--use-stream-change flag is used implicitly.  If a change specifier
	is used on the file arguments, then the stream view is derived from the
	stream spec version at or before the change specifier.  If multiple
	file arguments are used with different change specifiers, the greatest
	change number is used.  If no change specifiers are used, or at least
	one file in the filelist has no change specifier, then the stream view
	comes from the head version of the stream spec.  The
	--use-stream-change flag may also be used to override any change
	specifier on a file argument.

	The --sync-time specifies the file modification time to be set when
	updating the server metadata with the -k flag.

	See 'p4 help-graph sync' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpTag = { ErrorOf( ES_HELP, 91, E_INFO, EV_NONE, 0 ),
R"(
    tag -- Tag files with a label

    p4 tag [-d -g -n -U] -l label file[revRange] ...

	Tag associates the named label with the file revisions specified by
	the file argument.  After file revisions are tagged with a label,
	revision specifications of the form '@label' can be used to refer
	to them.

	If the file argument does not include a revision specification, the
	head revisions is tagged.  See 'p4 help revisions' for revision
	specification options.

	If the file argument includes a revision range specification, only
	the files with revisions in that range are tagged.  Files with more
	than one revision in the range are tagged at the highest revision.

	If multiple arguments are given, they are applied to the label
	sequentially, such that the last revision specified for a given
	depot file is the one that will be tagged with the label.

	The -d deletes the association between the specified files and the
	label, regardless of revision.

	The -n flag previews the results of the operation.

	Tag can be used with an existing label (see 'p4 help labels') or
	with a new one.  An existing label can be used only by its owner,
	and only if it is unlocked. (See 'p4 help label').

	The -U flag specifies that the new label should be created with the
	'autoreload' option (See 'p4 help label'). It has no effect on an
	existing label.

	To list the file revisions tagged with a label, use 'p4 files
	@label'.

	The -g flag is used on an Edge Server to update a global label.
	Configuring rpl.labels.global=1 reverses this default and causes this
	flag to have the opposite meaning.

	See 'p4 help-graph tag' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpTickets = { ErrorOf( ES_HELP, 92, E_INFO, EV_NONE, 0 ),
R"(
    tickets -- Display list of session tickets for this user

    p4 tickets

	'p4 tickets' lists all the tickets that have been granted to the
	user by 'p4 login'.
)"
};

ErrorId MsgHelp::HelpTopology = { ErrorOf( ES_HELP, 269, E_INFO, EV_NONE, 0 ),
R"(
    topology -- Display the list of connected servers (Technical Preview)

    p4 topology [-a | -t numOfDays] [-F filter] [-T field,...]
    p4 topology -D date [-y] [-e]
	                [-s serveraddress [-i serverID] [-p targetaddress]]
    p4 topology -d [-y] [-s serveraddress -i serverID [-p targetaddress]] |
	                [-c date [-e]] | [-l lastSeenDate [-e]]
    p4 topology -m [-y] -s serveraddress -i serverID
	                -S newserveraddress -I newserverID
    p4 topology -m [-y] -s serveraddress -i serverID -p targetaddress
	                -S newserveraddress -I newserverID -P newtargetaddress

	Reports the servers that are connected directly or indirectly to the
	innermost server, including an indicator of the server this command is
	running on. By default, reports only the latest configurations based on
	the Server address, Target server address, and ServerID for each
	service.

	The -a flag displays all the configurations of the services that have
	been recorded in the database.

	The -d flag indicates the delete marker. This flag ensures that the
	delete operation marks the addressed topology records as deleted
	instead of removing them from the topology table. If there are multiple
	entries with the same server address, destination address and the
	server id then the operation is performed on the server record with the
	latest timestamp of its creation. To execute the operation, specify the
	-y flag.

	The -c flag indicates the creation date. This flag helps in marking the
	topology records as deleted based on the specified date of creation.

	The -l flag indicates the last seen date. This flag helps in marking
	the records as deleted based on the specified date that represents the
	last time the server record was updated.

	The -D flag displays the preview of the deletion operation based on the
	date in epoch format. The date in this format can be found in the
	tagged topology output by running the command 'p4 -ztag topology'. To
	delete the specified records from the table, specify the -y flag.

	The -e flag ensures that the deletion operation also includes all the
	records dated earlier than the specified date.

	The -s flag indicates the server address. This flag is mandatory to
	delete records specific to that server address.

	The -p flag indicates the target address.
	
	The -i flag indicates the serverid.

	The -S flag indicates the new server address.

	The -P flag indicates the new target address.

	The -I flag indicates the new serverid.

	The -m flag indicates the move marker. This flag ensures that the move
	operation marks the specified source server record as being moved to
	the target server. This will preview the results unless the -y flag is
	also provided. If the topology record has the target address then
	it is mandatory to provide both the source target address (-p) and the
	new target address (-P).

	The -t flag displays all the recent configurations of the server
	services that have been recorded in the topology based on the requested
	past number of days from now.

	The -F filter flag limits output to records that match the filter
	pattern.  The filter pattern uses the same syntax as the -F filter
	flag on the 'p4 fstat' command.  Filtering is case-sensitive.

	The -T flag can be used to limit the fields that are displayed.

	This command requires that the user be an operator, or have 'super'
	access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpTrigger = { ErrorOf( ES_HELP, 79, E_INFO, EV_NONE, 0 ),
R"(
    trigger -- see 'p4 help triggers'
)"
};

ErrorId MsgHelp::HelpTriggers = { ErrorOf( ES_HELP, 80, E_INFO, EV_NONE, 0 ),
R"(
    triggers -- Modify list of server triggers

    p4 triggers
    p4 triggers -o
    p4 triggers -i

	'p4 triggers' edits the table of triggers, which are used for
	change submission validation, form validation, external authentication,
	external job fix integration, external archive integration,
	post-failover and post-failback activities, server response
	monitoring, and command policies.

	P4 extensions are a distinct means of customization, separate from
	triggers.  See 'p4 help extension'.

	Triggers are administrator-defined commands that the server runs
	to perform the following:

	Validate changelist submissions.

	    The server runs changelist triggers before the file transfer,
	    between file transfer and changelist commit, upon commit failure,
	    or after the commit.

	Validate shelve operations.

	    The server runs shelve triggers before files are shelved, after
	    files are shelved, or when shelved files have been discarded
	    (via shelve -d).

	Manipulate and validate forms.

	    The server runs form-validating triggers between generating
	    and outputting the form, between inputting and parsing the
	    form, between parsing and saving the form, or when deleting
	    the form.

	Authenticate or change a user password.

	    The server runs authentication triggers to either validate
	    a user password during login or when setting a new password.

	Intercept job fix additions or deletions.

	    The server run fix triggers prior to adding or deleting a fix
	    between a job and changelist.

	Access external archive files.

	    For files with the +X filetype modifier, the server runs an
	    archive trigger to read, write, or delete files in the archive.

	Command execution policy.

	    Command triggers can be specified to run before and after
	    processing of user requests.  Pre-execution triggers can
	    prevent the command from running.

	Server maintenance.

	    Journal rotation triggers can be specified to run after the
	    journals have rotated to the target server. These will only run
	    when the journal rotation is triggered by 'p4 admin journal',
	    'p4 admin checkpoint' or on replicas when they rotate to match
	    the master.

	Perform post-failover or post-failback activities.

	    The new master after a successful failover may run a failed-over
	    trigger to handle a DNS change, or any other changes that may
	    be required after the failover. A failed-back trigger may be
	    run after a successful failback to perform changes that may
	    be required after the failback.

	Monitor server responsiveness.

	    The 'p4 heartbeat' command that is used to ping a target server
	    may run 'heartbeat-missing', 'heartbeat-resumed', or
	    'heartbeat-dead' triggers based on the target server's responses.

	Validate graph pushes.

	    The server runs graph triggers before commit trees, lfs-files and
	    reference updates are received and after they have been committed
	    to the database.

	Perform alternative archive file transfer.

	    Pull-archive triggers can be used to replace the default
	    archive file transfer mechanism used by 'p4 pull'.

	The trigger form has a single entry 'Triggers:', followed by any
	number of trigger lines.  Each trigger line must be indented with
	spaces or tabs in the form. Triggers are executed in the order listed
	and if a trigger fails, subsequent triggers are not run.  A trigger
	succeeds if the executed command exits returning 0 and fails otherwise.
	Normally the failure of a trigger prevents the operation from
	completing, except for the commit and journal triggers, which run after
	the operation is complete, or the 'change-failed', 'heartbeat-missing',
	or 'heartbeat-resumed' triggers which are only advisory.

	Each trigger line contains a trigger name, a trigger type, a depot
	file path pattern matching where the trigger will be executed, a
	command name or form type and a command to run.

	Name:   The name of the trigger.  For change triggers, a run of the
		same trigger name on contiguous lines is treated as a single
		trigger so that multiple paths can be specified.  Only the
		command of the first such trigger line is used.

	Type:	When the trigger is to execute:

		archive:
		    Execute an archive trigger for the server to access
		    any file with the +X filetype modifier.

		auth-check:
		service-check:
		    Execute an authentication check trigger to verify a
		    user's password against an external password manager
		    during login or when setting a new password.

		auth-check-sso:
		    Facilitate a single sign-on user authentication. This
		    configuration requires two programs or scripts to run;
		    one on the client, the other on the server.

		    client:
		        Set the environment variable 'P4LOGINSSO' to point to
		        a script that can be executed to obtain the user's
		        credentials or other information that the server
		        trigger can verify.  The client-side script must
		        write the message to the standard output
		        (max length 128K).

		        Example:  P4LOGINSSO=/Users/joe/bin/runsso

		        The two variables available to this trigger are
		        %%P4PORT%% and %%serverAddress%%.  The distinction is
		        that serverAddress is the address of the target server
		        and the P4PORT is what the client is connecting to,
		        which might be an intermediate like a P4 Proxy.

		        These variables can be passed to the client script by
		        appending them to the client command string, as in:

		        P4LOGINSSO="/Users/joe/bin/runsso %%serverAddress%%"

		    server:
		        Execute an authentication (sso) trigger that gets
		        this message from the standard input and returns an
		        exit status of 0 (for verified) or otherwise failed.

		        Example:
		            sso auth-check-sso auth "/secure/verify %%user%%"

		    The user must issue the 'p4 login' command, but no password
		    prompting is invoked.  If the server determines that the
		    user is valid, they are issued a P4 ticket just as if they
		    had logged in with a password.

		    Pre-2007.2 clients cannot run a client-side single
		    sign-on.  Specifying an 'auth-check' trigger as a backup
		    for a user to gain access will prompt the user for a
		    password if it's an older client or P4LOGINSSO has not
		    been configured.

		    Unlike passwords which are encrypted, the sso message is
		    sent to the server in clear text.

		auth-invalidate:
		    Execute an authentication invalidate trigger to inform
		    external authentication systems when 'p4 logout' is run.

		auth-set:
		    Execute an authentication set trigger to send a new
		    password to an external password manager.

		auth-pre-2fa:
		auth-init-2fa:
		auth-check-2fa:
		    Execute a multi factor authentication trigger.
		    See 'p4 help mfa' for more details.

		bgtask:
		    Store named command invocations for use with 'p4 bgtask'.

		change-submit:
		    Execute pre-submit trigger after changelist has been
		    created and files locked but prior to file transfer.

		change-content:
		    Execute mid-submit trigger after file transfer but prior
		    to commit.  Files can be accessed by the 'p4 diff2',
		    'p4 files', 'p4 fstat', and 'p4 print' commands using
		    the revision specification '@=change', where 'change' is
		    the pending changelist number passed as %%changelist%%.

		change-commit:
		    Execute post-submit trigger after changelist commit.

		change-failed:
		    Executes only if the changelist commit failed.
		    Note that this trigger only fires on errors
		    occurring after a commit process has started. It does
		    not fire for early usage errors, or due to errors from
		    the submit form. In short, if a change-* trigger
		    could have run, then the change-failed trigger
		    will fire if that commit fails.

		command:
		    Execute pre/post trigger when users run commands.

		edge-submit:
		    Execute pre-submit trigger on Edge Server after changelist
		    has been created but prior to file transfer.

		edge-content:
		    Execute mid-submit trigger on Edge Server after file
		    transfer but prior to beginning submit on Commit Server.
		    This trigger could be used to perform an alternative file
		    transfer mechanism for copying archive files from an
		    Edge to a Commit Server.

		failed-over:
		    Execute only when a new master first starts up after
		    successful failover. This trigger could be used to
		    perform a DNS change, or any other activities needed
		    to support replication after failover. The special
		    variables %%standbyserverid%% and %%standbyserverport%%
		    expand to the serverid and serverport of the standby
		    before it became the new master. %%serverid%% and
		    %%serverport%% refer to the current serverid and
		    serverport of the new master. Note that most of the
		    usual trigger variables are not available for this
		    trigger as this fires at server startup before
		    any client connection. Also note that multiple
		    failed-over triggers run in the background asynchronously,
		    and that a failed failed-over trigger will not prevent
		    any other failed-over triggers from running.

		failed-back:
		    Execute only when a new master first starts up after
		    successful failback. This trigger could be used to
		    perform a DNS change, or any other activities needed
		    to support replication after failback. The special
		    variables %%standbyserverid%% and %%standbyserverport%%
		    expand to the serverid and serverport of the standby
		    before it became the new master. %%serverid%% and
		    %%serverport%% refer to the current serverid and
		    serverport of the new master. Note that most of the
		    usual trigger variables are not available for this
		    trigger as this fires at server startup before
		    any client connection. Also note that multiple
		    failed-back triggers run in the background asynchronously,
		    and that a failed failed-back trigger will not prevent
		    any other failed-back triggers from running.

		fix-add:
		    Execute fix trigger prior to adding a fix.  The special
		    variable %%jobs%% is available for expansion and must be
		    the last argument to the trigger as it expands to one
		    argument for each job listed on the 'p4 fix' command.

		fix-delete:
		    Execute fix trigger prior to deleting a fix.  The special
		    variable %%jobs%% is available for expansion and must be
		    the last argument to the trigger as it expands to one
		    argument for each job listed on the 'p4 fix -d' command.

		form-out:
		    Execute form trigger on generation of form.	 Trigger may
		    modify form.

		form-in:
		    Execute form trigger on input of form before its contents
		    are parsed and validated.  Trigger may modify form.

		form-save:
		    Execute form trigger prior to save of form after its
		    contents are parsed.

		form-commit:
		    Execute form trigger after it has been committed, allowing
		    access to automatically generated fields (jobname, dates
		    etc).  It cannot modify the form.  This trigger for job
		    forms is run by 'p4 job' and 'p4 fix' (after the status
		    is updated), 'p4 change' (if the job is added or deleted)
		    and 'p4 submit' (if the job is associated with the change).
		    The 'form-commit' trigger has access to the new job name
		    created with 'p4 job', while the 'form-in' and 'form-save'
		    triggers are run before the job name is created.  The
		    special variable %%action%% is available on the job
		    'form-commit' trigger command line, and is expanded when
		    the job is modified by a fix.

		form-delete:
		    Execute form trigger prior to delete of form after its
		    contents are parsed.

		heartbeat-dead:
		    Execute heartbeat dead trigger when 'p4 heartbeat' has
		    detected that the target server is non-responsive or dead.

		heartbeat-missing:
		    Execute heartbeat missing trigger when 'p4 heartbeat' has
		    detected the first missing response from its target server.

		heartbeat-resumed:
		    Execute heartbeat resumed trigger when 'p4 heartbeat' has
		    detected the first resumed response after missing responses
		    from its target server.

		journal-rotate:
		    Execute journal rotation trigger after the journal has been
		    rotated and after the database has been unlocked.

		journal-rotate-lock:
		    Execute journal rotation trigger after the journal has been
		    rotated, but before the database has been unlocked. All
		    commands against the server will wait for the trigger to
		    complete before running: do not run commands against the
		    server from this trigger.

		pull-archive:
		    Execute pull archive trigger to perform archive file
		    transfer from within the trigger during 'p4 pull'.
		    This file transfer replaces the default file transfer
		    mechanism used by 'p4 pull'.

		shelve-submit:
		    Execute pre-shelve trigger after changelist has been
		    created but prior to file transfer. Also executed for
		    reshelve command.

		shelve-commit:
		    Execute post-shelve trigger after files are shelved or
		    reshelved.

		shelve-delete:
		    Execute shelve trigger prior to discarding shelved files.

		push-submit:
		    Execute trigger during a push, fetch, or unzip operation,
		    after changelist has been created on the destination
		    server and files are locked but prior to file transfer.

		push-content:
		    Execute trigger during a push, fetch, or unzip operation,
		    after file transfer but prior to commit. Files can be
		    accessed by the trigger using any of the 'p4 diff2',
		    'p4 files', 'p4 fstat', and 'p4 print' commands with
		    the revision specification '@=change', where 'change' is
		    the pending changelist number passed as %%changelist%%.

		push-commit:
		    Execute trigger during a push, fetch, or unzip operation,
		    after changelist commit.

		graph-push-start:
		    Execute trigger prior to any data being transferred as
		    part of a 'git push' operation through the connector or
		    a 'p4 submit' to a graph depot.

		graph-push-complete:
		    Execute trigger after a 'git push' operation has
		    completed through the connector or a 'p4 submit' to a
		    graph depot.

		graph-push-reference:
		    Execute trigger prior to a reference being created or
		    updated as part of a 'git push' operation through the
		    connector or a 'p4 submit' to a graph depot.

		graph-push-reference-complete:
		    Execute trigger after a reference has been created or
		    updated as part of a 'git push' operation through the
		    connector or a 'p4 submit' to a graph depot.

		graph-lfs-push:
		    Execute trigger prior to any LFS files being transferred.

)"
R"(
	Path:   For change and submit triggers, a file pattern to match files
		in the changelist.  This file pattern can be an exclusion
		mapping (-pattern), to exclude files.  For form triggers, the
		name of the form (branch, client, etc).  For fix triggers
		'fix' is required as the path value.  For authentication
		triggers (including multi factor authentication triggers),
		'auth' is required as the path value. For archive triggers,
		a file pattern to match the name of the file being accessed
		in the archive.  Note that, due to lazy copying when branching
		files, the name of the file in the archive can not be the same
		as the name of the file in the depot.  For command triggers,
		use the name of the command to match, e.g. 'pre-user-$cmd'
		or a regular expression, e.g. '(pre|post)-user-add'.  For
		journal rotation triggers, set the path value to 'any' if
		the trigger should run on all servers or a suitable server.id
		or cluster.id may be used to restrict the trigger to running on
		either a specific server or a named group of servers.  For
		pull-archive triggers, 'pull' is required as the path value.
		For graph triggers, set the path to a file pattern that matches
		the desired repos.  The path should end with a wildcard even if
		only matching a single repo.  For 'bgtask' triggers, use a path
		of 'unset'. For 'failed-over' triggers, use 'failed-over' as
		the path. For 'failed-back' triggers, use 'failed-back' as the
		path. For 'heartbeat-dead', 'heartbeat-missing', and
		'heartbeat-resumed' triggers, use 'heartbeat' as the path.

	Command: The OS command to run for validation.  If the command
		contains spaces, enclose it in double quotes.  The
		following variables are expanded in the command string.
		Use of the triggers.io configurable with a value greater than
		zero is recommended, as some vars may be empty or contain
		shell metacharacters.
		    %%//depot/trigger.exe%% -- depot paths within %%vars%%
		    are filled with the path to a temporary file containing
		    the referenced file's contents.  Only standard and stream
		    depot files whose contents is available are valid.
		    %%argc%% -- number of command arguments
		    %%args%% -- command argument string
		    %%argsQuoted%% -- command argument string, CSV delimited
		    %%client%% -- the client issuing the command
		    %%clientcwd%% -- client current working directory
		    %%clienthost%% -- the hostname of the client
		    %%clientip%% -- the IP address of the client
		    %%clientprog%% -- the program name of the client
		    %%clientversion%% -- the version of the client
		    %%command%% -- name of command being run
		    %%groups%% -- list of groups user is a member of
		    %%intermediateService%% -- presence of a Broker/Proxy/etc
		    %%language%% -- user-specified language, defaults to 'unset'
		    %%maxErrorSeverity%% -- highest error seen for this cmd
		    %%maxErrorText%% -- text and errno for highest error
		    %%maxLockTime%% -- user-specified override of group value
		    %%maxOpenFiles%% -- user-specified override of group value
		    %%maxMemory%% -- user-specified override of group value
		    %%maxResults%% -- user-specified override of group value
		    %%maxScanRows%% -- user-specified override of group value
		    %%quote%% -- double quote character
		    %%serverhost%% -- the hostname of the server
		    %%serverid%% -- the value of the server's server.id
		    %%serverip%% -- the IP address of the server
		    %%servername%% -- the value of the server's $P4NAME
		    %%serverpid%% -- the PID of the server
		    %%serverport%% -- the IP address:port of the server
		                    preceded by the transport prefix,
		                    if needed (i.e. P4PORT)
		    %%serverroot%% -- the value of the server's $P4ROOT
		    %%serverservices%% -- the services provided by the server
		    %%serverVersion%% -- the server's version string
		    %%target%% -- target changelist for reshelve command
		    %%terminated%% -- if the command was forced to quit early
		    %%termReason%% -- reason for early termination
		    %%triggerMeta_action%% -- command to execute by trigger
		    %%triggerMeta_depotFile%% -- third field in trigger def.
		    %%triggerMeta_name%% -- name from trigger definition
		    %%triggerMeta_trigger%% -- second field in trigger definition
		    %%user%% -- the user issuing the command
		    %%sessionsActive%% -- Number of active client commands.
		    %%sessionsPaused%% -- Number of paused client commands.
		    %%pauseRateCPU%% -- Percentage of commands blocked for CPU.
		    %%pauseRateMem%% -- Percentage of commands blocked for mem.
		    %%pressureMem%% -- low/medium/high memory pressure
		    %%pressureCPU%% -- high CPU pressure
		    %%changelist%% -- the changelist being submitted
		    %%changeroot%% -- the root path of files submitted
		    %%oldchangelist%% -- the pre-commit changelist number
		                    For push-* triggers, oldchangelist is
		                    the changelist number in the source
		                    server.

		        (More information can be gathered about the
		        changelist being submitted by running
		        'p4 describe %%changelist%%'.)

		    %%formfile%% -- path to temp file containing form
		    %%formname%% -- the form's name (branch name, etc)
		    %%formtype%% -- the type of form (branch, etc)
		    %%action%% -- added/deleted/submitted on job form-commit
		    %%specdef%% -- the definition of form's spec

		    %%jobs%% -- list of job names for fix triggers

		    %%op%% -- read/write/delete for archive access
		    %%file%% -- name of archive file
		    %%rev%% -- revision of archive file

		    If the command was sent via a proxy, broker, or replica:
		    %%peerhost%% -- the hostname of the proxy/broker/replica
		    %%peerip%% -- the IP address of the proxy/broker/replica
		    If the command was sent directly, %%peerhost%% and
		    %%peerip%% match %%clienthost%% and %%clientip%%.

		    For a change-* trigger in a distributed installation,
		    %%submitserverid%% -- the server.id where submit was run

		    For an edge-content trigger using external file transfer:
		    %%triggerdir%% -- pull.trigger.dir used for tmp files

		    For a post-rmt-Push trigger:
		    %%firstPushedChange%% -- first new changelist number
		    %%lastPushedChange%% -- last new changelist number

		    For a journal-rotate* trigger:
		    %%journal%% -- the name of the newly rotated journal
		    %%checkpoint%% -- the name of the newly created checkpoint
		                    if a checkpoint was taken during the
		                    journal rotation

		    For a pull-archive trigger:
		    %%archiveList%% -- filename containing files to be pulled

		    For a graph-push* or graph-lfs-push trigger:
		    %%depotName%% -- the depot the repo resides in
		    %%repo%% -- the repo
		    %%repoName%% -- the name of the repo
		    %%pusher%% -- the user credited with the push

		    Additionally, for a graph-push-reference* trigger:
		    %%reference%% -- the reference being created/updated
		    %%newValue%% -- the new value of the reference
		    %%oldValue%% -- the old value of the reference
		    %%refType%% -- the type of the reference (branch/tag/etc)
		    %%refFlags%% -- properties of the ref. (fast-fwd/delete/etc)

		    For an auth-* trigger:
		    %%user%%     -- the user's username
		    %%fullname%% -- the user's fullname
		    %%email%%    -- the user's email address

		    Additionally, for an auth-*-2fa trigger:
		    %%host%%     -- the user's host's IP address
		    %%method%%   -- the authentication method from list-methods
		                  (may be set to "unknown")
		    %%scheme%%   -- the authentication schemed set by init-auth
		                  (may be set to "unknown")
		    %%token%%    -- the stashed token from the last init-auth
		                  (may be empty)

		    For an auth-invalidate trigger:
		    %%user%%     -- the user's username
		    %%fullname%% -- the user's fullname
		    %%email%%    -- the user's email address
		    %%host%%     -- the host IP address of the ticket being
		                  invalidated or 'all-hosts' for 'p4 logout -a'
		    %%2fa%%      -- 'true' if the multi factor authentication
		                  status was reset, 'false' if the user doesn't
		                  use multi factor authentication and 'only2fa'
		                  for 'p4 logout -2'

		    For a failed-over trigger:
		    %%standbyserverid   -- the serverid of the standby before
		                          it became the master during failover
		    %%standbyserverport -- the serverport of the standby before
		                          it became the master during failover

		    For a failed-back trigger:
		    %%standbyserverid   -- the serverid of the standby before
		                          it became the master during failback
		    %%standbyserverport -- the serverport of the standby before
		                          it became the master during failback

		    For the heartbeat-* triggers:
		    %%targetport%% -- the serverport of the target server
		                      being monitored

		    Note that not all variables are available for every
		    trigger type.  E.g. argc and argv only show up for
		    pre-user-$cmd and change-submit (and so on), but not for
		    post-user-$cmd or change-commit.

		The command's standard input depends on the value of the
		triggers.io configurable.  When it is set to zero, stdin is
		empty for change, shelve, fix and command triggers, it
		is the file content for the archive, and password for auth
		triggers.  When triggers.io is set to 1, stdin is a textual
		dictionary containing connection information that the trigger
		must read (with the exception of archive/auth triggers,
		which behave the same as when triggers.io=0.)

		If the command fails, the command's standard output (not
		error output) is sent to the client as the text of a trigger
		failure error message.

		If the command succeeds, the command's standard output is
		sent as an unadorned message to the client for all triggers
		except archive triggers; for archive triggers, the command's
		standard output is the file content.

	The -o flag writes the trigger table to the standard output.
	The user's editor is not invoked.

	The -i flag reads the trigger table from the standard input.
	The user's editor is not invoked.

	'p4 triggers' requires 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpTypeMap = { ErrorOf( ES_HELP, 81, E_INFO, EV_NONE, 0 ),
R"(
    typemap -- Edit the filename-to-filetype mapping table

    p4 typemap
    p4 typemap -o
    p4 typemap -i

	'p4 typemap' edits a name-to-type mapping table for 'p4 add', which
	uses the table to assign a file's filetype based on its name.

	The typemap form has a single field, 'TypeMap', followed by any
	number of typemap lines.  Each typemap line contains a filetype
	and a depot file path pattern:

	Filetype:   See 'p4 help filetypes' for a list of valid filetypes.

	Path:       Names to be mapped to the filetype.  The mapping is
		    a file pattern in depot syntax.  When a user adds a file
		    matching this pattern, its default filetype is the
		    file type specified in the table.  To exclude files from
		    the typemap, use exclusionary (-pattern) mappings.
		    To match all files anywhere in the depot hierarchy,
		    the pattern must begin with '//...'.  To match files
		    with a specified suffix, use '//.../*.suffix' or
		    use '//....suffix' (four dots).

	Later entries override earlier entries. If no matching entry is found
	in the table, 'p4 add' determines the filetype by examining the file's
	contents and execution permission bits.

	The -o flag writes the typemap table to standard output. The user's
	editor is not invoked.

	The -i flag reads the typemap table from standard input. The user's
	editor is not invoked.

	'p4 typemap' requires 'admin' access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpUndo = { ErrorOf( ES_HELP, 203, E_INFO, EV_NONE, 0 ),
R"(
    undo -- Undo a range of revisions

    p4 undo [-n -K] [-c changelist#] file[revRange]

	'p4 undo' opens files in order to undo a set of previously submitted
	changes.  The 'undone' changes remain a part of the history, but the
	new revisions submitted after 'p4 undo' will reverse their effect.

	If a single revision is specified, the specified revision is undone.
	If a revision range is specified, the entire range is undone.

	The workspace files opened by 'p4 undo' will be synced to the revision
	prior to the range being undone, and opened at the most recent undone
	revision.  Files that are opened at a revision prior to the head must
	be resolved prior to submit -- run 'p4 sync' and 'p4 resolve'.

	The -n flag previews the operation without changing any files.

	The -K flag suppresses keyword expansion when updating ktext files on
	the client.

	If -c changelist# is included, files are opened in the specified
	pending changelist instead of the default changelist.

	See 'p4 help-graph undo' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpUnlock = { ErrorOf( ES_HELP, 82, E_INFO, EV_NONE, 0 ),
R"(
    unlock -- Release a locked file, leaving it open

    p4 unlock [-c | -s changelist# | -x] [-f] [file ...]
    p4 -c client unlock [-f] -r

	'p4 unlock' releases locks on the specified files.

	The changelist flag and file specification limit the files to be
	unlocked. If no file specification is given and no changelist is
	specified, all open files are unlocked.

	If a changelist is specified, only those files open in that
	changelist are unlocked.

	If a file specification is given, only those files are unlocked.

	If both changelist and file specification are provided, only the
	matching files in the specified changelist are unlocked.

	The -s flag unlocks files from a shelved changelist caused by an
	aborted 'submit -e' operation. The -c flag applies to opened files
	in a pending changelist locked by 'p4 lock' or by a failed submit
	of a change that is not shelved.

	By default, files can be unlocked only by the changelist owner who
	must also be the person who has the files locked. The -f flag
	enables you to unlock files in changelists owned by other users.
	The -f flag requires 'admin' access, which is granted by 'p4
	protect'.

	If 'p4 unlock' is called from an Edge Server, any corresponding
	files locked globally via 'p4 lock -g' by that client will be
	unlocked on the Commit Server.

	The -x option unlocks files that are opened 'exclusive' but are
	orphaned (see 'p4 opened -x'). This option only applies to a
	distributed installation where global tracking of these file types
	is necessary across servers. On the commit server, an administrator
	may specify 'p4 -c client unlock -f -x [file...]' to unlock the
	global exclusive locks of files which aren't marked orphaned.

	If a push command from a remote server to this server fails, files
	can be left locked on this server, preventing other users from
	submitting changes to those files. Files may also be left locked on
	a Commit Server from a failed submit or unlock. In these cases, the
	user who issued the failed command can use the second syntax form.
	The -r flag indicates the remote or commit server holding the lock.
	The -c global flag indicates the name of the client that issued the
	failed command. The -f flag allows the administrator to force the
	lock to be released.

	See 'p4 help-graph unlock' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpUnshelve = { ErrorOf( ES_HELP, 120, E_INFO, EV_NONE, 0 ),
R"(
    unshelve -- Restore shelved files and/or a stream spec from a pending
	change into a workspace

    p4 unshelve -s changelist# [options] [-As| [-Af] file ...]
	Options: [-f -n] [-c changelist#]
	         [-b branch|-S stream [-P parent]]

	'p4 unshelve' retrieves shelved files from the specified pending
	changelist, opens them in a pending changelist and copies them
	to the invoking user's workspace.  Unshelving files from a pending
	changelist is restricted by the user's permissions on the files.
	A successful unshelve operation places the shelved files on the
	user's workspace with the same open action and pending integration
	history as if it had originated from that user and client.

	Unshelving a file over an already opened file is permitted if both
	shelved file and opened file are opened for 'edit'. In a distributed
	environment, the shelf must either be promoted or have been created
	on the same edge server. After unshelving, the workspace file is
	flagged as unresolved, and 'p4 resolve' must be run to resolve the
	differences between the shelved file and the workspace file.

	Unshelving a file opened for 'add' when the file already exists
	in the depot will result in the file being opened for edit. After
	unshelving, the workspace file is flagged as unresolved, and
	'p4 resolve' must be run to resolve the differences between the
	shelved file and the depot file at the head revision. Note that
	unshelving a file opened for 'add' over an already opened file is
	not supported.

	The -s flag specifies the number of the pending changelist that
	contains the shelved files.

	If a file pattern is specified, 'p4 unshelve' unshelves files that
	match the pattern.

	The -A flag specifies whether files and/or the stream spec (if any)
	associated with the shelved changelist should be unshelved to the
	current client; by default, the unshelve command acts on both the
	files (-Af) and the stream spec (-As).  (See 'p4 help streamcmds'.)

	The -b flag specifies a branch spec that the shelved files will be
	mapped through prior to being unshelved, allowing files to be shelved
	in one branch and unshelved in another.  As with unshelving into an
	open file, it may be necessary to run 'p4 resolve'. In a distributed
	environment, an additional requirement is that the shelve was created
	on the same edge server.

	The -S flag uses a generated branch view to map the shelved files
	between the specified stream and its parent stream.  The -P flag
	can be used to generate the view using a parent stream other than
	the actual parent.

	The -c flag specifies the changelist to which files are unshelved.
	By default,  'p4 unshelve' opens shelved files in the default
	changelist.

	The -f flag forces the clobbering of any writeable but unopened files
	that are being unshelved.

	The -n flag previews the operation without changing any files or
	metadata.
)"
};

ErrorId MsgHelp::HelpUser = { ErrorOf( ES_HELP, 83, E_INFO, EV_NONE, 0 ),
R"(
    user -- Create or edit a user specification

    p4 user [-f] [name]
    p4 user -d [-f | -F] name
    p4 user -D [-f] [-y] name
    p4 user -o [name]
    p4 user -i [-f]

	Create, edit, or delete a user specification.
	The specification form is put into a temporary file and the editor
	(configured by the environment variable $P4EDITOR) is invoked.

	Normally, a user specification is created automatically the first
	time that the user issues any command that updates the depot. The
	'p4 user' command is typically used to edit the user's subscription
	list for change review.

	The user specification form contains the following fields:

	User:        The user name (read-only).

	Email:       The user's email address (Default: user@client).

	Update:      The date the specification was last modified (read-only).

	Access:      The date that the user last issued a client command.

	FullName:    The user's real name.

	JobView:     Selects jobs that are displayed when the user creates
		     a changelist. These jobs can be closed automatically
		     when the user submits the changelist. For a description
		     of jobview syntax, see 'p4 help jobview'

	Reviews:     The subscription list for change review.  There is no
	             limit on the number of lines that this field can contain.
		     You can include the following wildcards:

			 ...            matches any characters including /
			 *              matches any character except /

	Password:    The user's password.  See 'p4 help passwd'.

	Type:        Must be 'service', operator, or 'standard'. Default is
		     'standard'. Once set, the user type cannot be changed.

	AuthMethod:  Must be 'perforce' or 'ldap' and may include the '+2fa'
		     modifier.  The default is 'perforce' unless overridden
		     by the 'auth.default.method' configurable; see
		     'p4 help configurables'.  AuthMethod can only be changed
		      when the -f flag has been provided.

	The -d flag deletes the specified user (unless the user has files
	open).

	The -D flag deletes the specified user, the user's pending
	changelists, and these deleted changelists' associations to
	jobs and will revert the user's opened files. If the -F flag
	is also used, it will remove the user from the protections
	table, and remove the user from its groups. The -D flag will
	not revert files in this user's clients opened by other users,
	nor delete such clients, unless the -f or -F option is specified.
	Shelved files will not be affected. Use the -y flag to perform
	the operation. Without the -y flag, a preview of the actions
	is reported.

	The -o flag writes the user specification to the standard output.
	The user's editor is not invoked.

	The -i flag reads a user specification from the standard input.
	The user's editor is not invoked.

	The -f flag forces the creation, update or deletion of the specified
	user, and enables you to change the Last Modified date. By default,
	users can only delete or modify their own user specifications.  The
	-f flag requires 'super' access, which is granted by 'p4 protect'.

	The -F flag, used only with -d, forces the deletion of the
	specified user, and also removes the user from the protections
	table and from all groups. This command fails if removing the
	user from any group would cause the group to be deleted. In that
	case the group should be deleted prior to deleting the user.
	This flag can also be used with -D for the same purpose but
	requires -y for the action to be performed. The -F flag
	requires 'super' access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpUsers = { ErrorOf( ES_HELP, 84, E_INFO, EV_NONE, 0 ),
R"(
    users -- List P4 users

    p4 users [-l -a -r -c] [-m max] [user ...]

	Lists all P4 users or users that match the 'user' argument.
	The report includes the last time that each user accessed the system.

	The -m max flag limits output to the first 'max' number of users.

	The -a flag includes service and operator users in the output.

	The -l flag includes additional information in the output.  The -l
	flag requires 'super' access, which is granted by 'p4 protect'.

	The -r and -c flags are only allowed on replica servers.  When
	-r is given only users who have used a replica are reported and
	when -c is given only the user information from the central server
	is reported.  Otherwise on a replica server, the user list will
	be slightly different from the master server as the user access times
	will reflect replica usage or master usage whichever is newer.
)"
};

ErrorId MsgHelp::HelpVerify = { ErrorOf( ES_HELP, 85, E_INFO, EV_NONE, 0 ),
R"(
    verify -- Verify that the server archives are intact

    p4 verify [-u | -v ] [-m max -q -s -z -X -b N] file[revRange]
    p4 verify [-t [ -r ] ] [ --only BAD|MISSING ] [-U | -A]
                [ --compressed 0|1 ] [-z -m max -q -s -X -b N] file[revRange]
    p4 verify -S [-v | -t [ -r ] [ --only BAD|MISSING ] ]
                [ --compressed 0|1 ] [-m max -q -X -b N] file
    p4 verify -U [-u | -v | -z] [-m max -q -s -X -b N] file
    p4 verify -A [-u | -v | -z] [-m max -q -s -X -b N] file[revRange]
    p4 verify -Z [ -q ] file
    p4 verify -R [-m max -q -s -X -b N] [ -v | --only BAD|MISSING ]
                file[revRange]

	For each revision of the specified files, 'p4 verify' reports
	revision-specific information and an MD5 digest of the revision's
	contents.  See 'p4 help revisions' for help specifying revisions.

	By default, 'p4 verify' computes and displays the digest of each
	revision.  If a revision cannot be reproduced (for example, if the
	file is missing from the archive), the revision's output line ends
	with MISSING!  If there is a saved digest, 'p4 verify' compares it
	with the computed one. If they differ, the output line ends with BAD!

	The -m max flag specifies the maximum number of revisions to process.
	This option can be used with the -u flag to compute and save digests
	for a limited number of revisions in each 'p4 verify' invocation.

	When the '-t' flag is used on a replica any damaged or missing
	revisions are scheduled for transfer from the P4TARGET server.
	On a commit server the damaged or missing revisions will be scheduled
	for transfer from the first replica that confirms it has a valid
	archive file for that revision.
	The '-t' option cannot be used with the '-v', '-Z' or '-u' options.

	The -r flag is only used with the '-t' option. When the '-r'
	option is specified, then revisions of the text type (contained
	within an RCS file) are not transferred.

	The --only filter flag, where filter can be either 'BAD' or 'MISSING'
	selects which type of error is of interest. By selecting
	the 'MISSING' filter, only missing revisions will be reported
	and checksum calculations will be avoided. The '--only' flag can
	be combined with the '-t' flag to restrict the revisions
	to be transferred. The '--only' option cannot be used
	with the '-v', '-Z' or '-u' options.

	The --compressed flag may be used to override how revisions stored
	as compressed archives are verified. By default, if the digest
	of the compressed archive has been recorded, the archive is verified
	without extracting the content, falling back to verify the extracted
	content in the case that the compressed checksum doesn't match.
	Specifying '--compressed 0' will skip the attempt to verify the archive
	based on the compressed checksum, verifying only the extracted content.
	Specifying '--compressed 1' will prevent the second verify attempt
	against the extracted content, raising a verify error immediately.

	The -u flag computes and saves the digest only for revisions that
	have no saved digest.

	The -v flag computes and saves the digest for each revision, regardless
	of whether the revision already has a saved digest.  This option can
	be used to update the saved digest if the archive was deliberately
	changed. The '-u' and '-v' options are mutually exclusive.

	The -z flag optimizes digest computations by skipping revisions that
	have already been computed in the current pass. This option is useful
	when the specified files contain lazy copies.  The resulting output
	might report a lazy copy revision if it is the first revision in the
	sort order to access a common archive file. This option cannot be used
	with the '-v' or '-u' options.

	The -q flag minimizes command output, displaying only errors from
	mismatched digests or unreproducible revisions.

	The -s flag specifies that the file size should also be verified. The
	-v flag implies the -s flag.

	The -X flag specifies that files with the +X filetype modifier should
	be skipped.

	The -Z flag specifies that the digest and file length check should be
	performed by using the storage table as opposed to the revision table.
	Using the storage table should generally be faster as the lazy copied
	files will only be checked once. The output is slight different as the
	storage table does not contain the action and change fields available
	in the revision table. Instead the reference count value is displayed.
	Note that the 'file' argument is subtly different with this flag
	as a lazy copied depot file will not be selectable. To demonstrate this
	issue consider file //x/x with a lazy copy with the path //y/y. Calling
	'p4 verify -Z //y/...' will not verify any files as there will not be
	a storage record that matches the //y path. Whereas a
	'p4 verify //y/...' will use the revision table and find and check
	the //x/x file. You can only combine the -q option with this flag.

	By default, 'p4 verify' processes files in batches of 10000 files at
	a time. The -b flag specifies the batch size. Specify -b 0 to disable
	batching and process all files in a single batch. If the -z flag is
	specified, the -b flag is ignored and all files are processed in a
	single batch.

	The following command verifies all revisions of all archive files:

		p4 verify -qz //...

	Saved digests are used by 'p4 diff' rather than recomputing them.

	The -S flag verifies shelved files. If -S is specified, the only
	valid revision specifier is @=change, specifying a single shelf.
	In a distributed installation, 'verify -S' should be run on the
	Edge Server where the shelf was created. If the shelf has been
	promoted, 'verify -S' may also be run on the Commit Server.

	The -U flag verifies files in the unload depot (see 'p4 help unload').

	The -A flag verifies files in the archive depot (see
	'p4 help archive').
	
	The -R flag provides a repair mode to help with disaster recovery.
	If an archive file is missing, this mode will check if an identical
	file is shelved, the matching archive is then copied.

	NOTE: Task stream files require special handling when using the
	verify command. You must pay attention to whether you are running
	the command with a client switched to that task stream or not. If
	the verify command is run without using a client bound to that task
	stream, it will only verify the promoted task stream files, while if
	a task stream client is used, the command will verify all the task
	stream files. When using 'verify -v' to rewrite the digest or size
	information for a task stream file, run the command twice: first run
	the command with a non-task-stream client, then re-run the command
	with a client bound to the task stream. This process allows the
	server to process the database data in the correct order.

	'p4 verify' requires that the user be an operator or have 'admin'
	 access, which is granted by 'p4 protect'.

	See 'p4 help-graph verify' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpWebServer = { ErrorOf( ES_HELP, 281, E_INFO, EV_NONE, 0 ),
R"(
    webserver -- Control the P4 web server

    p4 webserver start -p [tcp:][ip:]port
    p4 webserver stop -p [ip:]port
    p4 webserver show

	Use 'p4 webserver start' to start the P4 web server. Use '-p' to
	specify the IP address and port that the web server will listen on.
	By default, the web server detects if an SSL certificate is configured
	for the P4 Server and if one is configured, the web server starts
	in HTTPS mode using the same certificate as the P4 Server. To force the
	web server to start in HTTP mode, Use the 'tcp:' prefix.

	Use 'p4 webserver stop' to stop a running P4 web server. Use '-p' to
	specifies the ip address and port of the web server to stop.

	'p4 webserver show' displays information about running P4 web
	servers.
)"
};

ErrorId MsgHelp::HelpWhere = { ErrorOf( ES_HELP, 86, E_INFO, EV_NONE, 0 ),
R"(
    where -- Show how file names are mapped by the client view

    p4 where [file ...]

	Where shows how the specified files are mapped by the client view.
	For each argument, three names are produced: the name in the depot,
	the name on the client in P4 syntax, and the name on the client in
	local syntax.

	If the file parameter is omitted, the mapping for all files in the
	current directory and below) is returned.

	Note that 'p4 where' does not determine where any real files reside.
	It only displays the locations that are mapped by the client view.
)"
};

ErrorId MsgHelp::HelpTunables = { ErrorOf( ES_HELP, 106, E_INFO, EV_NONE, 0 ),
R"(
	For information on server configuration variables, see
	'p4 help configurables'.

	For information about unsupported server parameters, see the
	'configurables' section of 'p4 help undoc'. Change these values only
	as directed by Technical Support.
)"
};

ErrorId MsgHelp::HelpDbschema = { ErrorOf( ES_HELP, 109, E_INFO, EV_NONE, 0 ),
R"(
    dbschema -- Report meta database information

    p4 dbschema [-A] [tablename[:tableversion]] ...

	'p4 dbschema' reports database structure information about the
	P4 metadata.  Super permission is required to execute this command.

	The command reports schema information for the metadata tables.
	By default, all current tables are reported. You can optionally specify
	table names and versions. The results are returned as tagged output.
	Table names are the file names that start with 'db.'

	The -A flag causes all versions of the tables without version
	specifiers to be reported.
)"
};

ErrorId MsgHelp::HelpExport = { ErrorOf( ES_HELP, 112, E_INFO, EV_NONE, 0 ),
R"(
    export -- Extract journal or checkpoint records

    p4 export -c token [-J prefix] [-f] [-l lines] [-F filter]
		[-T tableexcludelist] [-P filterpattern]
    p4 export -j token [-J prefix] [-f] [-l lines] [-F filter]
		[-T tableexcludelist] [-P filterpattern]
    p4 export -j token [-J prefix] -r [-F filter]
		[-T tableexcludelist] [-P filterpattern]

	'p4 export' extracts journal or checkpoint records from the P4
	metadata.  Super permission is required to execute this command.

	This command extracts records from database journals or checkpoints.
	By default, the records are returned in tagged output.  Compressed
	journals or checkpoints are not supported.

	The -j token flag specifies a journal number and optional position
	(in the form: journal number/offset) from which to start exporting.

	The -c token flag specifies a checkpoint number and optional position
	(in the form: checkpoint number#offset) from which to start exporting.

	The -f flag formats non-textual datatypes appropriately.

	The -r flag changes the output to raw format (journals only).

	The -J prefix flag specifies a file name prefix to match the one used
	with p4d -jc <prefix>.

	The -l flag limits the number of lines (records) exported.

	The -P filterpattern flag limits output to records that match the
	filter pattern. Multiple filter patterns can be specified using
	multiple -P flags. Each filter pattern should specify either a client
	filter or a depot filter, and should specify whether the pattern is
	to be included or excluded, using the syntax:

	        -Pic://client/pattern  -- client records to include
	        -Pxc://client/pattern  -- client records to exclude
	        -Pif://depot/pattern   -- depot records to include
	        -Pxf://depot/pattern   -- depot records to exclude

	If at least one include pattern is specified using -P, all records
	that are output will match one of the include patterns. If at least
	one exclude pattern is specified using -P, all records that are
	output will match none of the exclude patterns.

	The -F filter flag limits output to records that match the filter
	pattern.  The filter pattern uses the same syntax as the -F filter
	flag on the 'p4 fstat' command.  Filtering is case-sensitive.

	        Example: -F "table = db.protect"

	The -T flag specifies a list of database tables that are excluded
	from the exported output.  To specify multiple tables, double-quote
	the list and separate the table names with spaces. The table names
	must start with "db.". Table names can also be separated by commas.

	Note: to delimit transactions and indicate the end of the journal,
	additional fields are added to the tagged output.

	Note: The final record is a position record that specifies the
	position where subsequent records are located.  When processing a
	checkpoint, a position can be either a continuation of the checkpoint
	position (if the line count option was supplied) or a journal number.
	When processing a journal, if the last record processed is the last
	record the server has written, an 'atend' tag is inserted.  This
	tag indicates to a polling process that it must wait for more data
	before issuing the next export command.  Note that this final record
	is not filtered.

	Note: Journal 'ex' records can have a 'markers' tag, with the value
	'complete'.  This tag indicates that, when that transaction ended,
	there were no known outstanding transactions.

	Note: Raw journal output places the raw journal text in a 'data' tag.
	Other tags can indicate positions where transactions were known to be
	complete in the journal.
)"
};

ErrorId MsgHelp::HelpReplicate = { ErrorOf( ES_HELP, 121, E_INFO, EV_NONE, 0 ),
R"(
    replicate -- poll for journal changes and apply them to another server

    p4 replicate [-j token][-s statefile][-i interval][-k -x -R]
	          [-J prefix][-T tables excluded][-o output][command]

	This client command polls for new journal entries from a server and
	outputs them or invokes another command that pipes the journal records
	to a subprocess.

	The -j token flag specifies a journal number and optional position
	(journal number/offset) from which to start replicating.

	The -J prefix flag specifies a file name prefix to match the one used
	with p4d -jc <prefix>.

	The -s statefile flag specifies a file that tracks the most recent
	journal position.

	The -i interval flag specifies a polling interval.  A zero interval
	causes the client to check once and exit (default value is 2 seconds).

	The -k flag configures the pipe to the sub-process command to remain
	open across polling intervals.  By default, the pipe to the sub-
	process is restarted for every record batch.

	The -x flag configures 'p4 replicate' to terminate when a
	journal rotation is detected.

	The -R flag causes the command to attempt reconnection to the server
	in the event of a connection loss or serious error.  An interval
	must be given with this option.

	The -T flag specifies a quoted, space separated list of table names
	which should not be transferred for replication.

	The -o output flag specifies an output file where journal records
	are written in addition to the sub-process command.

	See P4 Knowledge Base article 1099 for detailed usage.

	Super permission is required to run this command.
)"
};

ErrorId MsgHelp::HelpReshelve = { ErrorOf( ES_HELP, 193, E_INFO, EV_NONE, 0 ),
R"(
    reshelve -- Copy shelved files to a new or existing shelf.

    p4 reshelve [-p] -s changelist# [file ...]
    p4 reshelve [-f] [-p] [-As | -Af] -s changelist# -c changelist# [file ...]

	'p4 reshelve' copies shelved files from an existing shelf into
	either a new shelf or one that has already been created. This
	command leaves the source shelf intact.

	If a file pattern is specified, 'p4 reshelve' shelves the files that
	match the pattern.

	The -s flag specifies the shelved changelist that contains the
	shelved files to be copied.

	The -c flag specifies the pending changelist that will be the target
	for the shelved files rather than creating a new one. To update
	a target shelf you must be the owner of the changelist.

	The -f (force) flag must be used when the same file already exists in
	the target changelist in order to overwrite it.

	The -p flag promotes the new or target changelist where it can be
	accessed by other edge servers participating in the distributed
	configuration.  Once a shelved change has been promoted, all
	subsequent local modifications to the shelf are also pushed to
	the commit server and remain until the shelf is deleted.

	The -Af flag specifies that only files should be reshelved with this
	changelist. The -As  flag specifies that only an opened stream
	specification should be reshelved with this change list. By default,
	if the stream spec is shelved and neither -Af nor -As is given, the
	stream specification will also be included with any reshelved files.
	(See 'p4 help streamcmds')
)"
};

ErrorId MsgHelp::HelpDbstat = { ErrorOf( ES_HELP, 113, E_INFO, EV_NONE, 0 ),
R"(
    dbstat -- Display size or simple statistics for a database table

    p4 dbstat [-h] [-f] | [-p [-w worklevel] [-n numfiles]]
	        { -a | dbtable ... }
    p4 dbstat -s

	The -h flag displays a histogram showing the distances between
	leaf pages,  enabling you to anticipate disk seeks required for
	sequential database scans.  Dbstat scans only the internal node
	pages of the file, not the leaf or overflow pages, and thus
	reads only 1-5%% of the file.  Use this command with caution:
	it locks out write access to the database table during its scan.

	The -f flag outputs a page count, pages free, and percent free.
	If the -h flag is given together with -f, a histogram of free page
	distribution is shown and distance report is omitted.

	The -p flag requests level and page count information for a database
	table. Included in the output are details of how the file would
	be split for a multifile checkpoint request. The -w worklevel
	and -n numfiles flags override the similarly named configurables
	'db.checkpoint.worklevel' and 'db.checkpoint.numfiles'. The
	'worklevel' determines which level of the table should be used to
	split the table. 'numfiles' determines, along with the depth of the
	database table, the number of checkpoint files that would be
	generated.

	The -a flag outputs information for all tables.

	The -s flag returns the size of the database table files.

	This command requires that the user be an operator or have 'super'
	access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpDbverify = { ErrorOf( ES_HELP, 141, E_INFO, EV_NONE, 0 ),
R"(
    dbverify -- Perform low-level verification of the database tables

    p4 dbverify [-t db.table] [-U] [-v]

	This command performs a series of low-level structural integrity
	checks on the P4 database tables. It can be run periodically to
	determine if tables have become damaged.

	The -t flag restricts the verification to the specified table.

	When validating all the tables, the partitioned tables (readonly,
	partitioned, and the partitioned-jnl type) are excluded from the
	process of verification. However, the partitioned-jnl type tables that
	are related to the server can be verified if the configurable
	'db.partition.verify' is enabled.

	When validating a specific partitioned table (e.g., 'p4 dbverify
	-t db.have.pt%pclient') or all partitioned tables (e.g., 'p4 dbverify
	-t db.have.pt'), all types of partitioned tables (readonly,
	partitioned, and partitioned-jnl) that are associated with the server
	are verified automatically, without requiring the 'db.partition.verify'
	configurable to be enabled.

	The -U flag specifies that only the table-not-unlocked check should
	be performed. This check is very fast, so it can be performed
	routinely. The table-not-unlocked check reports if, at some time in
	the past, the table was opened and written, but not cleanly closed.

	The -v flag gives verbose information on the verification.

	This command requires that the user be an operator or have 'super'
	access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLogstat = { ErrorOf( ES_HELP, 114, E_INFO, EV_NONE, 0 ),
R"(
    logstat -- Report the size of the journal, error log, audit log,
		or server log files

    p4 logstat [-s | -l log]

	If the -l argument is not specified, logstat reports the file size
	of the server's journal, error log, and audit log, skipping those
	which are not defined for this server.

	If the -s flag is provided, logstat also reports the file size for
	each of the structured log files defined for this server."

	If a named log is specified using the -l flag, logstat instead
	reports the file size of that named log. The log name should be one
	of: 'journal', 'errorLog', 'auditLog', or the name of one of the
	structured log files defined with 'serverlog.file.N'.

	This command requires that the user be an operator or have 'super'
	access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLogappend = { ErrorOf( ES_HELP, 142, E_INFO, EV_NONE, 0 ),
R"(
    logappend -- Write text to any 'user' log files.

    p4 logappend -a args ...

	Logappend writes the user-supplied text arguments to any
	structured server log file which is including 'user' log events.

	At most 25 arguments may be specified to the command.

	This command requires that the user be an operator or have 'list'
	access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLogexport = { ErrorOf( ES_HELP, 280, E_INFO, EV_NONE, 0 ),
R"(
    logexport -- Export a server log file to an external service

    p4 logexport [ -l | -e endpoint [ -h headers...] [ -I interval ] ] logname

	Logexport parses the named structured logfile and exports the log
	entries to the OTLP endpoint specified with the -e flag. This will
	create an export state file for the structured log, so that repeated
	calls for a given log will continue from the last entry exported.

	It is expected that this command will be configured as a startup
	command, with the -I interval flag set such that the export will
	pause for the specified number of milliseconds when the end of the
	log has been reached before automatically looking for more log entries.

	The current state of a logexport can be viewed by specifying the -l
	flag.

	This command requires that the user be an operator or have 'super'
	access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLogrotate = { ErrorOf( ES_HELP, 136, E_INFO, EV_NONE, 0 ),
R"(
    logrotate -- Rotate one or more server log files

    p4 logrotate [-l logname]

	Logrotate rotates the named logfile, or rotates all server logs if
	the -l flag is not specified.  This only applies to structured logs.

	This command requires that the user be an operator or have 'super'
	access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLogparse = { ErrorOf( ES_HELP, 143, E_INFO, EV_NONE, 0 ),
R"(
    logparse -- Parse a server logfile and return log data

    p4 logparse [-e] [-T field,field...] [-F filter] [-s offset] [-m max]
                logfilename

	Logparse parses the indicated structured logfile and returns the
	log data in tagged format.  It does not work on the basic errorLog or
	auditLog.  See the logtail command for reading the errorLog.

	The -e flag displays special characters as hexadecimal encodings.

	The -T flag can be used to limit the fields that are displayed.

	The -F flag limits output to records that match the filter pattern.

	The -s flag starts the parse at the given file offset (as returned
	in the f_offset field.)

	The -m flag limits the number of lines returned.

	This command requires that the user be an operator or have 'super'
	access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLogschema = { ErrorOf( ES_HELP, 144, E_INFO, EV_NONE, 0 ),
R"(
    logschema -- Describe the schema of a log record type

    p4 logschema [-A] -a
    p4 logschema [-A] recordtype

	Logschema returns a description of the specified log record type.

	The -a flag requests a specification of every known log record type.

	The -A flag requests a specification of every version of the specified
	log record types.

	This command requires that the user be an operator or have 'super'
	access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLockstat = { ErrorOf( ES_HELP, 115, E_INFO, EV_NONE, 0 ),
R"(
    lockstat -- Report lock status of database tables

    p4 lockstat [-c client | -C]

	By default, lockstat reports database tables that are currently
	locked for a read or write operation.

	If the -c argument is supplied, lockstat instead reports whether the
	specified client workspace is currently locked for a read or write
	operation.

	If the -C argument is supplied, lockstat instead reports on all client
	workspaces that are currently locked for a read or write operation.

	Database table lock information is also reported by 'p4 monitor' if
	the server has been configured to collect that data. For more
	information, see 'p4 help monitor'.

	This command requires that the user be an operator or have 'super'
	access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpLogtail = { ErrorOf( ES_HELP, 116, E_INFO, EV_NONE, 0 ),
R"(
    logtail -- Display the last block(s) of the errorLog

    p4 logtail [-b blocksize] [-s startingOffset [-m maxBlocks]] [-l log]

	Logtail outputs the last block(s) of the errorLog and the offset
	required to get the next block when it becomes available.

	The -s flag option enables you to specify the offset from the
	beginning of the file (in bytes).

	The -b flag specifies the block size in bytes (default 8192).

	The -m flag specifies a maximum number of blocks to output.
	The -m flag is ignored unless the -s flag is specified.

	By default, the command outputs all blocks from the offset until the
	end of the file. The data is returned in the tagged field 'data', in
	blocks of the size specified by the blocksize parameter. The 'offset'
	field contains the start of the next block, which can be used with -s
	to request the next batch of errorLog data.

	If a named log is specified using the -l flag, logtail instead
	outputs the last block(s) of that named log.

	This command requires that the user be an operator or have 'super'
	access, which is granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpDbpack = { ErrorOf( ES_HELP, 117, E_INFO, EV_NONE, 0 ),
R"(
    dbpack -- reorder database index pages (unsupported)

    p4 dbpack [-c pages -l level] { -a | dbtable... }

	Dbpack reorders database index pages so that leaf pages are sequential.

	The -c flag causes the operation to stop after the specified number of
	leaf pages are reordered.

	The -l flag is a threshold that must be exceeded before an index
	page is reordered.  Values for threshold can be:

	    0: index page more than 90%% full and 80%% out of order (default)
	    1: index page more than 50%% full and 50%% out of order
	    2: index page more than 0%% out of order

	The -a flag reorders all tables.

)"
};



ErrorId MsgHelp::HelpPing = { ErrorOf( ES_HELP, 118, E_INFO, EV_NONE, 0 ),
R"(
    ping -- test network performance

    p4 ping [-f][-p pausetime][-c count][-t seconds][-i iterations]
	     [-s send size][-r receive size]

	Ping sends messages from the server to the client and back, timing
	the round trips. The round trip time is typically too fast to time
	a single packet, so a number of packets are sent. Ping reports the
	time in milliseconds.

	The -c flag specifies the number of messages in a test.

	The -t flag specifies how long to transmit (maximum 6000 seconds).

	The -i flag specifies the number of times to repeat the test.

	The -f flag causes the server to transmit continuously, without
	waiting for one message to be confirmed before sending the next.

	The -p flag specifies how long to pause between tests: specify 0 for
	no pause (maximum wait 120 seconds).

	The -s flag specifies the server-to-client message size, up to a
	maximum value of 10,000,000 bytes

	The -r flag specifies the client-to server message size, up to a
	maximum value of 100,000 bytes

	Be advised that 'p4 ping' can flood the network with traffic. The
	'ping' command requires admin privileges.

)"
};

ErrorId MsgHelp::HelpConfigure = { ErrorOf( ES_HELP, 124, E_INFO, EV_NONE, 0 ),
R"(
    configure -- manage server configuration variables

    p4 configure set [--comment=<comment>] [<serverid>#]variable=value
    p4 configure unset [--comment=<comment>] [<serverid>#]variable }
    p4 configure show [allservers | <serverid> | <variable>]
    p4 configure history [allservers | <serverid> | <variable>]
    p4 configure history [--iteration=N] --comment=<comment> variable
    p4 configure help

	'p4 configure set' sets the value of the specified configuration
	variable.  For a list of configuration variables, see 'p4 help
	configurables'.

	Server names are useful when multiple servers share the table. If the
	serverid has been defined for a server (see 'p4 help serverid'),
	the serverid is used as the server name. The server name can also be
	explicitly specified using P4NAME or -In.

	In the 'p4 configure' command, if the serverid is omitted, the
	special server name 'any' is used, and the value is used by any
	server that is using this server root.

	'p4 configure unset' removes the specified configuration variable.

	'p4 configure show' displays the current configuration of this
	server.

	'p4 configure show allservers' displays the persistent configuration
	variables for all servers.

	'p4 configure show <serverid>' displays the persistent configuration
	variables that are set for the specified server.

	'p4 configure show <variable>' displays the setting(s) of the
	specified variable. This variant of the command shows all the
	settings of the variable that it can find, in order of precedence,
	and can be useful for understanding why a particular variable is
	not configured with the value that you expect, since variable
	settings can come from multiple sources.

	'p4 configure history' displays the history of configurables' values
	recorded by the 2019.2 server onwards.  The options of variable,
	serverid or 'allservers' filter the history in the same way as
	'p4 configure show' uses these options.
	Passing the long option --comment will set the comment for the most
	recent history, and an optional --iteration can be used to target a
	specific history entry.

	'p4 configure help' displays all documented configurables.
	Shows their value, description, value type, min value (if available),
	max value (if available), default value, recommended value (if
	available), restart requirements, support level, category and
	URL of the online help.

	Note that the following variables cannot be set using 'p4 configure':
	'unicode', 'P4JOURNAL', and 'P4ROOT'.

	This command requires that the user be an operator or have 'super'
	access, which is granted by 'p4 protect'.

)"
};

ErrorId MsgHelp::HelpPull = { ErrorOf( ES_HELP, 129, E_INFO, EV_NONE, 0 ),
R"(
    pull -- cause this replica server to retrieve data from its target

    p4 pull [-J prefix] [-i <N>] [-b <N>]
            [-T tableexcludelist] [-P serverid]
    p4 pull -u [-i <N> -b <N> --batch=N --min-size=N --max-size=N --trigger]
    p4 pull -l [--filter=failed|active|retry] [-C] [-s] [-v] [-h] | -j [-v] ]
               [-J prefix] ]
    p4 pull -d -f file -r revision
    p4 pull -L [-i <N>]
    p4 pull -R [ file ]
    p4 pull -u -t target [-b <N> -i <N>]

	'p4 pull' instructs this replica server to retrieve either journal
	records or file contents from its target server.

	The -i flag causes the command to automatically repeat its action
	every N seconds. If -i is not specified, the command runs once,
	then exits.

	The -b flag specifies a wait time before retrying a pull after
	a failed pull attempt.  This defaults to 60 seconds.

	The -u flag causes the command to retrieve file content. If -u is
	not specified, the command retrieves journal records and applies
	them to this server. Multiple 'p4 pull -u' commands may be active
	on a single replica server.

	The -l flag displays information about pending file content
	transfers. If -C is also specified, transfer records for files that
	are already in place or are not needed will be purged. If -s is also
	specified, only a summary is displayed. If --filter=failed is also
	specified, only records that failed and won't be retried will be
	displayed. With --filter=active all the other records that are still
	active will be displayed. With --filter=retry all records that have
	failed at least once but still will be retried will be displayed.
	With -ls a -v can be added to show numbers in the result separately
	for four categories: Active, Queued, Failed and Retry. Adding a -h
	for -ls will show file size numbers in human-readable format.
	If -j is instead specified, a summary of pending journal transfers
	is displayed. When the -v flag is added the current lag is displayed
	regardless of the value of the rpl.track.behind configurable. See
	'p4 help configurables' for more information.

	The -d flag specifies that the pending file content transfer should
	be cancelled. You must also specify the filename and revision using
	the -f and -r flags. Note that this is not the normal P4 file and
	revision data, but rather the archive file and revision. The correct
	archive filename and revision information to provide can be viewed
	using 'pull -l'. The 'pull -d' command can be useful when a file
	transfer is failing due to unrecoverable errors on the master.

	Without -u, -l, -R, -t, or -d, pull retrieves journal records.

	For the pull command which retrieves journal records, you may specify
	-i 0, which causes the pull thread to use an alternate scheduling
	algorithm that can reduce replica lag at the cost of slightly higher
	resource use on the target server.

	The -J prefix flag can be used when pulling journal records or when
	listing pending journal transfers to specify a file name prefix to
	match the one used with p4d -jc prefix.

	The -P flag controls replica data filtering. Typically, filtering is
	defined in the server spec (see 'p4 help server'), and the replica's
	journal pull thread specifies the replica's serverid to enable the
	filtering. It is also possible to specify 'p4 export' filter patterns
	using the -P flag, although this is an unusual configuration style.
	See 'p4 help export' for information about filter patterns.

	The -T flag specifies a list of database tables that are excluded
	from the journal records.  To specify multiple tables, double-quote
	the list and separate the table names with spaces. The table names
	must start with "db.". Table names can also be separated by commas.

	The -L flag specifies that journal records shall be retrieved from a
	local journal file, such as is produced by 'p4 journalcopy'. The
	'pull -L' and 'journalcopy' commands are designed to be used together
	in a standby replica (see 'p4 help replication').

	The --batch flag specifies the number of files a pull thread should
	process in a single request. The default is 1 and is usually
	adequate. For high-latency configurations, a larger value may
	improve archive transfer speed for large numbers of small files.

	The --min-size and --max-size options (which must be used with -u
	and --batch) can be used to create different pull threads for files
	with different sizes. Pull threads called with these options will
	only pull a file if its data size falls within the specified data
	size range. The size unit is bytes, but K, M, G, and T modifiers
	can also be used (i.e. 2K, etc).

	The --trigger option is used with a pull-archive trigger to transfer
	files using an alternative file transfer mechanism from within the
	trigger. This option is only used in a distributed environment and
	is not supported for RCS storage. The configurable pull.trigger.dir
	must be set to a location to write temporary files. It is also
	recommended to set lbr.replica.notransfer=1 to suppress 'on demand'
	file transfer.

	The -R flag can be used to retry failed archive file transfers.
	While the -d option may be used to remove pending or failed file
	transfers individually, this option will instead reset the retry
	count of failed transfers so that subsequent pull operations can
	retry the transfers.

	The -t option supports background and parallel archive transfer.
	If any pull threads get "stuck", issue this command
	manually on the commit server to pull the stuck threads.

	This command requires 'super' access granted by 'p4 protect'.

)"
};

ErrorId MsgHelp::HelpConfigurables = { ErrorOf( ES_HELP, 130, E_INFO, EV_NONE, 0 ),
R"(
   P4 server configurables

	The P4 server behavior can be controlled by setting
	configuration variables using the 'p4 configure set' command. The
	configuration variables that can be set include the following, in
	addition to environment variables (see 'p4 help environment'):

	Name                       Use
	----                       ---
	auth.autologinprompt       Allow P4 to prompt users to login
	auth.2fa.persist           Allow persistent 2nd factor authentication
	                           0: Disabled
	                           1: Allowed (default)
	                           2: Implied
	auth.default.method perforce Default auth method for new users
	                           perforce: classic password validation
	                           ldap: passwords validated by LDAP server
	auth.id                    Unique identifier for authentication
	auth.ldap.cafile           Path to PEM encoded CA validation file
	auth.ldap.order.N          Enabled LDAP configuration names
	auth.ldap.pagesize         Limit for LDAP searches with paged results
	auth.ldap.sslciphers       SSL ciphers to present to LDAP servers
	auth.ldap.ssllevel         Level of SSL certificate validation
	                           0: no validation (default)
	                           1: cert must be valid, but CN not checked
	                           2: cert must be valid and CN must match
	auth.ldap.timeout          Maximum wait time for LDAP connections
	auth.ldap.userautocreate   Enables user autocreation on successful auth
	                           0: no user autocreation  (default)
	                           1: user created on successful auth
	                           2: user must have also already have protects
	auth.licenseexpiry.warn    Enable license expiry warning
	                           0: No license expiry warning
	                           1: Warn users with permission level admin or
	                              higher when they log in (default)
	                           2: Warn all users when they log in
	auth.licenseexpiry.warnthreshold   Days before license expiry to start
	                                   giving warning
	auth.sso.allow.passwd      Allow fallback to database password auth
	auth.sso.args              Passed to client P4LOGINSSO triggers
	auth.sso.nonldap           Non-LDAP users can use SSO when LDAP in use
	auth.tickets.nounlocked    Allow 'p4 login -a's
	                           0: Allowed (default)
	                           1: Disabled silently
	                           2: Disallowed
	client.readonly.dir        Storage location for readonly, partitioned
	                           and partitioned-jnl clients. A relative path
	                           to the server's root directory is
	                           recommended.
	client.sendq.dir           Storage location for partitioned db.sendq
	db.checkpoint.bufsize      Minimum journal buffer size during
	                           checkpoint
	db.checkpoint.threads      Number of threads to use for checkpoint
	                           0: Use non-threaded checkpointing
	                           N: Use N threads to produce the
	                              checkpoint
	db.checkpoint.reqlevel     Only database files at this level or
	                           deeper will be considered to be split
	                           into multiple checkpoint files during
	                           a checkpoint or dump request.
	db.checkpoint.worklevel    The number of pages at this level of
	                           a database file is used along with
	                           'db.checkpoint.numfiles' to determine the
	                           records keys for a multifile split.
	db.checkpoint.numfiles     This is used to calculate the divisor of the
	                           number of pages at the worklevel to
	                           determine the record keys for a multifile
	                           split. For more details, see the P4 Server
	                           Administrator Guide on "Parallel
	                           checkpointing, dumping and recovery".
	db.journalrotate.warnthresh Days without journal rotation before
	                           warning starts to show up for admin users.
	db.monitor.addthresh       Milliseconds before adding command or
	                           connection at monitor level 1 or 2
	db.monitor.interval        Seconds between the terminate process check
	db.monitor.term.allow      Allow users to terminate own process
	                           0: disabled (only admin/operator can)
	                           1: users can terminate own process
	                           2: users can also pause/resume own process
	db.monitor.shared          Pages of shared memory for monitoring use
	db.partition.dropondelete  When this setting is enabled, individual
	                           records from the table are not deleted.
	                           Instead, the entire table is dropped, and
	                           the corresponding journal note 31 is
	                           logged in the journal file. While this
	                           approach improves deletion performance,
	                           since the logging of record by record
	                           deletion in the journal file is avoided, it
	                           has a known consequence that the table
	                           cannot be recovered once it has been
	                           removed.
	db.partition.verify        Enables verifying the partitioned have table
	db.peeking                 Lockless operation
	                           0: classic locking
	                           1: new locking
	                           2: basic lockless operation (default)
	                           3: extra lockless operation
	db.peeking.usemaxlock      When peeking, obey maxlocktime setting.
	db.replication             Replica metadata access mode
	db.rt.io                   Database IO tracked by rt.monitorfile
	dbjournal.bufsize          Journal/checkpoint read/write size
	dbopen.nofsync             Disable fsync of db files
	defaultChangeType          Default for new change: public/restricted
	dm.annotate.maxsize        Maximum revision size for default annotate
	dm.batch.net               Batch size for files sent from an edge to
	                           a commit server during 'p4 labelsync'
	dm.change.skipkeyed        Disable generation of digest for ktext
	                           revisions during submitted change updates
	dm.configure.comment.mandatory Require comment when changing
	                               configurable value with 'p4 configure'
	dm.copy.attributes         Controls if 'p4 copy' considers attributes
	dm.domain.accessupdate     Time interval to update domain access time
	dm.domain.accessforce      Time interval to force domain access time
	dm.fetch.preservechangenumbers Preserve change numbers on 'p4 fetch'
	                               or 'p4 push' to this server
	dm.flushforce              Max record count for a batch; waits on locks
	dm.flushtry                Record count interval for trying to take
	                           locks to flush a batch.
	dm.info.hide               Suppress output of sensitive info fields
	dm.integ.streamspec        Default stream spec integration value
	                           0: Stream spec integration is not allowed
	                           1: Allow stream spec integration only for
	                              target streams with noinherit ParentViews
	                           2: Allow stream spec integration for all
	                              streams
	dm.grep.maxrevs            Maximum number of revs that can be searched
	dm.keys.hide               Users require admin for 'keys' command
	dm.locks.excl.batch.net    Batch size for exclusively (un)locked files
	                           sent from an edge to a commit server
	dm.locks.global.batch.net  Batch size for globally (un)locked files
	                           sent from an edge to a commit server
	dm.locks.global.result.batch.net Batch size for globally (un)locked
	                                 files returned from a commit
	                                 server to an edge server
	dm.obliterate.skiphave     Obliterate uses -h by default
	dm.obliterate.skiplabel    Obliterate uses -l by default
	dm.open.show.globallocks   Report global locks on file open from edge
	dm.password.minlength      Minimum password length (when enabled)
	dm.populate.skipkeyed      Disable generation of digest for ktext
	                           revisions during populate
	dm.protects.allow.admin    Enable users with admin protections to run
	                           'p4 protects -a|-u|-g|-s'
	dm.protects.exclusioncheck Users with protections that have been
	                           entirely removed by exclusionary protection
	                           rules will be treated as if they had no
	                           protections
	dm.protects.streamspec     Enable streamspec permissions
	dm.proxy.protects          Add 'proxy-' to IP (see 'p4 help protect')
	dm.resolve.attrib          Enable resolve for attributes
	dm.resolve.ignoredeleted   Enable integrations into deleted files to be
	                           resolved as 'ignored' by 'p4 resolve -as'
	dm.rotatelogwithjnl        Rotate logs when journals are rotated.
	dm.shelve.accessupdate     Time interval to update shelve access time
	dm.shelve.maxfiles         Max number of files that can be shelved
	dm.shelve.maxsize          Limit size of a file that can be shelved
	dm.shelve.promote          Promote shelved changes from edge server
	db.reorg.disable           Disable BTree reorganization
	dm.repo.noautocreate       Repo autocreation level
	dm.repo.unpack             Repo unpacking mode
	                           0: keep pack files
	                           1: unpack on update
	                           2: unpack all
	dm.stream.components       Enables stream components functionality
	dm.stream.parentview       Default for stream spec ParentView field
	                           0: inherit ParentView for all new streams
	                           1: noinherit Parentview for new mainline,
	                              development, and release streams;
	                              inherit ParentView for new sparsedev,
	                              sparserel, task, and virtual streams
	                           2: noinherit Parentview for new release
	                              streams; inherit ParentView for all
	                              other streams
	dm.stream.sparse.branchmax Maximum number of depot files in the branch
	                           view of a sparse stream.
	                           0: No limit
	                           N: Limit the number of files to N
)"
R"(	dm.topology.lastseenupdate Time interval to update topology record.
	                           It represents the time that a record
	                           must age before it is re-written merely
	                           to change its 'lastSeenDate' field.
	dm.user.accessupdate       Time interval to update user access time
	dm.user.accessforce        Time interval to force user access time
	dm.user.accessmode         Conditions for updating the user access time
	                           0: Only update on command start
	                           1: Users with IdleTimeouts update during and
	                              at the end of a commands
	                           2: All user's periodically update
	                           3: Service type users update too
	dm.user.loginattempts      Number of password attempts before delay
	dm.user.allowselfupdate    Users may update their email and fullname
	dm.user.autoprotections    First user automatically populates protects
	dm.user.hideinvalid        Hide invalid user error on authentication
	                           failure
	dm.user.noautocreate       User autocreation level
	dm.user.resetpassword      New user requires password reset
	dm.user.setinitialpasswd   Unset password handling
	                           0: Only super users whose password is set
	                              can set/unset initial passwords (default)
	                           1: Users can set their own initial passwords
	dm.sync.streamchange       Syncing a stream client to a specific
	                           changelist will also apply the stream's view
	                           at that change
	filesys.atomic.rename      Enables atomic rename semantics for the
	                           journal file on Windows
	filesys.binaryscan         'add' looks this far for binary chars
	filesys.bufsize            Client file I/O buffer size
	filesys.checklinks         Reject symlinked directories on add
	filesys.depot.min          Minimum space for depot filesystem
	filesys.extendlowmark      Minimum filesize before preallocation(NT)
	filesys.P4ROOT.min         Minimum space for P4ROOT filesystem
	filesys.P4JOURNAL.min      Minimum space for P4JOURNAL filesystem
	filesys.P4LOG.min          Minimum space for P4LOG filesystem
	filesys.TEMP.min           Minimum space for TEMP filesystem
	filesys.windows.lfn        Enables long filename support on Windows:
	                           0: Off
	                           1: Paths longer than 255 characters use LFN
	                           10: All paths use LFN
	filetype.maxtextsize       Maximum file size for text type detection
	journalPrefix              Prefix or directory location for journals
	                           and checkpoints
	journalPrefixBackup        Secondary location for journalPrefix
	info.p4auth.usercheck      Validate username against P4AUTH server
	lbr.s3.uploadretries       Number of times an upload to S3 will be
	                           retried
	lbr.s3.multipart           Files bigger than this value use the S3
	                           multipart protocol for transfers. Value
	                           is specified in megabytes
	lbr.s3.multipartsz         Files using the S3 multipart protocol
	                           are split into chunks of this size.
	                           Value is specified in megabytes
	lbr.autocompress           By default, use compressed text storage
	                           instead of RCS
	lbr.bufsize                Archive file I/O buffer size
	lbr.proxy.case             Proxy cache case-handling (see 'p4p -h')
	lbr.rcs.existcheck         Perform RCS validation check during submit
	lbr.rcs.maxlen             Maximum line length in a +k file.
	lbr.replica.notransfer     Suppress on-demand file fetch
	lbr.replication            Replica depot access and replication mode
	lbr.retry.max              Times replica should retry failed transfer
	lbr.scan.ignore_pattern    A JSON encoded array of regular expressions
	                           used by the archive file scanner
	                           (p4 storage -l start --nonlbr). A filename
	                           matching any of the patterns will be ignored
	                           by the scanner.
	lbr.stat.interval          Proxy file status interval (see 'p4p -h')
	lbr.storage.delay          Required age for storage record deletes
	lbr.storage.allowsymlink   Allows symlinks in orphan storage scanner
	lbr.storage.skipkeyed      Ignores keyword revisions during upgrade
	                           If set to 0 then a digest is created for
	                           all keyword revisions
	                           If set to 1 then no digest is created for
	                           keyword revisions and a warning message is
	                           logged
	                           If set to 2 then no digest is created for
	                           keyword revisions and no message is logged
	lbr.storage.threads        Number of threads to use for a storage -U
	                           request
	                           0: Don't use threads
	                           N: Use N threads to compute the digests
	lbr.verify.in              Verify contents from the client to server
	lbr.verify.out             Verify contents from the server to client
	lbr.verify.script.out      Verify +X contents from server to client
	lbr.unloaddepot.compress   When enabled, writes unload-depot in
	                           compressed mode.
	log.originhost             Origin and peer IPs in the structured logs
	minClient                  Lowest client version that may connect
	minClientMessage           Message to issue for client-too-old
	monitor                    Server monitoring level
	monitor.lsof               Set to /usr/bin/lsof to enable on Linux
	net.autotune	           Allow OS TCP autotune/autoscale of buffers
	                           If set, net.tcpsize is ignored
	net.backlog                Maximum pending connections queue length
	net.delta.rpl.minsize      Minimum file size to perform replica delta
	                           transfer. Must be non-zero on both upstream
	                           and downstream servers to activate delta
	                           transfer. Upstream server values are used
	                           for replica delta transfer.
	net.delta.rpl.threshold    Maximum percentage of file size to perform
	                           replica delta transfer before reverting to
	                           streaming the complete file. Must be
	                           non-zero on both upstream and downstream
	                           servers to activate delta transfer. Upstream
	                           server values are used for replica delta
	                           transfer.
	net.delta.transfer.minsize Minimum file size to perform client-server
	                           delta transfer
	net.heartbeat.interval     Milliseconds between sending heartbeats
	net.heartbeat.wait         Milliseconds before response times out
	net.heartbeat.missing.interval Milliseconds between sending
	                               heartbeats after a missing response
	net.heartbeat.missing.wait Milliseconds before request times out
	                           after a missing response
	net.heartbeat.missing.count Missing consecutive heartbeats before
	                            detected as dead
	net.keepalive.disable      Disable sending TCP keepalive packets
	net.keepalive.idle         Seconds before starting to send keepalives
	net.keepalive.interval     Seconds between sending keepalives
	net.keepalive.count        Unacknowledged keepalives before failure
	net.maxfaultpub            Max size of faults (MB) to share via proxy
	net.maxclosewait           Milliseconds to wait for a network close
	net.maxwait                Seconds to wait for a network read or write
	net.mimcheck               Man-in-the-middle network security level
	net.nagle                  Enable or disable the Nagle algorithm
	                           0: Disable - send data immediately
	                           1: Enable - delay sending data
	net.parallel.max           Highest allowed degree of parallel transfer
	net.parallel.threads       Auto parallel sync w/#threads
	net.parallel.batch         Files in batch for auto parallel sync
	net.parallel.batchsize     Size of batch for auto parallel sync
	net.parallel.checkalive    Files transferred between checks
	net.parallel.min           Minimum # files for auto parallel sync
	net.parallel.minsize       Minimum size for auto parallel sync
	net.parallel.shelve.threads Auto parallel shelve w/#threads
	net.parallel.shelve.batch  Files in batch for auto parallel shelve
	net.parallel.shelve.min    Minimum # files for auto parallel shelve
	net.parallel.submit.threads Auto parallel submit w/#threads
	net.parallel.submit.batch  Files in batch for auto parallel submit
	net.parallel.submit.min    Minimum # files for auto parallel submit
	net.parallel.sync.svrthreads Server-wide parallel thread limit
	net.quickack               Enable or disable QuickAck
	                           0: Disable - delay ACKs
	                           1: Enable - send ACKs immediately
	net.reuseport              Set SO_REUSEPORT for listening socket
	net.rfc3484                Allow OS to choose between IPv4 and IPv6
	net.tcpsize                TCP sndbuf/rcvbuf sizes set at connect
	                           Ignored if net.autotune is set
	pull.trigger.dir           Tmp directory for alternative archive copy
	push.unlocklocked          Unlock locked files if push fails
	proxy.monitor.level        Proxy monitoring level (see 'p4p -h')
	proxy.monitor.interval     Proxy monitoring interval (see 'p4p -h')
	proxy.clearcachethresh     Threshold for clearing proxy cache
	rcs.nofsync                Disable fsync of RCS files
	rmt.allow.remoteuser       Allow remote user for server to server
	                           connections. (Disabled when security >= 4)
	rmt.enforce.serviceusertype Enforce service user type for server to
	                           server connections (see serviceUser).
	rejectList                 List of server blocked applications
	rpl.checksum.auto          Level to checksum when rotating journal
	rpl.checksum.change        Control change-by-change checksum behavior
	rpl.checksum.table         Control table checksum behavior
	rpl.compress               Enable replica-master network compression
	                           0: No compression
	                           1: Compress only archive transfers
	                           2: Compress metadata and archive transfers
	                           3: Compress metadata and archive transfers,
	                              and data streams for forwarded commands
	                           4: Compress only metadata transfers
	rpl.deferred.sends         Allow 'pull -u' to fetch missing archives
	                           during sync on edge and build servers
	rpl.filter.expand          Allow 'pull' to automatically run the
	                           'admin replica-filter-reconcile' expand
	                           operation when a server filter change is
	                           detected.
	rpl.filter.expand.batch    Block size used for the
	                           'admin replica-filter-reconcile' expand
	                           operation.
	rpl.filter.expand.threads  The number of threads to use in the replica
	                           filter expand operation.
	rpl.filter.restrict        Allow 'pull' to automatically run the
	                           'admin replica-filter-reconcile' restrict
	                           operation when a server filter change is
	                           detected.
	rpl.filter.restrict.batch  Number of records to batch process per
	                           database table in the
	                           'admin replica-filter-reconcile' restrict
	                           operation.
	rpl.filter.restrict.threads The number of threads to use in the
	                            replica filter restrict operation.
	rpl.forward.all            Enable replica update command forwarding
	rpl.forward.login          Enable replica login command forwarding
	rpl.jnl.batch.size         Max size of a single journal transfer
	rpl.jnlwait.adjust         Per-loop ms to add to rpl.jnlwait.interval
	rpl.jnlwait.interval       Initial value in ms of jnlwait loop timer
	rpl.jnlwait.max            Highest value in ms that loop will attain
	rpl.journalcopy.location   Current journalcopy'd journal location:
	                           0: written directly to journalPrefix
	                           1: rotated to journalPrefix
	rpl.labels.global          Label default for distributed installations
	rpl.pull.position          Interval in ms for pull position reports
	rpl.pull.reload            Interval in milliseconds for 'p4 pull' to
	                           reload any configuration changes
	rpl.replay.userrp          Include db.user.rp data from P4TARGET
	rpl.submit.nocopy          Disable default submit archive file copy
	rpl.track.behind           Report journals/total bytes not replicated
	                           0: No reporting
	                           1: pull logs at rpl=1 and via rt.monitorfile
	                           2: pull -l -j logs at rpl=1
	rpl.track.behind.interval  Max interval in ms for pull lag reports
	rpl.verify.cache           Verify contents in the replica cache
	rpl.pull.archivedepots     Replicate file content for archive depots
	rt.monitorfile             Realtime performance counter file
)"
R"(	run.clientexts.allow       Allow client-side Extensions to run
	run.renameclient.allow     Allow the 'renameclient' command
	                           0: disallow for all users
	                           1: allow for client owners, admins and super
	                           2: allow for admins and super
	                           3: allow for super
	run.unzip.user.allow       Should 'p4 unzip' allow '-u'
	run.users.authorize        Should 'p4 users' require authentication
	s3.error.retry             A JSON format array of errors that can be
	                           used to retry requested S3 operations
	s3.credentialerror.retry   A JSON format array of errors that can be
	                           used to retry requested S3 operations that
	                           first reload the current credentials
	security                   User/password security level
	security.digest            The digest algorithm used in the round trip
	                           tamper checking
	server                     Turn off tracking
	                           1: Include the start information for each
	                              command
	                           2: Include the start and stop information
	                              for each command
	                           3: Add a "compute end" message for certain
	                              commands
	                           4: Include errors sent to the client
	server.allowfetch          Whether changes can be fetched:
	                           1: This server can fetch from other servers
	                           2: Other servers can fetch from this server
	                           3: Both (1) and (2) are allowed
	server.allowpush           Whether changes can be pushed:
	                           1: This server can push to other servers
	                           2: Other servers can push to this server
	                           3: Both (1) and (2) are allowed
	server.allowremotelocking  Allow DVCS servers to lock files here
	server.allowrewrite        Whether submitted changes can be rewritten
	server.depot.root          Base directory of depots with relative maps
	server.extensions.dir      Directory for Extension-owned storage
	server.extensions.allow.admin When enabled, users with 'admin'
	                              protections can install and configure
	                              extensions
	server.extensions.allow.unsigned Enables users with super protections
	                                 to install unsigned extensions
	server.locks.dir           "server.locks" server lock directory
	server.locks.archive       Should archive/restore lock metadata
	server.locks.global        Lock globally by default from edge server
	                           0: Locks are local to edges by default
	                           1: Locks are global by default
	server.locks.sync          Should sync command lock client workspace
	server.commandlimits       Policy for per-command resource limits
	server.global.client.views
	                           Set to 1 to make client view maps global
	                           either globally:
	                              server.global.client.views=1
	                           or scoped on replica:
	                              replica#server.global.client.views=1
	server.maxcommands         Max simultaneous commands (if monitoring)
	server.maxcommands.allow 1 Allow super and operator user access even
	                           if server.maxcommands is reached
	server.rolechecks          Should server enforce 'p4 server' settings
	server.start.unlicensed    When enabled, the server may start even if
	                           the license is invalid. User's commands will
	                           be rejected until a valid license is
	                           installed.
	server.startup.autorestart When enabled, the server will automatically
	                           restart startup commands.
	serverlog.file.N           Server log file name(s)
	serverlog.maxmb.N          Size at which log file should be rotated
	serverlog.retain.N         Number of rotated log files to retain
	serverlog.counter.N        Counter to use for file rotation number
	serverlog.bufsz.N          Size of log write buffer
	serverlog.version.N        Server protocol version to be used when
	                           writing structured log file. Defaults to
	                           current server protocol version.
	serviceUser                Intermediate service identity
	spec.hashbuckets           Maximum number of subdirectories for hashed
	                           directory structures
	ssl.cipher.list            The list of OpenSSL ciphers the server will
	                           allow when establishing a TLS 1.2 or below
	                           connection, overriding the OpenSSL defaults
	ssl.cipher.suites          The list of OpenSSL cipher suites the server
	                           will allow when establishing a TLS 1.3
	                           connection, overriding the OpenSSL defaults
	ssl.client.ca.path         Path of CA PEM file to validate server cert
	ssl.client.cert.validate   Mode of validation of server cert
	                           0: Only use P4TRUST
	                           1: Cert chains accepted if valid CA found
	                           2: Cert chains accepted if subject valid
	ssl.client.cipher.list     The list of OpenSSL ciphers the client will
	                           allow when establishing a TLS 1.2 or below
	                           connection, overriding the default, or
	                           'ssl.cipher.list' if set.
	ssl.client.cipher.suites   The list of OpenSSL cipher suites the client
	                           will allow when establishing a TLS 1.3
	                           connection, overriding the OpenSSL defaults,
	                           or 'ssl.cipher.suites' if set.
	ssl.client.tls.version.min Min TLS version for client connections
	                           [10=1.0, 11=1.1, 12=1.2, 13=1.3]
	ssl.client.tls.version.max Max TLS version for client connections
	                           [10=1.0, 11=1.1, 12=1.2, 13=1.3]
	ssl.client.trust.name      P4TRUST uses hostname for cert chains
	                           0: Only the server IP is recorded
	                           1: Server IP and hostname is recorded
	                           2: Only server hostname is recorded
	ssl.secondary.suite        Set SSL cipher suite to the secondary choice
	ssl.tls.version.min        Set min TLS version [10=1.0, 11=1.1, 12=1.2,
	                           13=1.3]
	ssl.tls.version.max        Set max TLS version [10=1.0, 11=1.1, 12=1.2,
	                           13=1.3]
	startup.N                  Background 'pull' command(s) for replica
	statefile                  Replica server state tracking file name
	submit.collision.check     Check file/directory collision on submit
	submit.noretransfer        Avoid file re-transfer after failed submit
	                           0: Retransfer all files
	                           1: Retransfer files not already transferred
	submit.allowbgtransfer     Allow edge submit background file transfer
	submit.autobgtransfer      Auto edge submit background file transfer
	submit.unlocklocked        Unlock locked files if submit fails
	submit.identity            Changelist identity auto-generation:
	                           uuid: generate as uuid
	                           checksum: generate as checksum
	                           serverid: generate as serverid+change
	submit.storagefields       Controls the action of a submit request with
	                           regards to the 'compCksum' field in a
	                           storage record
	sys.pressure.max.pause.time Max seconds to wait while paused.
	sys.pressure.max.paused    Max number of processes allowed to be
	                           paused.
	sys.pressure.mem.high      Percentage of memory used for 'high'.
	sys.pressure.mem.high.duration Milliseconds of time to average
	                               input.
	sys.pressure.mem.medium    Percentage of memory used for 'medium'.
	sys.pressure.mem.medium.duration Milliseconds of time to average
	                                 input.
	sys.pressure.os.cpu.high   Percentage of processes stalled for CPU.
	sys.pressure.os.cpu.high.duration Milliseconds of time to average
	                                  input.
	sys.pressure.os.mem.high   Percentage of processes stalled for memory.
	sys.pressure.os.mem.high.duration Milliseconds of time to average
	                                  input.
	sys.pressure.os.mem.medium Percentage of processes stalled for
	                           memory.
	sys.pressure.os.mem.medium.duration Milliseconds of time to
	                                    average input.
	sys.rename.max             Limit for retrying a failed file rename
	sys.rename.wait            Timeout in ms between file rename attempts
	sys.threading.groups       Use multiple processor groups on Windows.
	                           Only available on Windows Server 2008 R2+.
	template.client            Client to use as template if -t omitted
	template.label             Label to use as template if -t omitted
	track                      Default based on number of licensed users
	                            -1: Default based on number of licensed
	                                users
	                             0: Turn off tracking
	                             1: Track all commands
	                           2-5: Report tracking metrics exceeding
	                                incrementally higher thresholds
	trait.storagedepot.min     Allow trait value to be stored in depot
	                           0 : Trait values stored in db.traits table
	                           >0: Trait value stored in depot when size
	                               in bytes meets or exceeds this value
	triggers.io                Method used for server/trigger communication
	unicode                    When enabled, the server operates in
	                           Unicode mode. This should only be enabled
	                           using 'p4d -xi'.
	webserver.log.path         Set the directory path to store the P4 Web
	                           Server log files. If not set, log files are
	                           stored in P4ROOT.
	webserver.log.retain       Set the maximum number of old log files to
	                           retain for each P4 Web Server log type on
	                           each web server process. When the limit is
	                           reached, the oldest files are removed.
	zerosyncPrefix             Client prefix for zerosync (-k) namespace
	zlib.compression.level     Compression level: -1 to 9, -1 is the
	                           default (6), 0 is lowest, 9 is highest.
)"
R"(
   P4 client configurables

	The P4 client behavior can be controlled by setting certain
	environment variables (see 'p4 help environment'). Additionally, a
	P4CONFIG file may contain settings for the following configurables
	to affect certain client behaviors:

	Name                       Use
	----                       ---
	filesys.binaryscan         'add' looks this far for binary chars
	filesys.bufsize            Client file I/O buffer size
	lbr.verify.out             Verify contents from the server to client
	net.delta.transfer.minsize Minimum file size to perform client-server
	                           delta transfer
	net.delta.transfer.threshold
	                           Maximum percentage of file size to perform
	                           client-server delta transfer before
	                           reverting to streaming the complete file
	net.keepalive.disable      Disable sending TCP keepalive packets
	net.keepalive.idle         Seconds before starting to send keepalives
	net.keepalive.interval     Seconds between sending keepalives
	net.keepalive.count        Unacknowledged keepalives before failure
	net.maxclosewait           Milliseconds to wait for a network close
	net.maxwait                Seconds to wait for a network read or write
	net.nagle                  Enable or disable the Nagle algorithm
	                           0: Disable - send data immediately
	                           1: Enable - delay sending data
	net.quickack               Enable or disable QuickAck
	                           0: Disable - delay ACKs
	                           1: Enable - send ACKs immediately
	net.rfc3484                Allow OS to choose between IPv4 and IPv6
	net.tcpsize                TCP sndbuf/rcvbuf sizes set at connect
	ssl.client.ca.path         Path of CA PEM file to validate server cert
	ssl.client.cert.validate   Mode of validation of server cert
	                           0: Only use P4TRUST
	                           1: Cert chains accepted if valid CA found
	                           2: Cert chains accepted if subject valid
	ssl.client.timeout         Timeout in seconds for SSL handshake
	                           negotiation on client connects. If
	                           'net.maxwait' is longer, 'net.maxwait' is
	                           used instead.
	ssl.client.tls.version.min Min TLS version for client connections
	                           [10=1.0, 11=1.1, 12=1.2, 13=1.3]
	ssl.client.tls.version.max Max TLS version for client connections
	                           [10=1.0, 11=1.1, 12=1.2, 13=1.3]
	ssl.client.trust.name      P4TRUST uses hostname for cert chains
	                           0: Only the server IP is recorded
	                           1: Server IP and hostname is recorded
	                           2: Only server hostname is recorded
	sys.rename.max             Limit for retrying a failed file rename
	sys.rename.wait            Timeout in ms between file rename attempts
	zlib.compression.level     Compression level: -1 to 9, -1 is the
	                           default (6), 0 is lowest, 9 is highest.
	                           

)"
};

ErrorId MsgHelp::ResolveUserHelp = { ErrorOf( ES_HELP, 22, E_INFO, EV_NONE, 1 ),
R"(%resolveType% options:

    Accept:
            at              Keep action from their file.
            ay              Keep only action from your file.
            am              Keep merged action.
            a               Keep autoselected action.

    Misc:
            s               Skip this file.
            h               Print this help message.
            ^C              Quit the resolve operation.
)"
};

ErrorId MsgHelp::HelpServer = { ErrorOf( ES_HELP, 137, E_INFO, EV_NONE, 0 ),
R"(
    server -- Create, modify, or delete a server specification

    p4 server <serverID>
    p4 server -g
    p4 server -d <serverID>
    p4 server -o [-c edge-server|commit-server] <serverID>
    p4 server -i [-c edge-server|commit-server]
    p4 server -c edge-server|commit-server <serverID>

	A server specification describes the high-level configuration
	and usage of a server. For installations with only a single server,
	the server specification is optional; server specifications are
	necessary when your installation expands to include features such
	as: replication, proxying, disaster recovery, etc.

	For additional help on replication, see 'p4 help replication'.

	The 'server' command puts the server spec into a temporary file and
	invokes the editor configured by the environment variable $P4EDITOR.
	Saving the file creates or modifies the server spec.

	The server spec contains the following fields:

	ServerID:  The server identifier.

	Type:      The server executable type.
	           One of: server(p4d), proxy(p4p), broker(p4broker),
	           or connector(p4gconn)

	%'Services'%:  Services provided by this server.
	           One of the following (grouped by server type):
	    Type %''server''%:
	         %'standard'%: standard %'Perforce'% server
	         %'replica'%: read-only replica server
	         %'commit-server'%: central server in distributed installation
	         %'edge-server'%: node in distributed installation
	         %'forwarding-replica'%: replica which forwards update commands
	         %'build-server'%: replica which supports build farm integration
	         %'P4AUTH'%: server which provides central authentication
	         %'P4CHANGE'%: server which provides central change numbers
	         %'standby'%: read-only replica server which uses %'journalcopy'%
	         %'forwarding-standby'%: forwarding-replica which uses %'journalcopy'%
	         %'local'%: personal server created by %'init'%
	         %'distribution-server'%: content distribution server
	    Type %''broker''%:
	         %'broker'%: %'p4broker'% process
	    Type %''proxy''%:
	         %'proxy'%: %'p4p'% caching proxy
	    Type %''connector''%:
	         %'git-connector'%: %'p4gconn'% caching proxy

	%'Options'%:   Flags that change the server's behavor.
	           The defaults are marked with *.

	    %'mandatory'%      A %'standby'% or %'forwarding-standby'% server
	                   that persists %'journalcopy'ed'% metadata before
	                   that metadata is replicated to other replicas.
	                   A %'standby'% or %'forwarding-standby'% server with
	                   this option set can be used for failover
	                   whether or not the server from which it
	                   is %'journalcopy'ing'% is available at the
	                   time of the failover.
	    %'nomandatory'% *  Replication to other replicas can occur
	                   before the metadata has been persisted by
	                   this %'standby'% or %'forwarding-standby'% server.
	                   Failover can occur to this %'standby'% or
	                   %'forwarding-standby'% server only if the
	                   server from which it is %'journalcopy'ing'%
	                   is available at the time of the failover.

	%'ReplicatingFrom'%:
	           Server ID of the server from which this server is
	           replicating or %'journalcopy'ing'%. This field is required
	           when the server is a %'standby'% or %'forwarding-standby'%
	           server.

	Name:      The P4NAME that is used by the server (optional).

	Address:   The P4PORT that is used by the server (optional).

	ExternalAddress:
	           For an edge server, this optional field can contain its
	           external address for connections from the commit server.
	           This field is required for distributed parallel submit.

	UpdateCachedRepos:
	           For a git-connector, this optional field contains a list
	           of graph repos which will be updated from P4 by
	           'gconn poll-repos', typically called by a cron process.

	Description:
	           A description of the server (optional).

	User:      The service user that is used by the server (optional).

	AllowedAddresses:
	           A list of addresses that are valid this server.  At security
	           level 6 this field is used to associate intermediary
	           servers with specified service users. Connections through
	           intermediary servers without matching server specs will be
	           blocked.

	ClientDataFilter:
	           For a replica server, this optional field can contain one
	           or more patterns describing how active client data is to
	           be filtered. Active client data includes have lists,
	           working records, and pending resolves.

	           Each pattern, in client syntax with optional wildcards,
	           should specify whether the data is to be included or
	           excluded, using the syntax:
	               //client-pattern/...   -- client data to include
	               -//client-pattern/...  -- client data to exclude

	RevisionDataFilter:
	           For a replica server, this optional field can contain one
	           or more patterns describing how submitted revision data is
	           to be filtered. Submitted revision data includes revision
	           records, integration records, label contents, and the
	           files listed in submitted changelists.

	           Each pattern, in depot syntax with optional wildcards,
	           should specify whether the data is to be included or
	           excluded, using the syntax:
	               //depot/pattern/...   -- include this depot path
	               -//depot/pattern/...  -- exclude this depot path

	ArchiveDataFilter:
	           For a replica server, this optional field can contain one
	           or more patterns describing the policy for automatically
	           scheduling the replication of file content. If this field
	           is present, only those files described by the pattern are
	           automatically transferred to the replica; other files will
	           not be transferred until they are referenced by a replica
	           command which needs the file content.

	DistributedConfig:
	           This optional field shows configuration settings relevant
	           to this server. When invoked with the -c flag, the
	           configuration values are populated with currently
	           configured values, recommended default values if unset,
	           or 'unset' for unset values with no default. If this
	           field is present when invoked with -c, the configuration
	           commands in this field are run on the current server
	           using the scope of the ServerID server.

	The -g flag generates a new server UUID.

	The -d flag deletes the named server spec.

	The -o flag writes the server spec to standard output. The user's
	editor is not invoked.

	The -i flag causes a server spec to be read from the standard input.
	The user's editor is not invoked. This option may be combined with
	the -c flag to generate and run configuration variables used to set
	up an edge or commit server in a distributed environment. When used
	with -c, only the fields explicitly set in standard input from
	the DistributedConfig field will be configured.

	The -c <services> option allows the user to set, change or display
	configuration values used to set up the distributed environment
	on an edge or commit server by using the DistributedConfig field.
	Configuration fields are initially populated using configured
	values if set, default values for recommended configuration
	values if they are unset, or 'unset' for recommended configuration
	values that have no default. After exiting from the form, any
	configuration commands in the DistributedConfig	field will be run
	on the current server for the scope of the serverID. Note that the
	commands only apply to the serverID server, and so the 'server#'
	prefix is not allowed in these commands. The only supported
	services are edge-server and commit-server. The service dictates
	which configuration values are used to initially populate the form
	the first time that the server command is run.

	'p4 server' requires 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpServers = { ErrorOf( ES_HELP, 138, E_INFO, EV_NONE, 0 ),
R"(
    servers -- Display list of server specifications

    p4 servers [-J | --replication-status]

	Lists server specifications. (See 'p4 help server'.)

	The -J (or --replication-status) flag instead reports information
	about this server's journal, and about the replication status of all
	replicas that replicate from this server (if any).  The fields shown
	are the server id, last updated time, server type, persisted journal
	position, applied journal position, flags, is-alive, and the %'mandatory'%
	option (if set). The is-alive field is 0 for a replica that is not
	running, or 1 for a live replica.  The persisted and applied journal
	fields are different only for a replica running journalcopy: the
	persisted position is the last journal offset that this replica
	has persisted to disk and acknowledged to the master server, and
	the applied position is the offset of the last journal record that
	has been applied to the replica's database.

	The first row displays information about this server, and the following
	rows display information about each replica that has either requested
	journal records from this server or has this server's ID as the value
	for the %'ReplicatingFrom'% field in it's server definition.  Only
	replicas that have a serverID or a P4NAME are listed in the results.

	The flags are a set of fields, each printed in upper-case if it is set
	or in lower-case if it is not; the numeric value of the flags is
	displayed after the alphabetic display.  Each field is listed below
	with its value, name, and description:

	W/8 (Wait)              : this is a long-poll request (-i 0)
	w   (no wait)           : this is not a long-poll request
	A/4 (Acknowledging)     : this request ACKs previous journal records
	a   (non-acknowledging) : this request does not ACK previous records
	D/2 (Durable)           : only durable records should be returned
	d   (non-durable)       : non-durable records should also be returned
	L/1 (Local)             : this is data about the local journal
	l   (remote)            : this is a request from a replica

	Common field displays with their associated pull or journalcopy
	commands are:

	WAdl/12     journalcopy -i 0
	WaDl/10     pull -i 0
	wAdl/4      journalcopy -i 1
	waDl/2      pull -i 1
	wadL/1      (synthesized record for master status)

	If a replica is a %'standby'% or %'forwarding-standby'% server, the
	%'mandatory'% option indicates that the metadata %'journalcopy'ed'%
	by that replica must be persisted before that metadata is
	replicated to other replicas.  A %'standby'% or %'forwarding-standby'%
	server with the %'mandatory'% option set can be used for failover
	whether or not the server from which it is %'journalcopy'ing'%
	is available at the time of the failover.

	You can compare the journal positions of each replica with that of this
	server to see if any replicas are falling behind.
)"
};

ErrorId MsgHelp::HelpStorage = { ErrorOf( ES_HELP, 256, E_INFO, EV_NONE, 0 ),
R"(
    storage -- manipulate low-level storage archives

    p4 storage [-v [-q]] [-c change] [-T tags -F filters] [-m max] archive ...
    p4 storage -u [-c change] [-T tags -F filters] [-m max] archive ...
    p4 storage -d [-c change] [-y] [-D secs] [ -t target ] [-q] archive ...
    p4 storage -w
    p4 storage -U [-q] [-n threads] [--server-size-rcs] archive ...
    p4 storage -l start [--nonlbr [-D secs] [-y] [-t target]]
	|pause|restart|wait|status [--nonlbr] |cancel  //depotdirectory/...

	The first form of this command displays information about the server
	archive files and their usage.

	The fields that storage displays are:
		lbrFile            -- the path of the archive file
		lbrRev             -- the revision of the archive file
		lbrType            -- the file type the archive file,
		                      e.g. text+C
		lbrRefCount        -- how many revs refer to the archive file
		headTime           -- timestamp of when the record was written
		fileSize           -- size of the archive file
		digest             -- MD5 digest of the archive file
		serverSize         -- Size of the archive file on the server
		                      at the time that revision was submitted
		                      or shelved
		compCksum          -- MD5 of the archive file containing the
		                      revision on the server if the file is
		                      compressed and submit.storagefields is 1

	Additional fields when the -v flag is specified:
		actualSize         -- actual size of revision computed from
		                      archive file
		actualDigest       -- actual MD5 digest of revision computed
		                      from archive file
		status             -- outcome of verify, OK, BAD or MISSING


	The -v flag verifies that the specified archive files are intact.
	'p4 storage -v' computes and displays the digest of each archive.
	If an archive cannot be reproduced (for example, if the file is
	missing from the archive), the archive's output line ends with
	MISSING!  If there is a saved digest, 'p4 storage' compares it with
	the computed one. If they differ, the output line ends with BAD!

	The -u flag computes and saves the digest only for revisions that
	have no saved digest.

	The -q flag minimises the output of the 'storage -v' command,
	displaying only errors from mismatched digests or unreproducible
	revisions.

	The -d flag deletes unreferenced archives. If the specified archive
	has a reference count of 0 and the age of the storage record is
	greater than the expiry time, then the record describing the
	archive is removed and the underlying archive revision, if present,
	is deleted from the file system. Without the -y confirm, the command
	just lists the candidate revisions it would remove. With the
	-y flag the revisions and zero referenced storage records are
	removed. The expiry time is set by the tunable variable called
	"lbr.storage.delay". It has a default value of 86400 seconds (one day).
	This expiry value can be overridden on the command line by specifying
	the '-D secs' parameter. Archives with a reference count of 0
	are rare, but can be created by incomplete 'p4 submit' or 'p4 shelve'
	commands, system crashes, other similar errors or
	'p4 storage -l start'. The listing of the candidate revisions can be
	suppressed by giving the -q option. The -t target option requests that
	rather than deleting the unreferenced archives, the revisions are first
	copied to the specified 'target' directory and then deleted.

	The -F flag lists only records satisfying the filter expression. This
	filter syntax is similar to the one used for 'jobs -e jobview' and is
	used to evaluate the contents of the fields in the preceding list.
	Filtering is case-sensitive.

	        Example: -v -F "lbrRefCount = 1 & status=OK"

	The -T fields flag returns only the specified fields. The field names
	can be specified using a comma- or space-delimited list.

	        Example: -T "lbrFile, lbrRev"

	The -c changelist# flag displays records associated with the specified
	changelist.

	The -m max flag limits output to the specified number of records.

	The -w flag detects if the second phase of a storage upgrade
	is in progress. The command returns with the message
	"The storage upgrade process is complete." only once the
	storage upgrade is finished.

	The -U flag reads all the storage records looking for an unset
	digest or serverSize field. When it detects either it
	recomputes the unset field(s) and updates the record. This is
	intended to be used after a storage upgrade has been performed
	with lbr.storage.skipkeyed set to avoid the delay incurred by
	calculating the digest during the upgrade. The -q flag can be
	supplied with the -U flag and this suppresses the progress
	messages produced. The -n threads flag specifies the number of
	threads to use. 0 = no threads; n = use n threads. When the n
	flags is not specified, the command uses the lbr.storage.threads
	configuration variable. Calculating the serverSize field for
	RCS file types can be expensive and so must be explictly requested
	by giving the --server-size-rcs flag.
	The -U flag requires 'super' access.

	The -l flag controls a background process that scans the server root
	looking for orphaned files that are no longer referenced by the server,
	and makes a note of them so they can be removed using the -d flag.
	The background scanning process looks for files in the specified
	//depotdirectory and its sub directories that match the naming
	convention for the server's archive file format. Each file
	that is found along with its revision is checked for a matching entry
	in the db.storage table. If no such entry is found, then a new record
	is created that describes the file/revision combination with a
	reference count of 0. A later call to 'p4 storage -d' will locate these
	zero reference counted records and remove them along with the archive
	revision. This provides a two stage process to detect orphaned files
	and remove them.

	Subcommand options:
	    start  -  start a new background depot scanning task starting in
	              the specified //depotdirectory and scan it and all its
	              sub directories. The special //depotdirectory argument of
	              '//...' is handled by searching for all the local depots
	              listed in the depot table and scanning their top and
	              sub directories. There is no limit on the number of
	              background processes that can be started, subject to the
	              constraints of the hardware and operating system limits.
	              Each directory in the archive may only be the subject of
	              a single scan. An unfinished scan is automatically
	              marked as paused when the server is shutdown.

	              The start command can take an optional '--nonlbr'
	              argument that will request the scan to remove any files
	              that do not match the file naming convention used by the
	              server for archive files (nonlbr). Any files that
	              match any of the patterns in 'lbr.scan.pattern_ignore'
	              configurable will not be removed.
	              Additionally a '--target dir' argument can be given that
	              will request a file will be moved to the 'dir' directory
	              rather than be removed. Without the '-y' confirm flag
	              no files will be moved or removed, just previewed.
	              All files must have aged by 'lbr.storage.delay' seconds
	              to be eligible for removal. The 'lbr.storage.delay'
	              setting can be overriden on the command line by
	              using the '-D secs' argument.

	    pause   - suspend the named background scan process.
	    restart - resume the named paused background scan process.
	    wait    - wait until the named background scan process completes or
	              fails due to an error.
	    status  - report on the named background scan process, or all the
	              background scan processes if the special argument of
	              '//...' is specified.
	              For each scan the following fields are displayed:
	              scanName   - the name of the scan, the argument passed to
	                           the 'start' subcommand;
	              state      - one of 'done', 'paused', 'run', 'busy' or
	                           'error';
	              dirsProc   - the number of directories that have been
	                           processed;
	              filesProc  - the number of files that have been
	                           processed;
	              zeroRecs   - the number of zero referenced storage
	                           records created. This is the number of
	                           orphan revisions that have no matching
	                           storage record;
	              nonlbrFiles- number of nonlbr files deleted or removed;
	              reqAge     - number of nonlbr files not deleted due to
	                           age;
	              dirsErrs   - the number of directories skipped due to
	                           errors;
	              errmsg     - if the state is 'error', then this is the
	                           error message that caused the failure;

	              The status command takes an optional '--nonlbr' argument
	              that will display the report of deleted nonlbr files;
	    cancel  - remove the scan from the system. A scan remains in
	              the system until it is cancelled. This is true even
	              if the scan state is 'done' or 'error'. You must
	              pause an active scan before you can cancel it;

	Warning. Scanning requires that each file can only be addressed by
	         a single path under the scanning directory. This
	         requirement can easily be broken by the presence of
	         symlinks. Therefore by default the scanner will halt
	         with an error if it detects a symlink. Symlinks can
	         be allowed by setting the lbr.storage.allowsymlink
	         tunable to 1. This should only be done if all the
	         symlinks only reference files and directories that
	         are not under any scanned directory.
	         The scanner will also not run if the depot is being shared
	         between servers.

	'p4 storage' requires 'admin' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpReload = { ErrorOf( ES_HELP, 155, E_INFO, EV_NONE, 0 ),
R"(
    reload -- Reload an unloaded client, label, or task stream

    p4 reload [-f] [-c client | -l label | -s stream] [-p address]

	Reloads the specified client, label, or task stream from the
	unload depot.

	Specify -c and a client name to reload that client, or -l and a label
	name to reload that label, or -s and a stream name to reload a task
	stream.

	The -f flag forces the reloading of the specified client, label,
	or task stream. By default, users can only reload their own clients,
	labels, or task streams.  The -f flag requires 'admin' access,
	which is granted by 'p4 protect'.

	Note that reloading a client or label does not update the access
	time. See 'p4 help unload' for more information about which commands
	cause the access time of a client or label to change.

	The '-p' flag can be used in multi-server environments to reload an
	unloaded client workspace or label from the edge or commit server
	specified by the address.
	This migrates the workspace or label from that server to this one. To
	perform this operation, the service user of each server must be
	properly authenticated with the other server.

	When used with the -c option, the client workspace will automatically
	be unloaded on the remote server and then reloaded if it had not been
	unloaded previously.

	The address parameter can be specified either as the P4PORT or as the
	server id of the server. If you specify a server id, the server spec
	must contain the correct P4PORT value in its Address field.
)"
};

ErrorId MsgHelp::HelpUnload = { ErrorOf( ES_HELP, 156, E_INFO, EV_NONE, 0 ),
R"(
    unload -- Unloads a client, label, or task stream to the unload depot

    p4 unload [-f -L -p -z] [-c client | -l label | -s stream]
    p4 unload [-f -L -z] [-a|-al|-ac] [-d date | -u user]

	The first form of this command unloads the specified client, label,
	or task stream to the unload depot.

	Specify -c and a client name to unload that client, or -l and a label
	name to unload that label, or -s and stream name to unload a task
	stream.

	The -f flag forces the unloading of the specified client, label, or
	task stream. By default, users can only unload their own clients,
	labels, or task streams.  The -f flag requires 'admin' access,
	which is granted by 'p4 protect'.

	The -p flag promotes any non-promoted (local) shelved changes owned
	by the specified client from an edge server to a commit server where
	it can be accessed by other edge servers.

	The -z flag specifies that the client, label, or task stream should
	be stored in compressed format.

	The -L flag specifies that the client, label, or task stream should
	be unloaded even if it is locked.

	If the -a flag is specified, the specified set of clients and/or
	labels are unloaded. Specify -d and a date to unload all clients
	and/or labels whose access date is older than that date. Specify -u
	and a user to unload all clients and/or labels owned by that user.
	To restrict the command to clients only, specify -ac rather than -a.
	To restrict the command to labels only, specify -al rather than -a.
	When -a is specified, you must specify either -d or -u (or both),
	and you may not specify the -c or -l flags.

	The access date for a client is updated when the client is used by
	a command which directly references that client. These commands
	include: add, change, client, delete, diff, edit, have, integrate,
	labelsync, lock, move, opened, reconcile, reopen, resolve, revert,
	shelve, submit, sync, unshelve, and where.

	The access date for a label is updated when the label is used by a
	command which directly references that label. These commands include:
	label, and labelsync

	In addition, the access date for a client is updated when the client
	is used in a revision specifier of the form @client, or when used as a
	template when creating a client with 'p4 client -t'. The access date
	for a label is updated when the label is used in a revision specifier
	of the form @label.

	The configurables dm.domain.accessupdate and dm.domain.accessforce
	control the frequency of access date updates.
)"
};


ErrorId MsgHelp::HelpAdministration = { ErrorOf( ES_HELP, 140, E_INFO, EV_NONE, 0 ),
R"(
    P4 commands for administering and operating the server:

	admin        Perform administrative operations on the server
	archive      Archive obsolete revisions to archive depots
	bgtask       Run server-side tasks in the background.
	cachepurge   Reclaim disk space on a replica
	configure    Set, unset, or show server configuration variables
	counter      Display, set, or delete a counter
	counters     Display list of known counters
	dbschema     Report information about metadata in the P4
	             database
	dbstat       Display size or simple statistics for a database table
	dbverify     Perform low-level verification of the database tables
	depot        Create or edit a depot specification
	depots       Display list of depots
	diskspace    Display summary of disk space usage on server
	extension    Manage the P4 server extensibility mechanism
	failback     Fail back to restore a previous master to its former role
	failover     Fail over to a standby server
	group        Change members of a user group
	groups       List groups (of users)
	heartbeat    Monitor responsiveness of a target server
	info         Print out client/server information
	jobspec      Edit the job template
	journals     Display the checkpoint and journal history
	license      Update or display the license file
	ldap         Create or edit an LDAP configuration
	ldaps        Display list of LDAP configurations
	ldapsync     Synchronize members of P4 groups with LDAP groups
	lockstat     Report lock status of database tables
	logappend    Append a line to server log file(s)
	logparse     Display parsed server log records
	logrotate    Rotate server log file(s)
	logschema    Display schema information for server log
	logstat      Report size of journal/errorLog/auditLog files
	logtail      Display the last block(s) of the errorLog
	monitor      Display current running P4 process information
	obliterate   Remove files and their history from the depot
	ping         Test network performance
	property     Add, delete, or list property values
	protect      Modify protections in the server namespace
	protects     Display protections in place for a given user/path
	reload       Reload metadata for an unloaded client or label
	renameclient Rename a client workspace throughout the database
	renameuser   Completely rename a user throughout the database
	resource-monitor Monitor system resources
	restore      Restore archived revisions to their original location
	storage      Display, verify, or update physical archive storage
	streamspec   Edit the stream template
	sysinfo      Collect/report system info
	topology     Display the list of servers in this installation
	triggers     Modify list of server triggers
	typemap      Modify the file name-to-type mapping table
	unload       Unload metadata for an unused client or label
	unzip        Import files from a p4 zip package file
	upgrades     Report server upgrades
	user         Create or edit a user specification
	users        Display list of known users
	verify       Verify that the server archives are intact
	zip          Package a set of files for use by p4 unzip

    See 'p4 help replication' for more information about additional
    commands and issues of interest to those who are configuring and
    operating replicated servers.

    See 'p4 help dvcs' for more information about additional commands and
    issues of interest to those who are configuring and operating
    independent decentralized servers.

    See 'p4 help 2fa' for more information about multi factor authentication.

    See 'p4 help realtime' for more information about realtime performance
    counters.

    See 'p4 help server-resources' for more information about how the server
    can be configured to have awareness of operating system resource limits.
)"
};

ErrorId MsgHelp::HelpReplication = { ErrorOf( ES_HELP, 162, E_INFO, EV_NONE, 0 ),
R"(
    Topics regarding the configuration and operation of replicated servers:

	buildserver        General information about Build Farm servers
	cachepurge         Reclaim disk space on a replica
	distributed        Topics relevant to a distributed installation
	export             Extract journal or checkpoint records
	forwardingreplica  General information about forwarding replicas
	journalcopy        Copy journal data from master to local filesystem
	journaldbchecksums Write journal notes with table checksums
	pull               Pull journal or file data from master
	replicate          Poll for journal data and apply to another server
	server             Create or edit a server specification
	servers            Display list of servers

    A replica is a server which has a copy of another server's data.

    Common reasons for setting up a replica server include: offloading work
    from the main server, and providing better performance for users in
    remote locations.

    There are different types of replica servers, such as read-only
    replicas, forwarding replicas, Edge Servers, etc. See 'p4 help server'
    for more details about server types and features. The process of
    adding a new replica server to an existing installation involves:
    preparing a machine with adequate system resources, configuring the
    new server, and initializing the new server from a copy of the target
    server's data and archives. Once the replica server is operational,
    it should be monitored routinely to ensure that any error conditions
    are detected and resolved.

    Replica data filtering is configured by specifying filter patterns in
    the replica's server spec and specifying the replica's serverid in the
    -P flag on the 'p4 pull' command to enable the filtering.

    A distributed installation contains a Commit Server and one or more Edge
    Servers. See 'p4 help distributed' for more information.

    Replica servers support a variety of approaches to handling the depot
    archives, selected by the lbr.replication configurable setting:

	none:              Replica has no access to depot archives. Commands
	                   which reference file content will be rejected.

	readonly:          Replica will maintain a readonly copy of the
	                   target server's archives. New files are copied by
	                   'pull -u' threads and purged files are removed.
	                   Commands which reference file content are allowed,
	                   and the file will be retrieved from the target if
	                   it is missing from the replica.

	shared:            The same physical archives are shared between the
	                   replica and the commit or master server either by
	                   running on the same machine or via a network share.
	                   The sharing of the archives is checked for as the
	                   replica starts.

	ondemand:          Transfer of archive files only happens if those
	                   files are referenced by a command and do not already
	                   exist on the replica.
	                   The physical archives may be shared between the
	                   replica and the commit or master server but no
	                   checks are made to determine this.
	                   This mode can also be used when an external archive
	                   synchronization technique such as 'rsync' is used
	                   for archives.

	cache:             Replica allows commands which reference file
	                   content, but does not automatically transfer new
	                   files. Files which are purged from the target are
	                   removed from the replica when the purge operation
	                   is replicated. If a file is not present in the
	                   archives, the replica will retrieve it from the
	                   target server.

    Edge servers have the additional ability to write shelved files to their
    own depot archive, except when lbr.replication=none.

    Automatic retrieval of new file content can be further controlled by the
    ArchiveDataFilter setting in the 'p4 server' spec.

    Connections made via forwarding replicas and Edge Servers may require
    special handling in the protections table if the Host: field specifies
    an IP address; see 'p4 help protect' for more information.

    A standby replica processes journal records from its master server in
    such a way that it can be used for failover if the master server dies.
    A standby replica uses both the 'journalcopy' and 'pull -L' commands
    together to ensure that the replica journals exactly match the master.

    Complete information about replica configuration and operation can be
    found in the P4 Server Administrator Guide: Multi-Site
    Deployment.

)"
};

ErrorId MsgHelp::HelpBuildserver = { ErrorOf( ES_HELP, 165, E_INFO, EV_NONE, 0 ),
R"(
	A server of type 'build-server' (see 'p4 help server') is a replica
	which supports build farm integration.

	A build-server replica supports the same read-only commands that a
	simple replica supports. In addition, the 'p4 client' command may be
	used to create or edit client workspaces on a build-server.

	Such workspaces may issue the 'p4 sync' command, in addition to all
	read-only commands supported by the replica. The 'p4 sync' command on
	a bound workspace is processed entirely by the build-server replica,
	which entirely relieves the master server of all computation, file
	transfer, networking, and database update resource usage for those
	sync commands.

	The build-server replica records view-mapping and have-list metadata
	in its local database, using the separate db.view.rp and db.have.rp
	database tables. Domain information for bound workspaces is recorded
	in the db.domain table, which is global to all servers in the
	installation. A workspace which is bound to a build-server must still
	have a globally-unique name; this is enforced by the 'p4 client'
	command on the build-server.

	When creating or editing a client workspace for a build-server,
	the client specified by the optional 'name' argument, as well as the
	client specified by the $P4CLIENT environment variable or via the
	global '-c client' argument must not exist, or must be restricted
	to this server; this command may not be used to create or edit a
	workspace that is not restricted to this build-server.

	Since workspace mapping and have list information for bound workspaces
	is stored locally in the build-server database, the build-server should
	be checkpointed regularly.

	Since the database of an (unfiltered) build-server is a superset of
	the database of the master server, an unfiltered build-server can
	be used to recover from the catastrophic loss of the master server.
)"
};

ErrorId MsgHelp::HelpForwardingreplica = { ErrorOf( ES_HELP, 166, E_INFO, EV_NONE, 0 ),
R"(
	A server of type 'forwarding-replica' (see 'p4 help server') is a
	replica which supports the full P4 command set.

	A read-only command which is received by a forwarding-replica is
	processed locally by the forwarding replica, without consuming any
	resources on the master server.

	An update command which is received by a forwarding-replica is
	forwarded to the master server for processing, similarly to the way
	in which a proxy or broker forwards commands to the server.

	Like a proxy, a forwarding-replica acts as a cache of file content,
	so commands such as 'p4 sync' or 'p4 resolve', although they are
	processed by the master server, offload their file transfer operations
	to the forwarding-replica, thus reducing the load on the master server.

	The 'p4 login' command on a forwarding-replica automatically logs the
	user into both the forwarding-replica and the master server.

	Since the database of an (unfiltered) forwarding-replica is a superset
	of the database of the master server, an unfiltered forwarding-replica
	can be used to recover from the catastrophic loss of the master server.
)"
};

ErrorId MsgHelp::HelpDistributed = { ErrorOf( ES_HELP, 163, E_INFO, EV_NONE, 0 ),
R"(
    A distributed installation contains a Commit Server and one or more Edge
    Servers. Individual client workspaces are bound to the Edge Server on
    which they are created, and all work in progress for those workspaces
    resides only on their owning Edge Server.

    An Edge Server supports the full P4 command set; however, there are a few
    differences in behavior which may affect applications.

    A pending change can be viewed on an Edge Server other than the one
    where the change was created. In that case, the pending change will not
    display a list of open files; the open files are shown only on the
    owning server. A pending changelist can be updated or deleted only on
    the server where the change was created.

    Files of type +l are handled specially in a distributed installation.
    When +l filetypes are opened, the Edge Server contacts the Commit Server
    to ensure that the file is not locked on another Edge Server.

    By default, labels are also bound to the Edge Server on which they are
    created; the -g flag indicates that the label is to be defined globally
    on all servers in the installation. Configuring rpl.labels.global=1
    reverses this default and causes the flag to have the opposite meaning.

    When moving your workspace from one server to another, using 'sync -k'
    and 'reconcile' makes it easy to migrate without needing to reload your
    work. An alternate mechanism is to unload your workspace on the old
    server and use 'reload -p' to reload it onto the new server. In either
    case, shelves created by the workspace remain on the server where they
    were created. The unload/reload mechanism can also be used to move a
    locally-bound label from one Edge Server to another.

    A spec depot on an Edge Server contains all the contents of the spec
    depot on the Commit Server. Additionally, the Edge Server spec depot
    contains entries for clients and labels bound to that Edge Server.
    Consider using a Typemap definition such as text+CS4 for client and
    label spec depot entries, or using a SpecMap field on the spec depot
    definition to ensure that the Edge Server spec depot does not grow
    excessively large.

    An unload depot on an Edge Server contains all the contents of the
    unload depot on the Commit Server. Additionally, the Edge Server unload
    depot contains entries for clients and labels that were unloaded on
    that edge server.

    Shelved files behave differently in a distributed configuration. A
    shelf is accessible only on the Edge Server where it was created, unless
    it is promoted (see the discussion of the 'shelve' command below).

    When backing up your Edge Server(s), ensure that you checkpoint all
    database tables and retain rotated journals between checkpoints.
    Also, ensure that you back up any shelved file content, unloaded clients
    and labels, and spec depot entries for clients and labels that have been
    created on this Edge Server, as that data is local to this Edge Server
    and not contained in the backups of your Commit Server.

    Most commonly-used commands are executed directly on this Edge Server
    and are unaffected by resource constraints on other servers in your
    installation. However, there are certain performance implications of a
    distributed installation that you should anticipate:
    - Files using the RCS storage format undergo multiple file format
      conversions during submit. Use text+C or text+F storage formats instead
      to avoid this penalty.
    - The submit command is slower than in a non-distributed installation,
      since the changelist must be copied from the Edge Server to the Commit
      Server during submit processing. If you are creating a new branch, use
      the populate command instead to avoid this overhead.
    - Updates to workspace and label definitions, requests for new change
      numbers, and opening or reverting +l files require messages to be sent
      to the Commit server.
    - Background 'pull' threads in the Edge Server may contend for resources
      with commands run by Edge Server users.

    Notable differences in command behavior in a distributed installation:

    changes -s shelved file...:
        Searching for shelves affecting particular files can be done only
        on the Edge Server where the shelf was created, or on the Commit
        Server if the shelf was promoted.

    client, clients -a -s:
        A client workspace is bound to its Edge Server when it is created,
        and may not be used on other Edge Servers. The 'p4 clients' command
        displays only those clients bound to this Edge Server; use the -a
        flag to display the complete list of clients, or the -s serverID
        flag to display the list of clients bound to a specific Edge Server.

    describe, change
        The files associated with a pending change are visible only on the
        Edge Server where the change was created.

    label, labels -a -s:
        A label is present only on the Edge Server where it was created.
        The 'p4 labels' command displays only those labels bound to this
        Edge Server; use the -a flag to display the complete list of labels,
        or the -s serverID flag to display labels of a specific Edge Server.
        Labels created on the Commit Server are global, and are visible on
        all servers in the distributed installation.

    label -g, labelsync -g, tag -g:
        The -g flag specifies that the label, labelsync, or tag command should
        execute on the Commit Server to create or update a globally-visible
        label; otherwise the label is present only on this Edge Server.

    labels file[revrange]:
        When a file specification is provided, the 'p4 labels' command
        displays the matching labels, but only shows labels bound to this
        Edge Server. The file specification argument is not allowed if the
        -a or -s flags are provided.

    lock, unlock:
        The lock and unlock commands only prevent submissions from other
        clients on this Edge Server.

    logger:
        In a distributed installation, 'p4 logger' commands should be issued
        to the Commit Server, not to an Edge Server.

    obliterate:
        The obliterate command removes submitted revision data from the
        Commit Server, and also removes any related client data for clients
        resident on the Commit Server. When the file removal operations are
        replicated to an Edge Server, that Edge Server automatically reverts
        any open changes to those files for clients resident on that Edge
        Server. Edge Server clients may still list the obliterated file(s)
        in the output of 'p4 have'; the file(s) will be removed from their
        have list the next time that client issues a 'p4 sync'.

    opened -a:
        Displays only those files opened by other workspaces on this Edge
        Server; files opened on other Edge Servers do not appear.

    opened -x:
        Displays information about exclusively locked files.

    reload -p:
        Allows an unloaded client or label to be reloaded on a different
        server. Note that any shelves which were created by the client
        remain on the server where they were created.

    shelve, unshelve:
        A shelf is present only on the Edge Server where it is created, and
        can only be unshelved by workspaces on that Edge Server. A shelf can
        be promoted by specifying the '-p' flag; this causes the shelf data
        to be redundantly stored on the Commit Server as well as on this
        Edge Server. A promoted shelf is accessible by users on other Edge
        Servers, although there is a performance penalty as the shelf data
        must be retrieved on demand when it is referenced. Although it is
        accessible from other Edge Servers, the shelf can only be submitted
        from the Edge Server on which it was created. On an Edge Server
        which is running with lbr.replication=shared, all new shelves
        are automatically promoted without needing '-p'. If a workspace
        has promoted shelves, those shelves must be deleted prior to
        deleting, unloading, or migrating that workspace to another server.

    spec depot:
        The spec depot may have different contents on each Edge Server.
        Clients and labels bound to an Edge Server are recorded in the
        spec depot on that Edge Server only. The 'depot -d' command for the
        spec depot does not verify that the Edge Server spec depots are empty.

    triggers:
        The submit command on the Edge Server will run the 'edge-submit'
        and 'edge-content' triggers, if defined, prior to transferring
        the changelist to the Commit Server for final submission. The submit
        command on the Commit Server will run the standard trigger types;
        however, if the change was submitted from an Edge Server, the
        'change' and 'describe' commands must not be used in the trigger.
        The %%serverid%% variable in the trigger command should be used for
        trigger definitions which need to detect which server is running
        the trigger.

    unload depot:
        The unload depot may have different contents on each Edge Server.
        Clients and labels bound to an Edge Server are unloaded into the
        unload depot on that Edge Server. Specify the -s or -a flag to the
        'p4 clients -U' and 'p4 labels -U' commands on the Edge Server to
        view unloaded clients or labels for another Edge Server.

        Be sure to include the unload depot as part of your Edge Server
        backups. Since the Commit Server does not verify that the unload
        depot is empty on every Edge Server, you must specify 'depot -d -f'
        in order to delete the unload depot from the Commit Server.

    unlock -x:
        Releases exclusive locks which have become orphaned due to network
        outages between the Edge Server and the Commit Server.

    unsubmit, resubmit:
        These commands are not available on an Edge Server.

    user -i -f:
        Automatic creation of new user specs is disabled on Edge Servers.
        To create a new user, the administrator should run 'p4 user -f'.

    archive, attribute -f, labelsync -g, obliterate, populate, restore,
    retype, snap, tag -g:
        When one of these commands is issued to an Edge Server, the command
        is forwarded to the Commit Server for processing. In this case,
        specify any arguments to the command using depot syntax, not client
        syntax nor local syntax. In order to use client or local syntax for
        these commands, you must issue the command directly to the Commit
        Server (and use a workspace bound to the Commit Server).
)"
};

ErrorId MsgHelp::HelpCachepurge = { ErrorOf( ES_HELP, 168, E_INFO, EV_NONE, 0 ),
R"(
    cachepurge -- Reclaim disk space on a replica

    p4 cachepurge -a [-i N -n -R -S N -O] [-D file[revRange]]
    p4 cachepurge -f N [-i N -n -R -S N -O] [-D file[revRange]]
    p4 cachepurge -m N [-i N -n -R -S N -O] [-D file[revRange]]
    p4 cachepurge -s N [-i N -n -R -S N -O] [-D file[revRange]]

	A replica used as a standby spare or for disaster recovery will
	maintain a complete copy of the master server's versioned file
	archives. Replicas which are used for other purposes may not need
	to hold a copy of the content of every version of every file.

	The cachepurge command is useful for replicas with limited disk space
	resources. Depending on the scenario, a replica may not need to
	retain a copy of all of the versioned file archives of its master. If
	the replica is not being used for disaster recovery purposes, you can
	reclaim disk space by periodically deleting versioned files from the
	replica. Note: This is only safe to do if you are certain that you
	have a backup of these files, either a backup made on this replica or
	on another replica or on a backup made on the master server.

	The disk space goal is specified using one of the required flags:
	-a, -f, -m, or -s.

	Each time the cachepurge command runs, it attempts to permanently
	delete enough file content from the replica to achieve the goal
	specified by parameters to the command. Note that the file content
	is only deleted from this replica, not from the master server or any
	other replica, and also note that if a command is issued to this
	replica which accesses the file content in the future, the file will
	be re-retrieved from the master.

	The -a flag specifies to delete all file content. This reclaims the
	maximum amount of disk space, but also means any file content must
	be re-retrieved from the master.

	The -f flag specifies a free space target for the filesystem in bytes.
	When used regularly, this will cause the replica to maintain that
	amount of free disk space on the depot filesystem.

	The -m flag specifies the number of file revisions to delete. This
	may free up more or less space, depending on the size of the files.

	The -s flag specifies the number of bytes of file data to delete.
	This can be helpful for replicas which have a predictable observed
	growth rate in their filesystem resources.

	The -i flag causes the command to automatically repeat its action
	every N seconds. If -i is not specified, the command runs once,
	then exits.

	The -n flag displays a preview of the cachepurge operation without
	deleting any files.

	The cachepurge command normally examines all files in the repository.
	The -D flag may be used to specify the subset of the files that are
	to be eligible for purging.

	The cachepurge command normally chooses files to delete at random.
	The -R flag inhibits this random behavior, and causes files to be
	deleted in order. The -a flag implies the -R flag.

	The -S flag specifies that the N most recent revisions of each file
	are not to be removed. Specifying -S 1 therefore ensures that the
	head revision of each file is retained in the replica's cache if it
	is already present there.

	The -O flag specifies that the files are to be ordered from oldest to
	newest, in order to prefer the deletion of older content over newer.
	The -O flag requires that the -R flag also be specified.
)"
};

ErrorId MsgHelp::HelpTrust = { ErrorOf( ES_HELP, 150, E_INFO, EV_NONE, 0 ),
R"(
    trust -- Establish trust of an SSL connection

	p4 trust is a client side command.  For detailed help run
	p4 trust -h
)"
};

ErrorId MsgHelp::HelpRenameClient = { ErrorOf( ES_HELP, 275, E_INFO, EV_NONE, 0 ),
R"(
    renameclient -- Rename a client throughout the database

    renameworkspace -- synonym for 'renameclient'

    p4 renameclient --from=old --to=new

	'p4 renameclient' renames a client, modifying relevant database
	records which reference the client as needed.

	This includes pending and shelved changes created by the client,
	any files that the user has opened or shelved, any fixes that the
	user made to jobs associated with pending changes, files on the
	client's have list, and the client record itself. Note that this
	command does not modify submitted changes nor fixes that are
	associated with submitted changelists. The intent of this
	command is to create a new client, migrate all existing work
	in progress to that new client, and then delete the old client.

	The client name is not changed in descriptive text fields (such as job
	descriptions, change descriptions, or workspace descriptions). The
	client is modified only where it appears as a field or as part of a
	client path of a database record.

	This command will add an entry to the spec depot for the new client,
	and will add a deletion entry for the old client.

	This command does not process unloaded clients, so any unloaded
	client to be renamed should be reloaded first.

	This command is not supported for use with partitioned or readonly
	clients, clients with opened streams, or clients with promoted shelved
	changes. The client cannot be renamed to a client that already exists.

	'p4 renameclient' by default requires 'admin' access granted
	by 'p4 protect', or must be the owner of the client to be renamed.
	The 'run.renameclient.allow' configurable can be used to restrict
	users that can run the 'p4 renameclient' command.
)"
};

ErrorId MsgHelp::HelpRenameUser = { ErrorOf( ES_HELP, 169, E_INFO, EV_NONE, 0 ),
R"(
    renameuser -- Completely rename a user throughout the database

    p4 renameuser [-f] --from=old --to=new

	'p4 renameuser' completely renames a user, modifying all database
	records which mention the user.

	This includes all workspaces, labels, branches, streams, etc. which
	are owned by the user, all pending, shelved, and committed changes
	created by the user, any files that the user has opened or shelved,
	any fixes that the user made to jobs, any properties that apply to
	the user, any groups that the user is in, and the user record itself.

	The username is not changed in descriptive text fields (such as job
	descriptions, change descriptions, or workspace descriptions), only
	where it appears as the owner or user field of the database record.

	Protection table entries that apply to the user are updated only if
	the Name: field exactly matches the user name; if the Name: field
	contains wildcards, it is not modified.

	The only job field that is processed is attribute code 103. If you
	have included the username in other job fields they will have to be
	processed separately.

	This command does not modify existing content in the spec depot.

	Files of type +k which contain the $Author$ tag that were submitted
	by the user will have incorrect digests following this command. Use
	'p4 verify -v' to recompute the digest value after the rename.

	This command does not process unloaded workspaces, so all the user's
	workspaces should be reloaded (or deleted) first.

	To avoid confusion, the user should not be using the server at the
	moment this command is run. After this command completes, the user
	should logout and login again, and can then resume work. Similarly,
	the user issuing this command should not be the same user as the user
	being renamed.

	For best results, this command should not be used if the new user has
	been used in this server already, since work performed by that user
	may be unintentionally combined with work performed by the renamed
	user. The server attempts to check for such accidentally merging of
	two unrelated users, but the check is not perfect.

	The -f flag bypasses the accidental merge checks mentioned above.

	If a central authentication server has been configured using P4AUTH,
	the user must be renamed in both servers, using separate invocations
	of 'p4 renameuser'. The commands may be run in either order.

	Performance may be affected while the command is running due to the
	locking of multiple database tables and processing of file revisions
	with the ktext (text+k) file type.
	It is recommended that the command be run during a maintenance or
	quiet period on the server.

	File revisions that use the %Author% RCS keyword will require their
	MD5 digest stored in the database to be updated using 'p4 verify'.

	'p4 renameuser' requires 'super' access granted by 'p4 protect'.
)"
};

ErrorId MsgHelp::HelpJournals = { ErrorOf( ES_HELP, 170, E_INFO, EV_NONE, 0 ),
R"(
    journals -- Display the checkpoint and journal history of the server

    p4 journals [-F filter -T fields -m max]

	'p4 journals' displays information about the checkpoint and journal
	history of this server.

	The -F flag lists only records satisfying the filter expression. This
	filter syntax is similar to the one used for 'jobs -e jobview' and is
	used to evaluate the contents of the fields in the preceding list.
	Filtering is case-sensitive.

	        Example: -F type=checkpoint

	The -T fields flag returns only the specified fields. The field names
	can be specified using a comma- or space-delimited list.

	The -m max flag limits output to the specified number of records.
)"
};

ErrorId MsgHelp::HelpPush = { ErrorOf( ES_HELP, 175, E_INFO, EV_NONE, 0 ),
R"(
    push -- copy files from a personal server to a shared server

    p4 push [-n -r remotespec -v -O flags] [-S stream | filespec]
    p4 push [-n -r remotespec -v -O flags] -s shelf

	The push command takes the specified set of files, and the
	changelists which submitted those files, and copies them to
	the specified target server.

	A push is only allowed if the files being pushed fit cleanly into
	the target server, building precisely on a shared common history.
	If there are any conflicts or gaps, the push is rejected.
	Otherwise, the changelists become new submitted changelists
	in the target server.

	The second form of the command pushes a shelved changelist, rather
	than one or more submitted changelists, in which case conflicts do
	not arise; the result is a new shelved change in the target server.

	When the changelists are added to the target server, they
	are given newly assigned change numbers but they retain the
	same description, user, date, type, workspace, and set of files.

	When the files are added to the target server, they are kept
	in their same changelists, as new revisions starting after the
	current head. The new revisions retain the same revision number,
	file type, action, date, timestamp, digest, and file size.

	Although the changelists are new submitted changelists in the
	target server, none of the submit triggers are run in the
	target server. Instead, use the push-* triggers for that purpose.

	The push command can also be used to copy a shelf to the target
	server by specifying the -s flag.

	Typically, the push command specifies a remote spec, and the
	DepotMap field in the remote spec specifies which files are to be
	pushed. The push command may also specify an argument to further
	restrict the files to be pushed.

	The restriction argument may be either the name of a stream
	(e.g., '-S dev'), or it may be a filename pattern (e.g.,
	//stream/dev/...). You may not specify both types of restrictions
	in a single push command.

	If a stream name or filespec is provided, and the remote spec uses
	differing patterns for the local and remote sides of the DepotMap,
	the stream name or filespec argument must specify the files using the
	local filename syntax.

	If a particular changelist includes some files that match the
	restriction, and other files that do not, then only the matching
	files are included in the push. Note that if a remote spec is also
	provided, only the files that match the restriction and are mapped by
	the remote spec are included in the push.

	In addition to the file revisions and the changelists, the push
	command also copies the archive content to the target server.
	In the case of lazy copies, the target server may already have the
	corresponding archive, in which case the lazy copy is adjusted
	to reference the target server's existing archives.

	The push command also copies all integration records which
	describe integrations to the files being pushed. These integration
	records are adjusted in the target server to reflect the resulting
	changelist numbers and revision numbers of the target server.
	Integration records are copied only if the integration source is
	mapped by the remote spec specified for the push.

	The push command also copies all attributes which have been set on
	the files being pushed using 'p4 attribute', and copies all fixes
	which have been attached to the changelists being pushed, providing
	the jobs mentioned by those fixes exist in the destination server.

	In order to push a set of files, you must have read access to those
	files in the current repository, and you must have write access to
	those same files in the target repository (according to the target
	repository's protections table); your local userid is also used as
	the userid at the target server and you must already be logged in
	to both servers prior to running the push command.

	The push command is atomic: either all the specified files
	are pushed, or none of them are pushed.

	By default, changes cannot be pushed from server to server; in order
	to push changes between servers, an administrator of each server must
	enable pushing. Set server.allowpush=1 on the server which initiates
	the push; set server.allowpush=2 on the destination server.

	Files with the filetype modifier +S have some special considerations.
	When pushing files of type +S, old archives which exceed the
	specified limit are not purged by the push command.

	If a push command fails, it will leave those files locked on the
	remote server. 'p4 opened' will display '*locked*', and the files
	cannot be submitted by any other user. If the push command cannot be
	quickly retried, you can use the 'p4 unlock -r' commmand to unlock
	the files on the remote server. By default, the files in a failed
	push operation are left locked. To change that behavior, set the
	push.unlocklocked configurable which unlocks the files upon a
	push failure.

	The push command is not allowed if there are unsubmitted changes in
	this server; use 'p4 resubmit' to resubmit those changes first, or
	discard the shelves with 'p4 shelve -d' if they are not wanted.

	The -r flag specifies a remote spec which contains the address of
	the target server, and a file mapping which is to be used to re-map
	the files when they are pushed to the target server. If -r is not
	specified, the default is -r origin. See also 'p4 help remote'.

	The --remote-user flag may be used to specify the username for the
	target server, overriding any RemoteUser field in the remote spec.

	The --preserve-change-numbers flag may be used to preserve the
	pushed change numbers on the target server. Push will fail
	if there are any conflicting changelist numbers.

	The -S flag specifies a stream name to restrict the files to be
	included in the push.

	The -n flag performs all the correctness checks, but does not
	push any files or changelists to the target server.

	The -s flag specifies a shelved changelist to be copied, instead
	of copying one or more submitted changelists.

	The -v flag specifies verbose mode.

	The -O flag controls the output verbose mode, and has the format
	'-Ocfi'. If 'c' is specified, verbose mode includes information about
	each changelist; if 'f' is specified, verbose mode includes
	information about each file; if 'i' is specified, verbose mode
	includes information about each integration. The default is -Oc.

)"
};

ErrorId MsgHelp::HelpUnzip = { ErrorOf( ES_HELP, 176, E_INFO, EV_NONE, 0 ),
R"(
    unzip -- Import files from a p4 zip package file

    p4 unzip -i file [-f -n -A -I -v --enable-dvcs-triggers]

	The unzip command imports the file revisions contained in the
	specified zip file.

	The -i flag specifies the zip file name.

	The -n flag performs all the correctness checks, but does not
	push any files or changelists to the target server. Any archives
	that are missing from the zip file will be reported.

	The -f flag bypasses the correctness checks.

	The -A flag includes the archive content of the new revisions.

	The -I flag excludes integration records for the new revisions.

	The -v flag specifies verbose mode.

	The --enable-dvcs-triggers flag specifies that any push-* and fix-add
	triggers which have been defined are enabled for this unzip command.
	Otherwise, unzip does not run those triggers.

	The unzip command requires admin permission granted by
	p4 protect.
)"
};

ErrorId MsgHelp::HelpUpgrades = { ErrorOf( ES_HELP, 267, E_INFO, EV_NONE, 0 ),
R"(
    upgrades -- Report server upgrades.

    p4 upgrades [-g upgrade-step ]

	'p4 upgrades' shows the status of the each upgrade step on the
	current server.

	The -g flag queries whether the specified upgrade step has been
	completed on this and all upstream servers in multi-server
	environments.

	The possible upgrade states are:
	    NEEDED:    The upgrade has not run on this server yet.
	    PENDING:   An upstream server has completed the upgrade and
	               is scheduled to run on this replica.
	    RUNNING:   The upgrade is currently running on this server.
	    COMPLETED: The upgrade has successfully completed on this and all
	               upstream servers.
	    ISOLATED:  The upgrade has successfully completed on this replica,
	               but it was not triggered by replication.
	    FAILED:    The upgrade has failed on this server.

	'p4 upgrades' requires 'super' access, which is granted by
	'p4 protect'.
)"
};

ErrorId MsgHelp::HelpZip = { ErrorOf( ES_HELP, 177, E_INFO, EV_NONE, 0 ),
R"(
    zip -- Package a set of files and their history for use by p4 unzip

    p4 zip -o file [-r remote -A -I] [ filespec | -c change]
    p4 zip -o file -A [-r remote -I] -s shelf

	The zip command takes the specified set of files, and the
	changelists which submitted those files, and writes them to
	the specified zip file.

	The content of the zip file can be specified either by providing a
	filespec, which selects a set of revisions, or by providing one or
	more changelist numbers using the -c flag, which selects all the
	revisions modified by those changelists.

	The second form of the command writes a single shelved changelist
	to the specified zip file. A shelf zip may not omit archives.

	The -o flag specifies the zip file name. NOTE: this file is
	created directly on the server, and the file name should be specified
	accordingly, using file name syntax appropriate for the server.
	A file name with a relative path is created relative to the server's
	P4ROOT directory.

	The -r flag specifies a remote spec which is to be used to re-map
	the files when they are written to the zip file.
	See 'p4 help remote'.

	The -c flag specifies the change to export; this flag can be
	specified multiple times to include multiple changes.

	The -A flag includes the archive content of the new revisions.

	The -I flag excludes integration records for the new revisions.

	The -s flag specifies a shelved changelist to be exported, instead
	of exporting one or more submitted changelists.

	The zip command requires super permission granted by p4 protect.

)"
};


ErrorId MsgHelp::HelpUnsubmit = { ErrorOf( ES_HELP, 178, E_INFO, EV_NONE, 0 ),
R"(
    unsubmit -- unsubmit a changelist, making it a shelf

    p4 unsubmit [-n -r remote] file[revRange] ...

	The unsubmit command takes one or more submitted changelists and
	undoes the submission, leaving the changelist as a shelved change
	with the same content. The changelist can then be unshelved and
	further updated prior to resubmitting it.

	See 'p4 help resubmit' for information about the 'p4 resubmit'
	command, which is useful for resubmitting unsubmitted changelists
	in a controlled process.

	revRange is a revision or a revision range that limits the span of
	file history to be unsubmitted.

	If the file argument has a revision, the specified revision is
	unsubmitted.  If the file argument has a revision range, the
	revisions in that range are unsubmitted.  See 'p4 help revisions'
	for help.

	In all of these cases, all changes which affect any of the specified
	revision(s) of any of the specified file(s) will be unsubmitted.

	The changelist must have been submitted by the same user and
	workspace which are used in the unsubmit command.

	The files in the changelist must be the head revisions of those
	files in the repository. The files must not have been integrated
	into any other files in the repository. The files must not be open
	by any pending or shelved changelists. The files must not have been
	archived or purged. The files must not have associated attributes.

	If the command specifies multiple files and/or multiple revisions,
	all the changelists which affected the specified revisions of the
	specified files are unsubmitted; each such change becomes its own
	separate shelf.

	Fix records associated with the changelist are not modified. After
	unsubmitting a change which has associated jobs, you should review
	the job and fix status for accuracy.

	The shelved changelists that are created do not fire any triggers
	of type shelve-submit or shelve-commit.

	The -n flag performs all the correctness checks, but does not
	unsubmit any files.

	The -r flag specifies a remote spec. The map in the remote spec is
	used to limit the files affected by the unsubmit command. Thus a
	command such as 'p4 unsubmit -r rmt @>=17' will affect only the
	files specified by the remote spec, not all files in the depot.

	After all the specified changelists have been unsubmitted, the
	unsubmit command syncs the workspace to the head revision.

	By default, a server does not allow the unsubmit command. To enable
	this command, an administrator must set server.allowrewrite=1.

	In order to delete a shelf which was unsubmitted, you must specify
	-f to the shelve -d command.

	The unsubmit command is not available on an Edge Server.

	The unsubmit command requires admin permission granted by
	p4 protect.

)"
};

ErrorId MsgHelp::HelpRemote   = { ErrorOf( ES_HELP, 179, E_INFO, EV_NONE, 0 ),
R"(
    remote -- Create, modify, or delete a remote specification

    p4 remote [-f] remoteID
    p4 remote -d [-f] remoteID
    p4 remote -o remoteID
    p4 remote -i [-f]

	A remote specification describes the high-level configuration
	and usage of a remote.

	The 'remote' command puts the remote spec into a temporary file and
	invokes the editor configured by the environment variable $P4EDITOR.
	Saving the file creates or modifies the remote spec.

	Remote specs are used by the fetch, push, and clone commands. For
	the fetch and push commands, the specified remote spec must be
	present on the server which initiates the fetch or push operation.
	For the clone command, the specified remote spec must be present on
	the server being cloned; the clone command copies the remote spec
	to the new personal repository and renames the spec to 'origin'.

	The remote spec contains the following fields:

	RemoteID:    The identifier of the remote

	Address:     The P4PORT that is used by the server.

	Owner:       The user who created this remote spec. Can be changed.

	RemoteUser:  The username to use on the remote server (optional).
	             Can be changed.

	Update:      The date this remote spec was last modified.

	Access:      The last time this remote was used to fetch or push.

	Description: A description of the remote spec (optional).

	Options:     Flags to change the remote spec behavior. The defaults
		     are marked with *.

	    locked       Permits only the owner to change the remote,
	    unlocked *   and prevents the remote spec from being deleted.

	    compress     Compresses data sent between the local and
	    nocompress * remote server to speed up slow connections.

	    copyrcs *    Transfers entire RCS archives when possible,
	    nocopyrcs    or never transfers entire RCS archives.

	LastFetch: The last changelist that was fetched.

	LastPush:  The last changelist that was pushed.

	DepotMap:
	           Mapping between the local and remote files.

	ArchiveLimits:
	           Limits on the number of files fetched (optional).

	RemoteCharset:
	           Sets charset to be used when fetching from a
	           non unicode mode server to a unicode mode server.
	           This charset will be used to convert the non unicode
	           mode server's text data into UTF8 (optional).

	The filenames in the depot map must always be in depot syntax, but
	the local server's depot syntax may differ from the remote server's
	depot syntax in order to map the remote server's files into a
	alternate location in the local server. The left side of the map
	specifies the local server location; the right side of the map
	specifies the remote server location.

	Any mapping of depot files may be specified in a remote spec, and
	will be used by the fetch and push commands to guide their operation.
	The clone command has certain restrictions on the DepotMap field,
	which are described in the next paragraph.

	The clone command interprets the remote spec and uses it to guide
	the setup of the new personal repository. The left side of the map
	is used to determine the depot and stream(s) that will be created.
	The left side of the map must only specify mappings for a single
	depot, but the right side of the map may have locations in multiple
	depots of the server being cloned. The left side of the map may
	specify one or more stream names in the new personal repository. The
	stream name is the second level in the path name on the left side.
	The left side must specify at least one stream, or clone will fail.
	If multiple streams are specified on the left side of the map, each
	stream will be created by the clone command as a separate mainline,
	and the last stream in the map will be the current stream after the
	clone command has finished.

	The -d flag deletes the named remote spec.

	The -o flag writes the remote spec to standard output. The user's
	editor is not invoked.

	The -i flag causes a remote spec to be read from the standard input.
	The user's editor is not invoked.

	The -f flag enables a user with 'admin' privilege to delete the spec
	or set the 'last modified' date.  By default, specs can be deleted
	only by their owner.

)"
};

ErrorId MsgHelp::HelpRemotes  = { ErrorOf( ES_HELP, 180, E_INFO, EV_NONE, 0 ),
R"(
    remotes -- Display list of remote specifications

    p4 remotes [[-e|-E] nameFilter] [-m max] [-u user]

	Lists remote specifications. (See 'p4 help remote'.)

	The -e nameFilter flag lists remote specs with a name that matches
	the nameFilter pattern, for example: -e 'svr-dev-rel*'. The -e flag
	uses the server's normal case-sensitivity rules. The -E flag makes
	the matching case-insensitive, even on a case-sensitive server.

	The -m max flag limits output to the specified number of remote specs.

	The -u user flag lists remote specs owned by the specified user.

)"
};

ErrorId MsgHelp::HelpFetch  = { ErrorOf( ES_HELP, 181, E_INFO, EV_NONE, 0 ),
R"(
    fetch -- copy files from a shared server to a personal server

    p4 fetch [-r remotespec -m N -v -O flags -k -n -t] [-S stream | filespec]
    p4 fetch [-r remotespec -v -O flags -n] -s shelf

	The fetch command copies the specified set of files, and the
	changelists which submitted those files, into this server from
	the specified remote server.

	A fetch is only allowed if the files being fetched fit cleanly into
	this server, building precisely on a shared common history.
	If there are no conflicts, the files and their changelists become
	new submitted changelists in this server.

	The second form of the command copies a shelved changelist, rather
	than one or more submitted changelists, in which case conflicts do
	not arise; the result is a new shelved change in the target server.

	Conflict handling is enabled using the -t flag. If no conflict
	handling was specified, and there are any conflicts or gaps,
	the fetch is rejected.

	After handling the conflicting changes, the remote work is then
	fetched. After the fetch completes, use 'p4 resubmit' to resubmit
	the conflicting local changes.

	When the changelists are added to this server, they
	are given newly assigned change numbers but they retain the
	same description, user, date, type, workspace, and set of files.

	When the files are added to this server, they are kept
	in their same changelists, as new revisions starting after the
	current head. The new revisions retain the same revision number,
	file type, action, date, timestamp, digest, and file size.

	Although the changelists are new submitted changelists in this
	server, none of the submit triggers are run in this server.

	Typically, the fetch command specifies a remote spec, and the
	DepotMap field in the remote spec specifies which files are to be
	fetched. The fetch command may also specify an argument to further
	restrict the files to be fetched.

	The restriction argument may be either the name of a stream
	(e.g., '-S dev'), or it may be a filename pattern (e.g.,
	//stream/dev/...). You may not specify both types of restrictions
	in a single fetch command.

	If a stream name or filespec is provided, and the remote spec uses
	differing patterns for the local and remote sides of the DepotMap,
	the stream name or filespec argument must specify the files using the
	local server's depot syntax. Note that the filespec must always be
	provided using depot syntax, not client syntax nor filesystem syntax.

	If a particular changelist includes some files that match the
	restriction, and other files that do not, then only the matching
	files are included in the fetch. Note that if a remote spec is also
	provided, only the files that match the restriction and are mapped by
	the remote spec are included in the fetch.

	In addition to the file revisions and the changelists, the fetch
	command also copies the archive content to this server.
	In the case of lazy copies, this server may already have the
	corresponding archive, in which case the lazy copy is adjusted
	to reference this server's existing archives.

	The fetch command also copies all integration records which
	describe integrations to the files being fetched. These integration
	records are adjusted in this server to reflect the resulting
	changelist numbers and revision numbers of this server.
	Integration records are copied only if the integration source is
	mapped by the remote spec specified for the fetch.

	The fetch command also copies all attributes which have been set on
	the files being fetched using 'p4 attribute', and copies all fixes
	which have been attached to the changelists being fetched, providing
	the jobs mentioned by those fixes exist in the destination server.

	In order to fetch a set of files, you must have read access to those
	files in the remote repository, and you must have write access to
	those same files in this repository; your local userid is used as
	the userid at the target server and you must already be logged in
	to both servers prior to running the fetch command.

	By default, changes cannot be fetched from server to server; in order
	to fetch changes from another server, an administrator of each server
	must enable fetching. Set server.allowfetch=1 on the server which
	initiates the fetch; set server.allowfetch=2 on the remote server.

	The fetch command is atomic: either all the specified files
	are fetched, or none of them are fetched.

	Files with the filetype modifier +S have some special considerations.
	When fetching files of type +S, old archives which exceed the
	specified limit are not purged by the fetch command.

	The -r flag specifies a remote spec which contains the address of
	the target server, and a file mapping which is to be used to re-map
	the files when they are fetched from the target server. If -r is not
	specified, the default is -r origin. See also 'p4 help remote'.

	The --remote-user flag may be used to specify the username for the
	target server, overriding any RemoteUser field in the remote spec.

	The --preserve-change-numbers flag may be used to preserve the
	change numbers of the fetched changelists on this server. Fetch
	will fail if there are any conflicting changelist numbers.

	The -m flag specifies to perform a shallow fetch; only the last N
	revisions of each file are fetched.

	The -n flag performs all the correctness checks, but does not
	fetch any files or changelists from the target server.

	The -s flag specifies a shelved changelist to be fetched, instead
	of fetching one or more submitted changelists.

	The -S flag specifies a stream name to restrict the files to be
	included in the fetch.

	The -t flag specifies that conflicting changes should be moved to
	a new tangent, as described earlier. The -t flag requires the -r
	flag. The -t flag also requires that an administrator has set
	server.allowrewrite=1. In order to use the -t flag, you must have
	admin access to the files to be fetched.

	The -k flag specifies that the fetch command should not sync files.
	Normally, after fetching new work, the fetch command automatically
	syncs your workspace to the head revision. The -k flag prevents that
	automatic sync operation.

	The -v flag specifies verbose mode.

	The -O flag controls the output verbose mode, and has the format
	'-Ocfi'. If 'c' is specified, verbose mode includes information about
	each changelist; if 'f' is specified, verbose mode includes
	information about each file; if 'i' is specified, verbose mode
	includes information about each integration. The default is -Oc.

)"
};

ErrorId MsgHelp::HelpDvcs  = { ErrorOf( ES_HELP, 182, E_INFO, EV_NONE, 0 ),
R"(
    Using P4 as a decentralized version control system:

	init       Create a new personal server
	clone      Clone a new personal server from a shared server
	switch     Switch to a new stream, optionally creating it
	remote     Define a connection to a shared server
	remotes    List the known shared servers
	fetch      Copy files from a shared server to a personal server
	push       Copy files from a personal server to a shared server
	unsubmit   Unsubmit a change, leaving the work in a shelf
	resubmit   Resubmit unsubmitted changes
	zip        Package a set of files for use by p4 unzip
	unzip      Import files from a p4 zip package file

    P4 can be used as either a centralized version control system, or as a
    decentralized version control system, or as a blend of both approaches, to
    accomplish the following:

    - You can create a personal server that you can use to manage
      versioned content all by yourself, without interacting with
      any other P4 Server unless you want to.

    - You can copy work between P4 servers to implement various collaboration
      workflows.

    - You can rewrite and revise history to discard unwanted
      intermediate information.

    Use the 'p4 init' command to create a new personal repository,
    or use the 'p4 clone' command to create one with files from an
    existing server.  Use the 'p4 switch' command to create new
    streams, and to switch back and forth between existing streams.

    Use the 'p4 fetch' and 'p4 push' commands to copy work back and
    forth between servers, either peer-to-peer or in a hub-and-spoke
    or hierarchical arrangement. The work that is copied is typically
    one or more submitted changelists, but can also be a shelved change.
    Pushing a shelved change can accomodate various code review workflows.

    Use the 'p4 remote' and 'p4 remotes' commands to manage connections
    to multiple partner repositories.

    Use the 'p4 unsubmit' and 'p4 resubmit' commands to rewrite and
    revise history prior to pushing that work to other servers.

    The new fetch, push, and unsubmit functionality is disabled by
    default. To enable the new functionality, set the 'server.allowpush',
    'server.allowfetch', and/or 'server.allowrewrite' configurables
    to enable the desired level of operation.

    The 'p4 zip' and 'p4 unzip' commands enable the equivalent of fetching
    and pushing files in scenarios where a simple authenticated network
    connection between the two servers cannot be established.

    Files of type +l can be cooperatively locked across servers. The
    shared server must be configured as a Commit Server, and must also
    set server.allowremotelocking=1. Then individual personal servers
    can use the --remote flag on 'p4 edit' to manage exclusive file locks
    on the Commit Server; these locks are released when the files are
    pushed. See 'p4 help edit', 'p4 help push', and 'p4 help unlock'.
)"
};

ErrorId MsgHelp::HelpSwitch  = { ErrorOf( ES_HELP, 183, E_INFO, EV_NONE, 0 ),
R"(
    switch -- Switch to a different stream, or create a new stream.

    p4 switch [-c -m -v -P parent] [-Rx] [-r] [ --no-sync ]
              [ --allow-unrelated ] stream
    p4 switch [-r -v] [-Rx] [ --no-sync ] [ --allow-unrelated ] [stream]@change
    p4 switch -l -L
    p4 switch

	The switch command switches to a different stream, optionally
	creating and populating that stream.

	The second form of the command is intended for reproducing
	back-in-time view and revision synchronization. To use this form,
	specify a change number and, optionally, a stream. The client will
	be switched to the stream specification that was available at the
	time of that change; also the workspace will be synced to that
	changelist.  If the stream is omitted but '@change' is specified,
	no stream switch is performed, but the workspace is synced to that
	change.

	The --allow-unrelated flag allows switching to a different stream
	hierarchy.

	The -c flag specifies that the new stream should be created and
	populated with a copy of all the files in the current stream.
	Local (dvcs) server only command.

	The -m flag specifies that 'switch -c' should create a new stream
	with no parent (an independent mainline). The new stream will be
	initially empty.
	Local (dvcs) server only command.

	The -P flag specifies that 'switch -c' should create a new stream
	with the specified stream as its parent, and thus the new stream
	will be populated with the files from the specified stream, rather
	than the files from the current stream.
	Local (dvcs) server only command.

	The -l flag specifies that all known streams should be listed.

	The -L flag specifies that all streams that were switched @change
	with files open in that stream should be listed.

	The -r flag reopens files in the new mapped location of the specified
	stream. If you omit the -r flag, and you have opened files in the
	current stream, those files are shelved before switching to the new
	stream, and are unshelved subsequently when you switch back to this
	stream.

	The -R option changes the way 'reconcile' is run when switching
	between streams:

	        -Ra     reconcile files not currently under P4 control,
	                (these files are opened for add).
	        -Re     reconcile files that have been modified,
	                (these files are opened for edit).
	        -Rd     reconcile files that have been removed,
	                (these files are opened for delete).
	        -Rn     reconcile does not run.

	        Without this option, 'reconcile' runs as if all (-Raed)
	        options were set.

	The --no-sync flag prevents sync of file content for unopened files.

	The -v flag gives verbose information.

	With no arguments, the switch command displays the current stream.

	When switching to a different stream, the switch command first runs
	'p4 reconcile' to determine which files have been modified in the
	current stream.

	You cannot switch to a new stream if files are open in a numbered
	changelist. If files are open in the default changelist, they will
	be shelved and reverted prior to switching to the new stream, and
	will be automatically unshelved when switching back to this stream.

	After switching to the new stream, switch syncs your workspace to
	the head of the new stream, and unshelves any files which were open
	in the default changelist the last time you used that stream.

	See 'p4 help-graph switch' for information on using this command with
	graph depots.
)"
};

ErrorId MsgHelp::HelpInit  = { ErrorOf( ES_HELP, 184, E_INFO, EV_NONE, 0 ),
R"(
    init -- Initialize a new P4 repository.

	p4 init is a client side command.  For detailed help run
	p4 init -h
)"
};

ErrorId MsgHelp::HelpClone = { ErrorOf( ES_HELP, 188, E_INFO, EV_NONE, 0 ),
R"(
    clone -- Clone a new P4 repository from an existing one.

	p4 clone is a client side command.  For detailed help run
	p4 clone -h
)"
};

ErrorId MsgHelp::HelpResubmit = { ErrorOf( ES_HELP, 185, E_INFO, EV_NONE, 0 ),
R"(
    resubmit -- resolve and resubmit some or all unsubmitted changes

	p4 resubmit -l
	p4 resubmit [-R] -m
	p4 resubmit [-R] -e
	p4 resubmit [-R] -i [ [-r remote] file ...]

	The resubmit command resubmits changes which have been unsubmitted.

	The resubmit command is typically used in one of two scenarios:
	- to resolve conflicting changes which were detected and
	  unsubmitted by running 'p4 fetch -t'.
	- to revise a set of changelists that you have submitted locally,
	  but have not pushed to any other server, and have unsubmitted.

	If the -l flag is specified, resubmit lists all the unsubmitted
	changes but takes no action. This is useful as a way to preview
	the work that must be resubmitted.

	There are three modes in which resubmit may run:
	- automatic
	- partially interactive
	- fully interactive

	If the -m flag is specified, resubmit runs in automatic mode. The
	first thing resubmit -m does is to sync your workspace to #head.
	Then, resubmit -m processes each conflicting change, doing the
	following for each change:
	- sync
	- integrate tangent/...@=change dest/...
	- resolve -am
	- submit

	If, for any change in the list, the 'resolve -am' processing detects
	merge conflicts in any file in that change, the resubmit command
	terminates at that point. All the files in that change which had
	merge conflicts are left unresolved. You must then run the resolve
	command to resolve the conflicts. Then run 'resubmit -R -m' to
	resume the resubmit process.

	For all three resubmit modes, you can use the -R flag to resume the
	resubmit process once conflicts have been resolved. This flag causes
	resubmit to begin by submitting that fully-resolved change, after
	which it will proceed to the next unsubmitted change.

	The -e flag runs resubmit in partially-interactive mode.
	The -e flag allows you to resolve each change, build and run tests,
	examine differences, etc., prior to submitting it,
	while the -i flag is a fully-interactive resubmission tool.

	If the -e flag is specified, resubmit prepares the first (oldest)
	conflicting change, then exits. The files for the change are open
	in your workspace, but not resolved. You must then resolve the
	merges, reopen and modify already-resolved merges, or make any other
	necessary changes. You can additionally build and test the changes,
	inspect them, etc. When you are satisfied with this change, run
	'resubmit -R -e' to submit it and prepare the next conflicting change.

	If the -i flag is specified, resubmit runs in fully interactive mode.

	If a file path is also specified, interactive resubmit first
	unsubmits each change that modified a file in that path. If the -r
	flag is additionally specified, it names a remote spec. The mapping
	in the remote spec is then used to limit the files affected by the
	unsubmit operation. Thus a command such as 'p4 resubmit -r rmt @>=17'
	will affect only the files specified by the remote spec, not all
	files in the depot.

	Resubmit -i then processes each conflicting change. For each change,
	resubmit displays summary information about the change, then
	offers the following prompts:

	c		Modify the change description for this change
	m		Merge this change, then submit if no conflicts
	e		Merge this change, then exit for further editing
	r		Interactively resolve this change, then
	                submit if no conflicts
	a		Add (squash) this change into the next conflicting
	                change
	s		Skip this change and move on to the next
	d		Delete this change without submitting it
	b		Begin again from the earliest remaining change
	l		List the changes remaining to be processed
	v		View the current change in short form
	V		View the current change with full diffs
	R		Display the status of resolved and unresolved merges
	q		Quit the resubmit operation
	?		Display (short) help during the resubmit command.

	After choosing either 'e' or 'q', you can subsequently continue
	interactive resubmit by running 'resubmit -R -i'.

	The interactive resubmit command is very flexible: you can reorder
	changes, combine changes, discard changes, etc.

	When multiple changes are resubmitted by the resubmit command, the
	command will automatically switch to the correct stream at the start
	of resubmitting each change. This means that, when the command
	completes, your current stream may have been switched. You should
	then run 'p4 switch' and 'p4 switch -l' if necessary, and switch
	back to the desired stream before resuming your work.

	The SubmitOptions flags in the client spec have no effect on the
	changelists that are submitted by 'p4 resubmit'; this command always
	operates in 'submitunchanged' mode.

	The resubmit command is not available on an Edge Server.

	'p4 rebase' is an alias for 'p4 resubmit'.

)"
};

ErrorId MsgHelp::HelpResubmitShort = { ErrorOf( ES_HELP, 186, E_INFO, EV_NONE, 0 ),
R"(    The following actions are available:
	c		Modify the change description for this change
	m		Merge this change, then submit if no conflicts
	e		Merge this change, then exit for further editing
	r		Interactively resolve this change, then
	                submit if no conflicts
	a		Add (squash) this change into the next unsubmitted
	                change
	s		Skip this change and move on to the next
	d		Delete this change without submitting it
	b		Begin again from the earliest remaining change
	l		List the changes remaining to be processed
	v		View the current change in short form
	V		View the current change with full diffs
	R		Display the status of resolved and unresolved merges
	q		Quit the resubmit operation
	?		Display this help.
)"
};

ErrorId MsgHelp::HelpAliases = { ErrorOf( ES_HELP, 192, E_INFO, EV_NONE, 0 ),
R"(
    aliases -- Display the list of aliases from the P4ALIASES file.

    p4 aliases

	The aliases file can be used to define command line aliases.

	When using the p4 command line tool, each command you enter is
	transformed by any alias definitions in your aliases file prior
	to being sent to the server for execution.

	Typical uses of aliases include: abbreviating complex commands;
	displaying command results in alternate output formats; automating
	simple multi-command sequences; and providing alternate syntax
	for commands you find hard to remember.

	The name of the aliases file is ~/.p4aliases on non-Windows platforms
	and %%USERPROFILE%%\p4aliases.txt on Windows platforms; this
	can be overridden using the P4ALIASES environment variable.

	Your aliases file can contain as many aliases as you want, in any
	order, specified one per line in the format:

	    alias = transformation

	Your aliases file can also contain comments: if the first non-blank
	character on a line is '#', the entire line is treated as a comment.

	When you issue a command which matches one of your aliases, the
	command is automatically modified by applying the transformation
	before issuing the command.

	Alias matching is case-sensitive even on otherwise case-insensitive
	platforms.

	The transformation can be a simple replacement:

	    commit = submit
	    halt = admin shutdown

	Certain characters in an alias definition have special meaning:
	    =      : separates the alias and its transformation
	    $(var) : denotes a named variable
	    &&     : separates two commands in a multi-command alias
	    >      : directs the output of a command to a variable
	    <      : directs the input of a command from a variable
	    \      : indicates the transformation continues on the next line

	The transformation can thus be more complex, and result in multiple
	commands issued to the server:

	    show-status = monitor show && lockstat && changes -m 1

	The transformation can include parameter substitutions:

	    myChanges = changes -u $(P4USER)

	The parameters can be arguments to the alias:

	    my-recent-changes $(max) = changes -u $(P4USER) -m $(max)

	The special built-in variables $(EQ), $(LT), and $(GT) expand
	to =, <, and >, respectively.

	Complex aliases are possible, and often involve using the output
	of one command as input to another:

	    show-last-change=-F "%%change%%" -ztag changes -m 1 \
	                         -s submitted > $(chg) &&
	                     describe -s $(chg)

	    reopenCL $(cl) = -F "%%depotFile%%" -ztag files @$(EQ)$(cl) \
	                         > $(files) && \
	                     -x - edit < $(files)

)"
};

ErrorId MsgHelp::HelpGraph   = { ErrorOf( ES_HELP, 194, E_INFO, EV_NONE, 0 ),
R"(
    Graph Depot -- Server support for the git data model.

    Versioned files may be managed using either of two data models:

    - The P4 data model tracks individual file history with a rich
      set of information about each file's evolution, recording each
      changelist and the files that it modified.

    - The git data model tracks the history of entire repositories. The
      history is represented in a far more compact form, but less
      information is available about each individual file.

    The data model in use for a particular set of files is specified on
    a depot by depot basis. To use the git data model, use a depot of
    type graph; all other depot types use the P4 data model.

    Information about Graph Depot commands is available by using:

	p4 help-graph

    for convenience you may wish to add the line

	ghelp = help-graph

    to your P4ALIASES file. For more information, try:

	p4 help-graph commands        list standard commands
	p4 help-graph command         help on a specific command
	p4 help-graph administration  help on administration
)"
};

ErrorId MsgHelp::HelpGraphCommands = { ErrorOf( ES_HELP, 204, E_INFO, EV_NONE, 0 ),
R"(
    P4 Graph Depot user commands:

	add            Open a new file to add it to the repo
	client         Create or edit a client specification and its view
	describe       Display a commit description
	delete         Open an existing file to delete it from the repo
	diff           Diff utility for comparing workspace to repo content
	diff2          Diff utility for comparing the content at two repo paths
	dirs           List repo subdirectories
	edit           Open an existing file for edit
	filelog        List commit history of file
	files          List the files in the repo
	fstat          List file info
	graph gc       Delete records of orphaned objects from the object table
	graph log      List commits
	graph purge-refhist Delete refhist records for a repo
	graph rebase   Replay local history onto the target's new base
	graph recompute-refcnts Recompute object refcounts
	graph show-ref Display reference values
	graph tag      Tag a commit with a name
	graph tags     List tagged commits in all repos
	graph lfs-lock Create a git lfs-lock
	graph lfs-unlock Remove a git lfs-lock
	graph lfs-locks List git lfs-locks in repo
	graph verify   Verify repo integrity
	have           Display the most recently synced commit
	lock           Lock an opened file to prevent it from being submitted
	merge          Merge another branch into current/target branch
	opened         Display list of files opened for pending changelist
	print          Write a repo file to standard output
	reconcile      Open files for add, delete, and/or edit to reconcile
	               client with workspace changes made outside of P4
	resolve        Resolve updates to open workspace files
	revert         Discard changes from an opened file
	submit         Commit open files to the repo
	switch         Switch to a different branch
	sync           Synchronize the client with its view of the repo
	undo           Revert the changes of one or more commits
	unlock         Release a locked file, leaving it open

    See also 'p4 help-graph administration'
)"
};

ErrorId MsgHelp::HelpGraphAdministration = { ErrorOf( ES_HELP, 205, E_INFO, EV_NONE, 0 ),
R"(
    P4 Graph Depot administrative commands:

      repo              Create, modify, or delete a repo specification
      repos             Display list of repo specifications
      grant-permission  Grant access permission to a repo/graph depot
      revoke-permission Revoke access permission to a repo/graph depot
      show-permission   Show access permission granted to a repo/graph depot
      show-permissions  Show access permissions granted to a user
      check-permission  Check access permission granted to a repo
      pubkey            Add or update a SSH public key to the P4
                        Server
      pubkeys           Display a list of users SSH public keys

    For help on these topics, issue 'p4 help <topic>'.

    See also 'p4 help-graph commands'

)"
};

ErrorId MsgHelp::HelpGraphAdd = { ErrorOf( ES_HELP, 206, E_INFO, EV_NONE, 0 ),
R"(
    add -- Open a new file to add it to the repo.

    p4 add [-c changelist# -n -I -f -t filetype] file ...

	Open a file for adding to the depot.

	To associate the open files with a specific pending changelist, use
	the -c flag; if you omit the -c flag, the open files are associated
	with the default changelist.

	To specify file type, use the -t flag.  By default, 'p4 add'
	assumes the file is a simple text file. The following alternate
	filetypes are available:

	    text+x   executable file, such as a shell script
	    symlink  symbolic link
	    binary+F large binary file, to be stored using git-lfs

	To add files with filenames that contain wildcard characters, specify
	the -f flag. Filenames that contain the special characters '@', '#',
	'%' or '*' are stored unaltered. However to list these
	filenames using p4 commands (files, print, filelog, fstat), you must
	encode the characters using their ASCII hexadecimal representation.

	The -I flag informs the client that it should not perform any ignore
	checking configured by P4IGNORE.

	The -n flag displays a preview of the specified add operation without
	changing any files or metadata.
)"
};

ErrorId MsgHelp::HelpGraphClient = { ErrorOf( ES_HELP, 207, E_INFO, EV_NONE, 0 ),
R"(
    client -- Create or edit a client specification and its view.
    (for flag descriptions, see 'p4 help client').

    p4 client [-f] [-t template] -T graph [name]
    p4 client -o [-t template] -T graph [name]

	To modify Graph Depot files using p4 commands, your workspace
	must obey several additional rules, beyond those described in:
	'p4 help client':

	Specify 'Type: graph'.

	Specify 'View: map', where the map describes the files in the repos
	that are to be used by this client.

	Clients may contain Views into both classic and graph depots, but may
	update paths of only one type or the other. If the "type" is the
	default (unset) , updates to Graph Depot paths are prohibited.
	Using '-T graph' configures the client 'type' to 'graph', permitting
	updates to Graph Depot paths and prohibiting updates to any non-Graph
	Depot paths.

)"
};

ErrorId MsgHelp::HelpGraphCherryPick = { ErrorOf( ES_HELP, 236, E_INFO, EV_NONE, 0 ),
R"(
    graph cherry-pick -- Replay one commit onto another.

    p4 graph cherry-pick [-r ref -f --undo] --repo repo --target target source

	This command will replay the diffs from the source commit onto the
	target commit.  If there are conflicts detected when replaying the
	diffs, the cherry-pick will fail."

	The --repo flag specifies the repo.

	The --target flag specifies the commit on which to replay the source
	commit.

	The -f option is required if the --target is the repo default branch.

	The --undo flag reverts the changes introduced by the source.

	By default the target branch reference will be updated to the replayed
	commit.  The -r flag allows an alternative reference to be updated
	instead, allowing the result to be tested before the target branch
	is updated for real.
)"
};

ErrorId MsgHelp::HelpGraphDelete = { ErrorOf( ES_HELP, 208, E_INFO, EV_NONE, 0 ),
R"(
    delete -- Open a repo file for deletion.

    p4 delete [-c changelist# -n -f] file ...

	Open a repo file for deletion.

	If the file is synced in the client workspace, it is removed.  If a
	pending changelist number is specified using with the -c flag, the
	file is opened for delete in that changelist. Otherwise, it is opened
	in the default pending changelist.

	The -n flag displays a preview of the operation without changing any
	files or metadata.

	To delete filenames containing wildcard characters, specify
	the -f flag.
)"
};

ErrorId MsgHelp::HelpGraphFiles = { ErrorOf( ES_HELP, 227, E_INFO, EV_NONE, 0 ),
R"(
    files -- List the files in the repo

    p4 files [ -m max ] file ...

	List files in the repo.

	If client syntax is used to specify the file argument, the
	client view mapping is used to determine the corresponding
	repo files.

	The -m flag limits files to the first 'max' number of files.
)"
};

ErrorId MsgHelp::HelpGraphFstat = { ErrorOf( ES_HELP, 228, E_INFO, EV_NONE, 0 ),
R"(
    fstat -- List file info.

    p4 fstat [-F filter -T fields -m max] file ...

	Fstat lists information about files, one line per field.  Fstat is
	intended for use in P4 API applications, where the output can be
	accessed as variables, but its output is also suitable for parsing
	from the client command output in scripts.

	The fields that fstat displays are:

		depotFile            -- name in depot
		clientFile           -- local path (host or P4 syntax)
		isMapped             -- set if file is mapped in the client
		headCommit           -- the commit for the file at head rev
		headBlob             -- the sha for the file at head rev
		headType             -- head rev type, if in depot
		haveBlob             -- the sha for the have rev on client
		haveCommit           -- the commit for the have rev on client
		otherOpen#           -- list of user@client with file opened
		otherAction#         -- open action, if opened by someone else
		otherChange#         -- changelist, if opened by someone else
		otherLock#           -- user@client with file locked
		otherOpen            -- set if someone else has it open
		otherLock            -- set if someone else has it locked
		repo                 -- the name of the repo (.git suffix)
		repoName             -- the name of the repo

	If the file is opened in the current workspace, additional fields
	that fstat displays are:

		action               -- open action, if opened
		change               -- open changelist#, if opened
		type                 -- open type, if opened
		actionOwner          -- user who opened file, if opened
		workBlob             -- sha of open revision, if opened
		ourLock              -- set if this user/client has it locked

	The -F flag lists only files satisfying the filter expression. This
	filter syntax is similar to the one used for 'jobs -e jobview' and is
	used to evaluate the contents of the fields in the preceding list.
	Filtering is case-sensitive.

		Example: -F "headType=binary & repoName=//repo/git"

	Note: filtering is not optimized with indexes for performance.

	The -T fields flag returns only the specified fields. The field names
	can be specified using a comma- or space-delimited list.

		Example: -T "depotFile, clientFile, headCommit"

	The -m max flag limits output to the specified number of files.
)"
};

ErrorId MsgHelp::HelpGraphDescribe = { ErrorOf( ES_HELP, 209, E_INFO, EV_NONE, 0 ),
R"(
    describe -- Display a commit description

    p4 describe -n //repo/name [-a -d<flags> -s] sha

	Display information about the specified commit.

	Specify the repo name with the -n flag.

	sha is the corresponding sha1 of the commit.

	The -a flag will display the content of added files in addition
	to diff of updated files.

	The -d<flags> passes one or more flags to the built-in diff routine
	to modify the output: -dn (RCS), -dc[n] (context), -ds (summary),
	-du[n] (unified), -db (ignore whitespace changes), -dw (ignore
	whitespace), -dl (ignore line endings). The optional argument to
	to -dc/-du specifies number of context lines.

	The -s flag omits the diffs of files that were updated.
)"
};

ErrorId MsgHelp::HelpGraphDiff = { ErrorOf( ES_HELP, 210, E_INFO, EV_NONE, 0 ),
R"(
    diff -- Diff utility for comparing workspace to repo content

    (For comparing two repo paths, see p4 diff2).

    p4 diff [-d<flags> -s<flag> file ...]

	On the client machine, diff a client file against the corresponding
	revision in the repo. The file is compared only if the file is
	opened for edit.

	If the file specification is omitted, all open files are diffed.

	The -d<flags> modify the output as follows:
	    -dn (RCS),
	    -dc[n] (context),
	    -ds (summary),
	    -du[n] (unified),
	    -db (ignore whitespace changes),
	    -dw (ignore whitespace),
	    -dl (ignore line endings).
	The optional argument to -dc/-du specifies number of context lines.

	The -s options lists the files that satisfy the following criteria:

		-sa     Opened files that differ from the revision
			in the depot or are missing.

		-sd     Unopened files that are missing on the client.

		-se     Unopened files that differ from the revision
			in the depot.

		-sl	Every unopened file, along with the status of
			'same, 'diff', or 'missing' as compared to the
			corresponding revision in the depot.

		-sr     Opened files that do not differ from the revision in
			the depot.

)"
};

ErrorId MsgHelp::HelpGraphDiff2 = { ErrorOf( ES_HELP, 249, E_INFO, EV_NONE, 0 ),
R"(
    diff2 -- Diff utility for comparing the content at two repo paths.
    (For comparing workspace content to repo content, see p4 diff.)

    p4 diff2 [options] fromFile[@sha1|@reference] toFile[@sha1|@reference]

    	options: -d<flags> -Od -q -u

	'p4 diff2' runs on the server to compare one set of graph repo files
	(the 'source') to another (the 'target'). Source and target file sets
	can be specified on the 'p4 diff2' command line.

	fromFile and toFile can include @sha1 or @reference specifiers; by
	default, the head revisions are diffed.

	'p4 diff2' precedes each diffed file pair with a header line of the
	following form:

	    ==== source#rev (type) - target#rev (type) ==== summary

	A source or target file shown as '<none>' means there is no file
	at the specified name or reference to pair with its counterpart.
	The summary status is one of the following: 'identical' means file
	contents and types are identical, 'types' means file contents are
	identical but the types are different, and 'content' means file
	contents are different.

	The -d<flags> modify the output of diffs as follows:

		-dn (RCS)
		-dc[n] (context)
		-ds (summary)
		-du[n] (unified)
		-db (ignore whitespace changes)
		-dw (ignore whitespace)
		-dl (ignore line endings).

	The optional argument to -dc/-du specifies number of context lines.

	The -Od flag limits output to files that differ.

	The -q omits files that have identical content and types and
	suppresses the actual diff for all files.

	The -u flag uses the GNU diff -u format and displays only files that
	differ. The file names and dates are in P4 syntax, but the output can
	be used by the patch program.

)"
};

ErrorId MsgHelp::HelpGraphEdit = { ErrorOf( ES_HELP, 211, E_INFO, EV_NONE, 0 ),
R"(
    edit -- Open an existing file for edit.

    p4 edit [-c changelist# -n -f ] file ...

	Open an existing file for edit.  The server records the fact that
	the current user has opened the file in the current workspace, and
	changes the file permission from read-only to read/write.

	If -c changelist# is included, the file opened in the specified
	pending changelist.  If changelist number is omitted, the file is
	opened in the 'default' changelist.

	The -n flag previews the operation without changing any files or
	metadata.

	To edit filenames containing wildcard characters, specify
	the -f flag.
)"
};

ErrorId MsgHelp::HelpGraphFilelog = { ErrorOf( ES_HELP, 212, E_INFO, EV_NONE, 0 ),
R"(
    filelog -- List commit history of file

    p4 filelog [ -d -m max --first-parent ] depotfile

	List the commit history of the specified file, from the most
	recent commit to the first.

	By default, 'p4 filelog' is optimized not to display history of
	a file which is deleted in the most recent commit, or has been
	removed previously. To view the full history of a deleted file,
	specify the -d flag.

	The -d flag displays the full history of a deleted file.
	This option may not be used with paths containing wildcards.

	The -m max displays at most 'max' commits.

	The --first-parent flag follows only the first parent of each commit.
)"
};

ErrorId MsgHelp::HelpGraphHave = { ErrorOf( ES_HELP, 213, E_INFO, EV_NONE, 0 ),
R"(
    have -- Display the most recently synced commit

    p4 have [--graph-only] [repo-path ...]

	Lists the current branch, and the currently-synced commit, for the
	selected repos in this client workspace.

	For hybrid workspaces that have both classic and graph depots, the
	--graph-only flag limits the report to graph depots only. Without
	it 'p4 have' will report on both graph and classic depots.
)"
};

ErrorId MsgHelp::HelpGraphMerge = { ErrorOf( ES_HELP, 214, E_INFO, EV_NONE, 0 ),
R"(
    merge -- Merge another branch into current/target branch.

    p4 merge [Options] source-branch
    p4 merge [options] --repo=//repo/name --target=branch source-branch1
	                [source-branch2 ...]
    p4 merge [options] --repo=//repo/name:target:source1[:source2...]

	Options: [-n --no-ff --ff-only --squash -d description]

	'p4 merge' merges commits from the source branch into this branch,
	typically creating a merge commit as the result. In the interactive
	mode a 'graph' client is required which maps only a single repo, the
	target branch being the one that the client is currently synced to.

	The --repo flag specifies that the merge is a non-interactive
	clientless merge that will be performed entirely on the server.
	(requires a --target branch).

	The --target flag specifies the target branch to merge to, this
	is used when using the non-interactive version of the command
	specified by --repo.

	The second form of the command allows an atomic non-interactive
	clientless merge that will be performed entirely on the server
	involving multiple repos by specifying the --repo flag repeatedly
	with the single target branch and one or more source branches
	separated with a ':' character.

	The --no-ff flag instructs the command to create a merge commit
	even if it resolves as a fast-forward.

	The --ff-only flag refuses to merge unless the current branch
	head is already up-to-date or the merge can be resolved as a
	fast-forward.

	The --squash flag instructs merge to perform a merge without a
	merge commit (single parent).

	The -n flag previews the operation without updating the repo.

	The -d flag provides the merge commit description.
	The default description is 'Merge into <target> <source>'.
)"
};

ErrorId MsgHelp::HelpGraphUndo = { ErrorOf( ES_HELP, 252, E_INFO, EV_NONE, 0 ),
R"(
    undo -- Revert the changes of one or more commits for one or more repos.

    p4 undo [Options] --repo=//repo/name:target:source0[:sourcen ...]...

	Options: [-n -d description]

	Given one or more existing commits, revert the changes in these commits
	creating a new commit for each. Reverted merge commits will be
	diffed against parent one.

	This command performs an atomic non-interactive
	clientless undo entirely on the server
	involving multiple repos by specifying the --repo flag repeatedly
	with the single target branch and one or more source branches
	separated with a ':' character.

	The --repo flag specifies the graph repo.

	The -n flag previews the operation without updating the repo.

	The -d flag provides the merge commit description.
	The default description is 'Revert "<original message>"'.
)"
};

ErrorId MsgHelp::HelpGraphOpened = { ErrorOf( ES_HELP, 233, E_INFO, EV_NONE, 0 ),
R"(
    opened -- Display list of files opened for pending changelist.

    p4 opened [-a -c changelist# -C client -u user -m max -s ] [file ...]
    p4 opened [-a -m max ] [file ...]

	Lists files currently opened in pending changelists, or, for
	specified files, show whether they are currently opened or locked.
	If the file specification is omitted, all files open in the current
	client workspace are listed.

	By default, files opened by the current user in the current client
	workspace are listed.  The -a, -C client, and -u user flags
	alter this behavior. The -a flag lists opened files in all clients.
	The -C client flag lists files open in the specified client
	workspace. The -u user flag lists files opened by the specified
	user. The -C and -u flags may be combined; -a is ignored if either
	of the -C or -u flags is used.

	The -c changelist# flag lists files opened in the specified
	changelist#.

	The -m max flag limits output to the first 'max' number of files.

	The -s option produces 'short' and optimized output when used with
	the -a (all clients) option.  For large repositories '-a' can take
	a long time when compared to '-as'.

)"
};

ErrorId MsgHelp::HelpGraphRebase = { ErrorOf( ES_HELP, 237, E_INFO, EV_NONE, 0 ),
R"(
    graph rebase -- Replay local history onto the target's new base.

    p4 graph rebase [-r ref ] [-f] --repo repo --target target source

	Rebase effectively moves the base of the target branch from
	its current location in the source to the source's HEAD.

	Rebase determines the common base between the target
	and source, saving the local commits of the target branch committed
	after the base.  It then resets the target branch HEAD to the source
	branch HEAD.  Rebase then applies the saved local changes as new
	commits on the target.

	The target branch reference is updated to the last replayed commit.
	Rebase requires 'force-push' permissions.
	NOTE: The source branch is not altered.

	If there are conflicts detected when replaying the diffs, the rebase
	will fail, and any intermediary commits will be orphaned.
	If the common base cannot be found, the rebase will fail.

	The --repo flag specifies the repo.

	The --target flag specifies the branch reference whose history
	would be changed by rebasing its local changes onto its new base.

	The -r flag allows an alternative reference to be
	updated which can be tested without having updated the target.
	If the ref exists, 'force-push' permissions are required.
	If the ref does not exist, 'create-ref' permissions are required.

	The -f option is required if the --target is the repo default branch.

)"
};

ErrorId MsgHelp::HelpGraphSubmit = { ErrorOf( ES_HELP, 215, E_INFO, EV_NONE, 0 ),
R"(
    submit -- Commit open files to the repo.

    p4 submit [-i -c chg -d desc --allow-empty]

	'p4 submit' commits a pending changelist and its files to the repo,
	constructing a new commit and updating the current branch to refer
	to the new commit.

	By default, 'p4 submit' attempts to submit all files in the 'default'
	changelist.  Submit displays a dialog where you enter a description
	of the change and, optionally, delete files from the list of files
	to be checked in.

	To add files to a changelist before submitting, use any of the
	commands that open client workspace files: 'p4 add', 'p4 edit',
	etc.

	The -c flag submits the specified pending changelist instead of the
	default changelist. Additional changelists can be created manually,
	using the 'p4 change' command, or automatically as the result of a
	failed attempt to submit the default changelist. The pending
	changelist is deleted during the submit process; the result of the
	submit is a new commit identified by its SHA-1.

	The -d flag passes a description into the specified changelist rather
	than displaying the changelist dialog for manual editing. This option
	is useful for scripting, but does not allow you to modify
	the default changelist.

	The -i flag reads a changelist specification from the standard input.
	The user's editor is not invoked.

	The --allow-empty flag permits submitting with no file changes.
)"
};

ErrorId MsgHelp::HelpGraphSync = { ErrorOf( ES_HELP, 216, E_INFO, EV_NONE, 0 ),
R"(
    sync -- Synchronize the client with its view of the repo

    p4 sync [-f -n -q -k] [file[commit-sha1|reference] ...]

	Sync updates the client workspace to reflect the current contents
	of the repo (if it has changed).

	If a filepath is specified, which matches paths that exist in one or
	more repos mapped to the client, those repos will be synced.
	Note: The entire portion of the repo that is mapped to the client will
	be synced, not just the files in the specified filepath.  If a sha or
	reference is specified, the client will be syned to that commit.
	If a branch reference is specified, the branch the client is currently
	associated with will also be updated.  If the client has open files in
	the specified repo, syncing will only be permitted if the commit that
	is to be syned is a child of the current commit (a fast-forward).

	Sync adds files that have not been retrieved before, deletes
	previously retrieved files that have been deleted from the repo, and
	updates files that have been updated in the repo.

	Normally, sync does not overwrite workspace files that the user has
	manually made writable.  Setting the 'clobber' option in the
	client specification disables this safety check.

	The -f flag forces resynchronization even if the client already
	has the file, and overwriting any writable files.  This flag doesn't
	affect open files.

	The -n flag previews the operation without updating the workspace.

	The -q flag suppresses normal output messages. Messages regarding
	errors or exceptional conditions are not suppressed.

	The -k flag updates server metadata without syncing files. It is
	intended to enable you to ensure that the server correctly reflects
	the state of files in the workspace while avoiding a large data
	transfer. Caution: an erroneous update can cause the server to
	incorrectly reflect the state of the workspace.
)"
};

ErrorId MsgHelp::HelpGraphTag = { ErrorOf( ES_HELP, 217, E_INFO, EV_NONE, 0 ),
R"(
    graph tag -- Tag a commit with a name.

    p4 graph tag -n //repo/name [-c comment] [-f] tag sha
    p4 graph tag -n //repo/name -d tag
    p4 graph tag [-n //repo/name] -l [-o] [-m max]

	The graph tag command may be used to create, update, delete,
	or list tags.

	Specify the repo name with the -n flag.

	Use the first form of the command to create a new or update an
	existing tag. The -f flag is required to update an existing tag.
	The -c comment flag is used to add a comment to the tag.

	Use the second form of the command (with -d) to delete a tag.

	Use the third form of the command (with -l) to list the tags.
	If repo-name is not entered, it will list tags in all repos.

	Use the -o flag to list in detailed format.

	The -m max displays at most 'max' tags.
)"
};

ErrorId MsgHelp::HelpGraphTags = { ErrorOf( ES_HELP, 242, E_INFO, EV_NONE, 0 ),
R"(
    graph tags -- Lists tagged commits in all repos.

    p4 graph tags [-o] [-m max]

	The graph tags command may be used to list tags in all repos.

	Use the -o flag to list in detailed format.

	The -m max displays at most 'max' tags.
)"
};

ErrorId MsgHelp::HelpGraphSwitch  = { ErrorOf( ES_HELP, 218, E_INFO, EV_NONE, 0 ),
R"(
    switch -- Switch to a different branch, create a branch.

    p4 switch [-a | -n repo] branch
    p4 switch -c [-a | -n repo] branch
    p4 switch -d [-a | -f | -n repo] branch
    p4 switch -l

	The switch command moves the client to a different branch, optionally
	creating that branch.

	Use the first form of the command to switch the current branch for
	your client.

	The -a flag applies for clients having more than one repo. For
	switch, it specifies that all repos containing this branch should
	switch to it. Repos that do not will remain on their current
	branch. For create (-c), it specify that the branch should
	be created on all repos of the client. And finally, for delete (-d),
	it specifies to delete on all repos that has it.

	The -n flag applies for clients having more than one repo. This
	specifies which specific repo should switch to this branch (for
	switch) or should delete (with -d) or should create (with -c) and
	switch to the branch.

	The -c flag creates the specified branch and switch the client to it.

	The -d flag deletes the specified branch.

	The -f flag forces the deletion of a branch. This flag is required
	when a branch was not merged to master.

	The -l flag to list the branches.
)"
};

ErrorId MsgHelp::HelpGraphReconcile = { ErrorOf( ES_HELP, 226, E_INFO, EV_NONE, 0 ),
R"(
    reconcile -- Open files for add, delete, and/or edit to reconcile
                 client with workspace changes made outside of P4


    p4 reconcile [-e -a -d -n] [file ...]

	'p4 reconcile' finds unopened files in a client's workspace and
	detects the following:

	1. files in depot missing from workspace, but still on have list
	2. files on workspace that are not in depot
	3. files modified in workspace that are not opened for edit

	By default, the files matching each condition above in the path
	are reconciled by opening files for delete (scenario 1), add
	(scenario 2), and/or edit (scenario 3). The -e, -a, and -d flags
	may be used to limit to a subset of these operations.

	The -n flag previews the operation without performing any action.
	Although metadata updates from reconcile require open permission,
	the preview commands only require read access.

	The -e flag allows the user to reconcile files that have been modified
	outside of P4. The reconcile command will open these files for edit.

	The -a flag allows the user to reconcile files that are in the user's
	directory that are not under P4 control. These files are opened for
	add.

	The -d flag allows the user to reconcile files that have been
	removed from the user's directory but are still in the depot.
	These files will be opened for delete only if they are still on the
	user's have list.
)"
};

ErrorId MsgHelp::HelpGraphRevert = { ErrorOf( ES_HELP, 234, E_INFO, EV_NONE, 0 ),
R"(
    revert -- Discard changes from an opened file.

    p4 revert [-a -n -w -c changelist# -f] file ...

	Revert an open file to the revision that was synced from the depot,
	discarding any edits or integrations that have been made.  You must
	explicitly specify the files to be reverted.  Files are removed from
	the changelist in which they are open.  Locked files are unlocked.

	The -a flag reverts only files that are open for edit, add, or
	integrate and are unchanged or missing. Files with pending
	integration records are left open. The file arguments are optional
	when -a is specified.

	The -n flag displays a preview of the operation.

	The -w flag causes files that are open for add to be deleted from the
	workspace when they are reverted.

	The -c flag reverts files that are open in the specified changelist.

	To revert filenames containing wildcard characters, specify
	the -f flag.
)"
};

ErrorId MsgHelp::HelpRepo   = { ErrorOf( ES_HELP, 195, E_INFO, EV_NONE, 0 ),
R"(
    repo -- Create, modify, or delete a repo specification

    p4 repo [-f] repo
    p4 repo -d [-f] repo
    p4 repo -o repo
    p4 repo -i [-f]

	A repo specification describes the high-level configuration
	and usage of a repo.

	The 'repo' command puts the repo spec into a temporary file and
	invokes the editor configured by the environment variable $P4EDITOR.
	Saving the file creates or modifies the repo spec.

	The repo spec contains the following fields:

	Name:        The name of the repo

	Owner:       The user who created this repo spec. Can be changed.

	Created:     The date this repo spec was created.

	Pushed:      The last time this repo was pushed to.

	Description: A short description of the repo (optional).

	MirroredFrom: Upstream URL this read-only repo is mirrored from.

	DefaultBranch: The branch to include when cloning the repo.
           It must start with "refs/" if you want git to use it; eg,
           "refs/heads/trunk" rather than just "trunk".

	The -d flag deletes the named repo spec and files it contains.

	The -o flag writes the repo spec to standard output. The user's
	editor is not invoked.

	The -i flag causes a repo spec to be read from the standard input.
	The user's editor is not invoked.

	The -f flag enables a user with 'admin' privilege to delete the repo.
	By default, repos can be deleted only by their owner. The owner of a
	repo or a user with 'super' privilege can update the 'Owner' field
	of a repo using -f.

)"
};

ErrorId MsgHelp::HelpRepos  = { ErrorOf( ES_HELP, 196, E_INFO, EV_NONE, 0 ),
R"(
    repos -- Display list of repo specifications

    p4 repos [[-e|-E] nameFilter] [-m max] [-u user] [-O owner]
             [--from=origin]

	Lists repo specifications. (See 'p4 help repo'.)

	The -e nameFilter flag lists repo specs with a name that matches
	the nameFilter pattern, for example: -e '//linux/kernel/...'.

	The -e flag uses the server's normal case-sensitivity rules.

	The -E flag makes the matching case-insensitive, even on a
	case-sensitive server.

	The -m max flag limits output to the specified number of repo specs.

	The -O owner flag limits output to repos owned by the specified user
	or group.

	The -u user flag limits output to those repos that can be read by the
	specified user.

	The --from=origin flag limits output to repos which were forked from
	the specified repo.

)"
};

ErrorId MsgHelp::HelpGraphReceivePack = { ErrorOf( ES_HELP, 219, E_INFO, EV_NONE, 0 ),
R"(
    graph receive-pack -- Import git objects and/or references

    p4 graph receive-pack -n //repo/name [-i pack...] [-r|-F|-p|-P ref...]

	The graph receive-pack command is used by Git Connector to import
	one or more git packfiles and/or one or more git references to the
	specified Graph Depot repo.

	The -n flag specifies the repo name to hold the git data.

	The -i flag specifies the packfile to be imported. Multiple packfiles
	may be specified in a single invocation with multiple -i flags.

	The -r, -F, -p, and -P flags are used to import references. A
	reference may be added, updated, or deleted from the repo. Each -r
	flag specifies a single reference to add, update, or delete. Each -F
	flag specifies a single reference to force-push. The -p flag is used
	to update multiple references by specifying the name of a file in
	packed-refs format. The -P flag is used to update multiple references
	by specifying the name of a file in update-ref format.

)"
};

ErrorId MsgHelp::HelpGraphShowRef = { ErrorOf( ES_HELP, 220, E_INFO, EV_NONE, 0 ),
R"(
    graph show-ref -- Display reference values

    p4 graph show-ref [-n //repo/name] [-a -u user -m max -t ref-type ]
	               [[-e|-E] nameFilter]

	The graph show-ref command displays reference values.

	The -n flag specifies the repo name. If a repo name is specified,
	you have to have read permission for that repo; otherwise you must
	be a super user.

	The -a flag displays all references from all repos.

	The -u flag displays only references readable by the user argument.

	The -m flag specifies the maximum references to display.

	The -t flag specifies the type of reference (tag/branch).

	The -e nameFilter flag lists references with a name that matches
	the nameFilter pattern, for example: -e 'refs/heads/dev...'.

	The -e flag uses the server's normal case-sensitivity rules.

	The -E flag makes the matching case-insensitive, even on a
	case-sensitive server.

)"
};

ErrorId MsgHelp::HelpGraphRefHist = { ErrorOf( ES_HELP, 221, E_INFO, EV_NONE, 0 ),
R"(
    graph ref-hist -- Display reference history

    p4 graph ref-hist [options...]
	-n //repo/name
	-r reference-name
	-u user
	-t ref-type
	-a action
	-s start
	-e end
	-m max

	The graph ref-hist command displays reference history.

	The options are used to refine the results.

)"
};

ErrorId MsgHelp::HelpGraphPackObjects = { ErrorOf( ES_HELP, 222, E_INFO, EV_NONE, 0 ),
R"(
    graph pack-objects -- Export git data to a packfile

    p4 graph pack-objects -n //repo/name -o dir [-a | ref...]

	The graph pack-objects command exports git data into a packfile.

	The -n flag specifies the repo to be exported.

	The -o flag specifies where the packfile is to be written. Note that
	the command only creates a packfile, not an index, so you typically
	need to run git index-pack to index the new packfile.

	The -a flag specifies that the entire repo is to be exported. If the
	-a flag is not specified, instead you may specify the subset of the
	repo to be exported using git reference syntax.

)"
};

ErrorId MsgHelp::HelpGraphSubmodule = { ErrorOf( ES_HELP, 224, E_INFO, EV_NONE, 0 ),
R"(
    graph submodule -- Manage submodule configuration data

    p4 graph submodule [-d] -n //repo/name -p path -t //repo/submodule
    p4 graph submodule -l -n //repo/name

	The graph submodule command is used to specify the submodule
	configuration data for a particular repo.

	The first form of the command is used to add, update, or delete
	a submodule path for a particular repo.

	The second form of the command is used to display submodule
	configuration data for a particular repo.

	The -n flag specifies the repo name

	The -p flag specifies the path at which the submodule is linked

	The -t flag specifies the submodule repo name

	The -d flag specifies to delete a submodule from this repo

	The -l flag specifies to display the current configuration

)"
};

ErrorId MsgHelp::HelpGraphLfsLocks = { ErrorOf( ES_HELP, 257, E_INFO, EV_NONE, 0 ),
R"(
    graph lfs-locks -- List git lfs locks in repo

    p4 graph lfs-locks -n repo [ -r refpath ] [ -p file ]

	The graph lfs-locks lists a repo's git lfs locks.

	The -n flag specifies the repo.

	The -r flag specifies the branch reference.

	The -p flag specifies repo path.
)"
};

ErrorId MsgHelp::HelpGraphLfsLock = { ErrorOf( ES_HELP, 258, E_INFO, EV_NONE, 0 ),
R"(
    graph lfs-lock -- Lock a repo path for git lfs

    p4 graph lfs-lock [-u user ] -n repo -r refpath file

	The graph lfs-lock locks a repo path for git lfs.

	The file path must be a fully qualified repo path or
	a path relative to the root of the repo.

	The -u specifies the lock owner.

	The -n flag specifies the repo.

	The -r flag specifies the branch reference.
)"
};
ErrorId MsgHelp::HelpGraphLfsUnLock = { ErrorOf( ES_HELP, 259, E_INFO, EV_NONE, 0 ),
R"(
    graph lfs-unlock -- Remove a repo's git lfs lock

    p4 graph lfs-unlock [ -f ] [ -u user ] [ -n repo -r ref file ] | [ lockid ]

	The graph lfs-unlock removes a repo's git lfs lock.

	The -f allows removal of a lock owned by another user.

	The -u specifies the owner of the lock to be removed.

	The -n flag specifies the repo.

	The -r flag specifies the branch reference.
)"
};
ErrorId MsgHelp::HelpGraphLock = { ErrorOf( ES_HELP, 223, E_INFO, EV_NONE, 0 ),
R"(
    lock -- Lock an opened file to prevent it from being submitted.

    p4 lock [-c changelist#] [file ...]

	The specified files are locked in the depot, preventing any user
	other than the current user on the current client from submitting
	changes to the files.  If a file is already locked, the lock request
	is rejected.

	The -c flag and the file specification limit the files to be locked.
	If no file names are specified and no changelist is specified, all
	open files are locked.

	If a changelist is specified, only those files open in that
	changelist are locked.

	If a file specification is given, only the matching files are locked.

	If both the -c flag and a file specification are provided, only the
	matching files in the specified changelist are locked.
)"
};

ErrorId MsgHelp::HelpGraphResolve = { ErrorOf( ES_HELP, 244, E_INFO, EV_NONE, 0 ),
R"(
    resolve -- Resolve integrations and updates to repo workspace files

    p4 resolve [options] [file ...]

	options: [-a<flags> -n]

	'p4 resolve' resolves changes to files in the client workspace.

	'p4 resolve' works only on files that have been scheduled to be
	resolved.  The commands that can schedule resolves are: 'p4 sync',
	'p4 update', 'p4 submit', 'p4 merge', and 'p4 integrate'.  Files must
	be resolved before they can be submitted.

	Resolving involves two sets of files, a source and a target.  The
	target is a set of depot files that maps to opened files in the
	client workspace.  When resolving an integration, the source is a
	different set of depot files than the target.  When resolving an
	update, the source is the same set of depot files as the target,
	at a different revision.

	The 'p4 resolve' file argument specifies the target.  If the file
	argument is omitted, all unresolved files are resolved.

	Resolving can modify workspace files.

	The resolve process is a classic three-way merge. The participating
	files are referred to as follows:

	  'yours'       The target file open in the client workspace
	  'theirs'      The source file in the depot
	  'base'        The common ancestor; the highest revision of the
	                source file already accounted for in the target.
	  'merged'      The merged result.

	Filenames, filetypes, and text file content can be resolved by
	accepting 'yours', 'theirs', or 'merged'.  Branching, deletion, and
	binary file content can be resolved by accepting either 'yours' or
	'theirs'.

	When resolving integrated changes, 'p4 resolve' distinguishes among
	four results: entirely yours, entirely theirs, a pure merge, or an
	edited merge.  The distinction is recorded when resolved files are
	submitted, and will be used by future commands to determine whether
	integration is needed.

	In all cases, accepting 'yours' leaves the target file in its current
	state.  The result of accepting 'theirs' is as follows:

	   Content:     The target file content is overwritten.
	   Attribute:   The target's attributes are replaced.
 	   Deletion:    The target file is deleted.
 	   Filename:	The target file is moved or renamed.
 	   Filetype:    The target file's type is changed.

	For each unresolved change, the user is prompted to accept a result.
	Content and non-content changes are resolved separately.  For content,
	'p4 resolve' places the merged result into a temporary file in the
	client workspace.  If there are any conflicts, the merged file contains
	conflict markers that must be removed by the user.

	'p4 resolve' displays a count of text diffs and conflicts, and offers
	the following prompts:

	  Accept:
	     at              Keep only changes to their file.
	     ay              Keep only changes to your file.
	   * am              Keep merged file.
	   * ae              Keep merged and edited file.
	   * a               Keep autoselected file.

	  Diff:
	   * dt              See their changes alone.
	   * dy              See your changes alone.
	   * dm              See merged changes.
	     d               Diff your file against merged file.

	  Edit:
	     et              Edit their file (read only).
	     ey              Edit your file (read/write).
	   * e               Edit merged file (read/write).

	  Misc:
	   * m               Run '$P4MERGE base theirs yours merged'.
			     (Runs '$P4MERGEUNICODE charset base theirs
			      yours merged' if set and the file is a
			      unicode file.)
	     s               Skip this file.
	     h               Print this help message.
	     ^C              Quit the resolve operation.

	Options marked (*) appear only for text files. The suggested action
	will be displayed in brackets.

	The 'merge' (m) option enables you to invoke your own merge program,
	if one is configured using the $P4MERGE environment variable.  Four
	files are passed to the program: the base, yours, theirs, and the
	temporary file.. The program is expected to write merge results to
	the temporary file.

	The -a flag puts 'p4 resolve' into automatic mode. The user is not
	prompted, and files that can't be resolved automatically are skipped:

	    -as		'Safe' resolve; skip files that need merging.
	    -am		Resolve by merging; skip files with conflicts.
	    -af		Force acceptance of merged files with conflicts.
	    -at		Force acceptance of theirs; overwrites yours.
	    -ay		Force acceptance of yours; ignores theirs.

	The -as flag causes the workspace file to be replaced with their file
	only if theirs has changed and yours has not.

	The -am flag causes the workspace file to be replaced with the result
	of merging theirs with yours. If the merge detected conflicts, the
	file is left untouched and unresolved.

	The -af flag causes the workspace file to be replaced with the result
	of merging theirs with yours, even if there were conflicts.  This can
	leave conflict markers in workspace files.

	The -at flag resolves all files by copying theirs into yours. It
	should be used with care, as it overwrites any changes made to the
	file in the client workspace.

	The -ay flag resolves all files by accepting yours and ignoring
	theirs. It preserves the content of workspace files.

	The -n flag previews the operation without altering files.
)"
};

ErrorId MsgHelp::HelpGraphRevList = { ErrorOf( ES_HELP, 243, E_INFO, EV_NONE, 0 ),
R"(
    graph rev-list -- List commit objects in reverse chronological order.

    p4 graph rev-list -n //repo/name [-m N] [commit...]


	The specified commits from the specified repo are displayed.

	The -n flag specifies the repo name.

	The -m displays no more than max commits

	The commit arguments specifies the starting commit.
)"
};

ErrorId MsgHelp::HelpGraphLfsPush = { ErrorOf( ES_HELP, 245, E_INFO, EV_NONE, 0 ),
R"(
    graph lfs-push -- Push lfs file into repo.

    p4 graph lfs-push [ -u user ] -n repo -i /path/to/lfs/file -r sha-256-hash

	The -u flag specifies the user name.

	The -n flag specifies the repo name.

	The -i flag specifies file path to be added to the LFS archive.

	The -r flag specifies the sha256 of the LFS file.
)"
};

ErrorId MsgHelp::HelpGraphLfsFetch = { ErrorOf( ES_HELP, 246, E_INFO, EV_NONE, 0 ),
R"(
    graph lfs-fetch -- Fetch lfs file from repo.

    p4 graph lfs-fetch [ -u user ] -n repo -o /path/to/lfs/file -r sha-256-hash

	The -u flag specifies the user name.

	The -n flag specifies the repo name.

	The -o flag specifies file path to contain the LFS contents.

	The -r flag specifies the sha256 of the LFS file.
)"
};

ErrorId MsgHelp::HelpGraphLfsStat = { ErrorOf( ES_HELP, 247, E_INFO, EV_NONE, 0 ),
R"(
    graph lfs-stat -- Report data about LFS objects.

    p4 graph lfs-stat [ -u user ] -n repo oid...

	The -u flag specifies the user name.

	The -n flag specifies the repo name.

	The oid arguments are sha256 specifying the LFS objects to stat.
)"
};

ErrorId MsgHelp::HelpGraphVerify = { ErrorOf( ES_HELP, 248, E_INFO, EV_NONE, 0 ),
R"(
    graph verify -- verify repo integrity

    p4 graph verify [-q] -n //repo/name

	The repo DAG is traversed to verify that all blob objects exist
	in the archive.	Lfs archives are also verified.

	The -n flag specifies the repo name.

	The -q flag suppresses output.

	The return code is non-zero on failure.
)"
};

ErrorId MsgHelp::HelpGraphLog = { ErrorOf( ES_HELP, 225, E_INFO, EV_NONE, 0 ),
R"(
    graph log -- list commits

    p4 graph log -n //repo/name [options] commit...

	Options:
	    -a              : display all commits (default is first 1000)
	    -m max          : display no more than max commits
	    -A date         : display commits created after this date
	    -B date         : display commits created before this date
	    -u author       : display commits created by this author
	    -N minParents   : display commits having at least this many parents
	    -X maxParents   : display commits having at most this many parents
	    -p              : display commits following first parent only
	    -P path         : filters out commits that do not affect any file
	                      under this path
	    --oneline       : display one line per commit
	                    : --oneline=tree : display commit and tree sha1
	                    : --oneline --no-abbrev : display 40 char sha1

	The specified commits from the specified repo are displayed.

	The -n flag specifies the repo name.
)"
};

ErrorId MsgHelp::HelpGraphLsTree = { ErrorOf( ES_HELP, 229, E_INFO, EV_NONE, 0 ),
R"(
    graph ls-tree -- List tree contents

    p4 graph ls-tree [-r] [--name-only] treeish-sha1
    p4 graph ls-tree [-r] [--name-only] -n //repo/name [commit-sha1|reference]

	Display the contents of the specified tree object.
	treeish-sha1 may be a commit sha1 or a tree sha1.

	The -r flag recurses into sub-trees.

	The --name-only flag limits the output to the file path.

	The -n flag specifies the repo name.  In this case, a commit sha1 or
	reference may be used.

	You must have read permissions, otherwise you must be a super user.
)"
};

ErrorId MsgHelp::HelpGraphUnlock = { ErrorOf( ES_HELP, 238, E_INFO, EV_NONE, 0 ),
R"(
    unlock -- Release a locked file, leaving it open.

    p4 unlock [-c changelist# -f] [file ...]

	'p4 unlock' releases locks on the specified files.

	The changelist flag and file specification limit the files to be
	unlocked. If no file specification is given and no changelist is
	specified, all open files are unlocked.

	If a changelist is specified, only those files open in that
	changelist are unlocked.

	If a file specification is given, only those files are unlocked.

	If both changelist and file specification are provided, only the
	matching files in the specified changelist are unlocked.

	The -c flag applies to opened files in a pending changelist locked
	by 'p4 lock' or by a failed submit.

	By default, files can be unlocked only by the changelist owner who
	must also be the person who has the files locked. The -f flag
	enables you to unlock files in changelists owned by other users.
	The -f flag requires 'super' access, granted by 'p4 protect' or
	'admin' permissions on the affected repos.
)"
};

ErrorId MsgHelp::HelpGraphPrint = { ErrorOf( ES_HELP, 239, E_INFO, EV_NONE, 0 ),
R"(
    print -- Write a repo file to standard output.

    p4 print [-o localFile -q -m max] file ...

	Retrieve the contents of a repo file to the client's standard output.
	The file is not synced. If file is specified using client syntax,
	P4 uses the client view to determine the corresponding repo file.

	The -o localFile flag redirects the output to the specified file on
	the client filesystem. Multiple files may be written by using wildcards
	in the localFile argument that match wildcards in the file argument.

	The -q flag suppresses the initial line that displays the file name
	and revision.

	The -m flag limits print to the first 'max' number of files.
)"
};

ErrorId MsgHelp::HelpGraphDirs = { ErrorOf( ES_HELP, 241, E_INFO, EV_NONE, 0 ),
R"(
    dirs -- List repo subdirectories

    p4 dirs [-i] dir[@reference] ...

	List directories that match the specified file pattern (dir).
	This command supports wildcards ('...', '*').


	By default, all directories containing files are listed. If the dir
	argument includes a reference, only directories containing files
	at that reference are listed.

	The -i flag is used to ignore the case of the file pattern when
	listing directories in a case sensitive server. This flag is not
	compatible with the -C option.

)"
};

ErrorId MsgHelp::HelpGraphCatFile = { ErrorOf( ES_HELP, 250, E_INFO, EV_NONE, 0 ),
R"(
    graph cat-file -- Display object contents

    p4 graph cat-file [ commit | [ -o localFile ] -n repo blob | -t ]
	              object-sha

	Display the contents of the specified object.

	The commit flag displays a commit object.

	The -o localFile flag redirects the output to the specified file on
	the client filesystem.

	The -n flag specifies the repo name.

	The blob flag displays a blob  object.

	The -t flag displays a tree object.

	You must have read permissions, otherwise you must be a super user.
)"
};

ErrorId MsgHelp::Help2FA = { ErrorOf( ES_HELP, 232, E_INFO, EV_NONE, 0 ),
R"(
    MFA - Multi Factor Authentication.

	Support for multi factor authentication (formally referred to as
	second factor authentication or 2FA) is provided by installing three
	triggers of the types auth-pre-2fa, auth-init-2fa and auth-check-2fa
	(only one of each may be defined and all three must be present).
	These triggers are expected to return JSON results to the server.
	Once installed, and the server has been restarted, the security level
	will be implicitly set to 3 (it can be explicitly set higher).

	To configure a user to require multi factor authentication, the
	AuthMethod field in their user spec must be modified to either
	"perforce+2fa" or "ldap+2fa". This will require them to run
	"p4 login2" to perform the multi authentication steps (if automatic
	login prompting is enabled, users will automatically perform this
	after their normal password based authentication. See "p4 help login2".

	There are three phases to multi factor authentication, each based on
	the execution of that phase's trigger:

	1. list-methods

	    This phase presents the user with a list of available multi factor
	    authentication methods. For example, they may be configured to use
	    either SMS or a mobile authentication application.
	    In interactive mode, if the user only has one method, this is
	    chosen automatically. These methods are returned by the
	    "auth-pre-2fa" trigger. The trigger may also indicate that the
	    user doesn't require multi authentication in this instance, or
	    that they are not permitted access at all.

	    This trigger is expected to return 0 on success and return a JSON
	    string to the server via STDOUT. The JSON response should be in the
	    following format:

	        {
	          "status" : 0,
	          "methodlist" : [
	            [ "method1" , "method description1" ],
	            [ "method2" , "method description2" ]
	          ],
	          "message" : "Error message"
	        }

	    The status field is required, and should be 0 on success with the
	    methodlist populated: a dictionary of authentication where the
	    key is the method name and the value is the method description.
	    If the status is set to 2, this indicates that the multi factor
	    authentication is not required for this user on this host at this
	    time. Any other status value is considered to be a rejection of the
	    authentication attempt. In these cases the methodlist is not needed
	    and instead a message may be provided to be relayed to the user.
	    For example:

	        {
	          "status" : 2,
	          "message" : "Multi factor authentication not required"
	        }

	2. init-auth

	    This phase begins the multi authentication flow for the chosen
	    method. It calls the "auth-init-2fa" trigger, returning at
	    minimum the status (0 for success) and the scheme. In the case
	    of an error, the status would be non-zero and the scheme is not
	    required. A message may be reported to the user in either case.
	    An optional challenge may be set to be presented to the user. For
	    authentication flows that require state between init and check, a
	    token may be set: this is stored in the server but never reported
	    to the user: it is available to the next trigger via a trigger
	    variable. Here is an example JSON response:

	        {
	          "status" : 0,
	          "scheme" : "challenge",
	          "message" : "Please enter your response",
	          "challenge" : "ABBACD",
	          "token" : "REQID:20003339189"
	        }

	    There are 4 possible values for scheme:
	        otp-generated - A One-Time-Password generated by a user device.
	        otp-requested - A One-Time-Password sent to the user.
	        challenge     - A challenge/response based on a token displayed
	                        to the user
	        external      - A request to a 3rd-party prompting method, like
	                        an app based push notification.

	3. check-auth

	    This phases performs the verification step for the authentication
	    flow initialized by init-auth. If the scheme is "external", the
	    "auth-2fa-check" trigger is called to query the status of the
	    prompt from the authentication provider. Otherwise the user is
	    prompted for their One-Time-Password or challenge response, which
	    is passed to the "auth-2fa-check" trigger via STDIN and is
	    validated against the multi factor authentication provider. The
	    response of this trigger is represented in JSON as a status field
	    and an option message to the user. The status values are 0 for
	    success and non-zero for failure (authentication rejected). If the
	    scheme is "external", then it is possible that the authentication
	    provider might still be waiting for the user's response; returning
	    a status value of 2 will instruct the server to neither accept or
	    reject the authentication attempt. For example:

	        {
	          "status" : 2,
	          "message" : "A token was sent to your phone"
	        }

	All three trigger's specific variables are:

	    %%user%%     - the user's username
	    %%fullname%% - the user's fullname
	    %%email%%    - the user's email address
	    %%host%%     - the user's host's IP address
	    %%method%%   - the authentication method from list-methods
	                 (may be set to "unknown")
	    %%scheme%%   - the authentication schemed set by init-auth
	                 (may be set to "unknown")
	    %%token%%    - the stashed token from the last init-auth
	                 (may be empty)

	    Given that the %%fullname%% and %%email%% fields are populated from
	    fields in the user spec which are modifiable by default, if these
	    are used it is advisable to set dm.user.allowselfupdate=0. This
	    will prevent users modifying those fields.

)"
};

ErrorId MsgHelp::HelpGraphRecomputeRefcnts = { ErrorOf( ES_HELP, 270, E_INFO, EV_NONE, 0 ),
R"(
    graph recompute-refcnts -- Recompute refcount values in the object table.

    p4 graph recompute-refcnts [-r] [-y]

	The graph recompute-refcnts command may be used to recompute refcount
	values in the object table. If a recomputed value is different from
	the existing refcount, a correction will be required.

	By default, the recompute-refcnts command displays a preview of the
	required corrections. To execute the operation, specify the -y flag.

	Since this command can take a long time to complete, results for a
	preview run will be saved for future use. Specify the -r flag to make
	the command resume using the saved data from earlier runs.

)"
};

ErrorId MsgHelp::HelpGraphGc = { ErrorOf( ES_HELP, 271, E_INFO, EV_NONE, 0 ),
R"(
    graph gc -- Delete orphaned objects from the object table.

    p4 graph gc [-y]

	The graph gc command may be used to delete orphaned objects from the
	object table.

	By default, the gc command displays a preview of the results. To
	execute the operation, specify the -y flag.

)"
};

ErrorId MsgHelp::HelpGraphPurgeRefhist = { ErrorOf( ES_HELP, 272, E_INFO, EV_NONE, 0 ),
R"(
    graph purge-refhist -- Delete refhist records for a repo.

    p4 graph purge-refhist -n repo [-B date] [-y]

	The graph purge-refhist command deletes refhist records for a repo.
	These are for references that are already removed from the ref table.

	The -n flag specifies the repo name.

	The -B flag limits the deletion of reference history to records prior
	to the specified date, preserving more recent records.

	By default, the purge-refhist command displays a preview of the
	results. To execute the operation, specify the -y flag.

)"
};

ErrorId MsgHelp::HelpServerResources = { ErrorOf( ES_HELP, 277, E_INFO, EV_NONE, 0 ),
R"(
	P4 Server System Resource Monitoring

	The P4 server has the ability to monitor the availability of various
	operating system resources, and reduce the amount of work it accepts
	if resource availability is running thin.
	This prevents over-subscription of resources, and helps maintain more
	consistent performance.

	The target use-case for resource monitoring is to prevent large
	spikes of resource usage.  The server will even out the spike,
	spreading the load over a larger period of time.

	Servers with a high number of concurrently-running commands will
	benefit the most from this.  Configuring server monitoring such that
	its thresholds are above the day-to-day ceiling of load is suggested,
	as it will result in the least amount of performance change from
	the baseline.  Setting the thresholds too low will result in resource
	under-utilization and unnecessary slowdowns.

	The resources available for monitoring are CPU pressure and memory
	usage.  Resource monitoring interfaces in operating systems are
	not consistent, so not all resources can be monitored in the same
	way everywhere.

	Resource usage is categorized as being in a low/medium/high (0/1/2)
	state.

	When any resource is heading to or past its configured threshold, the
	server begins pausing incoming commands until resource usage has
	dropped below the threshold.  The paused state is entered after the
	client-server connection has been established, but before the
	requested command has begun executing.  The time a command is able to
	spend in the paused state is controlled by the
	'sys.pressure.max.pause.time' configurable.  Setting this configurable
	to zero disables pausing commands entirely.  The total number of
	commands the server will maintain in the paused state is controlled by
	the 'sys.pressure.max.paused' configurable.  A variety of
	administrative and replication commands are not subject to pausing and
	may run at any resource pressure level.

	When resource monitoring is enabled and running, 'p4 -ztag info' will
	supply a non-zero timestamp for the 'resourcePressureLastUpdate'
	field, representing the last time resource status was sampled.  When
	the server is under resource pressure, the 'resourcePressure.*'
	fields will show the rate at which commands are being paused at.

	When command monitoring is enabled, paused commands are visible with
	'p4 monitor show -s P' (the 'paused' monitor status).  The time spent
	in the paused state is recorded in the tracking entries of server
	P4LOG files and the command-end records of structured logs.

	Client commands that request progress output from the server (e.g.
	'p4 -I $command') will receive a 'command paused' message while their
	commands are being paused.  No other feedback is sent to the client
	while paused.

	Extensions may register a single 'pressure-pause' hook, which will be
	called periodically while a command going to be paused - the Extension
	can choose to pause, unpause, or defer the choice to the server - this
	can be used to implement site-specific throttling policy, picking and
	choosing which commands must wait while the OS is under pressure.

	The following configurables are available to customize server
	resource-monitoring behavior:

	    # Maximum number of seconds an individual command may wait while
	    # paused before giving up and returning an error to the client.
	    sys.pressure.max.pause.time

	    # Maximum number of concurrently-paused client commands on the
	    # server.  New incoming commands above this threshold will be
	    # rejected with an error.
	    sys.pressure.max.paused

	    # Percentage-based memory thresholds, ranged 0-100, and is the
	    # ratio of total system memory vs memory available to use without
	    # swapping.
	    sys.pressure.mem.high
	    sys.pressure.mem.medium

	    # OS-supplied resource pressure thresholds, ranged 0-100, defined
	    # as the percentage of processes on the system stalled for the
	    # resource.
	    sys.pressure.os.cpu.high
	    sys.pressure.os.mem.high
	    sys.pressure.os.mem.medium

	    # Window size for averaging samples of resource pressure.  The
	    # larger the window, the less sensitive/responsive the server
	    # will be to changes in pressure.  These values are milliseconds.
	    sys.pressure.mem.high.duration
	    sys.pressure.mem.medium.duration
	    sys.pressure.os.cpu.high.duration
	    sys.pressure.os.mem.high.duration
	    sys.pressure.os.mem.medium.duration

	The 'medium' and 'high' thresholds must not be the same, and the 'high'
	threshold must be greater than 'medium'.  When changing their settings,
	their default values will be used if these invariants are broken.

	There are two options for configuring memory pressure monitoring.  The
	first is percentage-based, which is calculated with the amount
	of memory available to use without swapping, and the total of system
	amount memory, in percent.  The second uses OS-supplied memory pressure
	data, which (if the OS supports it), gives the server a more accurate
	view of system status.  Both options may be configured together or
	separately.  Disable either or both by setting their 'high'
	configurable to zero.  The two options are complimentary, but serve
	distinct use-cases.  Percentage-based data is best used on servers
	that have a large enough buffer of free memory such that normal
	load does not reach high levels.  Percentage-based configuration is
	also how a site could reserve enough memory for the OS to maintain
	a filesystem cache.  The OS-supplied pressure option is targeted
	as the backstop that prevents application load from overrunning the
	capacity of the OS/hardware, leading to system/application instability
	and is suitable for any size of P4 Server installation.

	The 'sys.pressure.*.mem.medium' and 'sys.pressure.*.mem.high'
	configurables mark the relevant thresholds for memory monitoring.
	The 'medium' level specifies where the server will try to keep memory
	usage below.  The 'high' level is intended to be set below where the
	operating system is about to thrash into swap or become unstable (such
	as having processes subject to the OOM killer). New incoming commands
	received by the server while at the 'high' threshold will be rejected
	outright.  Existing commands that request more memory while the server
	is above the 'high' threshold may be canceled and return an error to
	the client.  Note that the server does not distinguish between memory
	used by other processes on the operating system and its own when
	limiting its work - e.g. if a large external process comes and consumes
	a lot of memory, the P4 Server can throttle itself in response.

	The OS-supplied memory pressure option is available on all Windows
	versions, and on Linux with cgroup v2.  For Linux, the
	'sys.pressure.os.mem.high' configurable represents the amount of time
	some processes on the system are spending stalled waiting for the
	memory.  See the Linux kernel's definition of
	'Pressure Stall Information' (PSI) for further details.  Note that on
	Windows, only the 'high' pressure state (low memory) is reported by
	the OS, so 'sys.pressure.os.mem.medium' is not available.

	CPU monitoring is only available on Linux platforms with cgroups v2
	support configured.  The 'sys.pressure.os.cpu.high' configurable
	represents the amount of time some processes on the system are spending
	stalled waiting for CPU time.

	For Linux cgroup support, only the system-wide '/proc/pressure/*' files
	are considered.

	Administrators are encouraged to consider their usage patterns and
	hardware configuration and to evaluate non-default settings.
	Setting the group 'MaxMemory' limit for all commands such that the
	upper limit is never too high is a good idea, so the distance between
	the medium memory threshhold and the high level has some multiple of
	the biggest possible command's memory.  Memory checking within each
	command as it is processing is also done at periodic intervals, so
	the distance between the 'high' threshold and the real out-of-memory
	zone for the operating system ought to have some room to accommodate
	allocations between checks.

	The prerequisites for the server to be able to respond to resource
	pressure are:

	    0) Operating system support

	    1) Realtime monitoring counters enabled.

	    2) The 'p4 admin resource-monitor' server startup command
	       running (see 'p4 help admin-resource-monitor').

	    3) Existing resource usage baselines have been established.

	    4) Tweak pressure-related configurables to above-baseline values.

	    5) Enough space between the baseline usage and the medium/high
	       thresholds such that the command-pausing is not always on.

	E.g.:

	    p4 serverid $name
	    p4 configure set rt.monitorfile=$monitor_file
	    p4 configure set "$name#startup.1=admin resource-monitor"
	    p4 admin restart

	It is recommended that prior to enabling the full resource monitoring
	configuration, that it be run in preview-mode for a while in order to
	gauge the effect of the configured thresholds.  Preview-mode is when
	the feature is configured, but 'sys.pressure.max.pause.time=0' - i.e.
	the 'p4 admin resource-monitor' background task is sampling resources,
	setting pressure levels, but commands are not subject to pausing.

	The following can be used to disable system resource monitoring:

	    0) If the configurable server.startup.autorestart is set to 0,
	       remove the 'p4 admin resource-monitor' startup process
	       with 'p4 monitor terminate'.

	    1) Setting the 'sys.pressure.max.pause.time' configurable to zero.
	       will disable pausing commands but monitoring will still run.

	    2) Depending on what you want to disable, set the value for one or
	       more of the following configurables to zero:

	         sys.pressure.mem.high
	         sys.pressure.os.cpu.high
	         sys.pressure.os.mem.high

	To further protect the P4 server from the effects of low system
	resources, on Linux, the server can be configured to coordinate with
	the system OOM killer (Out Of Memory), effectively shielding the P4
	server from being killed when the operating system kernel is running
	out of memory.  The P4 server, when configured, will change its OOM
	score during database writes, and restore it to the default when done
	writing in order to protect its database integrity.

	To enable OOM protection, the "sys resource" system capability must be
	granted to the P4 server executable ('p4d'), e.g. via:

	    sudo setcap 'CAP_SYS_RESOURCE=+ep' p4d

	The default score of -1000 effectively makes it impossible to kill the
	server during low memory conditions.  It's possible to disable or
	change the likelyhood of the OOM killer targeting a database writer via
	the 'server.oom_adj_score' configurable.  A value of zero means 'off',
	and increasing values between -1000 and 0 correspond to increasing
	likelyhood of OOM kills.

	To verify that the server is able to perform an OS-specific
	resource-related task (i.e. the configuration is correct), look for
	a server startup log message about capabilities, e.g. via:

	    p4d -p localhost:$tmp_port -r $tmp_p4root -L -
)"

};


ErrorId MsgHelp::HelpDiagnostics = { ErrorOf( ES_HELP, 278, E_INFO, EV_NONE, 0 ),
R"(
    diagnostics -- generate server diagnostics

    p4 diagnostics [-D] [-j] [-J [-L limit]]
    p4 diagnostics [-D] [-l] [-s [--strace-runtime secs]] [-p] [-t]
	[-h] [-a [-J [-L limit]] ]

	The first form of the command generates the basic
	diagnostic data, downloads it and places it
	into a compressed tar file (.tgz).
	Within this tar image are a number of files:
	diags - basic configuration details, configuration,
	        configuration history and server setting;
	info  - tagged output from the 'p4 info' and 'p4 servers -J'
	        commands and a list of all non default configurables;
	monitor - the monitor records, if monitoring is enabled;
	topology - a textual description of network that contains
	           this server. Servers that do not have a serverID
	           are ignored;

	The '-D' option requests that rather than download a single
	compressed tar file, the system creates a new directory
	and downloads the requested uncompressed individual files
	into this new directory.

	If the '-J' option is specified then a copy of the current
	journal file and log file will also be downloaded, using the
	configured names of these files.

	The '-L' limit option limits the download size
	of the journal and log files to the last 'limit' number of bytes
	of these files subject to buffer rounding. The 'limit' option is
	in bytes and can be suffixed with a single character to designate
	a unit multiplier, k or K for kilobytes, m or M for megabytes,
	g or G for gigabytes and t or T for terabytes.

	The '-j' option requests that the 'diags', ckphist' and
	'monitor' files	are encoded in JSON format. These output files
	are named 'diags.json', 'ckphist.json'  and 'monitor.json'. Any
	non-UFT8 sequences in a string are replaced by U+FFFD in the JSON
	output.

	The second form of the command attempts to run more costly
	operations so these are not run by default.

	The '-l' option requests that '/usr/bin/lsof' be run
	to provide locking details of all files in use by Perforce
	servers on this system. The data from this operation is
	downloaded into a file named 'manuallocks' in the current
	directory. This option is not supported on all platforms.

	The '-s' option runs an 'strace' command on the server
	processes for 20 seconds and downloads the trace output to a
	file named 'strace' in the current directory.
	The '--strace-runtime secs' overrides the 20 second time that
	strace executes.

	The '-t' option runs the 'top' program and returns its
	output.

	The '-p' option runs the 'ps' program and returns its
	output.

	The '-h' option dumps all the checkpoint history records
	into a 'ckphist' file.

	When the '-l', '-t', '-p', '-h'  or '-s' options are specified, then
	the basic diagnostic files are not produced. These can be requested
	by adding the '-a' option to the command.

	This command requires the operator role or 'admin' permission.
)"

};


// ErrorId graveyard: retired/deprecated ErrorIds.

ErrorId MsgHelp::HelpBrowse = { ErrorOf( ES_HELP, 107, E_INFO, EV_NONE, 0 ),
R"(
    browse -- Browse for a list of Zeroconf-registered P4 servers.

    p4 browse

	'p4 browse' lists all the P4 servers that have registered
	with Zeroconf.  Requires a 2008.2 or above client and local dynamic
	libraries and Zeroconf (Avahi or Bonjour) services.
)"
}; // DEPRECATED 2013.1 removed ZeroConf

ErrorId MsgHelp::HelpZeroConf = { ErrorOf( ES_HELP, 105, E_INFO, EV_NONE, 0 ),
R"(
   Zeroconf service support

   The following flags and environment variables configure both
   server and proxy processes

    p4d -0
	When the P4 server is registered as a Zeroconf service, the
	service-type is 'p4', which is visible in DNS Service browse mode
	Discovery as '_p4._tcp'.  If mDNS is not available on the host,
	service registration fails and logs an error in the server log,
	after which server continues the startup process.

	The -0 (digit zero) flag enables Zeroconf registry at server start time,
	overriding any value set for the $P4ZEROCONF environment variable.
	When using the environment variable, the presence of the variable
	in the environment enables the service registry.

	    The following identity flags can be used to configure Zeroconf
	    registry settings:

	    p4d -In name
		Specifies a unique name for your server.  This name is
		displayed in 'p4 -ztag info' output and by DNS Service
		Discovery, if you register the process as a Zeroconf service.
		This flag overrides any value set for the $P4NAME environment
		variable.  If the name includes spaces, double-quote it.
		A valid name must contain at least one non-numeric character.

	    p4d -Id description
		Specifies a description of your server. This description is
		displayed in 'p4 -ztag info' output and by DNS Service
		Discovery, if you register the process as a Zeroconf service.
		This flag overrides any value set for the $P4DESCRIPTION
		environment variable.  If the description includes spaces,
		double-quote it.

)"
}; // DEPRECATED 2013.1 removed ZeroConf

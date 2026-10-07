/*
 * Copyright 1995, 2000 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * msglbr.cc - definitions of errors for Rcs subsystem.
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
 * Current high value for a MsgLbr error code from main is: 151
 */
# include <stdhdrs.h>
# include <error.h>
# include <errornum.h>
# include "msglbr.h"

ErrorId MsgLbr::BadType1               = { ErrorOf( ES_LBR, 1, E_FATAL, EV_FAULT, 1 ), "Unsupported librarian file type %lbrType%!" } ;
ErrorId MsgLbr::Purged                 = { ErrorOf( ES_LBR, 2, E_FAILED, EV_EMPTY, 2 ), "Old revision %lbrRev% of tempobj %lbrFile% purged; try using head revision." } ;
ErrorId MsgLbr::ScriptFailed           = { ErrorOf( ES_LBR, 3, E_FAILED, EV_ADMIN, 3 ), "%trigger% %op%: %result%" } ;
                               
ErrorId MsgLbr::After                  = { ErrorOf( ES_LBR, 101, E_FATAL, EV_FAULT, 1 ), "RCS no revision after %name%!" } ;
ErrorId MsgLbr::Checkin                = { ErrorOf( ES_LBR, 102, E_FATAL, EV_FAULT, 2 ), "RCS checkin %file%#%rev% failed!" } ;
ErrorId MsgLbr::Checkout               = { ErrorOf( ES_LBR, 103, E_FATAL, EV_FAULT, 1 ), "RCS checkout %file% failed!" } ;
ErrorId MsgLbr::Commit                 = { ErrorOf( ES_LBR, 104, E_FATAL, EV_FAULT, 1 ), "RCS can't commit changes to %file%!" } ;
ErrorId MsgLbr::Diff                   = { ErrorOf( ES_LBR, 105, E_FATAL, EV_FAULT, 1 ), "RCS diff %file% failed!" } ;
ErrorId MsgLbr::Edit0                  = { ErrorOf( ES_LBR, 106, E_FATAL, EV_FAULT, 0 ), "RCS editLineNumber past currLineNumber!" } ;
ErrorId MsgLbr::Edit1                  = { ErrorOf( ES_LBR, 107, E_FATAL, EV_FAULT, 0 ), "RCS editLineCount bogus in RcsPieceDelete!" } ;
ErrorId MsgLbr::Edit2                  = { ErrorOf( ES_LBR, 108, E_FATAL, EV_FAULT, 1 ), "RCS editLine '%line%' bogus!" } ;
ErrorId MsgLbr::Empty                  = { ErrorOf( ES_LBR, 109, E_FATAL, EV_FAULT, 0 ), "RCS checkin author/state empty!" } ;
ErrorId MsgLbr::EofAt                  = { ErrorOf( ES_LBR, 110, E_FATAL, EV_FAULT, 0 ), "RCS EOF in @ block!" } ;
ErrorId MsgLbr::ExpDesc                = { ErrorOf( ES_LBR, 111, E_FATAL, EV_FAULT, 0 ), "RCS expected desc!" } ;
ErrorId MsgLbr::Expect                 = { ErrorOf( ES_LBR, 111, E_FATAL, EV_FAULT, 2 ), "RCS expected %token%, got %token2%!" } ;
ErrorId MsgLbr::ExpEof                 = { ErrorOf( ES_LBR, 112, E_FATAL, EV_FAULT, 0 ), "RCS expected EOF!" } ;
ErrorId MsgLbr::ExpRev                 = { ErrorOf( ES_LBR, 113, E_FATAL, EV_FAULT, 0 ), "RCS expected optional revision!" } ;
ErrorId MsgLbr::ExpSemi                = { ErrorOf( ES_LBR, 114, E_FATAL, EV_FAULT, 0 ), "RCS expected ;!" } ;
ErrorId MsgLbr::Lock                   = { ErrorOf( ES_LBR, 115, E_FATAL, EV_FAULT, 1 ), "RCS lock on %file% failed!" } ;
ErrorId MsgLbr::Loop                   = { ErrorOf( ES_LBR, 116, E_FATAL, EV_FAULT, 1 ), "RCS loop in revision tree at %name%!" } ;
ErrorId MsgLbr::Mangled                = { ErrorOf( ES_LBR, 117, E_FATAL, EV_FAULT, 1 ), "RCS delta mangled: %text%!" } ;
ErrorId MsgLbr::MkDir                  = { ErrorOf( ES_LBR, 118, E_FATAL, EV_FAULT, 1 ), "RCS can't make directory for %file%!" } ;
ErrorId MsgLbr::NoBrRev                = { ErrorOf( ES_LBR, 119, E_FATAL, EV_FAULT, 1 ), "RCS no branch to revision %rev%!" } ;
ErrorId MsgLbr::NoBranch               = { ErrorOf( ES_LBR, 120, E_FATAL, EV_FAULT, 1 ), "RCS no such branch %branch%!" } ;
ErrorId MsgLbr::NoRev                  = { ErrorOf( ES_LBR, 121, E_FATAL, EV_FAULT, 1 ), "RCS no such revision %rev%!" } ;
ErrorId MsgLbr::NoRev3                 = { ErrorOf( ES_LBR, 123, E_FATAL, EV_FAULT, 1 ), "RCS expected revision %rev% missing!" } ;
ErrorId MsgLbr::NoRevDel               = { ErrorOf( ES_LBR, 124, E_FATAL, EV_FAULT, 1 ), "RCS non-existant revision %rev% to delete!" } ;
ErrorId MsgLbr::Parse                  = { ErrorOf( ES_LBR, 125, E_FATAL, EV_FAULT, 1 ), "RCS parse error at line %line%!" } ;
ErrorId MsgLbr::RevLess                = { ErrorOf( ES_LBR, 126, E_FATAL, EV_FAULT, 0 ), "RCS log without matching revision!" } ;
ErrorId MsgLbr::TooBig                 = { ErrorOf( ES_LBR, 127, E_FATAL, EV_FAULT, 0 ), "RCS token too big!" } ;
ErrorId MsgLbr::RcsTooBig              = { ErrorOf( ES_LBR, 128, E_FAILED, EV_TOOBIG, 1 ), "Result RCS file '%file%' is too big; change type to compressed text." } ;
ErrorId MsgLbr::LbrOpenFail            = { ErrorOf( ES_LBR, 131, E_FAILED, EV_FAULT, 2 ), "Error opening librarian file %lbrFile% revision %lbrRev%." } ;
ErrorId MsgLbr::AlreadyOpen            = { ErrorOf( ES_LBR, 132, E_FATAL, EV_FAULT, 1 ), "Librarian for %path% is already open!" } ;
ErrorId MsgLbr::NotOpen                = { ErrorOf( ES_LBR, 149, E_FATAL, EV_FAULT, 2 ), "Librarian %op% attempted on %path% which is not open!" } ;
ErrorId MsgLbr::FmtLbrStat3            = { ErrorOf( ES_LBR, 133, E_INFO, EV_NONE, 15 ), "%file% %rev% %type% %state% %action% %digest% %size% %change% %revDate% %modTime% %process% %timestamp% %origin% %retries% %lastError%" } ;
ErrorId MsgLbr::FmtLbrStat4            = { ErrorOf( ES_LBR, 137, E_INFO, EV_NONE, 16 ), "%file% %rev% %type% %state% %action% %digest% %size% %change% %revDate% %modTime% %process% %timestamp% %target% %origin% %retries% %lastError%" } ;
ErrorId MsgLbr::FmtLbrStat5            = { ErrorOf( ES_LBR, 138, E_INFO, EV_NONE, 17 ), "%file% %rev% %type% %state% %action% %digest% %size% %change% %revDate% %modTime% %process% %timestamp% %target% %origin% %retries% %retryMissing% %lastError%" } ;
ErrorId MsgLbr::BadKeyword             = { ErrorOf( ES_LBR, 134, E_FATAL, EV_FAULT, 1 ), "RCS keyword for %file% is malformed!" } ;
ErrorId MsgLbr::KeywordUnterminated    = { ErrorOf( ES_LBR, 135, E_FATAL, EV_FAULT, 2 ), "While processing keywords in file %file%, a line longer than %length% was encountered which contained an initial keyword '$' sign but no matching terminating '$' sign. The maximum line length value can be configured by setting lbr.rcs.maxlen; alternately, if keyword expansion is not necessary for this file, change the file's type to remove the +k option (see 'p4 help filetypes')." } ;
ErrorId MsgLbr::ObjectReadError        = { ErrorOf( ES_LBR, 136, E_FAILED, EV_FAULT, 5 ), "Error reading object type %objType% with sha %objSha% of length %expectedLength% at offset %offset% from pack % pack%." } ;
ErrorId MsgLbr::SameFile               = { ErrorOf( ES_LBR, 139, E_FATAL, EV_FAULT, 3 ), "Librarian for %path% [rev %rev% and type %type%] is both target and source!" } ;
ErrorId MsgLbr::LbrTypeInsane          = { ErrorOf( ES_LBR, 140, E_WARN, EV_FAULT, 2 ), "Librarian for %path% has unexpected lbrType of %lbrType%!" } ;
ErrorId MsgLbr::LbrTrackInsane         = { ErrorOf( ES_LBR, 141, E_WARN, EV_FAULT, 2 ), "Librarian for %path% has unexpected lbrTrack of %lbrTrack%!" } ;

ErrorId MsgLbr::S3UploadFailed         = { ErrorOf( ES_LBR, 142, E_FAILED, EV_FAULT, 1 ), "Upload [of '%s3path%' ]to s3 failed!" } ;
ErrorId MsgLbr::S3DownloadFailed       = { ErrorOf( ES_LBR, 143, E_FAILED, EV_FAULT, 1 ), "Download [of '%s3path%' ]from s3 failed!" } ;
ErrorId MsgLbr::S3CopyFailed           = { ErrorOf( ES_LBR, 144, E_FAILED, EV_FAULT, 1 ), "Copy [of '%s3path%' ]to s3 failed!" } ;
ErrorId MsgLbr::S3StatFailed           = { ErrorOf( ES_LBR, 145, E_FAILED, EV_FAULT, 1 ), "Stat [of '%s3path%' ]from s3 failed!" } ;
ErrorId MsgLbr::S3DeleteFailed         = { ErrorOf( ES_LBR, 146, E_FAILED, EV_FAULT, 1 ), "Delete [of '%s3path%' ]from s3 failed!" } ;
ErrorId MsgLbr::S3UnsupportedOpen      = { ErrorOf( ES_LBR, 147, E_FAILED, EV_FAULT, 0 ), "Open mode not supported for S3!" } ;
ErrorId MsgLbr::S3LbrLockLoop          = { ErrorOf( ES_LBR, 150, E_FAILED, EV_FAULT, 0 ), "LbrS3 Lock loop detected in MaybeDownload()" } ;

ErrorId MsgLbr::ChunkingCreateNewLbr   = { ErrorOf( ES_LBR, 148, E_FAILED, EV_FAULT, 1 ), "Error recreating new archive: %error%" } ;
ErrorId MsgLbr::LbrFileNotSeekable     = { ErrorOf( ES_LBR, 151, E_FAILED, EV_FAULT, 1 ), "Librarian for %path% is not seekable!" } ;

// ErrorId graveyard: retired/deprecated ErrorIds. 

ErrorId MsgLbr::FmtLbrStat             = { ErrorOf( ES_LBR, 129, E_INFO, EV_NONE, 11 ), "%file% %rev% %type% %state% %action% %digest% %size% %process% %timestamp% %retries% %lastError%" } ;
ErrorId MsgLbr::FmtLbrStat2            = { ErrorOf( ES_LBR, 130, E_INFO, EV_NONE, 14 ), "%file% %rev% %type% %state% %action% %digest% %size% %change% %revDate% %modTime% %process% %timestamp% %retries% %lastError%" } ;

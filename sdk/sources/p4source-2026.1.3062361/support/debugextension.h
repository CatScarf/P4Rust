/*
 * Copyright 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

/*
 * -vexts=n  where n is 1 to 5
 * 1 = DL_ERROR
 * 2 = DL_WARNING
 * 3 = DL_INFO
 * 4 = DL_DETAILED
 * 5 = DL_DEBUG
 * (see debug.h)
 *
 */


# define EXTS_ERROR    ( p4debug.GetLevel( DT_EXTS ) >= DL_ERROR )
# define EXTS_WARNING  ( p4debug.GetLevel( DT_EXTS ) >= DL_WARNING )
# define EXTS_INFO     ( p4debug.GetLevel( DT_EXTS ) >= DL_INFO )
# define EXTS_DETAILED ( p4debug.GetLevel( DT_EXTS ) >= DL_DETAILED )
# define EXTS_DEBUG    ( p4debug.GetLevel( DT_EXTS ) >= DL_DEBUG )



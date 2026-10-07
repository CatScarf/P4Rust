/*
 * Copyright 2019 Perforce Software.  All rights reserved.
 *
 * This file is part of Perforce - the FAST SCM System.
 */

# include <stdhdrs.h>
# include <strbuf.h>
# include <error.h>

# ifdef HAS_EXTENSIONS

# include "p4script.h"
# include "p4script53.h"

// We include all of Lua here, compiled as C++ so exceptions work properly,
// for easier symbol redefinition (to prevent namespace clashes), and for
// potential performance improvements since the compiler can see everything
// at once.


# if defined(__GNUG__) && GCC_VERSION >= 40600
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-function"
# endif
# if defined(__clang__)
# pragma clang diagnostic push
# pragma clang diagnostic ignored "-Wunused"
# endif

# include "lua-5.3/one.cc"

# if defined(__GNUG__) && GCC_VERSION >= 40600
# pragma GCC diagnostic pop
# endif
# if defined(__clang__)
# pragma clang diagnostic pop
# endif

# endif

/* $Id$

   Replacement for a missing setenv.

   Written by Russ Allbery <rra@stanford.edu>
   This work is hereby placed in the public domain by its author.

   %AUTOCONF%
   AC_C_CONST
   %%

   Provides the same functionality as the standard library routine setenv
   for those platforms that don't have it. */

#include "config.h"

#include <errno.h>
#if STDC_HEADERS
# include <string.h>
#endif

/* Solaris at least wants _XOPEN_SOURCE defined and _XOPEN_VERSION set to at
   least 4 in order to make putenv available when compiling in a strictly
   standards-conforming environment (e.g. gcc -ansi). */
#ifndef _XOPEN_SOURCE
# define _XOPEN_SOURCE
# undef _XOPEN_VERSION
# define _XOPEN_VERSION 4
#endif
#include <stdlib.h>

/* If we're running the test suite, rename setenv to avoid conflicts with
   the system version. */
#if TESTING
# define setenv test_setenv
#endif

int
setenv(const char *name, const char *value, int overwrite)
{
    char *envstring;

    if (!overwrite && getenv(name))
        return 0;

    /* Allocate memory for the environment string.  We intentionally don't
       use concat here, or the xmalloc family of allocation routines, since
       the intention is to provide a replacement for the standard library
       function which sets errno and returns in the event of a memory
       allocation failure. */
    envstring = malloc(strlen(name) + 1 + strlen(value) + 1);
    if (!envstring) {
        errno = ENOMEM;
        return -1;
    }

    /* Build the environment string and add it to the environment using
       putenv.  Systems without putenv lose, but XPG4 requires it. */
    strcpy(envstring, name);
    strcat(envstring, "=");
    strcat(envstring, value);
    return putenv(envstring);

    /* Note that the memory allocated is not freed.  This is intentional;
       many implementations of putenv assume that the string passed to
       putenv will never be freed and don't make a copy of it.  Repeated use
       of this function will therefore leak memory, since most
       implementations of putenv also don't free strings removed from the
       environment (due to being overwritten). */
}
/*
 * Portability wrapper around the kafs API.
 *
 * This header includes kafs.h if it's available, prototypes k_hasafs,
 * k_setpag, and k_unlog replacements (generally provided by the kafs
 * replacement library) imlemented in terms of our system call layer or
 * lsetpag if it is available and libkafs isn't, and as a last resort provides
 * a k_hasafs function that always fails and k_setpag and k_unlog functions
 * that always succeed.
 *
 * It also defines the HAVE_KAFS macro to 1 if some AFS support was available,
 * in case programs that use it want to handle the case of no AFS support
 * differently (such as in help output).
 *
 * Written by Russ Allbery <rra@stanford.edu>
 * Copyright 2006, 2007, 2008, 2010
 *     Board of Trustees, Leland Stanford Jr. University
 *
 * See LICENSE for licensing terms.
 */

#ifndef PORTABLE_KAFS_H
#define PORTABLE_KAFS_H 1

#include <config.h>
#ifdef HAVE_KERBEROS
# include <portable/krb5.h>
#endif
#include <portable/macros.h>

#include <errno.h>
#ifdef HAVE_SYS_IOCCOM_H
# include <sys/ioccom.h>
#endif
#include <sys/ioctl.h>

BEGIN_DECLS

/* Assume we have some AFS support available and #undef below if not. */
#define HAVE_KAFS 1

/* We have a libkafs or libkopenafs library. */
#if HAVE_K_HASAFS
# if HAVE_KAFS_H
#  include <kafs.h>
# elif HAVE_KOPENAFS_H
#  include <kopenafs.h>
# endif
# ifndef HAVE_K_HASPAG
int k_haspag(void);
# endif

/* We're linking directly to the OpenAFS libraries. */
#elif HAVE_LSETPAG
# if HAVE_AFS_AFSSYSCALLS_H
#  include <afs/afssyscalls.h>
# else
int lsetpag(void);
# endif
# define k_hasafs() (1)
# define k_setpag() lsetpag()
# define k_unlog()  (errno = ENOSYS, -1)

int k_haspag(void);

/* We're using our local kafs replacement. */
#elif HAVE_KAFS_REPLACEMENT
# define HAVE_K_PIOCTL 1

struct ViceIoctl {
    void *in, *out;
    short in_size;
    short out_size;
};

int k_hasafs(void);
int k_haspag(void);
int k_pioctl(char *, int, struct ViceIoctl *, int);
int k_setpag(void);
int k_unlog(void);

/* We have no kafs implementation available. */
#else
# undef HAVE_KAFS
# define k_hasafs() (0)
# define k_haspag() (0)
# define k_setpag() (errno = ENOSYS, -1)
# define k_unlog()  (errno = ENOSYS, -1)
#endif

END_DECLS

#endif /* PORTABLE_KAFS_H */
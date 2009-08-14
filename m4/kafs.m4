dnl Test for a libkafs library or replacement.
dnl
dnl Check for a working libkafs library, and if not present, check how we can
dnl simulate what one would do ourselves, adding the appropriate things to
dnl LIBOBJS.  Provides the --with-libkafs configure option specify a
dnl non-standard path to libkafs or (as --without-libkafs) to force use of the
dnl internal implementation; --with-libkafs-include and --with-libkafs-lib to
dnl specify paths at a more granular level; and --with-afs,
dnl --with-afs-include, and --with-afs-lib configure options to specify the
dnl location of the AFS libraries.
dnl
dnl Provides the macro RRA_LIB_KAFS and sets the substition variables
dnl KAFS_CPPFLAGS, KAFS_LDFLAGS, and KAFS_LIBS.  If Kerberos libraries may be
dnl needed, LIBS and LDFLAGS must already be set appropriately before calling
dnl this.  Also provides RRA_LIB_KAFS_SWITCH to set CPPFLAGS, LDFLAGS, and
dnl LIBS to include libkafs, saving the current values first, and
dnl RRA_LIB_KAFS_RESTORE to restore those settings to before the last
dnl RRA_LIB_KFS_SWITCH.
dnl
dnl Sets HAVE_K_HASAFS if the k_hasafs function was found in a libkafs
dnl library.  Sets HAVE_LSETPAG if building against the AFS libraries and the
dnl lsetpag function is present.  Sets the Automake conditional NEED_KAFS if
dnl building the replacement for the libkafs library and defines
dnl HAVE_KAFS_LINUX or HAVE_KAFS_SYSCALL as appropriate.
dnl
dnl Depends on RRA_SET_LDFLAGS.
dnl
dnl Written by Russ Allbery <rra@stanford.edu>
dnl Copyright 2008, 2009 Board of Trustees, Leland Stanford Jr. University
dnl
dnl See LICENSE for licensing terms.

dnl Save the current CPPFLAGS, LDFLAGS, and LIBS settings and switch to
dnl versions that include the libkafs flags.  Used as a wrapper, with
dnl RRA_LIB_KAFS_RESTORE, around tests.
AC_DEFUN([RRA_LIB_KAFS_SWITCH],
[rra_kafs_save_CPPFLAGS="$CPPFLAGS"
 rra_kafs_save_LDFLAGS="$LDFLAGS"
 rra_kafs_save_LIBS="$LIBS"
 CPPFLAGS="$KAFS_CPPFLAGS $CPPFLAGS"
 LDFLAGS="$KAFS_LDFLAGS $LDFLAGS"
 LIBS="$KAFS_LIBS $LIBS"])

dnl Restore CPPFLAGS, LDFLAGS, and LIBS to their previous values (before
dnl RRA_LIB_KAFS_SWITCH was called).
AC_DEFUN([RRA_LIB_KAFS_RESTORE],
[CPPFLAGS="$rra_kafs_save_CPPFLAGS"
 LDFLAGS="$rra_kafs_save_LDFLAGS"
 LIBS="$rra_kafs_save_LIBS"])

dnl Set KAFS_CPPFLAGS and KAFS_LDFLAGS based on rra_kafs_root,
dnl rra_kafs_libdir, rra_kafs_includedir, rra_afs_root, rra_afs_libdir, and
dnl rra_afs_includedir.
AC_DEFUN([_RRA_LIB_KAFS_PATHS],
[KAFS_LDFLAGS=
 AS_IF([test x"$rra_kafs_libdir" != x],
    [KAFS_LDFLAGS="-L$rra_kafs_libdir"],
    [AS_IF([test x"$rra_kafs_root" != x],
        [RRA_SET_LDFLAGS([KAFS_LDFLAGS], [$rra_kafs_root])])])
 AS_IF([test x"$rra_kafs_includedir" != x],
    [KAFS_CPPFLAGS="-I$rra_kafs_includedir"],
    [AS_IF([test x"$rra_kafs_root" != x],
        [AS_IF([test x"$rra_kafs_root" != x/usr],
            [KAFS_CPPFLAGS="-I${rra_kafs_root}/include"])])])
 AS_IF([test x"$rra_afs_libdir" != x],
    [KAFS_LDFLAGS="$KAFS_LDFLAGS -L$rra_afs_libdir"],
    [AS_IF([test x"$rra_afs_root" != x],
        [RRA_SET_LDFLAGS([KAFS_LDFLAGS], [$rra_afs_root])])
         RRA_SET_LDFLAGS([KAFS_LDFLAGS], [$rra_afs_root], [afs])])
 AS_IF([test x"$rra_afs_includedir" != x],
    [KAFS_CPPFLAGS="-I$rra_afs_includedir"],
    [AS_IF([test x"$rra_afs_root" != x],
        [AS_IF([test x"$rra_afs_root" != x/usr],
            [KAFS_CPPFLAGS="$KAFS_CPPFLAGS -I${rra_afs_root}/include"])])])])

dnl Probe for lsetpag in the AFS libraries.  This is required on AIX and IRIX
dnl since we can't use the regular syscall interface there.
AC_DEFUN([_RRA_LIB_KAFS_LSETPAG],
[RRA_LIB_KAFS_SWITCH
 LIBS=
 AC_SEARCH_LIBS([pthread_getspecific], [pthread])
 AC_SEARCH_LIBS([res_search], [resolv], ,
    [AC_SEARCH_LIBS([__res_search], [resolv])])
 AC_SEARCH_LIBS([gethostbyname], [nsl])
 AC_SEARCH_LIBS([socket], [socket], ,
    [AC_CHECK_LIB([nsl], [socket], [LIBS="-lnsl -lsocket $LIBS"], ,
        [-lsocket])])
 rra_kafs_extra="$LIBS"
 LIBS="$rra_kafs_save_LIBS"
 AC_CHECK_LIB([afsauthent], [lsetpag],
    [KAFS_LIBS="-lafsauthent -lafsrpc $rra_kafs_extra"
     AC_DEFINE([HAVE_LSETPAG], [1],
        [Define to 1 if you have the OpenAFS lsetpag function.])],
    [AC_CHECK_LIB([sys], [lsetpag],
        [KAFS_LIBS="-lsys $rra_kafs_extra"
         AC_DEFINE([HAVE_LSETPAG], [1],
            [Define to 1 if you have the OpenAFS lsetpag function.])], ,
        [$rra_kafs_extra])],
    [-lafsrpc $rra_kafs_extra])
 AC_CHECK_HEADERS([afs/afssyscalls.h])
 RRA_LIB_KAFS_RESTORE])
        
dnl The public entry point.  Sets up the --with options and then decides what
dnl to do based on the system.
AC_DEFUN([RRA_LIB_KAFS],
[AC_REQUIRE([AC_CANONICAL_HOST])
 rra_libkafs=true
 rra_build_kafs=false
 KAFS_CPPFLAGS=
 KAFS_LDFLAGS=
 KAFS_LIBS=
 AC_SUBST([KAFS_CPPFLAGS])
 AC_SUBST([KAFS_LDFLAGS])
 AC_SUBST([KAFS_LIBS])

 dnl In addition to the normal path-finding options, support --without-libkafs
 dnl to force always using the internal AFS syscall code.
 AC_ARG_WITH([libkafs],
    AC_HELP_STRING([--with-libkafs=DIR],
        [Location of kafs headers and libraries]),
    [AS_IF([test x"$withval" = xno],
        [rra_libkafs=false],
        [AS_IF([test x"$withval" != xyes], [rra_krb5_root="$withval"])])])
 AC_ARG_WITH([kafs-include],
    [AS_HELP_STRING([--with-libkafs-include=DIR],
        [Location of kafs headers])],
    [AS_IF([test x"$withval" != xyes && test x"$withval" != xno],
        [rra_kafs_includedir="$withval"])])
 AC_ARG_WITH([kafs-lib],
    [AS_HELP_STRING([--with-libkafs-lib=DIR],
        [Location of kafs libraries])],
    [AS_IF([test x"$withval" != xyes && test x"$withval" != xno],
        [rra_kafs_libdir="$withval"])])

 dnl The location of the AFS headers and libraries.  We may not use these
 dnl results, but configure always includes the prompt, so always handle them.
 dnl This should probably be in a separate macro file.
 AC_ARG_WITH([afs],
    [AC_HELP_STRING([--with-afs=DIR],
        [Location of AFS headers and libraries])],
    [AS_IF([test x"$withval" != xno && test x"$withval" != xyes],
        [rra_afs_root="$withval"])])
 AC_ARG_WITH([afs-include],
    [AS_HELP_STRING([--with-afs-include=DIR],
        [Location of Kerberos v5 headers])],
    [AS_IF([test x"$withval" != xyes && test x"$withval" != xno],
        [rra_afs_includedir="$withval"])])
 AC_ARG_WITH([afs-lib],
    [AS_HELP_STRING([--with-afs-lib=DIR],
        [Location of Kerberos v5 libraries])],
    [AS_IF([test x"$withval" != xyes && test x"$withval" != xno],
        [rra_afs_libdir="$withval"])])

 dnl If we may use the system libkafs, see if we can find one.  We don't even
 dnl attempt to find additional dependent libraries for static linking or
 dnl shared libraries without transitive dependencies here.  Hopefully it no
 dnl longer matters.
 AS_IF([test x"$rra_libkafs" != xfalse],
    [_RRA_LIB_KAFS_PATHS
     RRA_LIB_KAFS_SWITCH
     AC_CHECK_LIB([kafs], [k_hasafs],
        [KAFS_LIBS="-lkafs"
         AC_CHECK_HEADERS([kafs.h])],
        [AC_CHECK_LIB([kopenafs], [k_hasafs],
            [KAFS_LIBS="-lkopenafs"
             AC_CHECK_HEADERS([kopenafs.h])],
            [rra_libkafs=false])])
     RRA_LIB_KAFS_RESTORE])

 dnl If we found a libkafs, we have k_hasafs.  Set the appropriate
 dnl preprocessor define.  Otherwise, we'll use our portability layer.
 AS_IF([test x"$rra_libkafs" = xtrue],
    [AC_DEFINE([HAVE_K_HASAFS], 1,
        [Define to 1 if you have the k_hasafs function.])],
    [AS_CASE([$host],
        [*-linux*],
        [rra_build_kafs=true
         AC_CHECK_HEADERS([sys/ioccom.h])
         AC_DEFINE([HAVE_KAFS_LINUX], [1],
            [Define to 1 to use the Linux AFS /proc interface.])],

        [*-aix*|*-irix*],
        [_RRA_LIB_KAFS_LSETPAG],

        [*],
        [rra_build_kafs=true
         _RRA_LIB_KAFS_PATHS
         RRA_LIB_KAFS_SWITCH
         AC_CHECK_HEADERS([afs/param.h sys/ioccom.h])
         RRA_LIB_KAFS_RESTORE
         AC_DEFINE([HAVE_KAFS_SYSCALL], [1],
            [Define to 1 to use the AFS syscall interface.])
         AC_DEFINE([_REENTRANT], [1],
            [Define to 1 on Solaris for correct threaded errno handling.])])])
 AM_CONDITIONAL([NEED_KAFS], [test x"$rra_build_kafs" = xtrue])
 AC_SUBST([KAFS_SYSCALL_OBJECTS])])
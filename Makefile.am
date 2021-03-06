# Automake makefile for rra-c-util.
#
# Written by Russ Allbery <eagle@eyrie.org
# Copyright 2015-2018 Russ Allbery <eagle@eyrie.org>
# Copyright 2008-2014
#     The Board of Trustees of the Leland Stanford Junior University
#
# SPDX-License-Identifier: MIT

ACLOCAL_AMFLAGS = -I m4
EXTRA_DIST = .gitignore .travis.yml LICENSE README.md bootstrap		    \
	docs/api/xmalloc.3 docs/api/xmalloc.pod docs/metadata m4/apache.m4  \
	m4/apr.m4 m4/aprutil.m4 m4/curl.m4 m4/ld-version.m4 m4/ldap.m4	    \
	m4/openssl.m4 m4/pcre.m4 m4/remctl.m4 m4/tinycdb.m4		    \
	portable/winsock.c perl/t/data/perl.conf perl/t/docs/pod-coverage.t \
	perl/t/docs/pod-spelling.t perl/t/docs/pod.t perl/t/docs/synopsis.t \
	perl/t/docs/spdx-license.t perl/t/style/coverage.t		    \
	perl/t/style/critic.t perl/t/style/minimum-version.t		    \
	perl/t/style/module-version.t perl/t/style/obsolete-strings.t	    \
	perl/t/style/strict.t tests/README tests/TESTS			    \
	tests/data/cppcheck.supp tests/data/generate-krb5-conf		    \
	tests/data/krb5-pam.conf tests/data/perl.conf			    \
	tests/data/perlcriticrc tests/data/perltidyrc			    \
	tests/data/valgrind.supp tests/docs/pod-spelling-t tests/docs/pod-t \
	tests/docs/spdx-license-t tests/fakepam/README tests/kafs/basic-t   \
	tests/perl/critic-t tests/perl/minimum-version-t		    \
	tests/perl/module-version-t tests/perl/strict-t			    \
	tests/style/obsolete-strings-t tests/tap/kerberos.sh		    \
	tests/tap/libtap.sh tests/tap/perl/Test/RRA.pm			    \
	tests/tap/perl/Test/RRA/Automake.pm				    \
	tests/tap/perl/Test/RRA/Config.pm				    \
	tests/tap/perl/Test/RRA/ModuleVersion.pm tests/tap/remctl.sh	    \
	tests/util/xmalloc-t tests/valgrind/logs-t

# Enable kafs support when running distcheck.
DISTCHECK_CONFIGURE_FLAGS = --enable-kafs

# This is unfortunately required since we have an object (k_hasafs) defined
# in LIBOBJS that needs the kafs headers.  There isn't any way in Automake
# that I can find to define additional CPPFLAGS for a specific LIBOBJS
# member.
AM_CPPFLAGS = $(KAFS_CPPFLAGS)

noinst_LIBRARIES = pam-util/libpamutil.a portable/libportable.a util/libutil.a
pam_util_libpamutil_a_SOURCES = pam-util/args.c pam-util/args.h		\
	pam-util/logging.c pam-util/logging.h pam-util/options.c	\
	pam-util/options.h pam-util/vector.c pam-util/vector.h
pam_util_libpamutil_a_CPPFLAGS = $(KRB5_CPPFLAGS)
portable_libportable_a_SOURCES = portable/apr.h portable/dummy.c	\
	portable/event.h portable/getaddrinfo.h portable/getnameinfo.h	\
	portable/getopt.h portable/gssapi.h portable/kadmin.h		\
	portable/kafs.h portable/krb5-extra.c portable/krb5.h		\
	portable/macros.h portable/pam.h portable/sd-daemon.h		\
	portable/socket.h portable/socket-unix.h portable/statvfs.h	\
	portable/stdbool.h portable/system.h portable/uio.h
portable_libportable_a_CPPFLAGS = $(KRB5_CPPFLAGS) $(LIBEVENT_CPPFLAGS)
portable_libportable_a_LIBADD = $(LIBOBJS)
util_libutil_a_SOURCES = util/buffer.c util/buffer.h util/fdflag.c	    \
	util/fdflag.h util/macros.h util/messages-krb5.c		    \
	util/messages-krb5.h util/messages.c util/messages.h util/network.c \
	util/network.h util/vector.c util/vector.h util/xmalloc.c	    \
	util/xmalloc.h util/xwrite.c util/xwrite.h
util_libutil_a_CPPFLAGS = $(KRB5_CPPFLAGS)

# Conditionally build the replacement kafs library.
if NEED_KAFS
    noinst_LIBRARIES += kafs/libkafs.a
    EXTRA_kafs_libkafs_a_SOURCES = kafs/sys-darwin10.c kafs/sys-darwin8.c \
	kafs/sys-linux.c kafs/sys-solaris.c kafs/sys-syscall.c
    kafs_libkafs_a_SOURCES = kafs/kafs.c
    kafs_libkafs_a_CPPFLAGS = $(KAFS_CPPFLAGS)
endif

# Work around the GNU Coding Standards, which leave all the Autoconf and
# Automake stuff around after make maintainer-clean, thus making that command
# mostly worthless.
DISTCLEANFILES = config.h.in~
MAINTAINERCLEANFILES = Makefile.in aclocal.m4 build-aux/ar-lib		\
        build-aux/compile build-aux/config.guess build-aux/config.sub	\
        build-aux/depcomp build-aux/install-sh build-aux/missing	\
        config.h.in configure

# Remove the Autoconf cache directory on make distclean.
distclean-local:
	rm -rf autom4te.cache

# Separate target for a human to request building everything with as many
# compiler warnings enabled as possible.
warnings:
	$(MAKE) V=0 CFLAGS='$(WARNINGS_CFLAGS) $(AM_CFLAGS)' \
	    KRB5_CPPFLAGS='$(KRB5_CPPFLAGS_GCC)'
	$(MAKE) V=0 CFLAGS='$(WARNINGS_CFLAGS) $(AM_CFLAGS)' \
	    KRB5_CPPFLAGS='$(KRB5_CPPFLAGS_GCC)' $(check_PROGRAMS)

# The bits below are for the test suite, not for the main package.
check_PROGRAMS = tests/runtests tests/kafs/basic tests/kafs/haspag-t	   \
	tests/pam-util/args-t tests/pam-util/fakepam-t			   \
	tests/pam-util/logging-t tests/pam-util/options-t		   \
	tests/pam-util/vector-t tests/portable/asprintf-t		   \
	tests/portable/daemon-t tests/portable/getaddrinfo-t		   \
	tests/portable/getnameinfo-t tests/portable/getopt-t		   \
	tests/portable/inet_aton-t tests/portable/inet_ntoa-t		   \
	tests/portable/inet_ntop-t tests/portable/mkstemp-t		   \
	tests/portable/reallocarray-t tests/portable/setenv-t		   \
	tests/portable/snprintf-t tests/portable/strndup-t		   \
	tests/util/buffer-t tests/util/fdflag-t tests/util/messages-t	   \
	tests/util/messages-krb5-t tests/util/network/addr-ipv4-t	   \
	tests/util/network/addr-ipv6-t tests/util/network/client-t	   \
	tests/util/network/server-t tests/util/vector-t tests/util/xmalloc \
	tests/util/xwrite-t
tests_runtests_CPPFLAGS = -DC_TAP_SOURCE='"$(abs_top_srcdir)/tests"' \
	-DC_TAP_BUILD='"$(abs_top_builddir)/tests"'
check_LIBRARIES = tests/fakepam/libfakepam.a tests/tap/libtap.a
tests_fakepam_libfakepam_a_CPPFLAGS = $(KRB5_CPPFLAGS)
tests_fakepam_libfakepam_a_SOURCES = tests/fakepam/config.c		   \
	tests/fakepam/data.c tests/fakepam/general.c			   \
	tests/fakepam/internal.h tests/fakepam/kuserok.c		   \
	tests/fakepam/logging.c tests/fakepam/pam.h tests/fakepam/script.c \
	tests/fakepam/script.h
tests_tap_libtap_a_CPPFLAGS = $(KADM5CLNT_CPPFLAGS) $(KRB5_CPPFLAGS)
tests_tap_libtap_a_SOURCES = tests/tap/basic.c tests/tap/basic.h	\
	tests/tap/kadmin.c tests/tap/kadmin.h tests/tap/kerberos.c	\
	tests/tap/kerberos.h tests/tap/macros.h tests/tap/messages.c	\
	tests/tap/messages.h tests/tap/process.c tests/tap/process.h	\
	tests/tap/remctl.c tests/tap/remctl.h tests/tap/string.c	\
	tests/tap/string.h

# kafs tests are buit differently depending on whether we use our local
# libkafs replacement.
tests_kafs_basic_CPPFLAGS = $(KAFS_CPPFLAGS)
tests_kafs_basic_LDFLAGS = $(KAFS_LDFLAGS)
tests_kafs_haspag_t_CPPFLAGS = $(KAFS_CPPFLAGS)
tests_kafs_haspag_t_LDFLAGS = $(KAFS_LDFLAGS)
if NEED_KAFS
    tests_kafs_basic_LDADD = kafs/libkafs.a portable/libportable.a \
	$(KAFS_LIBS)
    tests_kafs_haspag_t_LDADD = kafs/libkafs.a tests/tap/libtap.a \
	portable/libportable.a $(KAFS_LIBS)
else
    tests_kafs_basic_LDADD = portable/libportable.a $(KAFS_LIBS)
    tests_kafs_haspag_t_LDADD = tests/tap/libtap.a portable/libportable.a \
	$(KAFS_LIBS)
endif

# All of the other test programs.
tests_pam_util_args_t_LDFLAGS = $(KRB5_LDFLAGS)
tests_pam_util_args_t_LDADD = pam-util/libpamutil.a	\
	tests/fakepam/libfakepam.a tests/tap/libtap.a	\
	portable/libportable.a $(KRB5_LIBS)
tests_pam_util_fakepam_t_LDADD = tests/fakepam/libfakepam.a \
	tests/tap/libtap.a portable/libportable.a
tests_pam_util_logging_t_LDFLAGS = $(KRB5_LDFLAGS)
tests_pam_util_logging_t_LDADD = pam-util/libpamutil.a	\
	tests/fakepam/libfakepam.a tests/tap/libtap.a	\
	portable/libportable.a $(KRB5_LIBS)
tests_pam_util_options_t_LDFLAGS = $(KRB5_LDFLAGS)
tests_pam_util_options_t_LDADD = pam-util/libpamutil.a	\
	tests/fakepam/libfakepam.a tests/tap/libtap.a	\
	portable/libportable.a $(KRB5_LIBS)
tests_pam_util_vector_t_LDADD = pam-util/libpamutil.a	\
	tests/fakepam/libfakepam.a tests/tap/libtap.a	\
	portable/libportable.a
tests_portable_asprintf_t_SOURCES = tests/portable/asprintf-t.c \
	tests/portable/asprintf.c
tests_portable_asprintf_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_daemon_t_SOURCES = tests/portable/daemon-t.c \
	tests/portable/daemon.c
tests_portable_daemon_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_getaddrinfo_t_SOURCES = tests/portable/getaddrinfo-t.c \
	tests/portable/getaddrinfo.c
tests_portable_getaddrinfo_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_getnameinfo_t_SOURCES = tests/portable/getnameinfo-t.c \
	tests/portable/getnameinfo.c
tests_portable_getnameinfo_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a
tests_portable_getopt_t_SOURCES = tests/portable/getopt-t.c \
	tests/portable/getopt.c
tests_portable_getopt_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_inet_aton_t_SOURCES = tests/portable/inet_aton-t.c \
	tests/portable/inet_aton.c
tests_portable_inet_aton_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_inet_ntoa_t_SOURCES = tests/portable/inet_ntoa-t.c \
	tests/portable/inet_ntoa.c
tests_portable_inet_ntoa_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_inet_ntop_t_SOURCES = tests/portable/inet_ntop-t.c \
	tests/portable/inet_ntop.c
tests_portable_inet_ntop_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_mkstemp_t_SOURCES = tests/portable/mkstemp-t.c \
	tests/portable/mkstemp.c
tests_portable_mkstemp_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_reallocarray_t_SOURCES = tests/portable/reallocarray-t.c \
	tests/portable/reallocarray.c
tests_portable_reallocarray_t_LDADD = tests/tap/libtap.a \
	portable/libportable.a
tests_portable_setenv_t_SOURCES = tests/portable/setenv-t.c \
	tests/portable/setenv.c
tests_portable_setenv_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_snprintf_t_SOURCES = tests/portable/snprintf-t.c \
	tests/portable/snprintf.c
tests_portable_snprintf_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_portable_strndup_t_SOURCES = tests/portable/strndup-t.c \
	tests/portable/strndup.c
tests_portable_strndup_t_LDADD = tests/tap/libtap.a portable/libportable.a
tests_util_buffer_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a
tests_util_fdflag_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a
tests_util_messages_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a
tests_util_messages_krb5_t_CPPFLAGS = $(KRB5_CPPFLAGS)
tests_util_messages_krb5_t_LDFLAGS = $(KRB5_LDFLAGS)
tests_util_messages_krb5_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a $(KRB5_LIBS)
tests_util_network_addr_ipv4_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a
tests_util_network_addr_ipv6_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a
tests_util_network_client_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a
tests_util_network_server_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a
tests_util_vector_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a
tests_util_xmalloc_LDADD = util/libutil.a portable/libportable.a
tests_util_xwrite_t_SOURCES = tests/util/fakewrite.c tests/util/fakewrite.h \
	tests/util/xwrite.c tests/util/xwrite-t.c
tests_util_xwrite_t_LDADD = tests/tap/libtap.a util/libutil.a \
	portable/libportable.a

check-local: $(check_PROGRAMS)
	tests/runtests -l '$(abs_top_srcdir)/tests/TESTS'

# Used by maintainers to check the source code with cppcheck.
check-cppcheck:
	cd $(abs_top_srcdir) && cppcheck -q --error-exitcode=2	\
	    --suppressions-list=tests/data/cppcheck.supp	\
	    --enable=warning,performance,portability,style .

# The full path to valgrind and its options, used when doing valgrind
# testing.
VALGRIND_COMMAND = $(PATH_VALGRIND) --leak-check=full			\
	--trace-children=yes --trace-children-skip=/bin/sh		\
	--log-file=$(abs_top_builddir)/tests/tmp/valgrind/log.%p

# Used by maintainers to run the main test suite under valgrind.  Suppress
# the xmalloc and pod-spelling tests because the former won't work properly
# under valgrind (due to increased memory usage) and the latter is pointless
# to run under valgrind.
check-valgrind: $(check_PROGRAMS)
	rm -rf $(abs_top_builddir)/tests/tmp
	mkdir $(abs_top_builddir)/tests/tmp
	mkdir $(abs_top_builddir)/tests/tmp/valgrind
	C_TAP_VALGRIND="$(VALGRIND_COMMAND)" tests/runtests \
	    -l '$(abs_top_srcdir)/tests/TESTS'

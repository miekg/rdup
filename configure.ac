#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(rdup, 1.1.16, miek@miek.nl, rdup)
AC_CONFIG_SRCDIR([rdup.c])
AC_PREFIX_DEFAULT(/usr/local)

sinclude(glib-2.0.m4)
AC_AIX
AC_PROG_CC
AC_PROG_MAKE_SET

AM_PATH_GLIB_2_0(2.0.0,,AC_MSG_ERROR([** No glib2-dev library found.]))

# dirfd stuff
AC_CHECK_FUNCS([dirfd])
AC_CHECK_MEMBERS([DIR.d_fd],,,  [[#include <dirent.h>]])
AC_CHECK_MEMBERS([DIR.dd_fd],,, [[#include <dirent.h>]])

AC_CHECK_HEADERS([getopt.h dirent.h sys/vfs.h sys/statvfs.h sys/sysmacros.h])
AC_CHECK_HEADERS(sys/param.h sys/mount.h,,,
[
  [
   #if HAVE_SYS_PARAM_H
   # include <sys/param.h>
   #endif
  ]
])

AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug], [Turn on debugging]))

if test "x$enable_debug" = "xyes"; then
    CFLAGS="${CFLAGS} -g -DDEBUG"
    DEBUG=yes
    AC_SUBST(DEBUG)
fi

# post 1.0, this can be turned on
# --disable-nls if you do not need it
#AM_GNU_GETTEXT([external])
#AM_GNU_GETTEXT_VERSION

AC_CHECK_LIB(archive, archive_entry_copy_symlink, ,ARCHIVE_L="no")
if test "$ARCHIVE_L" = "no"; then
    AC_MSG_WARN([** No archive library found; not building rdup-tr.])
else
    :
    AC_SUBST(HAVE_LIBARCHIVE)
    AC_SUBST(ARCHIVE_L)
fi

AC_ARG_WITH(libpcre_includes,
       [  --with-libpcre-includes=DIR  libpcre include directory],
       [with_libpcre_includes="$withval"],[with_libpcre_includes=no])

AC_ARG_WITH(libpcre_libraries,
       [  --with-libpcre-libraries=DIR  libpcre library directory],
       [with_libpcre_libraries="$withval"],[with_libpcre_libraries=no])

if test "$with_libpcre_includes" != "no"; then
   CFLAGS="${CFLAGS} -I${with_libpcre_includes}"
else
   CFLAGS="${CFLAGS} `pcre2-config --cflags`"
fi

if test "$with_libpcre_libraries" != "no"; then
   LIBS="${LIBS} -L${with_libpcre_libraries}"
else
   LIBS="${LIBS} `pcre2-config --libs8`"
fi

# PCRE configuration (required)
# Verify that we have the headers
PCRE_H=""
AC_CHECK_HEADERS(pcre2.h,, PCRE_H="no", [#define PCRE2_CODE_UNIT_WIDTH 8])
if test "$PCRE_H" = "no"; then
	AC_MSG_ERROR([** No pcre2 header for library found.])
fi

# Verify that we have the library
PCRE_L=""
AC_CHECK_LIB(pcre2-8, pcre2_compile_8, , PCRE_L="no")
if test "$PCRE_L" = "no"; then
	AC_MSG_ERROR([** No pcre2 library found.])
fi

AC_ARG_WITH(libnettle_includes,
       [  --with-libnettle-includes=DIR  libnettle include directory],
       [with_libnettle_includes="$withval"],[with_libnettle_includes=no])

AC_ARG_WITH(libnettle_libraries,
       [  --with-libnettle-libraries=DIR  libnettle library directory],
       [with_libnettle_libraries="$withval"],[with_libnettle_libraries=no])

if test "$with_libnettle_includes" != "no"; then
   CFLAGS="${CFLAGS} -I${with_libnettle_includes}"
fi

if test "$with_libnettle_libraries" != "no"; then
   LIBS="${LIBS} -L${with_libnettle_libraries}"
fi

NETTLE_H=""
AC_CHECK_HEADERS(nettle/aes.h,, NETTLE_H="no")
if test "$NETTLE_H" = "no"; then
        AC_MSG_WARN([** No nettle library found.])
fi

# This fails on my Fedora 11 machine - just assume we
# can do aes...
NETTLE_L=""
AC_CHECK_LIB(nettle, nettle_aes_encrypt, ,NETTLE_L="no")
if test "$NETTLE_L" = "no"; then
    AC_MSG_WARN([** No nettle library found.])
fi
AC_SUBST(NETTLE_L)

# sysconfdir
if test ${sysconfdir} = '${prefix}/etc'; then
        sysconfdir='/etc'
fi

AC_CONFIG_FILES([GNUmakefile 
                 po/GNUmakefile
                 rdup.h
                 rdup-tr.h
                 rdup-up.h
                 doc/rdup.1
                 doc/rdup-tr.1
                 doc/rdup-up.1
                 doc/rdup-simple.1])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT

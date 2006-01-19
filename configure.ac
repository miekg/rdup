#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(rdup, 0.2.0, miek@miek.nl, rdup)
AC_CONFIG_SRCDIR([rdup.c])
AC_PREFIX_DEFAULT(/usr)

sinclude(glib-2.0.m4)

AC_AIX
# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for glib 
AM_PATH_GLIB_2_0(2.0.0)

AC_DEFUN([AC_CHECK_DELIM],
[AC_REQUIRE([AC_PROG_CC])
AC_MSG_CHECKING(for GNU getdelim)
AC_CACHE_VAL(ac_cv_c_delim,
[ac_cv_c_delim=no
AC_TRY_COMPILE(
[#define _GNU_SOURCE
#include <stdio.h>
void f(void) { FILE *fp; char *line; size_t len; (void)getdelim(&line, &len, '\n', fp); }],
[
        f();
],
[ac_cv_c_delim="yes"],
[ac_cv_c_delim="no"])
])

AC_MSG_RESULT($ac_cv_c_delim)
if test $ac_cv_c_delim = yes; then
        AC_DEFINE(HAVE_DELIM, 1, [for GNU getdelim])
fi
])dnl

AC_CHECK_DELIM



AC_CHECK_HEADERS([getopt.h sys/vfs.h sys/statvfs.h])
AC_CHECK_HEADERS(sys/param.h sys/mount.h,,,
[
  [
   #if HAVE_SYS_PARAM_H
   # include <sys/param.h>
   #endif
  ]
])

# /etc/rdup
if test $prefix = "NONE"; then
	if test $sysconfdir = '${prefix}/etc'; then
		sysconfdir="/etc"
	fi
fi

AC_CONFIG_FILES([Makefile
                 rdup.h
                 excl.sh 
                 mirror.sh 
                 rmirror.sh
                 rcat.sh
                 tools/hist.sh 
                 tools/yesterday.sh
                 doc/rmirror.sh.8
                 doc/rcat.sh.8
                 doc/mirror.sh.8
                 doc/rdup.8
                 doc/hist.sh.1
                 doc/yesterday.sh.1])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT

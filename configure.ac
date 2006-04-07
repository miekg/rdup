#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(rdup, 0.2.7, miek@miek.nl, rdup)
AC_CONFIG_SRCDIR([rdup.c])
AC_PREFIX_DEFAULT(/usr)

sinclude(glib-2.0.m4)

AC_AIX
# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for glib 
AM_PATH_GLIB_2_0(2.0.0)

AC_CHECK_FUNCS(getdelim) 

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
                 doc/restore.sh.8
                 doc/monthday.sh.8
                 doc/mirror.sh.8
                 doc/rdup.8
                 doc/dump.sh.8
                 doc/dump-here.sh.8
                 doc/purge.sh.8
                 doc/hist.sh.1
                 doc/yesterday.sh.1
                 doc/crypt.sh.1
                 doc/gzip.sh.1])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT

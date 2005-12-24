#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(rdump, 0.0.1, miek@miek.nl, rdump)
AC_CONFIG_SRCDIR([rdump.c])
AC_PREFIX_DEFAULT(/usr)

sinclude(glib-2.0.m4)

AC_AIX
# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

# Checks for glib 
AM_PATH_GLIB_2_0(2.0.0)

AC_CHECK_HEADERS([getopt.h sys/vfs.h sys/statvfs.h])
AC_CHECK_HEADERS(sys/param.h sys/mount.h,,,
[
  [
   #if HAVE_SYS_PARAM_H
   # include <sys/param.h>
   #endif
  ]
])

#if test $prefix = "NONE"; then
#	if test $sysconfdir = '${prefix}/etc'; then
#		sysconfdir="/etc"
#	fi
#fi
AC_CONFIG_FILES([Makefile
                 rdump.h])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT

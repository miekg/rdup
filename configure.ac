#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(rdup, 0.5.8, miek@miek.nl, rdup)
AC_CONFIG_SRCDIR([rdup.c])
AC_PREFIX_DEFAULT(/usr)

sinclude(glib-2.0.m4)
AC_AIX
AC_PROG_CC
AC_PROG_MAKE_SET

#  [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND 
AM_PATH_GLIB_2_0(2.0.0,,AC_MSG_ERROR([libglib2-dev not found !]))
AC_CHECK_FUNCS([dirfd openat attropen])
AC_CHECK_LIB(attr, lgetxattr)
AC_CHECK_HEADERS(attr/xattr.h,,,[AC_INCLUDES_DEFAULT])
AC_CHECK_HEADERS([getopt.h sys/vfs.h sys/statvfs.h])
AC_CHECK_HEADERS(sys/param.h sys/mount.h,,,
[
  [
   #if HAVE_SYS_PARAM_H
   # include <sys/param.h>
   #endif
  ]
])
AC_C_BIGENDIAN

# sysconfdir
if test ${sysconfdir} = '${prefix}/etc'; then
        sysconfdir='/etc'
fi

AC_CONFIG_FILES([GNUmakefile
                 rdup.h
                 doc/rdup.1
                 doc/rdup-cp.1
                 doc/rdup-restore.1
                 doc/rdup-snap-here.1
                 doc/rdup-snap-link.1
                 doc/rdup-snap.1
                 doc/rdup-simple.1
                 doc/rdup-crypt.1
                 doc/rdup-gzip.1
                 doc/rdup-gpg.1
                 doc/rdup-backups.1
                 doc/rdup-hist.1
                 shared/shared.sh
                 shared/shared.pl
                 sh-wrapper/rdup-snap-here
                 sh-wrapper/rdup-snap-link
                 sh-wrapper/rdup-restore
                 sh-wrapper/rdup-simple
                 pl-tools/rdup-gzip
                 pl-tools/rdup-crypt
                 pl-tools/rdup-snap
                 pl-tools/rdup-cp
                 pl-tools/rdup-gpg
                 pl-tools/rdup-hist
                 ])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT

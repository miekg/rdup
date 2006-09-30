#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(rdup, 0.3.30, miek@miek.nl, rdup)
AC_CONFIG_SRCDIR([rdup.c])
AC_PREFIX_DEFAULT(/usr)

sinclude(glib-2.0.m4)
sinclude(check_gnu_make.m4)

AC_AIX
AC_PROG_CC
AC_PROG_MAKE_SET

CHECK_GNU_MAKE
AM_PATH_GLIB_2_0(2.0.0)
AC_CHECK_LIB(attr, lgetxattr,, [
        AC_MSG_WARN([No xattr support. Not enabling -a])
        ]
)
AC_CHECK_HEADERS(attr/xattr.h,,,[AC_INCLUDES_DEFAULT])

AC_CHECK_FUNCS(getdelim) 
AC_CHECK_FUNCS(dirfd) 

AC_CHECK_HEADERS([getopt.h sys/vfs.h sys/statvfs.h])
AC_CHECK_HEADERS(sys/param.h sys/mount.h,,,
[
  [
   #if HAVE_SYS_PARAM_H
   # include <sys/param.h>
   #endif
  ]
])

AC_PATH_PROG(cstream, cstream)

# /etc/rdup
if test $prefix = "NONE"; then
	if test $sysconfdir = '${prefix}/etc'; then
		sysconfdir="/etc"
	fi
fi

AC_CONFIG_FILES([Makefile
                 rdup.h
                 doc/rdup.8
                 doc/dump.sh.8
                 doc/restore.sh.8
                 doc/dump-here.sh.8
                 doc/mirror.pl.8
                 doc/crypt.pl.1
                 doc/gzip.pl.1
                 doc/grep.pl.1
                 shared/shared.sh
                 shared/shared.pl
                 sh-wrapper/restore.sh
                 sh-wrapper/dump-here.sh
                 sh-wrapper/dump.sh
                 sh-wrapper/rdup-snapshot
                 sh-tools/crypt.sh
                 sh-tools/gzip.sh
                 sh-tools/mirror.sh
                 pl-tools/gzip.pl
                 pl-tools/crypt.pl
                 pl-tools/grep.pl
                 pl-tools/rdup-snap
                 pl-tools/mirror.pl
                 ])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(rdup, 0.6.7, miek@miek.nl, rdup)
AC_CONFIG_SRCDIR([rdup.c])
AC_PREFIX_DEFAULT(/usr)

sinclude(glib-2.0.m4)
AC_AIX
AC_PROG_CC
AC_PROG_MAKE_SET

AM_PATH_GLIB_2_0(2.0.0,,AC_MSG_ERROR([** No libglib2-dev found.]))
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION
AC_CHECK_FUNCS([dirfd])
AC_CHECK_HEADERS([getopt.h sys/vfs.h sys/statvfs.h archive.h])
AC_CHECK_HEADERS(sys/param.h sys/mount.h,,,
[
  [
   #if HAVE_SYS_PARAM_H
   # include <sys/param.h>
   #endif
  ]
])

AC_CHECK_LIB(archive, archive_write_new, ,ARCHIVE_L="no")
if test "$ARCHIVE_L" == "no"; then
    AC_MSG_WARN([** libarchive not found; not building rdup-tr.])
fi
AC_SUBST(ARCHIVE_L)

AC_ARG_WITH(libpcre_includes,
       [  --with-libpcre-includes=DIR  libpcre include directory],
       [with_libpcre_includes="$withval"],[with_libpcre_includes=no])

AC_ARG_WITH(libpcre_libraries,
       [  --with-libpcre-libraries=DIR  libpcre library directory],
       [with_libpcre_libraries="$withval"],[with_libpcre_libraries=no])

if test "$with_libpcre_includes" != "no"; then
   CFLAGS="${CFLAGS} -I${with_libpcre_includes}"
else
   CFLAGS="${CFLAGS} `pcre-config --cflags`"
fi

if test "$with_libpcre_libraries" != "no"; then
   LIBS="${LIBS} -L${with_libpcre_libraries}"
else
   LIBS="${LIBS} `pcre-config --libs`"
fi

# PCRE configuration (required)
# Verify that we have the headers
PCRE_H=""
AC_CHECK_HEADERS(pcre.h,, PCRE_H="no")
if test "$PCRE_H" = "no"; then
	AC_MSG_ERROR([** No pcre library found.])
fi

# Verify that we have the library
PCRE_L=""
AC_CHECK_LIB(pcre, pcre_compile, ,PCRE_L="no")
if test "$PCRE_L" = "no"; then
	AC_MSG_ERROR([** No pcre library found.])
fi

AC_C_BIGENDIAN

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
                 ])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT

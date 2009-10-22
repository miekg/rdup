#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(rdup, 1.1.0-dev0, miek@miek.nl, rdup)
AC_CONFIG_SRCDIR([rdup.c])
AC_PREFIX_DEFAULT(/usr)

sinclude(glib-2.0.m4)
AC_AIX
AC_PROG_CC
AC_PROG_MAKE_SET

AM_PATH_GLIB_2_0(2.0.0,,AC_MSG_ERROR([** No glib2-dev library found.]))
AC_CHECK_FUNCS([dirfd])
AC_CHECK_HEADERS([getopt.h sys/vfs.h sys/statvfs.h sys/sysmacros.h archive.h])
AC_CHECK_HEADERS(sys/param.h sys/mount.h,,,
[
  [
   #if HAVE_SYS_PARAM_H
   # include <sys/param.h>
   #endif
  ]
])

# post 1.0, this can be turned on
# --disable-nls if you do not need it
#AM_GNU_GETTEXT([external])
#AM_GNU_GETTEXT_VERSION

AC_CHECK_LIB(archive, archive_entry_copy_symlink, ,ARCHIVE_L="no")
if test "$ARCHIVE_L" = "no"; then
    AC_MSG_WARN([** No archive library found; not building rdup-tr.])
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

AC_ARG_WITH(openssl_includes,
       [  --with-openssl-includes=DIR  openssl include directory],
       [with_openssl_includes="$withval"],[with_openssl_includes=no])

AC_ARG_WITH(openssl_libraries,
       [  --with-openssl-libraries=DIR  openssl library directory],
       [with_openssl_libraries="$withval"],[with_openssl_libraries=no])

if test "$with_openssl_includes" != "no"; then
   CFLAGS="${CFLAGS} -I${with_openssl_includes}"
fi

if test "$with_openssl_libraries" != "no"; then
   LIBS="${LIBS} -L${with_openssl_libraries}"
fi

SSL_H=""
AC_CHECK_HEADERS(openssl/ssl.h,, SSL_H="no")
if test "$SSL_H" = "no"; then
	AC_MSG_ERROR([** No OpenSSL library found.])
fi

SSL_L=""
AC_CHECK_LIB(ssl, SHA1_Init, ,SSL_L="no")
if test "$SSL_L" = "no"; then
    AC_MSG_ERROR([** No OpenSSL library found.])
fi
AC_SUBST(SSL_L)
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
		 sh/rdup-simple
                 doc/rdup.1
                 doc/rdup-tr.1
                 doc/rdup-up.1
                 ])
AC_CONFIG_HEADER([config.h])
AC_OUTPUT

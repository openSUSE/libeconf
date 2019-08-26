AC_INIT([libeconf], [0.1.0])
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_SRCDIR([src/libeconf.c])
AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC
AC_GNU_SOURCE
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
LT_INIT([disable-static])

PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

dnl enable additional compiler checks
AC_ARG_ENABLE([compiler-checks],
    AS_HELP_STRING([--enable-compiler-checks],[enables additional compiler checks, not for production]))

if test x"$enable_compiler_checks" = x"yes" ; then
   CFLAGS_CHECKS="-Wall -O2 -fsanitize=address -fno-omit-frame-pointer -g"
   LDFLAGS_CHECKS="-fsanitize=address"
fi


AC_SUBST(CFLAGS_CHECKS)
AC_SUBST(LDFLAGS_CHECKS)
AC_CONFIG_FILES([Makefile lib/Makefile include/Makefile bin/Makefile tests/Makefile lib/libeconf.pc])
AC_OUTPUT

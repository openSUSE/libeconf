AC_INIT([libeconf], [0.1.0])
AC_SUBST(PACKAGE)
AC_SUBST(VERSION)

AM_INIT_AUTOMAKE([subdir-objects])

AC_CONFIG_SRCDIR([src/libeconf.c])
AC_CONFIG_MACRO_DIRS([m4])

AC_PROG_CC
AC_PROG_INSTALL
LT_INIT([disable-static])

PKG_PROG_PKG_CONFIG
PKG_INSTALLDIR

AC_CONFIG_FILES([Makefile lib/Makefile include/Makefile bin/Makefile lib/libeconf.pc])
AC_OUTPUT

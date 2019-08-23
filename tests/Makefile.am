
AM_CFLAGS = -I$(top_srcdir)/include -DTESTSDIR=\"$(top_srcdir)/tests/\"
LDADD = $(top_builddir)/lib/libeconf.la

CLEANFILES = *~

EXTRA_DIST = tst-arguments-data tst-logindefs1-data tst-merge1-data \
	tst-merge2-data tst-getconfdir1-data

TESTS = tst-filedoesnotexit1 tst-merge1 tst-merge2 tst-logindefs1 \
	tst-arguments1 tst-arguments2 tst-arguments3 tst-arguments4 \
	tst-getconfdirs1 tst-getconfdirs2

check_PROGRAMS = ${TESTS}

tst_getconfdirs1_CFLAGS = $(AM_CFLAGS) -DSUFFIX=\".conf\"
tst_getconfdirs2_CFLAGS = $(AM_CFLAGS) -DSUFFIX=\"conf\"

tst_getconfdirs2_SOURCES = tst-getconfdirs1.c

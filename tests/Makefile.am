
AM_CFLAGS = @CFLAGS_CHECKS@ @CFLAGS_WARNINGS@ -I$(top_srcdir)/include -DTESTSDIR=\"$(top_srcdir)/tests/\"
LDADD = @LDFLAGS_CHECKS@ @LDFLAGS_WARNINGS@ $(top_builddir)/lib/libeconf.la

CLEANFILES = *~

EXTRA_DIST = tst-arguments-data tst-logindefs1-data tst-merge1-data \
	tst-merge2-data tst-getconfdirs1-data tst-getconfdirs3-data \
	tst-getconfdirs4-data tst-getconfdirs5-data tst-getconfdirs6-data \
	tst-arguments5-data tst-groups3-data tst-parseconfig-data

TESTS = tst-filedoesnotexit1 tst-merge1 tst-merge2 tst-logindefs1 \
	tst-arguments1 tst-arguments2 tst-arguments3 tst-arguments4 \
	tst-arguments5 \
	tst-getconfdirs1 tst-getconfdirs2 tst-getconfdirs3 \
	tst-getconfdirs4 tst-getconfdirs5 tst-getconfdirs6 \
	tst-econf_errstring1 \
	tst-setgetvalues1 \
	tst-groups1 tst-groups2 tst-groups3 tst-groups4 \
	tst-parseconfig1

XFAIL_TESTS = tst-groups3 tst-parseconfig1

check_PROGRAMS = ${TESTS}

tst_getconfdirs1_CFLAGS = $(AM_CFLAGS) -DSUFFIX=\".conf\"
tst_getconfdirs2_CFLAGS = $(AM_CFLAGS) -DSUFFIX=\"conf\"

tst_getconfdirs2_SOURCES = tst-getconfdirs1.c

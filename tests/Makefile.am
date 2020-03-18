
AM_CFLAGS = @CFLAGS_CHECKS@ @CFLAGS_WARNINGS@ -I$(top_srcdir)/include -DTESTSDIR=\"$(top_srcdir)/tests/\"
LDADD = @LDFLAGS_CHECKS@ @LDFLAGS_WARNINGS@ $(top_builddir)/lib/libeconf.la

CLEANFILES = *~

check_SCRIPTS = tst-econftool_show1.sh tst-econftool1.sh

EXTRA_DIST = tst-arguments-data tst-logindefs1-data tst-logindefs2-data \
	tst-merge1-data tst-merge2-data tst-merge3-data tst-merge4-data \
	tst-merge5-data \
	tst-getconfdirs1-data tst-getconfdirs3-data \
	tst-getconfdirs4-data tst-getconfdirs5-data tst-getconfdirs6-data \
	tst-getconfdirs7-data \
	tst-arguments5-data tst-groups3-data tst-parseconfig-data \
	tst-quote1-data \
	tst-econftool-data $(check_SCRIPTS)

check_PROGRAMS = tst-filedoesnotexit1 tst-merge1 tst-merge2 tst-merge3 tst-merge4 \
	tst-merge5 \
	tst-logindefs1 tst-logindefs2 \
	tst-arguments1 tst-arguments2 tst-arguments3 tst-arguments4 \
	tst-arguments5 \
	tst-getconfdirs1 tst-getconfdirs2 tst-getconfdirs3 \
	tst-getconfdirs4 tst-getconfdirs5 tst-getconfdirs6 \
	tst-getconfdirs7 \
	tst-econf_errstring1 \
	tst-setgetvalues1 \
	tst-groups1 tst-groups2 tst-groups3 tst-groups4 \
	tst-parseconfig1 \
	tst-quote1

XFAIL_TESTS =

TESTS = $(check_SCRIPTS) ${check_PROGRAMS}

tst_getconfdirs1_CFLAGS = $(AM_CFLAGS) -DSUFFIX=\".conf\"
tst_getconfdirs2_CFLAGS = $(AM_CFLAGS) -DSUFFIX=\"conf\"

tst_getconfdirs2_SOURCES = tst-getconfdirs1.c

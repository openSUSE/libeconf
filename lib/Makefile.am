lib_LTLIBRARIES = libeconf.la
libeconf_la_SOURCES = ../src/libeconf.c ../src/getfilecontents.c ../src/mergefiles.c ../src/helpers.c ../src/keyfile.c ../src/econf_errString.c ../src/get_value_def.c
libeconf_la_CFLAGS = -D_REENTRANT=1 @CFLAGS_CHECKS@ @CFLAGS_WARNINGS@
libeconf_la_CPPFLAGS = -I$(top_srcdir)/include
libeconf_la_LDFLAGS = @LDFLAGS_CHECKS@ @CFLAGS_WARNINGS@ \
	-version-info 0:0:0 -Wl,--no-undefined \
	-Wl,--version-script=$(top_srcdir)/lib/libeconf.map
libeconf_la_LIBADD = -lm

CLEANFILES = *~
DISTCLEANFILES = *.pc
EXTRA_DIST = libeconf.map


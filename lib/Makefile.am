lib_LTLIBRARIES = libeconf.la
libeconf_la_SOURCES = libeconf.c getfilecontents.c mergefiles.c \
		      helpers.c keyfile.c econf_errString.c get_value_def.c
libeconf_la_CFLAGS = -D_REENTRANT=1 @CFLAGS_CHECKS@ @CFLAGS_WARNINGS@
libeconf_la_CPPFLAGS = -I$(top_srcdir)/include
libeconf_la_LDFLAGS = @LDFLAGS_CHECKS@ @CFLAGS_WARNINGS@ \
	-version-info 3:0:3 -Wl,--no-undefined \
	-Wl,--version-script=$(top_srcdir)/lib/libeconf.map
libeconf_la_LIBADD = -lm

CLEANFILES = *~
DISTCLEANFILES = *.pc
EXTRA_DIST = libeconf.map
pkgconfig_DATA = libeconf.pc

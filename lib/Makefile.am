lib_LTLIBRARIES = libeconf.la
libeconf_la_SOURCES = ../src/libeconf.c ../src/getfilecontents.c ../src/mergefiles.c ../src/helpers.c ../src/keyfile.c ../src/econf_errString.c
libeconf_la_CFLAGS = -Wall -I$(top_srcdir)/include
libeconf_la_LDFLAGS = -Wl,--version-script=$(top_srcdir)/lib/libeconf.map -lm

DISTCLEANFILES = *.pc
EXTRA_DIST = libeconf.map

install-exec-hook:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=execute "rm -f $(DESTDIR)$(libdir)/$(lib_LTLIBRARIES)";

uninstall-libLTLIBRARIES:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=execute "rm -f $(DESTDIR)$(libdir)/libeconf.so*";

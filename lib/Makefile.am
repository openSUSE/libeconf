lib_LTLIBRARIES = libeconf.la
libeconf_la_SOURCES = ../src/libeconf.c ../src/getfilecontents.c ../src/mergefiles.c ../src/helpers.c ../src/keyfile.c
libeconf_la_CFLAGS = -Wall
libeconf_la_LDFLAGS = -lm

CLEANFILES = *.pc

install-exec-hook:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=execute "rm -f $(DESTDIR)$(libdir)/$(lib_LTLIBRARIES)";

uninstall-libLTLIBRARIES:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=execute "rm -f $(DESTDIR)$(libdir)/libeconf.so*";

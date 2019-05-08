lib_LTLIBRARIES = libeconf.la
libeconf_la_SOURCES = ../src/libeconf.c
libeconf_la_CFLAGS = -Wall

CLEANFILES = *.pc

install-exec-hook:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=execute "rm -f $(DESTDIR)$(libdir)/$(lib_LTLIBRARIES)";

uninstall-libLTLIBRARIES:
	$(LIBTOOL) $(AM_LIBTOOLFLAGS) $(LIBTOOLFLAGS) --mode=execute "rm -f $(DESTDIR)$(libdir)/libeconf.so*";

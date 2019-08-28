noinst_PROGRAMS = example
example_SOURCES = ../example/example.c
example_CFLAGS = @CFLAGS_CHECKS@ @CFLAGS_WARNINGS@ -I $(top_srcdir)/include
example_LDADD = @LDFLAGS_CHECKS@ @LDFLAGS_WARNINGS@ $(top_builddir)/lib/libeconf.la

CLEANFILES = example

MAINTAINERCLEANFILES = Makefile.in

EDJE_CC = edje_cc
EDJE_FLAGS = -v -id $(top_srcdir)/data -fd $(top_srcdir)/data

filesdir = $(datadir)/wall-e/data
files_DATA = default.edj

EXTRA_DIST = \
default.edc

default.edj: Makefile $(EXTRA_DIST)
	$(EDJE_CC) $(EDJE_FLAGS) \
	$(top_srcdir)/data/theme/default/default.edc \
	$(top_builddir)/data/theme/default/default.edj

clean-local:
	rm -f *.edj

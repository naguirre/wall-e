MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS = wall-e

wall_e_SOURCES = \
main.c \
wall.c \
walle.c

wall_e_CPPFLAGS = \
-I$(top_srcdir) \
-DPACKAGE_DATA_DIR=\"$(datadir)/$(PACKAGE)\" \
@WALLE_CFLAGS@

wall_e_LDADD = @WALLE_LIBS@

wall_e_LDFLAGS = @lt_enable_auto_import@ -Wl,--as-needed

EXTRA_DIST = wall.h wall-e.h

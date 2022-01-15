## Process this file with automake to produce Makefile.in
plugindir=$(prefix)/lib/cssed/plugins
plugin_PROGRAMS=pythonscript.so

#the core pplugin files
SRCS=\
pythonscript.c pythonscript.h \
cssed-pyobject.c cssed-pyobject.h \
pycssed-constants-table.c pycssed-constants-table.h

if ISDARWIN
pythonscript_so_SOURCES = $(SRCS)
pythonscript_so_LDFLAGS =  -bundle -flat_namespace -undefined suppress
pythonscript_so_LDADD = @PACKAGE_LIBS@
pythonscript_so_CFLAGS = -fno-common
else
pythonscript_so_SOURCES= $(SRCS)
pythonscript_so_LDFLAGS = -shared -fPICS
pythonscript_so_LDADD = @PACKAGE_LIBS@
endif

INCLUDES=\
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/share/locale"\" \
	-DPACKAGE_DATA_DIR=\""$(datadir)/pythonscript-plugin"\" \
	-DPACKAGE_LIB_DIR=\""$(prefix)/lib/cssed/"\" \
	-I$(top_srcdir) -I$(top_srcdir)/src \
	@PACKAGE_CFLAGS@



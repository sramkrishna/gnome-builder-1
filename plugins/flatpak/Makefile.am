if ENABLE_FLATPAK_PLUGIN

DISTCLEANFILES =
BUILT_SOURCES =
CLEANFILES =
EXTRA_DIST = $(plugin_DATA)

plugindir = $(libdir)/gnome-builder/plugins
plugin_LTLIBRARIES = libflatpak-plugin.la
dist_plugin_DATA = flatpak.plugin

libflatpak_plugin_la_SOURCES = \
	gbp-flatpak-application-addin.c \
	gbp-flatpak-application-addin.h \
	gbp-flatpak-clone-widget.c \
	gbp-flatpak-clone-widget.h \
	gbp-flatpak-configuration.c \
	gbp-flatpak-configuration.h \
	gbp-flatpak-configuration-provider.c \
	gbp-flatpak-configuration-provider.h \
	gbp-flatpak-genesis-addin.c \
	gbp-flatpak-genesis-addin.h \
	gbp-flatpak-pipeline-addin.c \
	gbp-flatpak-pipeline-addin.h \
	gbp-flatpak-preferences-addin.c \
	gbp-flatpak-preferences-addin.h \
	gbp-flatpak-plugin.c \
	gbp-flatpak-runner.c \
	gbp-flatpak-runner.h \
	gbp-flatpak-runtime-provider.c \
	gbp-flatpak-runtime-provider.h \
	gbp-flatpak-runtime.c \
	gbp-flatpak-runtime.h \
	gbp-flatpak-sources.c \
	gbp-flatpak-sources.h \
	gbp-flatpak-subprocess-launcher.c \
	gbp-flatpak-subprocess-launcher.h \
	gbp-flatpak-transfer.c \
	gbp-flatpak-transfer.h \
	gbp-flatpak-util.c \
	gbp-flatpak-util.h \
	$(NULL)

nodist_libflatpak_plugin_la_SOURCES = \
	gbp-flatpak-resources.c \
	gbp-flatpak-resources.h

libflatpak_plugin_la_CFLAGS = $(PLUGIN_CFLAGS) $(FLATPAK_CFLAGS) $(GIT_CFLAGS) $(SOUP_CFLAGS)
libflatpak_plugin_la_LIBADD = $(FLATPAK_LIBS) $(GIT_LIBS) $(SOUP_LIBS)
libflatpak_plugin_la_LDFLAGS = $(PLUGIN_LDFLAGS)

glib_resources_c = gbp-flatpak-resources.c
glib_resources_h = gbp-flatpak-resources.h
glib_resources_xml = gbp-flatpak-resources.gresource.xml
glib_resources_namespace = gbp_flatpak
include $(top_srcdir)/build/autotools/Makefile.am.gresources

include $(top_srcdir)/plugins/Makefile.plugin

endif

-include $(top_srcdir)/git.mk

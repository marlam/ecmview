# Copyright (C) 2011, 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/base \
	-I$(top_srcdir)/src/uuid \
	-I$(top_srcdir)/src/glvm \
	-I$(top_srcdir)/src/download \
	-I$(top_srcdir)/src/cache \
	-I$(top_srcdir)/src/xgl \
	-I$(top_srcdir)/src/state \
	-I$(top_srcdir)/src/processor \
	-I$(top_srcdir)/src/renderer \
	-I$(top_srcdir)/src/gui \
        -I$(top_srcdir)/src/eq \
	$(libecmdb_CFLAGS) \
	$(libglewmx_CFLAGS)

bin_PROGRAMS = ecmview

ecmview_SOURCES = main.cpp

ecmview_LDADD =
if HAVE_LIBEQUALIZER
ecmview_LDADD += ../eq/libeq.la ../dtrack/libdtrack.la
endif
ecmview_LDADD += \
	../gui/libgui.la \
	../renderer/librenderer.la \
	../processor/libprocessor.la \
	../state/libstate.la \
	../xgl/libxgl.la \
	../cache/libcache.la \
        ../download/libdownload.la \
	../uuid/libuuid.la \
	../base/libbase.la \
	$(libuuid_LIBS) \
	$(libecmdb_LIBS) \
	$(libgta_LIBS) \
	$(libcurl_LIBS) \
	$(libgl_LIBS) \
	$(libglu_LIBS) \
	$(libqtopengl_LIBS) \
	$(libglewmx_LIBS)
if HAVE_LIBEQUALIZER
ecmview_LDADD += $(libequalizer_LIBS)
endif

if W32
ecmview_SOURCES += $(top_srcdir)/src/logo/ecmview_logo.ico
.ico.o:
	$(AM_V_GEN)echo "IDI_ICON1 ICON DISCARDABLE \"$<\"" | $(WINDRES) -o $@
endif

iconsdatadir = $(datadir)/icons/hicolor
icons16dir = $(iconsdatadir)/16x16/apps
icons32dir = $(iconsdatadir)/32x32/apps
icons48dir = $(iconsdatadir)/48x48/apps
icons64dir = $(iconsdatadir)/64x64/apps
icons128dir = $(iconsdatadir)/128x128/apps
icons256dir = $(iconsdatadir)/256x256/apps
iconsscaldir = $(iconsdatadir)/scalable/apps
dist_icons16_DATA = $(top_srcdir)/src/logo/16x16/ecmview.png
dist_icons32_DATA = $(top_srcdir)/src/logo/32x32/ecmview.png
dist_icons48_DATA = $(top_srcdir)/src/logo/48x48/ecmview.png
dist_icons64_DATA = $(top_srcdir)/src/logo/64x64/ecmview.png
dist_icons128_DATA = $(top_srcdir)/src/logo/128x128/ecmview.png
dist_icons256_DATA = $(top_srcdir)/src/logo/256x256/ecmview.png
dist_iconsscal_DATA = $(top_srcdir)/src/logo/scalable/ecmview.svgz
update-icon-cache:
	test -n "$(DESTDIR)" -o -z "$(GTK_UPDATE_ICON_CACHE)" || (umask 022 && $(GTK_UPDATE_ICON_CACHE) -q -f -t "$(iconsdatadir)")

appdatadir = $(datadir)/applications
dist_appdata_DATA = ecmview.desktop
update-desktop-database:
	test -n "$(DESTDIR)" -o -z "$(UPDATE_DESKTOP_DATABASE)" || (umask 022 && $(UPDATE_DESKTOP_DATABASE) "$(appdatadir)")

install-data-hook: update-icon-cache update-desktop-database
uninstall-hook: update-icon-cache update-desktop-database


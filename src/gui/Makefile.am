# Copyright (C) 2011, 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

noinst_LTLIBRARIES = libgui.la

libgui_la_CPPFLAGS = \
	-I$(top_srcdir)/src/base \
	-I$(top_srcdir)/src/uuid \
	-I$(top_srcdir)/src/glvm \
	-I$(top_srcdir)/src/download \
	-I$(top_srcdir)/src/cache \
	-I$(top_srcdir)/src/xgl \
	-I$(top_srcdir)/src/state \
	-I$(top_srcdir)/src/processor \
	-I$(top_srcdir)/src/context \
	-I$(top_srcdir)/src/renderer \
	-I$(top_srcdir)/src/eq \
	$(libecmdb_CFLAGS) \
	$(libglewmx_CFLAGS) \
	$(libqtopengl_CFLAGS)

libgui_la_SOURCES = \
	auth_cb.h auth_cb.cpp \
	quaddebug.h quaddebug.cpp \
	guitools.h guitools.cpp \
	lens.h lens.cpp \
	light.h light.cpp \
	info.h info.cpp \
	renderprops.h renderprops.cpp \
	statistics.h statistics.cpp \
	guicontext.h guicontext.cpp \
	databases.h databases.cpp \
	mainwindow.h mainwindow.cpp \
	guimain.h guimain.cpp

nodist_libgui_la_SOURCES = \
	quaddebug-moc.cpp \
	guitools-moc.cpp \
	lens-moc.cpp \
        light-moc.cpp \
	info-moc.cpp \
	renderprops-moc.cpp \
	statistics-moc.cpp \
	guicontext-moc.cpp \
	databases-moc.cpp \
	mainwindow-moc.cpp \
	resources-rcc.cpp

BUILT_SOURCES = $(nodist_libgui_la_SOURCES)

EXTRA_DIST = resources.qrc

CLEANFILES = $(BUILT_SOURCES)

moc_verbose = $(moc_verbose_$(V))
moc_verbose_ = $(moc_verbose_$(AM_DEFAULT_VERBOSITY))
moc_verbose_0 = @echo "  MOC   " $@;

%-moc.cpp: %.h
	$(moc_verbose)$(MOC) -o $@ $<

rcc_verbose = $(rcc_verbose_$(V))
rcc_verbose_ = $(rcc_verbose_$(AM_DEFAULT_VERBOSITY))
rcc_verbose_0 = @echo "  RCC   " $@;

%-rcc.cpp: %.qrc
	$(rcc_verbose)$(RCC) -o $@ $<

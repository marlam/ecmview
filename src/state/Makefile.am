# Copyright (C) 2011, 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

noinst_LTLIBRARIES = libstate.la

libstate_la_CPPFLAGS = \
	-I$(top_srcdir)/src/base \
	-I$(top_srcdir)/src/uuid \
	-I$(top_srcdir)/src/glvm \
	-I$(top_srcdir)/src/download \
	$(libecmdb_CFLAGS)

libstate_la_SOURCES = \
	processing_parameters.cpp \
	processing_parameters.h \
	lens_parameters.cpp \
	lens_parameters.h \
	light_parameters.cpp \
	light_parameters.h \
	renderer_parameters.cpp \
	renderer_parameters.h \
	database_description.cpp \
	database_description.h \
        auth_callback.h \
        auth_callback.cpp \
	state.cpp \
	state.h

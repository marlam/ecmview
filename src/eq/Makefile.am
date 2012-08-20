# Copyright (C) 2011, 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

include $(top_srcdir)/build-aux/glsl.mk

noinst_LTLIBRARIES = libeq.la

libeq_la_CPPFLAGS = \
	-I$(top_srcdir)/src/dtrack \
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
	$(libequalizer_CFLAGS) \
	$(libglewmx_CFLAGS) \
	$(libecm_CFLAGS)

libeq_la_SOURCES = \
	tracking.h tracking.cpp \
	tracking-dtrack.h tracking-dtrack.cpp \
	tracking-eqevent.h tracking-eqevent.cpp \
	warping.h warping.cpp \
	eqcontext.h eqcontext.cpp

EXTRA_DIST = warping.glsl
nodist_libeq_la_SOURCES = warping.glsl.h
BUILT_SOURCES = warping.glsl.h

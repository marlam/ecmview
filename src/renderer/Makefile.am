# Copyright (C) 2011, 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

include $(top_srcdir)/build-aux/glsl.mk

noinst_LTLIBRARIES = librenderer.la

librenderer_la_CPPFLAGS = \
	-I$(top_srcdir)/src/base \
	-I$(top_srcdir)/src/uuid \
	-I$(top_srcdir)/src/glvm \
	-I$(top_srcdir)/src/download \
	-I$(top_srcdir)/src/cache \
	-I$(top_srcdir)/src/xgl \
	-I$(top_srcdir)/src/state \
	-I$(top_srcdir)/src/processor \
	$(libecm_CFLAGS) \
	$(libglewmx_CFLAGS)

librenderer_la_SOURCES = \
	renderer-context.h renderer-context.cpp \
	navigator.h navigator.cpp \
	renderer.h renderer.cpp \
	terrain.h terrain.cpp \
        culler.h culler.cpp \
        lod.h lod.cpp

GLSL_SHADERS = \
        approx.fs.glsl \
        approx-minmax-prep.fs.glsl \
        approx-minmax.fs.glsl \
	cart-coord.fs.glsl \
	render.vs.glsl \
	render.fs.glsl
GLSL_SHADERS_H = $(patsubst %.glsl,%.glsl.h,$(GLSL_SHADERS))

EXTRA_DIST = $(GLSL_SHADERS)
nodist_librenderer_la_SOURCES = $(GLSL_SHADERS_H)
BUILT_SOURCES = $(GLSL_SHADERS_H)

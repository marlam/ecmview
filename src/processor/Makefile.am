# Copyright (C) 2011, 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

include $(top_srcdir)/build-aux/glsl.mk

noinst_LTLIBRARIES = libprocessor.la

libprocessor_la_CPPFLAGS = \
	-I$(top_srcdir)/src/base \
	-I$(top_srcdir)/src/uuid \
	-I$(top_srcdir)/src/glvm \
	-I$(top_srcdir)/src/cache \
	-I$(top_srcdir)/src/xgl \
	-I$(top_srcdir)/src/state \
	$(libecm_CFLAGS) \
	$(libglewmx_CFLAGS)

libprocessor_la_SOURCES = \
	processor.h processor.cpp \
	elevation/elevation_processor.h elevation/elevation_processor.cpp \
	texture/texture_processor.h texture/texture_processor.cpp \
	data/data_processor.h data/data_processor.cpp \
	e2c/e2c_processor.h e2c/e2c_processor.cpp

GLSL_SHADERS = \
	elevation/scale.fs.glsl \
	texture/color_correct.fs.glsl \
	e2c/e2c.fs.glsl \
        combine.fs.glsl combine_lens.fs.glsl
GLSL_SHADERS_H = $(patsubst %.glsl,%.glsl.h,$(GLSL_SHADERS))

EXTRA_DIST = $(GLSL_SHADERS)
nodist_libprocessor_la_SOURCES = $(GLSL_SHADERS_H)
BUILT_SOURCES = $(GLSL_SHADERS_H)

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
	$(libecmdb_CFLAGS) \
	$(libglewmx_CFLAGS)

libprocessor_la_SOURCES = \
	processor.h processor.cpp \
	elevation/elevation_processor.h elevation/elevation_processor.cpp \
	texture/texture_processor.h texture/texture_processor.cpp \
	sar-amplitude/sar_amplitude_processor.h sar-amplitude/sar_amplitude_processor.cpp \
		sar-amplitude/sub-processor.h sar-amplitude/sub-processor.cpp \
		sar-amplitude/drr.h sar-amplitude/drr.cpp \
		sar-amplitude/despeckling.h sar-amplitude/despeckling.cpp \
	data/data_processor.h data/data_processor.cpp \
	e2c/e2c_processor.h e2c/e2c_processor.cpp

GLSL_SHADERS = \
	elevation/scale.fs.glsl \
	texture/color_correct.fs.glsl \
	sar-amplitude/normalization.fs.glsl \
	sar-amplitude/despeckling-mean-0.fs.glsl \
	sar-amplitude/despeckling-mean-1.fs.glsl \
	sar-amplitude/despeckling-median-0.fs.glsl \
	sar-amplitude/despeckling-median-1.fs.glsl \
	sar-amplitude/despeckling-gauss-0.fs.glsl \
	sar-amplitude/despeckling-gauss-1.fs.glsl \
	sar-amplitude/despeckling-common-localstat-0.fs.glsl \
	sar-amplitude/despeckling-common-localstat-1.fs.glsl \
	sar-amplitude/despeckling-lee-2.fs.glsl \
	sar-amplitude/despeckling-kuan-2.fs.glsl \
	sar-amplitude/despeckling-xiao-2.fs.glsl \
	sar-amplitude/despeckling-frost-2.fs.glsl \
	sar-amplitude/despeckling-gammamap-0.fs.glsl \
	sar-amplitude/despeckling-gammamap-3.fs.glsl \
	sar-amplitude/despeckling-oddy.fs.glsl \
	sar-amplitude/despeckling-waveletst-0.fs.glsl \
	sar-amplitude/despeckling-waveletst-1.fs.glsl \
	sar-amplitude/despeckling-waveletst-2.fs.glsl \
	sar-amplitude/despeckling-waveletst-3.fs.glsl \
	sar-amplitude/despeckling-waveletst-4.fs.glsl \
	sar-amplitude/drr-linear.fs.glsl \
	sar-amplitude/drr-log.fs.glsl \
	sar-amplitude/drr-gamma.fs.glsl \
	sar-amplitude/drr-schlick.fs.glsl \
	sar-amplitude/drr-reinhard.fs.glsl \
	sar-amplitude/drr-common-localavg-0.fs.glsl \
	sar-amplitude/drr-common-localavg-1.fs.glsl \
	sar-amplitude/drr-schlicklocal-2.fs.glsl \
	sar-amplitude/drr-reinhardlocal-2.fs.glsl \
	sar-amplitude/coloring.fs.glsl \
	e2c/e2c.fs.glsl \
        combine.fs.glsl combine_lens.fs.glsl
GLSL_SHADERS_H = $(patsubst %.glsl,%.glsl.h,$(GLSL_SHADERS))

EXTRA_DIST = $(GLSL_SHADERS)
nodist_libprocessor_la_SOURCES = $(GLSL_SHADERS_H)
BUILT_SOURCES = $(GLSL_SHADERS_H)

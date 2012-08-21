# Copyright (C) 2011, 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

noinst_LTLIBRARIES = libcache.la
libcache_la_CPPFLAGS = \
	-I$(top_srcdir)/src/base \
	-I$(top_srcdir)/src/uuid \
	-I$(top_srcdir)/src/glvm \
	-I$(top_srcdir)/src/download \
	-I$(top_srcdir)/src/xgl \
	$(libecmdb_CFLAGS) \
	$(libglewmx_CFLAGS)
libcache_la_SOURCES = \
	quad-tex-pool.h quad-tex-pool.cpp \
	quad-cache.h quad-cache.cpp \
	quad-base-data-cache.h quad-base-data-cache.cpp

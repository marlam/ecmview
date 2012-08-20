# Copyright (C) 2011, 2012
# Martin Lambers <marlam@marlam.de>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice and this
# notice are preserved. This file is offered as-is, without any warranty.

SUBDIRS = logo base uuid glvm

if HAVE_LIBEQUALIZER
SUBDIRS += dtrack
endif

SUBDIRS += download xgl cache state processor renderer gui

if HAVE_LIBEQUALIZER
SUBDIRS += eq
endif

SUBDIRS += ecmview

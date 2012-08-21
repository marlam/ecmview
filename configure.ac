dnl Copyright (C) 2009, 2010, 2011, 2012
dnl Computer Graphics Group, University of Siegen, Germany.
dnl Written by Martin Lambers <martin.lambers@uni-siegen.de>
dnl See http://www.cg.informatik.uni-siegen.de/ for contact information.
dnl
dnl This program is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation; either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

dnl Autotools init stuff
AC_PREREQ([2.65])
AC_INIT([ecmview], [0.6.1], [martin.lambers@uni-siegen.de], [ecmview], [http://www.cg.informatik.uni-siegen.de/])
AC_CONFIG_SRCDIR([src/ecmview/main.cpp])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_TARGET
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_LARGEFILE
AM_INIT_AUTOMAKE([1.11.1 color-tests silent-rules tar-pax no-texinfo.tex -Wall -Wno-portability])
AM_SILENT_RULES([yes])
LT_PREREQ([2.2.6])
LT_INIT([disable-static])
AC_PROG_CXX
AC_PROG_INSTALL
AC_LANG([C++])

dnl We want C++0x, and blindly add -std=gnu++0x. This only works for gcc (and possibly clang)!
CXXFLAGS="-std=gnu++0x $CXXFLAGS"

dnl Useful definitions
AC_DEFINE_UNQUOTED([PLATFORM], ["${target}"], [Platform triplet])

dnl Special handling of W32
case "${target}" in *-*-mingw32*) w32="yes" ;; *) w32="no " ;; esac
if test "$w32" = "yes"; then
    LDFLAGS="-mwindows $LDFLAGS"
    AC_CHECK_TOOL([WINDRES], [windres], [:])
    if test "$WINDRES" = ":"; then
        AC_MSG_ERROR([windres not found])
    fi
    AC_CHECK_TOOL([MAKENSIS], [makensis], [:])
    if test "$MAKENSIS" = ":"; then
        AC_MSG_WARN([makensis not found: you will not be able to create a W32 installer])
    fi
else
    MAKENSIS=":"
fi
AM_CONDITIONAL([HAVE_MAKENSIS], [test "$MAKENSIS" != ":"])
AM_CONDITIONAL([W32], [test "$w32" = "yes"])
if test "$w32" = "yes"; then W32=1; else W32=0; fi
AC_DEFINE_UNQUOTED([W32], [$W32], [Use W32 API?])

dnl Configure switches
AC_ARG_ENABLE([debug],
    [AS_HELP_STRING([--enable-debug], [Enable debugging checks. Disabled by default.])],
    [if test "$enableval" = "yes"; then ndebug=0; else ndebug=1; fi], [ndebug=1])
if test "$ndebug" = "1"; then
    AC_DEFINE_UNQUOTED([NDEBUG], [1], [Define to 1 to disable assertions and other debugging checks.])
fi

dnl Feature checks needed by the base modules
dnl - dbg
AC_CHECK_FUNCS([backtrace sigaction])
dnl - fio
case "${target}" in *-*-mingw*) LIBS="$LIBS -lshlwapi" ;; esac
AC_CHECK_FUNCS([fdatasync fnmatch fseeko ftello getpwuid link mmap posix_fadvise readdir_r symlink])
dnl - opt
case "${target}" in *-*-mingw*) CPPFLAGS="$CPPFLAGS -D_BSD_SOURCE" ;; esac
AC_CHECK_DECLS([optreset], [], [], [#include <getopt.h>])
dnl - str
AM_ICONV([])
AC_CHECK_FUNCS([nl_langinfo vasprintf wcswidth])
dnl - sys
AC_CHECK_FUNCS([nanosleep sysconf sched_yield])
dnl - thread
AC_MSG_CHECKING([for GCC atomic builtins])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]],
    [[int b, x = 0; b = __sync_bool_compare_and_swap(&x, 0, 1);]])],
    HAVE_GCC_ATOMIC_BUILTINS=1, HAVE_GCC_ATOMIC_BUILTINS=0)
if test "$HAVE_GCC_ATOMIC_BUILTINS" = "1"; then
    AC_MSG_RESULT([yes])
else
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([GCC atomic builtins not found. Please use a newer compiler.])
fi
AC_SEARCH_LIBS([pthread_create], [pthread])
CPPFLAGS_bak="$CPPFLAGS"
LDFLAGS_bak="$LDFLAGS"
CPPFLAGS="-pthread $CPPFLAGS"
LDFLAGS="-pthread $LDFLAGS"
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])], [], [CPPFLAGS="$CPPFLAGS_bak"; LDFLAGS="$LDFLAGS_bak"])
dnl - timer
AC_SEARCH_LIBS([clock_gettime], [rt])
AC_CHECK_FUNCS([clock_gettime])

dnl pkg-config (required to detect libraries)
PKG_PROG_PKG_CONFIG([])
if test -z "$PKG_CONFIG"; then
    AC_MSG_WARN([pkg-config not found])
    AC_MSG_WARN([required libraries cannot be detected])
    AC_MSG_WARN([please install pkg-config; Debian package: pkg-config])
fi

dnl libecmdb
PKG_CHECK_MODULES([libecmdb], [ecmdb >= 0.0], [HAVE_LIBECMDB=1], [HAVE_LIBECMDB=0])
if test "$HAVE_LIBECMDB" != "1"; then
    AC_MSG_WARN([library libecmdb not found:])
    AC_MSG_WARN([$libecmdb_PKG_ERRORS])
fi

dnl libgta
PKG_CHECK_MODULES([libgta], [gta >= 0.0], [HAVE_LIBGTA=1], [HAVE_LIBGTA=0])
if test "$HAVE_LIBGTA" != "1"; then
    AC_MSG_WARN([library libgta not found:])
    AC_MSG_WARN([$libgta_PKG_ERRORS])
fi

dnl UUID
if test "$w32" = "yes"; then
    libuuid_CFLAGS=""
    libuuid_LIBS="-lrpcrt4"
else
    PKG_CHECK_MODULES([libuuid], [uuid >= 0.0], [HAVE_LIBUUID=1], [HAVE_LIBUUID=0])
    if test "$HAVE_LIBUUID" != "1"; then
        AC_MSG_WARN([library libuuid not found:])
        AC_MSG_WARN([$libuuid_PKG_ERRORS])
        AC_MSG_WARN([libuuid is provided by util-linux-ng; Debian package: uuid-dev])
    fi
    AC_DEFINE_UNQUOTED([HAVE_LIBUUID], [$HAVE_LIBUUID], [Have libuuid?])
fi

dnl Curl
PKG_CHECK_MODULES([libcurl], [libcurl >= 7.13.1], [HAVE_LIBCURL=1], [HAVE_LIBCURL=0])
if test "$HAVE_LIBCURL" != "1"; then
    AC_MSG_WARN([library libcurl not found:])
    AC_MSG_WARN([$libcurl_PKG_ERRORS])
    AC_MSG_WARN([libcurl is provided by curl; Debian package: libcurl-dev])
fi

dnl Qt
PKG_CHECK_MODULES([libqtopengl], [QtOpenGL >= 0.0], [HAVE_LIBQTOPENGL=1], [HAVE_LIBQTOPENGL=0])
if test "$HAVE_LIBQTOPENGL" != "1"; then
    AC_MSG_WARN([library QtOpenGL not found:])
    AC_MSG_WARN([$libqtopengl_PKG_ERRORS])
    AC_MSG_WARN([QtOpenGL is provided by Qt; Debian package: libqt4-opengl-dev])
fi
if test "$HAVE_LIBQTOPENGL" = "1"; then
    RCC=""
    AC_ARG_VAR([RCC], [rcc command])
    AC_CHECK_TOOLS([RCC], [rcc-qt4 rcc])
    MOC=""
    AC_ARG_VAR([MOC], [moc command])
    AC_CHECK_TOOLS([MOC], [moc-qt4 moc])
    dnl When running on X11, we need to link with -lX11 explicitly since we use X functions directly
    AC_MSG_CHECKING([whether we need to link with X11])
    CPPFLAGS_BAK="$CPPFLAGS"
    CPPFLAGS="$CPPFLAGS $libqtopengl_CFLAGS"
    AC_COMPILE_IFELSE(
        [AC_LANG_PROGRAM([[
            #include <QGLWidget>
            #ifndef Q_WS_X11
             error: No X11
            #endif]],
        [[int x = 0;]])],
        [NEED_LIBX11="yes"],[NEED_LIBX11="no"])
    CPPFLAGS="$CPPFLAGS_BAK"
    AC_MSG_RESULT([$NEED_LIBX11])
    if test "$NEED_LIBX11" = "yes"; then
        PKG_CHECK_MODULES([libX11], [x11 >= 0.0], [HAVE_LIBX11=1], [HAVE_LIBX11=0])
        if test "$HAVE_LIBX11" != "1"; then
            AC_MSG_WARN([required library libX11 not found:])
            AC_MSG_WARN([$libX11_PKG_ERRORS])
        else
            libqtopengl_CFLAGS="$libqtopengl_CFLAGS $libX11_CFLAGS"
            libqtopengl_LIBS="$libqtopengl_LIBS $libX11_LIBS"
        fi
    fi
fi

dnl Equalizer
dnl We assume that Equalizer does not use an included GLEW!
AC_ARG_WITH([equalizer],
    [AS_HELP_STRING([--with-equalizer], [Enable multi-display support via Equalizer (enabled by default)])],
    [if test "$withval" = "yes"; then equalizer="yes"; else equalizer="no "; fi], [equalizer="auto"])
if test "$equalizer" = "yes" -o "$equalizer" = "auto"; then
    PKG_CHECK_MODULES([libequalizer], [Equalizer >= 1.0], [HAVE_LIBEQUALIZER=1], [HAVE_LIBEQUALIZER=0])
    if test "$HAVE_LIBEQUALIZER" != "1"; then
        if test "$equalizer" = "yes"; then
            AC_MSG_WARN([library libequalizer not found:])
            AC_MSG_WARN([$libequalizer_PKG_ERRORS])
            AC_MSG_WARN([libequalizer is provided by Equalizer])
        else
            equalizer="no "
        fi
    else
        equalizer="yes"
    fi
else
    HAVE_LIBEQUALIZER=0
fi
AC_DEFINE_UNQUOTED([HAVE_LIBEQUALIZER], [$HAVE_LIBEQUALIZER], [Have Equalizer?])
AM_CONDITIONAL([HAVE_LIBEQUALIZER], [test "$HAVE_LIBEQUALIZER" = "1"])

dnl GLEWMX
PKG_CHECK_MODULES([libglewmx], [glewmx >= 1.5.0], [HAVE_LIBGLEWMX=1], [HAVE_LIBGLEWMX=0])
if test "$HAVE_LIBGLEWMX" != "1"; then
    AC_MSG_WARN([required library libglewmx >= 1.5.0 not found:])
    AC_MSG_WARN([$libglewmx_PKG_ERRORS])
    AC_MSG_WARN([libglewmx is provided by glew; Debian package: libglewmx-dev])
else
    dnl Older versions of glewmx.pc files lack -DGLEW_MX in CFLAGS.
    libglewmx_CFLAGS="$libglewmx_CFLAGS -DGLEW_MX"
fi

dnl OpenGL with GLU
dnl We need to explicitly link with OpenGL and GLU since we use their functions directly.
dnl However, this is most probably not necessary on Windows and Mac OS X (for now),
dnl so we just use a pkg-config check and ignore a failure.
PKG_CHECK_MODULES([libgl], [gl >= 0.0], [HAVE_LIBGL=1], [HAVE_LIBGL=0])
PKG_CHECK_MODULES([libglu], [glu >= 0.0], [HAVE_LIBGLU=1], [HAVE_LIBGLU=0])

dnl Icon and Menu tools. It is ok if these are missing.
GTK_UPDATE_ICON_CACHE=""
AC_ARG_VAR([GTK_UPDATE_ICON_CACHE], [gtk-update-icon-cache command])
AC_CHECK_PROGS([GTK_UPDATE_ICON_CACHE], [gtk-update-icon-cache])
UPDATE_DESKTOP_DATABASE=""
AC_ARG_VAR([UPDATE_DESKTOP_DATABASE], [update-desktop-database command])
AC_CHECK_PROGS([UPDATE_DESKTOP_DATABASE], [update-desktop-database])

dnl Check if all libraries were found
if test -z "$PKG_CONFIG" \
    -o "$HAVE_LIBECMDB" != "1" \
    -o "$HAVE_LIBGTA" != "1" \
    -o \( "$w32" != "yes" -a "$HAVE_LIBUUID" != "1" \) \
    -o "$HAVE_LIBCURL" != "1" \
    -o "$HAVE_LIBQTOPENGL" != "1" \
    -o "$HAVE_LIBGLEWMX" != "1" \
    -o \( "$equalizer" = "yes" -a "$HAVE_LIBEQUALIZER" != "1" \) ; then
    AC_MSG_ERROR([One or more libraries were not found. See messages above.])
fi

dnl Output
AC_CONFIG_FILES([Makefile \
	src/dtrack/Makefile \
	src/logo/Makefile \
	src/base/Makefile \
	src/glvm/Makefile \
	src/uuid/Makefile \
	src/download/Makefile \
	src/xgl/Makefile \
	src/cache/Makefile \
	src/state/Makefile \
	src/processor/Makefile \
	src/renderer/Makefile \
	src/gui/Makefile \
	src/eq/Makefile \
	src/ecmview/Makefile \
	src/Makefile \
	])
AC_OUTPUT

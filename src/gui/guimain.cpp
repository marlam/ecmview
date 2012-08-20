/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
 * Computer Graphics Group, University of Siegen, Germany.
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>.
 * See http://www.cg.informatik.uni-siegen.de/ for contact information.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <cstdlib>

#include <GL/glew.h>

#include <QCoreApplication>
#include <QApplication>
#include <QGLFormat>
#include <QtGlobal>

#include "msg.h"

#include "mainwindow.h"
#include "guimain.h"


static void qt_msg_handler(QtMsgType type, const char *msg)
{
    switch (type) {
    case QtDebugMsg:
        msg::dbg("%s", msg);
        break;
    case QtWarningMsg:
        msg::wrn("%s", msg);
        break;
    case QtCriticalMsg:
        msg::err("%s", msg);
        break;
    case QtFatalMsg:
        msg::err("%s", msg);
        std::exit(1);
    }
}

static int* gui_argc = NULL;
static char** gui_argv = NULL;

void gui_initialize(int* argc, char** argv)
{
    gui_argc = argc;
    gui_argv = argv;
#ifdef Q_WS_X11
    const char *display = getenv("DISPLAY");
    bool have_display = (display && display[0] != '\0');
#else
    bool have_display = true;
#endif
    qInstallMsgHandler(qt_msg_handler);
    new QApplication(*argc, argv, have_display);
    //QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale()); // necessary for i18n via gettext
    QCoreApplication::setOrganizationName(PACKAGE_TARNAME);
    //QCoreApplication::setOrganizationDomain("???");
    QCoreApplication::setApplicationName(PACKAGE_TARNAME);
}

int qInitResources();

int gui_run(state *master_state)
{
    if (!master_state) {
        // This will never be called; it is just here
        // to force linking with the Qt resource object file.
        qInitResources();
    }
    QGLFormat fmt(
              QGL::DoubleBuffer
            | QGL::DepthBuffer
            | QGL::Rgba
            | QGL::DirectRendering
            | QGL::NoSampleBuffers
            | QGL::NoAlphaChannel
            | QGL::NoAccumBuffer
            | QGL::NoStencilBuffer
            | QGL::NoStereoBuffers
            | QGL::NoOverlay
            | QGL::NoSampleBuffers);
    //fmt.setDepthBufferSize(24);
    QGLFormat::setDefaultFormat(fmt);
    MainWindow mainwindow(gui_argc, gui_argv, master_state);
    return QApplication::instance()->exec();
}

void gui_deinitialize()
{
    delete QApplication::instance();
}

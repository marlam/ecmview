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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "config.h"

#include <cstring>

#include <QMainWindow>
#include <QSettings>
#include <QComboBox>
#include <QImage>

#include "state.h"

#include "auth_cb.h"
#include "databases.h"
#include "guicontext.h"
#include "info.h"
#include "light.h"
#include "lens.h"
#include "quaddebug.h"
#include "renderprops.h"
#include "statistics.h"

class eq_context;

class MainWindow : public QMainWindow
{
Q_OBJECT

private:
    int* _argc;
    char** _argv;
    state* _master_state;
    std::string _state_file_name;

    QSettings* _settings;

    GUIContext* _guicontext;
    eq_context* _eqcontext;

    DBs* _dbs;
    Info* _info;
    Light* _light;
    Lens* _lens;
    QuadDebug* _quaddebug;
    RenderProps* _renderprops;
    Statistics* _statistics;

    qt_auth_callback _qt_auth_callback;

protected:
    void closeEvent(QCloseEvent* event);

public:
    MainWindow(int* argc, char** argv, state* master_state, QWidget* parent = NULL);
    ~MainWindow();

private:
    // Helper functions
    void save_image(QImage* img);

private slots:

    // Special actions
    void renderloop();

    // Menu actions: File
    void open_local_dataset();
    void open_remote_dataset();
    void open_state();
    void save_state_as();
    void save_state();
    // Menu actions: View
    void fullscreen();
    void copy_current_view();
    void save_current_view();
    void save_view();
    // Menu actions: Dialog
    void toggle_equalizer();
    void toggle_renderprops();
    void toggle_statistics();
    // Menu actions: Help
    void show_aboutbox();
};

#endif

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

#include <GL/glu.h>

#include <QGLWidget>
#include <QMessageBox>
#include <QApplication>
#include <QWheelEvent>
#include <QDesktopWidget>
#ifdef Q_WS_X11
# include <QX11Info>
# include <X11/Xlib.h>
#endif

#include "msg.h"
#include "sys.h"

#include "glvm.h"
#include "xgl.h"

#include "guicontext.h"

using namespace glvm;


GUIContext::GUIContext(class state* master_state, QWidget* parent) : QGLWidget(parent),
    _initialized(false), _permanent_failure(false), _width(-1), _height(-1),
    _want_viewer_info(false), _want_pointer_info(false),
    _last_frame_quads(0), _fullscreen(false),
    _master_state(master_state), _renderer(*this),
    _quad_disk_cache(master_state->app_id, master_state->cache_dir),
    _quad_disk_cache_checkers(16, &_quad_disk_cache),
    _quad_disk_cache_fetchers(16, &_quad_disk_cache),
    _quad_mem_cache(),
    _quad_mem_cache_loaders(min(255, sys::processors() * 3 / 2 + 1), &_quad_mem_cache),
    _quad_gpu_cache(),
    _quad_metadata_cache(),
    _quad_base_data_mem_cache(),
    _quad_base_data_mem_cache_computers(min(255, sys::processors() * 3 / 2 + 1), &_quad_base_data_mem_cache),
    _quad_base_data_gpu_cache(),
    _quad_tex_pool()
{
    if (!context()->isValid()) {
        QMessageBox::critical(this, "Fatal Error", "<p>Cannot get a valid OpenGL context. Aborting.</p>");
        std::exit(1);
    }
    setMouseTracking(true);
//    setFixedSize(512,512);
}

GUIContext::~GUIContext()
{
    exit_gl();
}

void GUIContext::init_gl()
{
    if (!_permanent_failure && !_initialized) {
        makeCurrent();
        std::vector<std::string> missing_gl_features = renderer_context::initialize_gl();
        if (!missing_gl_features.empty()) {
            std::string list("");
            std::vector<std::string>::iterator iter;
            for (iter = missing_gl_features.begin(); iter != missing_gl_features.end(); iter++) {
                list += std::string(*iter) + std::string(" ");
            }
            throw exc("The following OpenGL features are not available: " + list);
        }
        _quad_tex_pool.init_gl();
        _renderer.init_gl();
        xgl::CheckError(HERE);
        _initialized = true;
    }
}

void GUIContext::exit_gl()
{
    if (_initialized) {
        makeCurrent();
        quad_gpu_cache()->clear();
        quad_base_data_gpu_cache()->clear();
        quad_tex_pool()->exit_gl();
        _renderer.exit_gl();
        xgl::CheckError(HERE);
        _initialized = false;
    }
}

void GUIContext::initializeGL()
{
    try {
        init_gl();
    }
    catch (exc &e) {
        permanent_failure(e);
    }
}

void GUIContext::resizeGL(int width, int height)
{
    glViewport(0, 0, width, height);

    if (_permanent_failure || !_initialized)
        return;

    msg::dbg("Viewport size: %dx%d", width, height);

    _width = width;
    _height = height;

    // Inform the navigator
    _navigator.set_viewport(ivec4(0, 0, width, height));

    // Recompute frustum
    double w = static_cast<double>(width) / 100.0 * 0.0254;     // Window width in m (assuming 100 DPI monitor)
    double h = static_cast<double>(height) / 100.0 * 0.0254;    // Window height in m (assuming 100 DPI monitor)
    _frustum.l() = - w / 2.0;
    _frustum.r() = + w / 2.0;
    _frustum.b() = - h / 2.0;
    _frustum.t() = + h / 2.0;
    _frustum.n() = 0.5;                                         // Assumed distance in m between user and monitor
    _frustum.f() = 100.0;                                       // Does not matter because it is adapted by the renderer
}

void GUIContext::permanent_failure(const exc& e)
{
    _permanent_failure = true;
    this->setMouseTracking(false);
    this->setEnabled(false);
    QMessageBox::critical(this, "Error",
            tr("<p>%1</p><p>This is a fatal error; no further rendering will be done.</p>").arg(e.what()));
}

void GUIContext::paintGL()
{
    if (_permanent_failure || !_initialized) {
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    // Rendering
    try {
        _renderer.render(ivec4(0, 0, _width, _height), _frustum, dmat4(1.0));
        _last_frame_info = _renderer.get_info();
        _last_frame_quads = 0;
        int approx_quads = 0;
        for (int i = 0; i < _last_frame_info.depth_passes; i++) {
            _last_frame_quads += _last_frame_info.quads_rendered[i];
            approx_quads += _last_frame_info.quads_approximated[i];
        }
        emit update_statistics(fps(), _last_frame_info);
        emit update_approx_info(approx_quads);
        if (_last_frame_quads > 0) {
            if (_want_viewer_info || _want_pointer_info) {
                const class ecm ecm(state().semi_major_axis(), state().semi_minor_axis());
                if (_want_viewer_info) {
                    dvec3 viewer_geod;
                    ecm.cartesian_to_geodetic(_master_state->viewer_pos.vl, &(viewer_geod.x), &(viewer_geod.y), &(viewer_geod.z));
                    emit update_viewer_info(viewer_geod.x, viewer_geod.y, viewer_geod.z);
                }
                if (_want_pointer_info) {
                    if (!all(equal(_last_frame_info.pointer_coord, dvec3(0.0)))) {
                        dvec3 pointer_geod;
                        ecm.cartesian_to_geodetic(_last_frame_info.pointer_coord.vl, &(pointer_geod.x), &(pointer_geod.y), &(pointer_geod.z));
                        emit update_pointer_info(true, pointer_geod.x, pointer_geod.y, pointer_geod.z);
                    } else {
                        emit update_pointer_info(false, 0.0, 0.0, 0.0);
                    }
                }
            }
        }
#ifndef NDEBUG
        // Quad debugging
        emit update_quad_debug_range(_last_frame_quads);
        emit update_quad_debug_info(
                _last_frame_info.debug_quad[0], _last_frame_info.debug_quad[1],
                _last_frame_info.debug_quad[2], _last_frame_info.debug_quad[3],
                _last_frame_info.debug_quad_max_dist_to_quad_plane,
                _last_frame_info.debug_quad_min_elev,
                _last_frame_info.debug_quad_max_elev);
#endif
    }
    catch (exc& e) {
        permanent_failure(e);
    }
    tick();
}

void GUIContext::update_renderer_parameters(const class renderer_parameters renderer_parameters)
{
    _master_state->renderer = renderer_parameters;
}

void GUIContext::update_info_prefs(bool want_viewer_info, bool want_pointer_info)
{
    _want_viewer_info = want_viewer_info;
    _want_pointer_info = want_pointer_info;
}

/* Render interface */

void GUIContext::render()
{
    updateGL();
}

/* Render to image */

QImage* GUIContext::get_current_image()
{
    if (_permanent_failure || !_initialized) {
        return new QImage();
    }
    makeCurrent();
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    QImage* img = new QImage(grabFrameBuffer());
    QApplication::restoreOverrideCursor();
    return img;
}

QImage* GUIContext::render_to_image(int width, int height)
{
    if (_permanent_failure || !_initialized) {
        return new QImage();
    }

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    makeCurrent();

    blob *array = new blob();
    try {
        array->resize(width, height, 4 * sizeof(unsigned char));
        // Compute a new frustum. The aspect ratio of objects in the current view should remain the
        // same in the requested view, even if the aspect ratio of the requested view differs from
        // the aspect ratio of the current view.
        const double old_frustum_w = (_frustum.r() - _frustum.l());
        const double old_frustum_h = (_frustum.t() - _frustum.b());
        const double old_frustum_ar = old_frustum_w / old_frustum_h;
        const double new_frustum_ar = static_cast<double>(width) / static_cast<double>(height);
        dfrust new_frustum = _frustum;
        if (new_frustum_ar > old_frustum_ar) {
            const double additional_width = (new_frustum_ar / old_frustum_ar) * old_frustum_w - old_frustum_w;
            new_frustum.l() -= additional_width / 2.0;
            new_frustum.r() += additional_width / 2.0;
        } else if (new_frustum_ar < old_frustum_ar) {
            const double additional_height = (old_frustum_ar / new_frustum_ar) * old_frustum_h - old_frustum_h;
            new_frustum.b() -= additional_height / 2.0;
            new_frustum.t() += additional_height / 2.0;
        }
        _renderer.render(new_frustum, width, height, array->ptr());
    }
    catch (exc &e) {
        QApplication::restoreOverrideCursor();
        permanent_failure(e);
        delete array;
        return new QImage();
    }

    QImage *tmpimg = new QImage(array->ptr<unsigned char>(), width, height, QImage::Format_ARGB32);
    QImage *img = new QImage(tmpimg->copy(0, 0, width, height).mirrored(false, true));
    delete tmpimg;
    delete array;

    QApplication::restoreOverrideCursor();
    return img;
}

/* Fullscreen */

void GUIContext::enter_fullscreen(int screens)
{
    if (!_fullscreen) {
        setFocusPolicy(Qt::StrongFocus);
        setWindowFlags(Qt::Window);
        // Determine combined geometry of the chosen screens.
        int screen_count = 0;
        QRect geom;
        for (int i = 0; i < std::min(QApplication::desktop()->screenCount(), 16); i++) {
            if (screens & (1 << i)) {
                if (geom.isNull())
                    geom = QApplication::desktop()->screenGeometry(i);
                else
                    geom = geom.united(QApplication::desktop()->screenGeometry(i));
                screen_count++;
            }
        }
        if (geom.isNull()) {
            // Use default screen
            geom = QApplication::desktop()->screenGeometry(-1);
        }
        Qt::WindowFlags new_window_flags =
            windowFlags()
            | Qt::FramelessWindowHint
            | Qt::WindowStaysOnTopHint;
        // In the dual and multi screen cases we need to bypass the window manager
        // on X11 because Qt does not support _NET_WM_FULLSCREEN_MONITORS, and thus
        // the window manager would always restrict the fullscreen window to one screen.
        // Note: it may be better to set _NET_WM_FULLSCREEN_MONITORS ourselves, but that
        // would also require the window manager to support this extension...
        if (screen_count > 1)
            new_window_flags |= Qt::X11BypassWindowManagerHint;
        setWindowFlags(new_window_flags);
        setWindowState(windowState() | Qt::WindowFullScreen);
        setGeometry(geom);
        setCursor(Qt::BlankCursor);
        show();
        raise();
        activateWindow();
#ifdef Q_WS_X11
        /* According to the Qt documentation, it should be sufficient to call activateWindow()
         * to make a X11 window active when using Qt::X11BypassWindowManagerHint, but this
         * does not work for me (Ubuntu 11.04 Gnome-2D desktop). This is a workaround. */
        /* Note that using X11 functions directly means that we have to link with libX11
         * explicitly; see configure.ac. */
        if (new_window_flags & Qt::X11BypassWindowManagerHint) {
            QApplication::syncX();      // just for safety; not sure if it is necessary
            XSetInputFocus(QX11Info::display(), winId(), RevertToParent, CurrentTime);
            XFlush(QX11Info::display());
        }
#endif
        setFocus(Qt::OtherFocusReason);
        _fullscreen = true;
    }
}

bool GUIContext::toggle_fullscreen(int screens)
{
    if (_fullscreen)
        exit_fullscreen();
    else
        enter_fullscreen(screens);
    return _fullscreen;
}

void GUIContext::exit_fullscreen()
{
    if (_fullscreen) {
        setWindowFlags(Qt::Widget);
        setWindowFlags(windowFlags()
                & ~Qt::X11BypassWindowManagerHint
                & ~Qt::FramelessWindowHint
                & ~Qt::WindowStaysOnTopHint);
        setWindowState(windowState() & ~Qt::WindowFullScreen);
        setCursor(Qt::ArrowCursor);
        setFocusPolicy(Qt::NoFocus);
        show();
        _fullscreen = false;
    }
}

/* Light */

void GUIContext::set_light(class light_parameters lp)
{
    _master_state->light = lp;
}

/* Lens */

void GUIContext::set_lens(class lens_parameters lp)
{
    _master_state->lens = lp;
}

/* Quad Debug */

#ifdef NDEBUG
void GUIContext::set_quad_debug(int, bool)
{
}
#else
void GUIContext::set_quad_debug(int index, bool save)
{
    int depth_pass = 0;
    while (depth_pass < _last_frame_info.depth_passes - 1
            && index >= _last_frame_info.quads_rendered[depth_pass]) {
        index -= _last_frame_info.quads_rendered[depth_pass];
        depth_pass++;
    }
    _master_state->debug_quad_depth_pass = depth_pass;
    _master_state->debug_quad_index = index;
    _master_state->debug_quad_save = save;
}
#endif

/* Navigation */

void GUIContext::set_viewer(const glvm::dvec3& viewer_pos, const glvm::dquat& viewer_rot)
{
    _master_state->viewer_pos = viewer_pos;
    _master_state->viewer_rot = viewer_rot;
}

void GUIContext::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Escape:
        if (_fullscreen)
            exit_fullscreen();
        break;
    }
}

void GUIContext::mousePressEvent(QMouseEvent* event)
{
    if (_permanent_failure || _width < 1 || _height < 1 || !_initialized)
        return;

    ivec2 pos = ivec2(event->pos().x(), event->pos().y());
    if (event->modifiers() & Qt::MetaModifier && event->buttons() & Qt::LeftButton) {
        _navigator.surface_object_move_start(pos,
                _master_state->viewer_pos, _master_state->inner_bounding_sphere_radius);
        emit update_lens(_master_state->lens);
    } else if (event->buttons() & Qt::LeftButton) {
        _navigator.planet_rot_start(pos, _master_state->inner_bounding_sphere_radius);
    } else if (event->buttons() & Qt::MidButton) {
        _navigator.viewer_rot_start(pos);
    } else if (event->buttons() & Qt::RightButton) {
        _navigator.planet_distchange_start(pos, _master_state->viewer_pos, _master_state->inner_bounding_sphere_radius);
    }
}

void GUIContext::mouseMoveEvent(QMouseEvent* event)
{
    if (_permanent_failure || _width < 1 || _height < 1 || !_initialized)
        return;

    double speed_factor =
          (event->modifiers() & Qt::ShiftModifier) ? 0.1
        : (event->modifiers() & Qt::ControlModifier) ? 10.0
        : 1.0;

    ivec2 pos = ivec2(event->pos().x(), event->pos().y());
    if (_want_pointer_info) {
        _master_state->pointer_pos = ivec2(pos.x, _height - 1 - pos.y);
    } else {
        _master_state->pointer_pos = ivec2(-1, -1);
    }
    if (event->modifiers() & Qt::MetaModifier && event->buttons() & Qt::LeftButton) {
        _navigator.surface_object_move(speed_factor, pos, _master_state->lens.pos);
        emit update_lens(_master_state->lens);
    } else if (event->buttons() & Qt::LeftButton) {
        _navigator.planet_rot(speed_factor, pos, _master_state->viewer_pos, _master_state->viewer_rot);
    } else if (event->buttons() & Qt::MidButton) {
        _navigator.viewer_rot(speed_factor, pos, _master_state->viewer_rot);
    } else if (event->buttons() & Qt::RightButton) {
        _navigator.planet_distchange(speed_factor, pos, _master_state->viewer_pos);
    }
}

void GUIContext::wheelEvent(QWheelEvent* event)
{
    if (_permanent_failure || _width < 1 || _height < 1 || !_initialized)
        return;

    double speed_factor =
          (event->modifiers() & Qt::ShiftModifier) ? 0.1
        : (event->modifiers() & Qt::ControlModifier) ? 10.0
        : 1.0;

    ivec2 pos = ivec2(event->pos().x(), event->pos().y());
    if (event->modifiers() & Qt::MetaModifier) {
        _navigator.surface_object_scale(speed_factor, radians(event->delta() * 8.0),
                _master_state->viewer_pos, _master_state->inner_bounding_sphere_radius,
                _master_state->lens.radius);
        emit update_lens(_master_state->lens);
    } else {
        _navigator.viewer_shift(speed_factor, radians(event->delta() * 8.0),
                _master_state->inner_bounding_sphere_radius, _master_state->viewer_rot,
                _master_state->viewer_pos);
    }
}

/* Renderer interface */

const class state& GUIContext::state() const
{
    return *_master_state;
}

bool GUIContext::start_per_node_maintenance()
{
    return true;
}

void GUIContext::finish_per_node_maintenance()
{
}

bool GUIContext::start_per_glcontext_maintenance()
{
    return true;
}

void GUIContext::finish_per_glcontext_maintenance()
{
}

class quad_disk_cache* GUIContext::quad_disk_cache()
{
    return &_quad_disk_cache;
}

class quad_disk_cache_checkers* GUIContext::quad_disk_cache_checkers()
{
    return &_quad_disk_cache_checkers;
}

class quad_disk_cache_fetchers* GUIContext::quad_disk_cache_fetchers()
{
    return &_quad_disk_cache_fetchers;
}

class quad_mem_cache* GUIContext::quad_mem_cache()
{
    return &_quad_mem_cache;
}

class quad_mem_cache_loaders* GUIContext::quad_mem_cache_loaders()
{
    return &_quad_mem_cache_loaders;
}

class quad_gpu_cache* GUIContext::quad_gpu_cache()
{
    return &_quad_gpu_cache;
}

class quad_metadata_cache* GUIContext::quad_metadata_cache()
{
    return &_quad_metadata_cache;
}

class quad_base_data_mem_cache* GUIContext::quad_base_data_mem_cache()
{
    return &_quad_base_data_mem_cache;
}

class quad_base_data_mem_cache_computers* GUIContext::quad_base_data_mem_cache_computers()
{
    return &_quad_base_data_mem_cache_computers;
}

class quad_base_data_gpu_cache* GUIContext::quad_base_data_gpu_cache()
{
    return &_quad_base_data_gpu_cache;
}

class quad_tex_pool* GUIContext::quad_tex_pool()
{
    return &_quad_tex_pool;
}

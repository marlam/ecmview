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

#ifndef GUICONTEXT_H
#define GUICONTEXT_H

#include "config.h"

#include <vector>
#include <string>

#include <GL/glew.h>

#include <QGLWidget>

#include "exc.h"

#include "glvm.h"

#include "state.h"
#include "renderer.h"
#include "navigator.h"
#include "quad-cache.h"
#include "info.h"


class QImage;

class GUIContext : public QGLWidget, public renderer_context
{
Q_OBJECT

private:
    GLEWContext _glew_context;
    bool _initialized;
    bool _permanent_failure;
    int _width;
    int _height;
    bool _want_viewer_info;
    bool _want_pointer_info;
    renderpass_info _last_frame_info;
    int _last_frame_quads;
    bool _fullscreen;

    class state* _master_state;
    glvm::dfrust _frustum;
    renderer _renderer;
    navigator _navigator;

    class quad_disk_cache _quad_disk_cache;
    class quad_disk_cache_checkers _quad_disk_cache_checkers;
    class quad_disk_cache_fetchers _quad_disk_cache_fetchers;
    class quad_mem_cache _quad_mem_cache;
    class quad_mem_cache_loaders _quad_mem_cache_loaders;
    class quad_gpu_cache _quad_gpu_cache;
    class quad_metadata_cache _quad_metadata_cache;
    class quad_base_data_mem_cache _quad_base_data_mem_cache;
    class quad_base_data_mem_cache_computers _quad_base_data_mem_cache_computers;
    class quad_base_data_gpu_cache _quad_base_data_gpu_cache;
    class quad_tex_pool _quad_tex_pool;

    void permanent_failure(const exc &e);

public:
    GUIContext(class state *master_state, QWidget* parent = NULL);
    ~GUIContext();

    bool works() const { return _initialized && !_permanent_failure; }
    QImage* get_current_image();
    QImage* render_to_image(int width, int height);

    // Fullscreen management:
    bool fullscreen() const { return _fullscreen; }
    void enter_fullscreen(int screens);
    void exit_fullscreen();
    bool toggle_fullscreen(int screens);

    /* provide a GLEW context for the renderer_context */
    virtual GLEWContext* glewGetContext()
    {
        return &_glew_context;
    }

    /* renderer_context functions for use by the application */
    virtual void init_gl();
    virtual void exit_gl();
    virtual void render();

    /* renderer_context functions for use by the renderer */
    virtual const class state& state() const;
    virtual bool start_per_node_maintenance();
    virtual void finish_per_node_maintenance();
    virtual bool start_per_glcontext_maintenance();
    virtual void finish_per_glcontext_maintenance();
    virtual class quad_disk_cache* quad_disk_cache();
    virtual class quad_disk_cache_checkers* quad_disk_cache_checkers();
    virtual class quad_disk_cache_fetchers* quad_disk_cache_fetchers();
    virtual class quad_mem_cache* quad_mem_cache();
    virtual class quad_mem_cache_loaders* quad_mem_cache_loaders();
    virtual class quad_gpu_cache* quad_gpu_cache();
    virtual class quad_metadata_cache* quad_metadata_cache();
    virtual class quad_base_data_mem_cache* quad_base_data_mem_cache();
    virtual class quad_base_data_mem_cache_computers* quad_base_data_mem_cache_computers();
    virtual class quad_base_data_gpu_cache* quad_base_data_gpu_cache();
    virtual class quad_tex_pool* quad_tex_pool();

    /* Interaction */
    virtual void keyPressEvent(QKeyEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* event);

public slots:
    void set_viewer(const glvm::dvec3& viewer_pos, const glvm::dquat& viewer_rot);
    void set_light(class light_parameters);
    void set_lens(class lens_parameters);
    void set_quad_debug(int index, bool save);
    void update_renderer_parameters(const class renderer_parameters renderer_parameters);
    void update_info_prefs(bool want_viewer_info, bool want_pointer_info);

protected:
    void initializeGL();
    void resizeGL(int width, int height);
    void paintGL();

signals:
    void update_quad_debug_range(int);
    void update_quad_debug_info(int, int, int, int, float, float, float);
    void update_light(class light_parameters);
    void update_lens(class lens_parameters);
    void update_statistics(float, const renderpass_info&);
    void update_approx_info(int);
    void update_viewer_info(double, double, double);
    void update_pointer_info(bool, double, double, double);
};

#endif

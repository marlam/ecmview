/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013
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

#include <vector>
#include <string>
#include <limits>

#include <GL/glew.h>

#include "str.h"
#include "tmr.h"

#include "renderer-context.h"


renderer_context::renderer_context() :
    _ticks_available(0), _tick_intervals_index(0)
{
}

renderer_context::~renderer_context()
{
}

std::vector<std::string> renderer_context::initialize_gl()
{
    const char *extension_names[] = {
        "GL_VERSION_3_0",
        // TODO: Find out which GL version and which extensions are really required!
        /*
        "GL_ARB_transpose_matrix",
        "GL_EXT_direct_state_access",
        "GL_EXT_gpu_shader4",
        "GL_ARB_vertex_shader",
        "GL_ARB_geometry_shader4",
        "GL_ARB_fragment_shader",
        "GL_ARB_vertex_buffer_object",
        "GL_ARB_multitexture",
        "GL_ARB_texture_float",
        "GL_EXT_texture_integer",
        "GL_ARB_texture_buffer_object",
        "GL_EXT_texture_filter_anisotropic",
        "GL_ARB_texture_non_power_of_two",
        "GL_ARB_texture_rg",
        "GL_ARB_color_buffer_float",
        "GL_ARB_draw_buffers",
        "GL_ARB_framebuffer_object",
        "GL_EXT_framebuffer_blit",
        "GL_EXT_framebuffer_multisample",
        */
    };
    const int extensions = sizeof(extension_names) / sizeof(extension_names[0]);
    std::vector<std::string> missing;
    GLenum err;

    if ((err = glewInit()) != GLEW_OK) {
        missing.push_back(str::asprintf("Cannot initialize GLEW: %s", glewGetErrorString(err)));
    } else {
        for (int i = 0; i < extensions; i++) {
            if (!glewIsSupported(extension_names[i]))
                missing.push_back(extension_names[i]);
        }
        // TODO: Check for GL context properties (number of depth bits, ...) if required
    }

    return missing;
}

void renderer_context::tick()
{
    int64_t now = timer::get(timer::monotonic);
    _tick_intervals[_tick_intervals_index] = now - _last_tick;
    _last_tick = now;
    _tick_intervals_index++;
    if (_tick_intervals_index >= _ticks)
        _tick_intervals_index = 0;
    if (_ticks_available < _ticks)
        _ticks_available++;
}

float renderer_context::fps() const
{
    if (_ticks_available < _ticks) {
        return 0.0f;
    } else {
        int64_t tick_interval_avg = 0;
        for (int i = 0; i < _ticks; i++) {
            tick_interval_avg += _tick_intervals[i];
        }
        tick_interval_avg /= _ticks;
        return 1e6f / tick_interval_avg;
    }
}

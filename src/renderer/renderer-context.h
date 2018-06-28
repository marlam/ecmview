/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2018
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

#ifndef CONTEXT_H
#define CONTEXT_H

#include <vector>
#include <string>
#include <cstdint>

#include <GL/glew.h>

#include "quad-cache.h"
#include "quad-base-data-cache.h"
#include "state.h"


class renderer_context
{
private:
    // FPS measurement
    static const int _ticks = 10;
    int _ticks_available;
    int64_t _tick_intervals[_ticks];
    int _tick_intervals_index;
    int64_t _last_tick;

protected:
    void tick();
    /* Initialize OpenGL extensions, and check for required OpenGL
     * extensions and/or required properties of the OpenGL context.
     * Returns a vector strings that contains names of extensions
     * that are missing and descriptions of insufficient context
     * properties. If the vector is empty, everything's fine.
     * Use this function before doing anything fancy with the GL
     * context. */
    std::vector<std::string> initialize_gl();

public:
    renderer_context();
    virtual ~renderer_context();

    /* Gather statistics info */
    float fps() const;

    /* For use by the application */
    virtual void init_gl() = 0;
    virtual void exit_gl() = 0;
    virtual void render() = 0;

    /* For use by the renderer */
    virtual const class state& state() const = 0;
    virtual bool start_per_node_maintenance() = 0;
    virtual void finish_per_node_maintenance() = 0;
    virtual bool start_per_glcontext_maintenance() = 0;
    virtual void finish_per_glcontext_maintenance() = 0;
    virtual class quad_disk_cache* quad_disk_cache() = 0;
    virtual class quad_disk_cache_checkers* quad_disk_cache_checkers() = 0;
    virtual class quad_disk_cache_fetchers* quad_disk_cache_fetchers() = 0;
    virtual class quad_mem_cache* quad_mem_cache() = 0;
    virtual class quad_mem_cache_loaders* quad_mem_cache_loaders() = 0;
    virtual class quad_gpu_cache* quad_gpu_cache() = 0;
    virtual class quad_metadata_cache* quad_metadata_cache() = 0;
    virtual class quad_base_data_mem_cache* quad_base_data_mem_cache() = 0;
    virtual class quad_base_data_mem_cache_computers* quad_base_data_mem_cache_computers() = 0;
    virtual class quad_base_data_gpu_cache* quad_base_data_gpu_cache() = 0;
    virtual class quad_tex_pool* quad_tex_pool() = 0;
};

#endif

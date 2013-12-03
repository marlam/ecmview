/*
 * Copyright (C) 2009, 2010, 2011, 2012, 2013
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

#include <sstream>

#include "glvm.h"
#include "glvm-ser.h"

#include "renderer_parameters.h"


renderer_parameters::renderer_parameters()
{
    reset();
}

void renderer_parameters::reset()
{
    background_color[0] = 0;
    background_color[1] = 0;
    background_color[2] = 0;
    quad_screen_size_ratio = 2.0f;
    fixed_quadtree_depth = 0;
    quad_subdivision = 6;
    wireframe = false;
    bounding_boxes = false;
    quad_borders = false;
    mipmapping = false;
    force_lod_sync = false;
    statistics_overlay = false;
    gpu_cache_size = 256UL * 1024UL * 1024UL;
    mem_cache_size = 2048UL * 1024UL * 1024UL;
}

void renderer_parameters::save(std::ostream& os) const
{
    s11n::save(os, background_color[0]);
    s11n::save(os, background_color[1]);
    s11n::save(os, background_color[2]);
    s11n::save(os, quad_screen_size_ratio);
    s11n::save(os, fixed_quadtree_depth);
    s11n::save(os, quad_subdivision);
    s11n::save(os, wireframe);
    s11n::save(os, bounding_boxes);
    s11n::save(os, quad_borders);
    s11n::save(os, mipmapping);
    s11n::save(os, force_lod_sync);
    s11n::save(os, statistics_overlay);
    s11n::save(os, gpu_cache_size);
    s11n::save(os, mem_cache_size);
}

void renderer_parameters::load(std::istream& is)
{
    s11n::load(is, background_color[0]);
    s11n::load(is, background_color[1]);
    s11n::load(is, background_color[2]);
    s11n::load(is, quad_screen_size_ratio);
    s11n::load(is, fixed_quadtree_depth);
    s11n::load(is, quad_subdivision);
    s11n::load(is, wireframe);
    s11n::load(is, bounding_boxes);
    s11n::load(is, quad_borders);
    s11n::load(is, mipmapping);
    s11n::load(is, force_lod_sync);
    s11n::load(is, statistics_overlay);
    s11n::load(is, gpu_cache_size);
    s11n::load(is, mem_cache_size);
}

void renderer_parameters::save(std::ostream& os, const char* name) const
{
    s11n::startgroup(os, name);
    glvm::vector<uint8_t, 3> bc(background_color);
    s11n::save(os, "background-color", bc);
    s11n::save(os, "quad-screen-size-ratio", quad_screen_size_ratio);
    s11n::save(os, "fixed-quadtree-depth", fixed_quadtree_depth);
    s11n::save(os, "quad-subdivision", quad_subdivision);
    s11n::save(os, "wireframe", wireframe);
    s11n::save(os, "bounding-boxes", bounding_boxes);
    s11n::save(os, "quad-borders", quad_borders);
    s11n::save(os, "mipmapping", mipmapping);
    s11n::save(os, "force-lod-sync", force_lod_sync);
    s11n::save(os, "statistics-overlay", statistics_overlay);
    s11n::save(os, "gpu-cache-size", gpu_cache_size);
    s11n::save(os, "mem-cache-size", mem_cache_size);
    s11n::endgroup(os);
}

void renderer_parameters::load(const std::string& s)
{
    reset();
    std::istringstream iss(s);
    std::string name, value;
    while (iss.good()) {
        s11n::load(iss, name, value);
        if (name == "background-color") {
            glvm::vector<uint8_t, 3> bc;
            s11n::load(value, bc);
            background_color[0] = bc.r;
            background_color[1] = bc.g;
            background_color[2] = bc.b;
        } else if (name == "quad-screen-size-ratio") {
            s11n::load(value, quad_screen_size_ratio);
        } else if (name == "fixed-quadtree-depth") {
            s11n::load(value, fixed_quadtree_depth);
        } else if (name == "quad-subdivision") {
            s11n::load(value, quad_subdivision);
        } else if (name == "wireframe") {
            s11n::load(value, wireframe);
        } else if (name == "bounding-boxes") {
            s11n::load(value, bounding_boxes);
        } else if (name == "quad-borders") {
            s11n::load(value, quad_borders);
        } else if (name == "mipmapping") {
            s11n::load(value, mipmapping);
        } else if (name == "force-lod-sync") {
            s11n::load(value, force_lod_sync);
        } else if (name == "statistics-overlay") {
            s11n::load(value, statistics_overlay);
        } else if (name == "gpu-cache-size") {
            s11n::load(value, gpu_cache_size);
        } else if (name == "mem-cache-size") {
            s11n::load(value, mem_cache_size);
        }
    }
}

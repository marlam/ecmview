/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012
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

#ifndef RENDERER_H
#define RENDERER_H

#include <vector>

#include <GL/glew.h>

#include "glvm.h"

#include "xgl.h"

#include "renderer-context.h"
#include "terrain.h"


class renderpass_info;

class renderer
{
private:
    /* Configuration */
    static const int _min_depth_bits = 12;  // Number of required depth bits per pass
    static const int _max_depth_passes = 4; // Maximum number of depth passes

private:
    renderer_context& _renderer_context;
    int _cache_quad_size;      // The cached data is valid for this quad size only
    class ecm _cache_ecm;      // The cached data is valid for this ecm only
    int _last_frame_quads;
    terrain _terrain;
    std::vector<unsigned char> _xgl_stack;
    renderpass_info* _info;

    void compute_depth_passes(const glvm::dfrust& frustum, int depth_bits);

public:
    /* Constructor/Destructor */
    renderer(renderer_context& renderer_context);
    ~renderer();

    /* Standard OpenGL init/exit functions */
    void init_gl();
    void exit_gl();

    /* Render the scene. */
    void render(const glvm::ivec4& viewport, const glvm::dfrust& frustum, const glvm::dmat4& viewer_transform);

    /* Render the scene into a buffer in the format 0xffRRGGBB */
    void render(const glvm::dfrust& frustum, int width, int height, void* buffer);

    /* After a call to render(), information about the last render pass can
     * be retrieved using the following function. */
    const renderpass_info& get_info() const
    {
        return *_info;
    }

    friend class renderpass_info;
};

class renderpass_info
{
public:
    // Number of depth passes
    int depth_passes;
    // Per-depth-pass information
    glvm::dfrust frustum[renderer::_max_depth_passes];   // View frustrum
    int quads_culled[renderer::_max_depth_passes];       // Number of quads culled
    int quads_rendered[renderer::_max_depth_passes];     // Number of quads rendered
    int quads_approximated[renderer::_max_depth_passes]; // Number of quads approximated
    int lowest_quad_level[renderer::_max_depth_passes];  // Lowest quad level rendered
    int highest_quad_level[renderer::_max_depth_passes]; // Highest quad level rendered
    // Information about the pointer position
    glvm::dvec3 pointer_coord;                           // Cartesian coordinates, or 0
    // Information about the debug quad
    glvm::ivec4 debug_quad;                              // Quad coordinates
    float debug_quad_max_dist_to_quad_plane;             // Max dist to quad plane
    float debug_quad_min_elev;                           // Min elevation
    float debug_quad_max_elev;                           // Max elevation

    renderpass_info()
    {
        clear();
    }

    void clear()
    {
        depth_passes = 0;
        pointer_coord = glvm::dvec3(0.0);
        debug_quad = glvm::ivec4(-1);
    }

    void clear_depth_pass(int dp)
    {
        frustum[dp] = glvm::dfrust(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
        quads_culled[dp] = 0;
        quads_rendered[dp] = 0;
        quads_approximated[dp] = 0;
        lowest_quad_level[dp] = -1;
        highest_quad_level[dp] = -1;
    }
};


#endif

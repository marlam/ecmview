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

#ifndef RENDERER_PARAMETERS_H
#define RENDERER_PARAMETERS_H

#include "ser.h"


class renderer_parameters : public serializable
{
public:
    uint8_t background_color[3];    // used for glClearColor()
    float quad_screen_size_ratio;   // max allowed ratio between screen size of quad and quad size
    int fixed_quadtree_depth;       // 0 = off, > 0 = fixed level
    int quad_subdivision;           // >= 0
    bool wireframe;
    bool bounding_boxes;
    bool quad_borders;
    bool mipmapping;
    bool force_lod_sync;
    bool statistics_overlay;
    size_t gpu_cache_size;          // in bytes
    size_t mem_cache_size;          // in bytes

private:
    void reset();

public:
    renderer_parameters();

    void save(std::ostream& os) const;
    void load(std::istream& is);

    void save(std::ostream& os, const char* name) const;
    void load(const std::string& s);
};

#endif

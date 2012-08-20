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

#ifndef CULLER_H
#define CULLER_H

#include "glvm.h"


class culler
{
private:
    glvm::vec4 _plane[6];

    glvm::vec4 coeffs(float a, float b, float c, float d);

public:
    /* The culler works on viewer-relative cartesian coordinates, not on
     * planet-centric coordinates! */
    culler();
    
    /* Set the current ModelViewProjection matrix (for rendering with the viewer
     * in (0,0,0)). Required before calling any of the *_cull() functions below. */
    void set_mvp(const glvm::mat4 &MVP);

    /* Frustum culling of bounding boxes given by two quads bb0 and bb1. */
    bool frustum_cull(const glvm::vec3 bb0[4], const glvm::vec3 bb1[4]) const;
};

#endif

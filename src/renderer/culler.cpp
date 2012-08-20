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

#include "config.h"

#include <limits>

#include "dbg.h"

#include "glvm-str.h"

#include "culler.h"

using namespace glvm;


culler::culler()
{
#ifndef NDEBUG
    _plane[0].x = std::numeric_limits<float>::quiet_NaN();
#endif
}

vec4 culler::coeffs(float a, float b, float c, float d)
{
    vec3 normal = vec3(a, b, c);
    float len = length(normal);
    normal /= len;
    return vec4(normal, d / len);
}

void culler::set_mvp(const mat4& MVP)
{
    // See http://www.lighthouse3d.com/opengl/viewfrustum/index.php?clipspace
    // and http://www.lighthouse3d.com/opengl/viewfrustum/index.php?clipspaceimp

    // near
    _plane[0] = coeffs(
            MVP[0][2] + MVP[0][3],
            MVP[1][2] + MVP[1][3],
            MVP[2][2] + MVP[2][3],
            MVP[3][2] + MVP[3][3]);
    // far
    _plane[1] = coeffs(
            -MVP[0][2] + MVP[0][3],
            -MVP[1][2] + MVP[1][3],
            -MVP[2][2] + MVP[2][3],
            -MVP[3][2] + MVP[3][3]);
    // bottom
    _plane[2] = coeffs(
            MVP[0][1] + MVP[0][3],
            MVP[1][1] + MVP[1][3],
            MVP[2][1] + MVP[2][3],
            MVP[3][1] + MVP[3][3]);
    // top
    _plane[3] = coeffs(
            -MVP[0][1] + MVP[0][3],
            -MVP[1][1] + MVP[1][3],
            -MVP[2][1] + MVP[2][3],
            -MVP[3][1] + MVP[3][3]);
    // left
    _plane[4] = coeffs(
            MVP[0][0] + MVP[0][3],
            MVP[1][0] + MVP[1][3],
            MVP[2][0] + MVP[2][3],
            MVP[3][0] + MVP[3][3]);
    // right
    _plane[5] = coeffs(
            -MVP[0][0] + MVP[0][3],
            -MVP[1][0] + MVP[1][3],
            -MVP[2][0] + MVP[2][3],
            -MVP[3][0] + MVP[3][3]);
}

bool culler::frustum_cull(const vec3 bb0[4], const vec3 bb1[4]) const
{
    assert(isfinite(_plane[0].x));

    bool cull;
    for (int i = 0; i < 6; i++) {
        cull = true;
        for (int j = 0; j < 4; j++) {
            float dbb0 = _plane[i].w + dot(bb0[j], _plane[i].xyz());
            float dbb1 = _plane[i].w + dot(bb1[j], _plane[i].xyz());
            if (dbb0 >= 0.0f || dbb1 >= 0.0f) {
                cull = false;
                break;
            }
        }
        if (cull) {
            break;
        }
    }
    return cull;
}

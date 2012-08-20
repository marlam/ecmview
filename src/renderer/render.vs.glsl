/*
 * Copyright (C) 2011, 2012
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

#version 120

// LIGHTING
// NO_LIGHTING
#define $lighting

uniform sampler2D cart_coords;
uniform float cart_coords_texcoord_offset;
uniform float cart_coords_texcoord_factor;
uniform float cart_coords_halfstep;

#ifdef LIGHTING
varying vec3 P;
#endif

void main()
{
    vec2 q_orig = gl_Vertex.xy;
    vec2 q = clamp(q_orig, 0.0, 1.0);

    vec2 tc = cart_coords_texcoord_offset + cart_coords_texcoord_factor * q;
    if (q_orig.x < 0.0)
        tc.x = cart_coords_halfstep;
    else if (q_orig.x > 1.0)
        tc.x = 1.0 - cart_coords_halfstep;
    if (q_orig.y < 0.0)
        tc.y = cart_coords_halfstep;
    else if (q_orig.y > 1.0)
        tc.y = 1.0 - cart_coords_halfstep;
    vec3 cart_coord = texture2D(cart_coords, tc).rgb;

#ifdef LIGHTING
    P = cart_coord;
#endif

    gl_TexCoord[0] = vec4(q, 0.0, 0.0);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(cart_coord, 1.0);
}

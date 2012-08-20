/*
 * Copyright (C) 2009, 2010, 2011, 2012
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

uniform sampler2D data0_tex;
uniform sampler2D mask0_tex;
uniform sampler2D data1_tex;
uniform sampler2D mask1_tex;
uniform sampler2D offsets_tex;
uniform float quadcoord_offset;
uniform float quadcoord_factor;
uniform float texcoord0_offset;
uniform float texcoord0_factor;
uniform float texcoord1_offset;
uniform float texcoord1_factor;
uniform float base_data_texcoord_factor;
uniform float base_data_texcoord_offset;
uniform float base_data_mirror_x;
uniform float base_data_mirror_y;
uniform mat3 base_data_matrix;
uniform vec3 lens_pos;
uniform float lens_radius;
uniform vec3 quad_tl;
uniform vec3 quad_tr;
uniform vec3 quad_bl;
uniform vec3 quad_br;

void main()
{
    vec2 q = vec2(quadcoord_offset) + quadcoord_factor * gl_TexCoord[0].xy;
    q = vec2(q.x, 1.0 - q.y);

    vec3 cart_coord =
          quad_bl * (1.0 - q.x) * (1.0 - q.y)
        + quad_br * (      q.x) * (1.0 - q.y)
        + quad_tr * (      q.x) * (      q.y)
        + quad_tl * (1.0 - q.x) * (      q.y);

    vec2 base_data_texcoords = vec2(base_data_texcoord_factor)
        * vec2(mix(q.x, 1.0 - q.x, base_data_mirror_x), mix(q.y, 1.0 - q.y, base_data_mirror_y))
        + vec2(base_data_texcoord_offset);
    vec3 offset = texture2D(offsets_tex, base_data_texcoords).rgb;
    offset *= base_data_matrix;
    cart_coord += offset;

    float dist_to_lens_center = length(cart_coord - lens_pos);
    if (dist_to_lens_center > lens_radius) {
        vec2 q0 = vec2(texcoord0_offset) + texcoord0_factor * gl_TexCoord[0].xy;
        gl_FragData[0] = texture2D(data0_tex, q0);
        gl_FragData[1] = texture2D(mask0_tex, q0);
    } else {
        vec2 q1 = vec2(texcoord1_offset) + texcoord1_factor * gl_TexCoord[0].xy;
        gl_FragData[0] = texture2D(data1_tex, q1);
        gl_FragData[1] = texture2D(mask1_tex, q1);
    }
}

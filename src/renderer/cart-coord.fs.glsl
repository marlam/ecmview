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

uniform float step;
uniform float q_offset;
uniform float q_factor;

/* The quad corners in cartesian coordinates */
uniform vec3 quad_tl;
uniform vec3 quad_tr;
uniform vec3 quad_bl;
uniform vec3 quad_br;

/* Quad base data */
uniform bool have_base_data;
uniform vec3 fallback_normal;
uniform sampler2D offsets;
uniform sampler2D normals;
uniform float base_data_texcoord_factor;
uniform float base_data_texcoord_offset;
uniform float base_data_mirror_x;
uniform float base_data_mirror_y;
uniform mat3 base_data_matrix;
uniform float skirt_elevation;

/* Elevation data */
uniform bool have_elevation;
uniform float fallback_elevation;
uniform sampler2D elevation_data;
uniform sampler2D elevation_mask;
uniform float elevation_texcoord_factor;
uniform float elevation_texcoord_offset;

void main()
{
    vec2 t = gl_TexCoord[0].xy;
    vec2 q = q_offset + q_factor * t;
    bool skirt = (t.x < step || t.x > 1.0 - step
            || t.y < step || t.y > 1.0 - step);
    if (skirt) {
        q = clamp(q, 0.0, 1.0);
    }

    vec3 cart_coord =
          quad_bl * (1.0 - q.x) * (1.0 - q.y)
        + quad_br * (      q.x) * (1.0 - q.y)
        + quad_tr * (      q.x) * (      q.y)
        + quad_tl * (1.0 - q.x) * (      q.y);

    /* Base data */
    vec3 offset, normal;
    if (have_base_data) {
        vec2 base_data_texcoords = vec2(base_data_texcoord_factor)
            * vec2(mix(q.x, 1.0 - q.x, base_data_mirror_x), mix(q.y, 1.0 - q.y, base_data_mirror_y))
            + vec2(base_data_texcoord_offset);
        offset = texture2D(offsets, base_data_texcoords).rgb;
        vec2 nxy = texture2D(normals, base_data_texcoords).rg;
        normal = vec3(nxy, sqrt(max(0.0, 1.0 - nxy.x * nxy.x - nxy.y * nxy.y)));
        offset *= base_data_matrix;
        normal *= base_data_matrix;
    } else {
        offset = vec3(0.0);
        normal = fallback_normal;
    }

    /* Apply base data + elevation */
    cart_coord += offset;
    if (skirt) {
        cart_coord += skirt_elevation * normal;
    } else {
        vec2 elevation_texcoords = vec2(elevation_texcoord_factor)
            * vec2(q.x, 1.0 - q.y)
            + vec2(elevation_texcoord_offset);
        if (have_elevation && texture2D(elevation_mask, elevation_texcoords).r >= 0.5) {
            cart_coord += texture2D(elevation_data, elevation_texcoords).r * normal;
        } else {
            cart_coord += fallback_elevation * normal;
        }
    }

    gl_FragColor = vec4(cart_coord, 0.0);
}

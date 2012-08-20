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

// QUAD_BORDERS
// NO_QUAD_BORDERS
#define $quad_borders

/* Texture data */
uniform sampler2D texture_data;
uniform sampler2D texture_mask;
uniform float texture_texcoord_factor;
uniform float texture_texcoord_offset;

/* Lighting */
#ifdef LIGHTING
uniform sampler2D cart_coords;
uniform float cart_coords_texcoord_offset;
uniform float cart_coords_texcoord_factor;
uniform float cart_coords_step;
uniform vec3 L;
uniform vec4 ambient_color;
uniform vec4 light_color;
uniform float shininess;
varying vec3 P;
#endif

/* Quad Borders */
#ifdef QUAD_BORDERS
uniform int quad_side;
uniform int quad_level;
uniform int quad_x;
uniform int quad_y;
uniform int quads_in_level;
uniform float quad_border_thickness;
uniform vec3 quad_border_color;
uniform float side_border_thickness;
uniform vec3 side_border_color;
#endif

void main()
{
    vec2 q = gl_TexCoord[0].xy;

    vec2 texture_texcoords = vec2(texture_texcoord_factor)
        * vec2(q.x, 1.0 - q.y)
        + vec2(texture_texcoord_offset);

    float mask = texture2D(texture_mask, texture_texcoords).r;
    if (mask < 0.5)
        discard;

#ifdef LIGHTING
    vec2 t = cart_coords_texcoord_offset + cart_coords_texcoord_factor * q;
    vec3 P0 = texture2D(cart_coords, t + vec2(0.0, +cart_coords_step)).rgb;
    vec3 P1 = texture2D(cart_coords, t + vec2(0.0, -cart_coords_step)).rgb;
    vec3 P2 = texture2D(cart_coords, t + vec2(+cart_coords_step, 0.0)).rgb;
    vec3 P3 = texture2D(cart_coords, t + vec2(-cart_coords_step, 0.0)).rgb;
    vec3 N = normalize(-cross(P0 - P1, P2 - P3));
    vec4 material_color = texture2D(texture_data, texture_texcoords);
    vec4 diffuse = clamp(material_color * light_color
            * max(dot(N, L), 0.0), 0.0, 1.0);
    vec3 H = normalize(L + normalize(-P));
    vec4 specular = clamp(material_color * light_color
            * pow(max(dot(N, H), 0.0), shininess), 0.0, 1.0);
    vec4 result = diffuse + specular + ambient_color;
#else
    vec4 result = texture2D(texture_data, texture_texcoords);
#endif

#ifdef QUAD_BORDERS
    /*
    if (quad_x == 0 && q.x < quad_border_thickness
            || quad_y == 0 && q.y > 1.0 - quad_border_thickness
            || quad_x == quads_in_level - 1 && q.x > 1.0 - quad_border_thickness
            || quad_y == quads_in_level - 1 && q.y < quad_border_thickness) {
        result = vec4(quad_border_color, 1.0);
    }
    */
    if (q.x < quad_border_thickness
            || q.y < quad_border_thickness
            || q.x > 1.0 - quad_border_thickness
            || q.y > 1.0 - quad_border_thickness) {
        result = vec4(quad_border_color, 1.0);
    }
#endif

    gl_FragColor = result;
}

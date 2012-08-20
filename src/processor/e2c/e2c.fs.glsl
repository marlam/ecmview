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

// ISOLINES
// NO_ISOLINES
#define $isolines

uniform sampler2D data_tex;
uniform sampler2D mask_tex;
uniform sampler2D gradient_tex;

uniform float global_min_elev;
uniform float global_max_elev;
uniform float adapt_brightness; // 0.0f or 1.0f
#if defined(ISOLINES)
uniform float step;
uniform float isolines_distance;
uniform float isolines_thickness;
uniform vec3 isolines_color;
#endif

void main()
{
    /* +--+--+--+
     * |d0|d1|d2|
     * +--+--+--+
     * |d3|d4|d5|
     * +--+--+--+
     * |d6|d7|d8|
     * +--+--+--+
     */
    float d4 = texture2D(data_tex, gl_TexCoord[0].xy).r;
#if defined(ISOLINES)
    float d1 = texture2D(data_tex, gl_TexCoord[0].xy + vec2(0.0, - isolines_thickness * step)).r;
    float d3 = texture2D(data_tex, gl_TexCoord[0].xy + vec2(- isolines_thickness * step, 0.0)).r;
#endif
    float m4 = texture2D(mask_tex, gl_TexCoord[0].xy).r;
#if defined(ISOLINES)
    float m1 = texture2D(mask_tex, gl_TexCoord[0].xy + vec2(0.0, - isolines_thickness * step)).r;
    float m3 = texture2D(mask_tex, gl_TexCoord[0].xy + vec2(- isolines_thickness * step, 0.0)).r;
#endif

    float d = (d4 - global_min_elev) / (global_max_elev - global_min_elev);
    vec3 rgb = texture2D(gradient_tex, vec2(d, 0.5)).rgb;
    rgb = mix(rgb, d * rgb, adapt_brightness);

#if defined(ISOLINES)
    float d4_isoline_class = floor(d4 / isolines_distance);
    float d1_isoline_class = floor(d1 / isolines_distance);
    float d3_isoline_class = floor(d3 / isolines_distance);
    if ((m1 >= 0.5 && d4_isoline_class != d1_isoline_class)
            || (m3 >= 0.5 && d4_isoline_class != d3_isoline_class)) {
        rgb = isolines_color;
    }
#endif

    gl_FragData[0] = vec4(rgb, 1.0);
    gl_FragData[1] = vec4(m4, m4, m4, m4);
}

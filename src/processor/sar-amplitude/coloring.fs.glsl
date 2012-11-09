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

uniform sampler2D data_tex;
uniform sampler2D mask_tex;
uniform sampler2D gradient_tex;

uniform float adapt_brightness; // 0.0f or 1.0f

void main()
{
    float amp = texture2D(data_tex, gl_TexCoord[0].xy).r;
    float m = texture2D(mask_tex, gl_TexCoord[0].xy).r;

    vec3 rgb = texture2D(gradient_tex, vec2(amp, 0.5)).rgb;
    rgb = mix(rgb, amp * rgb, adapt_brightness);

    gl_FragData[0] = vec4(rgb, 1.0);
    gl_FragData[1] = vec4(m, m, m, m);
}

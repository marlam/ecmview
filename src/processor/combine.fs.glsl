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

const int n = $n;

// The following needs to be sorted by priority, lowest first!
uniform sampler2D data_texs[n];
uniform sampler2D mask_texs[n];
uniform float priorities[n];
uniform float weights[n];
uniform float texcoord_offsets[n];
uniform float texcoord_factors[n];

void main()
{
    gl_FragData[0] = vec4(0.0);
    gl_FragData[1] = vec4(0.0);

    float old_priority = priorities[0];
    float weight_sum = 0.0;
    vec4 data = vec4(0.0, 0.0, 0.0, 0.0);
    for (int i = 0; i < n; i++) {
        vec2 tc = vec2(texcoord_offsets[i]) + texcoord_factors[i] * gl_TexCoord[0].xy;
        vec4 old_data = texture2D(data_texs[i], tc);
        float old_mask = texture2D(mask_texs[i], tc).r;
        if (old_mask >= 0.5) {
            data += weights[i] * old_data;
            weight_sum += weights[i];
        }
        if (i == n - 1 || priorities[i + 1] > old_priority) {
            if (weight_sum > 0.0) {
                gl_FragData[0] = data / weight_sum;
                gl_FragData[1] = vec4(1.0);
            }
            weight_sum = 0.0;
            data = vec4(0.0);
            if (i < n - 1) {
                old_priority = priorities[i + 1];
            }
        }
    }
}

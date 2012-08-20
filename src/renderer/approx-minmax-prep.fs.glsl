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

#define pos_huge_val $pos_huge_val
#define neg_huge_val $neg_huge_val

uniform sampler2D data;
uniform sampler2D mask;

uniform float in_size;
uniform float out_size;

void main()
{
    vec2 out_index = floor(gl_TexCoord[0].xy * vec2(out_size));
    vec2 in0_index = 2.0 * out_index;
    vec2 in1_index = in0_index + vec2(1.0);
    vec2 in0 = (in0_index + vec2(0.5)) / vec2(in_size);
    vec2 in1 = (in1_index + vec2(0.5)) / vec2(in_size);

    vec2 p[4];
    p[0] = vec2(in0.x, in0.y);
    p[1] = vec2(in1.x, in0.y);
    p[2] = vec2(in0.x, in1.y);
    p[3] = vec2(in1.x, in1.y);

    float min_elev = pos_huge_val;
    float max_elev = neg_huge_val;
    float m = 0.0f;
    for (int i = 0; i < 4; i++) {
        float mm = (p[i].x < 1.0 && p[i].y < 1.0) ? texture2D(mask, p[i]).r : 0.0;
        if (mm > 0.5) {
            float elev = texture2D(data, p[i]).r;
            if (elev < min_elev)
                min_elev = elev;
            if (elev > max_elev)
                max_elev = elev;
            m = 1.0f;
        }
    }

    gl_FragColor = vec4(min_elev, max_elev, m, 1.0);
}

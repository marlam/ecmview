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

#define pos_huge_val $pos_huge_val
#define neg_huge_val $neg_huge_val

uniform sampler2D tex;
uniform vec2 step;

void main()
{
    vec2 c0 = (gl_TexCoord[0].xy - step / 2.0);
    vec2 c1 = c0 + step;

    vec2 p[4];
    p[0] = vec2(c0.x, c0.y);
    p[1] = vec2(c1.x, c0.y);
    p[2] = vec2(c0.x, c1.y);
    p[3] = vec2(c1.x, c1.y);
    
    float min_elev = pos_huge_val;
    float max_elev = neg_huge_val;
    float m = 0.0f;
    for (int i = 0; i < 4; i++) {
        vec3 v = texture2D(tex, p[i]).rgb;
        if (v.b > 0.5) {
            if (v.r < min_elev)
                min_elev = v.r;
            if (v.g > max_elev)
                max_elev = v.g;
            m = 1.0f;
        }
    }
    
    gl_FragColor = vec4(min_elev, max_elev, m, 1.0);
}

/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

uniform sampler2D tex;
uniform float L;

void main()
{
    vec4 oldval = texture2D(tex, gl_TexCoord[0].xy);
    float mean = oldval.b;
    float stddev = sqrt(oldval.a);
    float newval;

    float Cu = 1.0 / sqrt(L);
    float Ci = stddev / mean;
    float Cm = sqrt(2.0) / Cu;

    if (Ci <= Cu) {
        newval = mean;
    } else if (Ci < Cm) {
        float alpha = (1.0 + Cu * Cu) / (Ci * Ci - Cu * Cu);
        float B = alpha - L - 1.0;
        float D = mean * mean * B * B + 4.0 * alpha * L * mean * oldval.r;
        newval = (B * mean + sqrt(D)) / (2.0 * alpha);
    } else {
        newval = oldval.r;
    }

    gl_FragColor = vec4(sqrt(newval), 0.0, 0.0, 0.0);
}

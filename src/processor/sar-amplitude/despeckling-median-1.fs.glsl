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

/*
 * This is an approximate median filter. Comparisons are based on the amplitude values.
 *
 * From a twodimensional array of values, it computes the median values of the
 * columns, and then in a second step the median of these values. This is usually
 * close to the real median.
 */

const int kv = $kv;
uniform vec2 step;
uniform sampler2D tex;

void main()
{
    float row[2 * kv + 1], tmp;

    for (int r = -kv; r <= +kv; r++) {
        row[r + kv] = texture2D(tex, gl_TexCoord[0].xy + vec2(0, r) * step).r;
    }
    for (int i = 0; i < 2 * kv + 1; i++) {
        for (int j = 0; j < 2 * kv; j++) {
            if (row[j] > row[j + 1]) {
                tmp = row[j + 1];
                row[j + 1] = row[j];
                row[j] = tmp;
            }
        }
    }

    gl_FragColor = vec4(row[kv], 0.0, 0.0, 0.0);
}

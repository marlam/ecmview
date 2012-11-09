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
 * Compute mean and variance of a local neighborhood.
 * Step 2: vertical direction
 */

const int kh = $kh;
const int kv = $kv;
const int masksize = (2 * kh + 1) * (2 * kv + 1);
uniform vec2 step;
uniform sampler2D tex;

void main()
{
    float oldval = texture2D(tex, gl_TexCoord[0].xy).r;
    float sum = 0.0;
    float sum2 = 0.0;
    for (int r = -kv; r <= +kv; r++) {
        vec4 v = texture2D(tex, gl_TexCoord[0].xy + vec2(0, r) * step);
        sum += v.b;
        sum2 += v.a;
    }
    float mean = sum / float(masksize);
    // The sample variance should be computed by dividing through
    // (masksize - 1), but it makes no noticable difference in practice to
    // divide through (masksize), and that also works for masksize == 1.
    float var = max(0.0, (sum2 - sum * sum / float(masksize)) / float(masksize));
    gl_FragColor = vec4(oldval, 0.0, mean, var);
}

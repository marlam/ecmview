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

const int kv = $kv;
uniform vec2 step;
uniform float mask_v[2 * kv + 1];
uniform float factor_v;
uniform sampler2D tex;

void main()
{
    float v = 0.0;
    for (int r = -kv; r <= +kv; r++) {
        v += mask_v[r + kv] * texture2D(tex, gl_TexCoord[0].xy + vec2(0, r) * step).r;
    }
    gl_FragColor = vec4(factor_v * v, 0.0, 0.0, 0.0);
}

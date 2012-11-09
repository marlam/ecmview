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
uniform float l;

void main()
{
    vec4 oldval = texture2D(tex, gl_TexCoord[0].xy);
    float mean = oldval.b;
    float var = oldval.a;
    float var_orig = (l * var - mean * mean) / (l + 1.0);
    float newval = mean;
    if (var > 0.0 && var_orig >= 0.0) {
        newval += var_orig * (oldval.r - mean) / (var_orig + (mean * mean + var_orig) / l);
    }
    gl_FragColor = vec4(newval, 0.0, 0.0, 0.0);
}

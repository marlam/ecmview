/*
 * Copyright (C) 2007, 2008, 2009, 2010, 2011, 2012
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
uniform float p;

void main()
{
    // See HDRI book by Reinhard, Ward, Pattanaik, Debevec
    float amp = texture2D(tex, gl_TexCoord[0].xy).r;
    float g = (p * amp) / ((p - 1.0) * amp + 1.0);
    g = clamp(g, 0.0, 1.0);
    gl_FragColor = vec4(g, 0.0, 0.0, 0.0);
}

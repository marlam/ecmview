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

uniform sampler2D data_tex;
uniform sampler2D mask_tex;
uniform float data_offset;
uniform float data_factor;
uniform float scale_factor;
uniform float scale_center;
uniform float min_elev;
uniform float max_elev;

void main()
{
    float data = texture2D(data_tex, gl_TexCoord[0].xy).r;
    float mask = texture2D(mask_tex, gl_TexCoord[0].xy).r;
    data = data_offset + data_factor * data;
    float d = clamp((data - scale_center) * scale_factor + scale_center, min_elev, max_elev);
    gl_FragData[0] = vec4(d, d, d, d);
    gl_FragData[1] = vec4(mask, mask, mask, mask);
}

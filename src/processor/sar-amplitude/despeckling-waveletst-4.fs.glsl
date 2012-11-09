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
#extension GL_EXT_gpu_shader4 : enable

uniform sampler2D tex;
uniform int texwidth;
uniform int texheight;
float a_coeff[6] = float[] (
	// Coefficients for Approach part of the D6 wavelet
	+0.33267055295008261599851158914,
	+0.80689150931109257649449360409,
	+0.45987750211849157009515194215,
	-0.13501102001025458869638990670,
	-0.08544127388202666169281916918,
	+0.03522629188570953660274066472
	);
float d_coeff[6] = float[] (
	// Coefficients for Details part of the D6 wavelet
	+0.03522629188570953660274066472,
	+0.08544127388202666169281916918,
	-0.13501102001025458869638990670,
	-0.45987750211849157009515194215,
	+0.80689150931109257649449360409,
	-0.33267055295008261599851158914
	);

void main()
{
    float xtex = gl_TexCoord[0].x;
    float ytex = gl_TexCoord[0].y;
    int x = int(round((xtex - 0.5 / float(texwidth)) * float(texwidth)));
    float result = 0.0;
    float index;
    float a, d;
    // The awkward loop organization is necessary because we must access
    // a_coeff and d_coeff with indices that are known at compile time.
    if (x % 2 == 0)
    {
	for (int c = 0; c <= 4; c += 2)
	{
	    float index = (float(x / 2 - c / 2) + 0.5) / float(texwidth);
	    float a = texture2D(tex, vec2(index, ytex)).g;
	    float d = texture2D(tex, vec2(index + 0.5, ytex)).g; 
	    result += a_coeff[c] * a + d_coeff[c] * d;
	}
    }
    else
    {
	for (int c = 1; c <= 5; c += 2)
	{
	    float index = (float(x / 2 - c / 2) + 0.5) / float(texwidth);
	    float a = texture2D(tex, vec2(index, ytex)).g;
	    float d = texture2D(tex, vec2(index + 0.5, ytex)).g; 
	    result += a_coeff[c] * a + d_coeff[c] * d;
	}
    }
    // Reverse the logarithmic transformation from step 1
    result = exp(result) - 1.0;
    gl_FragColor = vec4(result, 0.0, 0.0, 0.0);
}

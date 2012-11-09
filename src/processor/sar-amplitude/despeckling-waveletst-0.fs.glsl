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

uniform float xstep;
uniform sampler2D tex;
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
    float oldval = texture2D(tex, gl_TexCoord[0].xy).r;
    float xtex = gl_TexCoord[0].x;
    float ytex = gl_TexCoord[0].y;
    float result = 0.0;
    if (xtex < 0.5)	// Approach
    {
	for (int c = 0; c < 6; c++)
	{
    	    // Use logarithmic values to transform multiplicative speckle to additive noise.
	    result += a_coeff[c] * log(1.0 + texture2D(tex, vec2(2.0 * xtex + float(c) * xstep, ytex)).r);
	}
    }
    else		// Details
    {
	for (int c = 0; c < 6; c++)
	{
    	    // Use logarithmic values to transform multiplicative speckle to additive noise.
	    result += d_coeff[c] * log(1.0 + texture2D(tex, vec2(2.0 * (xtex - 0.5) + float(c) * xstep, ytex)).r);
	}
    }
    gl_FragColor = vec4(oldval, result, 0.0, 0.0);
}

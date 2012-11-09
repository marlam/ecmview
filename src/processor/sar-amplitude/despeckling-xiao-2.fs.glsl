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

/*
 * This implements
 *
 * J. Xiao, J. Li, A. Moody
 * A detail-preserving and flexible adaptive filter for speckle suppression in
 * SAR imagery
 * Int. J. Remote Sensing, 2003, vol. 24, no. 12, 2451-2465.
 *
 * Note that Tmin and Tmax must be given as parameters because the
 * framework allows only local operators by design.
 */

#version 120

uniform float Tmin;
uniform float Tmax;
uniform float a;
uniform float b;
uniform sampler2D tex;

void main()
{
    vec4 oldval = texture2D(tex, gl_TexCoord[0].xy);
    float mean = oldval.b;
    float stddev = sqrt(oldval.a);

    /* t-Test */
    float x0 = oldval.r;
    float T = (stddev == 0.0) ? 0.0 : (x0 - mean) / stddev;
    float Tn = min(1.0, max(0.0, (T - Tmin) / (Tmax - Tmin)));
    float p = (1.0 - Tn);
    float k = 0.0;
    if (p >= b)
    {
        k = 1.0;
    }
    else if (b > a && p > a)
    {
        k = (p - a) / (b - a);
    }

    /* Weighted average between mean and old value */
    gl_FragColor = vec4(k * x0 + (1.0 - k) * mean, oldval.g, 0.0, 0.0);
}

/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012
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

const float PI = 3.14159265358979323846;
uniform vec2 step;
uniform sampler2D tex;
uniform float min_amp;
const float max_amp = 1.0;
uniform float avg_amp;
uniform float b;
uniform float l;
uniform float d;
uniform float threshold;
uniform float m;

void main()
{
    vec4 orig = texture2D(tex, gl_TexCoord[0].xy);
    float orig_amp = orig.r;

    const int maxdist = 9;
    const int increment = 1;
    float g[maxdist / increment + 1];
    float w[maxdist / increment + 1];
    g[0] = orig_amp;
    for (int d = 1; d <= maxdist - 3; d += increment)
    {
        g[d / increment] = 0.0;
        w[d / increment] = 0.0;
        float s = float(d) / 2.5;
        for (int r = -d; r <= +d; r++)
        {
            for (int c = -d; c <= +d; c++)
            {
                float weight = exp(- (float(r * r) + float(c * c)) / (2.0 * s * s)) / (sqrt(2.0 * PI) * s);
                g[d / increment] += weight * texture2D(tex, gl_TexCoord[0].xy + vec2(c, r) * step).r;
                w[d / increment] += weight;
            }
        }
        g[d / increment] /= w[d / increment];
    }
    g[maxdist / increment - 2] = orig.g;
    g[maxdist / increment - 1] = orig.b;
    g[maxdist / increment] = orig.a;

    float v[maxdist / increment];
    for (int i = 0; i < maxdist / increment; i++)
    {
        v[i] = abs((g[i] - g[i + 1]) / g[i]);
    }
    float local_avg = g[0];
    float t = abs(threshold);
    if (threshold >= 0.0)
    {
        for (int i = maxdist / increment - 1; i >= 0; i--)
        {
            if (v[i] > t)
            {
                local_avg = g[i + 1];
                break;
            }
        }
    }
    else
    {
        for (int i = 0; i < maxdist / increment; i++)
        {
            if (v[i] < t)
            {
                local_avg = g[i + 1];
            }
            else
            {
                break;
            }
        }
    }

    float La = l * ((1.0 - d) * orig_amp + d * local_avg) + (1.0 - l) * avg_amp;
    float x = orig_amp / (orig_amp + pow(b * La, m));

    x = clamp(x, 0.0, 1.0);
    gl_FragColor = vec4(x, 0.0, 0.0, 0.0);
}

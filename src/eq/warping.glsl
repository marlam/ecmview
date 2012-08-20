/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2012
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

// See A. Kolb, "Correction of the Projective Error for Cylinders",
// Technical Report, 2008

uniform vec3 P;         // Viewer position
uniform vec3 O;         // Lower left corner of wall segment
uniform vec3 U;         // Lower right corner of wall segment
uniform vec3 u;         // = U - O
uniform vec3 v;         // = (Upper left corner) - O
uniform vec3 n;         // = cross(u,v)
uniform float radius;   // Radius
// For original version of 2.2 (see below):
//uniform vec3 P_org;   // Original viewer position
//uniform float r2;     // Radius * Radius

uniform sampler2D tex;

void main()
{
    /* 1. */
    float x_org = gl_TexCoord[0].x;
    float y_org = gl_TexCoord[0].y;

    /* 2.1 */
    vec3 Q_plane_org = O + x_org * u + y_org * v;

    /* 2.2 */
    /* Original version, for arbitrary P_org:
    vec3 s = Q_plane_org - P_org;
    vec3 P_org_xz = vec3(P_org.x, 0.0, P_org.z);
    vec3 s_xz = vec3(s.x, 0.0, s.z);
    float s_xz_len = length(s_xz);
    s_xz /= s_xz_len;
    s /= s_xz_len;
    float sP = dot(s_xz, P_org_xz);
    float alpha = -sP + sqrt(sP * sP - dot(P_org_xz, P_org_xz) + r2);
    vec3 Q_cly = P_org + alpha * s;
    */
    // Optimized version for P_org = (0.0, 1.78, 0.0):
    vec3 s = Q_plane_org - vec3(0.0, 1.78, 0.0);
    s /= length(vec3(s.x, 0.0, s.z));
    vec3 Q_cly = vec3(0.0, 1.78, 0.0) + radius * s;

    /* 2.3 */
    vec3 t = Q_cly - P;
    float beta = dot(n, O - P) / dot(n, t);
    vec3 Q_plane = P + beta * t;

    /* 2.4 */
    float x = dot(Q_plane - O, u) / dot(u, u);
    float y = dot(Q_plane - O, v) / dot(v, v);

    /* 3. */
    gl_FragColor = texture2D(tex, vec2(x, y));
}

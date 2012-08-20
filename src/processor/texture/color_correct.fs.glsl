/*
 * Copyright (C) 2011, 2012
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

// NEED_COLOR_CORRECTION
// DONT_NEED_COLOR_CORRECTION
#define $color_correction

uniform sampler2D data_tex;
uniform sampler2D mask_tex;

#if defined(NEED_COLOR_CORRECTION)

uniform float contrast;
uniform float brightness;
uniform float saturation;
uniform float cos_hue;
uniform float sin_hue;

vec3 srgb_to_yuv(vec3 srgb)
{
    // According to ITU.BT-601 (see formulas in Sec. 2.5.1 and 2.5.2)
    mat3 m = mat3(
            0.299, -0.168736,  0.5,
            0.587, -0.331264, -0.418688,
            0.114,  0.5,      -0.081312);
    return m * srgb + vec3(0.0, 0.5, 0.5);
}

vec3 yuv_to_srgb(vec3 yuv)
{
    // According to ITU.BT-601 (see formulas in Sec. 2.5.1 and 2.5.2)
    mat3 m = mat3(
            1.0,    1.0,      1.0,
            0.0,   -0.344136, 1.772,
            1.402, -0.714136, 0.0);
    return m * (yuv - vec3(0.0, 0.5, 0.5));
}

vec3 adjust_yuv(vec3 yuv)
{
    // Adapted from http://www.silicontrip.net/~mark/lavtools/yuvadjust.c
    // (Copyright 2002 Alfonso Garcia-Patiño Barbolani, released under GPLv2 or later)

    // brightness and contrast
    float ay = (yuv.x - 0.5) * (contrast + 1.0) + brightness + 0.5;
    // hue and saturation
    float au = (cos_hue * (yuv.y - 0.5) - sin_hue * (yuv.z - 0.5)) * (saturation + 1.0) + 0.5;
    float av = (sin_hue * (yuv.y - 0.5) + cos_hue * (yuv.z - 0.5)) * (saturation + 1.0) + 0.5;

    return vec3(ay, au, av);
}

#endif

vec3 rgb_to_srgb(vec3 rgb)
{
    // Only an approximation; see GL_ARB_framebuffer_sRGB for correct formula
    return pow(rgb, vec3(1.0 / 2.2));
}

vec3 srgb_to_rgb(vec3 srgb)
{
    // Only an approximation; see GL_EXT_TEXTURE_sRGB for correct formula
    return pow(srgb, vec3(2.2));
}

void main()
{
    vec3 data = texture2D(data_tex, gl_TexCoord[0].xy).rgb;
    float mask = texture2D(mask_tex, gl_TexCoord[0].xy).r;

#if defined(NEED_COLOR_CORRECTION)
    vec3 srgb = rgb_to_srgb(data);
    vec3 yuv = srgb_to_yuv(srgb);
    vec3 adjusted_yuv = adjust_yuv(yuv);
    vec3 adjusted_srgb = yuv_to_srgb(adjusted_yuv);
    vec3 result = srgb_to_rgb(adjusted_srgb);
#else
    vec3 result = data;
#endif
    gl_FragData[0] = vec4(result, 1.0);
    gl_FragData[1] = vec4(mask, mask, mask, mask);
}

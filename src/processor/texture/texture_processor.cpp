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

#include "config.h"

#include <cmath>

#include "str.h"

#include "glvm-gl.h"
#include "xgl.h"

#include "texture_processor.h"
#include "texture/color_correct.fs.glsl.h"

using namespace glvm;


texture_processor::texture_processor() : _noop_prg(0), _prg(0)
{
}

void texture_processor::init_gl()
{
}

void texture_processor::exit_gl()
{
    if (_noop_prg != 0) {
        xgl::DeleteProgram(_noop_prg);
        _noop_prg = 0;
    }
    if (_prg != 0) {
        xgl::DeleteProgram(_prg);
        _prg = 0;
    }
}

bool texture_processor::processing_is_necessary(
        unsigned int /* frame */,
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */)
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    return (pp.texture.contrast < 0.0f || pp.texture.contrast > 0.0f)
        || (pp.texture.brightness < 0.0f || pp.texture.brightness > 0.0f)
        || (pp.texture.saturation < 0.0f || pp.texture.saturation > 0.0f)
        || (pp.texture.hue < 0.0f || pp.texture.hue > 0.0f);
}

void texture_processor::process(
        unsigned int frame,
        const database_description& dd, bool lens,
        const glvm::ivec4& quad,
        const ecmdb::metadata& quad_meta,
        bool* /* full_validity */,
        ecmdb::metadata* meta)
{
    assert(xgl::CheckError(HERE));

    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];

    bool need_color_correction = processing_is_necessary(frame, dd, lens, quad, quad_meta);

    if (need_color_correction) {
        if (_prg == 0) {
            std::string src(COLOR_CORRECT_FS_GLSL_STR);
            str::replace(src, "$color_correction", "NEED_COLOR_CORRECTION");
            _prg = xgl::CreateProgram("texture-color-correct", "", "", src);
            xgl::LinkProgram("texture-color-correct", _prg);
            glUseProgram(_prg);
            glvmUniform(xgl::GetUniformLocation(_prg, "data_tex"), 0);
            glvmUniform(xgl::GetUniformLocation(_prg, "mask_tex"), 1);
            _contrast_loc = xgl::GetUniformLocation(_prg, "contrast");
            _brightness_loc = xgl::GetUniformLocation(_prg, "brightness");
            _saturation_loc = xgl::GetUniformLocation(_prg, "saturation");
            _cos_hue_loc = xgl::GetUniformLocation(_prg, "cos_hue");
            _sin_hue_loc = xgl::GetUniformLocation(_prg, "sin_hue");
            assert(xgl::CheckError(HERE));
        }
        glUseProgram(_prg);
        glvmUniform(_contrast_loc, pp.texture.contrast);
        glvmUniform(_brightness_loc, pp.texture.brightness);
        glvmUniform(_saturation_loc, pp.texture.saturation);
        glvmUniform(_cos_hue_loc, std::cos(pp.texture.hue * const_pi<float>()));
        glvmUniform(_sin_hue_loc, std::sin(pp.texture.hue * const_pi<float>()));
    } else {
        if (_noop_prg == 0) {
            std::string src(COLOR_CORRECT_FS_GLSL_STR);
            str::replace(src, "$color_correction", "DONT_NEED_COLOR_CORRECTION");
            _noop_prg = xgl::CreateProgram("texture-color-correct-noop", "", "", src);
            xgl::LinkProgram("texture-color-correct-noop", _noop_prg);
            glUseProgram(_noop_prg);
            glvmUniform(xgl::GetUniformLocation(_noop_prg, "data_tex"), 0);
            glvmUniform(xgl::GetUniformLocation(_noop_prg, "mask_tex"), 1);
            assert(xgl::CheckError(HERE));
        }
        glUseProgram(_noop_prg);
    }
    float step = 1.0f / dd.db.total_quad_size(); 
    float t = step * (dd.db.overlap() - 1);
    float s = step * (dd.db.quad_size() + 2);
    xgl::DrawQuad(-1.0f, -1.0f, 2.0f, 2.0f, t, t, s, s); 
    assert(xgl::CheckError(HERE));
    *meta = quad_meta;
}

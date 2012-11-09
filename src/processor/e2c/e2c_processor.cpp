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

#include "e2c_processor.h"
#include "e2c/e2c.fs.glsl.h"

using namespace glvm;


e2c_processor::e2c_processor() :
    _last_frame(-1), _min_elev(0.0f), _max_elev(0.0f),
    _prg(0), _prg_isolines(0), _pbo(0), _gradient_tex(0)
{
}

void e2c_processor::init_gl()
{
}

void e2c_processor::exit_gl()
{
    if (_prg != 0) {
        xgl::DeleteProgram(_prg);
        _prg = 0;
    }
    if (_prg_isolines != 0) {
        xgl::DeleteProgram(_prg_isolines);
        _prg_isolines = 0;
    }
    if (_pbo != 0) {
        glDeleteBuffers(1, &_pbo);
        _pbo = 0;
    }
    if (_gradient_tex != 0) {
        glDeleteTextures(1, &_gradient_tex);
        _gradient_tex = 0;
    }
}

void e2c_processor::set_e2c_info(int quad_size, float min_elev, float max_elev)
{
    _quad_size = quad_size;
    _min_elev = min_elev;
    _max_elev = max_elev;
}

bool e2c_processor::processing_is_necessary(
        unsigned int /* frame */,
        const database_description& /* dd */, bool /* lens */,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */)
{
    return true;
}

void e2c_processor::process(
        unsigned int frame,
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& quad_meta,
        bool* /* full_validity */,
        ecmdb::metadata* meta)
{
    assert(xgl::CheckError(HERE));

    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    bool need_gradient_upload = (frame != _last_frame);
    bool need_isolines = (pp.e2c.isolines_distance > 0.0f && pp.e2c.isolines_thickness > 0.0f);

    if (!need_isolines && _prg == 0) {
        std::string src(E2C_FS_GLSL_STR);
        str::replace(src, "$isolines", "NO_ISOLINES");
        _prg = xgl::CreateProgram("e2c", "", "", src);
        xgl::LinkProgram("e2c", _prg);
        glUseProgram(_prg);
        glvmUniform(xgl::GetUniformLocation(_prg, "data_tex"), 0);
        glvmUniform(xgl::GetUniformLocation(_prg, "mask_tex"), 1);
        glvmUniform(xgl::GetUniformLocation(_prg, "gradient_tex"), 2);
        assert(xgl::CheckError(HERE));
    }
    if (need_isolines && _prg_isolines == 0) {
        std::string src(E2C_FS_GLSL_STR);
        str::replace(src, "$isolines", "ISOLINES");
        _prg_isolines = xgl::CreateProgram("e2c-isolines", "", "", src);
        xgl::LinkProgram("e2c-isolines", _prg_isolines);
        glUseProgram(_prg_isolines);
        glvmUniform(xgl::GetUniformLocation(_prg_isolines, "data_tex"), 0);
        glvmUniform(xgl::GetUniformLocation(_prg_isolines, "mask_tex"), 1);
        glvmUniform(xgl::GetUniformLocation(_prg_isolines, "gradient_tex"), 2);
        assert(xgl::CheckError(HERE));
    }
    if (_pbo == 0) {
        glGenBuffers(1, &_pbo);
    }
    if (_gradient_tex == 0 || xgl::GetTex2DParameter(_gradient_tex, GL_TEXTURE_WIDTH) != pp.e2c.gradient_length) {
        need_gradient_upload = true;
        if (_gradient_tex != 0) {
            glDeleteTextures(1, &_gradient_tex);
        }
        _gradient_tex = xgl::CreateTex2D(GL_SRGB8, pp.e2c.gradient_length, 1, GL_LINEAR);
        assert(xgl::CheckError(HERE));
    }
    GLuint prg = need_isolines ? _prg_isolines : _prg;
    glUseProgram(prg);

    GLint input_tex_size;
    glActiveTexture(GL_TEXTURE0);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &input_tex_size);
    glvmUniform(xgl::GetUniformLocation(prg, "global_min_elev"), _min_elev);
    if (equal(_max_elev, _min_elev)) {
        glvmUniform(xgl::GetUniformLocation(prg, "global_max_elev"), _min_elev + 1.0f);
    } else {
        glvmUniform(xgl::GetUniformLocation(prg, "global_max_elev"), _max_elev);
    }
    glvmUniform(xgl::GetUniformLocation(prg, "adapt_brightness"), pp.e2c.adapt_brightness ? 1.0f : 0.0f);
    if (need_isolines) {
        glvmUniform(xgl::GetUniformLocation(prg, "step"), 1.0f / input_tex_size);
        glvmUniform(xgl::GetUniformLocation(prg, "isolines_distance"), pp.e2c.isolines_distance);
        glvmUniform(xgl::GetUniformLocation(prg, "isolines_thickness"), pp.e2c.isolines_thickness);
        vec3 isolines_color = vec3(vector<uint8_t, 3>(pp.e2c.isolines_color)) / vec3(255.0f, 255.0f, 255.0f);
        glvmUniform(xgl::GetUniformLocation(prg, "isolines_color"), isolines_color);
    }
    assert(xgl::CheckError(HERE));
    if (need_gradient_upload) {
        xgl::WriteTex2D(_gradient_tex, 0, 0, pp.e2c.gradient_length, 1, GL_RGB, GL_UNSIGNED_BYTE,
                pp.e2c.gradient_length * 3 * sizeof(uint8_t), pp.e2c.gradient, _pbo);
        assert(xgl::CheckError(HERE));
    }
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _gradient_tex);
    assert(xgl::CheckError(HERE));

    assert(input_tex_size - _quad_size >= 0);
    assert((input_tex_size - _quad_size) % 2 == 0);
    int overlap = (input_tex_size - _quad_size) / 2;
    float step = 1.0f / input_tex_size;
    float t = step * (overlap - 1);
    float s = step * (_quad_size + 2);
    xgl::DrawQuad(-1.0f, -1.0f, 2.0f, 2.0f, t, t, s, s);
    assert(xgl::CheckError(HERE));
    *meta = quad_meta;
    _last_frame = frame;
}

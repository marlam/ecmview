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
#include "xgl-gta.h"

#include "sar_amplitude_processor.h"
#include "sar-amplitude/normalization.fs.glsl.h"
#include "despeckling.h"
#include "drr.h"
#include "sar-amplitude/coloring.fs.glsl.h"

using namespace glvm;


sar_amplitude_processor::sar_amplitude_processor() :
    _last_frame(-1),
    _pingpong { 0, 0 }, _despeckling(NULL), _drr(NULL),
    _prg0(0), _prg1(0), _pbo(0), _gradient_tex(0)
{
}

void sar_amplitude_processor::init_gl()
{
}

void sar_amplitude_processor::exit_gl()
{
    if (_pingpong[0] != 0) {
        glDeleteTextures(2, _pingpong);
        _pingpong[0] = 0;
        _pingpong[1] = 0;
    }
    if (_despeckling) {
        _despeckling->exit_gl();
        delete _despeckling;
        _despeckling = NULL;
    }
    if (_drr) {
        _drr->exit_gl();
        delete _drr;
        _drr = NULL;
    }
    if (_prg0 != 0) {
        xgl::DeleteProgram(_prg0);
        _prg0 = 0;
    }
    if (_prg1 != 0) {
        xgl::DeleteProgram(_prg1);
        _prg1 = 0;
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

bool sar_amplitude_processor::processing_is_necessary(
        unsigned int /* frame */,
        const database_description& /* dd */, bool /* lens */,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */)
{
    return true;
}

void sar_amplitude_processor::process(
        unsigned int frame,
        const database_description& dd, bool lens,
        const glvm::ivec4& quad,
        const ecmdb::metadata& quad_meta,
        bool* /* full_validity */,
        ecmdb::metadata* meta)
{
    assert(xgl::CheckError(HERE));

    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    bool need_gradient_upload = (frame != _last_frame);

    if (_pingpong[0] == 0 || xgl::GetTex2DParameter(_pingpong[0], GL_TEXTURE_WIDTH) != dd.db.total_quad_size()) {
        // XXX This is a performance problem. We recreate the pingpong textures everytime
        // the dd.db.total_quad_size() changes. If a user opened two SAR data sets with different overlap,
        // then we do this for each quad!! We should probably use the texture cache for this
        // purpose. Or have different pingpong pairs for different dd.db.total_quad_size() values.
        if (_pingpong[0] != 0)
            glDeleteTextures(2, _pingpong);
        _pingpong[0] = xgl::CreateTex2D(GL_RGBA32F, dd.db.total_quad_size(), dd.db.total_quad_size(), GL_NEAREST);
        _pingpong[1] = xgl::CreateTex2D(GL_RGBA32F, dd.db.total_quad_size(), dd.db.total_quad_size(), GL_NEAREST);
    }
    if (pp.sar_amplitude.despeckling_method != processing_parameters::sar_amplitude_despeckling_none
            && (!_despeckling || _despeckling->method() != pp.sar_amplitude.despeckling_method)) {
        if (_despeckling) {
            _despeckling->exit_gl();
            delete _despeckling;
        }
        switch (pp.sar_amplitude.despeckling_method) {
        case processing_parameters::sar_amplitude_despeckling_none:
            assert(false);      // cannot happen
            break;
        case processing_parameters::sar_amplitude_despeckling_mean:
            _despeckling = new DespecklingMean();
            break;
        case processing_parameters::sar_amplitude_despeckling_median:
            _despeckling = new DespecklingMedian();
            break;
        case processing_parameters::sar_amplitude_despeckling_gauss:
            _despeckling = new DespecklingGauss();
            break;
        case processing_parameters::sar_amplitude_despeckling_lee:
            _despeckling = new DespecklingLee();
            break;
        case processing_parameters::sar_amplitude_despeckling_kuan:
            _despeckling = new DespecklingKuan();
            break;
        case processing_parameters::sar_amplitude_despeckling_xiao:
            _despeckling = new DespecklingXiao();
            break;
        case processing_parameters::sar_amplitude_despeckling_frost:
            _despeckling = new DespecklingFrost();
            break;
        case processing_parameters::sar_amplitude_despeckling_gammamap:
            _despeckling = new DespecklingGammaMAP();
            break;
        case processing_parameters::sar_amplitude_despeckling_oddy:
            _despeckling = new DespecklingOddy();
            break;
        case processing_parameters::sar_amplitude_despeckling_waveletst:
            _despeckling = new DespecklingWaveletST();
            break;
        }
        _despeckling->init_gl();
    }
    if (!_drr || _drr->method() != pp.sar_amplitude.drr_method) {
        if (_drr) {
            _drr->exit_gl();
            delete _drr;
        }
        switch (pp.sar_amplitude.drr_method) {
        case processing_parameters::sar_amplitude_drr_linear:
            _drr = new DynamicRangeReductionLinear();
            break;
        case processing_parameters::sar_amplitude_drr_log:
            _drr = new DynamicRangeReductionLog();
            break;
        case processing_parameters::sar_amplitude_drr_gamma:
            _drr = new DynamicRangeReductionGamma();
            break;
        case processing_parameters::sar_amplitude_drr_schlick:
            _drr = new DynamicRangeReductionSchlick();
            break;
        case processing_parameters::sar_amplitude_drr_reinhard:
            _drr = new DynamicRangeReductionReinhard();
            break;
        case processing_parameters::sar_amplitude_drr_schlicklocal:
            _drr = new DynamicRangeReductionSchlickLocal();
            break;
        case processing_parameters::sar_amplitude_drr_reinhardlocal:
            _drr = new DynamicRangeReductionReinhardLocal();
            break;
        }
        _drr->init_gl();
    }
    if (_prg0 == 0) {
        std::string src(NORMALIZATION_FS_GLSL_STR);
        _prg0 = xgl::CreateProgram("sar-amplitude-normalization", "", "", src);
        xgl::LinkProgram("sar-amplitude-normalization", _prg0);
        glUseProgram(_prg0);
        glvmUniform(xgl::GetUniformLocation(_prg0, "data_tex"), 0);
        assert(xgl::CheckError(HERE));
    }
    if (_prg1 == 0) {
        std::string src(COLORING_FS_GLSL_STR);
        _prg1 = xgl::CreateProgram("sar-amplitude-coloring", "", "", src);
        xgl::LinkProgram("sar-amplitude-coloring", _prg1);
        glUseProgram(_prg1);
        glvmUniform(xgl::GetUniformLocation(_prg1, "data_tex"), 0);
        glvmUniform(xgl::GetUniformLocation(_prg1, "mask_tex"), 1);
        glvmUniform(xgl::GetUniformLocation(_prg1, "gradient_tex"), 2);
        assert(xgl::CheckError(HERE));
    }
    if (_pbo == 0) {
        glGenBuffers(1, &_pbo);
    }
    if (_gradient_tex == 0 || xgl::GetTex2DParameter(_gradient_tex, GL_TEXTURE_WIDTH) != pp.sar_amplitude.gradient_length) {
        need_gradient_upload = true;
        if (_gradient_tex != 0)
            glDeleteTextures(1, &_gradient_tex);
        _gradient_tex = xgl::CreateTex2D(GL_SRGB8, pp.sar_amplitude.gradient_length, 1, GL_LINEAR);
        assert(xgl::CheckError(HERE));
    }
    if (need_gradient_upload) {
        xgl::WriteTex2D(_gradient_tex, 0, 0, pp.sar_amplitude.gradient_length, 1, GL_RGB, GL_UNSIGNED_BYTE,
                pp.sar_amplitude.gradient_length * 3 * sizeof(uint8_t), pp.sar_amplitude.gradient, _pbo);
        assert(xgl::CheckError(HERE));
    }

    // For the intermediate processing steps, only one FBO output is used.
    // Save the FBO state here and restore it later.
    GLint fbo_attachment_0, fbo_attachment_1;
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &fbo_attachment_0);
    glGetFramebufferAttachmentParameteriv(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &fbo_attachment_1);
    assert(fbo_attachment_0 > 0);
    assert(glIsTexture(fbo_attachment_0));
    assert(fbo_attachment_1 > 0);
    assert(glIsTexture(fbo_attachment_1));
    // we need to remove the second attachment, or else its smaller size will limit our rendering size
    // even if we don't actually render into it!
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glViewport(0, 0, dd.db.total_quad_size(), dd.db.total_quad_size());
    assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));

    // Get name of source texture
    GLint data_tex;
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &data_tex);
    assert(data_tex > 0);
    GLuint src_tex = data_tex;
    
    // TODO: Use the validity mask in all processing shaders!

    // Normalize SAR amplitudes to [0,1] (the processing shaders expect that for historical reasons;
    // TODO: this should be removed to save one render pass).
    glUseProgram(_prg0);
    glvmUniform(xgl::GetUniformLocation(_prg0, "min_amp"), dd.meta.sar_amplitude.min);
    glvmUniform(xgl::GetUniformLocation(_prg0, "max_amp"), dd.meta.sar_amplitude.max);
    _drr->render_one_to_one(_pingpong[0], src_tex);
    GLuint res_tex = _pingpong[0];
    assert(xgl::CheckError(HERE));

    // Apply despeckling if necessary
    if (pp.sar_amplitude.despeckling_method != processing_parameters::sar_amplitude_despeckling_none) {
        src_tex = res_tex;
        _despeckling->set(dd.db.total_quad_size(), dd.db.levels() - 1, quad[1]);
        res_tex = _despeckling->apply(dd, lens, quad, quad_meta, src_tex, _pingpong);
    }

    // Apply dynamic range reduction
    src_tex = res_tex;
    _drr->set(dd.db.total_quad_size(), dd.db.levels() - 1, quad[1]);
    res_tex = _drr->apply(dd, lens, quad, quad_meta, src_tex, _pingpong);

    // Restore the FBO state.
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_attachment_0, 0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, fbo_attachment_1, 0);
    GLuint draw_buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, draw_buffers);
    glViewport(0, 0, dd.db.quad_size() + 2, dd.db.quad_size() + 2);
    assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));

    // Convert normalized data to color.
    // TODO: integrate this into the DRR shaders to save one render pass.
    src_tex = res_tex;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src_tex);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _gradient_tex);
    assert(xgl::CheckError(HERE));
    glUseProgram(_prg1);
    glvmUniform(xgl::GetUniformLocation(_prg1, "adapt_brightness"), pp.sar_amplitude.adapt_brightness ? 1.0f : 0.0f);
    assert(xgl::CheckError(HERE));

    float step = 1.0f / dd.db.total_quad_size(); 
    float t = step * (dd.db.overlap() - 1);
    float s = step * (dd.db.quad_size() + 2);
    xgl::DrawQuad(-1.0f, -1.0f, 2.0f, 2.0f, t, t, s, s); 
    assert(xgl::CheckError(HERE));
    *meta = quad_meta;
    _last_frame = frame;
}

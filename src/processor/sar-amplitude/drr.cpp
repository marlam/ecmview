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

#include "config.h"

#include <cmath>

#include "../../base/dbg.h"
#include "../../base/str.h"

#include "glvm.h"
#include "glvm-gl.h"
#include "xgl.h"

#include "drr.h"
#include "sar-amplitude/drr-linear.fs.glsl.h"
#include "sar-amplitude/drr-log.fs.glsl.h"
#include "sar-amplitude/drr-gamma.fs.glsl.h"
#include "sar-amplitude/drr-schlick.fs.glsl.h"
#include "sar-amplitude/drr-reinhard.fs.glsl.h"
#include "sar-amplitude/drr-common-localavg-0.fs.glsl.h"
#include "sar-amplitude/drr-common-localavg-1.fs.glsl.h"
#include "sar-amplitude/drr-schlicklocal-2.fs.glsl.h"
#include "sar-amplitude/drr-reinhardlocal-2.fs.glsl.h"

using namespace glvm;


DynamicRangeReductionLinear::DynamicRangeReductionLinear() : SubProcessor(processing_parameters::sar_amplitude_drr_linear)
{
}

void DynamicRangeReductionLinear::init_gl()
{
    _prgs[0] = xgl::CreateProgram("sar-drr-linear", "", "", DRR_LINEAR_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-linear", _prgs[0]);
}

void DynamicRangeReductionLinear::exit_gl()
{
    xgl::DeletePrograms(1, _prgs);
}

GLuint DynamicRangeReductionLinear::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "min_amp"), pp.sar_amplitude.drr.linear.min_amp);
    glvmUniform(glGetUniformLocation(_prgs[0], "max_amp"), pp.sar_amplitude.drr.linear.max_amp);
    glvmUniform(glGetUniformLocation(_prgs[0], "maxmin_diff"),
            max(0.0f, pp.sar_amplitude.drr.linear.max_amp - pp.sar_amplitude.drr.linear.min_amp));
    render_one_to_one(pingpong[pong_index], src_tex);
    return pingpong[pong_index];
}

DynamicRangeReductionLog::DynamicRangeReductionLog() : SubProcessor(processing_parameters::sar_amplitude_drr_log)
{
}

void DynamicRangeReductionLog::init_gl()
{
    _prgs[0] = xgl::CreateProgram("sar-drr-log", "", "", DRR_LOG_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-log", _prgs[0]);
}

void DynamicRangeReductionLog::exit_gl()
{
    xgl::DeletePrograms(1, _prgs);
}

GLuint DynamicRangeReductionLog::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "min_amp"), pp.sar_amplitude.drr.log.min_amp);
    glvmUniform(glGetUniformLocation(_prgs[0], "max_amp"), pp.sar_amplitude.drr.log.max_amp);
    glvmUniform(glGetUniformLocation(_prgs[0], "prescale"), pp.sar_amplitude.drr.log.prescale);
    render_one_to_one(pingpong[pong_index], src_tex);
    return pingpong[pong_index];
}

DynamicRangeReductionGamma::DynamicRangeReductionGamma() : SubProcessor(processing_parameters::sar_amplitude_drr_gamma)
{
}

void DynamicRangeReductionGamma::init_gl()
{
    _prgs[0] = xgl::CreateProgram("sar-drr-gamma", "", "", DRR_GAMMA_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-gamma", _prgs[0]);
}

void DynamicRangeReductionGamma::exit_gl()
{
    xgl::DeletePrograms(1, _prgs);
}

GLuint DynamicRangeReductionGamma::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "min_amp"), pp.sar_amplitude.drr.gamma.min_amp);
    glvmUniform(glGetUniformLocation(_prgs[0], "max_min_amp_diff"),
            pp.sar_amplitude.drr.gamma.max_amp - pp.sar_amplitude.drr.gamma.min_amp);
    glvmUniform(glGetUniformLocation(_prgs[0], "gamma_reciprocal"), 1.0f / pp.sar_amplitude.drr.gamma.gamma);
    render_one_to_one(pingpong[pong_index], src_tex);
    return pingpong[pong_index];
}

DynamicRangeReductionSchlick::DynamicRangeReductionSchlick() : SubProcessor(processing_parameters::sar_amplitude_drr_schlick)
{
}

void DynamicRangeReductionSchlick::init_gl()
{
    _prgs[0] = xgl::CreateProgram("sar-drr-schlick", "", "", DRR_SCHLICK_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-schlick", _prgs[0]);
}

void DynamicRangeReductionSchlick::exit_gl()
{
    xgl::DeletePrograms(1, _prgs);
}

GLuint DynamicRangeReductionSchlick::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "p"), pp.sar_amplitude.drr.schlick.brightness);
    render_one_to_one(pingpong[pong_index], src_tex);
    return pingpong[pong_index];
}

DynamicRangeReductionReinhard::DynamicRangeReductionReinhard() : SubProcessor(processing_parameters::sar_amplitude_drr_reinhard)
{
}

void DynamicRangeReductionReinhard::init_gl()
{
    _prgs[0] = xgl::CreateProgram("sar-drr-reinhard", "", "", DRR_REINHARD_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-reinhard", _prgs[0]);
}

void DynamicRangeReductionReinhard::exit_gl()
{
    xgl::DeletePrograms(1, _prgs);
}

GLuint DynamicRangeReductionReinhard::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    float min_amp = dd.meta.sar_amplitude.min;
    float max_amp = dd.meta.sar_amplitude.max;
    float avg_amp = dd.meta.sar_amplitude.sum / dd.meta.sar_amplitude.valid;
    float m = 0.3f + 0.7f * std::pow((1.0f - (avg_amp / max_amp)) / (1.0f - (min_amp / max_amp)), 1.4f);
    glvmUniform(glGetUniformLocation(_prgs[0], "min_amp"), min_amp / max_amp);
    glvmUniform(glGetUniformLocation(_prgs[0], "max_amp"), 1.0f);
    glvmUniform(glGetUniformLocation(_prgs[0], "avg_amp"), avg_amp / max_amp);
    glvmUniform(glGetUniformLocation(_prgs[0], "m"), m);
    glvmUniform(glGetUniformLocation(_prgs[0], "b"), std::exp(-pp.sar_amplitude.drr.reinhard.brightness));
    glvmUniform(glGetUniformLocation(_prgs[0], "l"), 1.0f - pp.sar_amplitude.drr.reinhard.contrast);
    render_one_to_one(pingpong[pong_index], src_tex);
    return pingpong[pong_index];
}

DynamicRangeReductionSchlickLocal::DynamicRangeReductionSchlickLocal() : SubProcessor(processing_parameters::sar_amplitude_drr_schlicklocal)
{
    _k[0] = 7;
    _k[1] = 8;
    _k[2] = 9;
    for (int i = 0; i < 3; i++) {
        _mask[i] = new float[2 * _k[i] + 1];
        gauss_mask(_k[i], _k[i] / 2.5f, _mask[i], &(_weightsum[i]));
    }
}

DynamicRangeReductionSchlickLocal::~DynamicRangeReductionSchlickLocal()
{
    delete[] _mask[0];
    delete[] _mask[1];
    delete[] _mask[2];
}

void DynamicRangeReductionSchlickLocal::init_gl()
{
    _prgs[0] = xgl::CreateProgram("sar-drr-schlicklocal-0", "", "", DRR_COMMON_LOCALAVG_0_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-schlicklocal-0", _prgs[0]);
    _prgs[1] = xgl::CreateProgram("sar-drr-schlicklocal-1", "", "", DRR_COMMON_LOCALAVG_1_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-schlicklocal-1", _prgs[1]);
    _prgs[2] = xgl::CreateProgram("sar-drr-schlicklocal-2", "", "", DRR_SCHLICKLOCAL_2_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-schlicklocal-2", _prgs[2]);
}

void DynamicRangeReductionSchlickLocal::exit_gl()
{
    xgl::DeletePrograms(3, _prgs);
}

GLuint DynamicRangeReductionSchlickLocal::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    assert(xgl::CheckError(HERE));
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step(), adapted_step()));
    glUniform1fv(glGetUniformLocation(_prgs[0], "mask0"), 2 * _k[0] + 1, _mask[0]);
    glUniform1fv(glGetUniformLocation(_prgs[0], "mask1"), 2 * _k[1] + 1, _mask[1]);
    glUniform1fv(glGetUniformLocation(_prgs[0], "mask2"), 2 * _k[2] + 1, _mask[2]);
    glUniform1f(glGetUniformLocation(_prgs[0], "factor0"), 1.0f / _weightsum[0]);
    glUniform1f(glGetUniformLocation(_prgs[0], "factor1"), 1.0f / _weightsum[1]);
    glUniform1f(glGetUniformLocation(_prgs[0], "factor2"), 1.0f / _weightsum[2]);
    render_one_to_one(pingpong[pong_index], src_tex);
    assert(xgl::CheckError(HERE));
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step(), adapted_step()));
    glUniform1fv(glGetUniformLocation(_prgs[1], "mask0"), 2 * _k[0] + 1, _mask[0]);
    glUniform1fv(glGetUniformLocation(_prgs[1], "mask1"), 2 * _k[1] + 1, _mask[1]);
    glUniform1fv(glGetUniformLocation(_prgs[1], "mask2"), 2 * _k[2] + 1, _mask[2]);
    glUniform1f(glGetUniformLocation(_prgs[1], "factor0"), 1.0f / _weightsum[0]);
    glUniform1f(glGetUniformLocation(_prgs[1], "factor1"), 1.0f / _weightsum[1]);
    glUniform1f(glGetUniformLocation(_prgs[1], "factor2"), 1.0f / _weightsum[2]);
    render_one_to_one(pingpong[pong_index], src_tex);
    assert(xgl::CheckError(HERE));
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[2]);
    glvmUniform(glGetUniformLocation(_prgs[2], "step"), glvm::vec2(adapted_step(), adapted_step()));
    glUniform1f(glGetUniformLocation(_prgs[2], "min_amp"),
            dd.meta.sar_amplitude.min / dd.meta.sar_amplitude.max);
    glUniform1f(glGetUniformLocation(_prgs[2], "avg_amp"),
            dd.meta.sar_amplitude.sum / dd.meta.sar_amplitude.valid / dd.meta.sar_amplitude.max);
    glUniform1f(glGetUniformLocation(_prgs[2], "b"), std::exp(pp.sar_amplitude.drr.schlicklocal.brightness + 3.5));
    glUniform1f(glGetUniformLocation(_prgs[2], "l"), pp.sar_amplitude.drr.schlicklocal.details);
    glUniform1f(glGetUniformLocation(_prgs[2], "threshold"), pp.sar_amplitude.drr.schlicklocal.threshold);
    render_one_to_one(pingpong[pong_index], src_tex);
    assert(xgl::CheckError(HERE));
    return pingpong[pong_index];
}

DynamicRangeReductionReinhardLocal::DynamicRangeReductionReinhardLocal() : SubProcessor(processing_parameters::sar_amplitude_drr_reinhardlocal)
{
    _k[0] = 7;
    _k[1] = 8;
    _k[2] = 9;
    for (int i = 0; i < 3; i++) {
        _mask[i] = new float[2 * _k[i] + 1];
        gauss_mask(_k[i], static_cast<float>(_k[i] / 2.5f), _mask[i], &(_weightsum[i]));
    }
}

DynamicRangeReductionReinhardLocal::~DynamicRangeReductionReinhardLocal()
{
    delete[] _mask[0];
    delete[] _mask[1];
    delete[] _mask[2];
}

void DynamicRangeReductionReinhardLocal::init_gl()
{
    _prgs[0] = xgl::CreateProgram("sar-drr-reinhardlocal-0", "", "", DRR_COMMON_LOCALAVG_0_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-reinhardlocal-0", _prgs[0]);
    _prgs[1] = xgl::CreateProgram("sar-drr-reinhardlocal-1", "", "", DRR_COMMON_LOCALAVG_1_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-reinhardlocal-1", _prgs[1]);
    _prgs[2] = xgl::CreateProgram("sar-drr-reinhardlocal-2", "", "", DRR_REINHARDLOCAL_2_FS_GLSL_STR);
    xgl::LinkProgram("sar-drr-reinhardlocal-2", _prgs[2]);
}

void DynamicRangeReductionReinhardLocal::exit_gl()
{
    xgl::DeletePrograms(3, _prgs);
}

GLuint DynamicRangeReductionReinhardLocal::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    assert(xgl::CheckError(HERE));
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step(), adapted_step()));
    glUniform1fv(glGetUniformLocation(_prgs[0], "mask0"), 2 * _k[0] + 1, _mask[0]);
    glUniform1fv(glGetUniformLocation(_prgs[0], "mask1"), 2 * _k[1] + 1, _mask[1]);
    glUniform1fv(glGetUniformLocation(_prgs[0], "mask2"), 2 * _k[2] + 1, _mask[2]);
    glUniform1f(glGetUniformLocation(_prgs[0], "factor0"), 1.0f / _weightsum[0]);
    glUniform1f(glGetUniformLocation(_prgs[0], "factor1"), 1.0f / _weightsum[1]);
    glUniform1f(glGetUniformLocation(_prgs[0], "factor2"), 1.0f / _weightsum[2]);
    render_one_to_one(pingpong[pong_index], src_tex);
    assert(xgl::CheckError(HERE));
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step(), adapted_step()));
    glUniform1fv(glGetUniformLocation(_prgs[1], "mask0"), 2 * _k[0] + 1, _mask[0]);
    glUniform1fv(glGetUniformLocation(_prgs[1], "mask1"), 2 * _k[1] + 1, _mask[1]);
    glUniform1fv(glGetUniformLocation(_prgs[1], "mask2"), 2 * _k[2] + 1, _mask[2]);
    glUniform1f(glGetUniformLocation(_prgs[1], "factor0"), 1.0f / _weightsum[0]);
    glUniform1f(glGetUniformLocation(_prgs[1], "factor1"), 1.0f / _weightsum[1]);
    glUniform1f(glGetUniformLocation(_prgs[1], "factor2"), 1.0f / _weightsum[2]);
    render_one_to_one(pingpong[pong_index], src_tex);
    assert(xgl::CheckError(HERE));
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[2]);
    float min_amp = dd.meta.sar_amplitude.min;
    float max_amp = dd.meta.sar_amplitude.max;
    float avg_amp = dd.meta.sar_amplitude.sum / dd.meta.sar_amplitude.valid;
    float m = 0.3f + 0.7f * std::pow((1.0f - (avg_amp / max_amp)) / (1.0f - (min_amp / max_amp)), 1.4f);
    glvmUniform(glGetUniformLocation(_prgs[2], "step"), glvm::vec2(adapted_step(), adapted_step()));
    glUniform1f(glGetUniformLocation(_prgs[2], "min_amp"), min_amp / max_amp);
    glUniform1f(glGetUniformLocation(_prgs[2], "avg_amp"), avg_amp / max_amp);
    glUniform1f(glGetUniformLocation(_prgs[2], "b"), std::exp(-pp.sar_amplitude.drr.reinhardlocal.brightness));
    glUniform1f(glGetUniformLocation(_prgs[2], "l"), 1.0f - pp.sar_amplitude.drr.reinhardlocal.contrast);
    glUniform1f(glGetUniformLocation(_prgs[2], "d"), pp.sar_amplitude.drr.reinhardlocal.details);
    glUniform1f(glGetUniformLocation(_prgs[2], "threshold"), pp.sar_amplitude.drr.reinhardlocal.threshold);
    glUniform1f(glGetUniformLocation(_prgs[2], "m"), m);
    render_one_to_one(pingpong[pong_index], src_tex);
    assert(xgl::CheckError(HERE));
    return pingpong[pong_index];
}

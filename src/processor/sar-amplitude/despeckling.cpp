/*
 * despeckling.cpp
 *
 * This file is part of plew, a planet viewer.
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2010
 * Computer Graphics Group, University of Siegen, Germany.
 * See http://www.cg.informatik.uni-siegen.de/ for contact information.
 *
 * All rights reserved.
 *
 * Written by Martin Lambers <lambers@fb12.uni-siegen.de>.
 */

#include "config.h"

#include "../../base/dbg.h"
#include "../../base/str.h"

#include "glvm.h"
#include "glvm-gl.h"
#include "xgl.h"
#include "xgl-gta.h"

#include "despeckling.h"
#include "sar-amplitude/despeckling-mean-0.fs.glsl.h"
#include "sar-amplitude/despeckling-mean-1.fs.glsl.h"
#include "sar-amplitude/despeckling-median-0.fs.glsl.h"
#include "sar-amplitude/despeckling-median-1.fs.glsl.h"
#include "sar-amplitude/despeckling-gauss-0.fs.glsl.h"
#include "sar-amplitude/despeckling-gauss-1.fs.glsl.h"
#include "sar-amplitude/despeckling-common-localstat-0.fs.glsl.h"
#include "sar-amplitude/despeckling-common-localstat-1.fs.glsl.h"
#include "sar-amplitude/despeckling-lee-2.fs.glsl.h"
#include "sar-amplitude/despeckling-kuan-2.fs.glsl.h"
#include "sar-amplitude/despeckling-xiao-2.fs.glsl.h"
#include "sar-amplitude/despeckling-frost-2.fs.glsl.h"
#include "sar-amplitude/despeckling-gammamap-0.fs.glsl.h"
#include "sar-amplitude/despeckling-gammamap-3.fs.glsl.h"
#include "sar-amplitude/despeckling-oddy.fs.glsl.h"
#include "sar-amplitude/despeckling-waveletst-0.fs.glsl.h"
#include "sar-amplitude/despeckling-waveletst-1.fs.glsl.h"
#include "sar-amplitude/despeckling-waveletst-2.fs.glsl.h"
#include "sar-amplitude/despeckling-waveletst-3.fs.glsl.h"
#include "sar-amplitude/despeckling-waveletst-4.fs.glsl.h"

using namespace glvm;


DespecklingMean::DespecklingMean() : SubProcessor(processing_parameters::sar_amplitude_despeckling_mean),
    _prgs { 0, 0 }, _kh(-1), _kv(-1)
{
}

void DespecklingMean::init_gl()
{
}

void DespecklingMean::exit_gl()
{
    xgl::DeletePrograms(2, _prgs);
}

GLuint DespecklingMean::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    if (_kh != pp.sar_amplitude.despeckling.mean.kh) {
        xgl::DeleteProgram(_prgs[0]);
        _prgs[0] = xgl::CreateProgram("sar-despeckling-mean-0", "", "",
                xglShaderSourcePrep(DESPECKLING_MEAN_0_FS_GLSL_STR, str::asprintf("$kh=%d", pp.sar_amplitude.despeckling.mean.kh)));
        xgl::LinkProgram("sar-despeckling-mean-0", _prgs[0]);
        _kh = pp.sar_amplitude.despeckling.mean.kh;
    }
    if (_kv != pp.sar_amplitude.despeckling.mean.kv) {
        xgl::DeleteProgram(_prgs[1]);
        _prgs[1] = xgl::CreateProgram("sar-despeckling-mean-1", "", "",
                xglShaderSourcePrep(DESPECKLING_MEAN_1_FS_GLSL_STR, str::asprintf("$kv=%d", pp.sar_amplitude.despeckling.mean.kv)));
        xgl::LinkProgram("sar-despeckling-mean-1", _prgs[1]);
        _kv = pp.sar_amplitude.despeckling.mean.kv;
    }
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], src_tex);
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    return pingpong[pong_index];
}

DespecklingMedian::DespecklingMedian() : SubProcessor(processing_parameters::sar_amplitude_despeckling_median),
    _prgs { 0, 0 }, _kh(-1), _kv(-1)
{
}

void DespecklingMedian::init_gl()
{
}

void DespecklingMedian::exit_gl()
{
    xgl::DeletePrograms(2, _prgs);
}

GLuint DespecklingMedian::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    if (_kh != pp.sar_amplitude.despeckling.median.kh) {
        xgl::DeleteProgram(_prgs[0]);
        _prgs[0] = xgl::CreateProgram("sar-despeckling-median-0", "", "",
                xglShaderSourcePrep(DESPECKLING_MEDIAN_0_FS_GLSL_STR, str::asprintf("$kh=%d", pp.sar_amplitude.despeckling.median.kh)));
        xgl::LinkProgram("sar-despeckling-median-0", _prgs[0]);
        _kh = pp.sar_amplitude.despeckling.median.kh;
    }
    if (_kv != pp.sar_amplitude.despeckling.median.kv) {
        xgl::DeleteProgram(_prgs[1]);
        _prgs[1] = xgl::CreateProgram("sar-despeckling-median-1", "", "",
                xglShaderSourcePrep(DESPECKLING_MEDIAN_1_FS_GLSL_STR, str::asprintf("$kv=%d", pp.sar_amplitude.despeckling.median.kv)));
        xgl::LinkProgram("sar-despeckling-median-1", _prgs[1]);
        _kv = pp.sar_amplitude.despeckling.median.kv;
    }
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], src_tex);
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    return pingpong[pong_index];
}

DespecklingGauss::DespecklingGauss() : SubProcessor(processing_parameters::sar_amplitude_despeckling_gauss),
    _prgs { 0, 0 }, _kh(-1), _kv(-1), _mask_h(NULL), _mask_v(NULL)
{
}

DespecklingGauss::~DespecklingGauss()
{
    delete _mask_h;
    delete _mask_v;
}

void DespecklingGauss::init_gl()
{
}

void DespecklingGauss::exit_gl()
{
    xgl::DeletePrograms(2, _prgs);
}

GLuint DespecklingGauss::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    if (_kh != pp.sar_amplitude.despeckling.gauss.kh) {
        xgl::DeleteProgram(_prgs[0]);
        _prgs[0] = xgl::CreateProgram("sar-despeckling-gauss-0", "", "",
                xglShaderSourcePrep(DESPECKLING_GAUSS_0_FS_GLSL_STR, str::asprintf("$kh=%d", pp.sar_amplitude.despeckling.gauss.kh)));
        xgl::LinkProgram("sar-despeckling-gauss-0", _prgs[0]);
        _mask_h = new float[2 * pp.sar_amplitude.despeckling.gauss.kh + 1];
        gauss_mask(pp.sar_amplitude.despeckling.gauss.kh, pp.sar_amplitude.despeckling.gauss.sh, _mask_h, &_weightsum_h);
        _kh = pp.sar_amplitude.despeckling.gauss.kh;
    }
    if (_kv != pp.sar_amplitude.despeckling.gauss.kv) {
        xgl::DeleteProgram(_prgs[1]);
        _prgs[1] = xgl::CreateProgram("sar-despeckling-gauss-1", "", "",
                xglShaderSourcePrep(DESPECKLING_GAUSS_1_FS_GLSL_STR, str::asprintf("$kv=%d", pp.sar_amplitude.despeckling.gauss.kv)));
        xgl::LinkProgram("sar-despeckling-gauss-1", _prgs[1]);
        _mask_v = new float[2 * pp.sar_amplitude.despeckling.gauss.kv + 1];
        gauss_mask(pp.sar_amplitude.despeckling.gauss.kv, pp.sar_amplitude.despeckling.gauss.sv, _mask_v, &_weightsum_v);
        _kv = pp.sar_amplitude.despeckling.gauss.kv;
    }
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step()));
    glUniform1fv(glGetUniformLocation(_prgs[0], "mask_h"), 2 * pp.sar_amplitude.despeckling.gauss.kh + 1, _mask_h);
    glUniform1f(glGetUniformLocation(_prgs[0], "factor_h"), 1.0f / _weightsum_h);
    render_one_to_one(pingpong[pong_index], src_tex);
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step()));
    glUniform1fv(glGetUniformLocation(_prgs[1], "mask_v"), 2 * pp.sar_amplitude.despeckling.gauss.kv + 1, _mask_v);
    glUniform1f(glGetUniformLocation(_prgs[1], "factor_v"), 1.0f / _weightsum_v);
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    return pingpong[pong_index];
}

DespecklingLee::DespecklingLee() : SubProcessor(processing_parameters::sar_amplitude_despeckling_lee),
    _prgs { 0, 0, 0 }, _kh(-1), _kv(-1)
{
}

void DespecklingLee::init_gl()
{
    _prgs[2] = xgl::CreateProgram("sar-despeckling-lee-2", "", "", DESPECKLING_LEE_2_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-lee-2", _prgs[2]);
}

void DespecklingLee::exit_gl()
{
    xgl::DeletePrograms(3, _prgs);
}

GLuint DespecklingLee::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    if (_kh != pp.sar_amplitude.despeckling.lee.kh || _kv != pp.sar_amplitude.despeckling.lee.kv) {
        xgl::DeleteProgram(_prgs[0]);
        _prgs[0] = xgl::CreateProgram("sar-despeckling-lee-0", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_0_FS_GLSL_STR,
                    str::asprintf("$kh=%d", pp.sar_amplitude.despeckling.lee.kh)));
        xgl::LinkProgram("sar-despeckling-lee-0", _prgs[0]);
        xgl::DeleteProgram(_prgs[1]);
        _prgs[1] = xgl::CreateProgram("sar-despeckling-lee-1", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_1_FS_GLSL_STR,
                    str::asprintf("$kh=%d, $kv=%d", pp.sar_amplitude.despeckling.lee.kh, pp.sar_amplitude.despeckling.lee.kv)));
        xgl::LinkProgram("sar-despeckling-lee-1", _prgs[1]);
        _kh = pp.sar_amplitude.despeckling.lee.kh;
        _kv = pp.sar_amplitude.despeckling.lee.kv;
    }
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], src_tex);
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[2]);
    glvmUniform(glGetUniformLocation(_prgs[2], "var_n"), pp.sar_amplitude.despeckling.lee.sigma_n * pp.sar_amplitude.despeckling.lee.sigma_n);
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    return pingpong[pong_index];
}

DespecklingKuan::DespecklingKuan() : SubProcessor(processing_parameters::sar_amplitude_despeckling_kuan),
    _prgs { 0, 0, 0 }, _kh(-1), _kv(-1)
{
}

void DespecklingKuan::init_gl()
{
    _prgs[2] = xgl::CreateProgram("sar-despeckling-kuan-2", "", "", DESPECKLING_KUAN_2_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-kuan-2", _prgs[2]);
}

void DespecklingKuan::exit_gl()
{
    xgl::DeletePrograms(3, _prgs);
}

GLuint DespecklingKuan::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    if (_kh != pp.sar_amplitude.despeckling.kuan.kh || _kv != pp.sar_amplitude.despeckling.kuan.kv) {
        xgl::DeleteProgram(_prgs[0]);
        _prgs[0] = xgl::CreateProgram("sar-despeckling-kuan-0", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_0_FS_GLSL_STR,
                    str::asprintf("$kh=%d", pp.sar_amplitude.despeckling.kuan.kh)));
        xgl::LinkProgram("sar-despeckling-kuan-0", _prgs[0]);
        xgl::DeleteProgram(_prgs[1]);
        _prgs[1] = xgl::CreateProgram("sar-despeckling-kuan-1", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_1_FS_GLSL_STR,
                    str::asprintf("$kh=%d, $kv=%d", pp.sar_amplitude.despeckling.kuan.kh, pp.sar_amplitude.despeckling.kuan.kv)));
        xgl::LinkProgram("sar-despeckling-kuan-1", _prgs[1]);
        _kh = pp.sar_amplitude.despeckling.kuan.kh;
        _kv = pp.sar_amplitude.despeckling.kuan.kv;
    }
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], src_tex);
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[2]);
    glvmUniform(glGetUniformLocation(_prgs[2], "l"), pp.sar_amplitude.despeckling.kuan.L);
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    return pingpong[pong_index];
}

DespecklingFrost::DespecklingFrost() : SubProcessor(processing_parameters::sar_amplitude_despeckling_frost),
    _prgs { 0, 0, 0 }, _kh(-1), _kv(-1)
{
}

void DespecklingFrost::init_gl()
{
}

void DespecklingFrost::exit_gl()
{
    xgl::DeletePrograms(3, _prgs);
}

GLuint DespecklingFrost::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    if (_kh != pp.sar_amplitude.despeckling.frost.kh || _kv != pp.sar_amplitude.despeckling.frost.kv) {
        xgl::DeleteProgram(_prgs[0]);
        _prgs[0] = xgl::CreateProgram("sar-despeckling-frost-0", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_0_FS_GLSL_STR,
                    str::asprintf("$kh=%d", pp.sar_amplitude.despeckling.frost.kh)));
        xgl::LinkProgram("sar-despeckling-frost-0", _prgs[0]);
        xgl::DeleteProgram(_prgs[1]);
        _prgs[1] = xgl::CreateProgram("sar-despeckling-frost-1", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_1_FS_GLSL_STR,
                    str::asprintf("$kh=%d, $kv=%d", pp.sar_amplitude.despeckling.frost.kh, pp.sar_amplitude.despeckling.frost.kv)));
        xgl::LinkProgram("sar-despeckling-frost-1", _prgs[1]);
        xgl::DeleteProgram(_prgs[2]);
        _prgs[2] = xgl::CreateProgram("sar-despeckling-frost-2", "", "",
                xglShaderSourcePrep(DESPECKLING_FROST_2_FS_GLSL_STR,
                    str::asprintf("$kh=%d, $kv=%d", pp.sar_amplitude.despeckling.frost.kh, pp.sar_amplitude.despeckling.frost.kv)));
        xgl::LinkProgram("sar-despeckling-frost-2", _prgs[2]);
        _kh = pp.sar_amplitude.despeckling.frost.kh;
        _kv = pp.sar_amplitude.despeckling.frost.kv;
    }
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], src_tex);
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[2]);
    glvmUniform(glGetUniformLocation(_prgs[2], "step"), glvm::vec2(adapted_step()));
    glvmUniform(glGetUniformLocation(_prgs[2], "a"), pp.sar_amplitude.despeckling.frost.a);
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    return pingpong[pong_index];
}

DespecklingGammaMAP::DespecklingGammaMAP() : SubProcessor(processing_parameters::sar_amplitude_despeckling_gammamap),
    _prgs { 0, 0, 0, 0 }, _kh(-1), _kv(-1)
{
}

void DespecklingGammaMAP::init_gl()
{
    _prgs[0] = xgl::CreateProgram("sar-despeckling-gammamap-0", "", "", DESPECKLING_GAMMAMAP_0_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-gammamap-0", _prgs[0]);
    _prgs[3] = xgl::CreateProgram("sar-despeckling-gammamap-3", "", "", DESPECKLING_GAMMAMAP_3_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-gammamap-3", _prgs[3]);
}

void DespecklingGammaMAP::exit_gl()
{
    xgl::DeletePrograms(4, _prgs);
}

GLuint DespecklingGammaMAP::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    if (_kh != pp.sar_amplitude.despeckling.gammamap.kh || _kv != pp.sar_amplitude.despeckling.gammamap.kv) {
        xgl::DeleteProgram(_prgs[2]);
        _prgs[1] = xgl::CreateProgram("sar-despeckling-gammamap-1", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_0_FS_GLSL_STR,
                    str::asprintf("$kh=%d", pp.sar_amplitude.despeckling.gammamap.kh)));
        xgl::LinkProgram("sar-despeckling-gammamap-1", _prgs[1]);
        xgl::DeleteProgram(_prgs[2]);
        _prgs[2] = xgl::CreateProgram("sar-despeckling-gammamap-2", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_1_FS_GLSL_STR,
                    str::asprintf("$kh=%d, $kv=%d", pp.sar_amplitude.despeckling.gammamap.kh, pp.sar_amplitude.despeckling.gammamap.kv)));
        xgl::LinkProgram("sar-despeckling-gammamap-2", _prgs[2]);
        _kh = pp.sar_amplitude.despeckling.gammamap.kh;
        _kv = pp.sar_amplitude.despeckling.gammamap.kv;
    }
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    render_one_to_one(pingpong[pong_index], src_tex);
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[2]);
    glvmUniform(glGetUniformLocation(_prgs[2], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[3]);
    glvmUniform(glGetUniformLocation(_prgs[3], "L"), pp.sar_amplitude.despeckling.gammamap.L);
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    return pingpong[pong_index];
}

DespecklingXiao::DespecklingXiao() : SubProcessor(processing_parameters::sar_amplitude_despeckling_xiao),
    _prgs { 0, 0, 0 }, _kh(-1), _kv(-1)
{
}

void DespecklingXiao::init_gl()
{
    _prgs[2] = xgl::CreateProgram("sar-despeckling-xiao-2", "", "", DESPECKLING_XIAO_2_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-xiao-2", _prgs[2]);
}

void DespecklingXiao::exit_gl()
{
    xgl::DeletePrograms(3, _prgs);
}

GLuint DespecklingXiao::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    if (_kh != pp.sar_amplitude.despeckling.xiao.kh || _kv != pp.sar_amplitude.despeckling.xiao.kv) {
        xgl::DeleteProgram(_prgs[0]);
        _prgs[0] = xgl::CreateProgram("sar-despeckling-xiao-0", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_0_FS_GLSL_STR,
                    str::asprintf("$kh=%d", pp.sar_amplitude.despeckling.xiao.kh)));
        xgl::LinkProgram("sar-despeckling-xiao-0", _prgs[0]);
        xgl::DeleteProgram(_prgs[1]);
        _prgs[1] = xgl::CreateProgram("sar-despeckling-xiao-1", "", "",
                xglShaderSourcePrep(DESPECKLING_COMMON_LOCALSTAT_1_FS_GLSL_STR,
                    str::asprintf("$kh=%d, $kv=%d", pp.sar_amplitude.despeckling.xiao.kh, pp.sar_amplitude.despeckling.xiao.kv)));
        xgl::LinkProgram("sar-despeckling-xiao-1", _prgs[1]);
        _kh = pp.sar_amplitude.despeckling.xiao.kh;
        _kv = pp.sar_amplitude.despeckling.xiao.kv;
    }
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], src_tex);
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "step"), glvm::vec2(adapted_step()));
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[2]);
    glvmUniform(glGetUniformLocation(_prgs[2], "Tmin"), pp.sar_amplitude.despeckling.xiao.Tmin);
    glvmUniform(glGetUniformLocation(_prgs[2], "Tmax"), pp.sar_amplitude.despeckling.xiao.Tmax);
    glvmUniform(glGetUniformLocation(_prgs[2], "a"), pp.sar_amplitude.despeckling.xiao.a);
    glvmUniform(glGetUniformLocation(_prgs[2], "b"), pp.sar_amplitude.despeckling.xiao.b);
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    return pingpong[pong_index];
}

DespecklingOddy::DespecklingOddy() : SubProcessor(processing_parameters::sar_amplitude_despeckling_oddy),
    _prgs { 0 }, _kh(-1), _kv(-1)
{
}

void DespecklingOddy::init_gl()
{
}

void DespecklingOddy::exit_gl()
{
    xgl::DeletePrograms(1, _prgs);
}

GLuint DespecklingOddy::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    if (_kh != pp.sar_amplitude.despeckling.oddy.kh || _kv != pp.sar_amplitude.despeckling.oddy.kv) {
        xgl::DeleteProgram(_prgs[0]);
        _prgs[0] = xgl::CreateProgram("sar-despeckling-oddy", "", "",
                xglShaderSourcePrep(DESPECKLING_ODDY_FS_GLSL_STR,
                    str::asprintf("$kh=%d, $kv=%d", pp.sar_amplitude.despeckling.oddy.kh, pp.sar_amplitude.despeckling.oddy.kv)));
        xgl::LinkProgram("sar-despeckling-oddy", _prgs[0]);
        _kh = pp.sar_amplitude.despeckling.oddy.kh;
        _kv = pp.sar_amplitude.despeckling.oddy.kv;
    }
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "step"), glvm::vec2(adapted_step()));
    glvmUniform(glGetUniformLocation(_prgs[0], "alpha"), pp.sar_amplitude.despeckling.oddy.alpha);
    render_one_to_one(pingpong[pong_index], src_tex);
    return pingpong[pong_index];
}

DespecklingWaveletST::DespecklingWaveletST() : SubProcessor(processing_parameters::sar_amplitude_despeckling_waveletst),
    _prgs { 0, 0, 0, 0, 0 }
{
}

void DespecklingWaveletST::init_gl()
{
    _prgs[0] = xgl::CreateProgram("sar-despeckling-waveletst-0", "", "", DESPECKLING_WAVELETST_0_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-waveletst-0", _prgs[0]);
    _prgs[1] = xgl::CreateProgram("sar-despeckling-waveletst-1", "", "", DESPECKLING_WAVELETST_1_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-waveletst-1", _prgs[1]);
    _prgs[2] = xgl::CreateProgram("sar-despeckling-waveletst-2", "", "", DESPECKLING_WAVELETST_2_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-waveletst-2", _prgs[2]);
    _prgs[3] = xgl::CreateProgram("sar-despeckling-waveletst-3", "", "", DESPECKLING_WAVELETST_3_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-waveletst-3", _prgs[3]);
    _prgs[4] = xgl::CreateProgram("sar-despeckling-waveletst-4", "", "", DESPECKLING_WAVELETST_4_FS_GLSL_STR);
    xgl::LinkProgram("sar-despeckling-waveletst-4", _prgs[4]);
}

void DespecklingWaveletST::exit_gl()
{
    xgl::DeletePrograms(5, _prgs);
}

GLuint DespecklingWaveletST::apply(
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */,
        const GLuint src_tex,
        const GLuint pingpong[2])
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    int pong_index = (pingpong[0] == src_tex ? 1 : 0);
    glUseProgram(_prgs[0]);
    glvmUniform(glGetUniformLocation(_prgs[0], "xstep"), step());
    render_one_to_one(pingpong[pong_index], src_tex);
    int ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[1]);
    glvmUniform(glGetUniformLocation(_prgs[1], "ystep"), step());
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[2]);
    float threshold = pp.sar_amplitude.despeckling.waveletst.threshold;
    if (level_difference() >= 0)
        threshold /= (1 << level_difference());
    else
        threshold *= (1 << (-level_difference()));
    glvmUniform(glGetUniformLocation(_prgs[2], "T"), threshold);
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[3]);
    glvmUniform(glGetUniformLocation(_prgs[3], "texwidth"), tile_size());
    glvmUniform(glGetUniformLocation(_prgs[3], "texheight"), tile_size());
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    ping_index = pong_index;
    pong_index = (ping_index == 0 ? 1 : 0);
    glUseProgram(_prgs[4]);
    glvmUniform(glGetUniformLocation(_prgs[4], "texwidth"), tile_size());
    glvmUniform(glGetUniformLocation(_prgs[4], "texheight"), tile_size());
    render_one_to_one(pingpong[pong_index], pingpong[ping_index]);
    return pingpong[pong_index];
}

/*
 * Copyright (C) 2009, 2010, 2011, 2012
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

#include "glvm-gl.h"
#include "xgl.h"

#include "elevation_processor.h"
#include "elevation/scale.fs.glsl.h"

using namespace glvm;


elevation_processor::elevation_processor() :
    _min_elev(0.0f), _max_elev(0.0f), _prg(0)
{
}

void elevation_processor::set_elevation_bounds(float min_elev, float max_elev)
{
    _min_elev = min_elev;
    _max_elev = max_elev;
}

void elevation_processor::get_processed_quad_elevation_bounds(
        unsigned int /* frame */,
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& quad_meta,
        float* processed_quad_min_elev, float* processed_quad_max_elev) const
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    const float& sf = pp.elevation.scale_factor;
    const float& sc = pp.elevation.scale_center;
    *processed_quad_min_elev = glvm::clamp((quad_meta.elevation.min - sc) * sf + sc, _min_elev, _max_elev);
    *processed_quad_max_elev = glvm::clamp((quad_meta.elevation.max - sc) * sf + sc, _min_elev, _max_elev);
}

void elevation_processor::init_gl()
{
}

void elevation_processor::exit_gl()
{
    if (_prg != 0) {
        xgl::DeleteProgram(_prg);
        _prg = 0;
    }
}

bool elevation_processor::processing_is_necessary(
        unsigned int /* frame */,
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& /* quad_meta */)
{
    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];
    return (dd.db.data_offset() < 0.0f || dd.db.data_offset() > 0.0f
            || dd.db.data_factor() < 1.0f || dd.db.data_factor() > 1.0f
            || pp.elevation.scale_factor < 1.0f || pp.elevation.scale_factor > 1.0f);
}

void elevation_processor::process(
        unsigned int /* frame */,
        const database_description& dd, bool lens,
        const glvm::ivec4& /* quad */,
        const ecmdb::metadata& quad_meta,
        bool* /* full_validity */,
        ecmdb::metadata* meta)
{
    assert(xgl::CheckError(HERE));

    const processing_parameters& pp = dd.processing_parameters[lens ? 1 : 0];

    if (_prg == 0) {
        _prg = xgl::CreateProgram("elevation-scale", "", "", SCALE_FS_GLSL_STR);
        xgl::LinkProgram("elevation-scale", _prg);
        glUseProgram(_prg);
        glvmUniform(xgl::GetUniformLocation(_prg, "data_tex"), 0);
        glvmUniform(xgl::GetUniformLocation(_prg, "mask_tex"), 1);
        _data_offset_loc = xgl::GetUniformLocation(_prg, "data_offset");
        _data_factor_loc = xgl::GetUniformLocation(_prg, "data_factor");
        _scale_factor_loc = xgl::GetUniformLocation(_prg, "scale_factor");
        _scale_center_loc = xgl::GetUniformLocation(_prg, "scale_center");
        _min_elev_loc = xgl::GetUniformLocation(_prg, "min_elev");
        _max_elev_loc = xgl::GetUniformLocation(_prg, "max_elev");
    }

    glUseProgram(_prg);
    glvmUniform(_data_offset_loc, dd.db.data_offset());
    glvmUniform(_data_factor_loc, dd.db.data_factor());
    glvmUniform(_scale_factor_loc, pp.elevation.scale_factor);
    glvmUniform(_scale_center_loc, pp.elevation.scale_center);
    glvmUniform(_min_elev_loc, _min_elev);
    glvmUniform(_max_elev_loc, _max_elev);
    float step = 1.0f / dd.db.total_quad_size(); 
    float t = step * (dd.db.overlap() - 2);
    float s = step * (dd.db.quad_size() + 4);
    /*
    msg::dbg("elevation processor: do=%g df=%g sf=%g sc=%g mi=%g ma=%g step=%g t=%g s=%g",
            dd.db.data_offset(), dd.db.data_factor(), pp.elevation.scale_factor, pp.elevation.scale_center,
            _min_elev, _max_elev, step, t, s);
    */
    xgl::DrawQuad(-1.0f, -1.0f, 2.0f, 2.0f, t, t, s, s); 
    assert(xgl::CheckError(HERE));

    const float& sf = pp.elevation.scale_factor;
    const float& sc = pp.elevation.scale_center;
    meta->elevation.min = glvm::clamp((quad_meta.elevation.min - sc) * sf + sc, _min_elev, _max_elev);
    meta->elevation.max = glvm::clamp((quad_meta.elevation.max - sc) * sf + sc, _min_elev, _max_elev);
}

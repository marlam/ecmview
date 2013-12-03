/*
 * Copyright (C) 2009, 2010, 2011, 2012, 2013
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

#include <GL/glew.h>

#include <ecmdb/ecm.h>

#include "dbg.h"
#include "msg.h"
#include "str.h"

#include "glvm-gl.h"
#include "xgl.h"

#include "processor.h"
#include "elevation/elevation_processor.h"
#include "texture/texture_processor.h"
#include "sar-amplitude/sar_amplitude_processor.h"
#include "data/data_processor.h"
#include "e2c/e2c_processor.h"

#include "combine.fs.glsl.h"
#include "combine_lens.fs.glsl.h"

using namespace glvm;


processor::processor()
{
    assert(ecmdb::category_elevation == 1);
    assert(ecmdb::category_texture == 2);
    assert(ecmdb::category_sar_amplitude == 3);
    assert(ecmdb::category_data == 4);
    _procs[0] = new e2c_processor();
    _procs[1] = new elevation_processor();
    _procs[2] = new texture_processor();
    _procs[3] = new sar_amplitude_processor();
    _procs[4] = new data_processor();
    for (int i = 0; i < max_combinable_quads - 1; i++)
        _prg_combine[i] = 0;
    _prg_combine_lens = 0;
}

processor::~processor()
{
    for (int i = 0; i < _nprocs; i++)
        delete _procs[i];
}

void processor::init_gl()
{
}

void processor::exit_gl()
{
    for (int i = 0; i < _nprocs; i++) {
        if (_procs[i]) {
            _procs[i]->exit_gl();
        }
    }
    for (int i = 0; i < max_combinable_quads - 1; i++) {
        if (_prg_combine[i] != 0) {
            xgl::DeleteProgram(_prg_combine[i]);
            _prg_combine[i] = 0;
        }
    }
    if (_prg_combine_lens != 0) {
        xgl::DeleteProgram(_prg_combine_lens);
        _prg_combine_lens = 0;
    }
}

void processor::set_elevation_bounds(float min_elev, float max_elev)
{
    static_cast<elevation_processor *>(_procs[ecmdb::category_elevation])->set_elevation_bounds(min_elev, max_elev);
}

void processor::get_processed_quad_elevation_bounds(
        unsigned int frame,
        const database_description& dd, bool lens,
        const ivec4& quad,
        const ecmdb::metadata& quad_meta,
        float* processed_quad_min_elev, float* processed_quad_max_elev) const
{
    static_cast<elevation_processor *>(_procs[ecmdb::category_elevation])->
        get_processed_quad_elevation_bounds(frame, dd, lens, quad,
                quad_meta, processed_quad_min_elev, processed_quad_max_elev);
}

void processor::set_e2c_info(int quad_size, float min_elev, float max_elev)
{
    static_cast<e2c_processor *>(_procs[0])->set_e2c_info(quad_size, min_elev, max_elev);
}

bool processor::processing_is_necessary(
        unsigned int frame,
        const database_description& dd, bool lens,
        const ivec4& quad,
        const ecmdb::metadata& quad_meta)
{
    if (dd.processing_parameters[0].category_e2c)
        return _procs[0]->processing_is_necessary(frame, dd, lens, quad, quad_meta);
    else
        return _procs[dd.db.category()]->processing_is_necessary(frame, dd, lens, quad, quad_meta);
}

void processor::process(
        unsigned int frame,
        const database_description& dd, bool lens,
        const ivec4& quad,
        const ecmdb::metadata& quad_meta,
        bool* full_validity,
        ecmdb::metadata* meta)
{
    if (dd.processing_parameters[0].category_e2c)
        _procs[0]->process(frame, dd, lens, quad, quad_meta, full_validity, meta);
    else
        _procs[dd.db.category()]->process(frame, dd, lens, quad, quad_meta, full_validity, meta);
}

void processor::combine(
        unsigned int /* frame */,
        unsigned int /* ndds */, const database_description* const* dds,
        bool lens,
        const ivec4& /* quad */,
        int quads,
        const int* relevant_dds,
        const ecmdb::metadata* metas,
        ecmdb::metadata* meta)
{
    assert(quads >= 2);
    
    int prg_index = quads - 2;
    assert(prg_index >= 0);
    assert(prg_index < max_combinable_quads - 1);
    
    if (_prg_combine[prg_index] == 0) {
        std::string src(COMBINE_FS_GLSL_STR);
        src = str::replace(src, "$n", str::from(quads));
        _prg_combine[prg_index] = xgl::CreateProgram(std::string("combine-") + str::from(quads), "", "", src);
        assert(xgl::CheckError(HERE));
        xgl::LinkProgram(std::string("combine-") + str::from(quads), _prg_combine[prg_index]);
        glUseProgram(_prg_combine[prg_index]);
        assert(xgl::CheckError(HERE));
        int data_texs[quads];
        int mask_texs[quads];
        for (int i = 0; i < quads; i++) {
            data_texs[i] = 2 * i;
            mask_texs[i] = 2 * i + 1;
        }
        glvmUniform(xgl::GetUniformLocation(_prg_combine[prg_index], "data_texs"), quads, data_texs);
        glvmUniform(xgl::GetUniformLocation(_prg_combine[prg_index], "mask_texs"), quads, mask_texs);
        assert(xgl::CheckError(HERE));
        _priorities_loc[prg_index] = xgl::GetUniformLocation(_prg_combine[prg_index], "priorities");
        _weights_loc[prg_index] = xgl::GetUniformLocation(_prg_combine[prg_index], "weights");
        _texcoord_offsets_loc[prg_index] = xgl::GetUniformLocation(_prg_combine[prg_index], "texcoord_offsets");
        _texcoord_factors_loc[prg_index] = xgl::GetUniformLocation(_prg_combine[prg_index], "texcoord_factors");
        assert(xgl::CheckError(HERE));
    }

    int first_non_e2c_index = 0;
    if (dds[0]->processing_parameters[0].category_e2c)
        first_non_e2c_index++;
    assert(!dds[first_non_e2c_index]->processing_parameters[0].category_e2c);
    int dst_overlap = dds[first_non_e2c_index]->db.category() == ecmdb::category_elevation ? 2 : 1;
    int dst_tex_size = dds[first_non_e2c_index]->db.quad_size() + 2 * dst_overlap;
    
    glUseProgram(_prg_combine[prg_index]);
    assert(xgl::CheckError(HERE));
    float priorities[quads];
    float weights[quads];
    float texcoord_offsets[quads];
    float texcoord_factors[quads];
    for (int i = 0; i < quads; i++) {
        priorities[i] = dds[relevant_dds[i]]->priority[lens ? 1 : 0];
        assert(i == 0 || priorities[i - 1] <= priorities[i]);
        weights[i] = dds[relevant_dds[i]]->weight[lens ? 1 : 0];
        GLint src_tex_size;
        glActiveTexture(GL_TEXTURE0 + 2 * i);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &src_tex_size);
        texcoord_offsets[i] = (src_tex_size - dst_tex_size) / 2 / static_cast<float>(src_tex_size);
        texcoord_factors[i] = static_cast<float>(dst_tex_size) / src_tex_size;
    }
    glvmUniform(_priorities_loc[prg_index], quads, priorities);
    glvmUniform(_weights_loc[prg_index], quads, weights);
    glvmUniform(_texcoord_offsets_loc[prg_index], quads, texcoord_offsets);
    glvmUniform(_texcoord_factors_loc[prg_index], quads, texcoord_factors);
    assert(xgl::CheckError(HERE));

    xgl::DrawQuad();
    assert(xgl::CheckError(HERE));

    *meta = ecmdb::metadata();
    if (dds[relevant_dds[0]]->db.category() == ecmdb::category_elevation) {
        meta->elevation.min = metas[0].elevation.min;
        meta->elevation.max = metas[0].elevation.max;
        for (int i = 1; i < quads; i++) {
            if (metas[i].elevation.min < meta->elevation.min)
                meta->elevation.min = metas[i].elevation.min;
            if (metas[i].elevation.max > meta->elevation.max)
                meta->elevation.max = metas[i].elevation.max;
        }
    }
}

void processor::combine_lens(
        unsigned int /* frame */,
        const ivec4& quad,
        int quad_size,
        int total_dst_quad_size,
        const vec3& lens_rel_pos,
        float lens_radius,
        const vec3 quad_corner_rel_pos[4],
        const ecmdb::metadata& meta0,
        const ecmdb::metadata& meta1,
        ecmdb::metadata* meta)
{
    if (_prg_combine_lens == 0) {
        std::string src(COMBINE_LENS_FS_GLSL_STR);
        _prg_combine_lens = xgl::CreateProgram("combine-lens", "", "", src);
        xgl::LinkProgram("combine-lens", _prg_combine_lens);
        glUseProgram(_prg_combine_lens);
        glvmUniform(xgl::GetUniformLocation(_prg_combine_lens, "data0_tex"), 0);
        glvmUniform(xgl::GetUniformLocation(_prg_combine_lens, "mask0_tex"), 1);
        glvmUniform(xgl::GetUniformLocation(_prg_combine_lens, "data1_tex"), 2);
        glvmUniform(xgl::GetUniformLocation(_prg_combine_lens, "mask1_tex"), 3);
        glvmUniform(xgl::GetUniformLocation(_prg_combine_lens, "offsets_tex"), 4);
        _quadcoord_offset_loc = xgl::GetUniformLocation(_prg_combine_lens, "quadcoord_offset");
        _quadcoord_factor_loc = xgl::GetUniformLocation(_prg_combine_lens, "quadcoord_factor");
        _texcoord0_offset_loc = xgl::GetUniformLocation(_prg_combine_lens, "texcoord0_offset");
        _texcoord0_factor_loc = xgl::GetUniformLocation(_prg_combine_lens, "texcoord0_factor");
        _texcoord1_offset_loc = xgl::GetUniformLocation(_prg_combine_lens, "texcoord1_offset");
        _texcoord1_factor_loc = xgl::GetUniformLocation(_prg_combine_lens, "texcoord1_factor");
        _base_data_texcoord_factor_loc = xgl::GetUniformLocation(_prg_combine_lens, "base_data_texcoord_factor");
        _base_data_texcoord_offset_loc = xgl::GetUniformLocation(_prg_combine_lens, "base_data_texcoord_offset");
        _base_data_mirror_x_loc = xgl::GetUniformLocation(_prg_combine_lens, "base_data_mirror_x");
        _base_data_mirror_y_loc = xgl::GetUniformLocation(_prg_combine_lens, "base_data_mirror_y");
        _base_data_matrix_loc = xgl::GetUniformLocation(_prg_combine_lens, "base_data_matrix");
        _lens_pos_loc = xgl::GetUniformLocation(_prg_combine_lens, "lens_pos");
        _lens_radius_loc = xgl::GetUniformLocation(_prg_combine_lens, "lens_radius");
        _quad_bl_loc = xgl::GetUniformLocation(_prg_combine_lens, "quad_bl");
        _quad_br_loc = xgl::GetUniformLocation(_prg_combine_lens, "quad_br");
        _quad_tl_loc = xgl::GetUniformLocation(_prg_combine_lens, "quad_tl");
        _quad_tr_loc = xgl::GetUniformLocation(_prg_combine_lens, "quad_tr");
        assert(xgl::CheckError(HERE));
    }

    int dst_overlap = (total_dst_quad_size - quad_size) / 2;
    assert((total_dst_quad_size - quad_size) % 2 == 0);
    assert(dst_overlap == 1 || dst_overlap == 2);
    float quadcoord_offset = static_cast<float>(-dst_overlap) / quad_size;
    float quadcoord_factor = static_cast<float>(total_dst_quad_size) / quad_size;
    GLint src0_tex_size;
    glActiveTexture(GL_TEXTURE0);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &src0_tex_size);
    float texcoord0_offset = (src0_tex_size - total_dst_quad_size) / 2 / static_cast<float>(src0_tex_size);
    float texcoord0_factor = static_cast<float>(total_dst_quad_size) / src0_tex_size;
    GLint src1_tex_size;
    glActiveTexture(GL_TEXTURE2);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &src1_tex_size);
    float texcoord1_offset = (src1_tex_size - total_dst_quad_size) / 2 / static_cast<float>(src1_tex_size);
    float texcoord1_factor = static_cast<float>(total_dst_quad_size) / src1_tex_size;
    assert(xgl::CheckError(HERE));

    glUseProgram(_prg_combine_lens);
    assert(xgl::CheckError(HERE));
    glvmUniform(_base_data_texcoord_offset_loc, 2.0f / (quad_size + 4));
    glvmUniform(_base_data_texcoord_factor_loc, static_cast<float>(quad_size) / (quad_size + 4));
    bool base_data_mirror_x, base_data_mirror_y;
    mat3 base_data_matrix;
    int devnull;
    ecm::symmetry_quad(quad[0], quad[1], quad[2], quad[3],
            &devnull, &devnull, &devnull, &devnull,
            &base_data_mirror_x, &base_data_mirror_y, base_data_matrix.vl);
    glvmUniform(_base_data_mirror_x_loc, base_data_mirror_x ? 1.0f : 0.0f);
    glvmUniform(_base_data_mirror_y_loc, base_data_mirror_y ? 1.0f : 0.0f);
    glvmUniform(_base_data_matrix_loc, base_data_matrix);
    glvmUniform(_lens_pos_loc, lens_rel_pos);
    glvmUniform(_lens_radius_loc, lens_radius);
    glvmUniform(_quad_bl_loc, quad_corner_rel_pos[ecm::corner_bl]);
    glvmUniform(_quad_br_loc, quad_corner_rel_pos[ecm::corner_br]);
    glvmUniform(_quad_tl_loc, quad_corner_rel_pos[ecm::corner_tl]);
    glvmUniform(_quad_tr_loc, quad_corner_rel_pos[ecm::corner_tr]);
    glvmUniform(_quadcoord_offset_loc, quadcoord_offset);
    glvmUniform(_quadcoord_factor_loc, quadcoord_factor);
    glvmUniform(_texcoord0_offset_loc, texcoord0_offset);
    glvmUniform(_texcoord0_factor_loc, texcoord0_factor);
    glvmUniform(_texcoord1_offset_loc, texcoord1_offset);
    glvmUniform(_texcoord1_factor_loc, texcoord1_factor);
    assert(xgl::CheckError(HERE));

    xgl::DrawQuad();
    assert(xgl::CheckError(HERE));

    *meta = ecmdb::metadata();
    if (meta0.category == ecmdb::category_elevation) {
        meta->elevation.min = min(meta0.elevation.min, meta1.elevation.min);
        meta->elevation.max = max(meta0.elevation.max, meta1.elevation.max);
    }
}

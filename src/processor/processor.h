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

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include <GL/glew.h>

#include "glvm.h"
#include "state.h"


class dbcategory_processor
{
public:
    virtual void init_gl() = 0;
    virtual void exit_gl() = 0;

    virtual bool processing_is_necessary(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::quad_metadata& quad_meta) = 0;
    virtual void process(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::quad_metadata& quad_meta,
            bool* full_validity,
            ecmdb::quad_metadata* meta) = 0;
};

class processor
{
public:
    static const int max_combinable_quads = 8;

private:
    static const int _nprocs = 4;
    dbcategory_processor *_procs[_nprocs];
    GLuint _prg_combine[max_combinable_quads - 1];
    GLint _priorities_loc[max_combinable_quads - 1];
    GLint _weights_loc[max_combinable_quads - 1];
    GLint _texcoord_offsets_loc[max_combinable_quads - 1];
    GLint _texcoord_factors_loc[max_combinable_quads - 1];
    GLuint _prg_combine_lens;
    GLint _quadcoord_offset_loc;
    GLint _quadcoord_factor_loc;
    GLint _texcoord0_offset_loc;
    GLint _texcoord0_factor_loc;
    GLint _texcoord1_offset_loc;
    GLint _texcoord1_factor_loc;
    GLint _base_data_texcoord_factor_loc;
    GLint _base_data_texcoord_offset_loc;
    GLint _base_data_mirror_x_loc;
    GLint _base_data_mirror_y_loc;
    GLint _base_data_matrix_loc;
    GLint _lens_pos_loc;
    GLint _lens_radius_loc;
    GLint _quad_bl_loc;
    GLint _quad_br_loc;
    GLint _quad_tl_loc;
    GLint _quad_tr_loc;

public:
    processor();
    ~processor();

    void init_gl();
    void exit_gl();

    /* Functions that apply only to the elevation processor. */

    // Set total min/max elevation for the entire scene.
    // Elevation processing will never return values outside of these bounds.
    void set_elevation_bounds(float min_elev, float max_elev);
    // Set information required for the e2c processor.
    void set_e2c_info(int quad_size, float min_elev, float max_elev);
    // Give bounds for the min/max elevation of a quad after processing.
    void get_processed_quad_elevation_bounds(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::quad_metadata& quad_meta,
            float* processed_quad_min_elev, float* processed_quad_max_elev) const;

    /**
     * Process the given quad from the given database.
     * Assumptions:
     * - data_tex is bound to texture unit 0
     * - mask_tex is bound to texture unit 1
     * - the active FBO is set up for rendering into two destination textures:
     *   the data texture at COLOR_ATTACHMENT0 and the mask texture at COLOR_ATTACHMENT1
     * - the destination texture has overlap size 1 in case it is of type GL_SRGB
     *   (for texture data) or 2 in case it is of type GL_R32F (for elevation data)
     */
    bool processing_is_necessary(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::quad_metadata& quad_meta);
    void process(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::quad_metadata& quad_meta,
            bool* full_validity,
            ecmdb::quad_metadata* meta);

    /**
     * Combine a set of processed quads into a single quad.
     * Assumptions:
     * - the data texture i is bound to texture unit 2i
     * - the mask texture i is bound to texture unit 2i+1
     * - the active FBO is set up for rendering into two destination textures:
     *   the data texture at COLOR_ATTACHMENT0 and the mask texture at COLOR_ATTACHMENT1
     * - the inputs are sorted according to ascending priority
     * If at least one of the input quads is fully valid, then the output quad will
     * also be fully valid.
     */
    void combine(
            unsigned int frame,
            unsigned int ndds, const database_description* const* dds,
            bool lens,
            const glvm::ivec4& quad,
            int quads,
            const int* relevant_dds,
            const ecmdb::quad_metadata* metas,
            ecmdb::quad_metadata* meta);

    /**
     * Combine the results of with-lens and without-lens
     * processing, for quads that intersect the lens area.
     * This selects the appropriate region from each input.
     * Assumptions:
     * - the data texture 0 is bound to texture unit 0
     * - the mask texture 0 is bound to texture unit 1
     * - the data texture 1 is bound to texture unit 2
     * - the mask texture 1 is bound to texture unit 3
     * - the offsets texture is bound to texture unit 4
     * - the active FBO is set up for rendering into two destination textures:
     *   the data texture at COLOR_ATTACHMENT0 and the mask texture at COLOR_ATTACHMENT1
     * If the input quads are fully valid, then the output quad will
     * also be fully valid.
     */
    void combine_lens(
            unsigned int frame,
            const glvm::ivec4& quad,
            int quad_size,
            int total_dst_quad_size,
            const glvm::vec3& lens_rel_pos, float lens_radius,
            const glvm::vec3 quad_corner_rel_pos[4],
            const ecmdb::quad_metadata& meta0,
            const ecmdb::quad_metadata& meta1,
            ecmdb::quad_metadata* meta);
};

#endif

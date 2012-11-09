/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012
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

#ifndef TERRAIN_H
#define TERRAIN_H

#include <vector>

#include <GL/glew.h>

#include "glvm.h"
#include "xgl.h"

#include "culler.h"
#include "processor.h"
#include "renderer-context.h"
#include "lod.h"

class renderpass_info;

class lod_thread : public thread
{
    /* This is a LOD thread for one depth pass in one frame. */

private:
    // Internal
    culler _culler;
    std::vector<glvm::vec2> _bbs;
    ecm_quadtree* _quadtree;
    std::vector<ecm_side_quadtree*> _lod_quads;
    // Input
    renderer_context* _context;
    unsigned int _frame;
    const class state* _state;
    const processor* _processor;
    glvm::dmat4 _MV;
    int _depth_pass;
    // Input/Output
    glvm::dmat4 _P;
    glvm::ivec4 _VP;
    renderpass_info* _info;
    // Output
    glvm::vec3 _lens_rel_pos;
    glvm::dmat4 _rel_MV;
    float _elevation_min;
    float _elevation_max;
    float _e2c_elevation_min;
    float _e2c_elevation_max;
    unsigned int _n_elevation_dds;
    std::vector<const database_description*> _elevation_dds;
    unsigned int _n_texture_dds;
    std::vector<const database_description*> _texture_dds;
    unsigned int _n_render_quads;
    std::vector<const ecm_side_quadtree*> _render_quads;

    // Helpers
    ecmdb::metadata get_metadata_with_caching(
            const database_description& dd,
            const glvm::ivec4& quad,
            int* level_difference);
    void get_quad_elevation_bounds(
            const glvm::ivec4& quad,
            bool quad_intersects_lens,
            unsigned int ndds, const database_description** dds,
            float* min_elev, float* max_elev, bool* valid);

public:
    lod_thread();
    virtual ~lod_thread();

    void init(renderer_context* context, unsigned int frame,
            const class state* state,
            const class processor* processor,
            const glvm::ivec4& VP,
            const glvm::dmat4& MV,
            int depth_pass,
            const glvm::dmat4& P,
            renderpass_info* info);

    virtual void run();

    const class state& state() const
    {
        return *_state;
    }

    int quad_size() const
    {
        return _state->quad_size();
    }

    const class ecm& ecm() const
    {
        return _quadtree->ecm();
    }

    const glvm::vec3& lens_rel_pos() const
    {
        return _lens_rel_pos;
    }

    const glvm::dmat4& rel_MV() const
    {
        return _rel_MV;
    }

    const glvm::dmat4& P() const
    {
        return _P;
    }

    const glvm::ivec4& VP() const
    {
        return _VP;
    }

    float elevation_min() const
    {
        return _elevation_min;
    }

    float elevation_max() const
    {
        return _elevation_max;
    }

    float e2c_elevation_min() const
    {
        return _e2c_elevation_min;
    }

    float e2c_elevation_max() const
    {
        return _e2c_elevation_max;
    }

    unsigned int n_elevation_dds() const
    {
        return _n_elevation_dds;
    }

    const database_description* const* elevation_dds() const
    {
        return &(_elevation_dds[0]);
    }

    unsigned int n_texture_dds() const
    {
        return _n_texture_dds;
    }

    const database_description* const* texture_dds() const
    {
        return &(_texture_dds[0]);
    }

    unsigned int n_render_quads() const
    {
        return _n_render_quads;
    }

    const ecm_side_quadtree* render_quad(unsigned int i) const
    {
        return _render_quads[i];
    }
};

class depth_pass_renderer
{
private:
    bool _initialized_gl;
    std::vector<unsigned char> _xgl_stack;
    GLuint _fbo, _read_fbo;
    GLuint _pbo[2];
    GLuint _invalid_data_tex;
    GLuint _invalid_mask_tex;
    GLuint _valid_mask_tex;
    GLuint _approx_prg;
    GLuint _approx_minmax_prep_prg;
    GLint _approx_minmax_prep_prg_in_size_loc;
    GLint _approx_minmax_prep_prg_out_size_loc;
    GLuint _approx_minmax_prg;
    GLint _approx_minmax_prg_step_loc;
    int _approx_minmax_pyramid_quad_size;
    std::vector<GLuint> _approx_minmax_pyramid;
    GLuint _cart_coord_prg;
    GLint _cart_coord_prg_quad_bl_loc;
    GLint _cart_coord_prg_quad_br_loc;
    GLint _cart_coord_prg_quad_tr_loc;
    GLint _cart_coord_prg_quad_tl_loc;
    GLint _cart_coord_prg_have_base_data_loc;
    GLint _cart_coord_prg_fallback_normal_loc;
    GLint _cart_coord_prg_base_data_texcoord_offset_loc;
    GLint _cart_coord_prg_base_data_texcoord_factor_loc;
    GLint _cart_coord_prg_base_data_mirror_x_loc;
    GLint _cart_coord_prg_base_data_mirror_y_loc;
    GLint _cart_coord_prg_base_data_matrix_loc;
    GLint _cart_coord_prg_skirt_elevation_loc;
    GLint _cart_coord_prg_have_elevation_loc;
    GLint _cart_coord_prg_fallback_elevation_loc;
    GLint _cart_coord_prg_elevation_texcoord_offset_loc;
    GLint _cart_coord_prg_elevation_texcoord_factor_loc;
    GLint _cart_coord_prg_step_loc;
    GLint _cart_coord_prg_q_offset_loc;
    GLint _cart_coord_prg_q_factor_loc;
    GLuint _render_prg;
    bool _render_prg_lighting;
    bool _render_prg_quad_borders;
    GLint _render_prg_cart_coords_texcoord_offset_loc;
    GLint _render_prg_cart_coords_texcoord_factor_loc;
    GLint _render_prg_cart_coords_halfstep_loc;
    GLint _render_prg_cart_coords_step_loc;
    GLint _render_prg_texture_texcoord_offset_loc;
    GLint _render_prg_texture_texcoord_factor_loc;
    GLint _render_prg_L_loc;
    GLint _render_prg_ambient_color_loc;
    GLint _render_prg_light_color_loc;
    GLint _render_prg_shininess_loc;
    GLuint _quad_vbo;
    int _quad_vbo_subdivision;
    GLenum _quad_vbo_mode;
    GLsizei _quad_vbo_vertices;
    std::vector<bool> _render_flags;
    std::vector<GLuint> _elevation_data_texs;
    std::vector<GLuint> _elevation_mask_texs;
    std::vector<bool> _elevation_data_texs_return_to_pool;
    std::vector<bool> _elevation_mask_texs_return_to_pool;
    std::vector<ecmdb::metadata> _elevation_metas;
    std::vector<GLuint> _texture_data_texs;
    std::vector<GLuint> _texture_mask_texs;
    std::vector<bool> _texture_data_texs_return_to_pool;
    std::vector<bool> _texture_mask_texs_return_to_pool;
    std::vector<ecmdb::metadata> _texture_metas;
    std::vector<GLuint> _cart_coord_texs;

    quad_gpu* create_approximation(
            renderer_context& context,
            const database_description& dd, const glvm::ivec4& quad, int approx_level,
            const quad_gpu* qgpu, size_t* approx_size_on_gpu);

    void get_quad_with_caching(
            renderer_context& context,
            const database_description& dd, const glvm::ivec4& quad,
            GLuint* data_tex, GLuint* mask_tex, ecmdb::metadata* meta,
            int* level_difference);

    void process_and_combine(
            renderer_context& context,
            unsigned int frame,
            class processor& processor,
            unsigned int ndds, const database_description* const* dds,
            int quad_size,
            const ecm_side_quadtree* quad,
            int quad_index,
            GLuint offsets_tex,
            int quad_lens_status,
            const glvm::vec3& lens_rel_pos, float lens_radius,
            const glvm::vec3 quad_corner_rel_pos[4],
            GLuint elevation_data_tex_for_e2c,
            GLuint elevation_mask_tex_for_e2c,
            const ecmdb::metadata& elevation_meta_for_e2c,
            std::vector<GLuint>& data_texs,
            std::vector<GLuint>& mask_texs,
            std::vector<bool>& return_data_texs_to_pool,
            std::vector<bool>& return_mask_texs_to_pool,
            std::vector<ecmdb::metadata>& metas,
            int* approximated_quads);

public:
    depth_pass_renderer();
    ~depth_pass_renderer();

    void init_gl();
    void exit_gl();

    void render(renderer_context *context, unsigned int frame,
            const class state* state,
            class processor* processor,
            int depth_pass,
            const lod_thread* lod_thread,
            renderpass_info* info);
};

class terrain
{
private:
    int _current_lod; // 0 or 1
    int _depth_passes[2];
    class state _state[2];
    renderpass_info* _info[2];
    std::vector<lod_thread*> _lod_threads[2];
    processor _processor;
    depth_pass_renderer _depth_pass_renderer;

public:
    terrain();
    ~terrain();

    void init_gl();
    void exit_gl();

    void render(renderer_context& context, unsigned int frame,
            const glvm::ivec4& VP,
            const glvm::dmat4& MV,
            unsigned int depth_passes,
            const glvm::dmat4* P,
            renderpass_info* info);
};

#endif

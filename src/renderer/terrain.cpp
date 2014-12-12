/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013
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

#include <vector>

#include <GL/glew.h>

#include "msg.h"
#include "dbg.h"
#include "tmr.h"

#include "glvm.h"
#include "glvm-gl.h"
#include "glvm-algo.h"
#include "glvm-str.h"
#include "xgl.h"
#include "xgl-gta.h"

#include "state.h"

#include "lod.h"
#include "culler.h"
#include "renderer.h"
#include "terrain.h"
#include "approx.fs.glsl.h"
#include "approx-minmax-prep.fs.glsl.h"
#include "approx-minmax.fs.glsl.h"
#include "cart-coord.fs.glsl.h"
#include "render.vs.glsl.h"
#include "render.fs.glsl.h"

using namespace glvm;


lod_thread::lod_thread() :
    _bbs(8),
    _quadtree(NULL),
    _n_elevation_dds(0),
    _n_texture_dds(0),
    _n_render_quads(0)
{
}

lod_thread::~lod_thread()
{
}

void lod_thread::init(
        renderer_context* context, unsigned int frame,
        const class state* state,
        const class processor* processor,
        const glvm::ivec4& VP,
        const glvm::dmat4& MV,
        int depth_pass,
        const glvm::dmat4& P,
        renderpass_info* info)
{
    _context = context;
    _frame = frame;
    _state = state;
    _processor = processor;
    _VP = VP;
    _MV = MV;
    _depth_pass = depth_pass;
    _P = P;
    _info = info;

    _n_elevation_dds = 0;
    _n_texture_dds = 0;
    _n_render_quads = 0;
}

ecmdb::metadata lod_thread::get_metadata_with_caching(
        const database_description& dd,
        const glvm::ivec4& quad,
        int* level_difference)
{
    quad_metadata_cache& metadata_cache = *(_context->quad_metadata_cache());
    quad_gpu_cache& gpu_cache = *(_context->quad_gpu_cache());
    quad_mem_cache& mem_cache = *(_context->quad_mem_cache());
    quad_mem_cache_loaders& mem_cache_loaders = *(_context->quad_mem_cache_loaders());
    quad_disk_cache& disk_cache = *(_context->quad_disk_cache());
    quad_disk_cache_checkers& disk_cache_checkers = *(_context->quad_disk_cache_checkers());
    quad_disk_cache_fetchers& disk_cache_fetchers = *(_context->quad_disk_cache_fetchers());

    const ecmdb::metadata* qm;
    const quad_gpu *qgpu;
    const quad_mem *qmem;
    const quad_disk *qdisk;

    msg::dbg("get_metadata_with_caching: %s from %s:", str::from(quad).c_str(), dd.url.c_str());
    int approx_level = quad[1];
    assert(!(dd.uuid == uuid()));
    quad_key key(dd.uuid, quad, approx_level);
    if ((qm = metadata_cache.locked_get(key))) {
        msg::dbg(4, "metadata cache: exact hit");
        *level_difference = 0;
        return *qm;
    } else {
        if (quad[1] >= dd.db.levels()) {
            // We don't have original quads in this level and must look
            // for an approximated quad instead.
            approx_level = dd.db.levels() - 1;
            key = quad_key(dd.uuid, quad, approx_level);
            if ((qm = metadata_cache.locked_get(key))) {
                msg::dbg(4, "metadata cache: found computed approx at leveldiff %d", quad[1] - approx_level);
                *level_difference = 0;
                return *qm;
            } else if ((qgpu = gpu_cache.locked_get(key))) {
                msg::dbg(4, "quad gpu cache: found computed approx at leveldiff %d", quad[1] - approx_level);
                metadata_cache.locked_put(key, new ecmdb::metadata(qgpu->meta));
                *level_difference = 0;
                return qgpu->meta;
            }
        }
        int qs = quad[0];
        int ql = quad[1];
        int qx = quad[2];
        int qy = quad[3];
        if (quad[1] >= dd.db.levels()) {
            // We don't have an approximated quad; fall back to using
            // the metadata of a full, lower-level quad (the one we would
            // approximate from).
            approx_level = dd.db.levels() - 1;
            int ld = quad[1] - approx_level;
            ql = approx_level;
            qx = qx >> ld;
            qy = qy >> ld;
        }
        if (!dd.db.has_quad(qs, ql, qx, qy)) {
            // Return invalid metadata
            msg::dbg(4, "database does not have this quad");
            *level_difference = 0;
            return ecmdb::metadata();
        }
        while (ql >= 0) {
            key = quad_key(dd.uuid, ivec4(qs, ql, qx, qy), ql);
            if ((qm = metadata_cache.locked_get(key))) {
                msg::dbg(4, "metadata cache: approx at leveldiff %d", quad[1] - ql);
                *level_difference = quad[1] - ql;
                return *qm;
            } else if ((qgpu = gpu_cache.locked_get(key))) {
                msg::dbg(4, "quad gpu cache: approx at leveldiff %d", quad[1] - ql);
                metadata_cache.locked_put(key, new ecmdb::metadata(qgpu->meta));
                *level_difference = quad[1] - ql;
                return qgpu->meta;
            } else if ((qmem = mem_cache.locked_get(key))) {
                msg::dbg(4, "quad mem cache: approx at leveldiff %d", quad[1] - ql);
                metadata_cache.locked_put(key, new ecmdb::metadata(qmem->meta));
                *level_difference = quad[1] - ql;
                return qmem->meta;
            } else if ((qdisk = disk_cache.locked_get(key))) {
                switch (qdisk->status) {
                case quad_disk::uncached:
                    if (ql == quad[1] - 1) {
                        msg::dbg(4, "quad disk cache: start fetching at leveldiff %d", quad[1] - ql);
                        (void)disk_cache_fetchers.locked_start_fetch(
                                key, dd.db, dd.url, dd.username, dd.password);
                    }
                    break;
                case quad_disk::cached:
                    if (ql == quad[1] - 1) {
                        msg::dbg(4, "quad disk cache: start loading at leveldiff %d", quad[1] - ql);
                        (void)mem_cache_loaders.locked_start_load(
                                key, dd.db, disk_cache.quad_filename(dd.url, ivec4(qs, ql, qx, qy)));
                    }
                    break;
                case quad_disk::cached_empty:
                    // We have nothing. Return invalid metadata.
                    msg::dbg(4, "quad disk cache: database does not have this quad");
                    *level_difference = 0;
                    return ecmdb::metadata();
                    break;
                case quad_disk::checking:
                case quad_disk::caching:
                    msg::dbg(4, "quad disk cache: waiting for ongoing operation");
                    // disk cache operation ongoing; we need to wait for it
                    break;
                }
            } else {
                if (ql == quad[1] - 1) {
                    msg::dbg(4, "quad disk cache: start checking at leveldiff %d", quad[1] - ql);
                    (void)disk_cache_checkers.locked_start_check(key, disk_cache.quad_filename(dd.url, ivec4(qs, ql, qx, qy)));
                }
            }
            if (ql == 0) {
                assert(qx == 0);
                assert(qy == 0);
                // OK, we have nothing. Use global metadata.
                msg::dbg(4, "falling back to global metadata - ugh!");
                ecmdb::metadata meta(dd.db.category());
                if (dd.db.category() == ecmdb::category_elevation) {
                    meta.elevation.min = dd.meta.elevation.min;
                    meta.elevation.max = dd.meta.elevation.max;
                }
                *level_difference = quad[1];
                return meta;
            }
            ql--;
            qx /= 2;
            qy /= 2;
        }
        // This point can never be reached
        assert(false);
        *level_difference = -1;
        return ecmdb::metadata();
    }
}

void lod_thread::get_quad_elevation_bounds(
        const glvm::ivec4& quad,
        bool quad_intersects_lens,
        unsigned int ndds, const database_description** dds,
        float* min_elev, float* max_elev, bool* valid)
{
    *valid = false;
    *min_elev = +std::numeric_limits<float>::max();
    *max_elev = -std::numeric_limits<float>::max();
    bool have_values = false;
    for (unsigned i = 0; i < ndds; i++) {
        float mine, maxe;
        int level_difference = -1;
        ecmdb::metadata meta = get_metadata_with_caching(*(dds[i]), quad, &level_difference);
        assert(level_difference >= 0);
        if (meta.is_valid()) {
            assert(meta.category == ecmdb::category_elevation);
            assert(isfinite(meta.elevation.min));
            assert(isfinite(meta.elevation.max));
            assert(meta.elevation.max >= meta.elevation.min);
            _processor->get_processed_quad_elevation_bounds(_frame,
                    *(dds[i]), false, quad, meta, &mine, &maxe);
            if (mine < *min_elev)
                *min_elev = mine;
            if (maxe > *max_elev)
                *max_elev = maxe;
            if (quad_intersects_lens) {
                _processor->get_processed_quad_elevation_bounds(_frame,
                        *(dds[i]), true, quad, meta, &mine, &maxe);
                if (mine < *min_elev)
                    *min_elev = mine;
                if (maxe > *max_elev)
                    *max_elev = maxe;
            }
            if (level_difference == 0)
                *valid = true;
            have_values = true;
        }
    }
    if (!have_values) {
        *min_elev = 0.0f;
        *max_elev = 0.0f;
    }
}

void lod_thread::run()
{
    if (!_state->have_databases()) {
        _n_elevation_dds = 0;
        _n_texture_dds = 0;
        _n_render_quads = 0;
        return;
    }
#ifndef NDEBUG
    int64_t time_lod_start = timer::get(timer::monotonic);
#endif

    /* Get relevant information from the state */
    const class ecm ecm(_state->semi_major_axis(), _state->semi_minor_axis());
    const int quad_size = _state->quad_size();
    dvec3 lens_pos;
    ecm.geodetic_to_cartesian(_state->lens.pos[0], _state->lens.pos[1], 0.0, lens_pos.vl);
    _lens_rel_pos = vec3(lens_pos - _state->viewer_pos);
    int max_level = 1;
    _n_elevation_dds = 0;
    _n_texture_dds = 0;
    for (size_t i = 0; i < _state->database_descriptions.size(); i++) {
        const database_description& dd = _state->database_descriptions[i];
        if ((dd.active[0] && dd.priority[0] > 0 && dd.weight[0] > 0.0f)
                || (_state->lens.active && dd.active[1] && dd.priority[1] > 0 && dd.weight[1] > 0.0f)) {
            if (dd.db.levels() - 1 > max_level)
                max_level = dd.db.levels() - 1;
            if (dd.db.category() == ecmdb::category_elevation) {
                if (_elevation_dds.size() < _n_elevation_dds + 1)
                    _elevation_dds.resize(_n_elevation_dds + 1);
                _elevation_dds[_n_elevation_dds] = &dd;
                _n_elevation_dds++;
            } else {
                if (_texture_dds.size() < _n_texture_dds + 1)
                    _texture_dds.resize(_n_texture_dds + 1);
                _texture_dds[_n_texture_dds] = &dd;
                _n_texture_dds++;
            }
        }
    }
    if (_n_texture_dds == 0
            || (_n_texture_dds == 1
                && _texture_dds[0]->processing_parameters[0].category_e2c
                && _n_elevation_dds == 0)) {
        // We have nothing to render onto our geometry
        return;
    }
    assert(max_level < ecmdb::max_levels);
    class quad_base_data_mem_cache& quad_base_data_mem_cache = *(_context->quad_base_data_mem_cache());
    class quad_base_data_mem_cache_computers& quad_base_data_mem_cache_computers = *(_context->quad_base_data_mem_cache_computers());
    class quad_base_data_gpu_cache& quad_base_data_gpu_cache = *(_context->quad_base_data_gpu_cache());

    /* Throw away / recreate obsolete data structures */
    if (!_quadtree || _quadtree->ecm() != ecm) {
        msg::dbg("Rebuilding LOD quadtree from scratch");
        delete _quadtree;
        _quadtree = new ecm_quadtree(ecm);
    }

    /* Compute global information */
    dmat4 MVP = _P * _MV;
    dmat4 rel_T(1.0);
    translation(rel_T) = _state->viewer_pos;
    _rel_MV = _MV * rel_T;
    mat4 rel_MVP = _P * _rel_MV;
    _culler.set_mvp(rel_MVP);
    _elevation_min = _state->inner_bounding_sphere_radius - ecm.semi_major_axis();
    _elevation_max = _state->outer_bounding_sphere_radius - ecm.semi_minor_axis();
    if (_n_elevation_dds == 0) {
        _e2c_elevation_min = 0.0f;
        _e2c_elevation_max = 0.0f;
    } else {
        _e2c_elevation_min = +std::numeric_limits<float>::max();
        _e2c_elevation_max = -std::numeric_limits<float>::max();
        for (unsigned int i = 0; i < _n_elevation_dds; i++) {
            ecmdb::metadata meta;
            meta.elevation.min = _elevation_dds[i]->meta.elevation.min;
            meta.elevation.max = _elevation_dds[i]->meta.elevation.max;
            float mine, maxe;
            _processor->get_processed_quad_elevation_bounds(_frame,
                    *(_elevation_dds[i]), false, ivec4(0), meta, &mine, &maxe);
            if (mine < _e2c_elevation_min)
                _e2c_elevation_min = mine;
            if (maxe > _e2c_elevation_max)
                _e2c_elevation_max = maxe;
            if (_state->lens.active) {
                _processor->get_processed_quad_elevation_bounds(_frame,
                        *(_elevation_dds[i]), true, ivec4(0), meta, &mine, &maxe);
                if (mine < _e2c_elevation_min)
                    _e2c_elevation_min = mine;
                if (maxe > _e2c_elevation_max)
                    _e2c_elevation_max = maxe;
            }
        }
    }

    /* Build LOD quadtree */
    _info->quads_culled[_depth_pass] = 0;
    unsigned int lod_quads = 0;
    if (_lod_quads.size() < 6)
        _lod_quads.resize(6);
    for (int i = 0; i < 6; i++)
        _lod_quads[lod_quads++] = _quadtree->side_root(i);
    _n_render_quads = 0;
    while (lod_quads > 0) {
        ecm_side_quadtree* quad = _lod_quads[--lod_quads];
        msg::dbg("LOD: quad %s", str::from(quad->quad()).c_str());
        bool split;
        int cull = 2;   // 0 = no, 1 = yes, 2 = undecided
        if (quad->level() == 0) {
            /* Always make sure to split the top level so that
             * 1) all childless quads have a parent
             * 2) we can use the ecm_quad_base_data symmetry optimization unconditionally. */
            msg::dbg(4, "splitting: level 0");
            split = true;
        } else {
            if (_state->lens.active) {
                dvec3 quad_center = (quad->corner(0) + quad->corner(1) + quad->corner(2) + quad->corner(3)) / 4.0;
                double quad_radius = max(
                        length(quad->corner(0) - quad_center),
                        length(quad->corner(1) - quad_center),
                        length(quad->corner(2) - quad_center),
                        length(quad->corner(3) - quad_center))
                    + quad->max_dist_to_quad_plane();
                double dist = length(lens_pos - quad_center);
                if (dist <= _state->lens.radius - quad_radius) {
                    quad->lens_status() = 1;
                } else if (dist > _state->lens.radius + quad_radius) {
                    quad->lens_status() = 0;
                } else {
                    quad->lens_status() = 2;
                }
            } else {
                quad->lens_status() = 0;
            }
            float min_elev = 0.0f, max_elev = 0.0f;
            bool minmax_elev_valid = false;
            if (_n_elevation_dds > 0) {
                get_quad_elevation_bounds(quad->quad(), quad->lens_status(),
                        _n_elevation_dds, &(_elevation_dds[0]),
                        &min_elev, &max_elev, &minmax_elev_valid);
            }
            assert(min_elev >= static_cast<float>(_state->inner_bounding_sphere_radius - ecm.semi_major_axis()));
            assert(max_elev <= static_cast<float>(_state->outer_bounding_sphere_radius - ecm.semi_minor_axis()));
            bool minmax_elev_changed = (min_elev < quad->min_elev() || min_elev > quad->min_elev()
                    || max_elev < quad->max_elev() || max_elev > quad->max_elev());
            if (minmax_elev_changed || !quad->max_dist_to_quad_plane_is_valid()) {
                msg::dbg(4, "recomputing bounding box");
                quad->min_elev() = min_elev;
                quad->max_elev() = max_elev;
                // Get quad base data
                ivec4 quad_base_data_sym_quad;
                ecm::symmetry_quad(quad->quad()[0], quad->quad()[1], quad->quad()[2], quad->quad()[3],
                        &(quad_base_data_sym_quad[0]), &(quad_base_data_sym_quad[1]), &(quad_base_data_sym_quad[2]), &(quad_base_data_sym_quad[3]),
                        NULL, NULL, NULL);
                quad_base_data_key qbdkey(quad_base_data_sym_quad);
                const quad_base_data_gpu *qbdgpu = quad_base_data_gpu_cache.locked_get(qbdkey);
                if (!qbdgpu) {
                    const quad_base_data_mem *qbdmem = quad_base_data_mem_cache.locked_get(qbdkey);
                    if (!qbdmem) {
                        double max_dist_to_quad_plane = ecm.max_quad_plane_distance_estimation(
                                quad->quad()[0], quad->quad()[1], quad->quad()[2], quad->quad()[3],
                                quad->plane_normal().vl, quad->plane_distance());
                        if (max_dist_to_quad_plane < 0.01) {
                            msg::dbg(4, "quad %s: max_dist_to_quad_plane estimate = %g: don't need quad base data",
                                    str::from(quad->quad()).c_str(), max_dist_to_quad_plane);
                            quad->max_dist_to_quad_plane() = 0.0;
                            quad->max_dist_to_quad_plane_is_valid() = true;
                            // Remember that we don't need quad base data here
                            quad_base_data_gpu_cache.locked_put(qbdkey, new quad_base_data_gpu(NULL, 0, 0, 0.0));
                            quad_base_data_mem_cache.locked_put(qbdkey, new quad_base_data_mem());
                        } else {
                            quad->max_dist_to_quad_plane() = max_dist_to_quad_plane;
                            quad->max_dist_to_quad_plane_is_valid() = false;
                            (void)quad_base_data_mem_cache_computers.locked_start_compute(qbdkey, ecm, quad_size);
                        }
                    } else {
                        // Do not move the data to the GPU now since we don't know yet if we will need it there.
                        quad->max_dist_to_quad_plane() = qbdmem->max_dist_to_quad_plane;
                        quad->max_dist_to_quad_plane_is_valid() = true;
                    }
                } else {
                    quad->max_dist_to_quad_plane() = qbdgpu->max_dist_to_quad_plane;
                    quad->max_dist_to_quad_plane_is_valid() = true;
                }
                // Compute bounding box
                quad->compute_bounding_box();
            }
            // Check if we are at the highest level that has data for this quad
            bool at_highest_level = true;
            for (unsigned int i = 0; i < _n_texture_dds && at_highest_level; i++) {
                if (_texture_dds[i]->processing_parameters[0].category_e2c)
                    continue;
                if (quad->level() < _texture_dds[i]->db.levels() - 1 && _texture_dds[i]->db.has_quad(
                            quad->quad()[0], quad->quad()[1], quad->quad()[2], quad->quad()[3]))
                    at_highest_level = false;
            }
            if (_texture_dds[0]->processing_parameters[0].category_e2c) {
                for (unsigned int i = 0; i < _n_elevation_dds && at_highest_level; i++) {
                    if (quad->level() < _elevation_dds[i]->db.levels() - 1 && _elevation_dds[i]->db.has_quad(
                                quad->quad()[0], quad->quad()[1], quad->quad()[2], quad->quad()[3]))
                        at_highest_level = false;
                }
            }
            // Now check if we want to split this quad
            if (_state->renderer.fixed_quadtree_depth > 0) {
                split = (quad->level() < _state->renderer.fixed_quadtree_depth - 1);
                msg::dbg(4, "%ssplitting: fixed quadtree depth", split ? "" : "not ");
            } else if (at_highest_level) {
                /* Avoid overflow of quad coordinates and needless splitting of quads that
                 * are at the highest available LOD */
                msg::dbg(4, "not splitting: max level");
                split = false;
            } else {
                /* Do not split the quad if it is outside the view frustum */
                vec3 bb0[4], bb1[4];
                for (int i = 0; i < 4; i++) {
                    bb0[i] = vec3(quad->bounding_box_inner()[i] - _state->viewer_pos);
                    bb1[i] = vec3(quad->bounding_box_outer()[i] - _state->viewer_pos);
                }
                cull = _culler.frustum_cull(bb0, bb1) ? 1 : 0;
                if (cull) {
                    msg::dbg(4, "not splitting: quad is culled");
                    split = false;
                } else {
                    /* Split the quad if the screen space area covered by its bounding box
                     * is larger than the quad size. */
                    for (int i = 0; i < 8; i++) {
                        _bbs[i] = glvmProject(i < 4 ? bb0[i] : bb1[i - 4], rel_MVP, _VP);
                    }
                    std::vector<vec2> bbs_convex_hull = glvm::convex_hull(_bbs);
                    float bbs_area = glvm::polygon_area(bbs_convex_hull);
                    split = (bbs_area / _state->renderer.quad_screen_size_ratio > quad_size * quad_size);
                    msg::dbg(4, "splitting: screen area = %g * quad area", bbs_area / (quad_size * quad_size));
                    if (split && !minmax_elev_valid && quad->quad()[1] > max_level / 2) {
                        // prevent elongated quads as long as we don't have valid data
                        float max_bb_height = max(distance(_bbs[0], _bbs[4]), distance(_bbs[1], _bbs[5]), distance(_bbs[2], _bbs[6]), distance(_bbs[3], _bbs[7]));
                        float max_bb0_side = max(distance(_bbs[0], _bbs[1]), distance(_bbs[1], _bbs[2]), distance(_bbs[2], _bbs[3]), distance(_bbs[3], _bbs[0]));
                        float max_bb1_side = max(distance(_bbs[4], _bbs[5]), distance(_bbs[5], _bbs[6]), distance(_bbs[6], _bbs[7]), distance(_bbs[7], _bbs[4]));
                        if (max_bb_height > 0.5f * max(max_bb0_side, max_bb1_side)) {
                            msg::dbg(4, "not splitting: quad is very high and we have no reliable info yet");
                            split = false;
                        }
                    }
                }
            }
            if (split) {
                msg::dbg(4, "not rendering: splitting");
            } else {
                if (cull == 2) {
                    vec3 bb0[4], bb1[4];
                    for (int i = 0; i < 4; i++) {
                        bb0[i] = vec3(quad->bounding_box_inner()[i] - _state->viewer_pos);
                        bb1[i] = vec3(quad->bounding_box_outer()[i] - _state->viewer_pos);
                    }
                    cull = _culler.frustum_cull(bb0, bb1) ? 1 : 0;
                }
                if (cull == 1) {
                    msg::dbg(4, "not rendering: culled");
                    _info->quads_culled[_depth_pass] += cull;
                } else {
                    msg::dbg(4, "rendering");
                    if (_render_quads.size() < _n_render_quads + 1)
                        _render_quads.resize(_n_render_quads + 1);
                    _render_quads[_n_render_quads++] = quad;
                }
            }
        }
        if (split && !quad->has_children()) {
            msg::dbg(4, "final decision: splitting");
            _quadtree->split(quad);
        } else if (!split && quad->has_children()) {
            msg::dbg(4, "final decision: merging");
            _quadtree->merge(quad);
        }
        if (quad->has_children()) {
            if (_lod_quads.size() < lod_quads + 4)
                _lod_quads.resize(lod_quads + 4);
            _lod_quads[lod_quads++] = quad->child(0);
            _lod_quads[lod_quads++] = quad->child(1);
            _lod_quads[lod_quads++] = quad->child(2);
            _lod_quads[lod_quads++] = quad->child(3);
        }
    }
#ifndef NDEBUG
    int64_t time_lod_stop = timer::get(timer::monotonic);
    msg::dbg("Time: depth pass %d: LOD took %.6f seconds", _depth_pass, (time_lod_stop - time_lod_start) / 1e6f);
#endif
}


depth_pass_renderer::depth_pass_renderer() : _initialized_gl(false)
{
    _elevation_data_texs.resize(1);
    _elevation_mask_texs.resize(1);
    _elevation_data_texs_return_to_pool.resize(1);
    _elevation_mask_texs_return_to_pool.resize(1);
    _elevation_metas.resize(1);
}

depth_pass_renderer::~depth_pass_renderer()
{
}

void depth_pass_renderer::init_gl()
{
    if (!_initialized_gl) {
        glGenFramebuffers(1, &_fbo);
        glGenFramebuffers(1, &_read_fbo);
        glGenBuffers(2, _pbo);
        glGenBuffers(1, &_quad_vbo);
        _quad_vbo_subdivision = -1;
        GLubyte invalid_rgba[4] = { 0, 0, 0, 0 };
        GLubyte valid_rgba[4] = { 0xff, 0xff, 0xff, 0xff };
        _invalid_data_tex = xgl::CreateTex2D(GL_R8, 1, 1, GL_NEAREST);
        xgl::WriteTex2D(_invalid_data_tex, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 4, invalid_rgba, _pbo[0]);
        _invalid_mask_tex = xgl::CreateTex2D(GL_R8, 1, 1, GL_NEAREST);
        xgl::WriteTex2D(_invalid_mask_tex, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 4, invalid_rgba, _pbo[1]);
        _valid_mask_tex = xgl::CreateTex2D(GL_R8, 1, 1, GL_NEAREST);
        xgl::WriteTex2D(_valid_mask_tex, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, 4, valid_rgba, _pbo[1]);
        _approx_prg = 0;
        _approx_minmax_prep_prg = 0;
        _approx_minmax_prg = 0;
        _approx_minmax_pyramid_quad_size = -1;
        _cart_coord_prg = 0;
        _render_prg = 0;
	_initialized_gl = true;
    }
}

void depth_pass_renderer::exit_gl()
{
    if (_initialized_gl) {
        glDeleteFramebuffers(1, &_fbo);
        glDeleteFramebuffers(1, &_read_fbo);
        glDeleteBuffers(2, _pbo);
        glDeleteBuffers(1, &_quad_vbo);
        glDeleteTextures(1, &_invalid_data_tex);
        glDeleteTextures(1, &_invalid_mask_tex);
        glDeleteTextures(1, &_valid_mask_tex);
        xgl::DeleteProgram(_approx_prg);
        xgl::DeleteProgram(_approx_minmax_prep_prg);
        xgl::DeleteProgram(_approx_minmax_prg);
        if (_approx_minmax_pyramid.size() > 0) {
            glDeleteTextures(_approx_minmax_pyramid.size(), &(_approx_minmax_pyramid[0]));
            _approx_minmax_pyramid.clear();
        }
        xgl::DeleteProgram(_cart_coord_prg);
        xgl::DeleteProgram(_render_prg);
        _initialized_gl = false;
    }
}

static quad_gpu* mem_quad_to_gpu(
        renderer_context& context,
        const GLuint pbo[2], const database_description& dd, const quad_mem* qmem, size_t* approx_size_on_gpu)
{
    assert(qmem->data.ptr());

    class quad_tex_pool& quad_tex_pool = *(context.quad_tex_pool());
    int tqs = dd.db.total_quad_size();

    GLuint mask_tex, data_tex;
    GLint internal_format;
    GLenum format;
    GLenum type;
    size_t line_size;

    *approx_size_on_gpu = 0;

    /* Copy the mask */
    if (qmem->mask.ptr()) {
        internal_format = GL_R8;
        format = GL_RED;
        type = GL_UNSIGNED_BYTE;
        line_size = tqs * 1;
        mask_tex = quad_tex_pool.get(internal_format, tqs);
        xgl::WriteTex2D(mask_tex, 0, 0, tqs, tqs, format, type, line_size, qmem->mask.ptr(), pbo[0]);
        *approx_size_on_gpu += tqs * tqs;                           // assuming R8 uses one byte per pixel
    } else {
        mask_tex = 0;
    }

    /* Copy the data */
    assert(dd.db.type() == ecmdb::type_uint8
            || dd.db.type() == ecmdb::type_int16
            || dd.db.type() == ecmdb::type_float32);
    assert(dd.db.channels() == 1 || dd.db.channels() == 3);
    internal_format = (dd.db.category() == ecmdb::category_texture ? (dd.db.channels() == 1 ? GL_SLUMINANCE : GL_SRGB) : GL_R32F);
    format = (dd.db.category() == ecmdb::category_texture && dd.db.channels() == 1 ? GL_LUMINANCE : dd.db.channels() == 1 ? GL_RED : GL_RGB);
    type = (dd.db.type() == ecmdb::type_uint8 ? GL_UNSIGNED_BYTE
            : dd.db.type() == ecmdb::type_int16 ? GL_SHORT : GL_FLOAT);
    line_size = tqs * dd.db.element_size();
    data_tex = quad_tex_pool.get(internal_format, tqs);
    if (type == GL_SHORT) {
        // Accurately undo the normalization to [-1,1] performed by OpenGL.
        glPixelTransferf(GL_RED_BIAS, -0.5f);
        glPixelTransferf(GL_RED_SCALE,
                (static_cast<float>(std::numeric_limits<int16_t>::max())
                 - static_cast<float>(std::numeric_limits<int16_t>::min())) / 2.0f);
    }
    xgl::WriteTex2D(data_tex, 0, 0, tqs, tqs, format, type, line_size, qmem->data.ptr(), pbo[1]);
    if (type == GL_SHORT) {
        glPixelTransferf(GL_RED_BIAS, 0.0f);
        glPixelTransferf(GL_RED_SCALE, 1.0f);
    }
    *approx_size_on_gpu += tqs * tqs * dd.db.element_size();    // assuming the GPU and CPU representations have the same size

    return new quad_gpu(&quad_tex_pool, data_tex, mask_tex, qmem->meta);
}

quad_gpu* depth_pass_renderer::create_approximation(
        renderer_context& context,
        const database_description& dd, const glvm::ivec4& quad, int approx_level,
        const quad_gpu* qgpu, size_t* approx_size_on_gpu)
{
    assert(qgpu->data_tex != 0);

    class quad_tex_pool& quad_tex_pool = *(context.quad_tex_pool());
    int qs = dd.db.quad_size();
    int os = dd.db.overlap();
    int tqs = dd.db.total_quad_size();
    bool have_valid_data = true;

    int level_difference = quad[1] - approx_level;
    int approx_factor = (1 << level_difference);
    float step = 1.0f / tqs;
    float approx_step = step / approx_factor;
    float tx = os * step + (quad[2] % approx_factor) * qs * approx_step - os * approx_step;
    float ty = os * step + (quad[3] % approx_factor) * qs * approx_step - os * approx_step;
    float tw = tqs * approx_step;
    float th = tqs * approx_step;

    assert(xgl::CheckError(HERE));
    xgl::PushViewport(_xgl_stack);
    glViewport(0, 0, tqs, tqs);
    glActiveTexture(GL_TEXTURE0);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    if (_approx_prg == 0) {
        _approx_prg = xgl::CreateProgram("approx", "", "", APPROX_FS_GLSL_STR);
        xgl::LinkProgram("approx", _approx_prg);
        glUseProgram(_approx_prg);
        glvmUniform(xgl::GetUniformLocation(_approx_prg, "data"), 0);
        assert(xgl::CheckError(HERE));
    }
    glUseProgram(_approx_prg);

    GLuint mask_tex = 0;
    if (qgpu->mask_tex != 0) {
        mask_tex = quad_tex_pool.get(xgl::GetTex2DParameter(qgpu->mask_tex, GL_TEXTURE_INTERNAL_FORMAT), tqs);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mask_tex, 0);
        assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
        glBindTexture(GL_TEXTURE_2D, qgpu->mask_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        xgl::DrawQuad(-1.0f, -1.0f, 2.0f, 2.0f, tx, ty, tw, th);
    }

    GLint data_internal_format = xgl::GetTex2DParameter(qgpu->data_tex, GL_TEXTURE_INTERNAL_FORMAT);
    if (data_internal_format == GL_SLUMINANCE) {
        // GL_FRAMEBUFFER_SRGB does not work on GL_SLUMINANCE FBO attachments (why not??),
        // therefore use GL_SRGB instead.
        data_internal_format = GL_SRGB;
    }
    GLuint data_tex = quad_tex_pool.get(data_internal_format, tqs);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data_tex, 0);
    assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
    glBindTexture(GL_TEXTURE_2D, qgpu->data_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    xgl::DrawQuad(-1.0f, -1.0f, 2.0f, 2.0f, tx, ty, tw, th);

    ecmdb::metadata meta(dd.db.category());
    if (dd.db.category() == ecmdb::category_elevation) {
#if 0
        // This reads the whole elevation texture back to CPU and scans it there.
        // This may be relatively slow.
        xgl::ReadTex2DStart(mask_tex, 0, 0, tqs, tqs, GL_RED, GL_UNSIGNED_BYTE, tqs * sizeof(unsigned char), _pbo[0]);
        xgl::ReadTex2DStart(data_tex, 0, 0, tqs, tqs, GL_RED, GL_FLOAT, tqs * sizeof(float), _pbo[1]);
        const unsigned char* mask_buffer = static_cast<const unsigned char*>(xgl::ReadTex2DGetData(_pbo[0]));
        const float* data_buffer = static_cast<const float*>(xgl::ReadTex2DGetData(_pbo[1]));
        have_valid_data = false;
        float min_elev = +std::numeric_limits<float>::max();
        float max_elev = -std::numeric_limits<float>::max();
        for (int i = 0; i < tqs * tqs; i++) {
            unsigned char mask = mask_buffer[i];
            if (mask > 127) {
                have_valid_data = true;
                float v = dd.db.data_offset() + dd.db.data_factor() * data_buffer[i];
                if (v < min_elev)
                    min_elev = v;
                if (v > max_elev)
                    max_elev = v;
            }
        }
        xgl::ReadTex2DFinish(_pbo[0]);
        xgl::ReadTex2DFinish(_pbo[1]);
#endif
#if 1
        // This alternative approach reduces the elevation data on the GPU first
        // to a manageable size and then reads back and scans only the rest.
        // The manageable size here (set in min_pqs) is chosen somewhat arbitrarily;
        // it should work ok for most GPU/CPU combinations.
        const int min_pqs = 32;
        if (_approx_minmax_prep_prg == 0) {
            std::string src(APPROX_MINMAX_PREP_FS_GLSL_STR);
            src = str::replace(src, "$pos_huge_val", str::from(+std::numeric_limits<float>::max()));
            src = str::replace(src, "$neg_huge_val", str::from(-std::numeric_limits<float>::max()));
            _approx_minmax_prep_prg = xgl::CreateProgram("approx-minmax-prep", "", "", src);
            xgl::LinkProgram("approx-minmax-prep", _approx_minmax_prep_prg);
            glUseProgram(_approx_minmax_prep_prg);
            glvmUniform(xgl::GetUniformLocation(_approx_minmax_prep_prg, "data"), 0);
            glvmUniform(xgl::GetUniformLocation(_approx_minmax_prep_prg, "mask"), 1);
            _approx_minmax_prep_prg_in_size_loc = xgl::GetUniformLocation(_approx_minmax_prep_prg, "in_size");
            _approx_minmax_prep_prg_out_size_loc = xgl::GetUniformLocation(_approx_minmax_prep_prg, "out_size");
            assert(xgl::CheckError(HERE));
        }
        if (_approx_minmax_prg == 0) {
            std::string src(APPROX_MINMAX_FS_GLSL_STR);
            src = str::replace(src, "$pos_huge_val", str::from(+std::numeric_limits<float>::max()));
            src = str::replace(src, "$neg_huge_val", str::from(-std::numeric_limits<float>::max()));
            _approx_minmax_prg = xgl::CreateProgram("approx-minmax", "", "", src);
            xgl::LinkProgram("approx-minmax", _approx_minmax_prg);
            glUseProgram(_approx_minmax_prg);
            glvmUniform(xgl::GetUniformLocation(_approx_minmax_prg, "tex"), 0);
            _approx_minmax_prg_step_loc = xgl::GetUniformLocation(_approx_minmax_prg, "step");
            assert(xgl::CheckError(HERE));
        }
        if (_approx_minmax_pyramid_quad_size != tqs) {
            if (_approx_minmax_pyramid.size() > 0) {
                glDeleteTextures(_approx_minmax_pyramid.size(), &(_approx_minmax_pyramid[0]));
                _approx_minmax_pyramid.clear();
            }
            int pqs = next_pow2(tqs) / 2;
            do {
                _approx_minmax_pyramid.push_back(xgl::CreateTex2D(GL_RGB32F, pqs, pqs, GL_LINEAR));
                pqs /= 2;
            }
            while (pqs >= min_pqs);
            assert(xgl::CheckError(HERE));
            _approx_minmax_pyramid_quad_size = tqs;
        }
        int pqs = next_pow2(tqs) / 2;
        int pyramid_level = 0;
        glUseProgram(_approx_minmax_prep_prg);
        glViewport(0, 0, pqs, pqs);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                _approx_minmax_pyramid[pyramid_level], 0);
        assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, mask_tex == 0 ? _valid_mask_tex : mask_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, data_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glvmUniform(_approx_minmax_prep_prg_in_size_loc, static_cast<float>(tqs));
        glvmUniform(_approx_minmax_prep_prg_out_size_loc, static_cast<float>(pqs));
        xgl::DrawQuad();
        assert(xgl::CheckError(HERE));
        glUseProgram(_approx_minmax_prg);
        while (pqs > min_pqs)
        {
            pqs /= 2;
            pyramid_level++;
            glViewport(0, 0, pqs, pqs);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                    _approx_minmax_pyramid[pyramid_level], 0);
            assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
            glBindTexture(GL_TEXTURE_2D, _approx_minmax_pyramid[pyramid_level - 1]);
            glvmUniform(_approx_minmax_prg_step_loc, vec2(1.0f / pqs));
            xgl::DrawQuad();
            assert(xgl::CheckError(HERE));
        }
        xgl::ReadTex2DStart(_approx_minmax_pyramid[pyramid_level], 0, 0, pqs, pqs,
                GL_RGB, GL_FLOAT, pqs * 3 * sizeof(float), _pbo[0]);
        const float* buffer = static_cast<const float*>(xgl::ReadTex2DGetData(_pbo[0]));
        have_valid_data = false;
        float min_elev = +std::numeric_limits<float>::max();
        float max_elev = -std::numeric_limits<float>::max();
        for (int i = 0; i < pqs * pqs; i++) {
            float mask = buffer[3 * i + 2];
            if (mask >= 0.5f) {
                have_valid_data = true;
                float minv = dd.db.data_offset() + dd.db.data_factor() * buffer[3 * i + 0];
                float maxv = dd.db.data_offset() + dd.db.data_factor() * buffer[3 * i + 1];
                if (minv < min_elev)
                    min_elev = minv;
                if (maxv > max_elev)
                    max_elev = maxv;
            }
        }
        xgl::ReadTex2DFinish(_pbo[0]);
#endif
        if (have_valid_data) {
            //msg::wrn("min is %s vs %s", str::from(min_elev).c_str(), str::from(qgpu->meta.elevation.min).c_str());
            //msg::wrn("max is %s vs %s", str::from(max_elev).c_str(), str::from(qgpu->meta.elevation.max).c_str());
            //assert(min_elev >= qgpu->meta.elevation.min);
            //assert(max_elev <= qgpu->meta.elevation.max);
            //assert(max_elev >= min_elev);
            meta.elevation.min = min_elev;
            meta.elevation.max = max_elev;
        } else {
            meta = ecmdb::metadata(); // invalid
        }
    } else {
        meta = qgpu->meta;
    }

    GLuint draw_buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, draw_buffers);
    xgl::PopViewport(_xgl_stack);
    assert(xgl::CheckError(HERE));

    if (!have_valid_data) {
        if (mask_tex != 0) {
            quad_tex_pool.put(mask_tex);
            mask_tex = 0;
        }
        quad_tex_pool.put(data_tex);
        data_tex = 0;
        *approx_size_on_gpu = 0;
    } else {
        *approx_size_on_gpu = 0;
        *approx_size_on_gpu += tqs * tqs;                           // assuming R8 uses one byte per pixel
        *approx_size_on_gpu += tqs * tqs * dd.db.element_size();    // assuming the GPU and CPU representations have the same size
    }
    return new quad_gpu(&quad_tex_pool, data_tex, mask_tex, meta);
}

void depth_pass_renderer::get_quad_with_caching(
        renderer_context& context,
        const database_description& dd, const glvm::ivec4& quad,
        GLuint* data_tex,
        GLuint* mask_tex,
        ecmdb::metadata* meta,
        int* level_difference)
{
    quad_tex_pool& tex_pool = *(context.quad_tex_pool());
    quad_disk_cache& disk_cache = *(context.quad_disk_cache());
    quad_disk_cache_checkers& disk_cache_checkers = *(context.quad_disk_cache_checkers());
    quad_disk_cache_fetchers& disk_cache_fetchers = *(context.quad_disk_cache_fetchers());
    quad_mem_cache& mem_cache = *(context.quad_mem_cache());
    quad_mem_cache_loaders& mem_cache_loaders = *(context.quad_mem_cache_loaders());
    quad_gpu_cache& gpu_cache = *(context.quad_gpu_cache());

    const quad_gpu *qgpu;
    const quad_mem *qmem;
    const quad_disk *qdisk;

    msg::dbg("get_quad_with_caching: %s from %s:", str::from(quad).c_str(), dd.url.c_str());
    if (quad[1] < dd.db.levels()) {
        if (!dd.db.has_quad(quad[0], quad[1], quad[2], quad[3])) {
            msg::dbg(4, "database does not have this quad");
            *data_tex = 0;
            *mask_tex = 0;
            *meta = ecmdb::metadata();
            *level_difference = 0;
            return;
        }
        quad_key key(dd.uuid, quad, quad[1]);
        if ((qgpu = gpu_cache.locked_get(key))) {              // In GPU cache?
            msg::dbg(4, "gpu: exact hit");
            *data_tex = qgpu->data_tex;
            *mask_tex = qgpu->mask_tex;
            *meta = qgpu->meta;
            *level_difference = 0;
            return;
        } else if ((qmem = mem_cache.locked_get(key))) {       // In Memory cache?
            msg::dbg(4, "mem: exact hit");
            size_t s;
            if (qmem->data.ptr()) {
                qgpu = mem_quad_to_gpu(context, _pbo, dd, qmem, &s);
            } else {
                s = 0;
                qgpu = new quad_gpu(&tex_pool, 0, 0, ecmdb::metadata());
            }
            gpu_cache.locked_put(key, qgpu, s);
            *data_tex = qgpu->data_tex;
            *mask_tex = qgpu->mask_tex;
            *meta = qgpu->meta;
            *level_difference = 0;
            return;
        } else if ((qdisk = disk_cache.locked_get(key))) {     // Get disk status
            switch (qdisk->status) {
            case quad_disk::checking:
            case quad_disk::caching:
                // Do nothing now; just wait until this operation is finished
                msg::dbg(4, "disk: op ongoing");
                break;
            case quad_disk::uncached:
                // Start caching this quad. Ignore if the fetcher start fails; we will retry later.
                msg::dbg(4, "disk: start fetching");
                (void)disk_cache_fetchers.locked_start_fetch(key, dd.db, dd.url, dd.username, dd.password);
                break;
            case quad_disk::cached:
                // Start transferring this quad to memory. Ignore if the loader start fails; we will retry later.
                msg::dbg(4, "mem: start loading");
                (void)mem_cache_loaders.locked_start_load(key, dd.db, disk_cache.quad_filename(dd.url, quad));
                break;
            case quad_disk::cached_empty:
                // We can handle this case immediately.
                msg::dbg(4, "disk: exact hit (empty)");
                qgpu = new quad_gpu(&tex_pool, 0, 0, ecmdb::metadata());
                gpu_cache.locked_put(key, qgpu, 0);
                *data_tex = qgpu->data_tex;
                *mask_tex = qgpu->mask_tex;
                *meta = qgpu->meta;
                *level_difference = 0;
                break;
            }
        } else {
            // We do not have a disk status yet; start getting one now.
            // Ignore if the checker start fails; we will retry later.
            msg::dbg(4, "disk: start checking");
            (void)disk_cache_checkers.locked_start_check(key, disk_cache.quad_filename(dd.url, quad));
        }
    }

    // Find the best approximation
    ivec4 approx_quad = ivec4(quad[0], quad[1] - 1, quad[2] / 2, quad[3] / 2);
    while (approx_quad[1] >= 0) {
        if (approx_quad[1] < dd.db.levels()) {
            if (!dd.db.has_quad(approx_quad[0], approx_quad[1], approx_quad[2], approx_quad[3])) {
                msg::dbg(4, "database cannot approximate this quad");
                *data_tex = 0;
                *mask_tex = 0;
                *meta = ecmdb::metadata();
                *level_difference = 0;
                return;
            }
            quad_key approx_key(dd.uuid, quad, approx_quad[1]);
            if ((qgpu = gpu_cache.locked_get(approx_key))) {           // Approximation in GPU cache?
                msg::dbg(4, "gpu: approx hit at leveldiff %d", quad[1] - approx_quad[1]);
                *data_tex = qgpu->data_tex;
                *mask_tex = qgpu->mask_tex;
                *meta = qgpu->meta;
                *level_difference = (approx_quad[1] == dd.db.levels() - 1 ? 0 : quad[1] - approx_quad[1]);
                return;
            }
            quad_key key(dd.uuid, approx_quad, approx_quad[1]);
            if ((qgpu = gpu_cache.locked_get(key))) {                  // Original in GPU cache?
                msg::dbg(4, "gpu: create approx at leveldiff %d", quad[1] - approx_quad[1]);
                size_t s;
                if (qgpu->data_tex == 0) {
                    s = 0;
                    qgpu = new quad_gpu(&tex_pool, 0, 0, ecmdb::metadata());
                } else {
                    qgpu = create_approximation(context, dd, quad, approx_quad[1], qgpu, &s);
                }
                gpu_cache.locked_put(approx_key, qgpu, s);
                *data_tex = qgpu->data_tex;
                *mask_tex = qgpu->mask_tex;
                *meta = qgpu->meta;
                *level_difference = (approx_quad[1] == dd.db.levels() - 1 ? 0 : quad[1] - approx_quad[1]);
                return;
            }
            if ((qmem = mem_cache.locked_get(key))) {                  // Original in Memory cache?
                msg::dbg(4, "mem: create approx at leveldiff %d", quad[1] - approx_quad[1]);
                size_t s;
                if (qmem->data.ptr()) {
                    qgpu = mem_quad_to_gpu(context, _pbo, dd, qmem, &s);
                } else {
                    s = 0;
                    qgpu = new quad_gpu(&tex_pool, 0, 0, ecmdb::metadata());
                }
                gpu_cache.locked_put(key, qgpu, s);
                if (qgpu->data_tex == 0) {
                    s = 0;
                    qgpu = new quad_gpu(&tex_pool, 0, 0, ecmdb::metadata());
                } else {
                    qgpu = create_approximation(context, dd, quad, approx_quad[1], qgpu, &s);
                }
                gpu_cache.locked_put(approx_key, qgpu, s);
                *data_tex = qgpu->data_tex;
                *mask_tex = qgpu->mask_tex;
                *meta = qgpu->meta;
                *level_difference = (approx_quad[1] == dd.db.levels() - 1 ? 0 : quad[1] - approx_quad[1]);
                return;
            } else if (approx_quad[1] == dd.db.levels() - 1) {
                if ((qdisk = disk_cache.locked_get(key))) {
                    switch (qdisk->status) {
                    case quad_disk::checking:
                    case quad_disk::caching:
                        // Do nothing now; just wait until this operation is finished
                        msg::dbg(4, "disk: approx op ongoing");
                        break;
                    case quad_disk::uncached:
                        // Start caching this quad. Ignore if the fetcher start fails; we will retry later.
                        msg::dbg(4, "disk: approx start fetching");
                        (void)disk_cache_fetchers.locked_start_fetch(key, dd.db, dd.url, dd.username, dd.password);
                        break;
                    case quad_disk::cached:
                        // Start transferring this quad to memory. Ignore if the loader start fails; we will retry later.
                        msg::dbg(4, "mem: approx start loading");
                        (void)mem_cache_loaders.locked_start_load(key, dd.db, disk_cache.quad_filename(dd.url, approx_quad));
                        break;
                    case quad_disk::cached_empty:
                        // We can handle this case immediately.
                        msg::dbg(4, "disk: approx hit (empty)");
                        qgpu = new quad_gpu(&tex_pool, 0, 0, ecmdb::metadata());
                        gpu_cache.locked_put(key, qgpu, 0);
                        *data_tex = qgpu->data_tex;
                        *mask_tex = qgpu->mask_tex;
                        *meta = qgpu->meta;
                        *level_difference = 0;
                        break;
                    }
                } else {
                    // We do not have a disk status yet; start getting one now.
                    // Ignore if the checker start fails; we will retry later.
                    msg::dbg(4, "disk: approx start checking");
                    (void)disk_cache_checkers.locked_start_check(key, disk_cache.quad_filename(dd.url, quad));
                }
            }
        }
        approx_quad[1]--;
        approx_quad[2] /= 2;
        approx_quad[3] /= 2;
    }

    /* Last resort: Load the root quad from disk. This will block. */

    msg::dbg(4, "approx from root quad - ugh!");
    approx_quad = ivec4(quad[0], 0, 0, 0);
    quad_key key(dd.uuid, approx_quad, approx_quad[1]);
    // Check root quad
    std::string quad_filename = disk_cache.quad_filename(dd.url, approx_quad);
    quad_disk_cache_checker disk_cache_checker(key, quad_filename);
    msg::dbg(4, "disk: checking root quad");
    disk_cache_checker.start();
    disk_cache_checker.finish();
    if (disk_cache_checker.result == quad_disk::uncached) {
        msg::dbg(4, "disk: fetching root quad");
        quad_disk_cache_fetcher disk_cache_fetcher(&disk_cache, key, dd.db, dd.url, dd.username, dd.password);
        disk_cache_fetcher.start();
        disk_cache_fetcher.finish();
        msg::dbg(4, "disk: checking root quad");
        disk_cache_checker.start();
        disk_cache_checker.finish();
    }
    assert(disk_cache_checker.result == quad_disk::cached
            || disk_cache_checker.result == quad_disk::cached_empty);
    msg::dbg(4, "disk: caching check result");
    disk_cache.locked_put(key, new quad_disk(disk_cache_checker.result));
    // Transfer root quad to mem
    size_t s;
    if (disk_cache_checker.result == quad_disk::cached) {
        msg::dbg(4, "mem: loading root quad");
        quad_mem_cache_loader mem_cache_loader(key, dd.db, disk_cache.quad_filename(dd.url, approx_quad));
        mem_cache_loader.start();
        mem_cache_loader.finish();
        qmem = mem_cache_loader.quad_mem.release();
        s = mem_cache_loader.quad_mem_size;
    } else {
        qmem = new quad_mem();
        s = 0;
    }
    msg::dbg(4, "mem: caching root quad");
    mem_cache.locked_put(key, qmem, s);
    // Transfer root quad to GPU
    if (qmem->data.ptr()) {
        qgpu = mem_quad_to_gpu(context, _pbo, dd, qmem, &s);
    } else {
        s = 0;
        qgpu = new quad_gpu(&tex_pool, 0, 0, ecmdb::metadata());
    }
    msg::dbg(4, "gpu: caching root quad");
    gpu_cache.locked_put(key, qgpu, s);
    // Compute approximation
    if (qgpu->data_tex == 0) {
        s = 0;
        qgpu = new quad_gpu(&tex_pool, 0, 0, ecmdb::metadata());
    } else {
        qgpu = create_approximation(context, dd, quad, approx_quad[1], qgpu, &s);
    }
    msg::dbg(4, "gpu: caching approximation");
    gpu_cache.locked_put(quad_key(dd.uuid, quad, 0), qgpu, s);
    *data_tex = qgpu->data_tex;
    *mask_tex = qgpu->mask_tex;
    *meta = qgpu->meta;
    *level_difference = quad[1] - 0;
}

void depth_pass_renderer::process_and_combine(
        renderer_context& context,
        unsigned int frame,
        class processor& processor,
        unsigned int ndds, const database_description* const* dds,
        int quad_size,
        const ecm_side_quadtree* quad,
        int quad_index,
        GLuint offsets_tex,
        int quad_lens_status,
        const vec3& lens_rel_pos, float lens_radius,
        const vec3 quad_corner_rel_pos[4],
        GLuint elevation_data_tex_for_e2c,
        GLuint elevation_mask_tex_for_e2c,
        const ecmdb::metadata& elevation_meta_for_e2c,
        std::vector<GLuint>& data_texs,
        std::vector<GLuint>& mask_texs,
        std::vector<bool>& return_data_texs_to_pool,
        std::vector<bool>& return_mask_texs_to_pool,
        std::vector<ecmdb::metadata>& metas,
        int* approximated_quads)
{
    int relevant_quads = 0;
    int relevant_dds[processor::max_combinable_quads];
    GLuint relevant_data_texs[processor::max_combinable_quads];
    GLuint relevant_mask_texs[processor::max_combinable_quads];
    ecmdb::metadata relevant_metas[processor::max_combinable_quads];

    class quad_tex_pool& quad_tex_pool = *(context.quad_tex_pool());

    GLuint data_tex, mask_tex;
    ecmdb::metadata meta;
    int level_difference;
    bool quad_is_approximated = false;

    assert(xgl::CheckError(HERE));

    bool lens = (quad_lens_status == 0 ? false : true);
    int lens_index = (lens ? 1 : 0);

    for (size_t i = 0; i < ndds; i++) {
        if (!dds[i]->active[lens_index]
                || dds[i]->priority[lens_index] <= 0
                || dds[i]->weight[lens_index] <= 0.0f) {
            continue;
        }
        data_tex = 0;
        if (dds[i]->processing_parameters[0].category_e2c) {
            data_tex = elevation_data_tex_for_e2c;
            mask_tex = elevation_mask_tex_for_e2c;
            meta = elevation_meta_for_e2c;
            level_difference = 0;
        } else {
            get_quad_with_caching(context, *(dds[i]), quad->quad(),
                    &data_tex, &mask_tex, &meta, &level_difference);
        }
        if (data_tex != 0) {
            if (relevant_quads >= processor::max_combinable_quads) {
                throw exc(std::string("Cannot combine more than ")
                        + str::from(processor::max_combinable_quads) + " quads");
            }
            // Insert, sorted to ascending priority.
            int slot = relevant_quads;
            while (slot > 0 && dds[relevant_dds[slot - 1]]->priority[lens_index] > dds[i]->priority[lens_index]) {
                if (relevant_mask_texs[slot - 1] == 0) {
                    // A higher priority quad with full validity cancels out all lower priority quads.
                    slot = -1;
                } else {
                    slot--;
                }
            }
            if (slot >= 0) {
                if (slot > 0) {
                    for (int j = relevant_quads; j >= slot; j--) {
                        relevant_dds[j] = relevant_dds[j - 1];
                        relevant_data_texs[j] = relevant_data_texs[j - 1];
                        relevant_mask_texs[j] = relevant_mask_texs[j - 1];
                        relevant_metas[j] = relevant_metas[j - 1];
                    }
                }
                relevant_dds[slot] = i;
                relevant_data_texs[slot] = data_tex;
                relevant_mask_texs[slot] = mask_tex;
                relevant_metas[slot] = meta;
                relevant_quads++;
                if (level_difference > 0) {
                    quad_is_approximated = true;
                }
                // A higher priority quad with full validity cancels out all lower priority quads.
                if (relevant_mask_texs[slot] == 0 && slot > 0) {
                    int overridden_quads = 0;
                    for (int j = 0; j < slot; j++) {
                        if (dds[relevant_dds[j]]->priority[lens_index]
                                < dds[relevant_dds[slot]]->priority[lens_index]) {
                            overridden_quads++;
                        }
                    }
                    if (overridden_quads > 0) {
                        for (int j = overridden_quads; j < relevant_quads; j++) {
                            relevant_dds[j - overridden_quads] = relevant_dds[j];
                            relevant_data_texs[j - overridden_quads] = relevant_data_texs[j];
                            relevant_mask_texs[j - overridden_quads] = relevant_mask_texs[j];
                            relevant_metas[j - overridden_quads] = relevant_metas[j];
                        }
                        relevant_quads -= overridden_quads;
                    }
                }
            }
        }
    }
    int dst_overlap = (relevant_quads > 0 ? dds[0]->db.category() == ecmdb::category_elevation ? 2 : 1 : 0);
    GLint data_internal_format = (relevant_quads > 0 ? dds[0]->db.category() == ecmdb::category_elevation ? GL_R32F : GL_SRGB : 0);
    msg::dbg("%d relevant data quads for quad %s", relevant_quads, str::from(quad->quad()).c_str());
    if (relevant_quads == 0) {
        // nothing here...
        data_texs[quad_index] = 0;
        mask_texs[quad_index] = 0;
        return_data_texs_to_pool[quad_index] = false;
        return_mask_texs_to_pool[quad_index] = false;
        metas[quad_index] = ecmdb::metadata();
    } else if (relevant_quads == 1
            && !processor.processing_is_necessary(frame, *(dds[relevant_dds[0]]),
                lens, quad->quad(), relevant_metas[0])) {
        // shortcut for a common case: avoid processing and combining entirely
        data_texs[quad_index] = relevant_data_texs[0];
        mask_texs[quad_index] = relevant_mask_texs[0];
        return_data_texs_to_pool[quad_index] = false;
        return_mask_texs_to_pool[quad_index] = false;
        metas[quad_index] = relevant_metas[0];
    } else if (relevant_quads == 1) {
        // shortcut for a common case: avoid combining
        data_texs[quad_index] = quad_tex_pool.get(data_internal_format, quad_size + 2 * dst_overlap);
        mask_texs[quad_index] = quad_tex_pool.get(GL_R8, quad_size + 2 * dst_overlap);
        return_data_texs_to_pool[quad_index] = true;
        return_mask_texs_to_pool[quad_index] = true;
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data_texs[quad_index], 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mask_texs[quad_index], 0);
        assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, relevant_data_texs[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, relevant_mask_texs[0] == 0 ? _valid_mask_tex : relevant_mask_texs[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        bool full_validity = (relevant_mask_texs[0] == 0);
        processor.process(frame, *(dds[relevant_dds[0]]), lens, quad->quad(), relevant_metas[0], &full_validity, &(metas[quad_index]));
        if (full_validity) {
            quad_tex_pool.put(mask_texs[quad_index]);
            mask_texs[quad_index] = 0;
            return_mask_texs_to_pool[quad_index] = false;
        }
    } else {
        // the general case: processing may be necessary, and combining is necessary
        // process (if necessary)
        GLuint processed_data_texs[relevant_quads];
        GLuint processed_mask_texs[relevant_quads];
        bool return_processed_data_texs_to_pool[relevant_quads];
        bool return_processed_mask_texs_to_pool[relevant_quads];
        std::vector<ecmdb::metadata> processed_metas(relevant_quads);
        for (int i = 0; i < relevant_quads; i++) {
            if (processor.processing_is_necessary(frame, *(dds[relevant_dds[i]]), lens, quad->quad(), relevant_metas[i])) {
                processed_data_texs[i] = quad_tex_pool.get(data_internal_format, quad_size + 2 * dst_overlap);
                processed_mask_texs[i] = quad_tex_pool.get(GL_R8, quad_size + 2 * dst_overlap);
                return_processed_data_texs_to_pool[i] = true;
                return_processed_mask_texs_to_pool[i] = true;
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, processed_data_texs[i], 0);
                glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, processed_mask_texs[i], 0);
                assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, relevant_data_texs[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, relevant_mask_texs[i] == 0 ? _valid_mask_tex : relevant_mask_texs[i]);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                bool full_validity = (relevant_mask_texs[i] == 0);
                processor.process(frame, *(dds[relevant_dds[i]]), lens, quad->quad(), relevant_metas[i], &full_validity, &(processed_metas[i]));
                if (full_validity) {
                    quad_tex_pool.put(processed_mask_texs[i]);
                    processed_mask_texs[i] = 0;
                    return_processed_mask_texs_to_pool[i] = false;
                }
            } else {
                processed_data_texs[i] = relevant_data_texs[i];
                processed_mask_texs[i] = relevant_mask_texs[i];
                return_processed_data_texs_to_pool[i] = false;
                return_processed_mask_texs_to_pool[i] = false;
                processed_metas[i] = relevant_metas[i];
            }
        }
        // combine
        data_texs[quad_index] = quad_tex_pool.get(data_internal_format, quad_size + 2 * dst_overlap);
        mask_texs[quad_index] = quad_tex_pool.get(GL_R8, quad_size + 2 * dst_overlap);
        return_data_texs_to_pool[quad_index] = true;
        return_mask_texs_to_pool[quad_index] = true;
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data_texs[quad_index], 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mask_texs[quad_index], 0);
        assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
        bool full_validity = false;
        for (int i = 0; i < relevant_quads; i++) {
            glActiveTexture(GL_TEXTURE0 + 2 * i + 0);
            glBindTexture(GL_TEXTURE_2D, processed_data_texs[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glActiveTexture(GL_TEXTURE0 + 2 * i + 1);
            glBindTexture(GL_TEXTURE_2D, processed_mask_texs[i] == 0 ? _valid_mask_tex : processed_mask_texs[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            if (processed_mask_texs[i] == 0) {
                full_validity = true;
            }
        }
        processor.combine(frame, ndds, dds, lens, quad->quad(), relevant_quads, relevant_dds, &processed_metas[0], &(metas[quad_index]));
        if (full_validity) {
            quad_tex_pool.put(mask_texs[quad_index]);
            mask_texs[quad_index] = 0;
            return_mask_texs_to_pool[quad_index] = false;
        }
        // cleanup
        for (int i = 0; i < relevant_quads; i++) {
            if (return_processed_data_texs_to_pool[i])
                quad_tex_pool.put(processed_data_texs[i]);
            if (return_processed_mask_texs_to_pool[i])
                quad_tex_pool.put(processed_mask_texs[i]);
        }
    }
    assert(xgl::CheckError(HERE));

    if (quad_lens_status == 2) {
        // We processed with lens == true.
        // Backup the results and process with lens == false.
        // Then combine both results into the final result.
        GLuint l1_data_tex = data_texs[quad_index];
        GLuint l1_mask_tex = mask_texs[quad_index];
        bool l1_return_data_texs_to_pool = return_data_texs_to_pool[quad_index];
        bool l1_return_mask_texs_to_pool = return_mask_texs_to_pool[quad_index];
        ecmdb::metadata l1_meta = metas[quad_index];
        process_and_combine(context, frame, processor, ndds, dds, quad_size, quad, quad_index, offsets_tex,
                0, lens_rel_pos, lens_radius, quad_corner_rel_pos,
                elevation_data_tex_for_e2c, elevation_mask_tex_for_e2c, elevation_meta_for_e2c,
                data_texs, mask_texs, return_data_texs_to_pool, return_mask_texs_to_pool, metas, approximated_quads);
        GLuint l0_data_tex = data_texs[quad_index];
        GLuint l0_mask_tex = mask_texs[quad_index];
        bool l0_return_data_texs_to_pool = return_data_texs_to_pool[quad_index];
        bool l0_return_mask_texs_to_pool = return_mask_texs_to_pool[quad_index];
        ecmdb::metadata l0_meta = metas[quad_index];

        if (l0_data_tex == 0 && l1_data_tex == 0) {
            data_texs[quad_index] = 0;
            mask_texs[quad_index] = 0;
            return_data_texs_to_pool[quad_index] = false;
            return_mask_texs_to_pool[quad_index] = false;
        } else {
            data_internal_format = (l0_data_tex != 0
                    ? xgl::GetTex2DParameter(l0_data_tex, GL_TEXTURE_INTERNAL_FORMAT)
                    : xgl::GetTex2DParameter(l1_data_tex, GL_TEXTURE_INTERNAL_FORMAT));
            data_texs[quad_index] = quad_tex_pool.get(data_internal_format, quad_size + 2 * dst_overlap);
            mask_texs[quad_index] = quad_tex_pool.get(GL_R8, quad_size + 2 * dst_overlap);
            return_data_texs_to_pool[quad_index] = true;
            return_mask_texs_to_pool[quad_index] = true;
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data_texs[quad_index], 0);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mask_texs[quad_index], 0);
            assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, l0_data_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, l0_mask_tex == 0 ? _valid_mask_tex : l0_mask_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, l1_data_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, l1_mask_tex == 0 ? _valid_mask_tex : l1_mask_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, offsets_tex);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            processor.combine_lens(frame, quad->quad(), quad_size, quad_size + 2 * dst_overlap,
                    lens_rel_pos, lens_radius, quad_corner_rel_pos,
                    l0_meta, l1_meta, &(metas[quad_index]));
            bool full_validity = (l0_mask_tex == 0 && l1_mask_tex == 0);
            if (full_validity) {
                quad_tex_pool.put(mask_texs[quad_index]);
                mask_texs[quad_index] = 0;
                return_mask_texs_to_pool[quad_index] = false;
            }
            if (l0_return_data_texs_to_pool)
                quad_tex_pool.put(l0_data_tex);
            if (l0_return_mask_texs_to_pool)
                quad_tex_pool.put(l0_mask_tex);
            if (l1_return_data_texs_to_pool)
                quad_tex_pool.put(l1_data_tex);
            if (l1_return_mask_texs_to_pool)
                quad_tex_pool.put(l1_mask_tex);
        }
    }
    if (quad_is_approximated) {
        (*approximated_quads)++;
    }
}

static void set_subquad(std::vector<float>& subquads, size_t i, float bl_x, float bl_y, float size)
{
    assert(i < subquads.size() * 4 * 2);
    subquads[2 * (4 * i + 0) + 0] = bl_x;
    subquads[2 * (4 * i + 0) + 1] = bl_y;
    subquads[2 * (4 * i + 1) + 0] = bl_x + size;
    subquads[2 * (4 * i + 1) + 1] = bl_y;
    subquads[2 * (4 * i + 2) + 0] = bl_x + size;
    subquads[2 * (4 * i + 2) + 1] = bl_y + size;
    subquads[2 * (4 * i + 3) + 0] = bl_x;
    subquads[2 * (4 * i + 3) + 1] = bl_y + size;
}

static void draw_bounding_box(const ecm_side_quadtree* quad, const dvec3& viewer_pos, bool highlight = false)
{
    ivec4 q = quad->quad();
    //int qs = q[0];
    int ql = q[1];
    int qx = q[2];
    int qy = q[3];

    GLint prg_bak;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prg_bak);

    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);       // XXX should not be necessary, but removing it gives wrong line colors!?
    glDisable(GL_TEXTURE_2D);

    if (highlight) {
        glColor3f(1.0f, 1.0f, 0.0f);
        glLineWidth(4.0f);
    } else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    if (true) {
        glBegin(GL_LINE_LOOP);
        glvmVertex(quad->corner(ecm::corner_tl) - viewer_pos);
        glvmVertex(quad->corner(ecm::corner_tr) - viewer_pos);
        glvmVertex(quad->corner(ecm::corner_br) - viewer_pos);
        glvmVertex(quad->corner(ecm::corner_bl) - viewer_pos);
        glEnd();
        if (!highlight)
            glColor3f(1.0f, 0.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glvmVertex(quad->bounding_box_inner()[ecm::corner_tl] - viewer_pos);
        glvmVertex(quad->bounding_box_inner()[ecm::corner_tr] - viewer_pos);
        glvmVertex(quad->bounding_box_inner()[ecm::corner_br] - viewer_pos);
        glvmVertex(quad->bounding_box_inner()[ecm::corner_bl] - viewer_pos);
        glEnd();
        if (!highlight)
            glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINE_LOOP);
        glvmVertex(quad->bounding_box_outer()[ecm::corner_tl] - viewer_pos);
        glvmVertex(quad->bounding_box_outer()[ecm::corner_tr] - viewer_pos);
        glvmVertex(quad->bounding_box_outer()[ecm::corner_br] - viewer_pos);
        glvmVertex(quad->bounding_box_outer()[ecm::corner_bl] - viewer_pos);
        glEnd();
        if (!highlight)
            glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_LINES);
        glvmVertex(quad->bounding_box_inner()[ecm::corner_tl] - viewer_pos);
        glvmVertex(quad->bounding_box_outer()[ecm::corner_tl] - viewer_pos);
        glvmVertex(quad->bounding_box_inner()[ecm::corner_tr] - viewer_pos);
        glvmVertex(quad->bounding_box_outer()[ecm::corner_tr] - viewer_pos);
        glvmVertex(quad->bounding_box_inner()[ecm::corner_bl] - viewer_pos);
        glvmVertex(quad->bounding_box_outer()[ecm::corner_bl] - viewer_pos);
        glvmVertex(quad->bounding_box_inner()[ecm::corner_br] - viewer_pos);
        glvmVertex(quad->bounding_box_outer()[ecm::corner_br] - viewer_pos);
        glEnd();
    } else {
        glBegin(GL_LINES);
        if (qx == 0) {
            glvmVertex(quad->corner(ecm::corner_tl) - viewer_pos);
            glvmVertex(quad->corner(ecm::corner_bl) - viewer_pos);
        }
        if (qx == (1 << ql) - 1) {
            glvmVertex(quad->corner(ecm::corner_tr) - viewer_pos);
            glvmVertex(quad->corner(ecm::corner_br) - viewer_pos);
        }
        if (qy == 0) {
            glvmVertex(quad->corner(ecm::corner_tl) - viewer_pos);
            glvmVertex(quad->corner(ecm::corner_tr) - viewer_pos);
        }
        if (qy == (1 << ql) - 1) {
            glvmVertex(quad->corner(ecm::corner_bl) - viewer_pos);
            glvmVertex(quad->corner(ecm::corner_br) - viewer_pos);
        }
        glEnd();
    }

    glEnable(GL_TEXTURE_2D);
    glUseProgram(prg_bak);
    if (highlight) {
        glLineWidth(1.0f);
    }
}

void depth_pass_renderer::render(renderer_context* context, unsigned int frame,
        const class state* state,
        class processor* processor,
        int depth_pass,
        const lod_thread* lod_thread,
        renderpass_info* info)
{
    assert(_initialized_gl);

    if (lod_thread->n_render_quads() == 0) {
        msg::dbg("Depth pass %d: no quads to render; terminating early", depth_pass);
        return;
    }

    /* Initialize global information */
    const int quad_size = lod_thread->quad_size();
    const unsigned int render_quads = lod_thread->n_render_quads();
    class quad_base_data_mem_cache& quad_base_data_mem_cache = *(context->quad_base_data_mem_cache());
    class quad_base_data_mem_cache_computers& quad_base_data_mem_cache_computers = *(context->quad_base_data_mem_cache_computers());
    class quad_base_data_gpu_cache& quad_base_data_gpu_cache = *(context->quad_base_data_gpu_cache());
    class quad_tex_pool& quad_tex_pool = *(context->quad_tex_pool());
    processor->set_elevation_bounds(lod_thread->elevation_min(), lod_thread->elevation_max());
    processor->set_e2c_info(quad_size, lod_thread->e2c_elevation_min(), lod_thread->e2c_elevation_max());

    /* Initialize GL context */
    xgl::PushEverything(_xgl_stack);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);
    glDisable(GL_DITHER);
    glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
    glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
    glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    GLboolean scissor_bak = glIsEnabled(GL_SCISSOR_TEST);
    glDisable(GL_SCISSOR_TEST);
    GLint draw_framebuffer_bak, read_framebuffer_bak;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &draw_framebuffer_bak);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &read_framebuffer_bak);
    GLint draw_buffer_bak, read_buffer_bak;
    glGetIntegerv(GL_DRAW_BUFFER, &draw_buffer_bak);
    glGetIntegerv(GL_READ_BUFFER, &read_buffer_bak);

    /* Prepare rendering relative to viewer */
    glMatrixMode(GL_PROJECTION);
    glvmLoadMatrix(lod_thread->P());
    glMatrixMode(GL_MODELVIEW);
    glvmLoadMatrix(lod_thread->rel_MV());

    /* Process/combine the data sets. */
    if (_cart_coord_prg == 0) {
        std::string src(CART_COORD_FS_GLSL_STR);
        _cart_coord_prg = xgl::CreateProgram("cart-coord", "", "", src);
        assert(xgl::CheckError(HERE));
        xgl::LinkProgram("cart-coord", _cart_coord_prg);
        assert(xgl::CheckError(HERE));
        glUseProgram(_cart_coord_prg);
        glvmUniform(xgl::GetUniformLocation(_cart_coord_prg, "offsets"), 0);
        glvmUniform(xgl::GetUniformLocation(_cart_coord_prg, "normals"), 1);
        glvmUniform(xgl::GetUniformLocation(_cart_coord_prg, "elevation_data"), 2);
        glvmUniform(xgl::GetUniformLocation(_cart_coord_prg, "elevation_mask"), 3);
        _cart_coord_prg_step_loc = xgl::GetUniformLocation(_cart_coord_prg, "step");
        _cart_coord_prg_q_offset_loc = xgl::GetUniformLocation(_cart_coord_prg, "q_offset");
        _cart_coord_prg_q_factor_loc = xgl::GetUniformLocation(_cart_coord_prg, "q_factor");
        _cart_coord_prg_quad_tl_loc = xgl::GetUniformLocation(_cart_coord_prg, "quad_tl");
        _cart_coord_prg_quad_tr_loc = xgl::GetUniformLocation(_cart_coord_prg, "quad_tr");
        _cart_coord_prg_quad_bl_loc = xgl::GetUniformLocation(_cart_coord_prg, "quad_bl");
        _cart_coord_prg_quad_br_loc = xgl::GetUniformLocation(_cart_coord_prg, "quad_br");
        _cart_coord_prg_have_base_data_loc = xgl::GetUniformLocation(_cart_coord_prg, "have_base_data");
        _cart_coord_prg_fallback_normal_loc = xgl::GetUniformLocation(_cart_coord_prg, "fallback_normal");
        _cart_coord_prg_base_data_texcoord_offset_loc = xgl::GetUniformLocation(_cart_coord_prg, "base_data_texcoord_offset");
        _cart_coord_prg_base_data_texcoord_factor_loc = xgl::GetUniformLocation(_cart_coord_prg, "base_data_texcoord_factor");
        _cart_coord_prg_base_data_mirror_x_loc = xgl::GetUniformLocation(_cart_coord_prg, "base_data_mirror_x");
        _cart_coord_prg_base_data_mirror_y_loc = xgl::GetUniformLocation(_cart_coord_prg, "base_data_mirror_y");
        _cart_coord_prg_base_data_matrix_loc = xgl::GetUniformLocation(_cart_coord_prg, "base_data_matrix");
        _cart_coord_prg_skirt_elevation_loc = xgl::GetUniformLocation(_cart_coord_prg, "skirt_elevation");
        _cart_coord_prg_have_elevation_loc = xgl::GetUniformLocation(_cart_coord_prg, "have_elevation");
        _cart_coord_prg_fallback_elevation_loc = xgl::GetUniformLocation(_cart_coord_prg, "fallback_elevation");
        _cart_coord_prg_elevation_texcoord_factor_loc = xgl::GetUniformLocation(_cart_coord_prg, "elevation_texcoord_factor");
        _cart_coord_prg_elevation_texcoord_offset_loc = xgl::GetUniformLocation(_cart_coord_prg, "elevation_texcoord_offset");
        assert(xgl::CheckError(HERE));
    }
    glUseProgram(_cart_coord_prg);
    glvmUniform(_cart_coord_prg_step_loc, 1.0f / (quad_size + 6));
    glvmUniform(_cart_coord_prg_q_offset_loc, -3.0f / quad_size);
    glvmUniform(_cart_coord_prg_q_factor_loc, static_cast<float>(quad_size + 6) / quad_size);
    glvmUniform(_cart_coord_prg_base_data_texcoord_offset_loc, 2.0f / (quad_size + 4));
    glvmUniform(_cart_coord_prg_base_data_texcoord_factor_loc, static_cast<float>(quad_size) / (quad_size + 4));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbo);
    GLuint draw_buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, draw_buffers);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, _read_fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    xgl::PushViewport(_xgl_stack);
    xgl::PushModelViewMatrix(_xgl_stack);
    xgl::PushProjectionMatrix(_xgl_stack);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    assert(xgl::CheckError(HERE));
    if (_render_flags.size() < render_quads)
        _render_flags.resize(render_quads);
    if (_texture_data_texs.size() < render_quads)
        _texture_data_texs.resize(render_quads);
    if (_texture_mask_texs.size() < render_quads)
        _texture_mask_texs.resize(render_quads);
    if (_texture_data_texs_return_to_pool.size() < render_quads)
        _texture_data_texs_return_to_pool.resize(render_quads);
    if (_texture_mask_texs_return_to_pool.size() < render_quads)
        _texture_mask_texs_return_to_pool.resize(render_quads);
    if (_texture_metas.size() < render_quads)
        _texture_metas.resize(render_quads);
    if (_cart_coord_texs.size() < render_quads)
        _cart_coord_texs.resize(render_quads);
    info->quads_approximated[depth_pass] = 0;
    info->quads_rendered[depth_pass] = 0;
    info->lowest_quad_level[depth_pass] = ecmdb::max_levels;
    info->highest_quad_level[depth_pass] = 0;
    for (unsigned int quad_index = 0; quad_index < render_quads; quad_index++) {
        const ecm_side_quadtree* quad = lod_thread->render_quad(quad_index);
        _render_flags[quad_index] = true;
        vec3 quad_corner_rel_pos[4];
        GLuint offsets_tex;
        GLuint normals_tex;
        for (int i = 0; i < 4; i++) {
            quad_corner_rel_pos[i] = vec3(quad->corner(i) - state->viewer_pos);
        }
        /* Get quad base data and cache it on GPU. */
        ivec4 quad_base_data_sym_quad;
        ecm::symmetry_quad(quad->quad()[0], quad->quad()[1], quad->quad()[2], quad->quad()[3],
                &(quad_base_data_sym_quad[0]), &(quad_base_data_sym_quad[1]), &(quad_base_data_sym_quad[2]), &(quad_base_data_sym_quad[3]),
                NULL, NULL, NULL);
        quad_base_data_key qbdkey(quad_base_data_sym_quad);
        const quad_base_data_gpu *qbdgpu = quad_base_data_gpu_cache.locked_get(qbdkey);
        if (!qbdgpu) {
            const quad_base_data_mem *qbdmem = quad_base_data_mem_cache.locked_get(qbdkey);
            if (!qbdmem || qbdmem->max_dist_to_quad_plane <= 0.0) {
                offsets_tex = 0;
                normals_tex = 0;
            } else {
                offsets_tex = quad_tex_pool.get(GL_RGB32F, quad_size + 4);
                xgl::WriteTex2D(offsets_tex, 0, 0, quad_size + 4, quad_size + 4, GL_RGB, GL_FLOAT,
                        (quad_size + 4) * sizeof(vec3), qbdmem->offsets.ptr(), _pbo[0]);
                normals_tex = quad_tex_pool.get(GL_RG32F, quad_size + 4);
                xgl::WriteTex2D(normals_tex, 0, 0, quad_size + 4, quad_size + 4, GL_RG, GL_FLOAT,
                        (quad_size + 4) * sizeof(vec2), qbdmem->normals.ptr(), _pbo[1]);
                quad_base_data_gpu_cache.locked_put(qbdkey, new quad_base_data_gpu(
                            &quad_tex_pool, offsets_tex, normals_tex,
                            qbdmem->max_dist_to_quad_plane),
                        (quad_size + 4) * (quad_size + 4) * (sizeof(vec3) + sizeof(vec2)));
            }
        } else {
            offsets_tex = qbdgpu->offsets_tex;
            normals_tex = qbdgpu->normals_tex;
        }
        assert(xgl::CheckError(HERE));
        /* Process and combine elevation. Must be done before
         * processing texture because we need the elevation
         * information filled for the e2c processor. */
        glViewport(0, 0, quad_size + 4, quad_size + 4);
        process_and_combine(*context, frame, *processor,
                lod_thread->n_elevation_dds(), lod_thread->elevation_dds(),
                quad_size, quad, 0, offsets_tex,
                quad->lens_status(), lod_thread->lens_rel_pos(), state->lens.radius, quad_corner_rel_pos,
                0, 0, ecmdb::metadata(),
                _elevation_data_texs,
                _elevation_mask_texs,
                _elevation_data_texs_return_to_pool,
                _elevation_mask_texs_return_to_pool,
                _elevation_metas,
                &info->quads_approximated[depth_pass]);
        assert(xgl::CheckError(HERE));
        if (_elevation_data_texs[0] == 0
                && lod_thread->n_texture_dds() == 1
                && lod_thread->texture_dds()[0]->processing_parameters[0].category_e2c) {
            assert(!_elevation_data_texs_return_to_pool[0]);
            assert(!_elevation_mask_texs_return_to_pool[0]);
            msg::dbg("No valid elevation data and no texture except e2c: not rendering quad");
            _render_flags[quad_index] = false;
            continue;
        }
        /* Process and combine texture, including texture
         * from elevation (e2c) if applicable. */
        glViewport(0, 0, quad_size + 2, quad_size + 2);
        process_and_combine(*context, frame, *processor,
                lod_thread->n_texture_dds(), lod_thread->texture_dds(),
                quad_size, quad, quad_index, offsets_tex,
                quad->lens_status(), lod_thread->lens_rel_pos(), state->lens.radius, quad_corner_rel_pos,
                _elevation_data_texs[0], _elevation_mask_texs[0], _elevation_metas[0],
                _texture_data_texs,
                _texture_mask_texs,
                _texture_data_texs_return_to_pool,
                _texture_mask_texs_return_to_pool,
                _texture_metas,
                &info->quads_approximated[depth_pass]);
        assert(xgl::CheckError(HERE));
        if (_texture_data_texs[quad_index] == 0) {
            assert(!_texture_data_texs_return_to_pool[quad_index]);
            assert(!_texture_mask_texs_return_to_pool[quad_index]);
            if (_elevation_data_texs_return_to_pool[0])
                quad_tex_pool.put(_elevation_data_texs[0]);
            if (_elevation_mask_texs_return_to_pool[0])
                quad_tex_pool.put(_elevation_mask_texs[0]);
            msg::dbg("No valid texture data: not rendering quad");
            _render_flags[quad_index] = false;
            continue;
        }
        /* Compute the cartesian coordinates from base data + elevation. */
        glViewport(0, 0, quad_size + 6, quad_size + 6);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        _cart_coord_texs[quad_index] = quad_tex_pool.get(GL_RGB32F, quad_size + 6);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _cart_coord_texs[quad_index], 0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
        assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, offsets_tex == 0 ? _invalid_data_tex : offsets_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // has to be linear to get
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   // the skirt coordinates correct
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, normals_tex == 0 ? _invalid_data_tex : normals_tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);   // has to be linear to get
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);   // the skirt coordinates correct
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, _elevation_data_texs[0] == 0 ? _invalid_data_tex : _elevation_data_texs[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        GLint elevation_total_quad_size;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &elevation_total_quad_size);
        int elevation_overlap = max(0, (elevation_total_quad_size - quad_size) / 2);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, _elevation_mask_texs[0] == 0 ? _valid_mask_tex : _elevation_mask_texs[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        assert(xgl::CheckError(HERE));
        glUseProgram(_cart_coord_prg);
        glvmUniform(_cart_coord_prg_quad_tl_loc, vec3(quad->corner(ecm::corner_tl) - state->viewer_pos));
        glvmUniform(_cart_coord_prg_quad_tr_loc, vec3(quad->corner(ecm::corner_tr) - state->viewer_pos));
        glvmUniform(_cart_coord_prg_quad_bl_loc, vec3(quad->corner(ecm::corner_bl) - state->viewer_pos));
        glvmUniform(_cart_coord_prg_quad_br_loc, vec3(quad->corner(ecm::corner_br) - state->viewer_pos));
        glvmUniform(_cart_coord_prg_have_base_data_loc, (offsets_tex != 0));
        glvmUniform(_cart_coord_prg_fallback_normal_loc, vec3(quad->plane_normal()));
        bool base_data_mirror_x, base_data_mirror_y;
        mat3 base_data_matrix;
        int devnull;
        ecm::symmetry_quad(quad->quad()[0], quad->quad()[1], quad->quad()[2], quad->quad()[3],
                &devnull, &devnull, &devnull, &devnull,
                &base_data_mirror_x, &base_data_mirror_y, base_data_matrix.vl);
        glvmUniform(_cart_coord_prg_base_data_mirror_x_loc, base_data_mirror_x ? 1.0f : 0.0f);
        glvmUniform(_cart_coord_prg_base_data_mirror_y_loc, base_data_mirror_y ? 1.0f : 0.0f);
        glvmUniform(_cart_coord_prg_base_data_matrix_loc, base_data_matrix);
        //glvmUniform(_cart_coord_prg_skirt_elevation_loc, quad->min_elev() - static_cast<float>(quad->max_dist_to_quad_plane()));
        glvmUniform(_cart_coord_prg_skirt_elevation_loc,
                min(static_cast<float>(state->inner_bounding_sphere_radius - state->semi_major_axis()),
                    -static_cast<float>(quad->max_dist_to_quad_plane())));
        glvmUniform(_cart_coord_prg_have_elevation_loc, (_elevation_data_texs[0] != 0));
        glvmUniform(_cart_coord_prg_fallback_elevation_loc, quad->min_elev());
        glvmUniform(_cart_coord_prg_elevation_texcoord_offset_loc, static_cast<float>(elevation_overlap) / elevation_total_quad_size);
        glvmUniform(_cart_coord_prg_elevation_texcoord_factor_loc, static_cast<float>(quad_size) / elevation_total_quad_size);
        assert(xgl::CheckError(HERE));
        msg::dbg("Cartesian coordinates: base data %d, elevation %d",
                offsets_tex == 0 ? 0 : 1,
                _elevation_data_texs[0] == 0 ? 0 : 1);
        xgl::DrawQuad();
        assert(xgl::CheckError(HERE));
        glDrawBuffers(2, draw_buffers);
        glViewport(0, 0, quad_size + 4, quad_size + 4);
        if (state->debug_quad_depth_pass == depth_pass
                && state->debug_quad_index == static_cast<int>(quad_index)) {
            info->debug_quad = quad->quad();
            info->debug_quad_max_dist_to_quad_plane = quad->max_dist_to_quad_plane();
            info->debug_quad_min_elev = quad->min_elev();
            info->debug_quad_max_elev = quad->max_elev();
            if (state->debug_quad_save) {
                xgl::SaveTex2D("debug-quad-offsets.gta", offsets_tex ? offsets_tex : _invalid_data_tex);
                xgl::SaveTex2D("debug-quad-normals.gta", normals_tex ? normals_tex : _invalid_data_tex);
                xgl::SaveTex2D("debug-quad-elevation-data.gta", _elevation_data_texs[0] ? _elevation_data_texs[0] : _invalid_data_tex);
                xgl::SaveTex2D("debug-quad-elevation-mask.gta", _elevation_mask_texs[0] ? _elevation_mask_texs[0] : _invalid_data_tex);
                xgl::SaveTex2D("debug-quad-cartcoords.gta", _cart_coord_texs[quad_index]);
                xgl::SaveTex2D("debug-quad-texture-data.gta", _texture_data_texs[quad_index]);
                xgl::SaveTex2D("debug-quad-texture-mask.gta", _texture_mask_texs[quad_index] ?  _texture_mask_texs[quad_index] : _invalid_data_tex);
            }
        }
        if (_elevation_data_texs_return_to_pool[0])
            quad_tex_pool.put(_elevation_data_texs[0]);
        if (_elevation_mask_texs_return_to_pool[0])
            quad_tex_pool.put(_elevation_mask_texs[0]);
        info->quads_rendered[depth_pass]++;
        if ((offsets_tex == 0 || normals_tex == 0)
                && (quad->max_dist_to_quad_plane() > 0.0 || !quad->max_dist_to_quad_plane_is_valid())) {
            info->quads_approximated[depth_pass]++;
            ivec4 quad_base_data_sym_quad;
            ecm::symmetry_quad(quad->quad()[0], quad->quad()[1], quad->quad()[2], quad->quad()[3],
                    &(quad_base_data_sym_quad[0]), &(quad_base_data_sym_quad[1]), &(quad_base_data_sym_quad[2]), &(quad_base_data_sym_quad[3]),
                    NULL, NULL, NULL);
            quad_base_data_key qbdkey(quad_base_data_sym_quad);
            (void)quad_base_data_mem_cache_computers.locked_start_compute(qbdkey, lod_thread->ecm(), quad_size);
        }
        if (quad->level() < info->lowest_quad_level[depth_pass])
            info->lowest_quad_level[depth_pass] = quad->level();
        if (quad->level() > info->highest_quad_level[depth_pass])
            info->highest_quad_level[depth_pass] = quad->level();
    }
    if (info->quads_rendered[depth_pass] == 0) {
        info->lowest_quad_level[depth_pass] = -1;
        info->highest_quad_level[depth_pass] = -1;
    }

    xgl::PopProjectionMatrix(_xgl_stack);
    xgl::PopModelViewMatrix(_xgl_stack);
    xgl::PopViewport(_xgl_stack);
    glPopClientAttrib();
    glPopAttrib();
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, draw_framebuffer_bak);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, read_framebuffer_bak);
    glDrawBuffer(draw_buffer_bak);
    glReadBuffer(read_buffer_bak);
    assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
    assert(xgl::CheckFBO(GL_READ_FRAMEBUFFER, HERE));
    glEnable(GL_TEXTURE_2D);
    if (scissor_bak) {
        glEnable(GL_SCISSOR_TEST);
    }

    /* Re-create the quad VBO. */
    if (_quad_vbo_subdivision != state->renderer.quad_subdivision) {
        glBindBuffer(GL_ARRAY_BUFFER, _quad_vbo);
        std::vector<float> vbo_data;
        unsigned int quad_subquads = 1 << (2 * state->renderer.quad_subdivision);
        unsigned int skirt_subquads = 4 * (1 << state->renderer.quad_subdivision);
        unsigned int subquads = quad_subquads + skirt_subquads;
        vbo_data.resize(subquads * 4 * 2);
        float subquad_size = 1.0f / (1 << state->renderer.quad_subdivision);
#if 0
        // quad subquads using z-order curve
        // this turned out to be ca. 0.5% SLOWER than the naive order!
        for (unsigned int z = 0; z < quad_subquads; z++) {
            unsigned int x, y;
            inv_morton(z, &x, &y);
            float subquad_bl_x = static_cast<float>(x) / (1 << state->renderer.quad_subdivision);
            float subquad_bl_y = static_cast<float>(y) / (1 << state->renderer.quad_subdivision);
            set_subquad(vbo_data, z, subquad_bl_x, subquad_bl_y, subquad_size);
        }
#else
        for (int y = 0; y < (1 << state->renderer.quad_subdivision); y++) {
            for (int x = 0; x < (1 << state->renderer.quad_subdivision); x++) {
                float subquad_bl_x = static_cast<float>(x) / (1 << state->renderer.quad_subdivision);
                float subquad_bl_y = static_cast<float>(y) / (1 << state->renderer.quad_subdivision);
                unsigned int z = y * (1 << state->renderer.quad_subdivision) + x;
                set_subquad(vbo_data, z, subquad_bl_x, subquad_bl_y, subquad_size);
            }
        }
#endif
        // skirt subquads
        unsigned int skirt_subquad_index = 0;
        for (int y = 0; y < (1 << state->renderer.quad_subdivision); y++) {
            float subquad_bl_y = static_cast<float>(y) / (1 << state->renderer.quad_subdivision);
            set_subquad(vbo_data, quad_subquads + skirt_subquad_index, -subquad_size, subquad_bl_y, subquad_size);
            skirt_subquad_index++;
        }
        for (int y = 0; y < (1 << state->renderer.quad_subdivision); y++) {
            float subquad_bl_y = static_cast<float>(y) / (1 << state->renderer.quad_subdivision);
            set_subquad(vbo_data, quad_subquads + skirt_subquad_index, 1.0f, subquad_bl_y, subquad_size);
            skirt_subquad_index++;
        }
        for (int x = 0; x < (1 << state->renderer.quad_subdivision); x++) {
            float subquad_bl_x = static_cast<float>(x) / (1 << state->renderer.quad_subdivision);
            set_subquad(vbo_data, quad_subquads + skirt_subquad_index, subquad_bl_x, -subquad_size, subquad_size);
            skirt_subquad_index++;
        }
        for (int x = 0; x < (1 << state->renderer.quad_subdivision); x++) {
            float subquad_bl_x = static_cast<float>(x) / (1 << state->renderer.quad_subdivision);
            set_subquad(vbo_data, quad_subquads + skirt_subquad_index, subquad_bl_x, 1.0f, subquad_size);
            skirt_subquad_index++;
        }
        glBufferData(GL_ARRAY_BUFFER, vbo_data.size() * sizeof(float), &(vbo_data[0]), GL_STATIC_DRAW);
        _quad_vbo_subdivision = state->renderer.quad_subdivision;
        _quad_vbo_mode = GL_QUADS;
        _quad_vbo_vertices = 4 * subquads;
    }
    /* Re-create the render program. */
    if (_render_prg == 0
            || _render_prg_lighting != state->light.active
            || _render_prg_quad_borders != state->renderer.quad_borders) {
        _render_prg_lighting = state->light.active;
        _render_prg_quad_borders = state->renderer.quad_borders;
        xgl::DeleteProgram(_render_prg);
        std::string render_vs_src(RENDER_VS_GLSL_STR);
        std::string render_fs_src(RENDER_FS_GLSL_STR);
        render_vs_src = str::replace(render_vs_src, "$lighting", state->light.active ? "LIGHTING" : "NO_LIGHTING");
        render_fs_src = str::replace(render_fs_src, "$lighting", state->light.active ? "LIGHTING" : "NO_LIGHTING");
        render_fs_src = str::replace(render_fs_src, "$quad_borders", state->renderer.quad_borders ? "QUAD_BORDERS" : "NO_QUAD_BORDERS");
        _render_prg = xgl::CreateProgram("render", render_vs_src, "", render_fs_src);
        assert(xgl::CheckError(HERE));
        xgl::LinkProgram("render", _render_prg);
        assert(xgl::CheckError(HERE));
        glUseProgram(_render_prg);
        glvmUniform(xgl::GetUniformLocation(_render_prg, "cart_coords"), 0);
        glvmUniform(xgl::GetUniformLocation(_render_prg, "texture_data"), 1);
        glvmUniform(xgl::GetUniformLocation(_render_prg, "texture_mask"), 2);
        _render_prg_cart_coords_texcoord_factor_loc = xgl::GetUniformLocation(_render_prg, "cart_coords_texcoord_factor");
        _render_prg_cart_coords_texcoord_offset_loc = xgl::GetUniformLocation(_render_prg, "cart_coords_texcoord_offset");
        _render_prg_cart_coords_halfstep_loc = xgl::GetUniformLocation(_render_prg, "cart_coords_halfstep");
        _render_prg_texture_texcoord_factor_loc = xgl::GetUniformLocation(_render_prg, "texture_texcoord_factor");
        _render_prg_texture_texcoord_offset_loc = xgl::GetUniformLocation(_render_prg, "texture_texcoord_offset");
        if (state->light.active) {
            _render_prg_cart_coords_step_loc = xgl::GetUniformLocation(_render_prg, "cart_coords_step");
            _render_prg_L_loc = xgl::GetUniformLocation(_render_prg, "L");
            _render_prg_ambient_color_loc = xgl::GetUniformLocation(_render_prg, "ambient_color");
            _render_prg_light_color_loc = xgl::GetUniformLocation(_render_prg, "light_color");
            _render_prg_shininess_loc = xgl::GetUniformLocation(_render_prg, "shininess");
        }
        assert(xgl::CheckError(HERE));
    }

    /* Render the quads. Do not integrate this into the previous loop to avoid FBO switches. */
    if (state->renderer.wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glUseProgram(_render_prg);
    glvmUniform(_render_prg_cart_coords_texcoord_offset_loc, 3.0f / (quad_size + 6));
    glvmUniform(_render_prg_cart_coords_texcoord_factor_loc, static_cast<float>(quad_size) / (quad_size + 6));
    glvmUniform(_render_prg_cart_coords_halfstep_loc, 0.5f / (quad_size + 6));
    if (state->light.active) {
        glvmUniform(_render_prg_cart_coords_step_loc, 1.0f / (quad_size + 6));
        glvmUniform(_render_prg_L_loc, state->light.dir);
        glvmUniform(_render_prg_ambient_color_loc, vec4(state->light.ambient, 1.0f));
        glvmUniform(_render_prg_light_color_loc, vec4(state->light.color, 1.0f));
        glvmUniform(_render_prg_shininess_loc, 1.0f / state->light.shininess);
    }
    for (unsigned int quad_index = 0; quad_index < render_quads; quad_index++) {
        if (!_render_flags[quad_index])
            continue;
        const ecm_side_quadtree* quad = lod_thread->render_quad(quad_index);
        // Bind textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _cart_coord_texs[quad_index]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glActiveTexture(GL_TEXTURE1);
        assert(_texture_data_texs[quad_index] != 0);
        glBindTexture(GL_TEXTURE_2D, _texture_data_texs[quad_index]);
        GLint texture_total_quad_size;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texture_total_quad_size);
        int texture_overlap = max(0, (texture_total_quad_size - quad_size) / 2);
        if (state->renderer.mipmapping && _texture_data_texs[quad_index] != 0) {
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, _texture_mask_texs[quad_index] == 0 ? _valid_mask_tex : _texture_mask_texs[quad_index]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Set per-quad uniforms
        glvmUniform(_render_prg_texture_texcoord_offset_loc, static_cast<float>(texture_overlap) / texture_total_quad_size);
        glvmUniform(_render_prg_texture_texcoord_factor_loc, static_cast<float>(quad_size) / texture_total_quad_size);
        if (state->renderer.quad_borders) {
            glvmUniform(glGetUniformLocation(_render_prg, "quad_side"), quad->side());
            glvmUniform(glGetUniformLocation(_render_prg, "quad_level"), quad->level());
            glvmUniform(glGetUniformLocation(_render_prg, "quad_x"), quad->x());
            glvmUniform(glGetUniformLocation(_render_prg, "quad_y"), quad->y());
            glvmUniform(glGetUniformLocation(_render_prg, "quads_in_level"), 1 << quad->level());
            glvmUniform(glGetUniformLocation(_render_prg, "quad_border_thickness"), 4.0f / 256.0f);
            glvmUniform(glGetUniformLocation(_render_prg, "quad_border_color"), vec3(1.0f, 0.0f, 0.0f));
        }
        // Render
        glBindBuffer(GL_ARRAY_BUFFER, _quad_vbo);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, 0);
        glDrawArrays(_quad_vbo_mode, 0, _quad_vbo_vertices);
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        /* glBegin(GL_QUADS); glVertex2f(1.0f, 1.0f); glVertex2f(0.0f, 1.0f); glVertex2f(0.0f, 0.0f); glVertex2f(1.0f, 0.0f); glEnd(); */
        if (state->debug_quad_depth_pass == depth_pass
                && state->debug_quad_index == static_cast<int>(quad_index)) {
            draw_bounding_box(quad, state->viewer_pos, true);
        } else if (state->renderer.bounding_boxes) {
            draw_bounding_box(quad, state->viewer_pos);
        }
        // Give unused textures back
        quad_tex_pool.put(_cart_coord_texs[quad_index]);
        if (_texture_data_texs_return_to_pool[quad_index])
            quad_tex_pool.put(_texture_data_texs[quad_index]);
        if (_texture_mask_texs_return_to_pool[quad_index])
            quad_tex_pool.put(_texture_mask_texs[quad_index]);
        assert(xgl::CheckError(HERE));
    }

#if 0
    glActiveTexture(GL_TEXTURE0);       // XXX should not be necessary, but removing it gives wrong line colors!?
    glDisable(GL_TEXTURE_2D);
    glUseProgram(0);
    glLineWidth(4.0f);
    // Planet Circumscribing Cube
    {
        double u = sqrt(1.0) * lod_thread->ecm().semi_major_axis;
        glColor3f(0.0f, 0.0f, 0.0f);
        glBegin(GL_LINE_LOOP);  // front: x = +1
        glvmVertex(dvec3(+u, -u, +u) - state->viewer_pos);
        glvmVertex(dvec3(+u, +u, +u) - state->viewer_pos);
        glvmVertex(dvec3(+u, +u, -u) - state->viewer_pos);
        glvmVertex(dvec3(+u, -u, -u) - state->viewer_pos);
        glEnd();
        glBegin(GL_LINE_LOOP);  // back: x = -1
        glvmVertex(dvec3(-u, -u, +u) - state->viewer_pos);
        glvmVertex(dvec3(-u, +u, +u) - state->viewer_pos);
        glvmVertex(dvec3(-u, +u, -u) - state->viewer_pos);
        glvmVertex(dvec3(-u, -u, -u) - state->viewer_pos);
        glEnd();
        glBegin(GL_LINE_LOOP);  // left: y = -1
        glvmVertex(dvec3(-u, -u, +u) - state->viewer_pos);
        glvmVertex(dvec3(+u, -u, +u) - state->viewer_pos);
        glvmVertex(dvec3(+u, -u, -u) - state->viewer_pos);
        glvmVertex(dvec3(-u, -u, -u) - state->viewer_pos);
        glEnd();
        glBegin(GL_LINE_LOOP);  // right: y = +1
        glvmVertex(dvec3(-u, +u, +u) - state->viewer_pos);
        glvmVertex(dvec3(+u, +u, +u) - state->viewer_pos);
        glvmVertex(dvec3(+u, +u, -u) - state->viewer_pos);
        glvmVertex(dvec3(-u, +u, -u) - state->viewer_pos);
        glEnd();
        glBegin(GL_LINE_LOOP);  // top: z = +1
        glvmVertex(dvec3(+u, -u, +u) - state->viewer_pos);
        glvmVertex(dvec3(+u, +u, +u) - state->viewer_pos);
        glvmVertex(dvec3(-u, +u, +u) - state->viewer_pos);
        glvmVertex(dvec3(-u, -u, +u) - state->viewer_pos);
        glEnd();
        glBegin(GL_LINE_LOOP);  // top: z = -11
        glvmVertex(dvec3(+u, -u, -u) - state->viewer_pos);
        glvmVertex(dvec3(+u, +u, -u) - state->viewer_pos);
        glvmVertex(dvec3(-u, +u, -u) - state->viewer_pos);
        glvmVertex(dvec3(-u, -u, -u) - state->viewer_pos);
        glEnd();
    }
    // Planet Inner Bounding Cube
    {
        vec3 bc_colors[6] = {
            vec3(1.0, 1.0, 1.0),
            vec3(1.0, 1.0, 0.0),
            vec3(1.0, 0.0, 1.0),
            vec3(1.0, 0.0, 0.0),
            vec3(0.0, 1.0, 1.0),
            vec3(0.0, 1.0, 0.0)
        };
        for (int s = 0; s < 6; s++) {
            dvec3 tl = ecm.ecm_to_cartesian(ecm.quad_to_ecm(ivec4(s, 0, 0, 0), ecm::corner_tl));
            dvec3 tr = ecm.ecm_to_cartesian(ecm.quad_to_ecm(ivec4(s, 0, 0, 0), ecm::corner_tr));
            dvec3 bl = ecm.ecm_to_cartesian(ecm.quad_to_ecm(ivec4(s, 0, 0, 0), ecm::corner_bl));
            dvec3 br = ecm.ecm_to_cartesian(ecm.quad_to_ecm(ivec4(s, 0, 0, 0), ecm::corner_br));
            glvmColor(bc_colors[s]);
            glBegin(GL_LINE_LOOP);
            glvmVertex(tr - state->viewer_pos);
            glvmVertex(tl - state->viewer_pos);
            glvmVertex(bl - state->viewer_pos);
            glvmVertex(br - state->viewer_pos);
            glEnd();
        }
    }
    // Axes
    {
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        glvmVertex(-viewer_pos);
        glvmVertex(dvec3(2.0 * quad->ecm().semi_major_axis, 0.0, 0.0) - viewer_pos);
        glEnd();
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        glvmVertex(-viewer_pos);
        glvmVertex(dvec3(0.0, 2.0 * quad->ecm().semi_major_axis, 0.0) - viewer_pos);
        glEnd();
        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_LINES);
        glvmVertex(-viewer_pos);
        glvmVertex(dvec3(0.0, 0.0, 2.0 * quad->ecm().semi_major_axis) - viewer_pos);
        glEnd();
    }
#endif

    /* Cleanup */
    xgl::PopEverything(_xgl_stack);
    xgl::CheckError(HERE);

    /* Compute the cartesian coordinates at the pointer position
     * (if it is inside the view frustum and we rendered something there
     * in this depth pass) */
    ivec2 pointer_pos = state->pointer_pos;
    if (pointer_pos.x >= lod_thread->VP()[0] && pointer_pos.x < lod_thread->VP()[0] + lod_thread->VP()[2]
            && pointer_pos.y >= lod_thread->VP()[1] && pointer_pos.y < lod_thread->VP()[1] + lod_thread->VP()[3]
            && all(equal(info->pointer_coord, dvec3(0.0)))) {
        float wz;
        glReadPixels(pointer_pos.x, pointer_pos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &wz);
        if (wz < 1.0f) {
            gluUnProject(pointer_pos.x, pointer_pos.y, wz,
                    lod_thread->rel_MV().vl, lod_thread->P().vl, lod_thread->VP().vl,
                    &info->pointer_coord.x, &info->pointer_coord.y, &info->pointer_coord.z);
            info->pointer_coord += state->viewer_pos;
        }
    }
}

terrain::terrain()
{
    _info[0] = new renderpass_info();
    _info[1] = new renderpass_info();
}

terrain::~terrain()
{
    delete _info[0];
    delete _info[1];
}

void terrain::init_gl()
{
    _processor.init_gl();
    _depth_pass_renderer.init_gl();
    _depth_passes[0] = 0;
    _depth_passes[1] = 0;
    _current_lod = 0;
}

void terrain::exit_gl()
{
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < _depth_passes[i]; j++) {
            _lod_threads[i][j]->finish();
            delete _lod_threads[i][j];
        }
        _lod_threads[i].clear();
    }
    _processor.exit_gl();
    _depth_pass_renderer.exit_gl();
}

void terrain::render(renderer_context& context, unsigned int frame,
        const ivec4& VP,
        const dmat4& MV,
        unsigned int depth_passes,
        const dmat4* P,
        renderpass_info* info)
{
    int old_lod = _current_lod;
    _current_lod = (old_lod == 0 ? 1 : 0);

    // Check if we have enough LOD thread objects
    {
        size_t lod_threads_size = _lod_threads[_current_lod].size();
        if (lod_threads_size < depth_passes) {
            _lod_threads[_current_lod].resize(depth_passes, NULL);
            for (size_t i = lod_threads_size; i < depth_passes; i++) {
                _lod_threads[_current_lod][i] = new lod_thread();
            }
        }
    }

    // Initialize current LOD
    _depth_passes[_current_lod] = depth_passes;
    _state[_current_lod] = context.state();
    *(_info[_current_lod]) = *info;

    // Do it
    for (int i = 0; i < _depth_passes[_current_lod]; i++) {
        _info[_current_lod]->clear_depth_pass(i);
        _lod_threads[_current_lod][i]->init(&context, frame,
                &(_state[_current_lod]), &_processor,
                VP, MV, i, P[i], _info[_current_lod]);
        _lod_threads[_current_lod][i]->start();
    }
    int render_lod = _state[_current_lod].renderer.force_lod_sync ? _current_lod : old_lod;
    for (int i = 0; i < _depth_passes[render_lod]; i++) {
        assert(_depth_passes[render_lod] <= 4);
        if (i >= 1) {
            glClear(GL_DEPTH_BUFFER_BIT);
        }
        _lod_threads[render_lod][i]->finish();
        _depth_pass_renderer.render(&context, frame,
                &(_state[render_lod]), &_processor,
                i, _lod_threads[render_lod][i],
                _info[render_lod]);
    }
    *info = *(_info[render_lod]);
}

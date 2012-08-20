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

#include "config.h"

#include <GL/glew.h>

#include "glvm.h"
#include "glvm-gl.h"
#include "glvm-str.h"

#include "msg.h"
#include "sys.h"

#include "xgl.h"

#include "renderer.h"

using namespace glvm;


const int renderer::_min_depth_bits;
const int renderer::_max_depth_passes;

renderer::renderer(renderer_context& renderer_context) :
    _renderer_context(renderer_context),
    _info(new renderpass_info())
{
    // Defer all OpenGL initialization to a later point.
}

renderer::~renderer()
{
    delete _info;
}

void renderer::init_gl()
{
    xgl::CheckError(HERE);
    _cache_quad_size = -1;
    _last_frame_quads = 0;
    _terrain.init_gl();
}

void renderer::exit_gl()
{
    xgl::CheckError(HERE);
    _terrain.exit_gl();
}

void renderer::compute_depth_passes(const dfrust& frustum, int depth_bits)
{
    const state& state = _renderer_context.state();

    /*
     * Dynamically compute the near and far planes for the viewing frustum.
     *
     * We assume an outer bounding sphere Bo that encompasses the whole
     * ellipsoid plus the highest mountains, and an inner bounding sphere Bi
     * that lies inside the ellipsoid.
     *
     * If the view position is outside Bo, then the near plane is set to the
     * distance of the viewer to Bo, multiplied with the cosinus of the maximum
     * angle between the viewing direction and the frustum's edges. If the view
     * position is inside Bo, the near plane is set to 0.1.
     *
     * The far plane is set to the distance between the viewer and a point on Bo
     * that lies on a line through the viewer position and a contact point on Bi.
     *
     * If the ratio of far plane and near plane is too high (so that too much
     * precision is lost), then multiple render passes with different near/far
     * planes are required. This function computes the number of necessary
     * passes and the near/far planes for each pass.
     */

    // Radii of outer and inner bounding spheres.
    const double ro = state.outer_bounding_sphere_radius;
    const double ri = state.inner_bounding_sphere_radius;

    const double dist_to_center = length(state.viewer_pos + dvec3(state.tracker_pos));
    const double dist_to_bo = dist_to_center - ro;      // Distance to outer bounding sphere

    const double cos_angle_l = - frustum.l() / sqrt(frustum.l() * frustum.l() + frustum.n() * frustum.n());
    const double cos_angle_r = + frustum.r() / sqrt(frustum.r() * frustum.r() + frustum.n() * frustum.n());
    const double cos_angle_b = - frustum.b() / sqrt(frustum.b() * frustum.b() + frustum.n() * frustum.n());
    const double cos_angle_t = + frustum.t() / sqrt(frustum.t() * frustum.t() + frustum.n() * frustum.n());
    const double cos_angle = std::min(std::min(std::min(cos_angle_l, cos_angle_r), cos_angle_b), cos_angle_t);
    double Near = std::max(dist_to_bo * cos_angle, 1.0);
    // The distance of the viewer point V to the point F on the outer sphere
    // is the distance of V to the contact point C on Bi plus the distance
    // of C and F.
#ifdef NDEBUG
# define SHOW_WHOLE_OUTER_BOUNDING_CUBE 0
#else
# define SHOW_WHOLE_OUTER_BOUNDING_CUBE 1
#endif
#if SHOW_WHOLE_OUTER_BOUNDING_CUBE
    double Far = dist_to_center + 1.5 * ro;
#else
    const double dist_vc = std::sqrt(std::max(dist_to_center * dist_to_center - ri * ri, 0.0));
    const double dist_cf = std::sqrt(ro * ro - ri * ri);
    double Far = dist_vc + dist_cf;
#endif

    //msg::dbg("NEAR: %f, FAR: %f, RATIO: %f, LOST BITS: %f", Near, Far, Far / Near, log2(Far / Near));

    /* Optimization for a special case:
     * If all edges of the frustum intersect Bi, set the far plane to the
     * maximum distance between the viewer and these intersection points.
     * This is a common case when data sets are examined in detail. */
    bool bi_fills_view = true;
    double max_dist_to_bi = -1.0;
    const dvec3 d[4] = {
        normalize(state.viewer_rot * dvec3(frustum.l(), frustum.t(), -frustum.n())),
        normalize(state.viewer_rot * dvec3(frustum.r(), frustum.t(), -frustum.n())),
        normalize(state.viewer_rot * dvec3(frustum.r(), frustum.b(), -frustum.n())),
        normalize(state.viewer_rot * dvec3(frustum.l(), frustum.b(), -frustum.n()))
    };
    const double hh = dot(state.viewer_pos + dvec3(state.tracker_pos), state.viewer_pos + dvec3(state.tracker_pos));
    const double rr = ri * ri;
    for (int i = 0; i < 4; i++) {
        const double dd = dot(d[i], d[i]);
        const double dh = dot(d[i], state.viewer_pos + dvec3(state.tracker_pos));
        const double p_2 = dh / dd;
        const double q = (hh - rr) / dd;
        const double radicand = p_2 * p_2 - q;
        if (radicand < 0.0) {
            // no intersection
            bi_fills_view = false;
            break;
        }
        const double squareroot = std::sqrt(radicand);
        const double t1 = -p_2 + squareroot;
        const double t2 = -p_2 - squareroot;
        // Intersections in view direction must have t > 0.
        // Since the direction is normalized, the distance to Bi is either t1 or t2.
        double dist;
        if (t1 <= 0.0 && t2 <= 0.0) {
            // no intersection in view direction
            bi_fills_view = false;
            break;
        } else if (t1 <= 0.0) {
            dist = t2;
        } else if (t2 <= 0.0) {
            dist = t1;
        } else {
            dist = std::min(t1, t2);
        }
        if (dist > max_dist_to_bi)
            max_dist_to_bi = dist;
    }
    if (bi_fills_view) {
        //msg::dbg("Reducing far plane from %f to %f", Far, min(Far, max_dist_to_bi));
        Far = min(Far, max_dist_to_bi);
    }

    int total_lost_bits = static_cast<int>(std::ceil(std::log2(Far / Near)));
    int depth_passes = std::max(1, std::min(_max_depth_passes, static_cast<int>(std::ceil(
                        static_cast<double>(total_lost_bits) / static_cast<double>(depth_bits - _min_depth_bits)))));
    double nearfar_factor = std::pow(2.0, static_cast<double>(total_lost_bits) / static_cast<double>(depth_passes));

    /* Write the computed information into _info */
    _info->depth_passes = depth_passes;
    for (int dp = _info->depth_passes - 1; dp >= 0; dp--) {
        _info->frustum[dp] = frustum;
        _info->frustum[dp].adjust_near(Near);
        _info->frustum[dp].f() = nearfar_factor * Near;
        Near *= nearfar_factor;
    }
    _info->frustum[0].f() = Far;

    /*
    for (int dp = 0; dp < depth_passes; dp++) {
        msg::dbg("depth pass %d/%d: near = %f, far = %f, lost bits = %f",
                dp + 1, depth_passes, _info.frustum[dp].n(), _info.frustum[dp].f(), log2(_info.frustum[dp].f() / _info.frustum[dp].n()));
    }
    */
}

void renderer::render(const ivec4& viewport, const dfrust& frustum, const dmat4& viewer_transform)
{
    assert(xgl::CheckError(HERE));

    /* Cache maintenance */
    bool clear_caches = false;
    if (_renderer_context.state().have_databases()) {
        const class ecm ecm(_renderer_context.state().semi_major_axis(),
                _renderer_context.state().semi_minor_axis());
        const int quad_size = _renderer_context.state().quad_size();
        if (_cache_quad_size != quad_size || _cache_ecm != ecm) {
            clear_caches = true;
            _cache_quad_size = quad_size;
            _cache_ecm = ecm;
        }
    }
    if (_renderer_context.start_per_node_maintenance()) {
        _renderer_context.quad_disk_cache_checkers()->get_results();
        _renderer_context.quad_disk_cache_fetchers()->get_results();
        _renderer_context.quad_mem_cache_loaders()->get_results();
        _renderer_context.quad_base_data_mem_cache_computers()->get_results();
        if (clear_caches) {
            msg::dbg("Clearing node caches");
            _renderer_context.quad_disk_cache()->clear();
            _renderer_context.quad_mem_cache()->clear();
            _renderer_context.quad_metadata_cache()->clear();
            _renderer_context.quad_base_data_mem_cache()->clear();
        }
        _renderer_context.quad_disk_cache()->set_max_size(10000);
        _renderer_context.quad_disk_cache()->shrink();
        _renderer_context.quad_mem_cache()->set_max_size(state().renderer.mem_cache_size / 4 * 3);
        _renderer_context.quad_mem_cache()->shrink();
        _renderer_context.quad_metadata_cache()->set_max_size(10000);
        _renderer_context.quad_metadata_cache()->shrink();
        _renderer_context.quad_base_data_mem_cache()->set_max_size(state().renderer.mem_cache_size / 4 * 1);
        _renderer_context.quad_base_data_mem_cache()->shrink();
        _renderer_context.finish_per_node_maintenance();
    }
    if (_renderer_context.start_per_glcontext_maintenance()) {
        if (clear_caches) {
            msg::dbg("Clearing glcontext caches");
            _renderer_context.quad_gpu_cache()->clear();
            _renderer_context.quad_base_data_gpu_cache()->clear();
        }
        _renderer_context.quad_gpu_cache()->set_max_size(state().renderer.gpu_cache_size / 4 * 3);
        _renderer_context.quad_gpu_cache()->shrink();
        _renderer_context.quad_base_data_gpu_cache()->set_max_size(state().renderer.gpu_cache_size / 4 * 1);
        _renderer_context.quad_base_data_gpu_cache()->shrink();
        _renderer_context.finish_per_glcontext_maintenance();
    }
    if (clear_caches) {
        msg::dbg("Clearing tex pool");
        _renderer_context.quad_tex_pool()->set_quad_size(_cache_quad_size);
    }
    _renderer_context.quad_tex_pool()->shrink(_last_frame_quads + _last_frame_quads / 4);   // must be called after GPU-based caches shrunk
    
    /* Internal frame number counter. Can wrap around. */
    static unsigned int frame = 0;

    /* Get the state */
    const class state& state = _renderer_context.state();

    /* Push */
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);

    /* Set viewport */
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    /* Determine depth passes */
    int depth_bits;
    glGetIntegerv(GL_DEPTH_BITS, &depth_bits);
    _info->clear();
    compute_depth_passes(frustum, depth_bits);

    assert(xgl::CheckError(HERE));

    /* Clear */
    {
        const class state& state = _renderer_context.state();
        float clear_r = state.renderer.background_color[0] / 255.0;
        float clear_g = state.renderer.background_color[1] / 255.0;
        float clear_b = state.renderer.background_color[2] / 255.0;
        glClearColor(clear_r, clear_g, clear_b, 0.0f);
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Render the depth passes */
    msg::dbg("Renderer: frame %u", frame);
    // Viewer position
    dvec3 viewer_pos = state.viewer_pos + translation(viewer_transform);
    msg::dbg("Renderer: viewer_pos %s", str::from(state.viewer_pos).c_str());
    // Modelview matrix
    dmat4 MV;
    MV = set(dmat4(1.0), strike(viewer_transform, 3, 3));
    MV = MV * toMat4(-state.viewer_rot);
    dmat4 T(1.0);
    translation(T) = -viewer_pos;
    MV = MV * T;
    msg::dbg("Renderer: MV %s", str::from(MV).c_str());
    // Projection matrix
    dmat4 P[_info->depth_passes];
    for (int dp = 0; dp < _info->depth_passes; dp++) {
        P[dp] = toMat4(_info->frustum[dp]);
        msg::dbg("Renderer: depth pass %d", dp);
        msg::dbg("Renderer: near %s far %s",
                str::from(_info->frustum[dp].n()).c_str(),
                str::from(_info->frustum[dp].f()).c_str());
        msg::dbg("Renderer: P %s", str::from(P[dp]).c_str());
    }
    // Render
    _terrain.render(_renderer_context, frame, viewport, MV, _info->depth_passes, P, _info);
    assert(xgl::CheckError(HERE));
    _last_frame_quads = 0;
    for (int dp = 0; dp < _info->depth_passes; dp++) {
        _last_frame_quads += _info->quads_rendered[dp];
    }
    frame++;

    glPopClientAttrib();
    glPopAttrib();
    assert(xgl::CheckError(HERE));
}

void renderer::render(const dfrust& frustum, int width, int height, void* buffer)
{
    const int block_width = 1024;
    const int block_height = 1024;
    const int blocks_x = width / block_width + (width % block_width == 0 ? 0 : 1);
    const int last_block_width = (width % block_width == 0 ? block_width : width % block_width);
    const int blocks_y = height / block_height + (height % block_height == 0 ? 0 : 1);
    const int last_block_height = (height % block_height == 0 ? block_height : height % block_height);

    xgl::CheckFBO(GL_FRAMEBUFFER, HERE);
    xgl::PushEverything(_xgl_stack);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    assert(xgl::CheckError(HERE));

    GLuint rb;
    glGenRenderbuffers(1, &rb);

    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    assert(xgl::CheckError(HERE));

    exc exception;
    for (int y = 0; exception.empty() && y < blocks_y; y++) {
        const int by = y * block_height;
        const int bh = (y == blocks_y - 1 ? last_block_height : block_height);
        for (int x = 0; exception.empty() && x < blocks_x; x++) {
            const int bx = x * block_width;
            const int bw = (x == blocks_x - 1 ? last_block_width : block_width);

            glBindTexture(GL_TEXTURE_2D, tex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, bw, bh, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            glBindRenderbuffer(GL_RENDERBUFFER, rb);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, bw, bh);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rb);
            assert(xgl::CheckFBO(GL_FRAMEBUFFER, HERE));
            assert(xgl::CheckError(HERE));

            dfrust bf;
            bf.l() = frustum.l() + static_cast<double>(bx) / static_cast<double>(width) * (frustum.r() - frustum.l());
            bf.r() = bf.l() + static_cast<double>(bw) / static_cast<double>(width) * (frustum.r() - frustum.l());
            bf.b() = frustum.b() + static_cast<double>(by) / static_cast<double>(height) * (frustum.t() - frustum.b());
            bf.t() = bf.b() + static_cast<double>(bh) / static_cast<double>(height) * (frustum.t() - frustum.b());
            bf.n() = frustum.n();
            bf.f() = frustum.f();

            int approximated_quads = 0;
            do {
                try {
                    render(ivec4(0, 0, bw, bh), bf, dmat4(1.0));
                    approximated_quads = 0;
                    const renderpass_info& info = get_info();
                    for (int i = 0; i < info.depth_passes; i++)
                        approximated_quads += info.quads_approximated[i];
                    if (approximated_quads > 0) {
                        msg::dbg("rendering block %d,%d to buffer: waiting for %d quads", x, y, approximated_quads);
                        sys::msleep(100);
                    }
                }
                catch (exc& e) {
                    exception = e;
                }
            }
            while (exception.empty() && approximated_quads > 0);

            blob block_array;
            if (exception.empty()) {
                try {
                    block_array.resize(bw, bh, 4);
                }
                catch (exc& e) {
                    exception = e;
                }
            }
            if (exception.empty()) {
                glBindTexture(GL_TEXTURE_2D, tex);
                glGetTexImage(GL_TEXTURE_2D, 0, GL_BGRA, GL_UNSIGNED_BYTE, block_array.ptr());
                assert(xgl::CheckError(HERE));
                for (int r = 0; r < bh; r++) {
                    for (int c = 0; c < bw; c++) {
                        for (int i = 0; i < 4; i++) {
                            // Avoid integer overflow for the array index
                            size_t array_index = static_cast<size_t>((r + by) * width);
                            array_index += static_cast<size_t>(c + bx);
                            array_index *= 4;
                            array_index += i;
                            static_cast<unsigned char*>(buffer)[array_index] = *block_array.ptr<unsigned char>(4 * (r * bw + c) + i);
                        }
                    }
                }
            }
        }
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rb);
    glDeleteTextures(1, &tex);

    xgl::PopEverything(_xgl_stack);
    assert(xgl::CheckError(HERE));

    if (!exception.empty()) {
        throw exception;
    }
}

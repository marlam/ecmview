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

#include <GL/glew.h>

#include "dbg.h"
#include "msg.h"

#include "xgl.h"

#include "quad-tex-pool.h"


const GLint quad_tex_pool::_formats[quad_tex_pool::_num_formats] = {
    GL_R8, GL_SLUMINANCE, GL_SRGB, GL_R32F, GL_RG32F, GL_RGB32F
};

quad_tex_pool::quad_tex_pool() : _quad_size(-1)
{
}

quad_tex_pool::~quad_tex_pool()
{
    for (int i = 0; i < _num_formats * _num_sizes; i++) {
        assert(_texpool[i].empty());
    }
}

void quad_tex_pool::init_gl()
{
}

void quad_tex_pool::exit_gl()
{
    for (int i = 0; i < _num_formats * _num_sizes; i++) {
        GLint fmt;
        int size;
        get_texpool_formatsize(i, &fmt, &size);
        if (_texpool[i].size() > 0) {
            msg::dbg("quad_tex_pool exit: deleting %d textures of format %s, size %d",
                    static_cast<int>(_texpool[i].size()),
                    (fmt == GL_R8 ? "R8"
                     : fmt == GL_SLUMINANCE ? "SLUMINANCE"
                     : fmt == GL_SRGB ? "SRGB"
                     : fmt == GL_R32F ? "R32F"
                     : fmt == GL_RG32F ? "RG32F"
                     : "RGB32F"),
                    size);
            glDeleteTextures(_texpool[i].size(), &(_texpool[i][0]));
            _texpool[i].clear();
        }
    }
}

void quad_tex_pool::set_quad_size(int quad_size)
{
    if (_quad_size != quad_size) {
        exit_gl();
        _quad_size = quad_size;
    }
}

int quad_tex_pool::get_texpool_index(GLint format, int size)
{
    assert(format == _formats[0]
            || format == _formats[1]
            || format == _formats[2]
            || format == _formats[3]
            || format == _formats[4]
            || format == _formats[5]);
    int format_index = 0;
    while (_formats[format_index] != format)
        format_index++;

    assert(size >= _quad_size + 2 * 0);
    assert(size <= _quad_size + 2 * ecmdb::max_overlap);
    assert((size - _quad_size) % 2 == 0);
    int size_index = (size - _quad_size) / 2;

    assert(format_index < _num_formats);
    assert(size_index < _num_sizes);
    return format_index * _num_sizes + size_index;
}

void quad_tex_pool::get_texpool_formatsize(int index, GLint* format, int* size)
{
    assert(index >= 0);
    assert(index < _num_formats * _num_sizes);
    int format_index = index / _num_sizes;
    int size_index = index % _num_sizes;
    assert(format_index < _num_formats);
    assert(size_index < _num_sizes);
    *format = _formats[format_index];
    *size = _quad_size + 2 * size_index;
}

GLuint quad_tex_pool::get(GLint format, int size)
{
    int index = get_texpool_index(format, size);
    if (_texpool[index].empty()) {
        msg::dbg("quad_tex_pool: creating new texture: format %s, size %d",
                (format == GL_R8 ? "R8"
                 : format == GL_SLUMINANCE ? "SLUMINANCE"
                 : format == GL_SRGB ? "SRGB"
                 : format == GL_R32F ? "R32F"
                 : format == GL_RG32F ? "RG32F"
                 : "RGB32F"), size);
        return xgl::CreateTex2D(format, size, size, GL_NEAREST);
    } else {
        GLuint t = _texpool[index].back();
        _texpool[index].pop_back();
        return t;
    }
}

void quad_tex_pool::put(GLuint tex)
{
    if (tex != 0) {
        GLint tex_bak;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_bak);
        GLint format;
        GLint size;
        glBindTexture(GL_TEXTURE_2D, tex);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &format);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &size);
        glBindTexture(GL_TEXTURE_2D, tex_bak);
        int index = get_texpool_index(format, size);
        _texpool[index].push_back(tex);
    }
}

void quad_tex_pool::shrink(int keep)
{
    assert(keep >= 0);
    for (int i = 0; i < _num_formats * _num_sizes; i++) {
        GLint fmt;
        int size;
        get_texpool_formatsize(i, &fmt, &size);
        size_t k = _keep_min;
        if ((size == _quad_size + 2 && fmt == GL_SRGB)
                || (size == _quad_size + 2 && fmt == GL_R8)
                || (size == _quad_size + 4 && (fmt == GL_RG32F || fmt == GL_RGB32F))
                || (size == _quad_size + 6 && fmt == GL_RGB32F)) {
            k += keep;
        }
        if (_texpool[i].size() > k) {
            msg::dbg("quad_tex_pool shrink: deleting %d textures of format %s, size %d",
                    static_cast<int>(_texpool[i].size() - k),
                    (fmt == GL_R8 ? "R8"
                     : fmt == GL_SLUMINANCE ? "SLUMINANCE"
                     : fmt == GL_SRGB ? "SRGB"
                     : fmt == GL_R32F ? "R32F"
                     : fmt == GL_RG32F ? "RG32F"
                     : "RGB32F"),
                    size);
            glDeleteTextures(_texpool[i].size() - k, &(_texpool[i][k]));
            _texpool[i].resize(k);
        }
    }
}

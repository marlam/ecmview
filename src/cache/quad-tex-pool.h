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

#ifndef QUAD_TEX_POOL_H
#define QUAD_TEX_POOL_H

#include <vector>

#include <GL/glew.h>

#include <ecmdb/ecmdb.h>


class quad_tex_pool
{
    /* Supported internal texture formats:
     * GL_R8, GL_SLUMINANCE, GL_SRGB, GL_R32F, GL_RG32F, GL_RGB32F  */
    /* Supported sizes:
     * (quad_size + 2 * overlap)^2, with overlap between 0 and db::max_overlap */

private:
    static const int _num_formats = 6;
    static const GLint _formats[_num_formats];
    static const int _num_sizes = ecmdb::max_overlap;
    static const size_t _keep_min = 8;        // keep at least this many textures of each category

    int _quad_size;
    std::vector<GLuint> _texpool[_num_formats * _num_sizes];

    int get_texpool_index(GLint format, int size);
    void get_texpool_formatsize(int index, GLint* format, int* size);

public:
    quad_tex_pool();
    ~quad_tex_pool();

    void init_gl();
    void exit_gl();

    void set_quad_size(int quad_size);
    GLuint get(GLint format, int size);
    void put(GLuint tex);

    // Shrink the tex pool; free unused textures.
    // But keep enough textures that are needed for intermediate processing
    // steps for rendering the given number of quads.
    void shrink(int keep);
};

#endif

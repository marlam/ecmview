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

#ifndef SUB_PROCESSOR_H
#define SUB_PROCESSOR_H


#include <GL/glew.h>

#include "glvm.h"

#include "database_description.h"


class SubProcessor
{
private:
    const int _method;
    int _highest_level;
    int _level;
    int _tile_size;

public:
    SubProcessor(int method) :
        _method(method), _highest_level(0), _level(0), _tile_size(0)
    {
    }

    virtual ~SubProcessor()
    {
    }

    int method() const
    {
        return _method;
    }

    void set(int tile_size, int highest_level, int level)
    {
        _tile_size = tile_size;
        _highest_level = highest_level;
        _level = level;
    }

    int tile_size() const
    {
        return _tile_size;
    }

    float step() const
    {
        return 1.0f / _tile_size;
    }

    int level_difference() const
    {
        return _highest_level - _level;
    }

    float adapted_step() const
    {
        int ld = level_difference();
        return (ld >= 0 ? step() / (1 << ld) : step() * (1 << -ld));
    }

    virtual void init_gl() = 0;
    virtual void exit_gl() = 0;
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]) = 0;

    // The following are compatibility functions that helped porting
    // the old code base to the ecmview project.
    // A future cleanup would probably want to remove this legacy stuff.
    std::string xglShaderSourcePrep(const std::string& src, const std::string& defines);
    void render_one_to_one(GLuint otex, GLuint itex);
    void gauss_mask(int k, float s, float* mask, float* weight_sum);
};

#endif

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

#ifndef QUAD_BASE_DATA_CACHE_H
#define QUAD_BASE_DATA_CACHE_H

#include <cstring>
#include <limits>
#include <memory>
#include <set>

#include <ecmdb/ecm.h>

#include "blob.h"

#include "glvm.h"

#include "lru-cache.h"
#include "quad-tex-pool.h"


/* A cache key for quad base data. */

class quad_base_data_key
{
public:
    glvm::ivec4 quad;

    quad_base_data_key(const glvm::ivec4& quad) : quad(quad)
    {
    }

    bool operator<(const quad_base_data_key& qk) const
    {
        return std::memcmp(&(this->quad), &(qk.quad), sizeof(this->quad)) < 0;
    }
};

/* GPU cache: offset and normal information */

class quad_base_data_gpu
{
private:
    quad_tex_pool* _quad_tex_pool;

public:
    const GLuint offsets_tex;
    const GLuint normals_tex;
    double max_dist_to_quad_plane;

    quad_base_data_gpu(quad_tex_pool* qtp, GLuint offsets_tex, GLuint normals_tex, double max_dist_to_quad_plane) :
        _quad_tex_pool(qtp), offsets_tex(offsets_tex), normals_tex(normals_tex), max_dist_to_quad_plane(max_dist_to_quad_plane)
    {
    };

    ~quad_base_data_gpu()
    {
        if (_quad_tex_pool) {
            _quad_tex_pool->put(offsets_tex);
            _quad_tex_pool->put(normals_tex);
        }
    }
};

class quad_base_data_gpu_cache : public lru_cache<quad_base_data_gpu, quad_base_data_key, false>
{
public:
    quad_base_data_gpu_cache() : lru_cache<quad_base_data_gpu, quad_base_data_key, false>(0)
    {
    }
};

/* Memory cache: offset and normal information plus the max_dist_to_quad information */

class quad_base_data_mem
{
public:
    blob offsets;
    blob normals;
    double max_dist_to_quad_plane;

    quad_base_data_mem() : max_dist_to_quad_plane(0.0)
    {
    }
};

class quad_base_data_mem_cache : public lru_cache<quad_base_data_mem, quad_base_data_key, false>
{
public:
    quad_base_data_mem_cache() : lru_cache<quad_base_data_mem, quad_base_data_key, false>(0)
    {
    }
};

class quad_base_data_mem_cache_computer : public thread
{
private:
    class ecm _ecm;
    int _quad_size;

public:
    quad_base_data_key key;
    std::unique_ptr<class quad_base_data_mem> quad_base_data_mem;
    size_t quad_base_data_mem_size;

    quad_base_data_mem_cache_computer(const quad_base_data_key& key, const class ecm& ecm, int quad_size);
    ~quad_base_data_mem_cache_computer();
    virtual void run();
};

class quad_base_data_mem_cache_computers : public thread_group
{
private:
    std::set<quad_base_data_key> _active_computers;
    quad_base_data_mem_cache* _quad_base_data_mem_cache;
    mutex _mutex;

public:
    quad_base_data_mem_cache_computers(unsigned char size, quad_base_data_mem_cache* qbdmc);
    bool start_compute(const quad_base_data_key& key, const class ecm& ecm, int quad_size);
    bool locked_start_compute(const quad_base_data_key& key, const class ecm& ecm, int quad_size);
    void get_results();
};

#endif

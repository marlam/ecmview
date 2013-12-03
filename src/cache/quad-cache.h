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

#ifndef QUAD_CACHE_H
#define QUAD_CACHE_H

#include <string>
#include <memory>
#include <set>

#include <GL/glew.h>

#include <ecmdb/ecmdb.h>

#include "glvm.h"

#include "blb.h"
#include "pth.h"

#include "uuid.h"
#include "lru.h"
#include "quad-tex-pool.h"


/* A cache key for a quad */

class quad_key
{
public:
    uuid db_id;
    glvm::ivec4 quad;
    int approx_level;

    quad_key(const uuid& db_id, const glvm::ivec4& quad, int approx_level = -1)
    {
        if (sizeof(quad_key) != sizeof(db_id) + sizeof(quad) + sizeof(approx_level))
            std::memset(this, 0, sizeof(*this));
        this->db_id = db_id;
        this->quad = quad;
        this->approx_level = approx_level;
    }

    quad_key(const quad_key &key)
    {
        std::memcpy(this, &key, sizeof(*this));
    }

    bool operator<(const quad_key& qk) const
    {
        return std::memcmp(this, &qk, sizeof(*this)) < 0;
    }
};

/* GPU cache */

class quad_gpu
{
private:
    quad_tex_pool *_quad_tex_pool;

public:
    const GLuint data_tex;
    const GLuint mask_tex;
    const ecmdb::metadata meta;
    // data_tex == 0: quad contains no valid data
    // data_tex != 0 && mask_tex == 0: quad contains fully valid data
    // data_tex != 0 && mask_tex != 0: quad data validity stored in mask_tex

    quad_gpu(quad_tex_pool* qtp, GLuint data_tex, GLuint mask_tex, const ecmdb::metadata& meta);
    ~quad_gpu();
};

class quad_gpu_cache : public lru_cache<quad_gpu, quad_key, false>
{
public:
    quad_gpu_cache() : lru_cache<quad_gpu, quad_key, false>(0)
    {
    }
};

/* Memory cache */

class quad_mem
{
public:
    blob data;
    blob mask;
    ecmdb::metadata meta;
    // data.ptr() == 0: quad contains no valid data
    // data.ptr() != 0 && mask.ptr() == 0: quad contains fully valid data
    // data.ptr() != 0 && mask.ptr() != 0: quad data validity stored in mask_tex
};

class quad_mem_cache : public lru_cache<quad_mem, quad_key, false>
{
public:
    quad_mem_cache() : lru_cache<quad_mem, quad_key, false>(0)
    {
    }
};

class quad_mem_cache_loader : public thread
{
private:
    const ecmdb _db;
    const std::string _filename;

public:
    quad_key key;
    std::unique_ptr<class quad_mem> quad_mem;
    size_t quad_mem_size;

    quad_mem_cache_loader(const quad_key& key, const ecmdb& db, const std::string& filename);
    ~quad_mem_cache_loader();
    virtual void run();
};

class quad_mem_cache_loaders : public thread_group
{
private:
    std::set<quad_key> _active_loaders;
    quad_mem_cache* _quad_mem_cache;
    mutex _mutex;

public:
    quad_mem_cache_loaders(unsigned char size, quad_mem_cache* qmc);
    bool start_load(const quad_key& key, const ecmdb& db, const std::string& filename);
    bool locked_start_load(const quad_key& key, const ecmdb& db, const std::string& filename);
    void get_results();
};

/* Disk cache */

class quad_disk
{
public:
    enum {
        checking,       // the check if the quad is cached is ongoing
        uncached,       // quad is not cached
        caching,        // quad is being cached (not finished yet)
        cached,         // quad is cached and contains data
        cached_empty    // quad is cached and is empty
    };
    unsigned char status;

    quad_disk(unsigned char s) : status(s)
    {
    }
};

class quad_disk_cache : public lru_cache<quad_disk, quad_key, false>
{
public:
    const std::string app_id;
    const std::string cache_dir;

    quad_disk_cache(const std::string& app_id, const std::string& cache_dir);
    static std::string db_dir(const std::string& db_url);
    std::string quad_filename(const std::string& db_url, const glvm::ivec4& quad);
};

class quad_disk_cache_checker : public thread
{
private:
    const std::string _filename;

public:
    const quad_key key;
    unsigned char result; // uncached, cached, or cached_empty

    quad_disk_cache_checker(const quad_key& key, const std::string& filename);
    virtual void run();
};

class quad_disk_cache_checkers : public thread_group
{
private:
    std::set<quad_key> _active_checkers;
    quad_disk_cache* _quad_disk_cache;
    mutex _mutex;

public:
    quad_disk_cache_checkers(unsigned char size, quad_disk_cache* qcd);
    bool start_check(const quad_key& key, const std::string& filename);
    bool locked_start_check(const quad_key& key, const std::string& filename);
    void get_results();
};

class quad_disk_cache_fetcher : public thread
{
private:
    const quad_disk_cache* _quad_disk_cache;
    const ecmdb _db;
    const std::string _db_url;
    const std::string _db_username;
    const std::string _db_password;

public:
    const quad_key key;
    unsigned char result; // caching, cached, or chached_empty

    quad_disk_cache_fetcher(const quad_disk_cache* _qcd, const quad_key& key,
            const ecmdb& db, const std::string& db_url, const std::string& db_username, const std::string& db_password);
    virtual void run();
};

class quad_disk_cache_fetchers : public thread_group
{
private:
    std::set<quad_key> _active_fetchers;
    quad_disk_cache* _quad_disk_cache;
    mutex _mutex;

public:
    quad_disk_cache_fetchers(unsigned char size, quad_disk_cache* qcd);
    bool start_fetch(const quad_key& key,
            const ecmdb& db, const std::string& db_url, const std::string& db_username, const std::string& db_password);
    bool locked_start_fetch(const quad_key& key,
            const ecmdb& db, const std::string& db_url, const std::string& db_username, const std::string& db_password);
    void get_results();
};

/* Metadata cache (in main memory) */

class quad_metadata_cache : public lru_cache<ecmdb::metadata, quad_key, false>
{
public:
    quad_metadata_cache() : lru_cache<ecmdb::metadata, quad_key, false>(0)
    {
    }
};

#endif

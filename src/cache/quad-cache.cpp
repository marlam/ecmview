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

#include "config.h"

#include <limits>
#include <memory>
#include <cstring>

#include <ecmdb/ecmdb.h>

#include "dbg.h"
#include "exc.h"
#include "str.h"
#include "msg.h"
#include "fio.h"

#include "download.h"

#include "quad-cache.h"


/* GPU cache */

quad_gpu::quad_gpu(quad_tex_pool* qtp, GLuint data_tex, GLuint mask_tex, const ecmdb::quad_metadata& meta) :
    _quad_tex_pool(qtp), data_tex(data_tex), mask_tex(mask_tex), meta(meta)
{
    assert(data_tex == 0 || meta.is_valid());
};

quad_gpu::~quad_gpu()
{
    _quad_tex_pool->put(data_tex);
    _quad_tex_pool->put(mask_tex);
}


/* Memory cache */

quad_mem_cache_loader::quad_mem_cache_loader(const quad_key& key, const ecmdb& db, const std::string& filename) :
    _db(db), _filename(filename), key(key), quad_mem(), quad_mem_size(0)
{
}

quad_mem_cache_loader::~quad_mem_cache_loader()
{
}

void quad_mem_cache_loader::run()
{
    quad_mem.reset(new class quad_mem);
    quad_mem.get()->data.resize(_db.data_size());
    quad_mem.get()->mask.resize(_db.mask_size());
    bool all_valid;
    _db.load_quad(_filename, quad_mem.get()->data.ptr(), quad_mem.get()->mask.ptr<uint8_t>(), &all_valid, &(quad_mem.get()->meta));
    if (all_valid)
        quad_mem.get()->mask.free();
    quad_mem_size = _db.data_size() + _db.mask_size() + sizeof(ecmdb::quad_metadata);
}


quad_mem_cache_loaders::quad_mem_cache_loaders(unsigned char size, quad_mem_cache* qmc) :
    thread_group(size), _quad_mem_cache(qmc)
{
}

bool quad_mem_cache_loaders::start_load(const quad_key& key, const ecmdb& db, const std::string& filename)
{
    if (_active_loaders.find(key) != _active_loaders.end())
        return true;
    std::unique_ptr<quad_mem_cache_loader> t(new quad_mem_cache_loader(key, db, filename));
    bool r = this->start(t.get(), thread::priority_min);
    if (r) {
        _active_loaders.insert(key);
        t.release();
    }
    return r;
}

bool quad_mem_cache_loaders::locked_start_load(const quad_key& key, const ecmdb& db, const std::string& filename)
{
    bool r;
    _mutex.lock();
    try {
        r = start_load(key, db, filename);
    }
    catch (exc& e) {
        _mutex.unlock();
        throw e;
    }
    catch (std::exception& e) {
        _mutex.unlock();
        throw exc(e);
    }
    _mutex.unlock();
    return r;
}

void quad_mem_cache_loaders::get_results()
{
    quad_mem_cache_loader* t;
    while ((t = static_cast<quad_mem_cache_loader*>(this->get_next_finished_thread()))) {
        auto it = _active_loaders.find(t->key);
        assert(it != _active_loaders.end());
        _active_loaders.erase(it);
        _quad_mem_cache->put(t->key, t->quad_mem.release(), t->quad_mem_size);
    }
}


/* Disk cache */

quad_disk_cache::quad_disk_cache(const std::string& app_id, const std::string& cache_dir) :
    lru_cache<quad_disk, quad_key, false>(0), app_id(app_id), cache_dir(cache_dir)
{
}

std::string quad_disk_cache::db_dir(const std::string& db_url)
{
    // Ideally, the db_dir for a given URL should be virtually collision-free.
    // Therefore, we should use a cryptographic hash.
    // But that seems expensive, and I also want to be able to guess the original
    // URL from the db_dir name. So for now I just use a sanitized version of the
    // URL + a primitive hash (see djb2 at http://www.cse.yorku.ca/~oz/hash.html)
    // to avoid the most obvious name collisions. FIXME: this needs to be improved!
    std::string db_dir = db_url;
    uint64_t djb2_hash = 5381;
    for (size_t i = 0; i < db_dir.length(); i++) {
        char &c = db_dir[i];
        if (!(c >= 'a' && c <= 'z')
                && !(c >= 'A' && c <= 'Z')
                && !(c >= '0' && c <= '9')
                && c != '_' && c != '-') {
            c = '_';
        }
        djb2_hash = djb2_hash * 33 + c;
    }
    return db_dir + str::hex(&djb2_hash, sizeof(djb2_hash));
}

std::string quad_disk_cache::quad_filename(const std::string& db_url, const glvm::ivec4& quad)
{
    return cache_dir + '/' + db_dir(db_url) + '/' + ecmdb::quad_filename(quad[0], quad[1], quad[2], quad[3]);
}

quad_disk_cache_checker::quad_disk_cache_checker(const quad_key& key, const std::string& filename) :
    _filename(filename), key(key)
{
}

void quad_disk_cache_checker::run()
{
    struct stat buf;
    bool r = fio::stat(_filename, &buf);
    result = r ? (buf.st_size == 0 ? quad_disk::cached_empty : quad_disk::cached) : quad_disk::uncached;
}


quad_disk_cache_checkers::quad_disk_cache_checkers(unsigned char size, quad_disk_cache *qcd) :
    thread_group(size), _quad_disk_cache(qcd)
{
}

bool quad_disk_cache_checkers::start_check(const quad_key& key, const std::string& filename)
{
    if (_active_checkers.find(key) != _active_checkers.end())
        return true;
    std::unique_ptr<quad_disk_cache_checker> t(new quad_disk_cache_checker(key, filename));
    bool r = this->start(t.get(), thread::priority_min);
    if (r) {
        _active_checkers.insert(key);
        t.release();
    }
    return r;
}

bool quad_disk_cache_checkers::locked_start_check(const quad_key& key, const std::string& filename)
{
    bool r;
    _mutex.lock();
    try {
        r = start_check(key, filename);
    }
    catch (exc& e) {
        _mutex.unlock();
        throw e;
    }
    catch (std::exception& e) {
        _mutex.unlock();
        throw exc(e);
    }
    _mutex.unlock();
    return r;
}

void quad_disk_cache_checkers::get_results()
{
    quad_disk_cache_checker* t;
    while ((t = static_cast<quad_disk_cache_checker*>(this->get_next_finished_thread()))) {
        auto it = _active_checkers.find(t->key);
        assert(it != _active_checkers.end());
        _active_checkers.erase(it);
        _quad_disk_cache->put(t->key, new quad_disk(t->result));
    }
}


quad_disk_cache_fetcher::quad_disk_cache_fetcher(const quad_disk_cache* qcd, const quad_key& key,
        const ecmdb& db, const std::string& db_url, const std::string& db_username, const std::string& db_password) :
    _quad_disk_cache(qcd), _db(db), _db_url(db_url), _db_username(db_username), _db_password(db_password), key(key)
{
}

void quad_disk_cache_fetcher::run()
{
    // This function may download the quad data and store it.
    // But it may also return immediately if it thinks that another process or
    // thread already is in the process of caching the quad.
    // This function must be thread safe, i.e. multiple processes and/or threads
    // must be able to use the cache simultaneously.
    // - If the quad exists in the cache, it must be complete, so that the
    //   application does not need to care about partially written files
    // - No concurrent processes or threads should write to the same files
    // - Stale files from a crashed instance should be handled gracefully

    // TODO: Set a timeout for this function, so that a hanging thread cannot
    // block caching of a quad forever.

    std::string base_dir = _quad_disk_cache->cache_dir + '/' + _quad_disk_cache->db_dir(_db_url);
    std::string quad_filename = ecmdb::quad_filename(key.quad[0], key.quad[1], key.quad[2], key.quad[3]);
    
    std::string quad_dst = base_dir + '/' + quad_filename;
    std::string quad_url = _db_url + quad_filename;

    fio::mkdir_p(_quad_disk_cache->cache_dir, _quad_disk_cache->db_dir(_db_url) + '/' + fio::dirname(quad_filename));

    // Shortcut: for 'file://' URLs, we can use symbolic links to cache
    // existing quads, to avoid disk usage.
    if (quad_url.substr(0, 7) == "file://") {
        if (fio::test_f(quad_url.substr(7))) {
            try {
                fio::symlink(quad_url.substr(7), quad_dst);
            }
            catch (exc& e) {
                if (e.sys_errno() != EEXIST)
                    throw e;
            }
            result = quad_disk::cached;
        } else {
            FILE* f = NULL;
            try {
                f = fio::open(quad_dst, "w", O_EXCL);
            }
            catch (exc& e) {
                if (e.sys_errno() != EEXIST)
                    throw e;
            }
            if (f)
                fio::close(f, quad_dst);
            result = quad_disk::cached_empty;
        }
        return;
    }
    
    std::string quad_tmp = quad_dst + '.' + _quad_disk_cache->app_id + ".hardlink";
    // Open quad_tmp for exclusive access. Since this file name includes
    // the application ID and will not be removed from the file system even
    // when the new name dst_quad is created, we can be sure that no two
    // threads or processes that belong to the same application instance
    // try to cache this quad.
    // It can happen that two independent application instances access the
    // same cache directory (although that would not make much sense). In
    // that case, it may happen that both try to cache the same quad. Only
    // one will be able to create the new name dst_quad. Therefore, we
    // ignore EEXIST on the call to link() below.
    FILE* quad_tmp_f = NULL;
    try {
        quad_tmp_f = fio::open(quad_tmp, "w", O_EXCL);
    }
    catch (exc& e) {
        if (e.sys_errno() != EEXIST)
            throw e;
    }
    if (quad_tmp_f) {
        bool quad_is_empty = false;
        try {
            download(quad_tmp_f, quad_url, _db_username, _db_password);
        }
        catch (exc& e) {
            if (e.sys_errno() == ENOENT)
                quad_is_empty = true;
            else
                throw e;
        }
        fio::flush(quad_tmp_f, quad_tmp);
        fio::advise(quad_tmp_f, POSIX_FADV_DONTNEED, quad_tmp);
        fio::close(quad_tmp_f, quad_tmp);
        try {
            fio::link(quad_tmp, quad_dst);
        }
        catch (exc &e) {
            if (e.sys_errno() != EEXIST)
                throw e;
        }
        result = quad_is_empty ? quad_disk::cached_empty : quad_disk::cached;
    } else {
        result = quad_disk::caching;
    }
}


quad_disk_cache_fetchers::quad_disk_cache_fetchers(unsigned char size, quad_disk_cache* qcd) :
    thread_group(size), _quad_disk_cache(qcd)
{
}

bool quad_disk_cache_fetchers::start_fetch(const quad_key& key,
            const ecmdb& db, const std::string& db_url, const std::string& db_username, const std::string& db_password)
{
    if (_active_fetchers.find(key) != _active_fetchers.end())
        return true;
    std::unique_ptr<quad_disk_cache_fetcher> t(new quad_disk_cache_fetcher(
                _quad_disk_cache, key, db, db_url, db_username, db_password));
    bool r = this->start(t.get(), thread::priority_min);
    if (r) {
        _active_fetchers.insert(key);
        t.release();
    }
    return r;
}

bool quad_disk_cache_fetchers::locked_start_fetch(const quad_key& key,
            const ecmdb& db, const std::string& db_url, const std::string& db_username, const std::string& db_password)
{
    bool r;
    _mutex.lock();
    try {
        r = start_fetch(key, db, db_url, db_username, db_password);
    }
    catch (exc& e) {
        _mutex.unlock();
        throw e;
    }
    catch (std::exception& e) {
        _mutex.unlock();
        throw exc(e);
    }
    _mutex.unlock();
    return r;
}

void quad_disk_cache_fetchers::get_results()
{
    quad_disk_cache_fetcher* t;
    while ((t = static_cast<quad_disk_cache_fetcher*>(this->get_next_finished_thread()))) {
        auto it = _active_fetchers.find(t->key);
        assert(it != _active_fetchers.end());
        _active_fetchers.erase(it);
        if (t->result != quad_disk::caching)
            _quad_disk_cache->put(t->key, new quad_disk(t->result));
    }
}

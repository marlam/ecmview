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

#include "quad-base-data-cache.h"


/* Memory cache */

quad_base_data_mem_cache_computer::quad_base_data_mem_cache_computer(const quad_base_data_key& key, const class ecm& ecm, int quad_size) :
    _ecm(ecm), _quad_size(quad_size), key(key), quad_base_data_mem(), quad_base_data_mem_size(0)
{
}

quad_base_data_mem_cache_computer::~quad_base_data_mem_cache_computer()
{
}

void quad_base_data_mem_cache_computer::run()
{
    quad_base_data_mem.reset(new class quad_base_data_mem);
    
    const int overlap = 2;

    double quad_tl_cart[3], quad_tr_cart[3], quad_bl_cart[3], quad_br_cart[3];
    double ecmx, ecmy;
    _ecm.quad_to_ecm(key.quad[0], key.quad[1], key.quad[2], key.quad[3], ecm::corner_tl, &ecmx, &ecmy);
    _ecm.ecm_to_cartesian(ecmx, ecmy, quad_tl_cart);
    _ecm.quad_to_ecm(key.quad[0], key.quad[1], key.quad[2], key.quad[3], ecm::corner_tr, &ecmx, &ecmy);
    _ecm.ecm_to_cartesian(ecmx, ecmy, quad_tr_cart);
    _ecm.quad_to_ecm(key.quad[0], key.quad[1], key.quad[2], key.quad[3], ecm::corner_bl, &ecmx, &ecmy);
    _ecm.ecm_to_cartesian(ecmx, ecmy, quad_bl_cart);
    _ecm.quad_to_ecm(key.quad[0], key.quad[1], key.quad[2], key.quad[3], ecm::corner_br, &ecmx, &ecmy);
    _ecm.ecm_to_cartesian(ecmx, ecmy, quad_br_cart);

    double plane_normal[3], plane_distance;
    _ecm.quad_plane(key.quad[0], key.quad[1], key.quad[2], key.quad[3],
            quad_tl_cart, quad_tr_cart, quad_bl_cart, quad_br_cart,
            plane_normal, &plane_distance);

    size_t offsets_size = (_quad_size + 2 * overlap) * (_quad_size + 2 * overlap) * 3 * sizeof(float);
    size_t normals_size = (_quad_size + 2 * overlap) * (_quad_size + 2 * overlap) * 2 * sizeof(float);
    quad_base_data_mem.get()->offsets.resize(offsets_size);
    quad_base_data_mem.get()->normals.resize(normals_size);
    _ecm.quad_base_data(key.quad[0], key.quad[1], key.quad[2], key.quad[3],
            quad_tl_cart, quad_tr_cart, quad_bl_cart, quad_br_cart, plane_normal, plane_distance,
            _quad_size, overlap,
            quad_base_data_mem.get()->offsets.ptr<float>(), quad_base_data_mem.get()->normals.ptr<float>(),
            &(quad_base_data_mem.get()->max_dist_to_quad_plane));

    quad_base_data_mem_size = offsets_size + normals_size + sizeof(double);
}

quad_base_data_mem_cache_computers::quad_base_data_mem_cache_computers(unsigned char size, quad_base_data_mem_cache* qbdmc) :
    thread_group(size), _quad_base_data_mem_cache(qbdmc)
{
}

bool quad_base_data_mem_cache_computers::start_compute(const quad_base_data_key& key, const class ecm& ecm, int quad_size)
{
    if (_active_computers.find(key) != _active_computers.end())
        return true;
    std::unique_ptr<quad_base_data_mem_cache_computer> t(new quad_base_data_mem_cache_computer(key, ecm, quad_size));
    bool r = this->start(t.get(), thread::priority_min);
    if (r) {
        _active_computers.insert(key);
        t.release();
    }
    return r;
}

bool quad_base_data_mem_cache_computers::locked_start_compute(const quad_base_data_key& key, const class ecm& ecm, int quad_size)
{
    bool r;
    _mutex.lock();
    try {
        r = start_compute(key, ecm, quad_size);
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

void quad_base_data_mem_cache_computers::get_results()
{
    quad_base_data_mem_cache_computer* t;
    while ((t = static_cast<quad_base_data_mem_cache_computer*>(this->get_next_finished_thread()))) {
        auto it = _active_computers.find(t->key);
        assert(it != _active_computers.end());
        _active_computers.erase(it);
        _quad_base_data_mem_cache->put(t->key, t->quad_base_data_mem.release(), t->quad_base_data_mem_size);
    }
}

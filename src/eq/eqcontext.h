/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2012
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

#ifndef EQCONTEXT_H
#define EQCONTEXT_H

#include "state.h"
#include "renderer-context.h"

class eq_node_factory;
class eq_config;

class eq_context : public renderer_context
{
private:
    bool _valid;                        // Is this context usable?
    class state *_master_state;         // Master state, synchronized with Equalizer
    eq_node_factory *_eq_node_factory;  // Equalizer node factory
    eq_config *_eq_config;              // Master Equalizer configuration

public:
    eq_context(int *argc, char *argv[], class state* master_state);
    ~eq_context();

    void quit();
    bool is_running();

    /* For use by the application. */
    virtual GLEWContext* glewGetContext();
    virtual void init_gl();
    virtual void exit_gl();
    virtual void render();

    /* For use by the renderer. */
    virtual const class state& state() const;
    virtual bool start_per_node_maintenance();
    virtual void finish_per_node_maintenance();
    virtual bool start_per_glcontext_maintenance();
    virtual void finish_per_glcontext_maintenance();
    virtual class quad_disk_cache* quad_disk_cache();
    virtual class quad_disk_cache_checkers* quad_disk_cache_checkers();
    virtual class quad_disk_cache_fetchers* quad_disk_cache_fetchers();
    virtual class quad_mem_cache* quad_mem_cache();
    virtual class quad_mem_cache_loaders* quad_mem_cache_loaders();
    virtual class quad_gpu_cache* quad_gpu_cache();
    virtual class quad_metadata_cache* quad_metadata_cache();
    virtual class quad_base_data_mem_cache* quad_base_data_mem_cache();
    virtual class quad_base_data_mem_cache_computers* quad_base_data_mem_cache_computers();
    virtual class quad_base_data_gpu_cache* quad_base_data_gpu_cache();
    virtual class quad_tex_pool* quad_tex_pool();
};

#endif

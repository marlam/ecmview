/*
 * Copyright (C) 2011, 2012, 2013
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

#ifndef STATE_H
#define STATE_H

#include <string>

#include "glvm.h"

#include "msg.h"
#include "ser.h"

#include "database_description.h"
#include "light_parameters.h"
#include "lens_parameters.h"
#include "renderer_parameters.h"


class state : public serializable
{
private:
    /* Private helper functions */
    void create_e2c_database(std::vector<class database_description>& db_descriptions);
    uuid open_database(const std::string& url, const std::string& username, const std::string& password,
            std::vector<class database_description>& db_descriptions);
    void reset_bounding_sphere();

public:
    /* Unchangeable data (init data) */
    msg::level_t msg_level;
    std::string app_id;
    std::string conf_file;
    std::string cache_dir;
    int tracking; // 0 = off, 1 = EqEvent, 2 = DTrack

    /* Changeable data (frame data) */
    // 1. The scene (saveable in statefile)
    std::vector<class database_description> database_descriptions;
    class light_parameters light;
    class lens_parameters lens;
    double inner_bounding_sphere_radius;
    double outer_bounding_sphere_radius;
    // 2. The view (saveable in statefile)
    glvm::dvec3 viewer_pos;        // cartesian
    glvm::dquat viewer_rot;
    // 3. The rendering (saveable in app preferences)
    class renderer_parameters renderer;
    // 4. Other, not saveable
    glvm::vec3 tracker_pos;        // cartesian
    glvm::quat tracker_rot;
    glvm::ivec2 pointer_pos;
    int debug_quad_depth_pass;
    int debug_quad_index;
    bool debug_quad_save;

public:
    // This constructor builds a master state:
    state(msg::level_t msg_level,
            const std::string& app_id,
            const std::string& conf_file,
            const std::string& cache_dir,
            int tracking);
    // The following constructors are only for slave states:
    state();
    state(const state& s);

    // Convenience access functions
    class database_description* get_database_description(const uuid& db_uuid);
    bool have_databases() const;
    double semi_major_axis() const;
    double semi_minor_axis() const;
    int quad_size() const;
    
    // Serialization
    void save(std::ostream& os) const;
    void load(std::istream& is);

    /* Interface to manage the master state */
    uuid open_database(const std::string& url, const std::string& username = "", const std::string& password = "");
    void close_database(const uuid& db_uuid);
    void reset_viewer();
    void save_statefile(const std::string& filename) const;
    void load_statefile(const std::string& filename);
};

#endif

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

#include <limits>

#include <ecm/ecm.h>

#include "fio.h"
#include "msg.h"
#include "dbg.h"

#include "glvm-s11n.h"
#include "glvm-str.h"
#include "download.h"

#include "auth_callback.h"
#include "state.h"

using namespace glvm;


state::state(msg::level_t msg_level,
        const std::string &app_id,
        const std::string &conf_file,
        const std::string &cache_dir,
        int tracking) :
    msg_level(msg_level),
    app_id(app_id), conf_file(conf_file), cache_dir(cache_dir),
    tracking(tracking),
    viewer_pos(0.0, 0.0, 0.0), viewer_rot(0.0, 0.0, 0.0, 0.0),
    tracker_pos(0.0f, 0.0f, 0.0f), tracker_rot(toQuat(0.0f, vec3(1.0f, 0.0f, 0.0f))),
    pointer_pos(-1, -1),
    debug_quad_depth_pass(-1), debug_quad_index(-1), debug_quad_save(false)
{
    create_e2c_database(database_descriptions);
}

state::state()
{
}

state::state(const state& s)
{
    std::stringstream ss;
    s11n::save(ss, s);
    s11n::load(ss, *this);
}

class database_description* state::get_database_description(const uuid& db_uuid)
{
    for (size_t i = 0; i < database_descriptions.size(); i++)
        if (database_descriptions[i].uuid == db_uuid)
            return &(database_descriptions[i]);
    return NULL;
}

bool state::have_databases() const
{
    assert(database_descriptions.size() > 0);
    assert(database_descriptions[0].processing_parameters[0].category_e2c);
    return (database_descriptions.size() > 1);
}

double state::semi_major_axis() const
{
    assert(database_descriptions.size() > 0);
    assert(database_descriptions[0].processing_parameters[0].category_e2c);
    return (database_descriptions.size() > 1
            ? database_descriptions[1].db.semi_major_axis()
            : std::numeric_limits<double>::quiet_NaN());
}

double state::semi_minor_axis() const
{
    assert(database_descriptions.size() > 0);
    assert(database_descriptions[0].processing_parameters[0].category_e2c);
    return (database_descriptions.size() > 1
            ? database_descriptions[1].db.semi_minor_axis()
            : std::numeric_limits<double>::quiet_NaN());
}

int state::quad_size() const
{
    assert(database_descriptions.size() > 0);
    assert(database_descriptions[0].processing_parameters[0].category_e2c);
    return (database_descriptions.size() > 1
            ? database_descriptions[1].db.quad_size()
            : -1);
}

void state::save(std::ostream& os) const
{
    s11n::save(os, static_cast<int>(msg_level));
    s11n::save(os, app_id);
    s11n::save(os, conf_file);
    s11n::save(os, cache_dir);
    s11n::save(os, tracking);

    s11n::save(os, database_descriptions);
    s11n::save(os, light);
    s11n::save(os, lens);
    s11n::save(os, inner_bounding_sphere_radius);
    s11n::save(os, outer_bounding_sphere_radius);

    s11n::save(os, viewer_pos);
    s11n::save(os, viewer_rot);
    
    s11n::save(os, renderer);

    s11n::save(os, tracker_pos);
    s11n::save(os, tracker_rot);
    s11n::save(os, pointer_pos);
    s11n::save(os, debug_quad_depth_pass);
    s11n::save(os, debug_quad_index);
    s11n::save(os, debug_quad_save);
}

void state::load(std::istream& is)
{
    int x;
    s11n::load(is, x);
    msg_level = static_cast<msg::level_t>(x);
    s11n::load(is, app_id);
    s11n::load(is, conf_file);
    s11n::load(is, cache_dir);
    s11n::load(is, tracking);

    s11n::load(is, database_descriptions);
    s11n::load(is, light);
    s11n::load(is, lens);
    s11n::load(is, inner_bounding_sphere_radius);
    s11n::load(is, outer_bounding_sphere_radius);

    s11n::load(is, viewer_pos);
    s11n::load(is, viewer_rot);
    
    s11n::load(is, renderer);

    s11n::load(is, tracker_pos);
    s11n::load(is, tracker_rot);
    s11n::load(is, pointer_pos);
    s11n::load(is, debug_quad_depth_pass);
    s11n::load(is, debug_quad_index);
    s11n::load(is, debug_quad_save);
}

void state::save_statefile(const std::string& filename) const
{
    std::ostringstream oss;
    s11n::save(oss, "viewer_pos", viewer_pos);
    oss.put('\n');
    s11n::save(oss, "viewer_rot", viewer_rot);
    oss.put('\n');
    s11n::save(oss, "light", light);
    oss.put('\n');
    s11n::save(oss, "lens", lens);
    oss.put('\n');
    for (size_t i = 0; i < database_descriptions.size(); i++) {
        if (database_descriptions[i].username.empty()) {
            s11n::save(oss, "database_url", database_descriptions[i].url);
        } else {
            s11n::save(oss, "database_auth_url", database_descriptions[i].url);
        }
        oss.put('\n');
        s11n::save(oss, "database_active", database_descriptions[i].active[0]);
        oss.put('\n');
        s11n::save(oss, "database_priority", database_descriptions[i].priority[0]);
        oss.put('\n');
        s11n::save(oss, "database_weight", database_descriptions[i].weight[0]);
        oss.put('\n');
        s11n::save(oss, "database_processing_parameters", database_descriptions[i].processing_parameters[0]);
        oss.put('\n');
        s11n::save(oss, "database_lens_active", database_descriptions[i].active[1]);
        oss.put('\n');
        s11n::save(oss, "database_lens_priority", database_descriptions[i].priority[1]);
        oss.put('\n');
        s11n::save(oss, "database_lens_weight", database_descriptions[i].weight[1]);
        oss.put('\n');
        s11n::save(oss, "database_lens_processing_parameters", database_descriptions[i].processing_parameters[1]);
        oss.put('\n');
    }
    FILE* f = fio::open(filename, "w");
    fio::write(oss.str().data(), oss.str().length(), 1, f, filename);
    fio::close(f);
}

void state::load_statefile(const std::string& filename)
{
    std::vector<std::string> contents;
    FILE* f = fio::open(filename, "r");
    char c;
    while ((c = fio::getc(f, filename)) != EOF && !::ferror(f)) {
        fio::ungetc(c, f, filename);
        contents.push_back(fio::readline(f, filename));
    }
    fio::close(f);

    dvec3 load_viewer_pos(0.0, 0.0, 0.0);
    dquat load_viewer_rot(0.0, 0.0, 0.0, 0.0);
    class light_parameters load_light;
    class lens_parameters load_lens;
    std::vector<class database_description> load_database_descriptions;

    std::string name, value;
    size_t contents_line_index = 0;

    while (contents_line_index < contents.size()) {
        std::istringstream iss(contents[contents_line_index++]);
        s11n::load(iss, name, value);
        if (name == "viewer_pos") {
            s11n::load(value, load_viewer_pos);
        } else if (name == "viewer_rot") {
            s11n::load(value, load_viewer_rot);
        } else if (name == "light") {
            s11n::load(value, load_light);
        } else if (name == "lens") {
            s11n::load(value, load_lens);
        } else if (name == "database_url") {
            std::string url;
            s11n::load(value, url);
            if (url == "") {
                if (!load_database_descriptions.empty()) {
                    msg::wrn("Ignoring databases specified before the elevation-to-texture database.");
                    load_database_descriptions.clear();
                }
                create_e2c_database(load_database_descriptions);
            } else {
                open_database(url, "", "", load_database_descriptions);
            }
        } else if (name == "database_auth_url") {
            std::string url, username, password;
            s11n::load(value, url);
            if (global_auth_callback) {
                global_auth_callback->get(url, username, password);
            }
            open_database(url, username, password, load_database_descriptions);
        } else if (load_database_descriptions.size() > 0 && name == "database_active") {
            s11n::load(value, load_database_descriptions.back().active[0]);
        } else if (load_database_descriptions.size() > 0 && name == "database_priority") {
            s11n::load(value, load_database_descriptions.back().priority[0]);
        } else if (load_database_descriptions.size() > 0 && name == "database_weight") {
            s11n::load(value, load_database_descriptions.back().weight[0]);
        } else if (load_database_descriptions.size() > 0 && name == "database_processing_parameters") {
            s11n::load(value, load_database_descriptions.back().processing_parameters[0]);
        } else if (load_database_descriptions.size() > 0 && name == "database_lens_active") {
            s11n::load(value, load_database_descriptions.back().active[1]);
        } else if (load_database_descriptions.size() > 0 && name == "database_lens_priority") {
            s11n::load(value, load_database_descriptions.back().priority[1]);
        } else if (load_database_descriptions.size() > 0 && name == "database_lens_weight") {
            s11n::load(value, load_database_descriptions.back().weight[1]);
        } else if (load_database_descriptions.size() > 0 && name == "database_lens_processing_parameters") {
            s11n::load(value, load_database_descriptions.back().processing_parameters[1]);
        } else  {
            msg::wrn(std::string("Ignored variable ") + name + " in state file "
                    + filename + " line " + str::from(contents_line_index));
        }
    }
    for (size_t i = 2; i < load_database_descriptions.size(); i++) {
        if (!load_database_descriptions[i].db.is_compatible(load_database_descriptions[1].db)) {
            throw exc("Invalid content in state file " + filename, EINVAL);
        }
    }

    database_descriptions = load_database_descriptions;
    light = load_light;
    lens = load_lens;
    viewer_pos = load_viewer_pos;
    viewer_rot = load_viewer_rot;

    reset_bounding_sphere();

    return;
}

void state::create_e2c_database(std::vector<class database_description>& db_descriptions)
{
    assert(db_descriptions.empty());
    // Create the database description for the pseudo dataset
    // that converts elevation to texture data
    class database_description dd;
    dd.db.create(1.0, 1.0, 4, 1, ecmdb::category_texture, ecmdb::type_uint8, 3, 1, 0.0f, 1.0f,
            "Texture from elevation", std::vector<std::string>(1,
                "Pseudo database that generates texture from elevation"));
    for (int s = 0; s < 6; s++)
        dd.db.add_quad(s, 0, 0);
    dd.active[0] = false;
    dd.active[1] = false;
    dd.priority[0] = 10;
    dd.priority[1] = 10;
    dd.weight[0] = 1.0f;
    dd.weight[1] = 1.0f;
    dd.processing_parameters[0] = processing_parameters(true);
    dd.processing_parameters[1] = processing_parameters(true);
    db_descriptions.push_back(dd);
}

uuid state::open_database(const std::string& url, const std::string& username, const std::string& password,
        std::vector<class database_description>& db_descriptions)
{
    class database_description dd;
    dd.url = url;
    dd.username = username;
    dd.password = password;

    if (dd.url.find("://") == std::string::npos /* && fio::test_d(dd.url) */)
        dd.url = "file://" + dd.url;
    if (dd.url.length() > 0 && dd.url[dd.url.length() - 1] != '/')
        dd.url.push_back('/');

    std::string tempdir = fio::mktempdir(PACKAGE_TARNAME);
    download(tempdir + "/ecmdb.txt", dd.url + "ecmdb.txt", dd.username, dd.password);
    download(tempdir + "/meta.txt", dd.url + "meta.txt", dd.username, dd.password);
    dd.db.open(tempdir + '/', dd.url);
    dd.meta.open(dd.db.category(), tempdir + '/', dd.url);
    fio::rm_r(tempdir);

    assert(db_descriptions.size() > 0);
    assert(db_descriptions[0].processing_parameters[0].category_e2c);
    if (db_descriptions.size() > 1 && !dd.db.is_compatible(db_descriptions[1].db)) {
        throw exc(url + ": database is incompatible with already opened databases!");
    }

    dd.uuid.generate();
    dd.active[0] = true;
    dd.active[1] = true;
    dd.priority[0] = 10;
    dd.priority[1] = 10;
    dd.weight[0] = 1.0f;
    dd.weight[1] = 1.0f;
    dd.processing_parameters[0] = processing_parameters(dd.db.category());
    dd.processing_parameters[1] = processing_parameters(dd.db.category());
    msg::dbg("Opened database %s with UUID %s", dd.url.c_str(), dd.uuid.to_string().c_str());
    db_descriptions.push_back(dd);
    return dd.uuid;
}

uuid state::open_database(const std::string& url, const std::string& username, const std::string& password)
{
    uuid u = open_database(url, username, password, database_descriptions);
    reset_bounding_sphere();
    return u;
}

void state::close_database(const uuid& db_uuid)
{
    assert(!(db_uuid == uuid()));
    for (size_t i = 0; i < database_descriptions.size(); i++) {
        if (database_descriptions[i].uuid == db_uuid) {
            assert(!database_descriptions[i].processing_parameters[0].category_e2c);
            database_descriptions.erase(database_descriptions.begin() + i);
            return;
        }
    }
    // If we reach this point, there was no such database!
    assert(false);
}

void state::reset_viewer()
{
    assert(database_descriptions.size() > 0);
    assert(database_descriptions[0].processing_parameters[0].category_e2c);
    if (database_descriptions.size() == 1) {
        viewer_pos = dvec3(0.0, 0.0, 0.0);
        viewer_rot = dquat(0.0, 0.0, 0.0, 0.0);
    } else {
        ecm m = ecm(outer_bounding_sphere_radius, outer_bounding_sphere_radius);
        m.geodetic_to_cartesian(0.0, 0.0, 4.0 * outer_bounding_sphere_radius, viewer_pos.vl);
        viewer_rot = toQuat(dvec3(0.0, 0.0, -1.0), -viewer_pos);
        viewer_rot *= toQuat(dvec3(1.0, 0.0, 0.0), dvec3(0.0, 1.0, 0.0));
    }
}

void state::reset_bounding_sphere()
{
    assert(database_descriptions.size() > 0);
    assert(database_descriptions[0].processing_parameters[0].category_e2c);
    if (database_descriptions.size() == 1) {
        inner_bounding_sphere_radius = 0.0;
        outer_bounding_sphere_radius = 0.0;
    } else {
        double min_axis = +std::numeric_limits<double>::max();
        double max_axis = -std::numeric_limits<double>::max();
        double min_elev = 0.0;
        double max_elev = 0.0;
        for (size_t i = 1; i < database_descriptions.size(); i++) {
            if (database_descriptions[i].db.semi_minor_axis() < min_axis) {
                min_axis = database_descriptions[i].db.semi_minor_axis();
            }
            if (database_descriptions[i].db.semi_major_axis() > max_axis) {
                max_axis = database_descriptions[i].db.semi_major_axis();
            }
            if (database_descriptions[i].db.category() == ecmdb::category_elevation) {
                if (database_descriptions[i].meta.elevation.min < min_elev) {
                    min_elev = database_descriptions[i].meta.elevation.min;
                }
                if (database_descriptions[i].meta.elevation.max > max_elev) {
                    max_elev = database_descriptions[i].meta.elevation.max;
                }
            }
        }
        inner_bounding_sphere_radius = min_axis + 5 * min_elev;
        outer_bounding_sphere_radius = max_axis + 5 * max_elev;
    }
}

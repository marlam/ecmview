/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012
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

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include "glvm.h"


class navigator
{
private:
    enum {
        state_inactive,
        state_planet_rot,
        state_planet_distchange,
        state_viewer_rot,
        state_surface_object_move
    } _state;
    glvm::ivec4 _viewport;
    glvm::ivec2 _last_pos;
    // for planet_rot:
    glvm::dvec3 _last_ballpos;
    // for planet_distchange:
    double _last_inner_bounding_sphere_radius;
    double _last_dist_to_inner_bounding_sphere;
    // for viewer rot:
    glvm::dvec3 _last_viewer_dir;

    glvm::dvec3 ballmap(const glvm::ivec2& pos);

public:
    navigator();

    void set_viewport(const glvm::ivec4& viewport);

    void planet_rot_start(const glvm::ivec2& pos, double inner_bounding_sphere_radius);
    void planet_rot(double speed_factor, const glvm::ivec2& pos, glvm::dvec3& viewer_pos, glvm::dquat& viewer_rot);

    void planet_distchange_start(const glvm::ivec2& pos, const glvm::dvec3& viewer_pos, double inner_bounding_sphere_radius);
    void planet_distchange(double speed_factor, const glvm::ivec2& pos, glvm::dvec3& viewer_pos);

    void viewer_rot_start(const glvm::ivec2& pos);
    void viewer_rot(double speed_factor, const glvm::ivec2& pos, glvm::dquat& viewer_rot);

    void viewer_shift(double speed_factor, double wheel_radians, double inner_bounding_sphere_radius,
            const glvm::dquat& viewer_rot, glvm::dvec3& viewer_pos);

    void surface_object_move_start(const glvm::ivec2& pos, const glvm::dvec3& viewer_pos, double inner_bounding_sphere_radius);
    void surface_object_move(double speed_factor, const glvm::ivec2& pos, glvm::dvec2& object_latlon);
    void surface_object_scale(double speed_factor, double wheel_radians, 
            const glvm::dvec3& viewer_pos, double inner_bounding_sphere_radius,
            double& object_size);
};

#endif

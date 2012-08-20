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

#include "config.h"

#include <limits>

#include "glvm.h"
#include "glvm-str.h"

#include "msg.h"
#include "dbg.h"

#include "navigator.h"

using namespace glvm;


navigator::navigator() :
    _state(state_inactive),
    _viewport(0, 0, -1, -1)
{
}

dvec3 navigator::ballmap(const ivec2& p)
{
    assert(_viewport[2] > 0);
    assert(_viewport[3] > 0);

    int x = p.x - _viewport[0];
    int y = p.y - _viewport[1];
    int w = max(2, _viewport[2]);
    int h = max(2, _viewport[3]);

    // bring v=(x,y) to [-1..1]^2
    dvec2 v(x, h - 1 - y);
    v /= dvec2(w - 1, h - 1);
    v -= dvec2(0.5, 0.5);
    v *= 2.0;

    double ll = dot(v, v);
    if (ll > 1.0) {
        // outside ArcBall
        return dvec3(v / sqrt(ll), 0.0);
    } else {
        // inside ArcBall
        return dvec3(v, sqrt(1.0 - ll));
    }
}

void navigator::set_viewport(const ivec4& viewport)
{
    _viewport = viewport;
}

void navigator::planet_rot_start(const ivec2& pos, double inner_bounding_sphere_radius)
{
    _state = state_planet_rot;
    _last_ballpos = ballmap(pos);
    _last_inner_bounding_sphere_radius = inner_bounding_sphere_radius;
}

void navigator::planet_rot(double speed_factor, const ivec2& pos, dvec3& viewer_pos, dquat& viewer_rot)
{
    if (_state == state_planet_rot) {
        dvec3 ballpos = ballmap(pos);
        dvec3 normal = cross(_last_ballpos, ballpos);
        dvec3 axis = viewer_rot * normal;
        double angle = acos(dot(_last_ballpos, ballpos));
        double dist_to_inner_bounding_sphere = length(viewer_pos) - _last_inner_bounding_sphere_radius;
        double factor = dist_to_inner_bounding_sphere / _last_inner_bounding_sphere_radius;
        factor /= 3.0;
        angle *= factor;
        angle *= speed_factor;
        dquat rot = toQuat(angle, axis);
        viewer_pos = -rot * viewer_pos;
        viewer_rot = -rot * viewer_rot;
        _last_ballpos = ballpos;
    }
}

void navigator::planet_distchange_start(const ivec2& pos, const dvec3& viewer_pos, double inner_bounding_sphere_radius)
{
    _state = state_planet_distchange;
    _last_pos = pos;
    _last_inner_bounding_sphere_radius = inner_bounding_sphere_radius;
    _last_dist_to_inner_bounding_sphere = length(viewer_pos) - _last_inner_bounding_sphere_radius;
}

void navigator::planet_distchange(double speed_factor, const ivec2& pos, dvec3& viewer_pos)
{
    if (_state == state_planet_distchange) {
        double distchange_per_pixel = 0.1 + _last_dist_to_inner_bounding_sphere / _viewport[3];
        double offset = (_last_pos.y - pos.y) * distchange_per_pixel;
        offset *= speed_factor;
        viewer_pos += offset * normalize(viewer_pos);
        _last_pos = pos;
        _last_dist_to_inner_bounding_sphere = length(viewer_pos) - _last_inner_bounding_sphere_radius;
    }
}

void navigator::viewer_rot_start(const ivec2& pos)
{
    _state = state_viewer_rot;
    _last_pos = pos;
}

void navigator::viewer_rot(double speed_factor, const ivec2& pos, dquat& viewer_rot)
{
    if (_state == state_viewer_rot) {
        dvec3 look_dir = viewer_rot * dvec3(0.0, 0.0, -1.0);
        dvec3 up_dir = viewer_rot * dvec3(0.0, 1.0, 0.0);
        dvec3 up_down_axis = cross(look_dir, up_dir);
        double up_down_angle = radians(45.0) * (_last_pos.y - pos.y) / static_cast<double>(_viewport[3]);
        up_down_angle *= speed_factor;
        dvec3 left_right_axis = up_dir;
        double left_right_angle = radians(45.0) * (_last_pos.x - pos.x) / static_cast<double>(_viewport[2]);
        left_right_angle *= speed_factor;
        viewer_rot = toQuat(up_down_angle, up_down_axis) * toQuat(left_right_angle, left_right_axis) * viewer_rot;
        _last_pos = pos;
    }
}

void navigator::viewer_shift(double speed_factor, double wheel_radians, double inner_bounding_sphere_radius,
        const dquat& viewer_rot, dvec3& viewer_pos)
{
    dvec3 look_dir = viewer_rot * dvec3(0.0, 0.0, -1.0);
    double shift_per_degree = 50000.0 * (length(viewer_pos) - inner_bounding_sphere_radius) / inner_bounding_sphere_radius;
    viewer_pos += speed_factor * shift_per_degree * wheel_radians / (2.0 * const_pi<double>()) * look_dir;
}

void navigator::surface_object_move_start(const ivec2& pos, const dvec3& viewer_pos, double inner_bounding_sphere_radius)
{
    _state = state_surface_object_move;
    _last_pos = pos;
    _last_inner_bounding_sphere_radius = inner_bounding_sphere_radius;
    _last_dist_to_inner_bounding_sphere = length(viewer_pos) - _last_inner_bounding_sphere_radius;
}

void navigator::surface_object_move(double speed_factor, const ivec2& pos, dvec2& object_latlon)
{
    if (_state == state_surface_object_move) {
        dvec2 change = dvec2(_last_pos - pos) / dvec2(_viewport[2], _viewport[3]);
        double t = _last_dist_to_inner_bounding_sphere / _last_inner_bounding_sphere_radius;
        dvec2 change_per_pixel = dvec2(sqrt(t)) / dvec2(_viewport[2], _viewport[3]);
        dvec2 offset = speed_factor * change * change_per_pixel;
        object_latlon += dvec2(offset.y, -offset.x);
        _last_pos = pos;
    }
}

void navigator::surface_object_scale(double speed_factor, double wheel_radians, 
        const dvec3& viewer_pos, double inner_bounding_sphere_radius,
        double& object_size)
{
    double change_per_degree = 5000.0 * (length(viewer_pos) - inner_bounding_sphere_radius) / inner_bounding_sphere_radius;
    object_size += speed_factor * wheel_radians / (2.0 * const_pi<double>()) * change_per_degree;
}

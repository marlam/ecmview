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

#include "glvm-str.h"

#include "dbg.h"
#include "msg.h"
#include "compiler.h"

#include "lod.h"

using namespace glvm;


ecm_side_quadtree::ecm_side_quadtree(const class ecm& ecm, int side) :
    _parent(NULL), _children{ NULL, NULL, NULL, NULL }
{
    set_data(ecm, ivec4(side, 0, 0, 0));
}

ecm_side_quadtree::ecm_side_quadtree(const class ecm& ecm, const ecm_side_quadtree* parent, int side, int level, int x, int y) :
    _parent(parent), _children{ NULL, NULL, NULL, NULL }
{
    set_data(ecm, ivec4(side, level, x, y));
}

ecm_side_quadtree::~ecm_side_quadtree()
{
    delete _children[0];
    delete _children[1];
    delete _children[2];
    delete _children[3];
}

void ecm_side_quadtree::set_data(const class ecm& ecm, const ivec4& quad)
{
    assert(ecm.is_valid());
    _side = quad[0];
    _level = quad[1];
    _x = quad[2];
    _y = quad[3];
    _max_dist_to_quad_plane = 0.0;
    _max_dist_to_quad_plane_is_valid = false;
    _min_elev = +std::numeric_limits<float>::max();
    _max_elev = -std::numeric_limits<float>::max();
    for (int i = 0; i < 4; i++) {
        dvec2 corner_ecm;
        ecm::quad_to_ecm(_side, _level, _x, _y, i, &(corner_ecm[0]), &(corner_ecm[1]));
        dvec2 corner_geod;
        ecm.ecm_to_geodetic(corner_ecm[0], corner_ecm[1], &(corner_geod[0]), &(corner_geod[1]));
        ecm.geodetic_to_cartesian(corner_geod[0], corner_geod[1], 0.0, _corner_cart[i].vl);
        assert(length(_corner_cart[i]) >= ecm.semi_minor_axis() - 1e-5);
        assert(length(_corner_cart[i]) <= ecm.semi_major_axis() + 1e-5);
        ecm.geodetic_normal(corner_geod[0], corner_geod[1], _corner_en[i].vl);
        assert(length(_corner_en[i]) >= 1.0 - 1e-8 && length(_corner_en[i]) <= 1.0 + 1e-8);
    }
    ecm.quad_plane(_side, _level, _x, _y,
            _corner_cart[0].vl, _corner_cart[1].vl, _corner_cart[2].vl, _corner_cart[3].vl,
            _plane_normal.vl, &_plane_distance);
}

void ecm_side_quadtree::compute_bounding_box()
{
    // The quad plane Q is given by Q: _plane_normal * x = _plane_distance

    // The inner bounding box plane I is given by shifting the quad plane
    // using the minimum elevation along one of the corner ellipsoid normals.
    // I: quad_plane_normal * x = dI
    double _dI[4];
    for (int i = 0; i < 4; i++) {
        dvec3 p = _corner_cart[i] + _corner_en[i] * static_cast<double>(_min_elev);
        _dI[i] = dot(_plane_normal, p);
    }
    double dI = min(_dI[0], _dI[1], _dI[2], _dI[3]);
    // The outer bounding box plane O is given by shifting the quad plane
    // using the maximum elevation and the maximum base data offset.
    // O: quad_plane_normal * x = dO
    double dO = _plane_distance + _max_dist_to_quad_plane + _max_elev;
    // The outer corners of the bounding box are given by the intersection
    // of O with the lines L defined by the ellipsoid normals e at the quad corners C.
    // L: x = C + t * e
    for (int i = 0; i < 4; i++) {
        double t = (dO - dot(_plane_normal, _corner_cart[i])) / dot(_plane_normal, _corner_en[i]);
        _bounding_box_outer[i] = _corner_cart[i] + t * _corner_en[i];
    }
    // The inner corners of the bounding box are given by projecting the outer
    // corners onto the plane I.
    for (int i = 0; i < 4; i++) {
        double t = (dI - dot(_plane_normal, _bounding_box_outer[i])) / dot(_plane_normal, _plane_normal);
        _bounding_box_inner[i] = _bounding_box_outer[i] + t * _plane_normal;
    }
}


ecm_quadtree::ecm_quadtree(const class ecm& ecm) :
    _ecm(ecm)
{
    for (int i = 0; i < 6; i++) {
        _side_roots[i] = new ecm_side_quadtree(_ecm, i);
    }
}

ecm_quadtree::~ecm_quadtree()
{
    for (int i = 0; i < 6; i++) {
        delete _side_roots[i];
    }
}

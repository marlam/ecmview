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

#ifndef LOD_H
#define LOD_H

#include <ecm/ecm.h>
#include <ecm/ecmdb.h>

#include "blob.h"


class ecm_side_quadtree
{
public:
    // Configuration:
    static const int max_level = ecmdb::max_levels - 1;   // Do not split levels >= 30 because that will overflow (x,y)

private:
    /* Tree information */
    const ecm_side_quadtree* _parent;
    ecm_side_quadtree* _children[4];

    /* Fixed quad data */
    glvm::dvec3 _corner_cart[4];        // The geocentric cartesian coordinates of the quad corners
    glvm::dvec3 _corner_en[4];          // The ellipsoid surface normals at the quad corners
    glvm::dvec3 _plane_normal;          // The ellipsoid surface normal at the quad center, i.e. the quad plane normal
    double _plane_distance;             // The quad plane distance to the origin
    // Base data
    double _max_dist_to_quad_plane;     // The max_dist_to_quad info from ecm_quad_base_data
    bool _max_dist_to_quad_plane_is_valid;      // False as long as the definitive value is not yet known
    // Quad coordinates
    int _x, _y;                         // Quad coordinates
    signed char _side;                  // Quad side
    signed char _level;                 // Quad level
    /* Volatile quad data (valid only for the current frame) */
    float _min_elev;                    // Minimum elevation value
    float _max_elev;                    // Maximum elevation value
    glvm::dvec3 _bounding_box_inner[4]; // The four inner points of the bounding box
    glvm::dvec3 _bounding_box_outer[4]; // The four outer points of the bounding box
    int _lens_status;                   // 0 = outside, 1 = inside, 2 = intersect. Only available for quads marked for rendering.

    ecm_side_quadtree(const class ecm& ecm, const ecm_side_quadtree* parent, int side, int level, int x, int y);
    void set_data(const class ecm& ecm, const glvm::ivec4& quad);

    // Split this node.
    void split(const class ecm& ecm)
    {
        _children[0] = new ecm_side_quadtree(ecm, this, side(), level() + 1, 2 * x()    , 2 * y()    );
        _children[1] = new ecm_side_quadtree(ecm, this, side(), level() + 1, 2 * x() + 1, 2 * y()    );
        _children[2] = new ecm_side_quadtree(ecm, this, side(), level() + 1, 2 * x()    , 2 * y() + 1);
        _children[3] = new ecm_side_quadtree(ecm, this, side(), level() + 1, 2 * x() + 1, 2 * y() + 1);
    }

    // Merge this node. Removes the children (and their children recursively).
    void merge()
    {
        for (int i = 0; i < 4; i++) {
            delete _children[i];
            _children[i] = NULL;
        }
    }

public:
    ecm_side_quadtree(const class ecm& ecm, int side);
    ~ecm_side_quadtree();

    // Compute the bounding box.
    void compute_bounding_box();

    /* Member access */

    const ecm_side_quadtree* parent() const
    {
        return _parent;
    }

    bool has_children() const
    {
        return _children[0];
    }

    const ecm_side_quadtree* child(int c) const
    {
        return _children[c];
    }

    ecm_side_quadtree* child(int c)
    {
        return _children[c];
    }

    int side() const
    {
        return _side;
    }

    int level() const
    {
        return _level;
    }

    int x() const
    {
        return _x;
    }

    int y() const
    {
        return _y;
    }

    glvm::ivec4 quad() const
    {
        return glvm::ivec4(side(), level(), x(), y());
    }

    /* Manipulation */

    // Get/set render properties.
    float min_elev() const
    {
        return _min_elev;
    }
    float& min_elev()
    {
        return _min_elev;
    }
    float max_elev() const
    {
        return _max_elev;
    }
    float& max_elev()
    {
        return _max_elev;
    }
    const glvm::dvec3& corner(int i) const
    {
        return _corner_cart[i];
    }
    const glvm::dvec3* corners() const
    {
        return _corner_cart;
    }
    const glvm::dvec3& plane_normal() const
    {
        return _plane_normal;
    }
    double plane_distance() const
    {
        return _plane_distance;
    }
    double max_dist_to_quad_plane() const
    {
        return _max_dist_to_quad_plane;
    }
    double& max_dist_to_quad_plane()
    {
        return _max_dist_to_quad_plane;
    }
    bool max_dist_to_quad_plane_is_valid() const
    {
        return _max_dist_to_quad_plane_is_valid;
    }
    bool& max_dist_to_quad_plane_is_valid()
    {
        return _max_dist_to_quad_plane_is_valid;
    }
    const glvm::dvec3* bounding_box_inner() const
    {
        return _bounding_box_inner;
    }
    const glvm::dvec3* bounding_box_outer() const
    {
        return _bounding_box_outer;
    }
    int lens_status() const
    {
        return _lens_status;
    }
    int& lens_status()
    {
        return _lens_status;
    }

    friend class ecm_quadtree;
};

class ecm_quadtree
{
private:
    const class ecm _ecm;
    ecm_side_quadtree* _side_roots[6];

public:
    ecm_quadtree(const class ecm& ecm);
    ~ecm_quadtree();

    const class ecm& ecm() const
    {
        return _ecm;
    }

    ecm_side_quadtree* side_root(int side)
    {
        return _side_roots[side];
    }

    /* Manipulation */

    void split(ecm_side_quadtree* node)
    {
        node->split(_ecm);
    }

    void merge(ecm_side_quadtree* node)
    {
        node->merge();
    }
};

#endif

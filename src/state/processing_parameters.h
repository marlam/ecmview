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

#ifndef PROCESSING_PARAMETERS_H
#define PROCESSING_PARAMETERS_H

#include <ecm/ecmdb.h>

#include "s11n.h"


class processing_parameters : public serializable
{
public:
    bool category_e2c;          // if this is set, then the special category e2c applies, and the category field is ignored.
    ecmdb::category_t category; // database category

    static const int max_gradient_length = 256;

    union {
        /* ecmdb::category_elevation */
        struct {
            float scale_factor;
            float scale_center;
        } elevation;

        /* ecmdb::category_texture */
        struct {
            float contrast;
            float brightness;
            float saturation;
            float hue;
        } texture;

        /* ecmdb::category_data */
        struct {
            float offset;
            float factor;
            int gradient_length;
            uint8_t gradient[max_gradient_length * 3];
        } data;

        /* SPECIAL: elevation -> color (e2c) */
        struct {
            int gradient_length;
            uint8_t gradient[max_gradient_length * 3];
            bool adapt_brightness;
            float isolines_distance;    // <= 0.0: no isolines
            float isolines_thickness;
            uint8_t isolines_color[3];
        } e2c;
    };

public:
    processing_parameters(ecmdb::category_t category);
    processing_parameters(bool category_e2c = false, ecmdb::category_t category = ecmdb::category_invalid);

    void reset(bool category_e2c = false, ecmdb::category_t category = ecmdb::category_invalid);

    void save(std::ostream& os) const;
    void load(std::istream& is);

    void save(std::ostream& os, const char* name) const;
    void load(const std::string& s);
};

#endif

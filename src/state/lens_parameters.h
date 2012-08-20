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

#ifndef LENS_PARAMETERS_H
#define LENS_PARAMETERS_H

#include "glvm.h"

#include "s11n.h"


class lens_parameters : public serializable
{
public:
    bool active;
    glvm::dvec2 pos;     // geodetic lat/lon relative to current ellipsoid
    double radius;

private:
    void reset();

public:
    lens_parameters();

    void save(std::ostream& os) const;
    void load(std::istream& is);

    void save(std::ostream& os, const char* name) const;
    void load(const std::string& s);
};

#endif

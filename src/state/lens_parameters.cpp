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

#include "glvm-s11n.h"

#include "lens_parameters.h"


lens_parameters::lens_parameters()
{
    reset();
}

void lens_parameters::reset()
{
    active = false;
    pos = glvm::dvec2(0.0, 0.0);
    radius = 0.0f;
}

void lens_parameters::save(std::ostream& os) const
{
    s11n::save(os, active);
    s11n::save(os, pos);
    s11n::save(os, radius);
}

void lens_parameters::load(std::istream &is)
{
    s11n::load(is, active);
    s11n::load(is, pos);
    s11n::load(is, radius);
}

void lens_parameters::save(std::ostream& os, const char* name) const
{
    s11n::startgroup(os, name);
    s11n::save(os, "active", active);
    s11n::save(os, "pos", pos);
    s11n::save(os, "radius", radius);
    s11n::endgroup(os);
}

void lens_parameters::load(const std::string& s)
{
    reset();
    std::istringstream iss(s);
    std::string name, value;
    while (iss.good()) {
        s11n::load(iss, name, value);
        if (name == "active") {
            s11n::load(value, active);
        } else if (name == "pos") {
            s11n::load(value, pos);
        } else if (name == "radius") {
            s11n::load(value, radius);
        }
    }
}

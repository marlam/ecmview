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

#include "glvm-s11n.h"

#include "light_parameters.h"


light_parameters::light_parameters()
{
    reset();
}

void light_parameters::reset()
{
    active = false;
    ambient = glvm::vec3(0.0f);
    dir = glvm::vec3(0.0f, 1.0f, 0.0f);
    color = glvm::vec3(1.0f);
    shininess = 0.0f;
}

void light_parameters::save(std::ostream& os) const
{
    s11n::save(os, active);
    s11n::save(os, ambient);
    s11n::save(os, dir);
    s11n::save(os, color);
    s11n::save(os, shininess);
}

void light_parameters::load(std::istream &is)
{
    s11n::load(is, active);
    s11n::load(is, ambient);
    s11n::load(is, dir);
    s11n::load(is, color);
    s11n::load(is, shininess);
}

void light_parameters::save(std::ostream& os, const char* name) const
{
    s11n::startgroup(os, name);
    s11n::save(os, "active", active);
    s11n::save(os, "ambient", ambient);
    s11n::save(os, "dir", dir);
    s11n::save(os, "color", color);
    s11n::save(os, "shininess", shininess);
    s11n::endgroup(os);
}

void light_parameters::load(const std::string& s)
{
    reset();
    std::istringstream iss(s);
    std::string name, value;
    while (iss.good()) {
        s11n::load(iss, name, value);
        if (name == "active") {
            s11n::load(value, active);
        } else if (name == "ambient") {
            s11n::load(value, ambient);
        } else if (name == "dir") {
            s11n::load(value, dir);
        } else if (name == "color") {
            s11n::load(value, color);
        } else if (name == "shininess") {
            s11n::load(value, shininess);
        }
    }
}

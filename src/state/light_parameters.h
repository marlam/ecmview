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

#ifndef LIGHT_PARAMETERS_H
#define LIGHT_PARAMETERS_H

#include "glvm.h"

#include "ser.h"


class light_parameters : public serializable
{
public:
    bool active;
    glvm::vec3 ambient;
    glvm::vec3 dir;     // normalized direction from (0,0,0) to light
    glvm::vec3 color;
    float shininess;    // 0 means no specular highlight, 1 means full specular highlight

private:
    void reset();

public:
    light_parameters();

    void save(std::ostream& os) const;
    void load(std::istream& is);

    void save(std::ostream& os, const char* name) const;
    void load(const std::string& s);
};

#endif

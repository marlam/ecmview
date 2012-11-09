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

#ifndef DATABASE_DESCRIPTION_H
#define DATABASE_DESCRIPTION_H

#include <string>

#include <ecmdb/ecmdb.h>

#include "s11n.h"

#include "uuid.h"
#include "processing_parameters.h"


class database_description: public serializable
{
public:
    std::string url;
    std::string username;
    std::string password;
    class uuid uuid;
    class ecmdb db;
    class ecmdb::metadata meta;
    // indices: 0=global, 1=lens
    bool active[2];
    int priority[2];
    float weight[2];
    class processing_parameters processing_parameters[2];

public:
    database_description();

    void save(std::ostream& os) const;
    void load(std::istream& is);
};

#endif

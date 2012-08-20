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

#include "database_description.h"


// Make ecmdb and its subclasses serializable:
namespace s11n
{
    void save(std::ostream& os, const ecmdb& x) { x.save(os); }
    void load(std::istream& is, ecmdb& x) { x.load(is); }
    void save(std::ostream& os, const ecmdb::global_metadata& x) { x.save(os); }
    void load(std::istream& is, ecmdb::global_metadata& x) { x.load(is); }
}


database_description::database_description() :
    active({ false, false }), priority({ 0, 0 }), weight({ 0.0f, 0.0f })
{
}

void database_description::save(std::ostream& os) const
{
    s11n::save(os, url);
    s11n::save(os, username);
    s11n::save(os, password);
    s11n::save(os, uuid);
    s11n::save(os, db);
    s11n::save(os, meta);
    s11n::save(os, active[0]);
    s11n::save(os, active[1]);
    s11n::save(os, priority[0]);
    s11n::save(os, priority[1]);
    s11n::save(os, weight[0]);
    s11n::save(os, weight[1]);
    s11n::save(os, processing_parameters[0]);
    s11n::save(os, processing_parameters[1]);
}

void database_description::load(std::istream& is)
{
    s11n::load(is, url);
    s11n::load(is, username);
    s11n::load(is, password);
    s11n::load(is, uuid);
    s11n::load(is, db);
    s11n::load(is, meta);
    s11n::load(is, active[0]);
    s11n::load(is, active[1]);
    s11n::load(is, priority[0]);
    s11n::load(is, priority[1]);
    s11n::load(is, weight[0]);
    s11n::load(is, weight[1]);
    s11n::load(is, processing_parameters[0]);
    s11n::load(is, processing_parameters[1]);
}

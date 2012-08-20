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

#include <cmath>

#include "glvm.h"
#include "xgl.h"

#include "data_processor.h"

using namespace glvm;


data_processor::data_processor()
{
}

void data_processor::init_gl()
{
}

void data_processor::exit_gl()
{
}

bool data_processor::processing_is_necessary(
        unsigned int /* frame */,
        const database_description& /* dd */, bool /* lens */,
        const glvm::ivec4& /* quad */,
        const ecmdb::quad_metadata& /* quad_meta */)
{
    return true;
}

void data_processor::process(
        unsigned int frame,
        const database_description& dd, bool lens,
        const glvm::ivec4& quad,
        const ecmdb::quad_metadata& quad_meta,
        bool* full_validity,
        ecmdb::quad_metadata* meta)
{
    assert(false);
}

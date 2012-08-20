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

#ifndef DATA_PROCESSOR_H
#define DATA_PROCESSOR_H

#include "../processor.h"

class data_processor : public dbcategory_processor
{
public:
    data_processor();

    virtual void init_gl();
    virtual void exit_gl();

    virtual bool processing_is_necessary(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::quad_metadata& quad_meta);
    virtual void process(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::quad_metadata& quad_meta,
            bool* full_validity,
            ecmdb::quad_metadata* meta);
};

#endif

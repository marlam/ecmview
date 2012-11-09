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

#ifndef ELEVATION_PROCESSOR_H
#define ELEVATION_PROCESSOR_H

#include "../processor.h"

class elevation_processor : public dbcategory_processor
{
private:
    float _min_elev;
    float _max_elev;
    GLuint _prg;
    GLint _data_offset_loc;
    GLint _data_factor_loc;
    GLint _scale_factor_loc;
    GLint _scale_center_loc;
    GLint _min_elev_loc;
    GLint _max_elev_loc;

public:
    elevation_processor();

    virtual void init_gl();
    virtual void exit_gl();

    void set_elevation_bounds(float min_elev, float max_elev);
    void get_processed_quad_elevation_bounds(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& /* quad */,
            const ecmdb::metadata& quad_meta,
            float* processed_quad_min_elev, float* processed_quad_max_elev) const;

    virtual bool processing_is_necessary(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta);
    virtual void process(
            unsigned int frame,
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            bool* full_validity,
            ecmdb::metadata* meta);
};

#endif

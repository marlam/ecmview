/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

#ifndef DRR_H
#define DRR_H

/*
 * Literature:
 *
 * M. Lambers, H. Nies, A. Kolb
 * Interactive Dynamic Range Reduction for SAR Images
 * In Geoscience and Remote Sensing Letters, 5(3), 2008, pages 507-511
 *
 * M. Lambers, A. Kolb
 * Adaptive Dynamic Range Reduction for SAR Images
 * In Proc. 7th European Conference on Synthetic Aperture Radar (EUSAR), 3, 2008, pages 371-374
 */


#include <GL/glew.h>

#include "sub-processor.h"


class DynamicRangeReductionLinear : public SubProcessor
{
private:
    GLuint _prgs[1];

public:
    DynamicRangeReductionLinear();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DynamicRangeReductionLog : public SubProcessor
{
private:
    GLuint _prgs[1];

public:
    DynamicRangeReductionLog();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DynamicRangeReductionGamma : public SubProcessor
{
private:
    GLuint _prgs[1];

public:
    DynamicRangeReductionGamma();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DynamicRangeReductionSchlick : public SubProcessor
{
private:
    GLuint _prgs[1];

public:
    DynamicRangeReductionSchlick();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DynamicRangeReductionReinhard : public SubProcessor
{
private:
    GLuint _prgs[1];

public:
    DynamicRangeReductionReinhard();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DynamicRangeReductionSchlickLocal : public SubProcessor
{
private:
    GLuint _prgs[3];
    int _k[3];
    float* _mask[3];
    float _weightsum[3];

public:
    DynamicRangeReductionSchlickLocal();
    ~DynamicRangeReductionSchlickLocal();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DynamicRangeReductionReinhardLocal : public SubProcessor
{
private:
    GLuint _prgs[3];
    int _k[3];
    float* _mask[3];
    float _weightsum[3];

public:
    DynamicRangeReductionReinhardLocal();
    ~DynamicRangeReductionReinhardLocal();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

#endif

/*
 * despeckling.h
 *
 * This file is part of plew, a planet viewer.
 *
 * Copyright (C) 2006, 2007, 2008, 2009, 2010
 * Computer Graphics Group, University of Siegen, Germany.
 * See http://www.cg.informatik.uni-siegen.de/ for contact information.
 *
 * All rights reserved.
 *
 * Written by Martin Lambers <lambers@fb12.uni-siegen.de>.
 */

#ifndef DESPECKLING_H
#define DESPECKLING_H


#include <GL/glew.h>

#include "sub-processor.h"


/*
 * Literature:
 *
 * Kuan, Lee, Gamma:
 * L. Gagnon, A. Jouan
 * Speckle Filtering of SAR Images -- A Comparative Study Between
 * Complex-Wavelet-Based and Standard Filters
 * Proceedings of SPIE -- Volume 3169, October 1997, pp. 80-91
 *
 * Gamma:
 * http://www.pcigeomatics.com/cgi-bin/pcihlp/IWORKS|Filter|Gamma+Filter
 *
 * Xiao:
 * J. Xiao, J. Li, A. Moody
 * A detail-preserving and flexible adaptive filter for speckle suppression in
 * SAR imagery
 * Int. J. Remote Sensing, 2003, vol. 24, no. 12, 2451-2465.
 */

class DespecklingMean : public SubProcessor
{
private:
    GLuint _prgs[2];
    int _kh, _kv;

public:
    DespecklingMean();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DespecklingMedian : public SubProcessor
{
private:
    GLuint _prgs[2];
    int _kh, _kv;

public:
    DespecklingMedian();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DespecklingGauss : public SubProcessor
{
private:
    GLuint _prgs[2];
    int _kh, _kv;
    float* _mask_h;
    float* _mask_v;
    float _weightsum_h;
    float _weightsum_v;

public:
    DespecklingGauss();
    virtual ~DespecklingGauss();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DespecklingLee : public SubProcessor
{
private:
    GLuint _prgs[3];
    int _kh, _kv;

public:
    DespecklingLee();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DespecklingKuan : public SubProcessor
{
private:
    GLuint _prgs[3];
    int _kh, _kv;

public:
    DespecklingKuan();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DespecklingXiao : public SubProcessor
{
private:
    GLuint _prgs[3];
    int _kh, _kv;

public:
    DespecklingXiao();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DespecklingFrost : public SubProcessor
{
private:
    GLuint _prgs[3];
    int _kh, _kv;

public:
    DespecklingFrost();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DespecklingGammaMAP : public SubProcessor
{
private:
    GLuint _prgs[4];
    int _kh, _kv;

public:
    DespecklingGammaMAP();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DespecklingOddy : public SubProcessor
{
private:
    GLuint _prgs[1];
    int _kh, _kv;

public:
    DespecklingOddy();
    virtual void init_gl();
    virtual void exit_gl();
    virtual GLuint apply(
            const database_description& dd, bool lens,
            const glvm::ivec4& quad,
            const ecmdb::metadata& quad_meta,
            const GLuint src_tex,
            const GLuint pingpong[2]);
};

class DespecklingWaveletST : public SubProcessor
{
private:
    GLuint _prgs[5];

public:
    DespecklingWaveletST();
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

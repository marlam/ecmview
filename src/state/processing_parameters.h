/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013
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

#ifndef PROCESSING_PARAMETERS_H
#define PROCESSING_PARAMETERS_H

#include <ecmdb/ecmdb.h>

#include "ser.h"


class processing_parameters : public serializable
{
public:
    bool category_e2c;          // if this is set, then the special category e2c applies, and the category field is ignored.
    ecmdb::category_t category; // database category

    static const int max_gradient_length = 256;

    /* enumerations */
    typedef enum {
        sar_amplitude_despeckling_none,
        sar_amplitude_despeckling_mean,
        sar_amplitude_despeckling_median,
        sar_amplitude_despeckling_gauss,
        sar_amplitude_despeckling_lee,
        sar_amplitude_despeckling_kuan,
        sar_amplitude_despeckling_xiao,
        sar_amplitude_despeckling_frost,
        sar_amplitude_despeckling_gammamap,
        sar_amplitude_despeckling_oddy,
        sar_amplitude_despeckling_waveletst
    } despeckling_method_t;
    typedef enum {
        sar_amplitude_drr_linear,
        sar_amplitude_drr_log,
        sar_amplitude_drr_gamma,
        sar_amplitude_drr_schlick,
        sar_amplitude_drr_reinhard,
        sar_amplitude_drr_schlicklocal,
        sar_amplitude_drr_reinhardlocal
    } drr_method_t;

    /* parameters */
    union {
        /* ecmdb::category_elevation */
        struct {
            float scale_factor;
            float scale_center;
        } elevation;

        /* ecmdb::category_texture */
        struct {
            float contrast;
            float brightness;
            float saturation;
            float hue;
        } texture;

        /* ecmdb::category_sar_amplitude */
        struct {
            int despeckling_method;
            struct {
                struct {
                    int kh, kv;
                } mean;
                struct {
                    int kh, kv;
                } median;
                struct {
                    int kh, kv;
                    float sh, sv;
                } gauss;
                struct {
                    int kh, kv;
                    float sigma_n;
                } lee;
                struct {
                    int kh, kv;
                    float L;
                } kuan;
                struct {
                    int kh, kv;
                    float Tmin, Tmax;
                    float a, b;
                } xiao;
                struct {
                    int kh, kv;
                    float a;
                } frost;
                struct {
                    int kh, kv;
                    float L;
                } gammamap;
                struct {
                    int kh, kv;
                    float alpha;
                } oddy;
                struct {
                    float threshold;
                } waveletst;
            } despeckling;
            int drr_method;
            struct {
                struct {
                    float min_amp, max_amp;
                } linear;
                struct {
                    float min_amp, max_amp;
                    float prescale;
                } log;
                struct {
                    float min_amp, max_amp;
                    float gamma;
                } gamma;
                struct {
                    float brightness;
                } schlick;
                struct {
                    float brightness, contrast;
                } reinhard;
                struct {
                    float brightness;
                    float details;
                    float threshold;
                } schlicklocal;
                struct {
                    float brightness, contrast;
                    float details;
                    float threshold;
                } reinhardlocal;
            } drr;
            int gradient_length;
            uint8_t gradient[max_gradient_length * 3];
            bool adapt_brightness;
        } sar_amplitude;

        /* ecmdb::category_data */
        struct {
            float offset;
            float factor;
            int gradient_length;
            uint8_t gradient[max_gradient_length * 3];
        } data;

        /* SPECIAL: elevation -> color (e2c) */
        struct {
            int gradient_length;
            uint8_t gradient[max_gradient_length * 3];
            bool adapt_brightness;
            float isolines_distance;    // <= 0.0: no isolines
            float isolines_thickness;
            uint8_t isolines_color[3];
        } e2c;
    };

public:
    processing_parameters(ecmdb::category_t category);
    processing_parameters(bool category_e2c = false, ecmdb::category_t category = ecmdb::category_invalid);

    void reset(bool category_e2c = false, ecmdb::category_t category = ecmdb::category_invalid);

    void save(std::ostream& os) const;
    void load(std::istream& is);

    void save(std::ostream& os, const char* name) const;
    void load(const std::string& s);
};

#endif

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

#include "config.h"

#include <sstream>
#include <cstring>

#include "msg.h"
#include "dbg.h"

#include "glvm-ser.h"

#include "processing_parameters.h"

using namespace glvm;


processing_parameters::processing_parameters(ecmdb::category_t category)
{
    reset(false, category);
}

processing_parameters::processing_parameters(bool category_e2c, ecmdb::category_t category)
{
    reset(category_e2c, category);
}

void processing_parameters::reset(bool category_e2c, ecmdb::category_t category)
{
    this->category_e2c = category_e2c;
    this->category = category;
    if (category_e2c) {
        e2c.gradient_length = 1;
        e2c.gradient[0] = 0xff;
        e2c.gradient[1] = 0xff;
        e2c.gradient[2] = 0xff;
        e2c.adapt_brightness = false;
        e2c.isolines_distance = 0.0f;
        e2c.isolines_thickness = 1.0f;
        e2c.isolines_color[0] = 0;
        e2c.isolines_color[1] = 0;
        e2c.isolines_color[2] = 0;
    } else if (category == ecmdb::category_elevation) {
        elevation.scale_factor = 1.0f;
        elevation.scale_center = 0.0f;
    } else if (category == ecmdb::category_texture) {
        texture.contrast = 0.0f;
        texture.brightness = 0.0f;
        texture.saturation = 0.0f;
        texture.hue = 0.0f;
    } else if (category == ecmdb::category_sar_amplitude) {
        sar_amplitude.despeckling_method = sar_amplitude_despeckling_none;
        sar_amplitude.despeckling.mean.kh = 1;
        sar_amplitude.despeckling.mean.kv = 1;
        sar_amplitude.despeckling.median.kh = 1;
        sar_amplitude.despeckling.median.kv = 1;
        sar_amplitude.despeckling.gauss.kh = 1;
        sar_amplitude.despeckling.gauss.kv = 1;
        sar_amplitude.despeckling.gauss.sh = 0.1f;
        sar_amplitude.despeckling.gauss.sv = 0.1f;
        sar_amplitude.despeckling.lee.kh = 1;
        sar_amplitude.despeckling.lee.kv = 1;
        sar_amplitude.despeckling.lee.sigma_n = 1.0f;
        sar_amplitude.despeckling.kuan.kh = 1;
        sar_amplitude.despeckling.kuan.kv = 1;
        sar_amplitude.despeckling.kuan.L = 1.0f;
        sar_amplitude.despeckling.xiao.kh = 1;
        sar_amplitude.despeckling.xiao.kv = 1;
        sar_amplitude.despeckling.xiao.Tmin = -5.0f;
        sar_amplitude.despeckling.xiao.Tmax = +5.0f;
        sar_amplitude.despeckling.xiao.a = 0.0f;
        sar_amplitude.despeckling.xiao.b = 1.0f;
        sar_amplitude.despeckling.frost.kh = 1;
        sar_amplitude.despeckling.frost.kv = 1;
        sar_amplitude.despeckling.frost.a = 0.0f;
        sar_amplitude.despeckling.gammamap.kh = 1;
        sar_amplitude.despeckling.gammamap.kv = 1;
        sar_amplitude.despeckling.gammamap.L = 1.0f;
        sar_amplitude.despeckling.oddy.kh = 1;
        sar_amplitude.despeckling.oddy.kv = 1;
        sar_amplitude.despeckling.oddy.alpha = 0.5f;
        sar_amplitude.despeckling.waveletst.threshold = 0.0f;
        sar_amplitude.drr_method = sar_amplitude_drr_schlick;
        sar_amplitude.drr.linear.min_amp = 0.0f;
        sar_amplitude.drr.linear.max_amp = 1.0f;
        sar_amplitude.drr.log.min_amp = 0.0f;
        sar_amplitude.drr.log.max_amp = 1.0f;
        sar_amplitude.drr.log.prescale = 1000.0f;
        sar_amplitude.drr.gamma.min_amp = 0.0f;
        sar_amplitude.drr.gamma.max_amp = 0.1f;
        sar_amplitude.drr.gamma.gamma = 2.5f;
        sar_amplitude.drr.schlick.brightness = 50.0f;
        sar_amplitude.drr.reinhard.brightness = 0.0f;
        sar_amplitude.drr.reinhard.contrast = 0.5f;
        sar_amplitude.drr.schlicklocal.brightness = 0.0f;
        sar_amplitude.drr.schlicklocal.details = 0.5f;
        sar_amplitude.drr.schlicklocal.threshold = 0.1f;
        sar_amplitude.drr.reinhardlocal.brightness = 0.0f;
        sar_amplitude.drr.reinhardlocal.contrast = 0.5f;
        sar_amplitude.drr.reinhardlocal.details = 0.5f;
        sar_amplitude.drr.reinhardlocal.threshold = 0.1f;
        sar_amplitude.gradient_length = 1;
        sar_amplitude.gradient[0] = 0xff;
        sar_amplitude.gradient[1] = 0xff;
        sar_amplitude.gradient[2] = 0xff;
        sar_amplitude.adapt_brightness = true;
    } else if (category == ecmdb::category_data) {
        data.offset = 0.0f;
        data.factor = 1.0f;
        data.gradient_length = 1;
        data.gradient[0] = 0xff;
        data.gradient[1] = 0xff;
        data.gradient[2] = 0xff;
    } else {
        assert(category == ecmdb::category_invalid);
    }
}

void processing_parameters::save(std::ostream &os) const
{
    s11n::save(os, category_e2c);
    s11n::save(os, static_cast<int>(category));
    if (category_e2c) {
        s11n::save(os, e2c.gradient_length);
        for (int i = 0; i < e2c.gradient_length; i++) {
            s11n::save(os, e2c.gradient[3 * i + 0]);
            s11n::save(os, e2c.gradient[3 * i + 1]);
            s11n::save(os, e2c.gradient[3 * i + 2]);
        }
        s11n::save(os, e2c.adapt_brightness);
        s11n::save(os, e2c.isolines_distance);
        s11n::save(os, e2c.isolines_thickness);
        s11n::save(os, e2c.isolines_color[0]);
        s11n::save(os, e2c.isolines_color[1]);
        s11n::save(os, e2c.isolines_color[2]);
    } else if (category == ecmdb::category_elevation) {
        s11n::save(os, elevation.scale_factor);
        s11n::save(os, elevation.scale_center);
    } else if (category == ecmdb::category_texture) {
        s11n::save(os, texture.contrast);
        s11n::save(os, texture.brightness);
        s11n::save(os, texture.saturation);
        s11n::save(os, texture.hue);
    } else if (category == ecmdb::category_sar_amplitude) {
        s11n::save(os, static_cast<int>(sar_amplitude.despeckling_method));
        s11n::save(os, sar_amplitude.despeckling.mean.kh);
        s11n::save(os, sar_amplitude.despeckling.mean.kv);
        s11n::save(os, sar_amplitude.despeckling.median.kh);
        s11n::save(os, sar_amplitude.despeckling.median.kv);
        s11n::save(os, sar_amplitude.despeckling.gauss.kh);
        s11n::save(os, sar_amplitude.despeckling.gauss.kv);
        s11n::save(os, sar_amplitude.despeckling.gauss.sh);
        s11n::save(os, sar_amplitude.despeckling.gauss.sv);
        s11n::save(os, sar_amplitude.despeckling.lee.kh);
        s11n::save(os, sar_amplitude.despeckling.lee.kv);
        s11n::save(os, sar_amplitude.despeckling.lee.sigma_n);
        s11n::save(os, sar_amplitude.despeckling.kuan.kh);
        s11n::save(os, sar_amplitude.despeckling.kuan.kv);
        s11n::save(os, sar_amplitude.despeckling.kuan.L);
        s11n::save(os, sar_amplitude.despeckling.xiao.kh);
        s11n::save(os, sar_amplitude.despeckling.xiao.kv);
        s11n::save(os, sar_amplitude.despeckling.xiao.Tmin);
        s11n::save(os, sar_amplitude.despeckling.xiao.Tmax);
        s11n::save(os, sar_amplitude.despeckling.xiao.a);
        s11n::save(os, sar_amplitude.despeckling.xiao.b);
        s11n::save(os, sar_amplitude.despeckling.frost.kh);
        s11n::save(os, sar_amplitude.despeckling.frost.kv);
        s11n::save(os, sar_amplitude.despeckling.frost.a);
        s11n::save(os, sar_amplitude.despeckling.gammamap.kh);
        s11n::save(os, sar_amplitude.despeckling.gammamap.kv);
        s11n::save(os, sar_amplitude.despeckling.gammamap.L);
        s11n::save(os, sar_amplitude.despeckling.oddy.kh);
        s11n::save(os, sar_amplitude.despeckling.oddy.kv);
        s11n::save(os, sar_amplitude.despeckling.oddy.alpha);
        s11n::save(os, sar_amplitude.despeckling.waveletst.threshold);
        s11n::save(os, static_cast<int>(sar_amplitude.drr_method));
        s11n::save(os, sar_amplitude.drr.linear.min_amp);
        s11n::save(os, sar_amplitude.drr.linear.max_amp);
        s11n::save(os, sar_amplitude.drr.log.min_amp);
        s11n::save(os, sar_amplitude.drr.log.max_amp);
        s11n::save(os, sar_amplitude.drr.log.prescale);
        s11n::save(os, sar_amplitude.drr.gamma.min_amp);
        s11n::save(os, sar_amplitude.drr.gamma.max_amp);
        s11n::save(os, sar_amplitude.drr.gamma.gamma);
        s11n::save(os, sar_amplitude.drr.schlick.brightness);
        s11n::save(os, sar_amplitude.drr.reinhard.brightness);
        s11n::save(os, sar_amplitude.drr.reinhard.contrast);
        s11n::save(os, sar_amplitude.drr.schlicklocal.brightness);
        s11n::save(os, sar_amplitude.drr.schlicklocal.details);
        s11n::save(os, sar_amplitude.drr.schlicklocal.threshold);
        s11n::save(os, sar_amplitude.drr.reinhardlocal.brightness);
        s11n::save(os, sar_amplitude.drr.reinhardlocal.contrast);
        s11n::save(os, sar_amplitude.drr.reinhardlocal.details);
        s11n::save(os, sar_amplitude.drr.reinhardlocal.threshold);
        s11n::save(os, sar_amplitude.gradient_length);
        for (int i = 0; i < sar_amplitude.gradient_length; i++) {
            s11n::save(os, sar_amplitude.gradient[3 * i + 0]);
            s11n::save(os, sar_amplitude.gradient[3 * i + 1]);
            s11n::save(os, sar_amplitude.gradient[3 * i + 2]);
        }
    } else if (category == ecmdb::category_data) {
        s11n::save(os, data.offset);
        s11n::save(os, data.factor);
        s11n::save(os, data.gradient_length);
        for (int i = 0; i < data.gradient_length; i++) {
            s11n::save(os, data.gradient[3 * i + 0]);
            s11n::save(os, data.gradient[3 * i + 1]);
            s11n::save(os, data.gradient[3 * i + 2]);
        }
    }
}

void processing_parameters::load(std::istream &is)
{
    s11n::load(is, category_e2c);
    int x;
    s11n::load(is, x); category = static_cast<ecmdb::category_t>(x);
    if (category_e2c) {
        s11n::load(is, e2c.gradient_length);
        for (int i = 0; i < e2c.gradient_length; i++) {
            s11n::load(is, e2c.gradient[3 * i + 0]);
            s11n::load(is, e2c.gradient[3 * i + 1]);
            s11n::load(is, e2c.gradient[3 * i + 2]);
        }
        s11n::load(is, e2c.adapt_brightness);
        s11n::load(is, e2c.isolines_distance);
        s11n::load(is, e2c.isolines_thickness);
        s11n::load(is, e2c.isolines_color[0]);
        s11n::load(is, e2c.isolines_color[1]);
        s11n::load(is, e2c.isolines_color[2]);
    } else if (category == ecmdb::category_elevation) {
        s11n::load(is, elevation.scale_factor);
        s11n::load(is, elevation.scale_center);
    } else if (category == ecmdb::category_texture) {
        s11n::load(is, texture.contrast);
        s11n::load(is, texture.brightness);
        s11n::load(is, texture.saturation);
        s11n::load(is, texture.hue);
    } else if (category == ecmdb::category_sar_amplitude) {
        s11n::load(is, sar_amplitude.despeckling_method);
        s11n::load(is, sar_amplitude.despeckling.mean.kh);
        s11n::load(is, sar_amplitude.despeckling.mean.kv);
        s11n::load(is, sar_amplitude.despeckling.median.kh);
        s11n::load(is, sar_amplitude.despeckling.median.kv);
        s11n::load(is, sar_amplitude.despeckling.gauss.kh);
        s11n::load(is, sar_amplitude.despeckling.gauss.kv);
        s11n::load(is, sar_amplitude.despeckling.gauss.sh);
        s11n::load(is, sar_amplitude.despeckling.gauss.sv);
        s11n::load(is, sar_amplitude.despeckling.lee.kh);
        s11n::load(is, sar_amplitude.despeckling.lee.kv);
        s11n::load(is, sar_amplitude.despeckling.lee.sigma_n);
        s11n::load(is, sar_amplitude.despeckling.kuan.kh);
        s11n::load(is, sar_amplitude.despeckling.kuan.kv);
        s11n::load(is, sar_amplitude.despeckling.kuan.L);
        s11n::load(is, sar_amplitude.despeckling.xiao.kh);
        s11n::load(is, sar_amplitude.despeckling.xiao.kv);
        s11n::load(is, sar_amplitude.despeckling.xiao.Tmin);
        s11n::load(is, sar_amplitude.despeckling.xiao.Tmax);
        s11n::load(is, sar_amplitude.despeckling.xiao.a);
        s11n::load(is, sar_amplitude.despeckling.xiao.b);
        s11n::load(is, sar_amplitude.despeckling.frost.kh);
        s11n::load(is, sar_amplitude.despeckling.frost.kv);
        s11n::load(is, sar_amplitude.despeckling.frost.a);
        s11n::load(is, sar_amplitude.despeckling.gammamap.kh);
        s11n::load(is, sar_amplitude.despeckling.gammamap.kv);
        s11n::load(is, sar_amplitude.despeckling.gammamap.L);
        s11n::load(is, sar_amplitude.despeckling.oddy.kh);
        s11n::load(is, sar_amplitude.despeckling.oddy.kv);
        s11n::load(is, sar_amplitude.despeckling.oddy.alpha);
        s11n::load(is, sar_amplitude.despeckling.waveletst.threshold);
        s11n::load(is, sar_amplitude.drr_method);
        s11n::load(is, sar_amplitude.drr.linear.min_amp);
        s11n::load(is, sar_amplitude.drr.linear.max_amp);
        s11n::load(is, sar_amplitude.drr.log.min_amp);
        s11n::load(is, sar_amplitude.drr.log.max_amp);
        s11n::load(is, sar_amplitude.drr.log.prescale);
        s11n::load(is, sar_amplitude.drr.gamma.min_amp);
        s11n::load(is, sar_amplitude.drr.gamma.max_amp);
        s11n::load(is, sar_amplitude.drr.gamma.gamma);
        s11n::load(is, sar_amplitude.drr.schlick.brightness);
        s11n::load(is, sar_amplitude.drr.reinhard.brightness);
        s11n::load(is, sar_amplitude.drr.reinhard.contrast);
        s11n::load(is, sar_amplitude.drr.schlicklocal.brightness);
        s11n::load(is, sar_amplitude.drr.schlicklocal.details);
        s11n::load(is, sar_amplitude.drr.schlicklocal.threshold);
        s11n::load(is, sar_amplitude.drr.reinhardlocal.brightness);
        s11n::load(is, sar_amplitude.drr.reinhardlocal.contrast);
        s11n::load(is, sar_amplitude.drr.reinhardlocal.details);
        s11n::load(is, sar_amplitude.drr.reinhardlocal.threshold);
        s11n::load(is, sar_amplitude.gradient_length);
        for (int i = 0; i < sar_amplitude.gradient_length; i++) {
            s11n::load(is, sar_amplitude.gradient[3 * i + 0]);
            s11n::load(is, sar_amplitude.gradient[3 * i + 1]);
            s11n::load(is, sar_amplitude.gradient[3 * i + 2]);
        }
        s11n::load(is, sar_amplitude.adapt_brightness);
    } else if (category == ecmdb::category_data) {
        s11n::load(is, data.offset);
        s11n::load(is, data.factor);
        s11n::load(is, data.gradient_length);
        for (int i = 0; i < data.gradient_length; i++) {
            s11n::load(is, data.gradient[3 * i + 0]);
            s11n::load(is, data.gradient[3 * i + 1]);
            s11n::load(is, data.gradient[3 * i + 2]);
        }
    } else {
        assert(category == ecmdb::category_invalid);
    }
}

void processing_parameters::save(std::ostream& os, const char* name) const
{
    s11n::startgroup(os, name);
    if (category_e2c) {
        vector<uint8_t, 3> c;
        s11n::startgroup(os, "e2c");
        s11n::startgroup(os, "gradient");
        s11n::save(os, "length", e2c.gradient_length);
        for (int i = 0; i < e2c.gradient_length; i++) {
            c = vector<uint8_t, 3>(e2c.gradient + 3 * i);
            s11n::save(os, "", c);
        }
        s11n::endgroup(os);
        s11n::save(os, "adapt_brightness", e2c.adapt_brightness);
        s11n::save(os, "isolines_distance", e2c.isolines_distance);
        s11n::save(os, "isolines_thickness", e2c.isolines_thickness);
        c = vector<uint8_t, 3>(e2c.isolines_color);
        s11n::save(os, "isolines_color", c);
        s11n::endgroup(os);
    } else if (category == ecmdb::category_elevation) {
        s11n::startgroup(os, "elevation");
        s11n::save(os, "scale-factor", elevation.scale_factor);
        s11n::save(os, "scale-center", elevation.scale_center);
        s11n::endgroup(os);
    } else if (category == ecmdb::category_texture) {
        s11n::startgroup(os, "texture");
        s11n::save(os, "contrast", texture.contrast);
        s11n::save(os, "brightness", texture.brightness);
        s11n::save(os, "saturation", texture.saturation);
        s11n::save(os, "hue", texture.hue);
        s11n::endgroup(os);
    } else if (category == ecmdb::category_sar_amplitude) {
        vector<uint8_t, 3> c;
        s11n::startgroup(os, "sar-amplitude");
        s11n::save(os, "despeckling_method", sar_amplitude.despeckling_method);
        s11n::save(os, "despeckling.mean.kh", sar_amplitude.despeckling.mean.kh);
        s11n::save(os, "despeckling.mean.kv", sar_amplitude.despeckling.mean.kv);
        s11n::save(os, "despeckling.median.kh", sar_amplitude.despeckling.median.kh);
        s11n::save(os, "despeckling.median.kv", sar_amplitude.despeckling.median.kv);
        s11n::save(os, "despeckling.gauss.kh", sar_amplitude.despeckling.gauss.kh);
        s11n::save(os, "despeckling.gauss.kv", sar_amplitude.despeckling.gauss.kv);
        s11n::save(os, "despeckling.gauss.sh", sar_amplitude.despeckling.gauss.sh);
        s11n::save(os, "despeckling.gauss.sv", sar_amplitude.despeckling.gauss.sv);
        s11n::save(os, "despeckling.lee.kh", sar_amplitude.despeckling.lee.kh);
        s11n::save(os, "despeckling.lee.kv", sar_amplitude.despeckling.lee.kv);
        s11n::save(os, "despeckling.lee.sigma_n", sar_amplitude.despeckling.lee.sigma_n);
        s11n::save(os, "despeckling.kuan.kh", sar_amplitude.despeckling.kuan.kh);
        s11n::save(os, "despeckling.kuan.kv", sar_amplitude.despeckling.kuan.kv);
        s11n::save(os, "despeckling.kuan.L", sar_amplitude.despeckling.kuan.L);
        s11n::save(os, "despeckling.xiao.kh", sar_amplitude.despeckling.xiao.kh);
        s11n::save(os, "despeckling.xiao.kv", sar_amplitude.despeckling.xiao.kv);
        s11n::save(os, "despeckling.xiao.Tmin", sar_amplitude.despeckling.xiao.Tmin);
        s11n::save(os, "despeckling.xiao.Tmax", sar_amplitude.despeckling.xiao.Tmax);
        s11n::save(os, "despeckling.xiao.a", sar_amplitude.despeckling.xiao.a);
        s11n::save(os, "despeckling.xiao.b", sar_amplitude.despeckling.xiao.b);
        s11n::save(os, "despeckling.frost.kh", sar_amplitude.despeckling.frost.kh);
        s11n::save(os, "despeckling.frost.kv", sar_amplitude.despeckling.frost.kv);
        s11n::save(os, "despeckling.frost.a", sar_amplitude.despeckling.frost.a);
        s11n::save(os, "despeckling.gammamap.kh", sar_amplitude.despeckling.gammamap.kh);
        s11n::save(os, "despeckling.gammamap.kv", sar_amplitude.despeckling.gammamap.kv);
        s11n::save(os, "despeckling.gammamap.L", sar_amplitude.despeckling.gammamap.L);
        s11n::save(os, "despeckling.oddy.kh", sar_amplitude.despeckling.oddy.kh);
        s11n::save(os, "despeckling.oddy.kv", sar_amplitude.despeckling.oddy.kv);
        s11n::save(os, "despeckling.oddy.alpha", sar_amplitude.despeckling.oddy.alpha);
        s11n::save(os, "despeckling.waveletst.threshold", sar_amplitude.despeckling.waveletst.threshold);
        s11n::save(os, "drr_method", sar_amplitude.drr_method);
        s11n::save(os, "drr.linear.min_amp", sar_amplitude.drr.linear.min_amp);
        s11n::save(os, "drr.linear.max_amp", sar_amplitude.drr.linear.max_amp);
        s11n::save(os, "drr.log.min_amp", sar_amplitude.drr.log.min_amp);
        s11n::save(os, "drr.log.max_amp", sar_amplitude.drr.log.max_amp);
        s11n::save(os, "drr.log.prescale", sar_amplitude.drr.log.prescale);
        s11n::save(os, "drr.gamma.min_amp", sar_amplitude.drr.gamma.min_amp);
        s11n::save(os, "drr.gamma.max_amp", sar_amplitude.drr.gamma.max_amp);
        s11n::save(os, "drr.gamma.gamma", sar_amplitude.drr.gamma.gamma);
        s11n::save(os, "drr.schlick.brightness", sar_amplitude.drr.schlick.brightness);
        s11n::save(os, "drr.reinhard.brightness", sar_amplitude.drr.reinhard.brightness);
        s11n::save(os, "drr.reinhard.contrast", sar_amplitude.drr.reinhard.contrast);
        s11n::save(os, "drr.schlicklocal.brightness", sar_amplitude.drr.schlicklocal.brightness);
        s11n::save(os, "drr.schlicklocal.details", sar_amplitude.drr.schlicklocal.details);
        s11n::save(os, "drr.schlicklocal.threshold", sar_amplitude.drr.schlicklocal.threshold);
        s11n::save(os, "drr.reinhardlocal.brightness", sar_amplitude.drr.reinhardlocal.brightness);
        s11n::save(os, "drr.reinhardlocal.contrast", sar_amplitude.drr.reinhardlocal.contrast);
        s11n::save(os, "drr.reinhardlocal.details", sar_amplitude.drr.reinhardlocal.details);
        s11n::save(os, "drr.reinhardlocal.threshold", sar_amplitude.drr.reinhardlocal.threshold);
        s11n::startgroup(os, "gradient");
        s11n::save(os, "length", sar_amplitude.gradient_length);
        for (int i = 0; i < sar_amplitude.gradient_length; i++) {
            c = vector<uint8_t, 3>(sar_amplitude.gradient + 3 * i);
            s11n::save(os, "", c);
        }
        s11n::endgroup(os);
        s11n::save(os, "adapt_brightness", sar_amplitude.adapt_brightness);
        s11n::endgroup(os);
    } else if (category == ecmdb::category_data) {
        vector<uint8_t, 3> c;
        s11n::startgroup(os, "data");
        s11n::save(os, "offset", data.offset);
        s11n::save(os, "factor", data.factor);
        s11n::startgroup(os, "gradient");
        s11n::save(os, "length", data.gradient_length);
        for (int i = 0; i < data.gradient_length; i++) {
            c = vector<uint8_t, 3>(data.gradient + 3 * i);
            s11n::save(os, "", c);
        }
        s11n::endgroup(os);
        s11n::endgroup(os);
    } else {
        s11n::save(os, "invalid", 1);
    }
    s11n::endgroup(os);
}

void processing_parameters::load(const std::string& s)
{
    std::istringstream iss(s);
    std::string name, value;
    s11n::load(iss, name, value);
    if (name == "e2c") {
        reset(true);
        iss.str(value);
        while (iss.good()) {
            s11n::load(iss, name, value);
            if (name == "gradient") {
                std::istringstream iss2(value);
                s11n::load(iss2, name, value);
                if (name == "length") {
                    s11n::load(value, e2c.gradient_length);
                    if (e2c.gradient_length < 1 || e2c.gradient_length > max_gradient_length)
                        e2c.gradient_length = 1;
                    std::memset(e2c.gradient, 0, e2c.gradient_length * sizeof(vector<uint8_t, 3>));
                    for (int i = 0; i < e2c.gradient_length && iss.good(); i++) {
                        vector<uint8_t, 3> c;
                        s11n::load(iss2, name, value);
                        s11n::load(value, c);
                        e2c.gradient[3 * i + 0] = c.x;
                        e2c.gradient[3 * i + 1] = c.y;
                        e2c.gradient[3 * i + 2] = c.z;
                    }
                }
            } else if (name == "adapt_brightness")
                s11n::load(value, e2c.adapt_brightness);
            else if (name == "isolines_distance")
                s11n::load(value, e2c.isolines_distance);
            else if (name == "isolines_thickness")
                s11n::load(value, e2c.isolines_thickness);
            else if (name == "isolines_color") {
                vector<uint8_t, 3> c;
                s11n::load(value, c);
                e2c.isolines_color[0] = c.x;
                e2c.isolines_color[1] = c.y;
                e2c.isolines_color[2] = c.z;
            }
        }
    } else if (name == "elevation") {
        reset(false, ecmdb::category_elevation);
        iss.str(value);
        while (iss.good()) {
            s11n::load(iss, name, value);
            if (name == "scale-factor")
                s11n::load(value, elevation.scale_factor);
            else if (name == "scale-center")
                s11n::load(value, elevation.scale_center);
        }
    } else if (name == "texture" || name == "srgb") {  // "srgb" is for compatibility; remove soon
        reset(false, ecmdb::category_texture);
        iss.str(value);
        while (iss.good()) {
            s11n::load(iss, name, value);
            if (name == "contrast")
                s11n::load(value, texture.contrast);
            else if (name == "brightness")
                s11n::load(value, texture.brightness);
            else if (name == "saturation")
                s11n::load(value, texture.saturation);
            else if (name == "hue")
                s11n::load(value, texture.hue);
        }
    } else if (name == "sar-amplitude") {
        reset(false, ecmdb::category_sar_amplitude);
        iss.str(value);
        while (iss.good()) {
            s11n::load(iss, name, value);
            if (name == "despeckling_method")
                s11n::load(value, sar_amplitude.despeckling_method);
            else if (name == "despeckling.mean.kh")
                s11n::load(value, sar_amplitude.despeckling.mean.kh);
            else if (name == "despeckling.mean.kv")
                s11n::load(value, sar_amplitude.despeckling.mean.kv);
            else if (name == "despeckling.median.kh")
                s11n::load(value, sar_amplitude.despeckling.median.kh);
            else if (name == "despeckling.median.kv")
                s11n::load(value, sar_amplitude.despeckling.median.kv);
            else if (name == "despeckling.gauss.kh")
                s11n::load(value, sar_amplitude.despeckling.gauss.kh);
            else if (name == "despeckling.gauss.kv")
                s11n::load(value, sar_amplitude.despeckling.gauss.kv);
            else if (name == "despeckling.gauss.sh")
                s11n::load(value, sar_amplitude.despeckling.gauss.sh);
            else if (name == "despeckling.gauss.sv")
                s11n::load(value, sar_amplitude.despeckling.gauss.sv);
            else if (name == "despeckling.lee.kh")
                s11n::load(value, sar_amplitude.despeckling.lee.kh);
            else if (name == "despeckling.lee.kv")
                s11n::load(value, sar_amplitude.despeckling.lee.kv);
            else if (name == "despeckling.lee.sigma_n")
                s11n::load(value, sar_amplitude.despeckling.lee.sigma_n);
            else if (name == "despeckling.kuan.kh")
                s11n::load(value, sar_amplitude.despeckling.kuan.kh);
            else if (name == "despeckling.kuan.kv")
                s11n::load(value, sar_amplitude.despeckling.kuan.kv);
            else if (name == "despeckling.kuan.L")
                s11n::load(value, sar_amplitude.despeckling.kuan.L);
            else if (name == "despeckling.xiao.kh")
                s11n::load(value, sar_amplitude.despeckling.xiao.kh);
            else if (name == "despeckling.xiao.kv")
                s11n::load(value, sar_amplitude.despeckling.xiao.kv);
            else if (name == "despeckling.xiao.Tmin")
                s11n::load(value, sar_amplitude.despeckling.xiao.Tmin);
            else if (name == "despeckling.xiao.Tmax")
                s11n::load(value, sar_amplitude.despeckling.xiao.Tmax);
            else if (name == "despeckling.xiao.a")
                s11n::load(value, sar_amplitude.despeckling.xiao.a);
            else if (name == "despeckling.xiao.b")
                s11n::load(value, sar_amplitude.despeckling.xiao.b);
            else if (name == "despeckling.frost.kh")
                s11n::load(value, sar_amplitude.despeckling.frost.kh);
            else if (name == "despeckling.frost.kv")
                s11n::load(value, sar_amplitude.despeckling.frost.kv);
            else if (name == "despeckling.frost.a")
                s11n::load(value, sar_amplitude.despeckling.frost.a);
            else if (name == "despeckling.gammamap.kh")
                s11n::load(value, sar_amplitude.despeckling.gammamap.kh);
            else if (name == "despeckling.gammamap.kv")
                s11n::load(value, sar_amplitude.despeckling.gammamap.kv);
            else if (name == "despeckling.gammamap.L")
                s11n::load(value, sar_amplitude.despeckling.gammamap.L);
            else if (name == "despeckling.oddy.kh")
                s11n::load(value, sar_amplitude.despeckling.oddy.kh);
            else if (name == "despeckling.oddy.kv")
                s11n::load(value, sar_amplitude.despeckling.oddy.kv);
            else if (name == "despeckling.oddy.alpha")
                s11n::load(value, sar_amplitude.despeckling.oddy.alpha);
            else if (name == "despeckling.waveletst.threshold")
                s11n::load(value, sar_amplitude.despeckling.waveletst.threshold);
            else if (name == "drr_method")
                s11n::load(value, sar_amplitude.drr_method);
            else if (name == "drr.linear.min_amp")
                s11n::load(value, sar_amplitude.drr.linear.min_amp);
            else if (name == "drr.linear.max_amp")
                s11n::load(value, sar_amplitude.drr.linear.max_amp);
            else if (name == "drr.log.min_amp")
                s11n::load(value, sar_amplitude.drr.log.min_amp);
            else if (name == "drr.log.max_amp")
                s11n::load(value, sar_amplitude.drr.log.max_amp);
            else if (name == "drr.log.prescale")
                s11n::load(value, sar_amplitude.drr.log.prescale);
            else if (name == "drr.gamma.min_amp")
                s11n::load(value, sar_amplitude.drr.gamma.min_amp);
            else if (name == "drr.gamma.max_amp")
                s11n::load(value, sar_amplitude.drr.gamma.max_amp);
            else if (name == "drr.gamma.gamma")
                s11n::load(value, sar_amplitude.drr.gamma.gamma);
            else if (name == "drr.schlick.brightness")
                s11n::load(value, sar_amplitude.drr.schlick.brightness);
            else if (name == "drr.reinhard.brightness")
                s11n::load(value, sar_amplitude.drr.reinhard.brightness);
            else if (name == "drr.reinhard.contrast")
                s11n::load(value, sar_amplitude.drr.reinhard.contrast);
            else if (name == "drr.schlicklocal.brightness")
                s11n::load(value, sar_amplitude.drr.schlicklocal.brightness);
            else if (name == "drr.schlicklocal.details")
                s11n::load(value, sar_amplitude.drr.schlicklocal.details);
            else if (name == "drr.schlicklocal.threshold")
                s11n::load(value, sar_amplitude.drr.schlicklocal.threshold);
            else if (name == "drr.reinhardlocal.brightness")
                s11n::load(value, sar_amplitude.drr.reinhardlocal.brightness);
            else if (name == "drr.reinhardlocal.contrast")
                s11n::load(value, sar_amplitude.drr.reinhardlocal.contrast);
            else if (name == "drr.reinhardlocal.details")
                s11n::load(value, sar_amplitude.drr.reinhardlocal.details);
            else if (name == "drr.reinhardlocal.threshold")
                s11n::load(value, sar_amplitude.drr.reinhardlocal.threshold);
            else if (name == "gradient") {
                std::istringstream iss2(value);
                s11n::load(iss2, name, value);
                if (name == "length") {
                    s11n::load(value, sar_amplitude.gradient_length);
                    if (sar_amplitude.gradient_length < 1 || sar_amplitude.gradient_length > max_gradient_length)
                        sar_amplitude.gradient_length = 1;
                    std::memset(sar_amplitude.gradient, 0, sar_amplitude.gradient_length * sizeof(vector<uint8_t, 3>));
                    for (int i = 0; i < sar_amplitude.gradient_length && iss.good(); i++) {
                        vector<uint8_t, 3> c;
                        s11n::load(iss2, name, value);
                        s11n::load(value, c);
                        sar_amplitude.gradient[3 * i + 0] = c.x;
                        sar_amplitude.gradient[3 * i + 1] = c.y;
                        sar_amplitude.gradient[3 * i + 2] = c.z;
                    }
                }
            } else if (name == "adapt_brightness") {
                s11n::load(value, sar_amplitude.adapt_brightness);
            }
        }
    } else if (name == "data") {
        reset(false, ecmdb::category_data);
        iss.str(value);
        while (iss.good()) {
            s11n::load(iss, name, value);
            if (name == "offset")
                s11n::load(value, data.offset);
            else if (name == "factor")
                s11n::load(value, data.factor);
            else if (name == "gradient") {
                std::istringstream iss2(value);
                s11n::load(iss2, name, value);
                if (name == "length") {
                    s11n::load(value, data.gradient_length);
                    if (data.gradient_length < 1 || data.gradient_length > max_gradient_length)
                        data.gradient_length = 1;
                    std::memset(data.gradient, 0, data.gradient_length * sizeof(vector<uint8_t, 3>));
                    for (int i = 0; i < data.gradient_length && iss.good(); i++) {
                        vector<uint8_t, 3> c;
                        s11n::load(iss2, name, value);
                        s11n::load(value, c);
                        data.gradient[3 * i + 0] = c.x;
                        data.gradient[3 * i + 1] = c.y;
                        data.gradient[3 * i + 2] = c.z;
                    }
                }
            }
        }
    } else {
        reset();
    }
}

/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012
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

#include "glvm-s11n.h"

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

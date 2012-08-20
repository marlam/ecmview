/*
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2012
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
#include <sstream>

#include <DTrack.hpp>

#include "msg.h"

#include "glvm.h"

#include "tracking-dtrack.h"

using namespace glvm;


/* Tracking Driver DTrack */

TrackingDriverDTrack::TrackingDriverDTrack()
{
    _dt = new DTrack();
}

TrackingDriverDTrack::~TrackingDriverDTrack()
{
    delete _dt;
}

bool TrackingDriverDTrack::update()
{
    if (!_dt->valid()) {
        msg::err("Tracking: DTrack initialization failed");
        return false;
    }
    if (!_dt->receive()) {
        msg::err("Tracking: DTrack tracking failed: %s",
                (_dt->timeout() ? "timeout" :
                 _dt->udperror() ? "UDP error" :
                 _dt->parseerror() ? "parse error" :
                 "unknown error"));
        return false;
    }
    return true;
}

void TrackingDriverDTrack::get_pos(const float dtrack_loc[3], vec3 &pos) const
{
    vec3 p(dtrack_loc);
    pos = p / 1000.0f;
}

void TrackingDriverDTrack::get_rot(const float dtrack_rot[3 * 3], mat3 &rot) const
{
    rot = mat3(dtrack_rot);
}

bool TrackingDriverDTrack::get(int type, int id,
        int64_t *timestamp,
        vec3 &pos, mat3 &rot,
        float joy[2], unsigned int *buttons)
{
    *timestamp = _dt->get_timestamp() * 1e6;
    if (type == Tracking::BODY) {
        for (int i = 0; i < _dt->get_num_body(); i++) {
            dtrack_body_type body = _dt->get_body(i);
            if (body.id == id && body.quality >= 0.0f) {
                get_pos(body.loc, pos);
                get_rot(body.rot, rot);
                joy[0] = 0.0f;
                joy[1] = 0.0f;
                *buttons = 0;
                return true;
            }
        }
    } else /* type == Tracking::FLYSTICK */ {
        for (int i = 0; i < _dt->get_num_flystick(); i++) {
            dtrack_flystick_type flystick = _dt->get_flystick(i);
            if (flystick.id == id && flystick.quality >= 0.0f) {
                get_pos(flystick.loc, pos);
                get_rot(flystick.rot, rot);
                joy[0] = flystick.joystick[0];
                joy[1] = flystick.joystick[1];
                *buttons = 0;
                for (int b = 0; b < flystick.num_button; b++) {
                    if (flystick.button[b]) {
                        *buttons |= (1 << b);
                    }
                }
                return true;
            }
        }
    }
    return false;
}

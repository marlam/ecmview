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

#ifndef TRACKING_DTRACK_H
#define TRACKING_DTRACK_H

#include "glvm.h"

#include "tracking.h"

class DTrack;

class TrackingDriverDTrack : public TrackingDriver
{
private:
    DTrack *_dt;
    void get_pos(const float dtrack_loc[3], glvm::vec3 &pos) const;
    void get_rot(const float dtrack_rot[3 * 3], glvm::mat3 &rot) const;

public:
    TrackingDriverDTrack();
    ~TrackingDriverDTrack();
    bool update();
    bool get(int type, int id,
            int64_t *timestamp,
            glvm::vec3 &pos, glvm::mat3 &rot,
            float joy[2], unsigned int *buttons);
};

#endif

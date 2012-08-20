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

#ifndef WARPING_H
#define WARPING_H

#include <string>

#include <eq/eq.h>
#include <GL/glew.h>

#include "glvm.h"


class warping
{
private:
    int _wall;              // 1-6 or 0 for "unknown"
    int _width;
    int _height;
    GLuint _color;
    GLuint _depth;
    GLuint _fbo;
    GLuint _prg;

public:
    warping(const std::string& window_name, const eq::PixelViewport& viewport);
    ~warping();

    void init_gl();
    void exit_gl();

    void setup() const;
    void apply(const glvm::vec3& viewer_pos) const;
};

#endif

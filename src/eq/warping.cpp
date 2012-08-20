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

#include <sstream>
#include <string>
#include <cstring>
#include <cstdlib>

#include <eq/eq.h>
#include <GL/glew.h>
#include <GL/glu.h>

#include "dbg.h"
#include "msg.h"

#include "glvm.h"
#include "glvm-gl.h"
#include "xgl.h"

#include "warping.h"

#include "warping.glsl.h"

using namespace glvm;


/* Measured values, taken from the output of the measurement program.
 * All measurements are in meters. */
static const vec3 O[4] = {
    vec3(-2.49f,    0.0f, -0.0771964f), // A
    vec3(-1.78883f, 0.0f, -1.73382f  ), // B
    vec3(-0.1549f,  0.0f, -2.48638f  ), // D
    vec3(+1.5598f,  0.0f, -1.94244f  )  // F
};
static const vec3 U[4] = {
    vec3(-1.5598f,  0.0f, -1.94244f  ), // C
    vec3(+0.1549f,  0.0f, -2.48638f  ), // E
    vec3(+1.78883f, 0.0f, -1.73382f  ), // G
    vec3(+2.49f,    0.0f, -0.0771964f)  // H
};
static const float wall_height = 2.6f;
static const float radius = 2.4912f;
static const vec3 P_org(0.0f, 1.78f, 0.0f);


warping::warping(const std::string& window_name, const eq::PixelViewport& viewport)
{
    if (window_name.compare(0, 12, "siegen-wall1") == 0)
        _wall = 1;
    else if (window_name.compare(0, 12, "siegen-wall2") == 0)
        _wall = 2;
    else if (window_name.compare(0, 12, "siegen-wall3") == 0)
        _wall = 3;
    else if (window_name.compare(0, 12, "siegen-wall4") == 0)
        _wall = 4;
    else if (window_name.compare(0, 12, "siegen-wall5") == 0)
        _wall = 5;
    else if (window_name.compare(0, 12, "siegen-wall6") == 0)
        _wall = 6;
    else
        _wall = 0;
    _width = viewport.w;
    _height = viewport.h;
}

void warping::init_gl()
{
    if (_wall >= 1 && _wall <= 4) {
        //msg::dbg("Setting up warping for Uni Siegen VR wall %d. Size: %dx%d.", _wall, viewport.w, viewport.h);
        glGenTextures(1, &_color);
        glBindTexture(GL_TEXTURE_2D, _color);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _width, _height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glGenRenderbuffers(1, &_depth);
        glBindRenderbuffer(GL_RENDERBUFFER, _depth);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, _width, _height);
        glGenFramebuffers(1, &_fbo);
        _prg = xgl::CreateProgram("warping", "", "", WARPING_GLSL_STR);
        xgl::LinkProgram("warping", _prg);
        xgl::CheckError(HERE);
    }
}

void warping::exit_gl()
{
    if (_wall >= 1 && _wall <= 4) {
        glDeleteTextures(1, &_color);
        glDeleteRenderbuffers(1, &_depth);
        glDeleteFramebuffers(1, &_fbo);
        xgl::DeleteProgram(_prg);
    }
}

warping::~warping()
{
}

void warping::setup() const
{
    if (_wall >= 1 && _wall <= 4) {
        glBindFramebuffer(GL_FRAMEBUFFER, _fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _color, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth);
        xgl::CheckFBO(GL_FRAMEBUFFER, HERE);
        xgl::CheckError(HERE);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //msg::dbg("Starting warping for CGG wall %d", _wall);
    }
}

void warping::apply(const vec3& viewer_pos) const
{
    if (_wall >= 1 && _wall <= 4) {
        //msg::dbg("Applying warping for CGG wall %d, viewer at %f %f %f", _wall, viewer_pos.x(), viewer_pos.y(), viewer_pos.z());

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        assert(xgl::CheckFBO(GL_FRAMEBUFFER, HERE));
        glUseProgram(_prg);
        assert(xgl::CheckError(HERE));

        const vec3 u = U[_wall - 1] - O[_wall - 1];
        const vec3 v = vec3(0.0f, wall_height, 0.0f);
        glvmUniform(glGetUniformLocation(_prg, "P"), viewer_pos);
        //glvmUniform(glGetUniformLocation(_prg, "P_org"), P_org);
        glvmUniform(glGetUniformLocation(_prg, "O"), O[_wall - 1]);
        glvmUniform(glGetUniformLocation(_prg, "U"), U[_wall - 1]);
        glvmUniform(glGetUniformLocation(_prg, "u"), u);
        glvmUniform(glGetUniformLocation(_prg, "v"), v);
        glvmUniform(glGetUniformLocation(_prg, "n"), cross(u, v));
        glvmUniform(glGetUniformLocation(_prg, "radius"), radius);
        //glvmUniform(glGetUniformLocation(_prg, "r2"), radius * radius);
        assert(xgl::CheckError(HERE));

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, _color);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glViewport(0, 0, _width, _height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        xgl::DrawQuad();
        glDisable(GL_TEXTURE_2D);
        glUseProgram(0);
        assert(xgl::CheckError(HERE));
    }
}

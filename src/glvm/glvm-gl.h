/*
 * C++ vector and matrix classes that resemble GLSL style.
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012
 * Martin Lambers <marlam@marlam.de>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the <ORGANIZATION> nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission. *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This file provides functions to use the GLVM data types with OpenGL.
 */

#ifndef GLVM_GL_H
#define GLVM_GL_H

#include <GL/glew.h>
#include <GL/glu.h>

#include "glvm.h"


/* Viewport */

inline void glvmViewport(const glvm::ivec4& vp)
{
    glViewport(vp[0], vp[1], vp[2], vp[3]);
}

inline void glvmGetViewport(glvm::ivec4& vp)
{
    glGetIntegerv(GL_VIEWPORT, vp.vl);
}

/* Get glvm::matrices */

inline void glvmGetProjectionMatrix(glvm::mat4& M)
{
    glGetFloatv(GL_PROJECTION_MATRIX, M.vl);
}

inline void glvmGetProjectionMatrix(glvm::dmat4& M)
{
    glGetDoublev(GL_PROJECTION_MATRIX, M.vl);
}

inline void glvmGetModelViewMatrix(glvm::mat4& M)
{
    glGetFloatv(GL_MODELVIEW_MATRIX, M.vl);
}

inline void glvmGetModelViewMatrix(glvm::dmat4& M)
{
    glGetDoublev(GL_MODELVIEW_MATRIX, M.vl);
}

inline void glvmGetTextureMatrix(glvm::mat4& M)
{
    glGetFloatv(GL_TEXTURE_MATRIX, M.vl);
}

inline void glvmGetTextureMatrix(glvm::dmat4& M)
{
    glGetDoublev(GL_TEXTURE_MATRIX, M.vl);
}

inline void glvmGetModelViewProjectionMatrix(glvm::mat4& M)
{
    glvm::mat4 p, m;
    glvmGetProjectionMatrix(p);
    glvmGetModelViewMatrix(m);
    M = p * m;
}

inline void glvmGetModelViewProjectionMatrix(glvm::dmat4& M)
{
    glvm::dmat4 p, m;
    glvmGetProjectionMatrix(p);
    glvmGetModelViewMatrix(m);
    M = p * m;
}

/* Load glvm::matrices */

inline void glvmLoadMatrix(const glvm::mat4& M)
{
    glLoadMatrixf(M.vl);
}

inline void glvmLoadMatrix(const glvm::dmat4& M)
{
    glLoadMatrixd(M.vl);
}

inline void glvmMatrixLoad(GLenum matrix_mode, const glvm::mat4& M)
{
    glMatrixLoadfEXT(matrix_mode, M.vl);
}

inline void glvmMatrixLoad(GLenum matrix_mode, const glvm::dmat4& M)
{
    glMatrixLoaddEXT(matrix_mode, M.vl);
}

inline void glvmMatrixLoadTranspose(GLenum matrix_mode, const glvm::mat4& M)
{
    glMatrixLoadTransposefEXT(matrix_mode, M.vl);
}

inline void glvmMatrixLoadTranspose(GLenum matrix_mode, const glvm::dmat4& M)
{
    glMatrixLoadTransposedEXT(matrix_mode, M.vl);
}

/* Manipulate glvm::matrices */

inline void glvmFrustum(const glvm::frust &f)
{
    glFrustum(f.l(), f.r(), f.b(), f.t(), f.n(), f.f());
}

inline void glvmMatrixFrustum(GLenum matrix_mode, const glvm::frust &f)
{
    glMatrixFrustumEXT(matrix_mode, f.l(), f.r(), f.b(), f.t(), f.n(), f.f());
}

inline void glvmFrustum(const glvm::dfrust &f)
{
    glFrustum(f.l(), f.r(), f.b(), f.t(), f.n(), f.f());
}

inline void glvmMatrixFrustum(GLenum matrix_mode, const glvm::dfrust &f)
{
    glMatrixFrustumEXT(matrix_mode, f.l(), f.r(), f.b(), f.t(), f.n(), f.f());
}

inline void glvmTranslate(const glvm::vec3& v)
{
    glTranslatef(v.x, v.y, v.z);
}

inline void glvmMatrixTranslate(GLenum matrix_mode, const glvm::vec3& v)
{
    glMatrixTranslatefEXT(matrix_mode, v.x, v.y, v.z);
}

inline void glvmTranslate(const glvm::dvec3& v)
{
    glTranslated(v.x, v.y, v.z);
}

inline void glvmMatrixTranslate(GLenum matrix_mode, const glvm::dvec3& v)
{
    glMatrixTranslatedEXT(matrix_mode, v.x, v.y, v.z);
}

inline void glvmScale(const glvm::vec3& s)
{
    glScalef(s.x, s.y, s.z);
}

inline void glvmMatrixScale(GLenum matrix_mode, const glvm::vec3& s)
{
    glMatrixScalefEXT(matrix_mode, s.x, s.y, s.z);
}

inline void glvmScale(const glvm::dvec3& s)
{
    glScaled(s.x, s.y, s.z);
}

inline void glvmMatrixScale(GLenum matrix_mode, const glvm::dvec3& s)
{
    glMatrixScaledEXT(matrix_mode, s.x, s.y, s.z);
}

inline void glvmRotate(const float angle, const glvm::vec3& xyz)
{
    glRotatef(angle, xyz.x, xyz.y, xyz.z);
}

inline void glvmMatrixRotate(GLenum matrix_mode, const float angle, const glvm::vec3& xyz)
{
    glMatrixRotatefEXT(matrix_mode, angle, xyz.x, xyz.y, xyz.z);
}

inline void glvmRotate(const double angle, const glvm::dvec3& xyz)
{
    glRotated(angle, xyz.x, xyz.y, xyz.z);
}

inline void glvmMatrixRotate(GLenum matrix_mode, const double angle, const glvm::dvec3& xyz)
{
    glMatrixRotatedEXT(matrix_mode, angle, xyz.x, xyz.y, xyz.z);
}

inline void glvmRotate(const glvm::quat &q)
{
    glMultMatrixf(toMat4(q).vl);
}

inline void glvmMatrixRotate(GLenum matrix_mode, const glvm::quat &q)
{
    glMatrixMultfEXT(matrix_mode, toMat4(q).vl);
}

inline void glvmRotate(const glvm::dquat &q)
{
    glMultMatrixd(toMat4(q).vl);
}

inline void glvmMatrixRotate(GLenum matrix_mode, const glvm::dquat &q)
{
    glMatrixMultdEXT(matrix_mode, toMat4(q).vl);
}

inline void glvmMultMatrix(const glvm::mat4& M)
{
    glMultMatrixf(M.vl);
}

inline void glvmMatrixMult(GLenum matrix_mode, const glvm::mat4& M)
{
    glMatrixMultfEXT(matrix_mode, M.vl);
}

inline void glvmMultMatrix(const glvm::dmat4& M)
{
    glMultMatrixd(M.vl);
}

inline void glvmMatrixMult(GLenum matrix_mode, const glvm::dmat4& M)
{
    glMatrixMultdEXT(matrix_mode, M.vl);
}

inline void glvmMultTransposeMatrix(const glvm::mat4& M)
{
    glMultTransposeMatrixf(M.vl);
}

inline void glvmMatrixMultTranspose(GLenum matrix_mode, const glvm::mat4& M)
{
    glMatrixMultTransposefEXT(matrix_mode, M.vl);
}

inline void glvmMultTransposeMatrix(const glvm::dmat4& M)
{
    glMultTransposeMatrixd(M.vl);
}

inline void glvmMatrixMultTranspose(GLenum matrix_mode, const glvm::dmat4& M)
{
    glMatrixMultTransposedEXT(matrix_mode, M.vl);
}

/* Colors */

inline void glvmColor(const glvm::ivec3& c)
{
    glColor3iv(c.vl);
}

inline void glvmColor(const glvm::uvec3& c)
{
    glColor3uiv(c.vl);
}

inline void glvmColor(const glvm::vec3& c)
{
    glColor3fv(c.vl);
}

inline void glvmColor(const glvm::dvec3& c)
{
    glColor3dv(c.vl);
}

inline void glvmColor(const glvm::ivec4& c)
{
    glColor4iv(c.vl);
}

inline void glvmColor(const glvm::vec4& c)
{
    glColor4fv(c.vl);
}

inline void glvmColor(const glvm::dvec4& c)
{
    glColor4dv(c.vl);
}

inline void glvmSecondaryColor(const glvm::ivec3& c)
{
    glSecondaryColor3iv(c.vl);
}

inline void glvmSecondaryColor(const glvm::uvec3& c)
{
    glSecondaryColor3uiv(c.vl);
}

inline void glvmSecondaryColor(const glvm::vec3& c)
{
    glSecondaryColor3fv(c.vl);
}

inline void glvmSecondaryColor(const glvm::dvec3& c)
{
    glSecondaryColor3dv(c.vl);
}

/* Vertices */

inline void glvmVertex(const glvm::ivec2& v)
{
    glVertex2iv(v.vl);
}

inline void glvmVertex(const glvm::vec2& v)
{
    glVertex2fv(v.vl);
}

inline void glvmVertex(const glvm::dvec2& v)
{
    glVertex2dv(v.vl);
}

inline void glvmVertex(const glvm::ivec3& v)
{
    glVertex3iv(v.vl);
}

inline void glvmVertex(const glvm::vec3& v)
{
    glVertex3fv(v.vl);
}

inline void glvmVertex(const glvm::dvec3& v)
{
    glVertex3dv(v.vl);
}

inline void glvmVertex(const glvm::ivec4& v)
{
    glVertex4iv(v.vl);
}

inline void glvmVertex(const glvm::vec4& v)
{
    glVertex4fv(v.vl);
}

inline void glvmVertex(const glvm::dvec4& v)
{
    glVertex4dv(v.vl);
}

/* Normals */

inline void glvmNormal(const glvm::ivec3& v)
{
    glNormal3iv(v.vl);
}

inline void glvmNormal(const glvm::vec3& v)
{
    glNormal3fv(v.vl);
}

inline void glvmNormal(const glvm::dvec3& v)
{
    glNormal3dv(v.vl);
}

/* TexCoords */

inline void glvmTexCoord(const int32_t v)
{
    glTexCoord1i(v);
}

inline void glvmTexCoord(const float v)
{
    glTexCoord1f(v);
}

inline void glvmTexCoord(const double v)
{
    glTexCoord1d(v);
}

inline void glvmTexCoord(const glvm::ivec2& v)
{
    glTexCoord2iv(v.vl);
}

inline void glvmTexCoord(const glvm::vec2& v)
{
    glTexCoord2fv(v.vl);
}

inline void glvmTexCoord(const glvm::dvec2& v)
{
    glTexCoord2dv(v.vl);
}

inline void glvmTexCoord(const glvm::ivec3& v)
{
    glTexCoord3iv(v.vl);
}

inline void glvmTexCoord(const glvm::vec3& v)
{
    glTexCoord3fv(v.vl);
}

inline void glvmTexCoord(const glvm::dvec3& v)
{
    glTexCoord3dv(v.vl);
}

inline void glvmTexCoord(const glvm::ivec4& v)
{
    glTexCoord4iv(v.vl);
}

inline void glvmTexCoord(const glvm::vec4& v)
{
    glTexCoord4fv(v.vl);
}

inline void glvmTexCoord(const glvm::dvec4& v)
{
    glTexCoord4dv(v.vl);
}

/* MultiTexCoords */

inline void glvmMultiTexCoord(GLenum target, int32_t v)
{
    glMultiTexCoord1i(target, v);
}

inline void glvmMultiTexCoord(GLenum target, float v)
{
    glMultiTexCoord1f(target, v);
}

inline void glvmMultiTexCoord(GLenum target, double v)
{
    glMultiTexCoord1d(target, v);
}

inline void glvmMultiTexCoord(GLenum target, const glvm::ivec2& v)
{
    glMultiTexCoord2iv(target, v.vl);
}

inline void glvmMultiTexCoord(GLenum target, const glvm::vec2& v)
{
    glMultiTexCoord2fv(target, v.vl);
}

inline void glvmMultiTexCoord(GLenum target, const glvm::dvec2& v)
{
    glMultiTexCoord2dv(target, v.vl);
}

inline void glvmMultiTexCoord(GLenum target, const glvm::ivec3& v)
{
    glMultiTexCoord3iv(target, v.vl);
}

inline void glvmMultiTexCoord(GLenum target, const glvm::vec3& v)
{
    glMultiTexCoord3fv(target, v.vl);
}

inline void glvmMultiTexCoord(GLenum target, const glvm::dvec3& v)
{
    glMultiTexCoord3dv(target, v.vl);
}

inline void glvmMultiTexCoord(GLenum target, const glvm::ivec4& v)
{
    glMultiTexCoord4iv(target, v.vl);
}

inline void glvmMultiTexCoord(GLenum target, const glvm::vec4& v)
{
    glMultiTexCoord4fv(target, v.vl);
}

inline void glvmMultiTexCoord(GLenum target, const glvm::dvec4& v)
{
    glMultiTexCoord4dv(target, v.vl);
}

/* VertexAttribs */

inline void glvmVertexAttrib(GLuint index, int32_t v)
{
    glVertexAttribI1iEXT(index, v);
}

inline void glvmVertexAttrib(GLuint index, uint32_t v)
{
    glVertexAttribI1uiEXT(index, v);
}

inline void glvmVertexAttrib(GLuint index, float v)
{
    glVertexAttrib1f(index, v);
}

inline void glvmVertexAttrib(GLuint index, double v)
{
    glVertexAttrib1d(index, v);
}

inline void glvmVertexAttrib(GLuint index, const glvm::ivec2& v)
{
    glVertexAttribI2ivEXT(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::uvec2& v)
{
    glVertexAttribI2uivEXT(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::vec2& v)
{
    glVertexAttrib2fv(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::dvec2& v)
{
    glVertexAttrib2dv(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::ivec3& v)
{
    glVertexAttribI3ivEXT(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::uvec3& v)
{
    glVertexAttribI3uivEXT(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::vec3& v)
{
    glVertexAttrib3fv(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::dvec3& v)
{
    glVertexAttrib3dv(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::ivec4& v)
{
    glVertexAttribI4ivEXT(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::uvec4& v)
{
    glVertexAttribI4uivEXT(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::vec4& v)
{
    glVertexAttrib4fv(index, v.vl);
}

inline void glvmVertexAttrib(GLuint index, const glvm::dvec4& v)
{
    glVertexAttrib4dv(index, v.vl);
}


/* glUniform */

inline void glvmUniform(GLint location, bool v)
{
    glUniform1i(location, v);
}

inline void glvmUniform(GLint location, int32_t v)
{
    glUniform1i(location, v);
}

inline void glvmUniform(GLint location, uint32_t v)
{
    glUniform1ui(location, v);
}

inline void glvmUniform(GLint location, float v)
{
    glUniform1f(location, v);
}

inline void glvmUniform(GLint location, const glvm::bvec2& v)
{
    glvm::ivec2 iv(v);
    glUniform2iv(location, 1, iv.vl);
}

inline void glvmUniform(GLint location, const glvm::ivec2& v)
{
    glUniform2iv(location, 1, v.vl);
}

inline void glvmUniform(GLint location, const glvm::uvec2& v)
{
    glUniform2uiv(location, 1, v.vl);
}

inline void glvmUniform(GLint location, const glvm::vec2& v)
{
    glUniform2fv(location, 1, v.vl);
}

inline void glvmUniform(GLint location, const glvm::bvec3& v)
{
    glvm::ivec3 iv(v);
    glUniform3iv(location, 1, iv.vl);
}

inline void glvmUniform(GLint location, const glvm::ivec3& v)
{
    glUniform3iv(location, 1, v.vl);
}

inline void glvmUniform(GLint location, const glvm::uvec3& v)
{
    glUniform3uiv(location, 1, v.vl);
}

inline void glvmUniform(GLint location, const glvm::vec3& v)
{
    glUniform3fv(location, 1, v.vl);
}

inline void glvmUniform(GLint location, const glvm::bvec4& v)
{
    glvm::ivec4 iv(v);
    glUniform4iv(location, 1, iv.vl);
}

inline void glvmUniform(GLint location, const glvm::ivec4& v)
{
    glUniform4iv(location, 1, v.vl);
}

inline void glvmUniform(GLint location, const glvm::uvec4& v)
{
    glUniform4uiv(location, 1, v.vl);
}

inline void glvmUniform(GLint location, const glvm::vec4& v)
{
    glUniform4fv(location, 1, v.vl);
}

inline void glvmUniform(GLint location, GLsizei count, int32_t* v)
{
    glUniform1iv(location, count, v);
}

inline void glvmUniform(GLint location, GLsizei count, float* v)
{
    glUniform1fv(location, count, v);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::ivec2* v)
{
    glUniform2iv(location, count, v[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::uvec2* v)
{
    glUniform2uiv(location, count, v[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::vec2* v)
{
    glUniform2fv(location, count, v[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::ivec3* v)
{
    glUniform3iv(location, count, v[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::uvec3* v)
{
    glUniform3uiv(location, count, v[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::vec3* v)
{
    glUniform3fv(location, count, v[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::ivec4* v)
{
    glUniform4iv(location, count, v[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::uvec4* v)
{
    glUniform4uiv(location, count, v[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::vec4* v)
{
    glUniform4fv(location, count, v[0].vl);
}

inline void glvmUniform(GLint location, const glvm::mat2& m)
{
    glUniformMatrix2fv(location, 1, GL_FALSE, m.vl);
}

inline void glvmUniform(GLint location, const glvm::mat3& m)
{
    glUniformMatrix3fv(location, 1, GL_FALSE, m.vl);
}

inline void glvmUniform(GLint location, const glvm::mat4& m)
{
    glUniformMatrix4fv(location, 1, GL_FALSE, m.vl);
}

inline void glvmUniform(GLint location, const glvm::mat2x3& m)
{
    glUniformMatrix2x3fv(location, 1, GL_FALSE, m.vl);
}

inline void glvmUniform(GLint location, const glvm::mat3x2& m)
{
    glUniformMatrix3x2fv(location, 1, GL_FALSE, m.vl);
}

inline void glvmUniform(GLint location, const glvm::mat2x4& m)
{
    glUniformMatrix2x4fv(location, 1, GL_FALSE, m.vl);
}

inline void glvmUniform(GLint location, const glvm::mat4x2& m)
{
    glUniformMatrix4x2fv(location, 1, GL_FALSE, m.vl);
}

inline void glvmUniform(GLint location, const glvm::mat3x4& m)
{
    glUniformMatrix3x4fv(location, 1, GL_FALSE, m.vl);
}

inline void glvmUniform(GLint location, const glvm::mat4x3& m)
{
    glUniformMatrix4x3fv(location, 1, GL_FALSE, m.vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::mat2* m)
{
    glUniformMatrix2fv(location, count, GL_FALSE, m[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::mat3* m)
{
    glUniformMatrix3fv(location, count, GL_FALSE, m[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::mat4* m)
{
    glUniformMatrix4fv(location, count, GL_FALSE, m[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::mat2x3* m)
{
    glUniformMatrix2x3fv(location, count, GL_FALSE, m[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::mat3x2* m)
{
    glUniformMatrix3x2fv(location, count, GL_FALSE, m[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::mat2x4* m)
{
    glUniformMatrix2x4fv(location, count, GL_FALSE, m[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::mat4x2* m)
{
    glUniformMatrix4x2fv(location, count, GL_FALSE, m[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::mat3x4* m)
{
    glUniformMatrix3x4fv(location, count, GL_FALSE, m[0].vl);
}

inline void glvmUniform(GLint location, GLsizei count, const glvm::mat4x3* m)
{
    glUniformMatrix4x3fv(location, count, GL_FALSE, m[0].vl);
}

/* GLU */

inline GLint glvmuProject(const glvm::dvec3& obj, const glvm::dmat4& modelviewmatrix,
        const glvm::dmat4& projectionmatrix, const glvm::ivec4& viewport, glvm::dvec3& win)
{
    return gluProject(obj.x, obj.y, obj.z,
            modelviewmatrix.vl, projectionmatrix.vl, viewport.vl,
            &(win.x), &(win.y), &(win.z));
}

inline GLint glvmuUnProject(const glvm::dvec3& win, const glvm::dmat4& model, const glvm::dmat4& proj,
        const glvm::ivec4& view, glvm::dvec3& obj)
{
    return gluUnProject(win.x, win.y, win.z,
            model.vl, proj.vl, view.vl,
            &(obj.x), &(obj.y), &(obj.z));
}

/* Own */

inline glvm::dvec2 glvmProject(const glvm::dvec3& obj,
        const glvm::dmat4& modelviewprojectionmatrix, const glvm::ivec4& viewport)
{
    glvm::dvec4 p = modelviewprojectionmatrix * glvm::dvec4(obj, 1.0);
    p.xy() /= p.w;
    p.xy() = p.xy() * 0.5 + glvm::dvec2(0.5);
    p.xy() = p.xy() * glvm::dvec2(viewport[2], viewport[3]) + glvm::dvec2(viewport[0], viewport[1]);
    return p.xy();
}

inline glvm::dvec2 glvmProject(const glvm::dvec3& obj, const glvm::dmat4& modelviewmatrix,
        const glvm::dmat4& projectionmatrix, const glvm::ivec4& viewport)
{
    return glvmProject(obj, projectionmatrix * modelviewmatrix, viewport);
}

inline glvm::vec2 glvmProject(const glvm::vec3& obj,
        const glvm::mat4& modelviewprojectionmatrix, const glvm::ivec4& viewport)
{
    glvm::vec4 p = modelviewprojectionmatrix * glvm::vec4(obj, 1.0f);
    p.xy() /= p.w;
    p.xy() = p.xy() * 0.5f + glvm::vec2(0.5f);
    p.xy() = p.xy() * glvm::vec2(viewport[2], viewport[3]) + glvm::vec2(viewport[0], viewport[1]);
    return p.xy();
}

inline glvm::vec2 glvmProject(const glvm::vec3& obj, const glvm::mat4& modelviewmatrix,
        const glvm::mat4& projectionmatrix, const glvm::ivec4& viewport)
{
    return glvmProject(obj, projectionmatrix * modelviewmatrix, viewport);
}

#endif

/*
 * Copyright (C) 2010, 2011, 2012
 * Martin Lambers <marlam@marlam.de>
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

#include <cstring>

#include <GL/glew.h>

#include "gettext.h"
#define _(string) gettext(string)

#include "dbg.h"
#include "exc.h"
#include "str.h"
#include "msg.h"

#include "xgl.h"


/*
 * Push and Pop things
 */

static void *xglPush(std::vector<unsigned char> &stack, size_t s)
{
    assert(s <= 1024);
    assert(stack.size() <= 1024 - s);
    stack.resize(stack.size() + s);
    return &(stack[stack.size() - s]);
}

static void *xglTop(std::vector<unsigned char> &stack, size_t s)
{
    assert(stack.size() >= s);
    return &(stack[stack.size() - s]);
}

static void xglPop(std::vector<unsigned char> &stack, size_t s)
{
    assert(stack.size() >= s);
    stack.resize(stack.size() - s);
}

void xgl::PushProgram(std::vector<unsigned char> &stack)
{
    glGetIntegerv(GL_CURRENT_PROGRAM, static_cast<GLint *>(xglPush(stack, sizeof(GLint))));
}

void xgl::PopProgram(std::vector<unsigned char> &stack)
{
    const GLint *prg = static_cast<const GLint *>(xglTop(stack, sizeof(GLint)));
    glUseProgram(*prg);
    xglPop(stack, sizeof(GLint));
}

void xgl::PushViewport(std::vector<unsigned char> &stack)
{
    glGetIntegerv(GL_VIEWPORT, static_cast<GLint *>(xglPush(stack, 4 * sizeof(GLint))));
}

void xgl::PopViewport(std::vector<unsigned char> &stack)
{
    const GLint *vp = static_cast<const GLint *>(xglTop(stack, 4 * sizeof(GLint)));
    glViewport(vp[0], vp[1], vp[2], vp[3]);
    xglPop(stack, 4 * sizeof(GLint));
}

void xgl::PushFBO(std::vector<unsigned char> &stack)
{
    glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, static_cast<GLint *>(xglPush(stack, sizeof(GLint))));
}

void xgl::PopFBO(std::vector<unsigned char> &stack)
{
    const GLint *fbo = static_cast<const GLint *>(xglTop(stack, sizeof(GLint)));
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, *fbo);
    xglPop(stack, sizeof(GLint));
}

void xgl::PushModelViewMatrix(std::vector<unsigned char> &stack)
{
    glGetFloatv(GL_MODELVIEW_MATRIX, static_cast<GLfloat *>(xglPush(stack, 16 * sizeof(GLfloat))));
}

void xgl::PopModelViewMatrix(std::vector<unsigned char> &stack)
{
    GLint m;
    glGetIntegerv(GL_MATRIX_MODE, &m);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(static_cast<const GLfloat *>(xglTop(stack, 16 * sizeof(GLfloat))));
    xglPop(stack, 16 * sizeof(GLfloat));
    glMatrixMode(m);
}

void xgl::PushProjectionMatrix(std::vector<unsigned char> &stack)
{
    glGetFloatv(GL_PROJECTION_MATRIX, static_cast<GLfloat *>(xglPush(stack, 16 * sizeof(GLfloat))));
}

void xgl::PopProjectionMatrix(std::vector<unsigned char> &stack)
{
    GLint m;
    glGetIntegerv(GL_MATRIX_MODE, &m);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(static_cast<const GLfloat *>(xglTop(stack, 16 * sizeof(GLfloat))));
    xglPop(stack, 16 * sizeof(GLfloat));
    glMatrixMode(m);
}

void xgl::PushEverything(std::vector<unsigned char> &stack)
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    xgl::PushModelViewMatrix(stack);
    xgl::PushProjectionMatrix(stack);
    xgl::PushFBO(stack);
    xgl::PushViewport(stack);
    xgl::PushProgram(stack);
}

void xgl::PopEverything(std::vector<unsigned char> &stack)
{
    xgl::PopProgram(stack);
    xgl::PopViewport(stack);
    xgl::PopFBO(stack);
    xgl::PopProjectionMatrix(stack);
    xgl::PopModelViewMatrix(stack);
    glPopClientAttrib();
    glPopAttrib();
}


/*
 * Error checking
 */

bool xgl::CheckFBO(const GLenum target, const std::string &where)
{
    GLenum status = glCheckFramebufferStatusEXT(target);
    if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
        std::string errstr;
        switch (status) {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
            errstr = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT";
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
            errstr = "GL_FRAMEBUFFER_UNSUPPORTED_EXT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
            errstr = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
            errstr = "GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
            errstr = "GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
            errstr = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
            errstr = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT:
            errstr = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT";
            break;
        case 0:
            errstr = "in glCheckFramebufferStatus";
            break;
        default:
            errstr = str::asprintf("0x%X", static_cast<unsigned int>(status));
            break;
        }
        std::string pfx = (where.length() > 0 ? where + ": " : "");
        throw exc(pfx + str::asprintf(_("OpenGL FBO error %s."), errstr.c_str()));
        return false;
    }
    return true;
}

bool xgl::CheckFBO(const GLenum target)
{
    return xgl::CheckFBO(target, "");
}

bool xgl::CheckError(const std::string &where)
{
    GLenum e = glGetError();
    if (e != GL_NO_ERROR) {
        std::string pfx = (where.length() > 0 ? where + ": " : "");
        throw exc(pfx + str::asprintf(_("OpenGL error 0x%04X: %s"),
                    static_cast<unsigned int>(e),
                    gluErrorString(e)));
        return false;
    }
    return true;
}

bool xgl::CheckError()
{
    return xgl::CheckError("");
}


/*
 * Shaders and Programs
 */

static void xglKillCrlf(char *str)
{
    size_t l = strlen(str);
    if (l > 0 && str[l - 1] == '\n')
        str[--l] = '\0';
    if (l > 0 && str[l - 1] == '\r')
        str[l - 1] = '\0';
}

GLuint xgl::CompileShader(const std::string &name, GLenum type, const std::string &src)
{
    msg::dbg("Compiling %s shader %s.",
            type == GL_VERTEX_SHADER ? "vertex" : type == GL_GEOMETRY_SHADER_EXT ? "geometry" : "fragment",
            name.c_str());

    GLuint shader = glCreateShader(type);
    const GLchar *glsrc = src.c_str();
    glShaderSource(shader, 1, &glsrc, NULL);
    glCompileShader(shader);

    std::string log;
    GLint e, l;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &e);
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &l);
    if (l > 0) {
        char *tmplog = new char[l];
        glGetShaderInfoLog(shader, l, NULL, tmplog);
        xglKillCrlf(tmplog);
        log = std::string(tmplog);
        delete[] tmplog;
    } else {
        log = std::string("");
    }

    if (e == GL_TRUE && log.length() > 0) {
        msg::wrn(_("OpenGL %s '%s': compiler warning:"),
                type == GL_VERTEX_SHADER ? _("vertex shader")
                : type == GL_GEOMETRY_SHADER_EXT ? _("geometry shader")
                : _("fragment shader"),
                name.c_str());
        msg::wrn_txt("%s", log.c_str());
    } else if (e != GL_TRUE) {
        std::string when = str::asprintf(_("OpenGL %s '%s': compilation failed"),
                type == GL_VERTEX_SHADER ? _("vertex shader")
                : type == GL_GEOMETRY_SHADER_EXT ? _("geometry shader")
                : _("fragment shader"),
                name.c_str());
        std::string what = str::asprintf("\n%s", log.length() > 0 ? log.c_str() : _("unknown error"));
        throw exc(str::asprintf(_("%s: %s"), when.c_str(), what.c_str()));
    }
    return shader;
}

GLuint xgl::CreateProgram(GLuint vshader, GLuint gshader, GLuint fshader)
{
    assert(vshader != 0 || gshader != 0 || fshader != 0);

    GLuint program = glCreateProgram();
    if (vshader != 0)
        glAttachShader(program, vshader);
    if (gshader != 0)
        glAttachShader(program, gshader);
    if (fshader != 0)
        glAttachShader(program, fshader);

    return program;
}

GLuint xgl::CreateProgram(const std::string &name,
        const std::string &vshader_src, const std::string &gshader_src, const std::string &fshader_src)
{
    GLuint vshader = 0, gshader = 0, fshader = 0;

    if (vshader_src.length() > 0)
        vshader = xgl::CompileShader(name, GL_VERTEX_SHADER, vshader_src);
    if (gshader_src.length() > 0)
        gshader = xgl::CompileShader(name, GL_GEOMETRY_SHADER_EXT, gshader_src);
    if (fshader_src.length() > 0)
        fshader = xgl::CompileShader(name, GL_FRAGMENT_SHADER, fshader_src);

    return xgl::CreateProgram(vshader, gshader, fshader);
}

void xgl::LinkProgram(const std::string &name, const GLuint prg)
{
    msg::dbg("Linking OpenGL program %s.", name.c_str());

    glLinkProgram(prg);

    std::string log;
    GLint e, l;
    glGetProgramiv(prg, GL_LINK_STATUS, &e);
    glGetProgramiv(prg, GL_INFO_LOG_LENGTH, &l);
    if (l > 0) {
        char *tmplog = new char[l];
        glGetProgramInfoLog(prg, l, NULL, tmplog);
        xglKillCrlf(tmplog);
        log = std::string(tmplog);
        delete[] tmplog;
    } else {
        log = std::string("");
    }

    if (e == GL_TRUE && log.length() > 0) {
        msg::wrn(_("OpenGL program '%s': linker warning:"), name.c_str());
        msg::wrn_txt("%s", log.c_str());
    } else if (e != GL_TRUE) {
        std::string when = str::asprintf(_("OpenGL program '%s': linking failed"), name.c_str());
        std::string what = str::asprintf("\n%s", log.length() > 0 ? log.c_str() : _("unknown error"));
        throw exc(when + ": " + what);
    }
}

void xgl::DeleteProgram(GLuint program)
{
    if (glIsProgram(program)) {
        GLint shader_count;
        glGetProgramiv(program, GL_ATTACHED_SHADERS, &shader_count);
        GLuint *shaders = new GLuint[shader_count];
        glGetAttachedShaders(program, shader_count, NULL, shaders);
        for (int i = 0; i < shader_count; i++)
            glDeleteShader(shaders[i]);
        delete[] shaders;
        glDeleteProgram(program);
    }
}

void xgl::DeletePrograms(GLsizei n, const GLuint *programs)
{
    for (GLsizei i = 0; i < n; i++)
        xgl::DeleteProgram(programs[i]);
}


/*
 * Renter into textures
 */

GLint xgl::GetTex2DParameter(GLuint tex, GLenum pname)
{
    GLint p, tex_bak;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_bak);
    glBindTexture(GL_TEXTURE_2D, tex);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, pname, &p);
    glBindTexture(GL_TEXTURE_2D, tex_bak);
    return p;
}

GLuint xgl::CreateTex2D(GLint internal_format, int w, int h, GLenum filter)
{
    GLint tex_bak;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_bak);
    GLuint t;
    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glBindTexture(GL_TEXTURE_2D, tex_bak);
    return t;
}

void xgl::WriteTex2D(GLuint dst_tex, int x, int y, int w, int h,
        GLenum format, GLenum type, size_t line_size, const void* data,
        GLuint pbo)
{
    assert(pbo != 0);
    assert(dst_tex != 0);
    assert(x >= 0);
    assert(y >= 0);
    assert(w > 0);
    assert(h > 0);
    assert(GetTex2DParameter(dst_tex, GL_TEXTURE_WIDTH) >= x + w);
    assert(GetTex2DParameter(dst_tex, GL_TEXTURE_HEIGHT) >= y + h);

    GLint tex_bak, pub_bak, ua_bak;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_bak);
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING, &pub_bak);
    glGetIntegerv(GL_UNPACK_ALIGNMENT, &ua_bak);

    glBindTexture(GL_TEXTURE_2D, dst_tex);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, line_size * h, NULL, GL_STREAM_DRAW);
    void *pboptr = glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
    if (!pboptr) {
        throw exc(_("OpenGL error: cannot create a PBO buffer."));
    }
    assert(reinterpret_cast<uintptr_t>(pboptr) % 4 == 0);
    std::memcpy(pboptr, data, line_size * h);
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    glPixelStorei(GL_UNPACK_ALIGNMENT, line_size % 4 == 0 ? 4 : line_size % 2 == 0 ? 2 : 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, w, h, format, type, NULL);

    glBindTexture(GL_TEXTURE_2D, tex_bak);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pub_bak);
    glPixelStorei(GL_UNPACK_ALIGNMENT, ua_bak);
}

void xgl::ReadTex2DStart(GLuint src_tex, int x, int y, int w, int h,
        GLenum format, GLenum type, size_t line_size, GLuint pbo)
{
    assert(pbo != 0);
    assert(src_tex != 0);
    assert(x >= 0);
    assert(y >= 0);
    assert(w > 0);
    assert(h > 0);
    assert(GetTex2DParameter(src_tex, GL_TEXTURE_WIDTH) >= x + w);
    assert(GetTex2DParameter(src_tex, GL_TEXTURE_HEIGHT) >= y + h);

    GLint rb_bak, ppb_bak, pa_bak;
    glGetIntegerv(GL_READ_BUFFER, &rb_bak);
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &ppb_bak);
    glGetIntegerv(GL_PACK_ALIGNMENT, &pa_bak);

    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, src_tex, 0);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    assert(xgl::CheckFBO(GL_READ_FRAMEBUFFER, HERE));

    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glBufferData(GL_PIXEL_PACK_BUFFER, line_size * h, NULL, GL_STREAM_READ);
    glPixelStorei(GL_PACK_ALIGNMENT, line_size % 4 == 0 ? 4 : line_size % 2 == 0 ? 2 : 1);
    glReadPixels(x, y, w, h, format, type, NULL);

    glReadBuffer(rb_bak);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, ppb_bak);
    glPixelStorei(GL_PACK_ALIGNMENT, pa_bak);

    // TODO FIXME: This function changes the color attachment of the GL_READ_FRAMEBUFFER
    // as I could not find a way to query and restore the original value.
}

const void* xgl::ReadTex2DGetData(GLuint pbo)
{
    GLint ppb_bak;
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &ppb_bak);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    const void* pboptr = glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    if (!pboptr) {
        throw exc(_("OpenGL error: cannot create a PBO buffer."));
    }
    glBindBuffer(GL_PIXEL_PACK_BUFFER, ppb_bak);
    return pboptr;
}

void xgl::ReadTex2DFinish(GLuint pbo)
{
    GLint ppb_bak;
    glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING, &ppb_bak);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);
    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, ppb_bak);
}

void xgl::ReadTex2D(GLuint src_tex, int x, int y, int w, int h,
        GLenum format, GLenum type, size_t line_size, void* data,
        GLuint pbo)
{
    ReadTex2DStart(src_tex, x, y, w, h, format, type, line_size, pbo);
    const void* pboptr = ReadTex2DGetData(pbo);
    std::memcpy(data, pboptr, line_size * h);
    ReadTex2DFinish(pbo);
}

void xgl::DrawQuad(float x, float y, float w, float h,
        float tex_x, float tex_y, float tex_w, float tex_h)
{
    glBegin(GL_QUADS);
    glTexCoord2f(tex_x, tex_y);
    glVertex2f(x, y);
    glTexCoord2f(tex_x + tex_w, tex_y);
    glVertex2f(x + w, y);
    glTexCoord2f(tex_x + tex_w, tex_y + tex_h);
    glVertex2f(x + w, y + h);
    glTexCoord2f(tex_x, tex_y + tex_h);
    glVertex2f(x, y + h);
    glEnd();
}

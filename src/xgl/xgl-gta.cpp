/*
 * Copyright (C) 2011, 2012, 2013
 * Computer Graphics Group, University of Siegen, Germany.
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>.
 * See http://www.cg.informatik.uni-siegen.de/ for contact information.
 *
 * All rights reserved.
 */

#include "config.h"

#include <gta/gta.hpp>

#include "fio.h"
#include "str.h"
#include "exc.h"
#include "msg.h"
#include "blb.h"

#include "xgl-gta.h"


void xgl::SaveTex2D(FILE* f, GLuint tex)
{
    if (!glIsTexture(tex)) {
        throw exc("Cannot save object that is not a texture");
    }

    // The following assumes that all textures are 2D!

    GLint tex_bak;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &tex_bak);

    glBindTexture(GL_TEXTURE_2D, tex);
    GLint w, h, internal_format;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);
    glBindTexture(GL_TEXTURE_2D, tex_bak);

    msg::dbg("Trying to save %dx%d texture with internal format 0x%04x", w, h, internal_format);

    gta::header hdr;
    hdr.global_taglist().set("GL/INTERNAL_FORMAT", str::asprintf("0x%04x", internal_format).c_str());
    if (w < 1 || h < 1) {
        throw exc("Cannot save texture: invalid size");
    }
    hdr.set_dimensions(w, h);
    int components;
    switch (internal_format) {
    case GL_LUMINANCE:
    case GL_RED:
    case GL_R8:
    case GL_R8_SNORM:
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
    case GL_R8I:
    case GL_R8UI:
    case GL_R16I:
    case GL_R16UI:
    case GL_R32I:
    case GL_R32UI:
        components = 1;
        break;
    case GL_LUMINANCE_ALPHA:
    case GL_RG:
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    case GL_RG8I:
    case GL_RG8UI:
    case GL_RG16I:
    case GL_RG16UI:
    case GL_RG32I:
    case GL_RG32UI:
        components = 2;
        break;
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB10:
    case GL_RGB12:
    case GL_SRGB:
    case GL_SRGB8:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_RGB:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGB16:
    case GL_RGB16_SNORM:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_RGB8I:
    case GL_RGB8UI:
    case GL_RGB16I:
    case GL_RGB16UI:
    case GL_RGB32I:
    case GL_RGB32UI:
        components = 3;
        break;
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_RGB10_A2:
#ifdef GL_RGB10_A2UI
    case GL_RGB10_A2UI:
#endif
    case GL_RGBA12:
    case GL_SRGB_ALPHA:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_RGBA16:
    case GL_RGBA16_SNORM:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_RGBA8I:
    case GL_RGBA8UI:
    case GL_RGBA16I:
    case GL_RGBA16UI:
    case GL_RGBA32I:
    case GL_RGBA32UI:
        components = 4;
        break;
    default:
        throw exc(str::asprintf("Cannot save texture: internal format 0x%04x not handled yet", internal_format));
    }
    gta::type gta_type;
    switch (internal_format) {
    case GL_LUMINANCE:
    case GL_RED:
    case GL_R8:
    case GL_R8_SNORM:
    case GL_LUMINANCE_ALPHA:
    case GL_RG:
    case GL_RG8:
    case GL_RG8_SNORM:
    case GL_R3_G3_B2:
    case GL_RGB4:
    case GL_RGB5:
    case GL_RGB10:
    case GL_RGB12:
    case GL_SRGB:
    case GL_SRGB8:
    case GL_RGB:
    case GL_RGB8:
    case GL_RGB8_SNORM:
    case GL_RGBA4:
    case GL_RGB5_A1:
    case GL_SRGB_ALPHA:
    case GL_SRGB8_ALPHA8:
    case GL_RGBA:
    case GL_RGBA8:
    case GL_RGBA8_SNORM:
    case GL_R8UI:
    case GL_RG8UI:
    case GL_RGB8UI:
    case GL_RGBA8UI:
        gta_type = gta::uint8;
        break;
    case GL_R8I:
    case GL_RG8I:
    case GL_RGB8I:
    case GL_RGBA8I:
        gta_type = gta::int8;
        break;
    case GL_R16:
    case GL_R16_SNORM:
    case GL_R16F:
    case GL_R32F:
    case GL_RG16:
    case GL_RG16_SNORM:
    case GL_RG16F:
    case GL_RG32F:
    case GL_R11F_G11F_B10F:
    case GL_RGB9_E5:
    case GL_RGB16:
    case GL_RGB16_SNORM:
    case GL_RGB16F:
    case GL_RGB32F:
    case GL_RGBA16:
    case GL_RGBA16_SNORM:
    case GL_RGBA16F:
    case GL_RGBA32F:
    case GL_RGB10_A2:
#ifdef GL_RGB10_A2UI
    case GL_RGB10_A2UI:
#endif
    case GL_RGBA12:
        gta_type = gta::float32;
        break;
    case GL_R16I:
    case GL_RG16I:
    case GL_RGB16I:
    case GL_RGBA16I:
        gta_type = gta::int16;
        break;
    case GL_R16UI:
    case GL_RG16UI:
    case GL_RGB16UI:
    case GL_RGBA16UI:
        gta_type = gta::uint16;
        break;
    case GL_R32I:
    case GL_RG32I:
    case GL_RGB32I:
    case GL_RGBA32I:
        gta_type = gta::int32;
        break;
    case GL_R32UI:
    case GL_RG32UI:
    case GL_RGB32UI:
    case GL_RGBA32UI:
        gta_type = gta::uint32;
        break;
    default:
        throw exc(str::asprintf("Cannot save texture: internal format 0x%04x not handled yet", internal_format));
    }
    gta::type gta_types[components];
    for (int i = 0; i < components; i++)
        gta_types[i] = gta_type;
    hdr.set_components(components, gta_types);
    msg::dbg("%d components of gta type %d", components, static_cast<int>(gta_type));

    msg::dbg("total data size is %d", checked_cast<int>(hdr.data_size()));
    blob data(checked_cast<size_t>(hdr.data_size()));

    GLenum format = (components == 1 ? GL_RED : components == 2 ? GL_RG : components == 3 ? GL_RGB : GL_RGBA);
    GLenum type = 
          gta_type == gta::int8   ? GL_BYTE
        : gta_type == gta::uint8  ? GL_UNSIGNED_BYTE
        : gta_type == gta::int16  ? GL_SHORT
        : gta_type == gta::uint16 ? GL_UNSIGNED_SHORT
        : gta_type == gta::int32  ? GL_INT
        : gta_type == gta::uint32 ? GL_UNSIGNED_INT
        : GL_FLOAT;
    glBindTexture(GL_TEXTURE_2D, tex);
    GLint pa_bak;
    glGetIntegerv(GL_PACK_ALIGNMENT, &pa_bak);
    msg::dbg("reading texture data...");
    size_t line_size = hdr.element_size() * hdr.dimension_size(0);
    if (line_size % 4 == 0)
        glPixelStorei(GL_PACK_ALIGNMENT, 4);
    else if (line_size % 2 == 0)
        glPixelStorei(GL_PACK_ALIGNMENT, 2);
    else
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(GL_TEXTURE_2D, 0, format, type, data.ptr());
    msg::dbg("...done");
    glPixelStorei(GL_PACK_ALIGNMENT, pa_bak);
    glBindTexture(GL_TEXTURE_2D, tex_bak);

    msg::dbg("writing GTA...");
    hdr.write_to(f);
    hdr.write_data(f, data.ptr());
    msg::dbg("...done");
}

void xgl::SaveTex2D(const std::string& filename, GLuint tex)
{
    FILE* f = fio::open(filename, "w");
    SaveTex2D(f, tex);
    fio::close(f, filename);
}

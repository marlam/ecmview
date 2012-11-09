/*
 * Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011, 2012
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

#include <GL/glew.h>

#include <cctype>

#include "../../base/dbg.h"
#include "../../base/str.h"
#include "../../base/msg.h"

#include "glvm.h"
#include "xgl.h"

#include "sub-processor.h"


static int xglSkipWhitespace(const std::string& s, int i)
{
    while (isspace(s[i]))
        i++;
    return i;
}

std::string SubProcessor::xglShaderSourcePrep(const std::string& src, const std::string& defines)
{
    std::string prepped_src(src);
    int defines_index = 0;
    for (;;) {
        defines_index = xglSkipWhitespace(defines, defines_index);
        if (defines[defines_index] == '\0')
            break;
        assert(defines[defines_index] == '$');
        int name_start = defines_index;
        defines_index++;
        while (isalnum(defines[defines_index]) || defines[defines_index] == '_')
            defines_index++;
        int name_end = defines_index - 1;
        defines_index = xglSkipWhitespace(defines, defines_index);
        assert(defines[defines_index] == '=');
        defines_index = xglSkipWhitespace(defines, defines_index + 1);
        int value_start = defines_index;
        while (isalnum(defines[defines_index]) || defines[defines_index] == '_')
            defines_index++;
        int value_end = defines_index - 1;
        str::replace(prepped_src,
                std::string(defines, name_start, name_end - name_start + 1),
                std::string(defines, value_start, value_end - value_start + 1));
        defines_index = xglSkipWhitespace(defines, defines_index);
        if (defines[defines_index] == ',')
            defines_index++;
    }
    return prepped_src;
}

void SubProcessor::render_one_to_one(GLuint otex, GLuint itex)
{
    // Set up destination: modify the attachments of the current FBO
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, otex, 0);
    assert(xgl::CheckFBO(GL_DRAW_FRAMEBUFFER, HERE));
    assert(xgl::CheckError(HERE));

    // Set up source
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, itex);
    assert(xgl::CheckError(HERE));

    // Set up rendering
    /*
    glViewport(0, 0, tile_size(), tile_size());
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    */
    assert(xgl::CheckError(HERE));

    // Render
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, -1.0f);
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, -1.0f);
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();
    assert(xgl::CheckError(HERE));
}

void SubProcessor::gauss_mask(int k, float s, float* mask, float* weight_sum)
{
    assert(k >= 0);
    assert(s >= 0.0f);
    assert(mask);

    float* gauss = new float[k + 1];

    float gauss_sum = 0.0f;
    for (int i = 0; i <= k; i++) {
        gauss[i] = glvm::exp(- i * i / (2.0f * s * s))
            / (glvm::sqrt(2.0f * glvm::const_pi<float>()) * s);
        gauss_sum += gauss[i];
    }

    for (int i = 0; i <= k; i++)
        mask[i] = gauss[k - i];
    for (int i = k + 1; i < 2 * k + 1; i++)
        mask[i] = gauss[i - k];
    if (weight_sum)
        *weight_sum = 2.0f * gauss_sum - gauss[0];

    delete[] gauss;
}

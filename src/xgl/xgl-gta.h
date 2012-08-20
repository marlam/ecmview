/*
 * Copyright (C) 2011, 2012
 * Computer Graphics Group, University of Siegen, Germany.
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>.
 * See http://www.cg.informatik.uni-siegen.de/ for contact information.
 *
 * All rights reserved.
 */

#ifndef XGL_GTA
#define XGL_GTA

#include <string>
#include <cstdio>

#include <GL/glew.h>

namespace xgl
{
    void SaveTex2D(FILE* f, GLuint tex);
    void SaveTex2D(const std::string& filename, GLuint tex);
}

#endif

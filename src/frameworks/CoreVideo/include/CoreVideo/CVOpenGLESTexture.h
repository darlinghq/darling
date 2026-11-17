/*
 This file is part of Darling.

 Copyright (C) 2026 Lubos Dolezel

 Darling is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 Darling is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef COREVIDEO_CVOPENGLESTEXTURE_H
#define COREVIDEO_CVOPENGLESTEXTURE_H

#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVImageBuffer.h>
#include <OpenGLES/gltypes.h>
#include <stddef.h>
#include <stdint.h>

#if defined(__cplusplus)
extern "C" {
#endif

// An OpenGL ES texture made from an image buffer by a CVOpenGLESTextureCache. It is a
// CVImageBuffer, so CVBuffer attachments work on it.
typedef CVImageBufferRef CVOpenGLESTextureRef;

CV_EXPORT CFTypeID CVOpenGLESTextureGetTypeID(void);

// The texture target (GL_TEXTURE_2D) and the name of the texture.
CV_EXPORT GLenum CVOpenGLESTextureGetTarget(CVOpenGLESTextureRef CV_NONNULL image);
CV_EXPORT GLuint CVOpenGLESTextureGetName(CVOpenGLESTextureRef CV_NONNULL image);

// True when the first row of the texture is the top of the image.
CV_EXPORT Boolean CVOpenGLESTextureIsFlipped(CVOpenGLESTextureRef CV_NONNULL image);

// The texture coordinates of the corners of the image, which take the flip into account.
CV_EXPORT void CVOpenGLESTextureGetCleanTexCoords(CVOpenGLESTextureRef CV_NONNULL image,
	GLfloat lowerLeft[CV_NONNULL 2], GLfloat lowerRight[CV_NONNULL 2],
	GLfloat upperRight[CV_NONNULL 2], GLfloat upperLeft[CV_NONNULL 2]);

#if defined(__cplusplus)
}
#endif

#endif

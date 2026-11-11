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

#ifndef CVOPENGLTEXTURE_H
#define CVOPENGLTEXTURE_H

#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVImageBuffer.h>
#include <OpenGL/gl.h>

__BEGIN_DECLS

// An OpenGL texture made from an image buffer by a CVOpenGLTextureCache. It is a CVImageBuffer,
// so CVBuffer attachments work on it.
typedef CVImageBufferRef CVOpenGLTextureRef;

CV_EXPORT CFTypeID CVOpenGLTextureGetTypeID(void);
CV_EXPORT CVOpenGLTextureRef CV_NULLABLE CVOpenGLTextureRetain(CVOpenGLTextureRef CV_NULLABLE texture);
CV_EXPORT void CVOpenGLTextureRelease(CVOpenGLTextureRef CV_NULLABLE texture);

// The texture target (GL_TEXTURE_RECTANGLE) and the name of the texture.
CV_EXPORT GLenum CVOpenGLTextureGetTarget(CVOpenGLTextureRef CV_NONNULL image);
CV_EXPORT GLuint CVOpenGLTextureGetName(CVOpenGLTextureRef CV_NONNULL image);

// Whether the first row of the texture is the top of the image.
CV_EXPORT Boolean CVOpenGLTextureIsFlipped(CVOpenGLTextureRef CV_NONNULL image);

// The coordinates of the corners of the image in the texture, in pixels.
CV_EXPORT void CVOpenGLTextureGetCleanTexCoords(CVOpenGLTextureRef CV_NONNULL image,
	GLfloat lowerLeft[CV_NONNULL 2], GLfloat lowerRight[CV_NONNULL 2], GLfloat upperRight[CV_NONNULL 2],
	GLfloat upperLeft[CV_NONNULL 2]);

__END_DECLS

#endif

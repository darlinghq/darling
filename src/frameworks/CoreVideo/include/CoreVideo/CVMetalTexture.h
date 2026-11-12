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

#ifndef CVMETALTEXTURE_H
#define CVMETALTEXTURE_H

#include <sys/cdefs.h>
#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVBase.h>
#include <CoreVideo/CVReturn.h>
#include <CoreVideo/CVImageBuffer.h>

__BEGIN_DECLS

CV_EXPORT const CFStringRef CV_NONNULL kCVMetalTextureUsage;
CV_EXPORT const CFStringRef CV_NONNULL kCVMetalTextureStorageMode;

CV_EXPORT CFTypeID CVMetalTextureGetTypeID(void);

#if defined(__OBJC__)
@protocol MTLTexture;

typedef CVImageBufferRef CVMetalTextureRef;

// There is no Metal texture interop, so no CVMetalTexture object can be created
// and the returned texture is always nil.
CV_EXPORT id <MTLTexture> CV_NULLABLE CVMetalTextureGetTexture(CVMetalTextureRef CV_NONNULL image);
#endif

__END_DECLS

#endif

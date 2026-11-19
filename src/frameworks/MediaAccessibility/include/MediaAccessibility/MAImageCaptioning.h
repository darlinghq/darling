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


#ifndef MediaAccessibility_MAImageCaptioning_h
#define MediaAccessibility_MAImageCaptioning_h

#include <MediaAccessibility/MADefinitions.h>
#include <CoreFoundation/CoreFoundation.h>

MA_EXTERN_C_BEGIN
CF_ASSUME_NONNULL_BEGIN

// The XMP tag path under which an image's accessibility caption is stored.
MA_EXPORT CFStringRef MAImageCaptioningCopyMetadataTagPath(void) CF_RETURNS_RETAINED;

CF_ASSUME_NONNULL_END
MA_EXTERN_C_END

#endif

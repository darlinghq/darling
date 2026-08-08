/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

#ifndef _LS_ICONSCORE_H
#define _LS_ICONSCORE_H

#include <CoreServices/CoreServices.h>
#include <OSServices/OSServices.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OpaqueIconRef *IconRef;

extern OSErr AcquireIconRef(IconRef theIconRef);
extern OSErr GetIconRef(SInt16 vRefNum, OSType creator, OSType iconType, IconRef *theIconRef);
extern OSStatus GetIconRefFromIconFamilyPtr(const IconFamilyResource *inIconFamilyPtr, Size inSize, IconRef *outIconRef);
extern OSStatus ReadIconFromFSRef(const FSRef *ref, IconFamilyHandle *iconFamily);
extern OSErr ReleaseIconRef(IconRef theIconRef);

#ifdef __cplusplus
}
#endif

#endif

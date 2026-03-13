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

#ifndef _CS_ICONSTORAGE_H
#define _CS_ICONSTORAGE_H

#include <CoreServices/CoreServices.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct IconFamilyElement {
    OSType elementType;
    SInt32 elementSize;
    unsigned char elementData[1];
} IconFamilyElement;
typedef struct IconFamilyResource {
    OSType resourceType;
    SInt32 resourceSize;
    IconFamilyElement elements[1];
} IconFamilyResource;
typedef IconFamilyResource *IconFamilyPtr;
typedef IconFamilyPtr *IconFamilyHandle;

#ifdef __cplusplus
}
#endif

#endif

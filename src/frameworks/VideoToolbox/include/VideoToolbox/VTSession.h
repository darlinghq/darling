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

#ifndef _VTSESSION_H_
#define _VTSESSION_H_

#include <VideoToolbox/VTBase.h>

CF_IMPLICIT_BRIDGING_ENABLED

typedef CFTypeRef VTSessionRef;

VT_EXPORT OSStatus VTSessionSetProperty(VTSessionRef session, CFStringRef propertyKey, CFTypeRef propertyValue);
VT_EXPORT OSStatus VTSessionSetProperties(VTSessionRef session, CFDictionaryRef propertyDictionary);
VT_EXPORT OSStatus VTSessionCopyProperty(VTSessionRef session, CFStringRef propertyKey, CFAllocatorRef allocator,
	void *propertyValueOut);
VT_EXPORT OSStatus VTSessionCopySupportedPropertyDictionary(VTSessionRef session,
	CF_RETURNS_RETAINED CFDictionaryRef *supportedPropertyDictionaryOut);

CF_IMPLICIT_BRIDGING_DISABLED

#endif

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

#ifndef _CMIOHARDWARE_H_
#define _CMIOHARDWARE_H_

#include <MacTypes.h>

typedef UInt32 CMIOObjectID;
typedef UInt32 CMIOClassID;
typedef UInt32 CMIOObjectPropertySelector;
typedef UInt32 CMIOObjectPropertyScope;
typedef UInt32 CMIOObjectPropertyElement;
typedef CMIOObjectPropertySelector CMIOHardwarePropertyID;

typedef struct CMIOObjectPropertyAddress
{
	CMIOObjectPropertySelector mSelector;
	CMIOObjectPropertyScope mScope;
	CMIOObjectPropertyElement mElement;
} CMIOObjectPropertyAddress;

// Four-character codes are written as hexadecimal values so that their
// numeric value does not depend on the host byte order.
enum
{
	kCMIOObjectUnknown = 0,
	kCMIOObjectSystemObject = 1,

	kCMIOObjectPropertySelectorWildcard = 0x2a2a2a2a, // '****'
	kCMIOObjectPropertyScopeWildcard = 0x2a2a2a2a,    // '****'
	kCMIOObjectPropertyElementWildcard = 0xffffffff,
	kCMIOObjectPropertyScopeGlobal = 0x676c6f62,      // 'glob'
	kCMIOObjectPropertyElementMain = 0,
	kCMIOObjectPropertyElementMaster = 0,

	kCMIOHardwarePropertyAllowScreenCaptureDevices = 0x79657320, // 'yes '
	kCMIOHardwarePropertyAllowWirelessScreenCaptureDevices = 0x77736364, // 'wscd'

	kCMIOHardwareNoError = 0,
	kCMIOHardwareUnspecifiedError = 0x77686174,         // 'what'
	kCMIOHardwareUnknownPropertyError = 0x77686f3f,     // 'who?'
	kCMIOHardwareBadPropertySizeError = 0x2173697a,     // '!siz'
	kCMIOHardwareIllegalOperationError = 0x6e6f7065,    // 'nope'
	kCMIOHardwareBadObjectError = 0x216f626a,           // '!obj'
	kCMIOHardwareUnsupportedOperationError = 0x756e6f70 // 'unop'
};

Boolean CMIOObjectHasProperty(CMIOObjectID objectID, const CMIOObjectPropertyAddress *address);

OSStatus CMIOObjectIsPropertySettable(CMIOObjectID objectID, const CMIOObjectPropertyAddress *address,
                                      Boolean *isSettable);

OSStatus CMIOObjectGetPropertyDataSize(CMIOObjectID objectID, const CMIOObjectPropertyAddress *address,
                                       UInt32 qualifierDataSize, const void *qualifierData, UInt32 *dataSize);

OSStatus CMIOObjectGetPropertyData(CMIOObjectID objectID, const CMIOObjectPropertyAddress *address,
                                   UInt32 qualifierDataSize, const void *qualifierData, UInt32 dataSize,
                                   UInt32 *dataUsed, void *data);

OSStatus CMIOObjectSetPropertyData(CMIOObjectID objectID, const CMIOObjectPropertyAddress *address,
                                   UInt32 qualifierDataSize, const void *qualifierData, UInt32 dataSize,
                                   const void *data);

#endif

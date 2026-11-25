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

// Windows from nibs. The nib reader gives the description of the IBCarbonWindow object with its
// controls (see IBCarbonNib.h); making the window and the controls out of it is up to the Window
// Manager and the Control Manager, through _HICreateWindowFromNibDescription(), which fails here
// until they provide it.

#include <HIToolbox/IBCarbonRuntime.h>
#include <CarbonCore/MacErrors.h>
#include "IBCarbonNib.h"

OSStatus _HICreateWindowFromNibDescription(CFDictionaryRef description, WindowRef* outWindow)
{
	*outWindow = NULL;
	return unimpErr;
}

OSStatus CreateWindowFromNib(IBNibRef inNibRef, CFStringRef inName, WindowRef* outWindow)
{
	if (!inNibRef || !inName || !outWindow)
		return paramErr;
	*outWindow = NULL;

	CFDictionaryRef description = _IBCarbonNibCopyObjectNamed(inNibRef, inName);

	if (!description)
		return kIBCarbonRuntimeCantFindObject;

	CFStringRef className = CFDictionaryGetValue(description, CFSTR("class"));
	OSStatus status;

	if (!className || CFStringCompare(className, CFSTR("IBCarbonWindow"), 0) != kCFCompareEqualTo)
		status = kIBCarbonRuntimeObjectNotOfRequestedType;
	else
		status = _HICreateWindowFromNibDescription(description, outWindow);
	CFRelease(description);
	return status;
}

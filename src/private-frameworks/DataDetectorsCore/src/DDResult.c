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

#include <CoreFoundation/CoreFoundation.h>
#include <CoreFoundation/CFRuntime.h>
#include <dispatch/dispatch.h>
#include <DataDetectorsCore/DDResult.h>

// A DDResult is a CoreFoundation object, so clients can tell one from other
// CF objects by its type ID (CFGetTypeID). The scanner that would create
// results is not implemented yet, so no instances are made; the class is
// registered so the type ID is a real one of its own.
struct __DDResult {
	CFRuntimeBase base;
};

static const CFRuntimeClass DDResultClass = {
	0,
	"DDResult",
	NULL, // init
	NULL, // copy
	NULL, // finalize
	NULL, // equal
	NULL, // hash
	NULL, // copyFormattingDesc
	NULL, // copyDebugDesc
};

CFTypeID DDResultGetCFTypeID(void)
{
	static CFTypeID typeID = _kCFRuntimeNotATypeID;
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		typeID = _CFRuntimeRegisterClass(&DDResultClass);
	});
	return typeID;
}

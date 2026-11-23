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

#ifndef _CMCLASS_H_
#define _CMCLASS_H_

#include <CoreFoundation/CFRuntime.h>
#include <dispatch/dispatch.h>

// Defines and registers (on first use) a CF class, as name##GetTypeID(). Not part of the public interface.
#define CM_CLASS(name, cname, ...) \
	static CFTypeID name##TypeID = _kCFRuntimeNotATypeID; \
	static const CFRuntimeClass name##Class = { .version = 0, .className = cname, __VA_ARGS__ }; \
	static CFTypeID name##GetTypeID(void) \
	{ \
		static dispatch_once_t once; \
		dispatch_once(&once, ^{ name##TypeID = _CFRuntimeRegisterClass(&name##Class); }); \
		return name##TypeID; \
	}

// Creates an instance of a class defined with CM_CLASS; the memory past the CFRuntimeBase is zeroed.
#define CM_NEW(type, name, allocator) \
	((type)_CFRuntimeCreateInstance(allocator, name##GetTypeID(), sizeof(*(type)NULL) - sizeof(CFRuntimeBase), NULL))

#endif

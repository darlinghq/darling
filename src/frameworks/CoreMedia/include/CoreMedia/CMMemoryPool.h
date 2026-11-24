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

#ifndef _CMMEMORYPOOL_H_
#define _CMMEMORYPOOL_H_

#include <CoreMedia/CMBase.h>

// A pool that holds on to recently deallocated memory so that the next allocation of the same size is fast. It
// works in whole pages, so it suits large blocks only. Its CFAllocator is what clients allocate through.

CF_IMPLICIT_BRIDGING_ENABLED

typedef struct CF_BRIDGED_TYPE(id) OpaqueCMMemoryPool *CMMemoryPoolRef;

CM_EXPORT CFTypeID CMMemoryPoolGetTypeID(void);

// The option that sets how long, in seconds, the pool keeps a block that nobody has asked for again before it
// frees it. Its value is a CFNumber, in the options dictionary of CMMemoryPoolCreate.
CM_EXPORT const CFStringRef kCMMemoryPoolOption_AgeOutPeriod;

// The options dictionary may be NULL, for the defaults.
CM_EXPORT CMMemoryPoolRef CMMemoryPoolCreate(CFDictionaryRef options);

// The allocator of the pool, not retained. It can outlive the pool, and then allocates without pooling.
CM_EXPORT CFAllocatorRef CMMemoryPoolGetAllocator(CMMemoryPoolRef pool);

// Frees all the memory the pool holds for reuse.
CM_EXPORT void CMMemoryPoolFlush(CMMemoryPoolRef pool);

// Stops the pool from recycling, and frees the memory it holds.
CM_EXPORT void CMMemoryPoolInvalidate(CMMemoryPoolRef pool);

CF_IMPLICIT_BRIDGING_DISABLED

#endif

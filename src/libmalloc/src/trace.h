/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

#ifndef __TRACE_H
#define __TRACE_H

// <sys/kdebug.h> defines these two subclasses for us:
//   DBG_UMALLOC_EXTERNAL - for external entry points into malloc
//   DBG_UMALLOC_INTERNAL - for tracing internal malloc state

#ifndef _MALLOC_BUILDING_CODES_
#include <sys/kdebug.h>
#define MALLOC_TRACE(code,arg1,arg2,arg3,arg4) \
	{ if (malloc_tracing_enabled) { kdebug_trace(code, arg1, arg2, arg3, arg4); } }
#define TRACE_CODE(name, subclass, code) \
	static const int TRACE_##name = KDBG_EVENTID(DBG_UMALLOC, subclass, code)
#else
# define DBG_UMALLOC 51
# define DBG_UMALLOC_EXTERNAL 0x1
# define DBG_UMALLOC_INTERNAL 0x2
# define STR(x) #x
# define TRACE_CODE(name, subclass, code) \
	printf("0x%x\t%s\n", ((DBG_UMALLOC << 24) | ((subclass & 0xff) << 16) | ((code & 0x3fff) << 2)), STR(name))
#endif

// "external" trace points
TRACE_CODE(malloc, DBG_UMALLOC_EXTERNAL, 0x01);
TRACE_CODE(free, DBG_UMALLOC_EXTERNAL, 0x02);
TRACE_CODE(realloc, DBG_UMALLOC_EXTERNAL, 0x03);
TRACE_CODE(memalign, DBG_UMALLOC_EXTERNAL, 0x04);

// "internal" trace points
TRACE_CODE(nano_malloc, DBG_UMALLOC_INTERNAL, 0x1);
TRACE_CODE(tiny_malloc, DBG_UMALLOC_INTERNAL, 0x2);
TRACE_CODE(small_malloc, DBG_UMALLOC_INTERNAL, 0x3);
TRACE_CODE(large_malloc, DBG_UMALLOC_INTERNAL, 0x4);
TRACE_CODE(nano_free, DBG_UMALLOC_INTERNAL, 0x5);
TRACE_CODE(tiny_free, DBG_UMALLOC_INTERNAL, 0x6);
TRACE_CODE(small_free, DBG_UMALLOC_INTERNAL, 0x7);
TRACE_CODE(large_free, DBG_UMALLOC_INTERNAL, 0x8);

#endif // __TRACE_H

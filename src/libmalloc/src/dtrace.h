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

#ifndef __DTRACE_H
#define __DTRACE_H

#ifndef DARWINTEST
#include "magmallocProvider.h"
#else
#define	MAGMALLOC_ALLOCREGION(arg0, arg1, arg2, arg3)
#define	MAGMALLOC_ALLOCREGION_ENABLED() (0)
#define	MAGMALLOC_DEALLOCREGION(arg0, arg1, arg2)
#define	MAGMALLOC_DEALLOCREGION_ENABLED() (0)
#define	MAGMALLOC_DEPOTREGION(arg0, arg1, arg2, arg3, arg4)
#define	MAGMALLOC_DEPOTREGION_ENABLED() (0)
#define	MAGMALLOC_MADVFREEREGION(arg0, arg1, arg2, arg3)
#define	MAGMALLOC_MADVFREEREGION_ENABLED() (0)
#define	MAGMALLOC_MALLOCERRORBREAK()
#define	MAGMALLOC_MALLOCERRORBREAK_ENABLED() (0)
#define	MAGMALLOC_PRESSURERELIEFBEGIN(arg0, arg1, arg2)
#define	MAGMALLOC_PRESSURERELIEFBEGIN_ENABLED() (0)
#define	MAGMALLOC_PRESSURERELIEFEND(arg0, arg1, arg2, arg3)
#define	MAGMALLOC_PRESSURERELIEFEND_ENABLED() (0)
#define	MAGMALLOC_RECIRCREGION(arg0, arg1, arg2, arg3, arg4)
#define	MAGMALLOC_RECIRCREGION_ENABLED() (0)
#define	MAGMALLOC_REFRESHINDEX(arg0, arg1, arg2)
#define	MAGMALLOC_REFRESHINDEX_ENABLED() (0)
#endif

#endif // __DTRACE_H

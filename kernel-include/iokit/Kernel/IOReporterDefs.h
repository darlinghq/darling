/*
 * Copyright (c) 2012-2013 Apple Computer, Inc.  All Rights Reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _IOEPORTERDEFS_H
#define _IOEPORTERDEFS_H

//#include "IOReportHubCommon.h"

//#define IORDEBUG_IOLOG

#if defined(IORDEBUG_IOLOG)
#define IORLOG(fmt, args...)    \
do {                            \
    IOLog((fmt), ##args);         \
    IOLog("\n");                \
} while(0)

#else
#define IORLOG(fmt, args...)
#endif

#define IORERROR_LOG

#ifdef IORERROR_LOG
#define IORERROR(fmt, args...) IOLog(fmt, ##args);
#else
#define IORERROR(fmt, args...)
#endif

// overflow detection routines
#if (SIZE_T_MAX < INT_MAX)
#error "(SIZE_T_MAX < INT_MAX) -> PREFL_MEMOP_*()) unsafe for size_t"
#endif

#define PREFL_MEMOP_FAIL(__val, __type) do {  \
    if (__val <= 0) {  \
	IORERROR("%s - %s <= 0!\n", __func__, #__val);  \
	res = kIOReturnUnderrun;  \
	goto finish;  \
    }  else if (__val > INT_MAX / (int)sizeof(__type)) {  \
	IORERROR("%s - %s > INT_MAX / sizeof(%s)!\n",__func__,#__val,#__type);\
	res = kIOReturnOverrun;  \
	goto finish;  \
    }  \
} while(0)

#define PREFL_MEMOP_PANIC(__val, __type) do {  \
    if (__val <= 0) {  \
	panic("%s - %s <= 0!", __func__, #__val);  \
    }  else if (__val > INT_MAX / (int)sizeof(__type)) {  \
	panic("%s - %s > INT_MAX / sizeof(%s)!", __func__, #__val, #__type);  \
    }  \
} while(0)

//#include "IOReportHubCommon.h"//



#define IOREPORTER_DEBUG_ELEMENT(idx)                                   \
do {                                                                    \
IOLog("IOReporter::DrvID: %llx | Elt:[%3d] |ID: %llx |Ticks: %llu |",   \
_elements[idx].provider_id,                                             \
idx,                                                                    \
_elements[idx].channel_id,                                              \
_elements[idx].timestamp);                                              \
IOLog("0: %llu | 1: %llu | 2: %llu | 3: %llu\n",                        \
_elements[idx].values.v[0],                                             \
_elements[idx].values.v[1],                                             \
_elements[idx].values.v[2],                                             \
_elements[idx].values.v[3]);                                            \
} while(0)


#define IOREPORTER_CHECK_LOCK()                                         \
do {                                                                    \
    if (!_reporterIsLocked) {                                           \
	panic("%s was called out of locked context!", __PRETTY_FUNCTION__); \
    }                                                                   \
} while(0)                                                              \

#define IOREPORTER_CHECK_CONFIG_LOCK()                                  \
do {                                                                    \
    if (!_reporterConfigIsLocked) {                                     \
	panic("%s was called out of config locked context!", __PRETTY_FUNCTION__); \
    }                                                                   \
} while(0)                                                              \

#endif /* ! _IOEPORTERDEFS_H */

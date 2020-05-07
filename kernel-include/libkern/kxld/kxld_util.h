/*
 * Copyright (c) 2007-2008 Apple Inc. All rights reserved.
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
#ifndef _KXLD_UTIL_H_
#define _KXLD_UTIL_H_

#include <sys/types.h>
#if KERNEL
    #include <libkern/kxld_types.h>
    #include <mach/machine.h>
#else
    #include <architecture/byte_order.h>
    #include "kxld_types.h"

/* Get machine.h from the kernel source so we can support all platforms
 * that the kernel supports. Otherwise we're at the mercy of the host.
 */
    #include "../../osfmk/mach/machine.h"
#endif

/* 64-bit helpers */
#if !defined(KERNEL)

    #define KXLD_3264_FUNC(cond32, rval, func32, func64, ...) \
    do {                        \
	if (cond32) {           \
	    (rval) = (func32)(__VA_ARGS__); \
	} else {                \
	    (rval) = (func64)(__VA_ARGS__); \
	}                       \
    } while(0)

#elif defined(__LP64__)

    #define KXLD_3264_FUNC(cond32, rval, func32, func64, ...) \
    do {                        \
	(rval) = (func64)(__VA_ARGS__);     \
    } while(0)

#else

    #define KXLD_3264_FUNC(cond32, rval, func32, func64, ...) \
    do {                        \
	(rval) = (func32)(__VA_ARGS__);     \
    } while(0)                  \

#endif

/* Misc string functions */
#define streq(str1, str2) (((str1) && (str2)) ? !strcmp((str1), (str2)) : 0)
#define streq_safe(str1, str2, len) (((str1) && (str2)) ? \
    !strncmp((str1), (str2), (len)) : 0)
#define const_strlen(str) (sizeof(str) - 1)

#define const_array_len(array) sizeof(array) / sizeof(*array)

/* Timing */
#define DECL_TIMER()   struct timeval start, end;
#define START_TIMER()  gettimeofday(&start, NULL);
#define END_TIMER()    gettimeofday(&end, NULL);
#define PRINT_TIMER(msg)  kxld_log("%s: %ds, %dus\n", (msg), \
	(end.tv_sec - start.tv_sec), (end.tv_usec - start.tv_usec));

/* Misc definitions */
#define KXLD_MAX_NAME_LEN                       256
#define KXLD_SEG_GOT                            "__DATA"
#define KXLD_SECT_GOT                           "__kxld_got"
#define KXLD_KMOD_INFO_SYMBOL                   "_kmod_info"
#define KXLD_WEAK_TEST_SYMBOL                   "_gOSKextUnresolved"
#define KXLD_OPERATOR_NEW_SYMBOL                "__Znwm"
#define KXLD_OPERATOR_NEW_ARRAY_SYMBOL          "__Znam"
#define KXLD_OPERATOR_DELETE_SYMBOL             "__ZdlPv"
#define KXLD_OPERATOR_DELETE_ARRAY_SYMBOL       "__ZdaPv"

struct kxld_section_name {
	char segname[16];
	char sectname[16];
};
typedef struct kxld_section_name KXLDSectionName;

/*******************************************************************************
* Logging
*******************************************************************************/

void kxld_set_logging_callback(KXLDLoggingCallback logging_callback)
__attribute__((visibility("hidden")));

void kxld_set_logging_callback_data(const char * name, void *user_data)
__attribute__((visibility("hidden")));

void kxld_log(KXLDLogSubsystem subsystem, KXLDLogLevel level,
    const char *format, ...)
__attribute__((visibility("hidden"), format(printf, 3, 4)));

/* Common logging strings */
#define kKxldLogArchNotSupported        "The target architecture (cputype 0x%x) is not supported by kxld."
#define kKxldLogArchNotFound            "The kext does not contain a fat slice for the target architecture."
#define kKxldLogFiletypeNotSupported    "The Mach-O filetype 0x%x is not supported on the target architecture."
#define kKxldLogTruncatedMachO          "The Mach-O file has been truncated. Make sure the Mach-O header structures are correct."
#define kKxldLogMalformedMachO          "The Mach-O file is malformed: "
#define kKxldLogMalformedVTable         "The vtable '%s' is malformed. Make sure your kext has been built against the correct headers."
#define kKxldLogMissingVtable           "Cannot find the vtable '%s' for class '%s'. This vtable symbol is required for binary compatibility, and it may have been stripped."
#define kKxldLogDirectPureVirtualCall   "This kext calls a pure virtual function. Make sure your kext's OSObject-derived classes implement all pure virtual functions."
#define kKxldLogParentOutOfDate         "The super class vtable '%s' for vtable '%s' is out of date. Make sure your kext has been built against the correct headers."
#define kKxldLogNoKmodInfo              "The kext is missing its kmod_info structure."
#define kKxldLogRelocationOverflow      "A relocation entry has overflowed. The kext may be too far from one " \
    "of its dependencies. Check your kext's load address."
#define kKxldLogRelocatingPatchedSym    "Warning: relocation failed because some class in this kext "    \
    "didn't use the OSDeclareDefaultStructors and OSDefineMetaClassAndStructors, "  \
    "or didn't export all vtable symbols, so it still "  \
    "references %s, which has been patched with another symbol for binary compatibility. "      \
    "Please make sure all classes that inherit from OSObject use these macros."

/*******************************************************************************
* Allocators
*******************************************************************************/

void * kxld_calloc(size_t size)
__attribute__((malloc, visibility("hidden")));

void * kxld_alloc(size_t size)
__attribute__((malloc, visibility("hidden")));

void * kxld_page_alloc(size_t size)
__attribute__((malloc, visibility("hidden")));

void * kxld_page_alloc_untracked(size_t size)
__attribute__((malloc, visibility("hidden")));

void * kxld_alloc_pageable(size_t size)
__attribute__((malloc, visibility("hidden")));

/*******************************************************************************
* Deallocators
*******************************************************************************/

void kxld_free(void *ptr, size_t size)
__attribute__((visibility("hidden")));

void kxld_page_free(void *ptr, size_t size)
__attribute__((visibility("hidden")));

void kxld_page_free_untracked(void *ptr, size_t size)
__attribute__((visibility("hidden")));

/*******************************************************************************
* Mach-O Functions
*******************************************************************************/

kern_return_t validate_and_swap_macho_32(u_char *file, u_long size
#if !KERNEL
    , enum NXByteOrder host_order
#endif /* !KERNEL */
    ) __attribute__((visibility("hidden")));

kern_return_t validate_and_swap_macho_64(u_char *file, u_long size
#if !KERNEL
    , enum NXByteOrder host_order
#endif /* !KERNEL */
    ) __attribute__((visibility("hidden")));

#if !KERNEL
void unswap_macho(u_char *file, enum NXByteOrder host_order,
    enum NXByteOrder target_order)
__attribute__((visibility("hidden")));
#endif /* !KERNEL */

/*******************************************************************************
* Miscellaneous
*******************************************************************************/

kxld_addr_t kxld_align_address(kxld_addr_t address, u_int align)
__attribute__((const, visibility("hidden")));

boolean_t kxld_is_32_bit(cpu_type_t)
__attribute__((const, visibility("hidden")));

const char * kxld_strstr(const char *s, const char *find)
__attribute__((pure, visibility("hidden")));

/*******************************************************************************
* Debugging
*******************************************************************************/

void kxld_print_memory_report(void)
__attribute__((visibility("hidden")));

/*******************************************************************************
* Cross Linking
*******************************************************************************/
#if !KERNEL
boolean_t kxld_set_cross_link_page_size(kxld_size_t target_page_size);
#endif /* !KERNEL */
kxld_size_t kxld_get_effective_page_size(void);
kxld_addr_t kxld_round_page_cross_safe(kxld_addr_t addr);

#if SPLIT_KEXTS_DEBUG
void kxld_show_split_info(splitKextLinkInfo *info);
#endif

#endif /* _KXLD_UTIL_H_ */

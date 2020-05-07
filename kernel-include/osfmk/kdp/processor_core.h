/*
 * Copyright (c) 2017 Apple Computer, Inc. All rights reserved.
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
#ifndef _PROCESSOR_CORE_H_
#define _PROCESSOR_CORE_H_

#include <stdint.h>
#include <mach/vm_types.h>
#include <mach/kern_return.h>
#include <mach/machine.h>
#include <mach_debug/mach_debug_types.h>

__BEGIN_DECLS

/*
 * Kernel support for generating corefiles on device.
 *
 * The kernel provides support for co-operatively generating core files
 * for any co/processors that register a coredump handler callback.
 *
 * The kernel will use the provided callbacks to generate a compressed
 * corefile in a file on disk.
 *
 * Corefiles consist of three main sections
 *      -- The headers that describe the corefile -- number of segments, etc
 *      -- The segment commands that describe the data in the corefile
 *      -- The segment data
 *
 * When a coredump handler is registered, a pointer to a kern_coredump_callback_config
 * structure is provided with callbacks that will be called as part of generating the
 * coredump.
 *
 * It's expected that each of these callbacks will return 0 on success (and non-zero on
 * error).
 */

void kern_coredump_log(void *context, const char *string, ...) __printflike(2, 3);

/*
 * The core_save_summary callback is provided with the call to the kcc_coredump_get_summary
 * routine that was registered. The caller should provide the following
 *
 * core_segment_count   -- Number of segments (LC_SEGMENT_KERNEL) that will be recorded
 * core_byte_count      -- Overall length of all data to be included across all segments
 * thread_count         -- Number of threads that will be recorded with thread state (LC_THREAD)
 * thread_state_size    -- Size of a thread's saved state (should be the overall LC_THREAD command size)
 * misc_bytes_count     -- Length of misc data that will be included in the core
 * mh_magic             -- mh_magic to be included in corefile
 * cpu_type             -- CPU type
 * cpu_subtype          -- CPU subtype
 * context              -- Passed to kcc_coredump_get_summary_routine
 */
typedef kern_return_t (*core_save_summary_cb)(uint64_t core_segment_count, uint64_t core_byte_count,
    uint64_t thread_count, uint64_t thread_state_size,
    uint64_t misc_bytes_count, void *context);

/*
 * The core_save_segment_descriptions callback is provided with the call to the
 * kcc_coredump_save_segment_descriptions routine that was registered.
 *
 * It's expected that the caller should iterate all of the segments they want to include in
 * the corefile and call the callback with the following for each:
 *
 * Please note that seg_end is address of the byte immediately following the last byte in the segment.
 * For example, if a segment spans addresses 0x1000 to 0x1FFF, seg_end would be 0x2000.
 *
 * seg_start -- Start of the segment in the core's address space
 * seg_end   -- End of the segment in the core's address space
 * context   -- Passed to kcc_coredump_save_segment_descriptions routine
 */
typedef kern_return_t (*core_save_segment_descriptions_cb)(uint64_t seg_start, uint64_t seg_end,
    void *context);
/*
 * The core_save_thread_state callback is provided with the call to the
 * kcc_coredump_save_thread_state routine that was registered.
 *
 * The routine is provided a pointer to a buffer of thread_state_size (as specified
 * previously) that can be used to populate thread state.
 *
 * It's expected that the caller should iterate all of the threads
 * that they would like to include and call the callback with the following
 * for each:
 *
 * thread_state -- A pointer to the buffer with an LC_THREAD command
 * context      -- Passed to kcc_coredump_save_thread_state routine
 */
typedef kern_return_t (*core_save_thread_state_cb)(void *thread_state, void *context);

/*
 * The core_save_sw_vers callback is provided with the call to the
 * kcc_coredump_save_sw_vers routine that was registered.
 *
 * The caller should call the callback with the following:
 *
 * sw_vers -- A pointer the software version information
 * length  -- Length of the software version information to be copied (< KERN_COREDUMP_VERSIONSTRINGMAXSIZE)
 * context -- Passed to kcc_coredump_save_sw_vers routine
 */
typedef kern_return_t (*core_save_sw_vers_cb)(void *sw_vers, uint64_t length, void *context);

/*
 * The core_save_segment_data callback is provided with the call to the
 * kcc_coredump_save_segment_data routine that was registered.
 *
 * It's expected that the caller should iterate all of the segments they want to include in
 * the corefile and call the callback with the following for each:
 *
 * seg_data -- A pointer to the segment data (mapped in the kernel's address space)
 * length   -- Length of the data to be copied from the segment
 * context  -- Passed to kcc_coredump_save_segment_data routine
 */
typedef kern_return_t (*core_save_segment_data_cb)(void *seg_data, uint64_t length, void *context);

/*
 *  ---------------------- OPTIONAL -------------------------
 * The core_save_misc_data callback is provided with the call to the
 * kcc_coredump_save_misc_data routine that was registered
 *
 * The caller should call the callback with the following:
 *
 * misc_data -- A pointer to the data to be copied
 * length    -- The length of the data to be copied
 * context   -- Passed to kcc_coredump_save_misc_data routine
 */
typedef kern_return_t (*core_save_misc_data_cb)(void *misc_data, uint64_t length, void *context);

typedef struct {
	kern_return_t (*kcc_coredump_init)(void *refcon, void *context); /* OPTIONAL -- return KERN_NODE_DOWN if the co-processor should be skipped */
	kern_return_t (*kcc_coredump_get_summary)(void *refcon, core_save_summary_cb callback, void *context);
	kern_return_t (*kcc_coredump_save_segment_descriptions)(void *refcon, core_save_segment_descriptions_cb callback, void *context);
	kern_return_t (*kcc_coredump_save_thread_state)(void *refcon, void *buf, core_save_thread_state_cb callback, void *context);
	kern_return_t (*kcc_coredump_save_sw_vers)(void *refcon, core_save_sw_vers_cb callback, void *context);
	kern_return_t (*kcc_coredump_save_segment_data)(void *refcon, core_save_segment_data_cb callback, void *context);
	kern_return_t (*kcc_coredump_save_misc_data)(void *refcon, core_save_misc_data_cb callback, void *context); /* OPTIONAL */
	/* End of version 1 */
} kern_coredump_callback_config;

#define KERN_COREDUMP_MAX_CORES MACH_CORE_FILEHEADER_MAXFILES
#define KERN_COREDUMP_MIN_CONFIG_VERSION 1
#define KERN_COREDUMP_CONFIG_VERSION 1
#define KERN_COREDUMP_VERSIONSTRINGMAXSIZE 256

/*
 * kern_register_coredump_helper is called to register a core with the kernel
 * coredump infrastructure. In addition to the callback config and version of the config
 * structure, a description of the core should be provided -- i.e.: AP
 */
kern_return_t kern_register_coredump_helper(int kern_coredump_config_vers, const kern_coredump_callback_config *kc_callbacks, void *refcon,
    const char *core_description, boolean_t is64bit, uint32_t mh_magic, cpu_type_t cpu_type, cpu_subtype_t cpu_subtype);

#if PRIVATE

kern_return_t kern_register_xnu_coredump_helper(kern_coredump_callback_config *kc_callbacks);

kern_return_t kern_do_coredump(void *core_outvars, boolean_t kernel_only, uint64_t first_file_offset, uint64_t *last_file_offset);

#define KERN_COREDUMP_HEADERSIZE 4096
static_assert((sizeof(struct mach_core_fileheader) <= KERN_COREDUMP_HEADERSIZE), "struct mach_core_fileheader larger than KERN_COREDUMP_HEADERSIZE (space that will be allocated for it in the corefile)");

#define KERN_COREDUMP_MAXDEBUGLOGSIZE 16384
#define KERN_COREDUMP_BEGIN_FILEBYTES_ALIGN 4096
#define KERN_COREDUMP_THREADSIZE_MAX 1024

#endif /* PRIVATE */

__END_DECLS
#endif /* _PROCESSOR_CORE_H_ */

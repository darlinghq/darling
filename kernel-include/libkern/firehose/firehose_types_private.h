/*
 * Copyright (c) 2015-2016 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __FIREHOSE_TYPES_PRIVATE__
#define __FIREHOSE_TYPES_PRIVATE__

#include <mach/mach_types.h>
#include <mach/std_types.h>
#include <os/base.h>

OS_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

/*!
 * @enum firehose_activity_flags_t
 *
 * @discussion
 * The lower 8 bits are or-ed in the upper 8 bits of Activity ID and propagated
 * to children activities
 */
    OS_OPTIONS(firehose_activity_flags, unsigned long,
    firehose_activity_flags_default             = 0x0000,

    firehose_activity_flags_info_mode           = 0x0001,
    firehose_activity_flags_debug_mode          = 0x0002,
    firehose_activity_flags_stream_live_mode    = 0x0004,

    firehose_activity_flags_precise_timestamp   = 0x0080,
    );

/*!
 * @typedef firehose_activity_id_t
 *
 * @abstract
 * Opaque activity identifier.
 *
 * @discussion
 * Scalar value type, not reference counted.
 */
typedef uint64_t firehose_activity_id_t;
#define FIREHOSE_ACTIVITY_ID_NULL    ((firehose_activity_id_t)0)
#define FIREHOSE_ACTIVITY_ID_INVALID ((firehose_activity_id_t)~0ULL)
#define FIREHOSE_ACTIVITY_ID_FLAGS_SHIFT 56
#define FIREHOSE_ACTIVITY_ID_FLAGS(aid) \
	        ((firehose_activity_flags_t)((aid) >> FIREHOSE_ACTIVITY_ID_FLAGS_SHIFT))
#define FIREHOSE_ACTIVITY_ID_MERGE_FLAGS(aid, flags) (\
	        ((firehose_activity_id_t)(aid)) | \
	        ((firehose_activity_id_t)(flags) << FIREHOSE_ACTIVITY_ID_FLAGS_SHIFT))

/*!
 * @enum firehose_stream_t
 */
OS_ENUM(firehose_stream, uint8_t,
    firehose_stream_persist                     = 0,
    firehose_stream_special                     = 1,
    firehose_stream_memory                      = 2,
    firehose_stream_metadata                    = 3,
    firehose_stream_signpost                    = 4,
    firehose_stream_memory_wifi                 = 5,
    firehose_stream_memory_baseband             = 6,

    _firehose_stream_max,
    );

/*!
 * @enum firehose_tracepoint_namespace_t
 *
 * @abstract
 * Namespaces of tracepoints.
 */
OS_ENUM(firehose_tracepoint_namespace, uint8_t,
    firehose_tracepoint_namespace_activity      = 0x02,
    firehose_tracepoint_namespace_trace         = 0x03,
    firehose_tracepoint_namespace_log           = 0x04,
    firehose_tracepoint_namespace_metadata      = 0x05,
    firehose_tracepoint_namespace_signpost      = 0x06,
    firehose_tracepoint_namespace_loss          = 0x07,
    );

/*!
 * @enum firehose_tracepoint_code_t
 *
 * @abstract
 * Codes of tracepoints.
 */
OS_ENUM(firehose_tracepoint_code, uint32_t,
    firehose_tracepoint_code_load               = 0x01,
    firehose_tracepoint_code_unload             = 0x02,
    );

/*!
 * @typedef firehose_tracepoint_type_t
 *
 * @abstract
 * Type of tracepoints.
 */
typedef uint8_t firehose_tracepoint_type_t;

/*!
 * @typedef firehose_tracepoint_flags_t
 *
 * @abstract
 * Flags for tracepoints.
 */
OS_OPTIONS(firehose_tracepoint_flags, uint16_t,
    _firehose_tracepoint_flags_base_has_current_aid         = 0x0001,
#define _firehose_tracepoint_flags_pc_style_mask                 (0x0007 << 1)
    _firehose_tracepoint_flags_pc_style_none                = 0x0000 << 1,
        _firehose_tracepoint_flags_pc_style_main_exe            = 0x0001 << 1,
        _firehose_tracepoint_flags_pc_style_shared_cache        = 0x0002 << 1,
        _firehose_tracepoint_flags_pc_style_main_plugin         = 0x0003 << 1,
        _firehose_tracepoint_flags_pc_style_absolute            = 0x0004 << 1,
        _firehose_tracepoint_flags_pc_style_uuid_relative       = 0x0005 << 1,
        _firehose_tracepoint_flags_pc_style__unused6            = 0x0006 << 1,
        _firehose_tracepoint_flags_pc_style__unused7            = 0x0007 << 1,
        _firehose_tracepoint_flags_base_has_unique_pid          = 0x0010,
    );

/*
 * Same as _OS_TRACE_DYNAMIC_BIT defined in libtrace/tracepoint_internal.h.
 * This bit is used by logd to know how to evaluate the format
 * string.
 * If it is set, logd assumes that the format is "%s" and the content of the
 * whole string is passed with the firehose_tracepoint.
 * Otherwise it tries to find the unformatted string within the text
 * section of the executable and expects only the content of the variables
 * on the firehose_tracepoint.
 */
#define FIREHOSE_TRACEPOINT_PC_DYNAMIC_BIT 0x80000000

/*
 * Same as KERNEL_MASK defined in logd/logd_main.c
 * It is used by logd to mask the pc before calling
 * OSKextCopyUUIDForAddress.
 */
#define FIREHOSE_TRACEPOINT_PC_KERNEL_MASK 0xffff000000000000

/*!
 * @typedef firehose_tracepoint_id_t
 *
 * @abstract
 * Opaque tracepoint identifier.
 */
typedef uint64_t firehose_tracepoint_id_t;

/*!
 * @enum _firehose_tracepoint_type_activity_t
 *
 * @abstract
 * Types of Activity tracepoints (namespace activity).
 */
OS_ENUM(_firehose_tracepoint_type_activity, firehose_tracepoint_type_t,
    _firehose_tracepoint_type_activity_create               = 0x01,
    _firehose_tracepoint_type_activity_swap                 = 0x02,
    _firehose_tracepoint_type_activity_useraction           = 0x03,
    );

/*!
 * @enum firehose_tracepoint_flags_activity_t
 *
 * @abstract
 * Flags for Activity tracepoints (namespace activity).
 */
OS_OPTIONS(_firehose_tracepoint_flags_activity, uint16_t,
    _firehose_tracepoint_flags_activity_user_interface      = 0x0100,
    _firehose_tracepoint_flags_activity_has_other_aid       = 0x0200,
    );

/*!
 * @enum firehose_tracepoint_type_trace_t
 *
 * @abstract
 * Types of trace tracepoints (namespace trace).
 */
OS_ENUM(_firehose_tracepoint_type_trace, firehose_tracepoint_type_t,
    _firehose_tracepoint_type_trace_default                 = 0x00,
    _firehose_tracepoint_type_trace_info                    = 0x01,
    _firehose_tracepoint_type_trace_debug                   = 0x02,
    _firehose_tracepoint_type_trace_error                   = 0x10,
    _firehose_tracepoint_type_trace_fault                   = 0x11,
    );

/*!
 * @enum firehose_tracepoint_type_log_t
 *
 * @abstract
 * Types of Log tracepoints (namespace log).
 */
OS_ENUM(_firehose_tracepoint_type_log, firehose_tracepoint_type_t,
    _firehose_tracepoint_type_log_default                   = 0x00,
    _firehose_tracepoint_type_log_info                      = 0x01,
    _firehose_tracepoint_type_log_debug                     = 0x02,
    _firehose_tracepoint_type_log_error                     = 0x10,
    _firehose_tracepoint_type_log_fault                     = 0x11,
    );

/*!
 * @enum firehose_tracepoint_flags_log_t
 *
 * @abstract
 * Flags for Log tracepoints (namespace log).
 */
OS_OPTIONS(_firehose_tracepoint_flags_log, uint16_t,
    _firehose_tracepoint_flags_log_has_private_data         = 0x0100,
    _firehose_tracepoint_flags_log_has_subsystem            = 0x0200,
    _firehose_tracepoint_flags_log_has_rules                = 0x0400,
    _firehose_tracepoint_flags_log_has_oversize             = 0x0800,
    _firehose_tracepoint_flags_log_has_context_data         = 0x1000,
    );

/*!
 * @enum _firehose_tracepoint_type_metadata_t
 *
 * @abstract
 * Types for metadata tracepoints (namespace metadata).
 */
OS_ENUM(_firehose_tracepoint_type_metadata, firehose_tracepoint_type_t,
    _firehose_tracepoint_type_metadata_dyld                 = 0x01,
    _firehose_tracepoint_type_metadata_subsystem            = 0x02,
    _firehose_tracepoint_type_metadata_kext                 = 0x03,
    );

/*!
 * @enum firehose_tracepoint_type_signpost_t
 *
 * @abstract
 * Types of Log tracepoints (namespace signpost).
 */
OS_ENUM(_firehose_tracepoint_type_signpost, firehose_tracepoint_type_t,
    _firehose_tracepoint_type_signpost_event                = 0x00,
    _firehose_tracepoint_type_signpost_interval_begin       = 0x01,
    _firehose_tracepoint_type_signpost_interval_end         = 0x02,

    _firehose_tracepoint_type_signpost_scope_mask           = 0xc0,
    _firehose_tracepoint_type_signpost_scope_thread         = 0x40,
    _firehose_tracepoint_type_signpost_scope_process        = 0x80,
    _firehose_tracepoint_type_signpost_scope_system         = 0xc0,
    );

/*!
 * @enum firehose_tracepoint_flags_signpost_t
 *
 * @abstract
 * Flags for Log tracepoints (namespace signpost).
 *
 * When flags are shared with the log type, they should havethe same values.
 */
OS_OPTIONS(_firehose_tracepoint_flags_signpost, uint16_t,
    _firehose_tracepoint_flags_signpost_has_private_data    = 0x0100,
    _firehose_tracepoint_flags_signpost_has_subsystem       = 0x0200,
    _firehose_tracepoint_flags_signpost_has_rules           = 0x0400,
    _firehose_tracepoint_flags_signpost_has_oversize        = 0x0800,
    _firehose_tracepoint_flags_signpost_has_context_data    = 0x1000,
    );

/* MIG firehose push reply structure */
typedef struct firehose_push_reply_s {
	uint64_t fpr_mem_flushed_pos;
	uint64_t fpr_io_flushed_pos;
} firehose_push_reply_t;

typedef struct firehose_buffer_map_info_s {
	mach_vm_address_t fbmi_addr;
	mach_vm_size_t fbmi_size;
} firehose_buffer_map_info_t;

#define FIREHOSE_PUSH_REPLY_CORRUPTED  ((firehose_push_reply_t){ ~0ULL, ~0ULL })

typedef union firehose_buffer_u *firehose_buffer_t;

__END_DECLS

    OS_ASSUME_NONNULL_END

#endif // __FIREHOSE_TYPES_PRIVATE__

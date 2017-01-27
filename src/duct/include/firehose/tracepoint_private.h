/*
 * Copyright (c) 2013-2016 Apple Inc. All rights reserved.
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

#ifndef __FIREHOSE_ACTIVITY__
#define __FIREHOSE_ACTIVITY__

#include <machine/cpu_capabilities.h>
#include <mach/mach_time.h>
#include <os/base.h>
#if KERNEL
#include <atm/atm_internal.h>
#endif
#include "firehose_types_private.h"

OS_ASSUME_NONNULL_BEGIN

/*!
 * @typedef firehose_tracepoint_id_u
 *
 * @abstract
 * Broken down tracepoint identifier.
 */
typedef union {
	struct {
		firehose_tracepoint_namespace_t _namespace;
		firehose_tracepoint_type_t _type;
		firehose_tracepoint_flags_t _flags;
		uint32_t _code;
	} ftid;
	firehose_tracepoint_id_t ftid_value;
} firehose_tracepoint_id_u;

#define FIREHOSE_STAMP_SLOP (1ULL << 36) // ~1minute

/*!
 * @typedef firehose_trace_uuid_info_t
 *
 * @abstract
 * Info needed by logd when kexts are loaded or unloaded
 *
 */
typedef struct firehose_trace_uuid_info_s {
	uuid_t ftui_uuid;      /* uuid of binary */
	uint64_t ftui_address; /* load address */
	uint64_t ftui_size;    /* load size */
	char ftui_path[];      /* full path of binary - Unused in the kernel*/
} *firehose_trace_uuid_info_t;

/*!
 * @typedef firehose_tracepoint_t
 */
typedef struct firehose_tracepoint_s {
	firehose_tracepoint_id_u ft_id;
	uint64_t ft_thread;
	union {
		struct {
			uint64_t ft_timestamp_delta : 48;
			uint64_t ft_length : 16;
		};
		uint64_t ft_stamp_and_length;
	};
	uint8_t ft_data[];
} *firehose_tracepoint_t;

#define FIREHOSE_TRACE_ID_MAKE(ns, type, flags, code) \
	(((firehose_tracepoint_id_u){ .ftid = { \
		._namespace = ns, \
		._type = type, \
		._flags = flags, \
		._code = code, \
	} }).ftid_value)

#define FIREHOSE_TRACE_ID_SET_NS(tid, ns) \
	((tid).ftid._namespace = firehose_tracepoint_namespace_##ns)

#define FIREHOSE_TRACE_ID_SET_TYPE(tid, ns, type) \
	((tid).ftid._type = _firehose_tracepoint_type_##ns##_##type)

#define FIREHOSE_TRACE_ID_HAS_FLAG(tid, ns, flag) \
	((tid).ftid._flags & _firehose_tracepoint_flags_##ns##_##flag)
#define FIREHOSE_TRACE_ID_SET_FLAG(tid, ns, flag) \
	((void)((tid).ftid._flags |= _firehose_tracepoint_flags_##ns##_##flag))
#define FIREHOSE_TRACE_ID_CLEAR_FLAG(tid, ns, flag) \
	((void)((tid).ftid._flags &= ~_firehose_tracepoint_flags_##ns##_##flag))

#define FIREHOSE_TRACE_ID_SET_CODE(tid, code) \
	((tid).ftid._code = code)

__BEGIN_DECLS

#if __has_feature(address_sanitizer)
__attribute__((no_sanitize("address")))
#endif
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0)
__TVOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0)
OS_ALWAYS_INLINE
static inline bool
firehose_precise_timestamps_enabled(void)
{
#if KERNEL
	return (atm_get_diagnostic_config() & 0x80) == 0;
#else
	return (*((volatile uint32_t *)_COMM_PAGE_ATM_DIAGNOSTIC_CONFIG) & 0x80) == 0;
#endif
}

#if __has_feature(address_sanitizer)
__attribute__((no_sanitize("address")))
#endif
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0)
__TVOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0)
OS_ALWAYS_INLINE
static inline uint64_t
firehose_tracepoint_time(firehose_activity_flags_t flags)
{
	if (firehose_precise_timestamps_enabled() ||
			(flags & firehose_activity_flags_precise_timestamp)) {
		return mach_continuous_time();
	} else {
		return mach_continuous_approximate_time();
	}
}

#ifdef KERNEL
__OSX_AVAILABLE(10.12) __IOS_AVAILABLE(10.0)
__TVOS_AVAILABLE(10.0) __WATCHOS_AVAILABLE(3.0)
void
firehose_trace_metadata(firehose_stream_t stream, firehose_tracepoint_id_u ftid,
			uint64_t stamp, const void* pubdata, size_t publen);
#endif
__END_DECLS

OS_ASSUME_NONNULL_END

#endif // __FIREHOSE_FIREHOSE__

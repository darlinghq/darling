// created for Darling

#ifndef _OS_SIGNPOST_H
#define _OS_SIGNPOST_H

#include <stdint.h>
#include <stddef.h>

#include <os/log.h>

typedef uint64_t os_signpost_id_t;

#define OS_SIGNPOST_ID_EXCLUSIVE ((os_signpost_id_t)0xEEEEB0B5B2B2EEEE)
#define OS_SIGNPOST_ID_NULL ((os_signpost_id_t)0)
#define OS_SIGNPOST_ID_INVALID ((os_signpost_id_t)~0)

os_signpost_id_t os_signpost_id_generate(os_log_t log);
os_signpost_id_t os_signpost_id_make_with_pointer(os_log_t log, const void* ptr);

/* DARLING: named type and values as in the macOS SDK, so binaries and Swift overlays agree. */
OS_ENUM(os_signpost_type, uint8_t,
	OS_SIGNPOST_EVENT = 0x00,
	OS_SIGNPOST_INTERVAL_BEGIN = 0x01,
	OS_SIGNPOST_INTERVAL_END = 0x02,
);

#define os_signpost_emit_with_type(log, type, spid, name, ...)

#define os_signpost_interval_begin(log, interval_id, name, ...)
#define os_signpost_event_emit(log, event_id, name, ...)
#define os_signpost_interval_end(log, interval_id, name, ...)

#endif // _OS_SIGNPOST_H

// created for Darling

#ifndef _OS_SIGNPOST_H
#define _OS_SIGNPOST_H

#include <stdint.h>
#include <stddef.h>

#include <os/log.h>

typedef uint64_t os_signpost_id_t;

#define OS_SIGNPOST_ID_EXCLUSIVE
#define OS_SIGNPOST_ID_NULL
#define OS_SIGNPOST_ID_INVALID

os_signpost_id_t os_signpost_id_generate(os_log_t log);
os_signpost_id_t os_signpost_id_make_with_pointer(os_log_t log, const void* ptr);

enum {
	OS_SIGNPOST_INTERVAL_BEGIN,
	OS_SIGNPOST_INTERVAL_END,
	OS_SIGNPOST_EVENT,
};

#define os_signpost_emit_with_type(log, type, spid, name, ...)

#define os_signpost_interval_begin(log, interval_id, name, ...)
#define os_signpost_event_emit(log, event_id, name, ...)
#define os_signpost_interval_end(log, interval_id, name, ...)

#endif // _OS_SIGNPOST_H

#ifndef __LAUNCHD_KTRACE_H__
#define __LAUNCHD_KTRACE_H__

#include <unistd.h>
#include <stdbool.h>

extern bool launchd_apple_internal;

#ifndef DBG_LAUNCHD
#define DBG_LAUNCHD 34
#endif /* DBG_LAUNCHD */

/* Class(8) | SubClass(8) | Code(14) | Qual(2) */
#define RTKT_CODE(c) ((DBG_LAUNCHD << 24) | (((c) & 0x3fffff) << 2))

typedef enum {
	RTKT_LAUNCHD_STARTING				= RTKT_CODE(1),
	RTKT_LAUNCHD_EXITING				= RTKT_CODE(2),
	RTKT_LAUNCHD_FINDING_STRAY_PG		= RTKT_CODE(3),
	RTKT_LAUNCHD_FINDING_ALL_STRAYS		= RTKT_CODE(4),
	RTKT_LAUNCHD_FINDING_EXECLESS		= RTKT_CODE(5),
	RTKT_LAUNCHD_FINDING_WEIRD_UIDS		= RTKT_CODE(6),
	RTKT_LAUNCHD_DATA_PACK				= RTKT_CODE(7),
	RTKT_LAUNCHD_DATA_UNPACK			= RTKT_CODE(8),
	RTKT_LAUNCHD_BUG					= RTKT_CODE(9),
	RTKT_LAUNCHD_MACH_IPC				= RTKT_CODE(10),
	RTKT_LAUNCHD_BSD_KEVENT				= RTKT_CODE(11),
	RTKT_VPROC_TRANSACTION_INCREMENT	= RTKT_CODE(12),
	RTKT_VPROC_TRANSACTION_DECREMENT	= RTKT_CODE(13),
} runtime_ktrace_code_t;

/* All of these log the return address as "arg4" */
void runtime_ktrace1(runtime_ktrace_code_t code);
void runtime_ktrace0(runtime_ktrace_code_t code);
void runtime_ktrace(runtime_ktrace_code_t code, long a, long b, long c);

#endif /* __LAUNCHD_KTRACE_H__ */

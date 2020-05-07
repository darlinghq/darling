/*
 * Copyright (c) 2000-2007 Apple Inc. All rights reserved.
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
#ifndef __AT386_MP_EVENTS__
#define __AT386_MP_EVENTS__

/* Interrupt types */

#ifndef ASSEMBLER

#include <sys/cdefs.h>

typedef enum {
	MP_TLB_FLUSH = 0,
	MP_KDP,
	MP_KDB,
	MP_AST,
	MP_IDLE,
	MP_UNIDLE,
	MP_CALL,
	MP_CALL_PM,
	MP_LAST
} mp_event_t;

#define MP_EVENT_NAME_DECL()    \
const char *mp_event_name[] = { \
	"MP_TLB_FLUSH",         \
	"MP_KDP",               \
	"MP_KDB",               \
	"MP_AST",               \
	"MP_IDLE",              \
	"MP_UNIDLE",            \
	"MP_CALL",              \
	"MP_CALL_PM",           \
	"MP_LAST"               \
}

typedef enum { SYNC, ASYNC, NOSYNC } mp_sync_t;

__BEGIN_DECLS

extern void     i386_signal_cpu(int cpu, mp_event_t event, mp_sync_t mode);
extern void     i386_activate_cpu(void);
extern void     i386_deactivate_cpu(void);
extern void     cpu_NMI_interrupt(int /* cpu */);

__END_DECLS

#endif

#endif

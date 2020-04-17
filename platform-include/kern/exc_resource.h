/*
 * Copyright (c) 2011-2012 Apple Computer, Inc. All rights reserved.
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
/*
 * Mach Operating System
 * Copyright (c) 1989 Carnegie-Mellon University
 * Copyright (c) 1988 Carnegie-Mellon University
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */

/*
 *        EXC_RESOURCE related macros, namespace etc.
 */

#ifndef _EXC_RESOURCE_H_
#define _EXC_RESOURCE_H_

/*
 * Generic exception code format:
 *
 * code:
 * +----------------------------------------------------------+
 * |[63:61] type | [60:58] flavor | [57:0] type-specific data |
 * +----------------------------------------------------------+
 */


/* EXC_RESOURCE type and flavor decoding routines */
#define EXC_RESOURCE_DECODE_RESOURCE_TYPE(code) \
	(((code) >> 61) & 0x7ULL)
#define EXC_RESOURCE_DECODE_FLAVOR(code) \
	(((code) >> 58) & 0x7ULL)

/* EXC_RESOURCE Types */
#define RESOURCE_TYPE_CPU       1
#define RESOURCE_TYPE_WAKEUPS   2
#define RESOURCE_TYPE_MEMORY    3
#define RESOURCE_TYPE_IO        4
#define RESOURCE_TYPE_THREADS   5

/* RESOURCE_TYPE_CPU flavors */
#define FLAVOR_CPU_MONITOR              1
#define FLAVOR_CPU_MONITOR_FATAL        2

/*
 * RESOURCE_TYPE_CPU exception code & subcode.
 *
 * This is sent by the kernel when the CPU usage monitor
 * is tripped. [See proc_set_cpumon_params()]
 *
 * code:
 * +-----------------------------------------------+
 * |[63:61] RESOURCE |[60:58] FLAVOR_CPU_ |[57:32] |
 * |_TYPE_CPU        |MONITOR[_FATAL]     |Unused  |
 * +-----------------------------------------------+
 * |[31:7]  Interval (sec)    | [6:0] CPU limit (%)|
 * +-----------------------------------------------+
 *
 * subcode:
 * +-----------------------------------------------+
 * |                          | [6:0] % of CPU     |
 * |                          | actually consumed  |
 * +-----------------------------------------------+
 *
 */

/* RESOURCE_TYPE_CPU decoding macros */
#define EXC_RESOURCE_CPUMONITOR_DECODE_INTERVAL(code) \
	(((code) >> 7) & 0x1FFFFFFULL)
#define EXC_RESOURCE_CPUMONITOR_DECODE_PERCENTAGE(code) \
	((code) & 0x7FULL)
#define EXC_RESOURCE_CPUMONITOR_DECODE_PERCENTAGE_OBSERVED(subcode) \
	((subcode) & 0x7FULL)


/* RESOURCE_TYPE_WAKEUPS flavors */
#define FLAVOR_WAKEUPS_MONITOR  1

/*
 * RESOURCE_TYPE_WAKEUPS exception code & subcode.
 *
 * This is sent by the kernel when the platform idle
 * wakeups monitor is tripped.
 * [See proc_set_wakeupsmon_params()]
 *
 * code:
 * +-----------------------------------------------+
 * |[63:61] RESOURCE |[60:58] FLAVOR_     |[57:32] |
 * |_TYPE_WAKEUPS    |WAKEUPS_MONITOR     |Unused  |
 * +-----------------------------------------------+
 * | [31:20] Observation     | [19:0] # of wakeups |
 * |         interval (sec)  | permitted (per sec) |
 * +-----------------------------------------------+
 *
 * subcode:
 * +-----------------------------------------------+
 * |                         | [19:0] # of wakeups |
 * |                         | observed (per sec)  |
 * +-----------------------------------------------+
 *
 */

#define EXC_RESOURCE_CPUMONITOR_DECODE_WAKEUPS_PERMITTED(code) \
	((code) & 0xFFFULL)
#define EXC_RESOURCE_CPUMONITOR_DECODE_OBSERVATION_INTERVAL(code) \
	(((code) >> 20) & 0xFFFFFULL)
#define EXC_RESOURCE_CPUMONITOR_DECODE_WAKEUPS_OBSERVED(subcode) \
	((subcode) & 0xFFFFFULL)

/* RESOURCE_TYPE_MEMORY flavors */
#define FLAVOR_HIGH_WATERMARK   1

/*
 * RESOURCE_TYPE_MEMORY / FLAVOR_HIGH_WATERMARK
 * exception code & subcode.
 *
 * This is sent by the kernel when a task crosses its high
 * watermark memory limit.
 *
 * code:
 * +------------------------------------------------+
 * |[63:61] RESOURCE |[60:58] FLAVOR_HIGH_ |[57:32] |
 * |_TYPE_MEMORY     |WATERMARK            |Unused  |
 * +------------------------------------------------+
 * |                         | [12:0] HWM limit (MB)|
 * +------------------------------------------------+
 *
 * subcode:
 * +------------------------------------------------+
 * |                                         unused |
 * +------------------------------------------------+
 *
 */

#define EXC_RESOURCE_HWM_DECODE_LIMIT(code) \
	((code) & 0x1FFFULL)

/* RESOURCE_TYPE_IO flavors */
#define FLAVOR_IO_PHYSICAL_WRITES               1
#define FLAVOR_IO_LOGICAL_WRITES                2

/*
 * RESOURCE_TYPE_IO exception code & subcode.
 *
 * This is sent by the kernel when a task crosses its
 * I/O limits.
 *
 * code:
 * +-----------------------------------------------+
 * |[63:61] RESOURCE |[60:58] FLAVOR_IO_  |[57:32] |
 * |_TYPE_IO         |PHYSICAL/LOGICAL    |Unused  |
 * +-----------------------------------------------+
 * |[31:15]  Interval (sec)    | [14:0] Limit (MB) |
 * +-----------------------------------------------+
 *
 * subcode:
 * +-----------------------------------------------+
 * |                           | [14:0] I/O Count  |
 * |                           | (in MB)           |
 * +-----------------------------------------------+
 *
 */

/* RESOURCE_TYPE_IO decoding macros */
#define EXC_RESOURCE_IO_DECODE_INTERVAL(code) \
	(((code) >> 15) & 0x1FFFFULL)
#define EXC_RESOURCE_IO_DECODE_LIMIT(code) \
	((code) & 0x7FFFULL)
#define EXC_RESOURCE_IO_OBSERVED(subcode) \
	((subcode) & 0x7FFFULL)


/*
 * RESOURCE_TYPE_THREADS exception code & subcode
 *
 * This is sent by the kernel when a task crosses its
 * thread limit.
 */

#define EXC_RESOURCE_THREADS_DECODE_THREADS(code) \
	((code) & 0x7FFFULL)

/* RESOURCE_TYPE_THREADS flavors */
#define FLAVOR_THREADS_HIGH_WATERMARK 1



#endif /* _EXC_RESOURCE_H_ */

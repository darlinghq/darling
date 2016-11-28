/*
 * Copyright (c) 1998-2010 Apple Inc. All rights reserved.
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
#ifndef IOKIT_IOTIMESTAMP_H
#define IOKIT_IOTIMESTAMP_H

#include <sys/kdebug.h>

static inline void
IOTimeStampStartConstant(unsigned int csc,
			 uintptr_t a = 0, uintptr_t b = 0,
			 uintptr_t c = 0, uintptr_t d = 0)
{
    KERNEL_DEBUG_CONSTANT(((uint32_t)csc) | DBG_FUNC_START, a, b, c, d, 0);
}

static inline void
IOTimeStampEndConstant(uintptr_t csc,
		       uintptr_t a = 0, uintptr_t b = 0,
		       uintptr_t c = 0, uintptr_t d = 0)
{
    KERNEL_DEBUG_CONSTANT(((uint32_t)csc) | DBG_FUNC_END, a, b, c, d, 0);
}

static inline void
IOTimeStampConstant(uintptr_t csc,
		    uintptr_t a = 0, uintptr_t b = 0,
		    uintptr_t c = 0, uintptr_t d = 0)
{
    KERNEL_DEBUG_CONSTANT(((uint32_t)csc) | DBG_FUNC_NONE, a, b, c, d, 0);
}

#if KDEBUG

static inline void
IOTimeStampStart(uintptr_t csc,
                 uintptr_t a = 0, uintptr_t b = 0,
                 uintptr_t c = 0, uintptr_t d = 0)
{
    KERNEL_DEBUG(((uint32_t)csc) | DBG_FUNC_START, a, b, c, d, 0);
}

static inline void
IOTimeStampEnd(uintptr_t csc,
               uintptr_t a = 0, uintptr_t b = 0,
               uintptr_t c = 0, uintptr_t d = 0)
{
    KERNEL_DEBUG(((uint32_t)csc) | DBG_FUNC_END, a, b, c, d, 0);
}

static inline void
IOTimeStamp(uintptr_t csc,
            uintptr_t a = 0, uintptr_t b = 0,
            uintptr_t c = 0, uintptr_t d = 0)
{
    KERNEL_DEBUG(((uint32_t)csc) | DBG_FUNC_NONE, a, b, c, d, 0);
}

#endif /* KDEBUG */

#define IODBG_STORAGE(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOSTORAGE, code))
#define IODBG_NETWORK(code)			(KDBG_CODE(DBG_IOKIT, DBG_IONETWORK, code))
#define IODBG_KEYBOARD(code)		(KDBG_CODE(DBG_IOKIT, DBG_IOKEYBOARD, code))
#define IODBG_HID(code)				(KDBG_CODE(DBG_IOKIT, DBG_IOHID, code))
#define IODBG_AUDIO(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOAUDIO, code))
#define IODBG_SERIAL(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOSERIAL, code))
#define IODBG_TTY(code)				(KDBG_CODE(DBG_IOKIT, DBG_IOTTY, code))
#define IODBG_SAM(code)				(KDBG_CODE(DBG_IOKIT, DBG_IOSAM, code))
#define IODBG_PARALLELATA(code)		(KDBG_CODE(DBG_IOKIT, DBG_IOPARALLELATA, code))
#define IODBG_PARALLELSCSI(code)	(KDBG_CODE(DBG_IOKIT, DBG_IOPARALLELSCSI, code))
#define IODBG_SATA(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOSATA, code))
#define IODBG_SAS(code)				(KDBG_CODE(DBG_IOKIT, DBG_IOSAS, code))
#define IODBG_FIBRECHANNEL(code)	(KDBG_CODE(DBG_IOKIT, DBG_IOFIBRECHANNEL, code))
#define IODBG_USB(code)				(KDBG_CODE(DBG_IOKIT, DBG_IOUSB, code))
#define IODBG_BLUETOOTH(code)		(KDBG_CODE(DBG_IOKIT, DBG_IOBLUETOOTH, code))
#define IODBG_FIREWIRE(code)		(KDBG_CODE(DBG_IOKIT, DBG_IOFIREWIRE, code))
#define IODBG_INFINIBAND(code)		(KDBG_CODE(DBG_IOKIT, DBG_IOINFINIBAND, code))


/* Backwards compatibility */
#define IODBG_DISK(code)			IODBG_STORAGE(code)
#define IODBG_POINTING(code)		IODBG_HID(code)


/* IOKit infrastructure subclasses */
#define IODBG_INTC(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOINTC, code))
#define IODBG_WORKLOOP(code)		(KDBG_CODE(DBG_IOKIT, DBG_IOWORKLOOP, code))
#define IODBG_INTES(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOINTES, code))
#define IODBG_TIMES(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOCLKES, code))
#define IODBG_CMDQ(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOCMDQ, code))
#define IODBG_MCURS(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOMCURS, code))
#define IODBG_MDESC(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOMDESC, code))
#define IODBG_POWER(code)			(KDBG_CODE(DBG_IOKIT, DBG_IOPOWER, code))
#define IODBG_IOSERVICE(code)		(KDBG_CODE(DBG_IOKIT, DBG_IOSERVICE, code))

/* IOKit specific codes - within each subclass */

/* DBG_IOKIT/DBG_IODISK codes */

/* DBG_IOKIT/DBG_IONETWORK codes */

/* DBG_IOKIT/DBG_IOKEYBOARD codes */

/* DBG_IOKIT/DBG_IOHID codes */

/* DBG_IOKIT/DBG_IOAUDIO codes */

/* DBG_IOKIT/DBG_IOSERIAL codes */

/* DBG_IOKIT/DBG_IOTTY codes */

/* DBG_IOKIT/DBG_IOINTC codes */
#define IOINTC_HANDLER	1	/* 0x05000004 */

/* DBG_IOKIT/DBG_IOWORKLOOP codes */
#define IOWL_CLIENT		1	/* 0x05010004 */
#define IOWL_WORK		2	/* 0x05010008 */

/* DBG_IOKIT/DBG_IOINTES codes */
#define IOINTES_CLIENT	1	/* 0x05020004 */
#define IOINTES_LAT		2	/* 0x05020008 */
#define IOINTES_SEMA	3	/* 0x0502000c */
#define IOINTES_INTCTXT 4	/* 0x05020010 */
#define IOINTES_INTFLTR 5	/* 0x05020014 */
#define IOINTES_ACTION	6	/* 0x05020018 */
#define IOINTES_FILTER	7	/* 0x0502001c */

/* DBG_IOKIT/DBG_IOTIMES codes */
#define IOTIMES_CLIENT	1	/* 0x05030004 */
#define IOTIMES_LAT		2	/* 0x05030008 */
#define IOTIMES_SEMA	3	/* 0x0503000c */
#define IOTIMES_ACTION	4	/* 0x05030010 */

/* DBG_IOKIT/DBG_IOCMDQ codes */
#define IOCMDQ_CLIENT	1	/* 0x05040004 */
#define IOCMDQ_LAT		2	/* 0x05040008 */
#define IOCMDQ_SEMA		3	/* 0x0504000c */
#define IOCMDQ_PSEMA	4	/* 0x05040010 */
#define IOCMDQ_PLOCK	5	/* 0x05040014 */
#define IOCMDQ_ACTION	6	/* 0x05040018 */

/* DBG_IOKIT/DBG_IOMCURS codes */

/* DBG_IOKIT/DBG_IOMDESC codes */

/* DBG_IOKIT/DBG_IOPOWER codes */
// See IOKit/pwr_mgt/IOPMlog.h for the power management codes

/* DBG_IOKIT/DBG_IOSERVICE codes */
#define IOSERVICE_BUSY		1	/* 0x05080004 */
#define IOSERVICE_NONBUSY	2	/* 0x05080008 */
#define IOSERVICE_MODULESTALL	3	/* 0x0508000C */
#define IOSERVICE_MODULEUNSTALL	4	/* 0x05080010 */

#define IOSERVICE_TERMINATE_PHASE1		5	/* 0x05080014 */
#define IOSERVICE_TERMINATE_REQUEST_OK		6	/* 0x05080018 */
#define IOSERVICE_TERMINATE_REQUEST_FAIL	7	/* 0x0508001C */
#define IOSERVICE_TERMINATE_SCHEDULE_STOP	8	/* 0x05080020 */
#define IOSERVICE_TERMINATE_SCHEDULE_FINALIZE	9	/* 0x05080024 */
#define IOSERVICE_TERMINATE_WILL		10	/* 0x05080028 */
#define IOSERVICE_TERMINATE_DID			11	/* 0x0508002C */
#define IOSERVICE_TERMINATE_DID_DEFER		12	/* 0x05080030 */
#define IOSERVICE_TERMINATE_FINALIZE		13	/* 0x05080034 */
#define IOSERVICE_TERMINATE_STOP		14	/* 0x05080038 */
#define IOSERVICE_TERMINATE_STOP_NOP		15	/* 0x0508003C */
#define IOSERVICE_TERMINATE_STOP_DEFER		16	/* 0x05080040 */
#define IOSERVICE_TERMINATE_DONE		17	/* 0x05080044 */

#define IOSERVICE_KEXTD_ALIVE			18	/* 0x05080048 */
#define IOSERVICE_KEXTD_READY			19	/* 0x0508004C */
#define IOSERVICE_REGISTRY_QUIET		20	/* 0x05080050 */

#define IOSERVICE_TERM_SET_INACTIVE		21	/* 0x05080054 */
#define IOSERVICE_TERM_SCHED_PHASE2		22	/* 0x05080058 */
#define IOSERVICE_TERM_START_PHASE2		23	/* 0x0508005C */
#define IOSERVICE_TERM_TRY_PHASE2		24	/* 0x05080060 */
#define IOSERVICE_TERM_UC_DEFER			25	/* 0x05080064 */
#define IOSERVICE_DETACH			26	/* 0x05080068 */


#endif /* ! IOKIT_IOTIMESTAMP_H */

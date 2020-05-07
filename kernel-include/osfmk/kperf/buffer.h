/*
 * Copyright (c) 2011-2016 Apple Computer, Inc. All rights reserved.
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

/* wrapper around kdebug */

#include <sys/kdebug.h>

/* kdebug codes */
#define PERF_CODE(SubClass, code) KDBG_CODE(DBG_PERF, SubClass, code)

/* broad sub-classes */
#define PERF_GENERIC    (0)
#define PERF_THREADINFO (1)
#define PERF_CALLSTACK  (2)
#define PERF_TIMER      (3)
#define PERF_PET        (4)
#define PERF_AST        (5)
#define PERF_KPC        (6)
#define PERF_KDBG       (7)
#define PERF_TASK       (8)
#define PERF_LAZY       (9)
#define PERF_MEMINFO    (10)

/* helpers for 32-bit */
#define UPPER_32(U64)        ((U64) >> 32)
#define LOWER_32(U64)        ((U64) & (UINT32_MAX))
#define ENCODE_UPPER_64(U32) (((uint64_t)(U32)) << 32)
#define ENCODE_LOWER_64(U32) (((uint64_t)(U32)) & (UINT32_MAX))

/* sub-class codes */
#define PERF_GEN_CODE(code) PERF_CODE(PERF_GENERIC, code)
#define PERF_GEN_EVENT      PERF_GEN_CODE(0)

#define PERF_TI_CODE(code)      PERF_CODE(PERF_THREADINFO, code)
#define PERF_TI_SAMPLE          PERF_TI_CODE(0)
#define PERF_TI_DATA            PERF_TI_CODE(1)
#define PERF_TI_XSAMPLE         PERF_TI_CODE(2)
#define PERF_TI_XPEND           PERF_TI_CODE(3)
#define PERF_TI_XDATA           PERF_TI_CODE(4)
#define PERF_TI_CSWITCH         PERF_TI_CODE(5)
#define PERF_TI_SCHEDSAMPLE     PERF_TI_CODE(6)
#define PERF_TI_SCHEDDATA       PERF_TI_CODE(7)
#define PERF_TI_SNAPSAMPLE      PERF_TI_CODE(8)
#define PERF_TI_SNAPDATA        PERF_TI_CODE(9)
#define PERF_TI_DISPSAMPLE      PERF_TI_CODE(10)
#define PERF_TI_DISPDATA        PERF_TI_CODE(11)
#define PERF_TI_DISPPEND        PERF_TI_CODE(12)
#define PERF_TI_SNAPDATA_32     PERF_TI_CODE(13)
#define PERF_TI_DISPDATA_32     PERF_TI_CODE(14)
#define PERF_TI_SCHEDDATA1_32   PERF_TI_CODE(15)
#define PERF_TI_SCHEDDATA2_32   PERF_TI_CODE(16)
#define PERF_TI_INSCYCDATA      PERF_TI_CODE(17)
#define PERF_TI_INSCYCDATA_32   PERF_TI_CODE(18)
#define PERF_TI_SCHEDDATA_2     PERF_TI_CODE(19)
#define PERF_TI_SCHEDDATA2_32_2 PERF_TI_CODE(20)
#define PERF_TI_SCHEDDATA3_32   PERF_TI_CODE(21)
#define PERF_TI_SCHEDDATA_3     PERF_TI_CODE(22)

#define PERF_CS_CODE(code) PERF_CODE(PERF_CALLSTACK, code)
#define PERF_CS_KSAMPLE    PERF_CS_CODE(0)
#define PERF_CS_UPEND      PERF_CS_CODE(1)
#define PERF_CS_USAMPLE    PERF_CS_CODE(2)
#define PERF_CS_KDATA      PERF_CS_CODE(3)
#define PERF_CS_UDATA      PERF_CS_CODE(4)
#define PERF_CS_KHDR       PERF_CS_CODE(5)
#define PERF_CS_UHDR       PERF_CS_CODE(6)
#define PERF_CS_ERROR      PERF_CS_CODE(7)
#define PERF_CS_BACKTRACE  PERF_CS_CODE(8)
#define PERF_CS_LOG        PERF_CS_CODE(9)

#define PERF_TM_CODE(code) PERF_CODE(PERF_TIMER, code)
#define PERF_TM_FIRE       PERF_TM_CODE(0)
#define PERF_TM_SCHED      PERF_TM_CODE(1)
#define PERF_TM_HNDLR      PERF_TM_CODE(2)
#define PERF_TM_PENDING    PERF_TM_CODE(3)
#define PERF_TM_SKIPPED    PERF_TM_CODE(4)

#define PERF_PET_CODE(code)    PERF_CODE(PERF_PET, code)
#define PERF_PET_THREAD        PERF_PET_CODE(0)
#define PERF_PET_ERROR         PERF_PET_CODE(1)
#define PERF_PET_RUN           PERF_PET_CODE(2)
#define PERF_PET_PAUSE         PERF_PET_CODE(3)
#define PERF_PET_IDLE          PERF_PET_CODE(4)
#define PERF_PET_SAMPLE        PERF_PET_CODE(5)
#define PERF_PET_SCHED         PERF_PET_CODE(6)
#define PERF_PET_END           PERF_PET_CODE(7)
#define PERF_PET_SAMPLE_TASK   PERF_PET_CODE(8)
#define PERF_PET_SAMPLE_THREAD PERF_PET_CODE(9)

#define PERF_AST_CODE(code) PERF_CODE(PERF_AST, code)
#define PERF_AST_HNDLR      PERF_AST_CODE(0)
#define PERF_AST_ERROR      PERF_AST_CODE(1)

#define PERF_KPC_CODE(code)    PERF_CODE(PERF_KPC, code)
#define PERF_KPC_HNDLR         PERF_KPC_CODE(0)
#define PERF_KPC_FCOUNTER      PERF_KPC_CODE(1)
#define PERF_KPC_COUNTER       PERF_KPC_CODE(2)
#define PERF_KPC_DATA          PERF_KPC_CODE(3)
#define PERF_KPC_CONFIG        PERF_KPC_CODE(4)
#define PERF_KPC_CFG_REG       PERF_KPC_CODE(5)
#define PERF_KPC_DATA32        PERF_KPC_CODE(6)
#define PERF_KPC_CFG_REG32     PERF_KPC_CODE(7)
#define PERF_KPC_DATA_THREAD   PERF_KPC_CODE(8)
#define PERF_KPC_DATA_THREAD32 PERF_KPC_CODE(9)
#define PERF_KPC_CPU_SAMPLE    PERF_KPC_CODE(10)
#define PERF_KPC_THREAD_SAMPLE PERF_KPC_CODE(11)

#define PERF_KDBG_CODE(code) PERF_CODE(PERF_KDBG, code)
#define PERF_KDBG_HNDLR      PERF_KDBG_CODE(0)

#define PERF_TK_CODE(code)    PERF_CODE(PERF_TASK, code)
#define PERF_TK_SNAP_SAMPLE   PERF_TK_CODE(0)
#define PERF_TK_SNAP_DATA     PERF_TK_CODE(1)
#define PERF_TK_SNAP_DATA1_32 PERF_TK_CODE(2)
#define PERF_TK_SNAP_DATA2_32 PERF_TK_CODE(3)
#define PERF_TK_INFO_DATA     PERF_TK_CODE(4)

#define PERF_LZ_CODE(code) PERF_CODE(PERF_LAZY, code)
#define PERF_LZ_MKRUNNABLE PERF_LZ_CODE(0)
#define PERF_LZ_WAITSAMPLE PERF_LZ_CODE(1)
#define PERF_LZ_CPUSAMPLE  PERF_LZ_CODE(2)

#define PERF_MI_CODE(code) PERF_CODE(PERF_MEMINFO, code)
#define PERF_MI_SAMPLE     PERF_MI_CODE(0)
#define PERF_MI_DATA       PERF_MI_CODE(1)
#define PERF_MI_SYS_DATA   PERF_MI_CODE(2)
#define PERF_MI_SYS_DATA_2 PERF_MI_CODE(3)

/* error sub-codes for trace data */
enum{
	ERR_TASK,
	ERR_THREAD,
	ERR_PID,
	ERR_FRAMES,
	ERR_GETSTACK,
	ERR_NOMEM,
};

/* level of kperf's logging to kdebug */
#define KPERF_DEBUG_DATA    0
#define KPERF_DEBUG_INFO    1
#define KPERF_DEBUG_VERBOSE 2
extern int kperf_debug_level;

/* BUF_DATA tracepoints are for logging actual kperf results. */

#define BUF_DATA_INT(EVENTID, A0, A1, A2, A3) KERNEL_DEBUG_CONSTANT_IST(~KDEBUG_ENABLE_PPT, EVENTID, A0, A1, A2, A3, 0)

#define BUF_DATA(EVENTID, ...)                          BUF_DATA_(EVENTID, ## __VA_ARGS__, 4, 3, 2, 1, 0)
#define BUF_DATA_(EVENTID, A1, A2, A3, A4, N_ARGS, ...) BUF_DATA##N_ARGS(EVENTID, A1, A2, A3, A4)
#define BUF_DATA0(EVENTID, A1, A2, A3, A4)              BUF_DATA_INT(EVENTID, 0, 0, 0, 0)
#define BUF_DATA1(EVENTID, A1, A2, A3, A4)              BUF_DATA_INT(EVENTID, A1, 0, 0, 0)
#define BUF_DATA2(EVENTID, A1, A2, A3, A4)              BUF_DATA_INT(EVENTID, A1, A2, 0, 0)
#define BUF_DATA3(EVENTID, A1, A2, A3, A4)              BUF_DATA_INT(EVENTID, A1, A2, A3, 0)
#define BUF_DATA4(EVENTID, A1, A2, A3, A4)              BUF_DATA_INT(EVENTID, A1, A2, A3, A4)

/*
 * BUF_INFO tracepoints are for logging debugging information relevant to
 * testing kperf's internal functions.
 */

#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD)
#define BUF_INFO_INT(EVENTID, A1, A2, A3, A4) if (__improbable(kperf_debug_level >= KPERF_DEBUG_INFO)) KERNEL_DEBUG_CONSTANT(EVENTID, A1, A2, A3, A4, 0)
#else
#define BUF_INFO_INT(EVENTID, A1, A2, A3, A4) do { (void)(EVENTID); (void)(A1); (void)(A2); (void)(A3); (void)(A4); } while ((0))
#endif

#define BUF_INFO(EVENTID, ...)                          BUF_INFO_(EVENTID, ## __VA_ARGS__, 4, 3, 2, 1, 0)
#define BUF_INFO_(EVENTID, A1, A2, A3, A4, N_ARGS, ...) BUF_INFO##N_ARGS(EVENTID, A1, A2, A3, A4)
#define BUF_INFO0(EVENTID, A1, A2, A3, A4)              BUF_INFO_INT(EVENTID, 0, 0, 0, 0)
#define BUF_INFO1(EVENTID, A1, A2, A3, A4)              BUF_INFO_INT(EVENTID, A1, 0, 0, 0)
#define BUF_INFO2(EVENTID, A1, A2, A3, A4)              BUF_INFO_INT(EVENTID, A1, A2, 0, 0)
#define BUF_INFO3(EVENTID, A1, A2, A3, A4)              BUF_INFO_INT(EVENTID, A1, A2, A3, 0)
#define BUF_INFO4(EVENTID, A1, A2, A3, A4)              BUF_INFO_INT(EVENTID, A1, A2, A3, A4)

/*
 * BUF_VERB tracepoints are for logging precise details of kperf's
 * internal functions, like timing information for samplers.
 */

#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD)
#define BUF_VERB_INT(EVENTID, A1, A2, A3, A4) if (__improbable(kperf_debug_level >= KPERF_DEBUG_VERBOSE)) KERNEL_DEBUG_CONSTANT(EVENTID, A1, A2, A3, A4, 0)
#else
#define BUF_VERB_INT(EVENTID, A1, A2, A3, A4) do { (void)(EVENTID); (void)(A1); (void)(A2); (void)(A3); (void)(A4); } while ((0))
#endif

#define BUF_VERB(EVENTID, ...)                          BUF_VERB_(EVENTID, ## __VA_ARGS__, 4, 3, 2, 1, 0)
#define BUF_VERB_(EVENTID, A1, A2, A3, A4, N_ARGS, ...) BUF_VERB##N_ARGS(EVENTID, A1, A2, A3, A4)
#define BUF_VERB0(EVENTID, A1, A2, A3, A4)              BUF_VERB_INT(EVENTID, 0, 0, 0, 0)
#define BUF_VERB1(EVENTID, A1, A2, A3, A4)              BUF_VERB_INT(EVENTID, A1, 0, 0, 0)
#define BUF_VERB2(EVENTID, A1, A2, A3, A4)              BUF_VERB_INT(EVENTID, A1, A2, 0, 0)
#define BUF_VERB3(EVENTID, A1, A2, A3, A4)              BUF_VERB_INT(EVENTID, A1, A2, A3, 0)
#define BUF_VERB4(EVENTID, A1, A2, A3, A4)              BUF_VERB_INT(EVENTID, A1, A2, A3, A4)

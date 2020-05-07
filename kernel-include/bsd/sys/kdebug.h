/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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

#ifndef BSD_SYS_KDEBUG_H
#define BSD_SYS_KDEBUG_H

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

#ifdef __APPLE_API_UNSTABLE

/*
 * Kdebug is a kernel facility for tracing events occurring on a system.  User
 * space processes should prefer os_signpost, instead.
 *
 * This header defines reserved debugids, which are 32-bit values that describe
 * each event:
 *
 * +----------------+----------------+----------------------------+----+
 * |   Class (8)    |  Subclass (8)  |          Code (14)         |Func|
 * |                |                |                            |(2) |
 * +----------------+----------------+----------------------------+----+
 * \_________________________________/
 *         ClassSubclass (CSC)
 * \________________________________________________________________00_/
 *                                 Eventid
 * \___________________________________________________________________/
 *                                 Debugid
 *
 * The eventid is a hierarchical ID, indicating which components an event is
 * referring to.  The debugid includes an eventid and two function qualifier
 * bits, to determine the structural significance of an event (whether it
 * starts or ends an interval).
 */

#define KDBG_CLASS_MASK   (0xff000000)
#define KDBG_CLASS_OFFSET (24)
#define KDBG_CLASS_MAX    (0xff)

#define KDBG_SUBCLASS_MASK   (0x00ff0000)
#define KDBG_SUBCLASS_OFFSET (16)
#define KDBG_SUBCLASS_MAX    (0xff)

/* class and subclass mask */
#define KDBG_CSC_MASK   (0xffff0000)
#define KDBG_CSC_OFFSET (KDBG_SUBCLASS_OFFSET)
#define KDBG_CSC_MAX    (0xffff)

#define KDBG_CODE_MASK   (0x0000fffc)
#define KDBG_CODE_OFFSET (2)
#define KDBG_CODE_MAX    (0x3fff)

#define KDBG_EVENTID_MASK (0xfffffffc)
#define KDBG_FUNC_MASK    (0x00000003)

/* Generate an eventid corresponding to Class, SubClass, and Code. */
#define KDBG_EVENTID(Class, SubClass, Code)                \
	(((unsigned)((Class)    &   0xff) << KDBG_CLASS_OFFSET)    | \
	 ((unsigned)((SubClass) &   0xff) << KDBG_SUBCLASS_OFFSET) | \
	 ((unsigned)((Code)     & 0x3fff) << KDBG_CODE_OFFSET))
/* Deprecated macro using old naming convention. */
#define KDBG_CODE(Class, SubClass, Code) \
	KDBG_EVENTID(Class, SubClass, Code)

/* Extract pieces of the debug code. */
#define KDBG_EXTRACT_CLASS(Debugid) \
	((uint8_t)(((Debugid) & KDBG_CLASS_MASK) >> KDBG_CLASS_OFFSET))
#define KDBG_EXTRACT_SUBCLASS(Debugid) \
	((uint8_t)(((Debugid) & KDBG_SUBCLASS_MASK) >> KDBG_SUBCLASS_OFFSET))
#define KDBG_EXTRACT_CSC(Debugid) \
	((uint16_t)(((Debugid) & KDBG_CSC_MASK) >> KDBG_CSC_OFFSET))
#define KDBG_EXTRACT_CODE(Debugid) \
	((uint16_t)(((Debugid) & KDBG_CODE_MASK) >> KDBG_CODE_OFFSET))
#define KDBG_CLASS_ENCODE(Class, SubClass) KDBG_EVENTID(Class, SubClass, 0)
#define KDBG_CLASS_DECODE(Debugid) (Debugid & KDBG_CSC_MASK)

/* function qualifiers  */
#define DBG_FUNC_START 1U
#define DBG_FUNC_END   2U
#define DBG_FUNC_NONE  0U

/* The Kernel Debug Classes  */

#define DBG_MACH        1
#define DBG_NETWORK     2
#define DBG_FSYSTEM     3
#define DBG_BSD         4
#define DBG_IOKIT       5
#define DBG_DRIVERS     6
#define DBG_TRACE       7
#define DBG_DLIL        8
#define DBG_PTHREAD     9
#define DBG_CORESTORAGE 10
#define DBG_CG          11
#define DBG_MONOTONIC   12
#define DBG_MISC        20
#define DBG_SECURITY    30
#define DBG_DYLD        31
#define DBG_QT          32
#define DBG_APPS        33
#define DBG_LAUNCHD     34
#define DBG_SILICON     35
#define DBG_PERF        37
#define DBG_IMPORTANCE  38
#define DBG_BANK        40
#define DBG_XPC         41
#define DBG_ATM         42
#define DBG_ARIADNE     43
#define DBG_DAEMON      44
#define DBG_ENERGYTRACE 45
#define DBG_DISPATCH    46
#define DBG_IMG         49
#define DBG_UMALLOC     51
#define DBG_TURNSTILE   53

#define DBG_MIG         255

/* **** The Kernel Debug Sub Classes for Mach (DBG_MACH) **** */
#define DBG_MACH_EXCP_KTRAP_x86 0x02 /* Kernel Traps on x86 */
#define DBG_MACH_EXCP_DFLT      0x03 /* Data Translation Fault */
#define DBG_MACH_EXCP_IFLT      0x04 /* Inst Translation Fault */
#define DBG_MACH_EXCP_INTR      0x05 /* Interrupts */
#define DBG_MACH_EXCP_ALNG      0x06 /* Alignment Exception */
#define DBG_MACH_EXCP_UTRAP_x86 0x07 /* User Traps on x86 */
#define DBG_MACH_EXCP_FP        0x08 /* FP Unavail */
#define DBG_MACH_EXCP_DECI      0x09 /* Decrementer Interrupt */
#define DBG_MACH_CHUD           0x0A /* deprecated name */
#define DBG_MACH_SIGNPOST       0x0A /* kernel signposts */
#define DBG_MACH_EXCP_SC        0x0C /* System Calls */
#define DBG_MACH_EXCP_TRACE     0x0D /* Trace exception */
#define DBG_MACH_EXCP_EMUL      0x0E /* Instruction emulated */
#define DBG_MACH_IHDLR          0x10 /* Interrupt Handlers */
#define DBG_MACH_IPC            0x20 /* Inter Process Comm */
#define DBG_MACH_RESOURCE       0x25 /* tracing limits, etc */
#define DBG_MACH_VM             0x30 /* Virtual Memory */
#define DBG_MACH_LEAKS          0x31 /* alloc/free */
#define DBG_MACH_WORKINGSET     0x32 /* private subclass for working set related debugging */
#define DBG_MACH_SCHED          0x40 /* Scheduler */
#define DBG_MACH_MSGID_INVALID  0x50 /* Messages - invalid */
#define DBG_MACH_LOCKS          0x60 /* new lock APIs */
#define DBG_MACH_PMAP           0x70 /* pmap */
#define DBG_MACH_CLOCK          0x80 /* clock */
#define DBG_MACH_MP             0x90 /* MP related */
#define DBG_MACH_VM_PRESSURE    0xA0 /* Memory Pressure Events */
#define DBG_MACH_STACKSHOT      0xA1 /* Stackshot/Microstackshot subsystem */
#define DBG_MACH_SFI            0xA2 /* Selective Forced Idle (SFI) */
#define DBG_MACH_ENERGY_PERF    0xA3 /* Energy/performance resource stats */
#define DBG_MACH_SYSDIAGNOSE    0xA4 /* sysdiagnose */
#define DBG_MACH_ZALLOC         0xA5 /* Zone allocator */
#define DBG_MACH_THREAD_GROUP   0xA6 /* Thread groups */
#define DBG_MACH_COALITION      0xA7 /* Coalitions */
#define DBG_MACH_SHAREDREGION   0xA8 /* Shared region */
#define DBG_MACH_SCHED_CLUTCH   0xA9 /* Clutch scheduler */
#define DBG_MACH_IO             0xAA /* I/O */

/* Codes for DBG_MACH_IO */
#define DBC_MACH_IO_MMIO_READ           0x1
#define DBC_MACH_IO_MMIO_WRITE          0x2
#define DBC_MACH_IO_PHYS_READ           0x3
#define DBC_MACH_IO_PHYS_WRITE          0x4
#define DBC_MACH_IO_PORTIO_READ         0x5
#define DBC_MACH_IO_PORTIO_WRITE        0x6

/* Interrupt type bits for DBG_MACH_EXCP_INTR */
#define DBG_INTR_TYPE_UNKNOWN   0x0     /* default/unknown interrupt */
#define DBG_INTR_TYPE_IPI       0x1     /* interprocessor interrupt */
#define DBG_INTR_TYPE_TIMER     0x2     /* timer interrupt */
#define DBG_INTR_TYPE_OTHER     0x3     /* other (usually external) interrupt */
#define DBG_INTR_TYPE_PMI       0x4     /* performance monitor interrupt */

/* Codes for Scheduler (DBG_MACH_SCHED) */
#define MACH_SCHED              0x0     /* Scheduler */
#define MACH_STACK_ATTACH       0x1     /* stack_attach() */
#define MACH_STACK_HANDOFF      0x2     /* stack_handoff() */
#define MACH_CALL_CONT          0x3     /* call_continuation() */
#define MACH_CALLOUT            0x4     /* callouts */
#define MACH_STACK_DETACH       0x5
#define MACH_MAKE_RUNNABLE      0x6     /* make thread runnable */
#define MACH_PROMOTE            0x7     /* promoted due to resource (replaced by MACH_PROMOTED) */
#define MACH_DEMOTE             0x8     /* promotion undone (replaced by MACH_UNPROMOTED) */
#define MACH_IDLE               0x9     /* processor idling */
#define MACH_STACK_DEPTH        0xa     /* stack depth at switch */
#define MACH_MOVED              0xb     /* did not use original scheduling decision */
#define MACH_PSET_LOAD_AVERAGE  0xc
#define MACH_AMP_DEBUG          0xd
#define MACH_FAILSAFE           0xe     /* tripped fixed-pri/RT failsafe */
#define MACH_BLOCK              0xf     /* thread block */
#define MACH_WAIT               0x10    /* thread wait assertion */
#define MACH_GET_URGENCY        0x14    /* Urgency queried by platform */
#define MACH_URGENCY            0x15    /* Urgency (RT/BG/NORMAL) communicated
	                                 * to platform
	                                 */
#define MACH_REDISPATCH         0x16    /* "next thread" thread redispatched */
#define MACH_REMOTE_AST         0x17    /* AST signal issued to remote processor */
#define MACH_SCHED_CHOOSE_PROCESSOR     0x18    /* Result of choose_processor */
#define MACH_DEEP_IDLE          0x19    /* deep idle on master processor */
/* unused                       0x1a    was MACH_SCHED_DECAY_PRIORITY */
#define MACH_CPU_THROTTLE_DISABLE       0x1b    /* Global CPU Throttle Disable */
#define MACH_RW_PROMOTE            0x1c /* promoted due to RW lock promotion */
#define MACH_RW_DEMOTE             0x1d /* promotion due to RW lock undone */
#define MACH_SCHED_MAINTENANCE     0x1f /* periodic maintenance thread */
#define MACH_DISPATCH              0x20 /* context switch completed */
#define MACH_QUANTUM_HANDOFF       0x21 /* quantum handoff occurred */
#define MACH_MULTIQ_DEQUEUE        0x22 /* Result of multiq dequeue */
#define MACH_SCHED_THREAD_SWITCH   0x23 /* attempt direct context switch to hinted thread */
#define MACH_SCHED_SMT_BALANCE     0x24 /* SMT load balancing ASTs */
#define MACH_REMOTE_DEFERRED_AST   0x25 /* Deferred AST started against remote processor */
#define MACH_REMOTE_CANCEL_AST     0x26 /* Canceled deferred AST for remote processor */
#define MACH_SCHED_CHANGE_PRIORITY 0x27 /* thread sched priority changed */
#define MACH_SCHED_UPDATE_REC_CORES     0x28    /* Change to recommended processor bitmask */
#define MACH_STACK_WAIT            0x29 /* Thread could not be switched-to because of kernel stack shortage */
#define MACH_THREAD_BIND           0x2a /* Thread was bound (or unbound) to a processor */
#define MACH_WAITQ_PROMOTE         0x2b /* Thread promoted by waitq boost */
#define MACH_WAITQ_DEMOTE          0x2c /* Thread demoted from waitq boost */
#define MACH_SCHED_LOAD            0x2d /* load update */
#define MACH_REC_CORES_FAILSAFE    0x2e /* recommended processor failsafe kicked in */
#define MACH_SCHED_QUANTUM_EXPIRED 0x2f /* thread quantum expired */
#define MACH_EXEC_PROMOTE          0x30 /* Thread promoted by exec boost */
#define MACH_EXEC_DEMOTE           0x31 /* Thread demoted from exec boost */
#define MACH_AMP_SIGNAL_SPILL      0x32 /* AMP spill signal sent to cpuid */
#define MACH_AMP_STEAL             0x33 /* AMP thread stolen or spilled */
#define MACH_SCHED_LOAD_EFFECTIVE  0x34 /* Effective scheduler load */
/* unused  MACH_PROMOTED              0x35 was: thread promoted due to mutex priority promotion */
/* unused  MACH_UNPROMOTED            0x36 was: thread unpromoted due to mutex priority promotion */
/* unused  MACH_PROMOTED_UPDATE       0x37 was: thread already promoted, but promotion priority changed */
#define MACH_QUIESCENT_COUNTER     0x38 /* quiescent counter tick */
#define MACH_TURNSTILE_USER_CHANGE 0x39 /* base priority change because of turnstile */
#define MACH_AMP_RECOMMENDATION_CHANGE 0x3a /* Thread group recommendation change */
#define MACH_TURNSTILE_KERNEL_CHANGE 0x40 /* sched priority change because of turnstile */

/* Codes for Clutch Scheduler (DBG_MACH_SCHED_CLUTCH) */
#define MACH_SCHED_CLUTCH_ROOT_BUCKET_STATE     0x0
#define MACH_SCHED_CLUTCH_TG_BUCKET_STATE       0x1
#define MACH_SCHED_CLUTCH_THREAD_SELECT         0x2
#define MACH_SCHED_CLUTCH_THREAD_STATE          0x3
#define MACH_SCHED_CLUTCH_TG_BUCKET_PRI         0x4

/* Variants for MACH_MULTIQ_DEQUEUE */
#define MACH_MULTIQ_BOUND     1
#define MACH_MULTIQ_GROUP     2
#define MACH_MULTIQ_GLOBAL    3

/* Arguments for vm_fault (DBG_MACH_VM) */
#define DBG_ZERO_FILL_FAULT   1
#define DBG_PAGEIN_FAULT      2
#define DBG_COW_FAULT         3
#define DBG_CACHE_HIT_FAULT   4
#define DBG_NZF_PAGE_FAULT    5
#define DBG_GUARD_FAULT       6
#define DBG_PAGEINV_FAULT     7
#define DBG_PAGEIND_FAULT     8
#define DBG_COMPRESSOR_FAULT  9
#define DBG_COMPRESSOR_SWAPIN_FAULT  10

/* Codes for IPC (DBG_MACH_IPC) */
#define MACH_TASK_SUSPEND                       0x0     /* Suspended a task */
#define MACH_TASK_RESUME                        0x1     /* Resumed a task */
#define MACH_THREAD_SET_VOUCHER                 0x2
#define MACH_IPC_MSG_SEND                       0x3     /* mach msg send, uniq msg info */
#define MACH_IPC_MSG_RECV                       0x4     /* mach_msg receive */
#define MACH_IPC_MSG_RECV_VOUCHER_REFUSED       0x5     /* mach_msg receive, voucher refused */
#define MACH_IPC_KMSG_FREE                      0x6     /* kernel free of kmsg data */
#define MACH_IPC_VOUCHER_CREATE                 0x7     /* Voucher added to global voucher hashtable */
#define MACH_IPC_VOUCHER_CREATE_ATTR_DATA       0x8     /* Attr data for newly created voucher */
#define MACH_IPC_VOUCHER_DESTROY                0x9     /* Voucher removed from global voucher hashtable */
#define MACH_IPC_KMSG_INFO                      0xa     /* Send/Receive info for a kmsg */
#define MACH_IPC_KMSG_LINK                      0xb     /* link a kernel kmsg pointer to user mach_msg_header_t */
#define MACH_IPC_PORT_ENTRY_MODIFY              0xc     /* A port space gained or lost a port right (reference) */
#define MACH_IPC_DESTROY_GUARDED_DESC           0xd     /* Unable to receive a guarded descriptor */

/* Codes for thread groups (DBG_MACH_THREAD_GROUP) */
#define MACH_THREAD_GROUP_NEW           0x0
#define MACH_THREAD_GROUP_FREE          0x1
#define MACH_THREAD_GROUP_SET           0x2
#define MACH_THREAD_GROUP_NAME          0x3
#define MACH_THREAD_GROUP_NAME_FREE     0x4
#define MACH_THREAD_GROUP_FLAGS         0x5

/* Codes for coalitions (DBG_MACH_COALITION) */
#define MACH_COALITION_NEW                      0x0
#define MACH_COALITION_FREE                     0x1
#define MACH_COALITION_ADOPT                    0x2
#define MACH_COALITION_REMOVE                   0x3
#define MACH_COALITION_THREAD_GROUP_SET         0x4

/* Codes for pmap (DBG_MACH_PMAP) */
#define PMAP__CREATE            0x0
#define PMAP__DESTROY           0x1
#define PMAP__PROTECT           0x2
#define PMAP__PAGE_PROTECT      0x3
#define PMAP__ENTER             0x4
#define PMAP__REMOVE            0x5
#define PMAP__NEST              0x6
#define PMAP__UNNEST            0x7
#define PMAP__FLUSH_TLBS        0x8
#define PMAP__UPDATE_INTERRUPT  0x9
#define PMAP__ATTRIBUTE_CLEAR   0xa
#define PMAP__REUSABLE          0xb     /* This appears to be unused */
#define PMAP__QUERY_RESIDENT    0xc
#define PMAP__FLUSH_KERN_TLBS   0xd
#define PMAP__FLUSH_DELAYED_TLBS        0xe
#define PMAP__FLUSH_TLBS_TO     0xf
#define PMAP__FLUSH_EPT         0x10
#define PMAP__FAST_FAULT        0x11
#define PMAP__SWITCH            0x12
#define PMAP__TTE               0x13
#define PMAP__SWITCH_USER_TTB   0x14
#define PMAP__UPDATE_CACHING    0x15

/* Codes for clock (DBG_MACH_CLOCK) */
#define MACH_EPOCH_CHANGE       0x0     /* wake epoch change */
#define MACH_BRIDGE_RCV_TS      0x1     /* receive timestamp pair from interrupt handler */
#define MACH_BRIDGE_REMOTE_TIME 0x2     /* calculate remote timestamp */
#define MACH_BRIDGE_RESET_TS    0x3     /* reset timestamp conversion parameters */
#define MACH_BRIDGE_TS_PARAMS   0x4     /* recompute timestamp conversion parameters */
#define MACH_BRIDGE_SKIP_TS     0x5     /* skip timestamp */
#define MACH_BRIDGE_TS_MISMATCH 0x6     /* mismatch between predicted and received remote timestamp */
#define MACH_BRIDGE_OBSV_RATE   0x7     /* out of range observed rates */

/* Codes for Stackshot/Microstackshot (DBG_MACH_STACKSHOT) */
#define MICROSTACKSHOT_RECORD   0x0
#define MICROSTACKSHOT_GATHER   0x1

/* Codes for sysdiagnose (DBG_MACH_SYSDIAGNOSE) */
#define SYSDIAGNOSE_NOTIFY_USER 0x0
#define SYSDIAGNOSE_FULL        0x1
#define SYSDIAGNOSE_STACKSHOT   0x2
#define SYSDIAGNOSE_TAILSPIN    0x3

/* Codes for Selective Forced Idle (DBG_MACH_SFI) */
#define SFI_SET_WINDOW                  0x0
#define SFI_CANCEL_WINDOW               0x1
#define SFI_SET_CLASS_OFFTIME           0x2
#define SFI_CANCEL_CLASS_OFFTIME        0x3
#define SFI_THREAD_DEFER                0x4
#define SFI_OFF_TIMER                   0x5
#define SFI_ON_TIMER                    0x6
#define SFI_WAIT_CANCELED               0x7
#define SFI_PID_SET_MANAGED             0x8
#define SFI_PID_CLEAR_MANAGED           0x9
#define SFI_GLOBAL_DEFER                0xa

/* Codes for Zone Allocator (DBG_MACH_ZALLOC) */
#define ZALLOC_ZCRAM                    0x0

/* Codes for Mach resource management (DBG_MACH_RESOURCE) */
/* _K32A/B codes start at double the low nibble */
#define RMON_ENABLE_CPUUSAGE_MONITOR    0x001
#define RMON_CPUUSAGE_VIOLATED          0x002
#define RMON_CPUUSAGE_SUSPENDED         0x003
#define RMON_CPUUSAGE_VIOLATED_K32A     0x004
#define RMON_CPUUSAGE_VIOLATED_K32B     0x005
#define RMON_CPUUSAGE_RESUMED           0x006
#define RMON_DISABLE_CPUUSAGE_MONITOR   0x00f

#define RMON_ENABLE_CPUWAKES_MONITOR    0x011
#define RMON_CPUWAKES_VIOLATED          0x012
#define RMON_CPUWAKES_VIOLATED_K32A     0x014
#define RMON_CPUWAKES_VIOLATED_K32B     0x015
#define RMON_DISABLE_CPUWAKES_MONITOR   0x01f

#define RMON_ENABLE_IO_MONITOR          0x021
#define RMON_LOGWRITES_VIOLATED         0x022
#define RMON_PHYSWRITES_VIOLATED        0x023
#define RMON_LOGWRITES_VIOLATED_K32A    0x024
#define RMON_LOGWRITES_VIOLATED_K32B    0x025
#define RMON_DISABLE_IO_MONITOR         0x02f

/* **** The Kernel Debug Sub Classes for Network (DBG_NETWORK) **** */
#define DBG_NETIP       1       /* Internet Protocol */
#define DBG_NETARP      2       /* Address Resolution Protocol */
#define DBG_NETUDP      3       /* User Datagram Protocol */
#define DBG_NETTCP      4       /* Transmission Control Protocol */
#define DBG_NETICMP     5       /* Internet Control Message Protocol */
#define DBG_NETIGMP     6       /* Internet Group Management Protocol */
#define DBG_NETRIP      7       /* Routing Information Protocol */
#define DBG_NETOSPF     8       /* Open Shortest Path First */
#define DBG_NETISIS     9       /* Intermediate System to Intermediate System */
#define DBG_NETSNMP     10      /* Simple Network Management Protocol */
#define DBG_NETSOCK     11      /* Socket Layer */

/* For Apple talk */
#define DBG_NETAARP     100     /* Apple ARP */
#define DBG_NETDDP      101     /* Datagram Delivery Protocol */
#define DBG_NETNBP      102     /* Name Binding Protocol */
#define DBG_NETZIP      103     /* Zone Information Protocol */
#define DBG_NETADSP     104     /* Name Binding Protocol */
#define DBG_NETATP      105     /* Apple Transaction Protocol */
#define DBG_NETASP      106     /* Apple Session Protocol */
#define DBG_NETAFP      107     /* Apple Filing Protocol */
#define DBG_NETRTMP     108     /* Routing Table Maintenance Protocol */
#define DBG_NETAURP     109     /* Apple Update Routing Protocol */

#define DBG_NETIPSEC    128     /* IPsec Protocol  */
#define DBG_NETVMNET    129     /* VMNet */

/* **** The Kernel Debug Sub Classes for IOKIT (DBG_IOKIT) **** */
#define DBG_IOINTC                      0       /* Interrupt controller */
#define DBG_IOWORKLOOP                  1       /* Work from work loop */
#define DBG_IOINTES                     2       /* Interrupt event source */
#define DBG_IOCLKES                     3       /* Clock event source */
#define DBG_IOCMDQ                      4       /* Command queue latencies */
#define DBG_IOMCURS                     5       /* Memory Cursor */
#define DBG_IOMDESC                     6       /* Memory Descriptors */
#define DBG_IOPOWER                     7       /* Power Managerment */
#define DBG_IOSERVICE                   8       /* Matching etc. */
#define DBG_IOREGISTRY                  9       /* Registry */

/* **** 9-32 reserved for internal IOKit usage **** */

#define DBG_IOSTORAGE           32      /* Storage layers */
#define DBG_IONETWORK           33      /* Network layers */
#define DBG_IOKEYBOARD          34      /* Keyboard */
#define DBG_IOHID               35      /* HID Devices */
#define DBG_IOAUDIO             36      /* Audio */
#define DBG_IOSERIAL            37      /* Serial */
#define DBG_IOTTY               38      /* TTY layers */
#define DBG_IOSAM               39      /* SCSI Architecture Model layers */
#define DBG_IOPARALLELATA       40      /* Parallel ATA */
#define DBG_IOPARALLELSCSI      41      /* Parallel SCSI */
#define DBG_IOSATA              42      /* Serial-ATA */
#define DBG_IOSAS               43      /* SAS */
#define DBG_IOFIBRECHANNEL      44      /* FiberChannel */
#define DBG_IOUSB               45      /* USB */
#define DBG_IOBLUETOOTH         46      /* Bluetooth */
#define DBG_IOFIREWIRE          47      /* FireWire */
#define DBG_IOINFINIBAND        48      /* Infiniband */
#define DBG_IOCPUPM             49      /* CPU Power Management */
#define DBG_IOGRAPHICS          50      /* Graphics */
#define DBG_HIBERNATE           51      /* hibernation related events */
#define DBG_IOTHUNDERBOLT       52      /* Thunderbolt */
#define DBG_BOOTER              53      /* booter related events */
#define DBG_IOAUDIO2            54      /* Audio (extended) */

#define DBG_IOSURFACEPA         64      /* IOSurface page mappings */
#define DBG_IOMDPA              65      /* IOMemoryDescriptor page mappings */
#define DBG_IODARTPA            66      /* DART page mappings */
/* **** 67-79 reserved for physical address mapping information **** */

/* Backwards compatibility */
#define DBG_IOPOINTING          DBG_IOHID                       /* OBSOLETE: Use DBG_IOHID instead */
#define DBG_IODISK                      DBG_IOSTORAGE           /* OBSOLETE: Use DBG_IOSTORAGE instead */

/* **** The Kernel Debug Sub Classes for Device Drivers (DBG_DRIVERS) **** */
#define DBG_DRVSTORAGE        1 /* Storage layers */
#define DBG_DRVNETWORK        2 /* Network layers */
#define DBG_DRVKEYBOARD       3 /* Keyboard */
#define DBG_DRVHID            4 /* HID Devices */
#define DBG_DRVAUDIO          5 /* Audio */
#define DBG_DRVSERIAL         7 /* Serial */
#define DBG_DRVSAM            8 /* SCSI Architecture Model layers */
#define DBG_DRVPARALLELATA    9 /* Parallel ATA */
#define DBG_DRVPARALLELSCSI  10 /* Parallel SCSI */
#define DBG_DRVSATA          11 /* Serial ATA */
#define DBG_DRVSAS           12 /* SAS */
#define DBG_DRVFIBRECHANNEL  13 /* FiberChannel */
#define DBG_DRVUSB           14 /* USB */
#define DBG_DRVBLUETOOTH     15 /* Bluetooth */
#define DBG_DRVFIREWIRE      16 /* FireWire */
#define DBG_DRVINFINIBAND    17 /* Infiniband */
#define DBG_DRVGRAPHICS      18 /* Graphics */
#define DBG_DRVSD            19 /* Secure Digital */
#define DBG_DRVNAND          20 /* NAND drivers and layers */
#define DBG_SSD              21 /* SSD */
#define DBG_DRVSPI           22 /* SPI */
#define DBG_DRVWLAN_802_11   23 /* WLAN 802.11 */
#define DBG_DRVSSM           24 /* System State Manager(AppleSSM) */
#define DBG_DRVSMC           25 /* System Management Controller */
#define DBG_DRVMACEFIMANAGER 26 /* Mac EFI Manager */
#define DBG_DRVANE           27 /* ANE */
#define DBG_DRVETHERNET      28 /* Ethernet */
#define DBG_DRVMCC           29 /* Memory Cache Controller */
#define DBG_DRVACCESSORY     30 /* Accessories */

/* Backwards compatibility */
#define DBG_DRVPOINTING         DBG_DRVHID      /* OBSOLETE: Use DBG_DRVHID instead */
#define DBG_DRVDISK             DBG_DRVSTORAGE  /* OBSOLETE: Use DBG_DRVSTORAGE instead */

/* **** The Kernel Debug Sub Classes for the DLIL Layer (DBG_DLIL) **** */
#define DBG_DLIL_STATIC 1       /* Static DLIL code */
#define DBG_DLIL_PR_MOD 2       /* DLIL Protocol Module */
#define DBG_DLIL_IF_MOD 3       /* DLIL Interface Module */
#define DBG_DLIL_PR_FLT 4       /* DLIL Protocol Filter */
#define DBG_DLIL_IF_FLT 5       /* DLIL Interface FIlter */

/* The Kernel Debug Sub Classes for File System (DBG_FSYSTEM) */
#define DBG_FSRW      0x1     /* reads and writes to the filesystem */
#define DBG_DKRW      0x2     /* reads and writes to the disk */
#define DBG_FSVN      0x3     /* vnode operations (inc. locking/unlocking) */
#define DBG_FSLOOOKUP 0x4     /* namei and other lookup-related operations */
#define DBG_JOURNAL   0x5     /* journaling operations */
#define DBG_IOCTL     0x6     /* ioctl to the disk */
#define DBG_BOOTCACHE 0x7     /* bootcache operations */
#define DBG_HFS       0x8     /* HFS-specific events; see the hfs project */
#define DBG_APFS      0x9     /* APFS-specific events; see the apfs project */
#define DBG_SMB       0xA     /* SMB-specific events; see the smb project */
#define DBG_MOUNT     0xB     /* Mounting/unmounting operations */
#define DBG_EXFAT     0xE     /* ExFAT-specific events; see the exfat project */
#define DBG_MSDOS     0xF     /* FAT-specific events; see the msdosfs project */
#define DBG_ACFS      0x10    /* Xsan-specific events; see the XsanFS project */
#define DBG_THROTTLE  0x11    /* I/O Throttling events */
#define DBG_DECMP     0x12    /* Decmpfs-specific events */
#define DBG_VFS       0x13    /* VFS layer events */
#define DBG_LIVEFS    0x14    /* LiveFS events; see the UserFS project */
#define DBG_CONTENT_PROT 0xCF /* Content Protection Events: see bsd/sys/cprotect.h */

/*
 * For Kernel Debug Sub Class DBG_HFS, state bits for hfs_update event
 */
#define DBG_HFS_UPDATE_ACCTIME   0x01
#define DBG_HFS_UPDATE_MODTIME   0x02
#define DBG_HFS_UPDATE_CHGTIME   0x04
#define DBG_HFS_UPDATE_MODIFIED  0x08
#define DBG_HFS_UPDATE_FORCE     0x10
#define DBG_HFS_UPDATE_DATEADDED 0x20
#define DBG_HFS_UPDATE_MINOR     0x40
#define DBG_HFS_UPDATE_SKIPPED   0x80

/* The Kernel Debug Sub Classes for BSD */
#define DBG_BSD_PROC              0x01 /* process/signals related */
#define DBG_BSD_MEMSTAT           0x02 /* memorystatus / jetsam operations */
#define DBG_BSD_KEVENT            0x03 /* kqueue / kevent related */
#define DBG_BSD_EXCP_SC           0x0C /* System Calls */
#define DBG_BSD_AIO               0x0D /* aio (POSIX async IO) */
#define DBG_BSD_SC_EXTENDED_INFO  0x0E /* System Calls, extended info */
#define DBG_BSD_SC_EXTENDED_INFO2 0x0F /* System Calls, extended info */
#define DBG_BSD_KDEBUG_TEST       0xFF /* for testing kdebug */

/* The Codes for BSD subcode class DBG_BSD_PROC */
#define BSD_PROC_EXIT              1  /* process exit */
#define BSD_PROC_FRCEXIT           2  /* Kernel force termination */
#define BSD_PROC_EXEC              3  /* process spawn / exec */
#define BSD_PROC_EXITREASON_CREATE 4  /* exit reason creation */
#define BSD_PROC_EXITREASON_COMMIT 5  /* exit reason commited to a proc */

/* Codes for BSD subcode class DBG_BSD_MEMSTAT */
#define BSD_MEMSTAT_SCAN             1  /* memorystatus thread awake */
#define BSD_MEMSTAT_JETSAM           2  /* LRU jetsam */
#define BSD_MEMSTAT_JETSAM_HIWAT     3  /* highwater jetsam */
#define BSD_MEMSTAT_FREEZE           4  /* freeze process */
#define BSD_MEMSTAT_FREEZE_SCAN      5  /* select a process to freeze and freeze it */
#define BSD_MEMSTAT_UPDATE           6  /* priority update */
#define BSD_MEMSTAT_IDLE_DEMOTE      7  /* idle demotion fired */
#define BSD_MEMSTAT_CLEAR_ERRORS     8  /* reset termination error state */
#define BSD_MEMSTAT_DIRTY_TRACK      9  /* track the process state */
#define BSD_MEMSTAT_DIRTY_SET       10  /* set the process state */
#define BSD_MEMSTAT_DIRTY_CLEAR     11  /* clear the process state */
#ifdef  PRIVATE
#define BSD_MEMSTAT_GRP_SET_PROP    12  /* set group properties */
#define BSD_MEMSTAT_DO_KILL         13  /* memorystatus kills */
#define BSD_MEMSTAT_CHANGE_PRIORITY 14  /* priority changed */
#endif /* PRIVATE */
#define BSD_MEMSTAT_FAST_JETSAM     15  /* Aggressive jetsam ("clear-the-deck") */
#define BSD_MEMSTAT_COMPACTOR_RUN   16  /* run VM compactor after process kill */
#define BSD_MEMSTAT_FREEZE_DISABLE  17  /* disable freeze and kill frozen processes */
#define BSD_MEMSTAT_RELAUNCH_FLAGS  18  /* flags representing jetsam behavior; based on launchd data */

/* Codes for BSD subcode class DBG_BSD_KEVENT */
#define BSD_KEVENT_KQ_PROCESS_BEGIN   1
#define BSD_KEVENT_KQ_PROCESS_END     2
#define BSD_KEVENT_KQWQ_PROCESS_BEGIN 3
#define BSD_KEVENT_KQWQ_PROCESS_END   4
#define BSD_KEVENT_KQWQ_BIND          5
#define BSD_KEVENT_KQWQ_UNBIND        6
#define BSD_KEVENT_KQWQ_THREQUEST     7
#define BSD_KEVENT_KQWL_PROCESS_BEGIN 8
#define BSD_KEVENT_KQWL_PROCESS_END   9
#define BSD_KEVENT_KQWL_THREQUEST     10
#define BSD_KEVENT_KQWL_THADJUST      11
#define BSD_KEVENT_KQ_REGISTER        12
#define BSD_KEVENT_KQWQ_REGISTER      13
#define BSD_KEVENT_KQWL_REGISTER      14
#define BSD_KEVENT_KNOTE_ACTIVATE     15
#define BSD_KEVENT_KQ_PROCESS         16
#define BSD_KEVENT_KQWQ_PROCESS       17
#define BSD_KEVENT_KQWL_PROCESS       18
#define BSD_KEVENT_KQWL_BIND          19
#define BSD_KEVENT_KQWL_UNBIND        20
#define BSD_KEVENT_KNOTE_ENABLE       21
#define BSD_KEVENT_KNOTE_VANISHED     22

/* The Kernel Debug Sub Classes for DBG_TRACE */
#define DBG_TRACE_DATA      0
#define DBG_TRACE_STRING    1
#define DBG_TRACE_INFO      2

/* The Kernel Debug events: */
#define TRACE_DATA_NEWTHREAD            (TRACEDBG_CODE(DBG_TRACE_DATA, 1))
#define TRACE_DATA_EXEC                 (TRACEDBG_CODE(DBG_TRACE_DATA, 2))
#define TRACE_DATA_THREAD_TERMINATE     (TRACEDBG_CODE(DBG_TRACE_DATA, 3))
#define TRACE_DATA_THREAD_TERMINATE_PID (TRACEDBG_CODE(DBG_TRACE_DATA, 4))
#define TRACE_STRING_GLOBAL             (TRACEDBG_CODE(DBG_TRACE_STRING, 0))
#define TRACE_STRING_NEWTHREAD          (TRACEDBG_CODE(DBG_TRACE_STRING, 1))
#define TRACE_STRING_EXEC               (TRACEDBG_CODE(DBG_TRACE_STRING, 2))
#define TRACE_STRING_PROC_EXIT          (TRACEDBG_CODE(DBG_TRACE_STRING, 3))
#define TRACE_STRING_THREADNAME         (TRACEDBG_CODE(DBG_TRACE_STRING, 4))
#define TRACE_STRING_THREADNAME_PREV    (TRACEDBG_CODE(DBG_TRACE_STRING, 5))
#define TRACE_PANIC                     (TRACEDBG_CODE(DBG_TRACE_INFO, 0))
#define TRACE_TIMESTAMPS                (TRACEDBG_CODE(DBG_TRACE_INFO, 1))
#define TRACE_LOST_EVENTS               (TRACEDBG_CODE(DBG_TRACE_INFO, 2))
#define TRACE_WRITING_EVENTS            (TRACEDBG_CODE(DBG_TRACE_INFO, 3))
#define TRACE_INFO_STRING               (TRACEDBG_CODE(DBG_TRACE_INFO, 4))
#define TRACE_RETROGRADE_EVENTS         (TRACEDBG_CODE(DBG_TRACE_INFO, 5))

/* The Kernel Debug Sub Classes for DBG_CORESTORAGE */
#define DBG_CS_IO       0

/* The Kernel Debug Sub Classes for DBG_SECURITY */
#define DBG_SEC_KERNEL  0 /* raw entropy collected by the kernel */
#define DBG_SEC_SANDBOX 1

/* Sub-class codes for CoreGraphics (DBG_CG) are defined in its component. */

/* The Kernel Debug Sub Classes for DBG_MONOTONIC */
#define DBG_MT_INSTRS_CYCLES 1
#define DBG_MT_DEBUG 2
#define DBG_MT_TMPTH 0xfe
#define DBG_MT_TMPCPU 0xff

/* The Kernel Debug Sub Classes for DBG_MISC */
#define DBG_MISC_COREBRIGHTNESS 0x01
#define DBG_EVENT               0x10
#define DBG_MISC_INSTRUMENTS    0x11
#define DBG_MISC_INSTRUMENTSBT  0x12
#define DBG_MISC_LAYOUT         0x1a
#define DBG_BUFFER              0x20

/* The Kernel Debug Sub Classes for DBG_DYLD */
#define DBG_DYLD_UUID (5)

/* Kernel Debug codes for the DBG_DYLD_UUID subclass */
#define DBG_DYLD_UUID_MAP_A             (0)
#define DBG_DYLD_UUID_MAP_B             (1)
#define DBG_DYLD_UUID_MAP_32_A          (2)
#define DBG_DYLD_UUID_MAP_32_B          (3)
#define DBG_DYLD_UUID_MAP_32_C          (4)
#define DBG_DYLD_UUID_UNMAP_A           (5)
#define DBG_DYLD_UUID_UNMAP_B           (6)
#define DBG_DYLD_UUID_UNMAP_32_A        (7)
#define DBG_DYLD_UUID_UNMAP_32_B        (8)
#define DBG_DYLD_UUID_UNMAP_32_C        (9)
#define DBG_DYLD_UUID_SHARED_CACHE_A    (10)
#define DBG_DYLD_UUID_SHARED_CACHE_B    (11)
#define DBG_DYLD_UUID_SHARED_CACHE_32_A (12)
#define DBG_DYLD_UUID_SHARED_CACHE_32_B (13)
#define DBG_DYLD_UUID_SHARED_CACHE_32_C (14)

/* The Kernel Debug modifiers for the DBG_DKRW sub class */
#define DKIO_DONE       0x01
#define DKIO_READ       0x02
#define DKIO_ASYNC      0x04
#define DKIO_META       0x08
#define DKIO_PAGING     0x10
#define DKIO_THROTTLE   0x20 /* Deprecated, still provided so fs_usage doesn't break */
#define DKIO_PASSIVE    0x40
#define DKIO_NOCACHE    0x80
#define DKIO_TIER_MASK  0xF00
#define DKIO_TIER_SHIFT 8
#define DKIO_TIER_UPGRADE 0x1000

/* Kernel Debug Sub Classes for Applications (DBG_APPS) */
#define DBG_APP_LOGINWINDOW     0x03
#define DBG_APP_AUDIO           0x04
#define DBG_APP_SYSTEMUI        0x05
#define DBG_APP_SIGNPOST        0x0A
#define DBG_APP_APPKIT          0x0C
#define DBG_APP_UIKIT           0x0D
#define DBG_APP_DFR             0x0E
#define DBG_APP_LAYOUT          0x0F
#define DBG_APP_COREDATA        0x10
#define DBG_APP_SAMBA           0x80
#define DBG_APP_EOSSUPPORT      0x81
#define DBG_APP_MACEFIMANAGER   0x82

/* Kernel Debug codes for Throttling (DBG_THROTTLE) */
#define OPEN_THROTTLE_WINDOW    0x1
#define PROCESS_THROTTLED       0x2
#define IO_THROTTLE_DISABLE     0x3
#define IO_TIER_UPL_MISMATCH    0x4

/* Subclasses for MACH Importance Policies (DBG_IMPORTANCE) */
/* TODO: Split up boost and task policy? */
#define IMP_ASSERTION                       0x10    /* Task takes/drops a boost assertion */
#define IMP_BOOST                           0x11    /* Task boost level changed */
#define IMP_MSG                             0x12    /* boosting message sent by donating task on donating port */
#define IMP_WATCHPORT                       0x13    /* port marked as watchport, and boost was transferred to the watched task */
#define IMP_TASK_SUPPRESSION                0x17    /* Task changed suppression behaviors */
#define IMP_TASK_APPTYPE                    0x18    /* Task launched with apptype */
#define IMP_UPDATE                          0x19    /* Requested -> effective calculation */
#define IMP_USYNCH_QOS_OVERRIDE             0x1A    /* Userspace synchronization applied QoS override to resource owning thread */
#define IMP_DONOR_CHANGE                    0x1B    /* The iit_donor bit changed */
#define IMP_MAIN_THREAD_QOS                 0x1C    /* The task's main thread QoS was set */
#define IMP_SYNC_IPC_QOS                    0x1D    /* Sync IPC QOS override */
/* DBG_IMPORTANCE subclasses  0x20 - 0x3F are reserved for task policy flavors */

/* thread and task attributes */
#define IMP_TASK_POLICY_DARWIN_BG           0x21
#define IMP_TASK_POLICY_IOPOL               0x22
#define IMP_TASK_POLICY_IO                  0x23
#define IMP_TASK_POLICY_PASSIVE_IO          0x24

/* task only attributes */
#define IMP_TASK_POLICY_DARWIN_BG_IOPOL     0x27
#define IMP_TASK_POLICY_TAL                 0x28
#define IMP_TASK_POLICY_BOOST               0x29
#define IMP_TASK_POLICY_ROLE                0x2A
/* unused                                   0x2B */
#define IMP_TASK_POLICY_TERMINATED          0x2C
#define IMP_TASK_POLICY_NEW_SOCKETS_BG      0x2D
#define IMP_TASK_POLICY_SUP_ACTIVE          0x2E
#define IMP_TASK_POLICY_LATENCY_QOS         0x2F
#define IMP_TASK_POLICY_THROUGH_QOS         0x30
#define IMP_TASK_POLICY_WATCHERS_BG         0x31

#define IMP_TASK_POLICY_SFI_MANAGED         0x34
#define IMP_TASK_POLICY_ALL_SOCKETS_BG      0x37

#define IMP_TASK_POLICY_BASE_LATENCY_AND_THROUGHPUT_QOS  0x39 /* latency as value1, throughput as value2 */
#define IMP_TASK_POLICY_OVERRIDE_LATENCY_AND_THROUGHPUT_QOS  0x3A /* latency as value1, throughput as value2 */

/* thread only attributes */
#define IMP_TASK_POLICY_PIDBIND_BG          0x32
/* unused                                   0x33 */
/* reserved                                 0x35 */
#define IMP_TASK_POLICY_QOS_OVERRIDE        0x36
#define IMP_TASK_POLICY_QOS_AND_RELPRIO     0x38 /* QoS as value1, relative priority as value2 */
#define IMP_TASK_POLICY_QOS_WORKQ_OVERRIDE  0x3B
#define IMP_TASK_POLICY_QOS_PROMOTE         0x3C
#define IMP_TASK_POLICY_QOS_KEVENT_OVERRIDE 0x3D
#define IMP_TASK_POLICY_QOS_IPC_OVERRIDE    IMP_TASK_POLICY_QOS_KEVENT_OVERRIDE /* legacy name */
#define IMP_TASK_POLICY_QOS_SERVICER_OVERRIDE 0x3E

/* Codes for IMP_ASSERTION */
#define IMP_HOLD                0x2     /* Task holds a boost assertion */
#define IMP_DROP                0x4     /* Task drops a boost assertion */
#define IMP_EXTERN              0x8     /* boost assertion moved from kernel to userspace responsibility (externalized) */

/* Codes for IMP_BOOST */
#define IMP_BOOSTED             0x1
#define IMP_UNBOOSTED           0x2     /* Task drops a boost assertion */

/* Codes for IMP_MSG */
#define IMP_MSG_SEND            0x1     /* boosting message sent by donating task on donating port */
#define IMP_MSG_DELV            0x2     /* boosting message delivered to task */

/* Codes for IMP_UPDATE */
#define IMP_UPDATE_TASK_CREATE  0x1

/* Codes for IMP_USYNCH_QOS_OVERRIDE */
#define IMP_USYNCH_ADD_OVERRIDE         0x0             /* add override for a contended resource */
#define IMP_USYNCH_REMOVE_OVERRIDE      0x1             /* remove override for a contended resource */

/* Codes for IMP_DONOR_CHANGE */
#define IMP_DONOR_UPDATE_LIVE_DONOR_STATE       0x0
#define IMP_DONOR_INIT_DONOR_STATE              0x1

/* Code for IMP_SYNC_IPC_QOS */
#define IMP_SYNC_IPC_QOS_APPLIED                0x0
#define IMP_SYNC_IPC_QOS_REMOVED                0x1
#define IMP_SYNC_IPC_QOS_OVERFLOW               0x2
#define IMP_SYNC_IPC_QOS_UNDERFLOW              0x3

/* Subclasses for Turnstiles (DBG_TURNSTILE) */
#define TURNSTILE_HEAP_OPERATIONS               0x10
#define TURNSTILE_PRIORITY_OPERATIONS           0x20
#define TURNSTILE_FREELIST_OPERATIONS           0x30

/* Codes for TURNSTILE_HEAP_OPERATIONS */
#define THREAD_ADDED_TO_TURNSTILE_WAITQ         0x1
#define THREAD_REMOVED_FROM_TURNSTILE_WAITQ     0x2
#define THREAD_MOVED_IN_TURNSTILE_WAITQ         0x3
#define TURNSTILE_ADDED_TO_TURNSTILE_HEAP       0x4
#define TURNSTILE_REMOVED_FROM_TURNSTILE_HEAP   0x5
#define TURNSTILE_MOVED_IN_TURNSTILE_HEAP       0x6
#define TURNSTILE_ADDED_TO_THREAD_HEAP          0x7
#define TURNSTILE_REMOVED_FROM_THREAD_HEAP      0x8
#define TURNSTILE_MOVED_IN_THREAD_HEAP          0x9
#define TURNSTILE_UPDATE_STOPPED_BY_LIMIT       0xa
#define THREAD_NOT_WAITING_ON_TURNSTILE         0xb

/* Codes for TURNSTILE_PRIORITY_OPERATIONS */
#define TURNSTILE_PRIORITY_CHANGE               0x1
#define THREAD_USER_PROMOTION_CHANGE            0x2

/* Codes for TURNSTILE_FREELIST_OPERATIONS */
#define TURNSTILE_PREPARE                       0x1
#define TURNSTILE_COMPLETE                      0x2

/* Subclasses for MACH Bank Voucher Attribute Manager (DBG_BANK) */
#define BANK_ACCOUNT_INFO               0x10    /* Trace points related to bank account struct */
#define BANK_TASK_INFO                  0x11    /* Trace points related to bank task struct */

/* Subclasses for MACH ATM Voucher Attribute Manager (ATM) */
#define ATM_SUBAID_INFO                 0x10
#define ATM_GETVALUE_INFO               0x20
#define ATM_UNREGISTER_INFO             0x30

/* Codes for BANK_ACCOUNT_INFO */
#define BANK_SETTLE_CPU_TIME            0x1     /* Bank ledger(chit) rolled up to tasks. */
#define BANK_SECURE_ORIGINATOR_CHANGED  0x2     /* Secure Originator changed. */
#define BANK_SETTLE_ENERGY              0x3     /* Bank ledger(energy field) rolled up to tasks. */

/* Codes for ATM_SUBAID_INFO */
#define ATM_MIN_CALLED                          0x1
#define ATM_LINK_LIST_TRIM                      0x2

/* Codes for ATM_GETVALUE_INFO */
#define ATM_VALUE_REPLACED                      0x1
#define ATM_VALUE_ADDED                         0x2

/* Codes for ATM_UNREGISTER_INFO */
#define ATM_VALUE_UNREGISTERED                  0x1
#define ATM_VALUE_DIFF_MAILBOX                  0x2

/* Kernel Debug Sub Classes for daemons (DBG_DAEMON) */
#define DBG_DAEMON_COREDUET 0x1
#define DBG_DAEMON_POWERD   0x2

/* Subclasses for the user space allocator */
#define DBG_UMALLOC_EXTERNAL                    0x1
#define DBG_UMALLOC_INTERNAL                    0x2

/**********************************************************************/

#define KDBG_MIGCODE(msgid) (((unsigned)DBG_MIG << KDBG_CLASS_OFFSET) | \
	                     ((unsigned)((msgid) & 0x3fffff) << KDBG_CODE_OFFSET))

#define MACHDBG_CODE(SubClass, code) KDBG_CODE(DBG_MACH, SubClass, code)
#define NETDBG_CODE(SubClass, code) KDBG_CODE(DBG_NETWORK, SubClass, code)
#define FSDBG_CODE(SubClass, code) KDBG_CODE(DBG_FSYSTEM, SubClass, code)
#define BSDDBG_CODE(SubClass, code) KDBG_CODE(DBG_BSD, SubClass, code)
#define IOKDBG_CODE(SubClass, code) KDBG_CODE(DBG_IOKIT, SubClass, code)
#define DRVDBG_CODE(SubClass, code) KDBG_CODE(DBG_DRIVERS, SubClass, code)
#define TRACEDBG_CODE(SubClass, code) KDBG_CODE(DBG_TRACE, SubClass, code)
#define SILICONDBG_CODE(SubClass, code) KDBG_CODE(DBG_SILICON, SubClass, code)
#define MISCDBG_CODE(SubClass, code) KDBG_CODE(DBG_MISC, SubClass, code)
#define DLILDBG_CODE(SubClass, code) KDBG_CODE(DBG_DLIL, SubClass, code)
#define SECURITYDBG_CODE(SubClass, code) KDBG_CODE(DBG_SECURITY, SubClass, code)
#define DYLDDBG_CODE(SubClass, code) KDBG_CODE(DBG_DYLD, SubClass, code)
#define QTDBG_CODE(SubClass, code) KDBG_CODE(DBG_QT, SubClass, code)
#define APPSDBG_CODE(SubClass, code) KDBG_CODE(DBG_APPS, SubClass, code)
#define ARIADNEDBG_CODE(SubClass, code) KDBG_CODE(DBG_ARIADNE, SubClass, code)
#define DAEMONDBG_CODE(SubClass, code) KDBG_CODE(DBG_DAEMON, SubClass, code)
#define CPUPM_CODE(code) IOKDBG_CODE(DBG_IOCPUPM, code)

#define KMEM_ALLOC_CODE MACHDBG_CODE(DBG_MACH_LEAKS, 0)
#define KMEM_ALLOC_CODE_2 MACHDBG_CODE(DBG_MACH_LEAKS, 1)
#define KMEM_FREE_CODE MACHDBG_CODE(DBG_MACH_LEAKS, 2)
#define KMEM_FREE_CODE_2 MACHDBG_CODE(DBG_MACH_LEAKS, 3)
#define ZALLOC_CODE MACHDBG_CODE(DBG_MACH_LEAKS, 4)
#define ZALLOC_CODE_2 MACHDBG_CODE(DBG_MACH_LEAKS, 5)
#define ZFREE_CODE MACHDBG_CODE(DBG_MACH_LEAKS, 6)
#define ZFREE_CODE_2 MACHDBG_CODE(DBG_MACH_LEAKS, 7)

#define PMAP_CODE(code) MACHDBG_CODE(DBG_MACH_PMAP, code)

#define IMPORTANCE_CODE(SubClass, code) KDBG_CODE(DBG_IMPORTANCE, (SubClass), (code))
#define BANK_CODE(SubClass, code) KDBG_CODE(DBG_BANK, (SubClass), (code))
#define ATM_CODE(SubClass, code) KDBG_CODE(DBG_ATM, (SubClass), (code))
#define TURNSTILE_CODE(SubClass, code) KDBG_CODE(DBG_TURNSTILE, (SubClass), (code))

/* Kernel Debug Macros for specific daemons */
#define COREDUETDBG_CODE(code) DAEMONDBG_CODE(DBG_DAEMON_COREDUET, code)
#define POWERDDBG_CODE(code) DAEMONDBG_CODE(DBG_DAEMON_POWERD, code)

/* VFS lookup events for serial traces */
#define VFS_LOOKUP      (FSDBG_CODE(DBG_FSRW,36))
#define VFS_LOOKUP_DONE (FSDBG_CODE(DBG_FSRW,39))

#endif /* __APPLE_API_UNSTABLE */

__END_DECLS

#if defined(__has_include) && __has_include(<sys/kdebug_private.h>)
#include <sys/kdebug_private.h>
#endif /* __has_include(<sys/kdebug_private.h>) */

#ifdef KERNEL
#include <sys/kdebug_kernel.h>
#endif /* defined(KERNEL) */

#endif /* !defined(BSD_SYS_KDEBUG_H) */

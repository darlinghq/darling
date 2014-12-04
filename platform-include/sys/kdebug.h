/*
 * Copyright (c) 2000-2014 Apple Inc. All rights reserved.
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

/* 	Copyright (c) 1997 Apple Computer, Inc.  All rights reserved. 
 *
 * kdebug.h -   kernel_debug definitions
 *
 */

#ifndef BSD_SYS_KDEBUG_H
#define BSD_SYS_KDEBUG_H

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
__BEGIN_DECLS

#ifdef __APPLE_API_UNSTABLE

#include <mach/clock_types.h>
#include <stdint.h>
#if	defined(KERNEL_BUILD)
#include <kdebug.h>
#endif /* KERNEL_BUILD */


typedef enum
{
	KD_CALLBACK_KDEBUG_ENABLED,   		// Trace is now enabled. No arguments
	KD_CALLBACK_KDEBUG_DISABLED,  		// Trace is now disabled. No arguments
	KD_CALLBACK_SYNC_FLUSH,      		// Request the latest entries from the IOP, and block until complete. No arguments
	KD_CALLBACK_TYPEFILTER_CHANGED,		// Typefilter is enabled. A read-only pointer to the typefilter is provided, but is only valid while in the callback.
} kd_callback_type;
typedef void (*kd_callback_fn) (void* context, kd_callback_type reason, void* arg);

struct kd_callback {
	kd_callback_fn	func;
	void*		context;
	char		iop_name[8]; // null-terminated string with name of core.
};

typedef struct kd_callback kd_callback_t;

/*
 * Registers an IOP for participation in tracing. 
 *  
 * The registered callback function will be called with the 
 * supplied context as the first argument, followed by a 
 * kd_callback_type and an associated void* argument. 
 *  
 * The return value is a nonzero coreid that shall be used in 
 * kernel_debug_enter() to refer to your IOP. If the allocation 
 * failed, then 0 will be returned. 
 *  
 *  
 * Caveats: 
 * Note that not all callback calls will indicate a change in 
 * state (e.g. disabling trace twice would send two disable 
 * notifications). 
 *  
 */
extern int kernel_debug_register_callback(kd_callback_t callback);

extern void kernel_debug_enter(
	uint32_t	coreid,
	uint32_t	debugid,
	uint64_t	timestamp,
	uintptr_t	arg1,
	uintptr_t	arg2,
	uintptr_t	arg3,
	uintptr_t	arg4,
	uintptr_t	threadid
	);


/*
 * state bits for hfs_update event
 */
#define DBG_HFS_UPDATE_ACCTIME   0x01
#define DBG_HFS_UPDATE_MODTIME	 0x02
#define DBG_HFS_UPDATE_CHGTIME	 0x04
#define DBG_HFS_UPDATE_MODIFIED	 0x08
#define DBG_HFS_UPDATE_FORCE	 0x10
#define DBG_HFS_UPDATE_DATEADDED 0x20


/*
 * types of faults that vm_fault handles
 * and creates trace entries for
 */
#define DBG_ZERO_FILL_FAULT   1
#define DBG_PAGEIN_FAULT      2
#define DBG_COW_FAULT         3
#define DBG_CACHE_HIT_FAULT   4
#define DBG_NZF_PAGE_FAULT    5
#define DBG_GUARD_FAULT	      6	
#define DBG_PAGEINV_FAULT     7
#define DBG_PAGEIND_FAULT     8
#define DBG_COMPRESSOR_FAULT  9
#define DBG_COMPRESSOR_SWAPIN_FAULT  10


/* The debug code consists of the following 
*
* ----------------------------------------------------------------------
*|              |               |                               |Func   |
*| Class (8)    | SubClass (8)  |          Code (14)            |Qual(2)|
* ----------------------------------------------------------------------
* The class specifies the higher level 
*/

/* The Function qualifiers  */
#define DBG_FUNC_START		1
#define DBG_FUNC_END		2
#define DBG_FUNC_NONE		0


/* The Kernel Debug Classes  */
#define DBG_MACH		1
#define DBG_NETWORK		2	
#define DBG_FSYSTEM		3
#define DBG_BSD			4
#define DBG_IOKIT		5
#define DBG_DRIVERS		6
#define DBG_TRACE           	7
#define DBG_DLIL	        8
#define DBG_WORKQUEUE		9
#define DBG_CORESTORAGE		10
#define DBG_CG         		11
#define DBG_MISC		20
#define DBG_SECURITY		30
#define DBG_DYLD           	31
#define DBG_QT              	32
#define DBG_APPS            	33
#define DBG_LAUNCHD         	34
#define DBG_PERF                37
#define DBG_IMPORTANCE          38
#define DBG_BANK                40
#define DBG_XPC                 41
#define DBG_ATM                 42

#define DBG_MIG			255

/* **** The Kernel Debug Sub Classes for Mach (DBG_MACH) **** */
#define	DBG_MACH_EXCP_KTRAP_x86	0x02	/* Kernel Traps on x86 */
#define	DBG_MACH_EXCP_DFLT	0x03	/* Data Translation Fault */
#define	DBG_MACH_EXCP_IFLT	0x04	/* Inst Translation Fault */
#define	DBG_MACH_EXCP_INTR	0x05	/* Interrupts */
#define	DBG_MACH_EXCP_ALNG	0x06	/* Alignment Exception */
#define	DBG_MACH_EXCP_UTRAP_x86	0x07	/* User Traps on x86 */
#define	DBG_MACH_EXCP_FP	0x08	/* FP Unavail */
#define	DBG_MACH_EXCP_DECI	0x09	/* Decrementer Interrupt */
#define	DBG_MACH_CHUD		0x0A	/* CHUD */
#define	DBG_MACH_EXCP_SC	0x0C	/* System Calls */
#define	DBG_MACH_EXCP_TRACE	0x0D	/* Trace exception */
#define	DBG_MACH_EXCP_EMUL	0x0E	/* Instruction emulated */
#define	DBG_MACH_IHDLR		0x10	/* Interrupt Handlers */
#define	DBG_MACH_IPC		0x20	/* Inter Process Comm */
#define	DBG_MACH_VM		0x30	/* Virtual Memory */
#define	DBG_MACH_LEAKS		0x31    /* alloc/free */
#define	DBG_MACH_SCHED		0x40	/* Scheduler */
#define	DBG_MACH_MSGID_INVALID	0x50	/* Messages - invalid */
#define DBG_MACH_LOCKS		0x60	/* new lock APIs */
#define DBG_MACH_PMAP		0x70	/* pmap */
/* #define unused		0x80    */
#define DBG_MACH_MP		0x90	/* MP related */
#define DBG_MACH_VM_PRESSURE	0xA0	/* Memory Pressure Events */
#define DBG_MACH_STACKSHOT		0xA1	/* Stackshot/Microstackshot subsystem */
#define DBG_MACH_SFI			0xA2	/* Selective Forced Idle (SFI) */
#define DBG_MACH_ENERGY_PERF	0xA3 /* Energy/performance resource stats */

/* Codes for Scheduler (DBG_MACH_SCHED) */     
#define MACH_SCHED              0x0     /* Scheduler */
#define MACH_STACK_ATTACH       0x1     /* stack_attach() */
#define MACH_STACK_HANDOFF      0x2     /* stack_handoff() */
#define MACH_CALL_CONT          0x3     /* call_continuation() */
#define MACH_CALLOUT            0x4     /* callouts */
#define MACH_STACK_DETACH       0x5
#define MACH_MAKE_RUNNABLE      0x6     /* make thread runnable */
#define	MACH_PROMOTE            0x7	/* promoted due to resource */
#define	MACH_DEMOTE             0x8	/* promotion undone */
#define MACH_IDLE               0x9	/* processor idling */
#define MACH_STACK_DEPTH        0xa	/* stack depth at switch */
#define MACH_MOVED              0xb	/* did not use original scheduling decision */
#define MACH_FAIRSHARE_ENTER    0xc	/* move to fairshare band */
#define MACH_FAIRSHARE_EXIT     0xd	/* exit fairshare band */
#define MACH_FAILSAFE           0xe	/* tripped fixed-pri/RT failsafe */
#define MACH_BLOCK              0xf	/* thread block */
#define MACH_WAIT		0x10	/* thread wait assertion */
#define	MACH_GET_URGENCY	0x14	/* Urgency queried by platform */
#define	MACH_URGENCY		0x15	/* Urgency (RT/BG/NORMAL) communicated
   					 * to platform
					 */
#define	MACH_REDISPATCH		0x16	/* "next thread" thread redispatched */
#define	MACH_REMOTE_AST		0x17	/* AST signal issued to remote processor */

#define	MACH_SCHED_CHOOSE_PROCESSOR	0x18	/* Result of choose_processor */
#define MACH_DEEP_IDLE          0x19	/* deep idle on master processor */
#define MACH_SCHED_DECAY_PRIORITY	0x1a	/* timeshare thread priority decayed/restored */
#define MACH_CPU_THROTTLE_DISABLE	0x1b	/* Global CPU Throttle Disable */
#define MACH_RW_PROMOTE            0x1c	/* promoted due to RW lock promotion */
#define MACH_RW_DEMOTE             0x1d	/* promotion due to RW lock undone */
#define MACH_SCHED_MAINTENANCE     0x1f /* periodic maintenance thread */
#define MACH_DISPATCH              0x20 /* context switch completed */
#define MACH_QUANTUM_HANDOFF       0x21 /* quantum handoff occurred */
#define MACH_MULTIQ_DEQUEUE        0x22 /* Result of multiq dequeue */
#define MACH_SCHED_THREAD_SWITCH   0x23 /* attempt direct context switch to hinted thread */
#define MACH_SCHED_SMT_BALANCE     0x24 /* SMT load balancing ASTs */

/* Variants for MACH_MULTIQ_DEQUEUE */
#define MACH_MULTIQ_BOUND     1
#define MACH_MULTIQ_GROUP     2
#define MACH_MULTIQ_GLOBAL    3

/* Codes for IPC (DBG_MACH_IPC) */
#define	MACH_TASK_SUSPEND			0x0	/* Suspended a task */
#define	MACH_TASK_RESUME 		  	0x1	/* Resumed a task */
#define MACH_THREAD_SET_VOUCHER 		0x2
#define MACH_IPC_MSG_SEND			0x3	/* mach msg send, uniq msg info */
#define MACH_IPC_MSG_RECV			0x4	/* mach_msg receive */
#define MACH_IPC_MSG_RECV_VOUCHER_REFUSED	0x5	/* mach_msg receive, voucher refused */
#define MACH_IPC_KMSG_FREE			0x6	/* kernel free of kmsg data */
#define MACH_IPC_VOUCHER_CREATE			0x7	/* Voucher added to global voucher hashtable */
#define MACH_IPC_VOUCHER_CREATE_ATTR_DATA	0x8	/* Attr data for newly created voucher */
#define MACH_IPC_VOUCHER_DESTROY		0x9	/* Voucher removed from global voucher hashtable */

/* Codes for pmap (DBG_MACH_PMAP) */     
#define PMAP__CREATE		0x0
#define PMAP__DESTROY		0x1
#define PMAP__PROTECT		0x2
#define PMAP__PAGE_PROTECT	0x3
#define PMAP__ENTER		0x4
#define PMAP__REMOVE		0x5
#define PMAP__NEST		0x6
#define PMAP__UNNEST		0x7
#define PMAP__FLUSH_TLBS	0x8
#define PMAP__UPDATE_INTERRUPT	0x9
#define PMAP__ATTRIBUTE_CLEAR	0xa
#define PMAP__REUSABLE		0xb
#define PMAP__QUERY_RESIDENT	0xc
#define PMAP__FLUSH_KERN_TLBS	0xd
#define PMAP__FLUSH_DELAYED_TLBS	0xe
#define PMAP__FLUSH_TLBS_TO	0xf

/* Codes for Stackshot/Microstackshot (DBG_MACH_STACKSHOT) */
#define MICROSTACKSHOT_RECORD	0x0
#define MICROSTACKSHOT_GATHER	0x1

/* Codes for Selective Forced Idle (DBG_MACH_SFI) */
#define SFI_SET_WINDOW			0x0
#define SFI_CANCEL_WINDOW		0x1
#define SFI_SET_CLASS_OFFTIME	0x2
#define SFI_CANCEL_CLASS_OFFTIME	0x3
#define SFI_THREAD_DEFER		0x4
#define SFI_OFF_TIMER			0x5
#define SFI_ON_TIMER			0x6
#define SFI_WAIT_CANCELED		0x7
#define SFI_PID_SET_MANAGED		0x8
#define SFI_PID_CLEAR_MANAGED	0x9

/* **** The Kernel Debug Sub Classes for Network (DBG_NETWORK) **** */
#define DBG_NETIP	1	/* Internet Protocol */
#define DBG_NETARP	2	/* Address Resolution Protocol */
#define	DBG_NETUDP	3	/* User Datagram Protocol */
#define	DBG_NETTCP	4	/* Transmission Control Protocol */
#define	DBG_NETICMP	5	/* Internet Control Message Protocol */
#define	DBG_NETIGMP	6	/* Internet Group Management Protocol */
#define	DBG_NETRIP	7	/* Routing Information Protocol */
#define	DBG_NETOSPF	8	/* Open Shortest Path First */
#define	DBG_NETISIS	9	/* Intermediate System to Intermediate System */
#define	DBG_NETSNMP	10	/* Simple Network Management Protocol */
#define DBG_NETSOCK	11	/* Socket Layer */

/* For Apple talk */
#define	DBG_NETAARP	100	/* Apple ARP */
#define	DBG_NETDDP	101	/* Datagram Delivery Protocol */
#define	DBG_NETNBP	102	/* Name Binding Protocol */
#define	DBG_NETZIP	103	/* Zone Information Protocol */
#define	DBG_NETADSP	104	/* Name Binding Protocol */
#define	DBG_NETATP	105	/* Apple Transaction Protocol */
#define	DBG_NETASP	106	/* Apple Session Protocol */
#define	DBG_NETAFP	107	/* Apple Filing Protocol */
#define	DBG_NETRTMP	108	/* Routing Table Maintenance Protocol */
#define	DBG_NETAURP	109	/* Apple Update Routing Protocol */

#define	DBG_NETIPSEC	128	/* IPsec Protocol  */
#define	DBG_NETVMNET	129	/* VMNet */

/* **** The Kernel Debug Sub Classes for IOKIT (DBG_IOKIT) **** */
#define DBG_IOINTC			0	/* Interrupt controller */
#define DBG_IOWORKLOOP		1	/* Work from work loop */
#define DBG_IOINTES			2	/* Interrupt event source */
#define DBG_IOCLKES			3	/* Clock event source */
#define DBG_IOCMDQ			4	/* Command queue latencies */
#define DBG_IOMCURS			5	/* Memory Cursor */
#define DBG_IOMDESC			6	/* Memory Descriptors */
#define DBG_IOPOWER			7	/* Power Managerment */
#define DBG_IOSERVICE		8	/* Matching etc. */

/* **** 9-32 reserved for internal IOKit usage **** */

#define DBG_IOSTORAGE		32	/* Storage layers */
#define	DBG_IONETWORK		33	/* Network layers */
#define	DBG_IOKEYBOARD		34	/* Keyboard */
#define	DBG_IOHID		35	/* HID Devices */
#define	DBG_IOAUDIO		36	/* Audio */
#define	DBG_IOSERIAL		37	/* Serial */
#define	DBG_IOTTY		38	/* TTY layers */
#define DBG_IOSAM		39	/* SCSI Architecture Model layers */
#define DBG_IOPARALLELATA   	40	/* Parallel ATA */
#define DBG_IOPARALLELSCSI	41	/* Parallel SCSI */
#define DBG_IOSATA		42	/* Serial-ATA */
#define DBG_IOSAS		43	/* SAS */
#define DBG_IOFIBRECHANNEL	44	/* FiberChannel */
#define DBG_IOUSB		45	/* USB */
#define DBG_IOBLUETOOTH		46	/* Bluetooth */
#define DBG_IOFIREWIRE		47	/* FireWire */
#define DBG_IOINFINIBAND	48	/* Infiniband */
#define DBG_IOCPUPM		49	/* CPU Power Management */
#define DBG_IOGRAPHICS		50	/* Graphics */
#define DBG_HIBERNATE		51	/* hibernation related events */
#define DBG_IOTHUNDERBOLT	52	/* Thunderbolt */


/* Backwards compatibility */
#define	DBG_IOPOINTING		DBG_IOHID			/* OBSOLETE: Use DBG_IOHID instead */
#define DBG_IODISK			DBG_IOSTORAGE		/* OBSOLETE: Use DBG_IOSTORAGE instead */

/* **** The Kernel Debug Sub Classes for Device Drivers (DBG_DRIVERS) **** */
#define DBG_DRVSTORAGE		1	/* Storage layers */
#define	DBG_DRVNETWORK		2	/* Network layers */
#define	DBG_DRVKEYBOARD		3	/* Keyboard */
#define	DBG_DRVHID			4	/* HID Devices */
#define	DBG_DRVAUDIO		5	/* Audio */
#define	DBG_DRVSERIAL		7	/* Serial */
#define DBG_DRVSAM			8	/* SCSI Architecture Model layers */
#define DBG_DRVPARALLELATA  9	/* Parallel ATA */
#define DBG_DRVPARALLELSCSI	10	/* Parallel SCSI */
#define DBG_DRVSATA			11	/* Serial ATA */
#define DBG_DRVSAS			12	/* SAS */
#define DBG_DRVFIBRECHANNEL	13	/* FiberChannel */
#define DBG_DRVUSB			14	/* USB */
#define DBG_DRVBLUETOOTH	15	/* Bluetooth */
#define DBG_DRVFIREWIRE		16	/* FireWire */
#define DBG_DRVINFINIBAND	17	/* Infiniband */
#define DBG_DRVGRAPHICS		18  /* Graphics */
#define DBG_DRVSD		19 	/* Secure Digital */
#define DBG_DRVNAND		20	/* NAND drivers and layers */
#define DBG_SSD			21	/* SSD */
#define DBG_DRVSPI		22	/* SPI */

/* Backwards compatibility */
#define	DBG_DRVPOINTING		DBG_DRVHID		/* OBSOLETE: Use DBG_DRVHID instead */
#define DBG_DRVDISK			DBG_DRVSTORAGE	/* OBSOLETE: Use DBG_DRVSTORAGE instead */

/* **** The Kernel Debug Sub Classes for the DLIL Layer (DBG_DLIL) **** */
#define DBG_DLIL_STATIC 1       /* Static DLIL code */
#define DBG_DLIL_PR_MOD 2       /* DLIL Protocol Module */
#define DBG_DLIL_IF_MOD 3       /* DLIL Interface Module */
#define DBG_DLIL_PR_FLT 4       /* DLIL Protocol Filter */
#define DBG_DLIL_IF_FLT 5       /* DLIL Interface FIlter */

/* The Kernel Debug Sub Classes for File System (DBG_FSYSTEM) */
#define DBG_FSRW      1       /* reads and writes to the filesystem */
#define DBG_DKRW      2       /* reads and writes to the disk */
#define DBG_FSVN      3       /* vnode operations (inc. locking/unlocking) */
#define DBG_FSLOOOKUP 4       /* namei and other lookup-related operations */
#define DBG_JOURNAL   5       /* journaling operations */
#define DBG_IOCTL     6       /* ioctl to the disk */
#define DBG_BOOTCACHE 7       /* bootcache operations */
#define DBG_HFS       8       /* HFS-specific events; see bsd/hfs/hfs_kdebug.h */
#define DBG_EXFAT     0xE     /* ExFAT-specific events; see the exfat project */
#define DBG_MSDOS     0xF     /* FAT-specific events; see the msdosfs project */
#define DBG_ACFS      0x10    /* Xsan-specific events; see the XsanFS project */
#define DBG_THROTTLE  0x11    /* I/O Throttling events */	
#define DBG_CONTENT_PROT 0xCF /* Content Protection Events: see bsd/sys/cprotect.h */

/* The Kernel Debug Sub Classes for BSD */
#define DBG_BSD_PROC		0x01	/* process/signals related */
#define DBG_BSD_MEMSTAT		0x02	/* memorystatus / jetsam operations */
#define	DBG_BSD_EXCP_SC		0x0C	/* System Calls */
#define	DBG_BSD_AIO		0x0D	/* aio (POSIX async IO) */
#define DBG_BSD_SC_EXTENDED_INFO 0x0E	/* System Calls, extended info */
#define DBG_BSD_SC_EXTENDED_INFO2 0x0F	/* System Calls, extended info */


/* The Codes for BSD subcode class DBG_BSD_PROC */
#define BSD_PROC_EXIT		1	/* process exit */
#define BSD_PROC_FRCEXIT 	2	/* Kernel force termination */

/* Codes for BSD subcode class DBG_BSD_MEMSTAT */
#define BSD_MEMSTAT_SCAN             1  /* memorystatus thread awake */
#define BSD_MEMSTAT_JETSAM           2  /* LRU jetsam */
#define BSD_MEMSTAT_JETSAM_HIWAT     3  /* highwater jetsam */
#define BSD_MEMSTAT_FREEZE           4  /* freeze process */
#define BSD_MEMSTAT_LATENCY_COALESCE 5  /* delay imposed to coalesce jetsam reports */
#define BSD_MEMSTAT_UPDATE           6  /* priority update */	
#define BSD_MEMSTAT_IDLE_DEMOTE      7  /* idle demotion fired */
#define BSD_MEMSTAT_CLEAR_ERRORS     8  /* reset termination error state */
#define BSD_MEMSTAT_DIRTY_TRACK      9  /* track the process state */
#define BSD_MEMSTAT_DIRTY_SET       10  /* set the process state */
#define BSD_MEMSTAT_DIRTY_CLEAR     11  /* clear the process state */

/* The Kernel Debug Sub Classes for DBG_TRACE */
#define DBG_TRACE_DATA      0
#define DBG_TRACE_STRING    1
#define	DBG_TRACE_INFO	    2

/*
 * TRACE_DATA_NEWTHREAD			0x1
 * TRACE_DATA_EXEC			0x2
 */
#define TRACE_DATA_THREAD_TERMINATE	0x3	/* thread has been queued for deallocation and can no longer run */

/* The Kernel Debug Sub Classes for DBG_CORESTORAGE */
#define DBG_CS_IO	0

/* The Kernel Debug Sub Classes for DBG_SECURITY */
#define	DBG_SEC_KERNEL	0	/* raw entropy collected by the kernel */

/* Sub-class codes for CoreGraphics (DBG_CG) are defined in its component. */

/* The Kernel Debug Sub Classes for DBG_MISC */
#define DBG_EVENT	0x10
#define	DBG_BUFFER	0x20

/* The Kernel Debug Sub Classes for DBG_DYLD */
#define DBG_DYLD_STRING   5

/* The Kernel Debug modifiers for the DBG_DKRW sub class */
#define DKIO_DONE 	0x01
#define DKIO_READ	0x02
#define DKIO_ASYNC	0x04
#define DKIO_META	0x08
#define DKIO_PAGING	0x10
#define DKIO_THROTTLE	0x20 /* Deprecated, still provided so fs_usage doesn't break */
#define DKIO_PASSIVE	0x40
#define DKIO_NOCACHE	0x80
#define DKIO_TIER_MASK	0xF00
#define DKIO_TIER_SHIFT	8

/* Kernel Debug Sub Classes for Applications (DBG_APPS) */
#define DBG_APP_LOGINWINDOW     0x03
#define DBG_APP_AUDIO           0x04
#define DBG_APP_SAMBA           0x80

/* Kernel Debug codes for Throttling (DBG_THROTTLE) */
#define OPEN_THROTTLE_WINDOW	0x1
#define PROCESS_THROTTLED	0x2	
#define IO_THROTTLE_DISABLE	0x3


/* Subclasses for MACH Importance Policies (DBG_IMPORTANCE) */
/* TODO: Split up boost and task policy? */
#define IMP_ASSERTION           0x10    /* Task takes/drops a boost assertion */
#define IMP_BOOST               0x11    /* Task boost level changed */
#define IMP_MSG                 0x12    /* boosting message sent by donating task on donating port */
#define IMP_WATCHPORT           0x13    /* port marked as watchport, and boost was transferred to the watched task */
#define IMP_TASK_SUPPRESSION    0x17    /* Task changed suppression behaviors */
#define IMP_TASK_APPTYPE        0x18    /* Task launched with apptype */
#define IMP_UPDATE              0x19    /* Requested -> effective calculation */
#define IMP_USYNCH_QOS_OVERRIDE 0x1A    /* Userspace synchronization applied QoS override to resource owning thread */
#define IMP_DONOR_CHANGE        0x1B    /* The iit_donor bit changed */
#define IMP_MAIN_THREAD_QOS     0x1C    /* The task's main thread QoS was set */
/* DBG_IMPORTANCE subclasses  0x20 - 0x3F reserved for task policy flavors */

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
#define IMP_USYNCH_ADD_OVERRIDE		0x0		/* add override for a contended resource */
#define IMP_USYNCH_REMOVE_OVERRIDE	0x1		/* remove override for a contended resource */

/* Codes for IMP_DONOR_CHANGE */
#define IMP_DONOR_UPDATE_LIVE_DONOR_STATE	0x0
#define IMP_DONOR_INIT_DONOR_STATE		0x1

/* Subclasses for MACH Bank Voucher Attribute Manager (DBG_BANK) */
#define BANK_ACCOUNT_INFO		0x10	/* Trace points related to bank account struct */
#define BANK_TASK_INFO			0x11	/* Trace points related to bank task struct */

/* Subclasses for MACH ATM Voucher Attribute Manager (ATM) */
#define ATM_SUBAID_INFO			0x10	
#define ATM_GETVALUE_INFO		0x20	
#define ATM_UNREGISTER_INFO		0x30	

/* Codes for BANK_ACCOUNT_INFO */
#define BANK_SETTLE_CPU_TIME		0x1	/* Bank ledger(chit) rolled up to tasks. */

/* Codes for ATM_SUBAID_INFO */
#define ATM_MIN_CALLED				0x1
#define ATM_MIN_LINK_LIST			0x2

/* Codes for ATM_GETVALUE_INFO */
#define ATM_VALUE_REPLACED			0x1
#define ATM_VALUE_ADDED 			0x2

/* Codes for ATM_UNREGISTER_INFO */
#define ATM_VALUE_UNREGISTERED			0x1
#define ATM_VALUE_DIFF_MAILBOX			0x2

/**********************************************************************/

#define KDBG_CODE(Class, SubClass, code) (((Class & 0xff) << 24) | ((SubClass & 0xff) << 16) | ((code & 0x3fff)  << 2))

#define KDBG_MIGCODE(msgid) ((DBG_MIG << 24) | (((msgid) & 0x3fffff)  << 2))

#define MACHDBG_CODE(SubClass, code) KDBG_CODE(DBG_MACH, SubClass, code)
#define NETDBG_CODE(SubClass, code) KDBG_CODE(DBG_NETWORK, SubClass, code)
#define FSDBG_CODE(SubClass, code) KDBG_CODE(DBG_FSYSTEM, SubClass, code)
#define BSDDBG_CODE(SubClass, code) KDBG_CODE(DBG_BSD, SubClass, code)
#define IOKDBG_CODE(SubClass, code) KDBG_CODE(DBG_IOKIT, SubClass, code)
#define DRVDBG_CODE(SubClass, code) KDBG_CODE(DBG_DRIVERS, SubClass, code)
#define TRACEDBG_CODE(SubClass,code) KDBG_CODE(DBG_TRACE, SubClass, code)
#define MISCDBG_CODE(SubClass,code) KDBG_CODE(DBG_MISC, SubClass, code)
#define DLILDBG_CODE(SubClass,code) KDBG_CODE(DBG_DLIL, SubClass, code)
#define SECURITYDBG_CODE(SubClass,code) KDBG_CODE(DBG_SECURITY, SubClass, code)
#define DYLDDBG_CODE(SubClass,code) KDBG_CODE(DBG_DYLD, SubClass, code)
#define QTDBG_CODE(SubClass,code) KDBG_CODE(DBG_QT, SubClass, code)
#define APPSDBG_CODE(SubClass,code) KDBG_CODE(DBG_APPS, SubClass, code)
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

/*   Usage:
* kernel_debug((KDBG_CODE(DBG_NETWORK, DNET_PROTOCOL, 51) | DBG_FUNC_START), 
*	offset, 0, 0, 0,0) 
* 
* For ex, 
* 
* #include <sys/kdebug.h>
* 
* #define DBG_NETIPINIT NETDBG_CODE(DBG_NETIP,1)
* 
* 
* void
* ip_init()
* {
*	register struct protosw *pr;
*	register int i;
* 	
*	KERNEL_DEBUG(DBG_NETIPINIT | DBG_FUNC_START, 0,0,0,0,0)
* 	--------
*	KERNEL_DEBUG(DBG_NETIPINIT, 0,0,0,0,0)
* 	--------
*	KERNEL_DEBUG(DBG_NETIPINIT | DBG_FUNC_END, 0,0,0,0,0)
* }
*

*/

extern unsigned int kdebug_enable;
#define KDEBUG_ENABLE_TRACE   0x1
#define KDEBUG_ENABLE_ENTROPY 0x2		/* Obsolescent */
#define KDEBUG_ENABLE_CHUD    0x4
#define KDEBUG_ENABLE_PPT     0x8

/*
 * Infer the supported kernel debug event level from config option.
 * Use (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) as a guard to protect
 * unaudited debug code. 
 */
#define KDEBUG_LEVEL_NONE     0
#define KDEBUG_LEVEL_IST      1
#define KDEBUG_LEVEL_STANDARD 2
#define KDEBUG_LEVEL_FULL     3

#if NO_KDEBUG
#define KDEBUG_LEVEL KDEBUG_LEVEL_NONE    
#elif IST_KDEBUG
#define KDEBUG_LEVEL KDEBUG_LEVEL_IST
#elif KDEBUG
#define KDEBUG_LEVEL KDEBUG_LEVEL_FULL
#else
#define KDEBUG_LEVEL KDEBUG_LEVEL_STANDARD
#endif

#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD)
#define KERNEL_DEBUG_CONSTANT(x,a,b,c,d,e)				\
do {									\
	if (kdebug_enable & ~KDEBUG_ENABLE_PPT)						\
        kernel_debug(x,(uintptr_t)a,(uintptr_t)b,(uintptr_t)c,		\
		       (uintptr_t)d,(uintptr_t)e);			\
} while(0)

#define KERNEL_DEBUG_CONSTANT1(x,a,b,c,d,e)				\
do {									\
	if (kdebug_enable & ~KDEBUG_ENABLE_PPT)						\
        kernel_debug1(x,(uintptr_t)a,(uintptr_t)b,(uintptr_t)c,		\
			(uintptr_t)d,(uintptr_t)e);			\
} while(0)

#define KERNEL_DEBUG_EARLY(x,a,b,c,d)					\
do {									\
        kernel_debug_early((uint32_t)x,  (uintptr_t)a, (uintptr_t)b,	\
		           (uintptr_t)c, (uintptr_t)d);			\
} while(0)
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) */
#define KERNEL_DEBUG_CONSTANT(x,a,b,c,d,e) do { } while(0)
#define KERNEL_DEBUG_CONSTANT1(x,a,b,c,d,e) do { } while(0)
#define KERNEL_DEBUG_EARLY(x,a,b,c,d) do { } while(0)
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) */

/* 
 * Specify KDEBUG_PPT to indicate that the event belongs to the
 * limited PPT set.
 */
#define KDEBUG_COMMON (KDEBUG_ENABLE_TRACE|KDEBUG_ENABLE_CHUD|KDEBUG_ENABLE_PPT)
#define KDEBUG_TRACE  (KDEBUG_ENABLE_TRACE|KDEBUG_ENABLE_CHUD)
#define KDEBUG_PPT    (KDEBUG_ENABLE_PPT)

/*
 * KERNEL_DEBUG_CONSTANT_IST events provide an audited subset of
 * tracepoints for userland system tracing tools.
 */
#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST)
#define KERNEL_DEBUG_CONSTANT_IST(type,x,a,b,c,d,e)				\
do {									\
	if (kdebug_enable & type)						\
        kernel_debug(x,(uintptr_t)a,(uintptr_t)b,(uintptr_t)c,		\
			(uintptr_t)d,(uintptr_t)e);			\
} while(0)
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST) */
#define KERNEL_DEBUG_CONSTANT_IST(type,x,a,b,c,d,e) do { } while(0)
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST) */

#if NO_KDEBUG
#define __kdebug_constant_only __unused
#endif

extern void kernel_debug(
		uint32_t  debugid,
		uintptr_t arg1,
		uintptr_t arg2,
		uintptr_t arg3,
		uintptr_t arg4,
		uintptr_t arg5);

extern void kernel_debug1(
		uint32_t  debugid,
		uintptr_t arg1,
		uintptr_t arg2,
		uintptr_t arg3,
		uintptr_t arg4,
		uintptr_t arg5);

extern void kernel_debug_early(
		uint32_t  debugid,
		uintptr_t arg1,
		uintptr_t arg2,
		uintptr_t arg3,
		uintptr_t arg4);

extern void kernel_debug_string(
		const char *message);

#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_FULL)
#define KERNEL_DEBUG(x,a,b,c,d,e)					\
do {									\
	if (kdebug_enable & ~KDEBUG_ENABLE_PPT)				\
        kernel_debug((uint32_t)x,  (uintptr_t)a, (uintptr_t)b,		\
		     (uintptr_t)c, (uintptr_t)d, (uintptr_t)e);		\
} while(0)

#define KERNEL_DEBUG1(x,a,b,c,d,e)					\
do {									\
	if (kdebug_enable & ~KDEBUG_ENABLE_PPT)				\
        kernel_debug1((uint32_t)x,  (uintptr_t)a, (uintptr_t)b,		\
		      (uintptr_t)c, (uintptr_t)d, (uintptr_t)e);	\
} while(0)
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_FULL) */
#define KERNEL_DEBUG(x,a,b,c,d,e) do {} while (0)
#define KERNEL_DEBUG1(x,a,b,c,d,e) do {} while (0)

#define __kdebug_only __unused
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_FULL) */



#endif /* __APPLE_API_UNSTABLE */
__END_DECLS



#endif /* !BSD_SYS_KDEBUG_H */

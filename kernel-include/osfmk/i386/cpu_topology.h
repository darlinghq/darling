/*
 * Copyright (c) 2003-2010 Apple Inc. All rights reserved.
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
#ifdef KERNEL_PRIVATE
#ifndef _I386_CPU_TOPOLOGY_H_
#define _I386_CPU_TOPOLOGY_H_

/*
 * This was originally part of cpu_threads.h.  It was split out so that
 * these structures could be referenced without pulling in all of the headers
 * required for the definition of cpu_data.  These data structures are
 * used by KEXTs in order to deal with the physical topology.
 *
 * NOTE: this header must stand on its own as much as possible
 * and not be dependent upon any unexported, kernel-private header.
 */

/*
 * Cache structure that can be used to identify the cache heirarchy.
 */
typedef struct x86_cpu_cache {
	struct x86_cpu_cache *next;     /* next cache at this level/lcpu */
	struct x86_die      *die;       /* die containing this cache (only for LLC) */
	uint8_t             maxcpus;    /* maximum # of cpus that can share */
	uint8_t             nlcpus;     /* # of logical cpus sharing this cache */
	uint8_t             type;       /* type of cache */
	uint8_t             level;      /* level of cache */
	uint16_t            ways;       /* # of ways in cache */
	uint16_t            partitions; /* # of partitions in cache */
	uint16_t            line_size;  /* size of a cache line */
	uint32_t            cache_size; /* total size of cache */
	struct x86_lcpu     *cpus[0];   /* cpus sharing this cache */
} x86_cpu_cache_t;

#define CPU_CACHE_TYPE_DATA     1       /* data cache */
#define CPU_CACHE_TYPE_INST     2       /* instruction cache */
#define CPU_CACHE_TYPE_UNIF     3       /* unified cache */

#define CPU_CACHE_DEPTH_L1      0
#define CPU_CACHE_DEPTH_L2      1
#define CPU_CACHE_DEPTH_L3      2

#define MAX_CACHE_DEPTH         3       /* deepest cache */

struct pmc;
struct cpu_data;
struct mca_state;

/*
 * Define the states that a (logical) CPU can be in.
 *
 * LCPU_OFF	This indicates that the CPU is "off".  It requires a full
 *		restart.  This is the state of a CPU when the system first
 *		boots or when it comes out of "sleep" (aka S3/S5).
 *
 * LCPU_HALT	This indicates that the CPU has been "halted".  It has been
 *		removed from the system but still retains its internal state
 *		so that it can be quickly brought back on-line.
 *
 * LCPU_NONSCHED	This indicates that the CPU is not schedulable.  It
 *		will still appear in the system as a viable CPU however no
 *		work will be sceduled on it.
 *
 * LCPU_PAUSE	This indicates that the CPU is "paused".  This is usually
 *		done only during kernel debug.
 *
 * LCPU_IDLE	This indicates that the CPU is idle.  The scheduler has
 *		determined that there is no work for this CPU to do.
 *
 * LCPU_RUN	This indicates that the CPU is running code and performing work.
 *
 * In normal system operation, CPUs will usually be transitioning between
 * LCPU_IDLE and LCPU_RUN.
 */
typedef enum lcpu_state {
	LCPU_OFF            = 0,/* 0 so the right thing happens on boot */
	LCPU_HALT           = 1,
	LCPU_NONSCHED       = 2,
	LCPU_PAUSE          = 3,
	LCPU_IDLE           = 4,
	LCPU_RUN            = 5,
} lcpu_state_t;

/*
 * In each topology structure there are two numbers: a logical number and a
 * physical number.
 *
 * The logical numbers represent the ID of that structure
 * relative to the enclosing structure and always starts at 0.  So when using
 * logical numbers, it is necessary to specify all elements in the topology
 * (ie to "name" a logical CPU using logical numbers, 4 numbers are required:
 * package, die, core, logical CPU).
 *
 * The physical numbers represent the ID of that structure and is unique (for
 * that structure) across the entire topology.
 *
 * The logical CPU structure contains a third number which is the CPU number.
 * This number is identical to the CPU number used in other parts of the kernel.
 */
typedef struct x86_lcpu {
	struct x86_lcpu     *next_in_core;/* next logical cpu in core */
	struct x86_lcpu     *next_in_die;/* next logical cpu in die */
	struct x86_lcpu     *next_in_pkg;/* next logical cpu in package */
	struct x86_lcpu     *lcpu;      /* pointer back to self */
	struct x86_core     *core;      /* core containing the logical cpu */
	struct x86_die      *die;       /* die containing the logical cpu */
	struct x86_pkg      *package;   /* package containing the logical cpu */
	struct cpu_data     *cpu;       /* cpu_data structure */
	uint32_t            flags;
	uint32_t            cpu_num;    /* cpu number */
	uint32_t            lnum;       /* logical cpu number (within core) */
	uint32_t            pnum;       /* physical cpu number */
	boolean_t           master;     /* logical cpu is the master (boot) CPU */
	boolean_t           primary;    /* logical cpu is primary CPU in package */
	volatile lcpu_state_t       state;/* state of the logical CPU */
	volatile boolean_t  stopped;    /* used to indicate that the CPU has "stopped" */
	uint64_t            rtcPop;     /* next timer pop programmed */
	uint64_t            rtcDeadline;/* next etimer-requested deadline */
	x86_cpu_cache_t     *caches[MAX_CACHE_DEPTH];
	void                *pmStats;   /* Power management stats for lcpu */
	void                *pmState;   /* Power management state for lcpu */
} x86_lcpu_t;

#define X86CORE_FL_PRESENT      0x80000000      /* core is present */
#define X86CORE_FL_READY        0x40000000      /* core struct is init'd */
#define X86CORE_FL_HAS_HPET     0x10000000      /* core has HPET assigned */
#define X86CORE_FL_HALTED       0x00008000      /* core is halted */
#define X86CORE_FL_IDLE         0x00004000      /* core is idle */

typedef struct x86_core {
	struct x86_core     *next_in_die;/* next core in die */
	struct x86_core     *next_in_pkg;/* next core in package */
	struct x86_die      *die;       /* die containing the core */
	struct x86_pkg      *package;   /* package containing core */
	struct x86_lcpu     *lcpus;     /* list of logical cpus in core */
	uint32_t            flags;
	uint32_t            lcore_num;  /* logical core # (unique within die) */
	uint32_t            pcore_num;  /* physical core # (globally unique) */
	uint32_t            num_lcpus;  /* Number of logical cpus */
	uint32_t            active_lcpus;/* Number of {running, idle} cpus */
	void                *pmStats;   /* Power management stats for core */
	void                *pmState;   /* Power management state for core */
} x86_core_t;

#define X86DIE_FL_PRESENT       0x80000000      /* die is present */
#define X86DIE_FL_READY         0x40000000      /* die struct is init'd */

typedef struct x86_die {
	struct x86_die      *next_in_pkg;/* next die in package */
	struct x86_lcpu     *lcpus;     /* list of lcpus in die */
	struct x86_core     *cores;     /* list of cores in die */
	struct x86_pkg      *package;   /* package containing the die */
	uint32_t            flags;
	uint32_t            ldie_num;   /* logical die # (unique to package) */
	uint32_t            pdie_num;   /* physical die # (globally unique) */
	uint32_t            num_cores;  /* Number of cores in die */
	x86_cpu_cache_t     *LLC;       /* LLC contained in this die */
	void                *pmStats;   /* Power Management stats for die */
	void                *pmState;   /* Power Management state for die */
} x86_die_t;

#define X86PKG_FL_PRESENT       0x80000000      /* package is present */
#define X86PKG_FL_READY         0x40000000      /* package struct init'd */
#define X86PKG_FL_HAS_HPET      0x10000000      /* package has HPET assigned */
#define X86PKG_FL_HALTED        0x00008000      /* package is halted */
#define X86PKG_FL_IDLE          0x00004000      /* package is idle */

typedef struct x86_pkg {
	struct x86_pkg      *next;      /* next package */
	struct x86_lcpu     *lcpus;     /* list of logical cpus in package */
	struct x86_core     *cores;     /* list of cores in package */
	struct x86_die      *dies;      /* list of dies in package */
	uint32_t            flags;
	uint32_t            lpkg_num;   /* logical package # */
	uint32_t            ppkg_num;   /* physical package # */
	uint32_t            num_dies;   /* number of dies in package */
	void                *pmStats;   /* Power Management stats for package*/
	void                *pmState;   /* Power Management state for package*/
	struct mca_state    *mca_state; /* MCA state for memory errors */
	uint64_t            package_idle_exits;
	uint32_t            num_idle;
} x86_pkg_t;

extern x86_pkg_t        *x86_pkgs;      /* root of all CPU packages */

typedef struct x86_topology_parameters {
	uint32_t            LLCDepth;
	uint32_t            nCoresSharingLLC;
	uint32_t            nLCPUsSharingLLC;
	uint32_t            maxSharingLLC;
	uint32_t            nLThreadsPerCore;
	uint32_t            nPThreadsPerCore;
	uint32_t            nLCoresPerDie;
	uint32_t            nPCoresPerDie;
	uint32_t            nLDiesPerPackage;
	uint32_t            nPDiesPerPackage;
	uint32_t            nLThreadsPerDie;
	uint32_t            nPThreadsPerDie;
	uint32_t            nLThreadsPerPackage;
	uint32_t            nPThreadsPerPackage;
	uint32_t            nLCoresPerPackage;
	uint32_t            nPCoresPerPackage;
	uint32_t            nPackages;
	boolean_t           stable;
} x86_topology_parameters_t;

/* Called after cpu discovery */
extern void             cpu_topology_sort(int ncpus);
extern kern_return_t    cpu_topology_start_cpu(int cpunum);


#endif /* _I386_CPU_TOPOLOGY_H_ */
#endif /* KERNEL_PRIVATE */

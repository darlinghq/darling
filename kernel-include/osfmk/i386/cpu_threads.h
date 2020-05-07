/*
 * Copyright (c) 2003-2008 Apple Inc. All rights reserved.
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
#ifndef _I386_CPU_THREADS_H_
#define _I386_CPU_THREADS_H_

#include <i386/cpu_data.h>
#include <i386/cpu_topology.h>
#include <i386/mp.h>

/*
 * These are defined here rather than in cpu_topology.h so as to keep
 * cpu_topology.h from having a dependency on cpu_data.h.
 */
#define CPU_THREAD_MASK                 0x00000001
#define cpu_to_core_lapic(cpu)          (cpu_to_lapic[cpu] & ~CPU_THREAD_MASK)
#define cpu_to_core_cpu(cpu)            (lapic_to_cpu[cpu_to_core_lapic(cpu)])
#define cpu_to_logical_cpu(cpu)         (cpu_to_lapic[cpu] & CPU_THREAD_MASK)
#define cpu_is_core_cpu(cpu)            (cpu_to_logical_cpu(cpu) == 0)

#define _cpu_to_lcpu(cpu)               (&cpu_datap(cpu)->lcpu)
#define _cpu_to_core(cpu)               (_cpu_to_lcpu(cpu)->core)
#define _cpu_to_package(cpu)            (_cpu_to_core(cpu)->package)

#define cpu_to_lcpu(cpu)                ((cpu_datap(cpu) != NULL) ? _cpu_to_lcpu(cpu) : NULL)
#define cpu_to_core(cpu)                ((cpu_to_lcpu(cpu) != NULL) ? _cpu_to_lcpu(cpu)->core : NULL)
#define cpu_to_die(cpu)                 ((cpu_to_lcpu(cpu) != NULL) ? _cpu_to_lcpu(cpu)->die : NULL)
#define cpu_to_package(cpu)             ((cpu_to_lcpu(cpu) != NULL) ? _cpu_to_lcpu(cpu)->package : NULL)

/* Fast access: */
#define x86_lcpu()                      (&current_cpu_datap()->lcpu)
#define x86_core()                      (x86_lcpu()->core)
#define x86_die()                       (x86_lcpu()->die)
#define x86_package()                   (x86_lcpu()->package)

#define cpu_is_same_core(cpu1, cpu2)     (cpu_to_core(cpu1) == cpu_to_core(cpu2))
#define cpu_is_same_die(cpu1, cpu2)      (cpu_to_die(cpu1) == cpu_to_die(cpu2))
#define cpu_is_same_package(cpu1, cpu2)  (cpu_to_package(cpu1) == cpu_to_package(cpu2))
#define cpus_share_cache(cpu1, cpu2, _cl) (cpu_to_lcpu(cpu1)->caches[_cl] == cpu_to_lcpu(cpu2)->caches[_cl])

/* always take the x86_topo_lock with mp_safe_spin_lock */
boolean_t       mp_safe_spin_lock(usimple_lock_t lock);
extern decl_simple_lock_data(, x86_topo_lock);

extern void *cpu_thread_alloc(int);
extern void cpu_thread_init(void);
extern void cpu_thread_halt(void);

extern void x86_set_logical_topology(x86_lcpu_t *lcpu, int pnum, int lnum);
extern void x86_validate_topology(void);

extern x86_topology_parameters_t        topoParms;

extern boolean_t        topo_dbg;
#define TOPO_DBG(x...)                  \
	do {                            \
	        if (topo_dbg)           \
	                kprintf(x);     \
	} while (0)                     \

#endif /* _I386_CPU_THREADS_H_ */

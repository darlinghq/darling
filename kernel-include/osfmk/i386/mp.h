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
/*
 * @OSF_COPYRIGHT@
 */
/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */

/*
 */
#ifdef  KERNEL_PRIVATE

#ifndef _I386_MP_H_
#define _I386_MP_H_

//#define	MP_DEBUG 1

#include <i386/apic.h>
#include <i386/mp_events.h>
#include <machine/limits.h>

#define MAX_CPUS        64              /* 8 * sizeof(cpumask_t) */

#ifndef ASSEMBLER
#include <stdint.h>
#include <sys/cdefs.h>
#include <mach/boolean.h>
#include <mach/kern_return.h>
#include <mach/i386/thread_status.h>
#include <mach/vm_types.h>
#include <kern/simple_lock.h>
#include <kern/assert.h>

__BEGIN_DECLS

extern kern_return_t intel_startCPU(int slot_num);
extern kern_return_t intel_startCPU_fast(int slot_num);
extern void i386_init_slave(void) __dead2;
extern void i386_init_slave_fast(void) __dead2;
extern void smp_init(void);

extern void cpu_interrupt(int cpu);
__END_DECLS

extern  unsigned int    real_ncpus;             /* real number of cpus */
extern  unsigned int    max_ncpus;              /* max number of cpus */
decl_simple_lock_data(extern, kdb_lock); /* kdb lock		*/

__BEGIN_DECLS

extern  void    console_init(void);
extern  void    *console_cpu_alloc(boolean_t boot_cpu);
extern  void    console_cpu_free(void *console_buf);

extern  int     kdb_cpu;                /* current cpu running kdb	*/
extern  int     kdb_debug;
extern  int     kdb_active[];

extern  volatile boolean_t mp_kdp_trap;
extern  volatile boolean_t mp_kdp_is_NMI;
extern  volatile boolean_t force_immediate_debugger_NMI;
extern  volatile boolean_t pmap_tlb_flush_timeout;
extern  volatile usimple_lock_t spinlock_timed_out;
extern  volatile uint32_t spinlock_owner_cpu;
extern  uint32_t spinlock_timeout_NMI(uintptr_t thread_addr);

extern  uint64_t        LastDebuggerEntryAllowance;

extern  void      mp_kdp_enter(boolean_t proceed_on_failure);
extern  void      mp_kdp_exit(void);
extern  boolean_t mp_kdp_all_cpus_halted(void);

extern  boolean_t       mp_recent_debugger_activity(void);
extern  void    kernel_spin(uint64_t spin_ns);

/*
 * All cpu rendezvous:
 */
extern void mp_rendezvous(
	void (*setup_func)(void *),
	void (*action_func)(void *),
	void (*teardown_func)(void *),
	void *arg);
extern void mp_rendezvous_no_intrs(
	void (*action_func)(void *),
	void *arg);
extern void mp_rendezvous_break_lock(void);
extern void mp_rendezvous_lock(void);
extern void mp_rendezvous_unlock(void);

/*
 * All cpu broadcast.
 * Called from thread context, this blocks until all active cpus have
 * run action_func:
 */
extern void mp_broadcast(
	void (*action_func)(void *),
	void *arg);
#if MACH_KDP
typedef long (*kdp_x86_xcpu_func_t) (void *arg0, void *arg1, uint16_t lcpu);

extern  long kdp_x86_xcpu_invoke(const uint16_t lcpu,
    kdp_x86_xcpu_func_t func,
    void *arg0, void *arg1);
typedef enum    {KDP_XCPU_NONE = 0xffff, KDP_CURRENT_LCPU = 0xfffe} kdp_cpu_t;
#endif

typedef uint32_t cpu_t;
typedef volatile uint64_t cpumask_t;

static_assert(sizeof(cpumask_t) * CHAR_BIT >= MAX_CPUS, "cpumask_t bitvector is too small for current MAX_CPUS value");

static inline cpumask_t
cpu_to_cpumask(cpu_t cpu)
{
	return (cpu < MAX_CPUS) ? (1ULL << cpu) : 0;
}
#define CPUMASK_ALL     0xffffffffffffffffULL
#define CPUMASK_SELF    cpu_to_cpumask(cpu_number())
#define CPUMASK_OTHERS  (CPUMASK_ALL & ~CPUMASK_SELF)

/* Initialation routing called at processor registration */
extern void mp_cpus_call_cpu_init(int cpu);

/*
 * Invoke a function (possibly NULL) on a set of cpus specified by a mask.
 * The mask may include the local cpu.
 * If the mode is:
 *	- ASYNC:  other cpus make their calls in parallel
 *      - SYNC:   the calls are performed serially in logical cpu order
 *      - NOSYNC: the calls are queued
 * Unless the mode is NOSYNC, mp_cpus_call() returns when the function has been
 * called on all specified cpus.
 * The return value is the number of cpus where the call was made or queued.
 * The action function is called with interrupts disabled.
 */
extern cpu_t mp_cpus_call(
	cpumask_t       cpus,
	mp_sync_t       mode,
	void            (*action_func)(void *),
	void            *arg);
extern cpu_t mp_cpus_call1(
	cpumask_t       cpus,
	mp_sync_t       mode,
	void            (*action_func)(void *, void*),
	void            *arg0,
	void            *arg1,
	cpumask_t       *cpus_calledp);

typedef enum {
	NONE = 0,
	SPINLOCK_TIMEOUT,
	TLB_FLUSH_TIMEOUT,
	CROSSCALL_TIMEOUT,
	INTERRUPT_WATCHDOG
} NMI_reason_t;
extern void NMIPI_panic(cpumask_t cpus, NMI_reason_t reason);

/* Interrupt a set of cpus, forcing an exit out of non-root mode */
extern void mp_cpus_kick(cpumask_t cpus);
/*
 * Power-management-specific SPI to:
 *  - register a callout function, and
 *  - request the callout (if registered) on a given cpu.
 */
extern void PM_interrupt_register(void (*fn)(void));
extern void cpu_PM_interrupt(int cpu);

__END_DECLS

#if MP_DEBUG
typedef struct {
	uint64_t        time;
	int             cpu;
	mp_event_t      event;
} cpu_signal_event_t;

#define LOG_NENTRIES    100
typedef struct {
	uint64_t                count[MP_LAST];
	int                     next_entry;
	cpu_signal_event_t      entry[LOG_NENTRIES];
} cpu_signal_event_log_t;

extern cpu_signal_event_log_t   *cpu_signal[];
extern cpu_signal_event_log_t   *cpu_handle[];

#define DBGLOG(log, _cpu, _event) {                                     \
	boolean_t		spl = ml_set_interrupts_enabled(FALSE); \
	cpu_signal_event_log_t	*logp = log[cpu_number()];              \
	int			next = logp->next_entry;                \
	cpu_signal_event_t	*eventp = &logp->entry[next];           \
                                                                        \
	logp->count[_event]++;                                          \
                                                                        \
	eventp->time = rdtsc64();                                       \
	eventp->cpu = _cpu;                                             \
	eventp->event = _event;                                         \
	if (next == (LOG_NENTRIES - 1))                                 \
	        logp->next_entry = 0;                                   \
	else                                                            \
	        logp->next_entry++;                                     \
                                                                        \
	(void) ml_set_interrupts_enabled(spl);                          \
}

#define DBGLOG_CPU_INIT(cpu)    {                                       \
	cpu_signal_event_log_t	**sig_logpp = &cpu_signal[cpu];         \
	cpu_signal_event_log_t	**hdl_logpp = &cpu_handle[cpu];         \
                                                                        \
	if (*sig_logpp == NULL &&                                       \
	        kmem_alloc(kernel_map,                                  \
	                (vm_offset_t *) sig_logpp,                      \
	                sizeof(cpu_signal_event_log_t)) != KERN_SUCCESS)\
	        panic("DBGLOG_CPU_INIT cpu_signal allocation failed\n");\
	bzero(*sig_logpp, sizeof(cpu_signal_event_log_t));              \
	if (*hdl_logpp == NULL &&                                       \
	        kmem_alloc(kernel_map,                                  \
	                (vm_offset_t *) hdl_logpp,                      \
	                sizeof(cpu_signal_event_log_t)) != KERN_SUCCESS)\
	        panic("DBGLOG_CPU_INIT cpu_handle allocation failed\n");\
	bzero(*hdl_logpp, sizeof(cpu_signal_event_log_t));              \
}
#else   /* MP_DEBUG */
#define DBGLOG(log, _cpu, _event)
#define DBGLOG_CPU_INIT(cpu)
#endif  /* MP_DEBUG */

#endif  /* ASSEMBLER */

#ifdef ASSEMBLER
#define i_bit(bit, word)        ((long)(*(word)) & (1L << (bit)))
#else
__attribute__((always_inline)) static inline long
i_bit_impl(long word, long bit)
{
	long bitmask = 1L << bit;
	return word & bitmask;
}
#define i_bit(bit, word)        i_bit_impl((long)(*(word)), bit)
#endif


#endif /* _I386_MP_H_ */

#endif /* KERNEL_PRIVATE */

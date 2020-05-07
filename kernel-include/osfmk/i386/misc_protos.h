/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
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

#ifndef _I386_MISC_PROTOS_H_
#define _I386_MISC_PROTOS_H_

#include <machine/thread.h>

struct boot_args;
struct cpu_data;

extern  boolean_t virtualized;

extern void             vstart(vm_offset_t);
extern void             i386_init(void);
extern void             x86_init_wrapper(uintptr_t, uintptr_t) __attribute__((noreturn));
extern void             i386_vm_init(
	uint64_t,
	boolean_t,
	struct boot_args *);

#if NCOPY_WINDOWS > 0
extern void             cpu_userwindow_init(int);
extern void             cpu_physwindow_init(int);
#endif

extern void             machine_startup(void);

extern void             get_root_device(void);
extern void             picinit(void);
extern void             interrupt_processor(
	int             cpu);
extern void             mp_probe_cpus(void);
extern void             panic_io_port_read(void);

extern void             remote_kdb(void);
extern void             clear_kdb_intr(void);
extern void             cpu_init(void);
extern void             fix_desc(
	void            * desc,
	int             num_desc);
extern void             fix_desc64(
	void            * desc,
	int             num_desc);
extern void             cnpollc(
	boolean_t       on);
extern void             form_pic_mask(void);
extern void             intnull(
	int             unit);
extern char *           i386_boot_info(
	char            *buf,
	vm_size_t       buf_len);

extern void             blkclr(
	const char       *from,
	int              nbytes);

extern void             memset_word(
	int              *dst,
	int              pattern,
	int              nwords);


/* Move arbitrarily-aligned data from one physical address to another */
extern void bcopy_phys(addr64_t from, addr64_t to, vm_size_t nbytes);

/* allow a function to get a quick virtual mapping of a physical page */
extern int apply_func_phys(addr64_t src64, vm_size_t bytes, int (*func)(void * buffer, vm_size_t bytes, void * arg), void * arg);

extern int ml_copy_phys(addr64_t, addr64_t, vm_size_t);

/* Flush all cachelines for a page. */
extern void cache_flush_page_phys(ppnum_t pa);

/* Flushing for incoherent I/O */
extern void dcache_incoherent_io_flush64(addr64_t pa, unsigned int count);
extern void dcache_incoherent_io_store64(addr64_t pa, unsigned int count);


extern processor_t      cpu_processor_alloc(boolean_t is_boot_cpu);
extern void             cpu_processor_free(processor_t proc);

extern void             sysclk_gettime_interrupts_disabled(
	mach_timespec_t *cur_time);

extern void rtc_nanotime_init_commpage(void);

extern void     rtc_sleep_wakeup(uint64_t base);

extern void     rtc_timer_start(void);

extern void     rtc_clock_napped(uint64_t, uint64_t);
extern void     rtc_clock_adjust(uint64_t);

extern void     pmap_lowmem_finalize(void);

thread_t Switch_context(thread_t, thread_continue_t, thread_t);

__not_tail_called thread_t
Shutdown_context(thread_t thread, void (*doshutdown)(processor_t), processor_t  processor);

#ifdef __x86_64__
uint64_t x86_64_pre_sleep(void);
void x86_64_post_sleep(uint64_t new_cr3);
#endif

boolean_t
debug_state_is_valid32(x86_debug_state32_t *ds);

boolean_t
debug_state_is_valid64(x86_debug_state64_t *ds);

void
copy_debug_state32(x86_debug_state32_t *src, x86_debug_state32_t *target, boolean_t all);

void
copy_debug_state64(x86_debug_state64_t *src, x86_debug_state64_t *target, boolean_t all);

extern void act_machine_switch_pcb(thread_t old, thread_t new);

extern void Idle_PTs_release(vm_offset_t start, vm_offset_t end);
extern ppnum_t released_PT_ppn;
extern uint32_t released_PT_cnt;

/* Fast-restart parameters */
#define FULL_SLAVE_INIT (NULL)
#define FAST_SLAVE_INIT ((void *)(uintptr_t)1)

void cpu_pmc_control(void *);

extern void pstate_trace(void);

extern void mp_interrupt_watchdog(void);

#endif /* _I386_MISC_PROTOS_H_ */

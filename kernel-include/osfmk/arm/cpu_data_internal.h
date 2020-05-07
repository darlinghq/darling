/*
 * Copyright (c) 2007 Apple Inc. All rights reserved.
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
 *
 */

#ifndef ARM_CPU_DATA_INTERNAL
#define ARM_CPU_DATA_INTERNAL

#include <mach_assert.h>
#include <kern/assert.h>
#include <kern/kern_types.h>
#include <kern/processor.h>
#include <pexpert/pexpert.h>
#include <arm/dbgwrap.h>
#include <arm/proc_reg.h>
#include <arm/thread.h>
#include <arm/pmap.h>

#if MONOTONIC
#include <machine/monotonic.h>
#endif /* MONOTONIC */

#define NSEC_PER_HZ     (NSEC_PER_SEC / 100)

typedef struct reset_handler_data {
	vm_offset_t     assist_reset_handler;           /* Assist handler phys address */
	vm_offset_t     cpu_data_entries;                       /* CpuDataEntries phys address */
#if !__arm64__
	vm_offset_t     boot_args;                                      /* BootArgs phys address */
#endif
} reset_handler_data_t;

extern  reset_handler_data_t    ResetHandlerData;

#if __ARM_SMP__
#ifdef CPU_COUNT
#define MAX_CPUS                        CPU_COUNT
#else
#define MAX_CPUS                        2
#endif
#else
#define MAX_CPUS                        1
#endif

/* Put the static check for cpumap_t here as it's defined in <kern/processor.h> */
static_assert(sizeof(cpumap_t) * CHAR_BIT >= MAX_CPUS, "cpumap_t bitvector is too small for current MAX_CPUS value");

#ifdef  __arm__
#define CPUWINDOWS_BASE_MASK            0xFFF00000UL
#else
#define CPUWINDOWS_BASE_MASK            0xFFFFFFFFFFF00000UL
#endif
#define CPUWINDOWS_BASE                 (VM_MAX_KERNEL_ADDRESS & CPUWINDOWS_BASE_MASK)
#define CPUWINDOWS_TOP                  (CPUWINDOWS_BASE + (MAX_CPUS * CPUWINDOWS_MAX * PAGE_SIZE))

typedef struct cpu_data_entry {
	void                                    *cpu_data_paddr;             /* Cpu data physical address */
	struct  cpu_data                *cpu_data_vaddr;             /* Cpu data virtual address */
#if __arm__
	uint32_t                                cpu_data_offset_8;
	uint32_t                                cpu_data_offset_12;
#elif __arm64__
#else
#error Check cpu_data_entry padding for this architecture
#endif
} cpu_data_entry_t;


typedef struct rtclock_timer {
	mpqueue_head_t                  queue;
	uint64_t                                deadline;
	uint32_t                                is_set:1,
	    has_expired:1,
	:0;
} rtclock_timer_t;

typedef struct {
	/*
	 * The wake variants of these counters are reset to 0 when the CPU wakes.
	 */
	uint64_t irq_ex_cnt;
	uint64_t irq_ex_cnt_wake;
	uint64_t ipi_cnt;
	uint64_t ipi_cnt_wake;
	uint64_t timer_cnt;
	uint64_t timer_cnt_wake;
#if MONOTONIC
	uint64_t pmi_cnt_wake;
#endif /* MONOTONIC */
	uint64_t undef_ex_cnt;
	uint64_t unaligned_cnt;
	uint64_t vfp_cnt;
	uint64_t data_ex_cnt;
	uint64_t instr_ex_cnt;
} cpu_stat_t;

typedef struct cpu_data {
	unsigned short                          cpu_number;
	unsigned short                          cpu_flags;
	vm_offset_t                             istackptr;
	vm_offset_t                             intstack_top;
#if __arm64__
	vm_offset_t                             excepstackptr;
	vm_offset_t                             excepstack_top;
	boolean_t                               cluster_master;
#else
	vm_offset_t                             fiqstackptr;
	vm_offset_t                             fiqstack_top;
#endif
	boolean_t                               interrupts_enabled;
	thread_t                                cpu_active_thread;
	vm_offset_t                             cpu_active_stack;
	unsigned int                            cpu_ident;
	cpu_id_t                                cpu_id;
	unsigned volatile int                   cpu_signal;
	void                                    *cpu_cache_dispatch;
	ast_t                                   cpu_pending_ast;
	struct processor                        *cpu_processor;
	int                                     cpu_type;
	int                                     cpu_subtype;
	int                                     cpu_threadtype;
	int                                     cpu_running;

#ifdef __LP64__
	uint64_t                                cpu_base_timebase;
	uint64_t                                cpu_timebase;
#else
	union {
		struct {
			uint32_t        low;
			uint32_t        high;
		} split;
		struct {
			uint64_t        val;
		} raw;
	} cbtb;
#define cpu_base_timebase_low cbtb.split.low
#define cpu_base_timebase_high cbtb.split.high

	union {
		struct {
			uint32_t        low;
			uint32_t        high;
		} split;
		struct {
			uint64_t        val;
		} raw;
	} ctb;
#define cpu_timebase_low ctb.split.low
#define cpu_timebase_high ctb.split.high
#endif

	uint32_t                                cpu_decrementer;
	void                                    *cpu_get_decrementer_func;
	void                                    *cpu_set_decrementer_func;
	void                                    *cpu_get_fiq_handler;

	void                                    *cpu_tbd_hardware_addr;
	void                                    *cpu_tbd_hardware_val;

	void                                    *cpu_console_buf;

	void                                    *cpu_idle_notify;
	uint64_t                                cpu_idle_latency;
	uint64_t                                cpu_idle_pop;

#if     __arm__ || __ARM_KERNEL_PROTECT__
	vm_offset_t                             cpu_exc_vectors;
#endif /* __ARM_KERNEL_PROTECT__ */
	vm_offset_t                             cpu_reset_handler;
	uint32_t                                cpu_reset_type;
	uintptr_t                               cpu_reset_assist;

	void                                    *cpu_int_state;
	IOInterruptHandler                      interrupt_handler;
	void                                    *interrupt_nub;
	unsigned int                            interrupt_source;
	void                                    *interrupt_target;
	void                                    *interrupt_refCon;

	void                                    *idle_timer_notify;
	void                                    *idle_timer_refcon;
	uint64_t                                idle_timer_deadline;

	uint64_t                                quantum_timer_deadline;
	uint64_t                                rtcPop;
	rtclock_timer_t                         rtclock_timer;
	struct _rtclock_data_                   *rtclock_datap;

	arm_debug_state_t                       *cpu_user_debug;                                /* Current debug state */
	vm_offset_t                             cpu_debug_interface_map;

	volatile int                            debugger_active;

	void                                    *cpu_xcall_p0;
	void                                    *cpu_xcall_p1;
	void                                    *cpu_imm_xcall_p0;
	void                                    *cpu_imm_xcall_p1;

#if     __ARM_SMP__ && defined(ARMA7)
	volatile uint32_t                       cpu_CLW_active;
	volatile uint64_t                       cpu_CLWFlush_req;
	volatile uint64_t                       cpu_CLWFlush_last;
	volatile uint64_t                       cpu_CLWClean_req;
	volatile uint64_t                       cpu_CLWClean_last;
#endif


#if     __arm64__
	vm_offset_t                             coresight_base[CORESIGHT_REGIONS];
#endif

	/* CCC ARMv8 registers */
	uint64_t                                cpu_regmap_paddr;

	uint32_t                                cpu_phys_id;
	uint32_t                                cpu_l2_access_penalty;
	void                                    *platform_error_handler;

	int                                     cpu_mcount_off;

	#define ARM_CPU_ON_SLEEP_PATH           0x50535553UL
	volatile unsigned int                   cpu_sleep_token;
	unsigned int                            cpu_sleep_token_last;

	cpu_stat_t                              cpu_stat;

	volatile int                            PAB_active; /* Tells the console if we are dumping backtraces */

#if KPC
	/* double-buffered performance counter data */
	uint64_t                                *cpu_kpc_buf[2];
	/* PMC shadow and reload value buffers */
	uint64_t                                *cpu_kpc_shadow;
	uint64_t                                *cpu_kpc_reload;
#endif
#if MONOTONIC
	struct mt_cpu                           cpu_monotonic;
#endif /* MONOTONIC */
	cluster_type_t                          cpu_cluster_type;
	uint32_t                                cpu_cluster_id;
	uint32_t                                cpu_l2_id;
	uint32_t                                cpu_l2_size;
	uint32_t                                cpu_l3_id;
	uint32_t                                cpu_l3_size;

	struct pmap_cpu_data                    cpu_pmap_cpu_data;
	dbgwrap_thread_state_t                  halt_state;
	enum {
		CPU_NOT_HALTED = 0,
		CPU_HALTED,
		CPU_HALTED_WITH_STATE
	}                                       halt_status;
#if defined(HAS_APPLE_PAC)
	uint64_t        rop_key;
#endif /* defined(HAS_APPLE_PAC) */
} cpu_data_t;

/*
 * cpu_flags
 */
#define SleepState              0x0800
#define StartedState    0x1000

extern  cpu_data_entry_t        CpuDataEntries[MAX_CPUS];
extern  cpu_data_t                      BootCpuData;
extern  boot_args                       *BootArgs;

#if __arm__
extern  unsigned int            *ExceptionLowVectorsBase;
extern  unsigned int            *ExceptionVectorsTable;
#elif __arm64__
extern  unsigned int            LowResetVectorBase;
extern  unsigned int            LowResetVectorEnd;
#if WITH_CLASSIC_S2R
extern  uint8_t                 SleepToken[8];
#endif
extern  unsigned int            LowExceptionVectorBase;
#else
#error Unknown arch
#endif

extern cpu_data_t                       *cpu_datap(int cpu);
extern cpu_data_t                       *cpu_data_alloc(boolean_t is_boot);
extern void                             cpu_stack_alloc(cpu_data_t*);
extern void                             cpu_data_init(cpu_data_t *cpu_data_ptr);
extern void                             cpu_data_free(cpu_data_t *cpu_data_ptr);
extern kern_return_t                    cpu_data_register(cpu_data_t *cpu_data_ptr);
extern cpu_data_t                       *processor_to_cpu_datap( processor_t processor);

#if __arm64__
typedef struct sysreg_restore {
	uint64_t                tcr_el1;
} sysreg_restore_t;

extern sysreg_restore_t sysreg_restore;
#endif  /* __arm64__ */

#endif  /* ARM_CPU_DATA_INTERNAL */

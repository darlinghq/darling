/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef KERN_KPC_H
#define KERN_KPC_H

/* Kernel interfaces to KPC PMC infrastructure. */

#include <machine/machine_kpc.h>
#include <kern/thread.h> /* thread_* */

__BEGIN_DECLS

/* cross-platform class constants */
#define KPC_CLASS_FIXED         (0)
#define KPC_CLASS_CONFIGURABLE  (1)
#define KPC_CLASS_POWER         (2)
#define KPC_CLASS_RAWPMU        (3)

#define KPC_CLASS_FIXED_MASK         (1u << KPC_CLASS_FIXED)
#define KPC_CLASS_CONFIGURABLE_MASK  (1u << KPC_CLASS_CONFIGURABLE)
#define KPC_CLASS_POWER_MASK         (1u << KPC_CLASS_POWER)
#define KPC_CLASS_RAWPMU_MASK        (1u << KPC_CLASS_RAWPMU)

#define KPC_PMU_ERROR     (0)
#define KPC_PMU_INTEL_V3  (1)
#define KPC_PMU_ARM_APPLE (2)
#define KPC_PMU_INTEL_V2  (3)
#define KPC_PMU_ARM_V2    (4)

#define KPC_ALL_CPUS (1u << 31)

/* action id setters/getters */
#define FIXED_ACTIONID(ctr)                     (kpc_actionid[(ctr)])
#define CONFIGURABLE_ACTIONID(ctr)              (kpc_actionid[(ctr) + kpc_fixed_count()])

/* reload counter setters/getters */
#define FIXED_RELOAD(ctr)                       (current_cpu_datap()->cpu_kpc_reload[(ctr)])
#define FIXED_RELOAD_CPU(cpu, ctr)              (cpu_datap(cpu)->cpu_kpc_reload[(ctr)])
#define CONFIGURABLE_RELOAD(ctr)                (current_cpu_datap()->cpu_kpc_reload[(ctr) + kpc_fixed_count()])
#define CONFIGURABLE_RELOAD_CPU(cpu, ctr)       (cpu_datap(cpu)->cpu_kpc_reload[(ctr) + kpc_fixed_count()])

/* shadow counter setters/getters */
#define FIXED_SHADOW(ctr)                       (current_cpu_datap()->cpu_kpc_shadow[(ctr)])
#define FIXED_SHADOW_CPU(cpu, ctr)              (cpu_datap(cpu)->cpu_kpc_shadow[(ctr)])
#define CONFIGURABLE_SHADOW(ctr)                (current_cpu_datap()->cpu_kpc_shadow[(ctr) + kpc_fixed_count()])
#define CONFIGURABLE_SHADOW_CPU(cpu, ctr)       (cpu_datap(cpu)->cpu_kpc_shadow[(ctr) + kpc_fixed_count()])

/**
 * Callback for notification when PMCs are acquired/released by a task. The
 * argument is equal to TRUE if the Power Manager (PM) can use its reserved PMCs.
 * Otherwise, the argument is equal to FALSE.
 */
typedef void (*kpc_pm_handler_t)(boolean_t);

/*
 * Register a CPU to kpc and allocate its buffers.
 *
 * @param cpu_data
 * CPU data associated to the CPU being registered.
 *
 * @return
 * TRUE if buffers are correctly allocated, FALSE otherwise.
 */
struct cpu_data;
extern boolean_t kpc_register_cpu(struct cpu_data *cpu_data);
extern void kpc_unregister_cpu(struct cpu_data *cpu_data);

extern bool kpc_supported;

/* bootstrap */
extern void kpc_init(void);

/* common initialization */
extern void kpc_common_init(void);

/* Architecture specific initialisation */
extern void kpc_arch_init(void);

/* Thread counting initialization */
extern void kpc_thread_init(void);

/* Get the bitmask of available classes */
extern uint32_t kpc_get_classes(void);

/* Get the bitmask of currently running counter classes  */
extern uint32_t kpc_get_running(void);

/* Get the version of KPC that's being run */
extern int kpc_get_pmu_version(void);

/* Set the bitmask of currently running counter classes. Specify
 * classes = 0 to stop counters
 */
extern int kpc_set_running(uint32_t classes);

/* Read CPU counters */
extern int kpc_get_cpu_counters(boolean_t all_cpus, uint32_t classes,
    int *curcpu, uint64_t *buf);

/* Read shadow counters */
extern int kpc_get_shadow_counters( boolean_t all_cpus, uint32_t classes,
    int *curcpu, uint64_t *buf );

/* Read current thread's counter accumulations */
extern int kpc_get_curthread_counters(uint32_t *inoutcount, uint64_t *buf);

/* Given a config, how many counters and config registers there are */
extern uint32_t kpc_get_counter_count(uint32_t classes);
extern uint32_t kpc_get_config_count(uint32_t classes);

/* enable/disable thread counting */
extern uint32_t kpc_get_thread_counting(void);
extern int      kpc_set_thread_counting(uint32_t classes);

/* get and set config registers */
extern int kpc_get_config(uint32_t classes, kpc_config_t *current_config);
extern int kpc_set_config(uint32_t classes, kpc_config_t *new_config);

/* get and set PMI period */
extern int kpc_get_period(uint32_t classes, uint64_t *period);
extern int kpc_set_period(uint32_t classes, uint64_t *period);

/* get and set kperf actionid */
extern int kpc_get_actionid(uint32_t classes, uint32_t *actionid);
extern int kpc_set_actionid(uint32_t classes, uint32_t *actionid);

/* hooks on thread create and delete */
extern void kpc_thread_create(thread_t thread);
extern void kpc_thread_destroy(thread_t thread);

/* allocate a buffer big enough for all counters */
extern uint64_t *kpc_counterbuf_alloc(void);
extern void      kpc_counterbuf_free(uint64_t*);
extern uint32_t  kpc_get_counterbuf_size(void);

/* whether we're currently accounting into threads */
extern int kpc_threads_counting;

/* AST callback for KPC */
extern void kpc_thread_ast_handler( thread_t thread );

#ifdef MACH_KERNEL_PRIVATE

/* context switch callback for KPC */

extern boolean_t kpc_off_cpu_active;

extern void kpc_off_cpu_internal(thread_t thread);
extern void kpc_off_cpu_update(void);

static inline void
kpc_off_cpu(thread_t thread)
{
	if (__improbable(kpc_off_cpu_active)) {
		kpc_off_cpu_internal(thread);
	}
}

#endif /* defined(MACH_KERNEL_PRIVATE) */

/* acquire/release the counters used by the Power Manager */
extern int kpc_force_all_ctrs( task_t task, int val );
extern int kpc_get_force_all_ctrs( void );

/* arch-specific routine for acquire/release the counters used by the Power Manager */
extern int kpc_force_all_ctrs_arch( task_t task, int val );

extern int kpc_set_sw_inc( uint32_t mask );

/* disable/enable whitelist of allowed events */
extern int kpc_get_whitelist_disabled( void );
extern int kpc_disable_whitelist( int val );

/*
 * Register the Power Manager as a PMCs user.
 *
 * This is a deprecated function used by old Power Managers, new Power Managers
 * should use the @em kpc_reserve_pm_counters() function. This function actually
 * calls @em kpc_reserve_pm_counters() with the following arguments:
 *      - handler	= handler
 *      - pmc_mask	= 0x83
 *      - custom_config	= TRUE
 *
 * See @em kpc_reserve_pm_counters() for more details about the return value.
 */
extern boolean_t kpc_register_pm_handler(void (*handler)(boolean_t));

/*
 * Register the Power Manager as a PMCs user.
 *
 * @param handler
 * Notification callback to use when PMCs are acquired/released by a task.
 * Power management must acknowledge the change using kpc_pm_acknowledge.
 *
 * @param pmc_mask
 * Bitmask of the configurable PMCs used by the Power Manager. The number of bits
 * set must less or equal than the number of configurable counters
 * available on the SoC.
 *
 * @param custom_config
 * If custom_config=TRUE, the legacy sharing mode is enabled, otherwise the
 * Modern Sharing mode is enabled. These modes are explained in more details in
 * the kperf documentation.
 *
 * @return
 * FALSE if a task has acquired all the PMCs, otherwise TRUE and the Power
 * Manager can start using the reserved PMCs.
 */
extern boolean_t kpc_reserve_pm_counters(uint64_t pmc_mask, kpc_pm_handler_t handler,
    boolean_t custom_config);

/*
 * Unregister the Power Manager as a PMCs user, and release the previously
 * reserved counters.
 */
extern void kpc_release_pm_counters(void);

/*
 * Acknowledge the callback that PMCs are available to power management.
 *
 * @param available_to_pm Whether the counters were made available to power
 * management in the callback.  Pass in whatever was passed into the handler
 * function.  After this point, power management is able to use POWER_CLASS
 * counters.
 */
extern void kpc_pm_acknowledge(boolean_t available_to_pm);

/*
 * Is the PMU used by both the power manager and userspace?
 *
 * This is true when the power manager has been registered. It disables certain
 * counter configurations (like RAWPMU) that are incompatible with sharing
 * counters.
 */
extern boolean_t kpc_multiple_clients(void);

/*
 * Is kpc controlling the fixed counters?
 *
 * This returns false when the power manager has requested custom configuration
 * control.
 */
extern boolean_t kpc_controls_fixed_counters(void);

/*
 * Is kpc controlling a specific PMC ?
 */
extern boolean_t kpc_controls_counter(uint32_t ctr);


extern void kpc_idle(void);
extern void kpc_idle_exit(void);


/*
 * KPC PRIVATE
 */

extern uint32_t kpc_actionid[KPC_MAX_COUNTERS];

/* handler for mp operations */
struct kpc_config_remote {
	uint32_t classes;
	kpc_config_t *configv;
	uint64_t pmc_mask;
};

/* handler for mp operations */
struct kpc_running_remote {
	uint32_t        classes;                /* classes to run */
	uint64_t        cfg_target_mask;        /* configurable counters selected */
	uint64_t        cfg_state_mask;         /* configurable counters new state */
};

/* handler for mp operations */
struct kpc_get_counters_remote {
	uint32_t classes;
	uint32_t nb_counters;
	uint32_t buf_stride;
	uint64_t *buf;
};

extern int kpc_get_all_cpus_counters(uint32_t classes, int *curcpu, uint64_t *buf);
extern int kpc_get_curcpu_counters(uint32_t classes, int *curcpu, uint64_t *buf);
extern int kpc_get_fixed_counters(uint64_t *counterv);
extern int kpc_get_configurable_counters(uint64_t *counterv, uint64_t pmc_mask);
extern boolean_t kpc_is_running_fixed(void);
extern boolean_t kpc_is_running_configurable(uint64_t pmc_mask);
extern uint32_t kpc_fixed_count(void);
extern uint32_t kpc_configurable_count(void);
extern uint32_t kpc_fixed_config_count(void);
extern uint32_t kpc_configurable_config_count(uint64_t pmc_mask);
extern uint32_t kpc_rawpmu_config_count(void);
extern int kpc_get_fixed_config(kpc_config_t *configv);
extern int kpc_get_configurable_config(kpc_config_t *configv, uint64_t pmc_mask);
extern int kpc_get_rawpmu_config(kpc_config_t *configv);
extern uint64_t kpc_fixed_max(void);
extern uint64_t kpc_configurable_max(void);
extern int kpc_set_config_arch(struct kpc_config_remote *mp_config);
extern int kpc_set_period_arch(struct kpc_config_remote *mp_config);
extern void kpc_sample_kperf(uint32_t actionid);
extern int kpc_set_running_arch(struct kpc_running_remote *mp_config);


/*
 * Helpers
 */

/* count the number of bits set */
extern uint8_t kpc_popcount(uint64_t value);

/* for a set of classes, retrieve the configurable PMCs mask */
extern uint64_t kpc_get_configurable_pmc_mask(uint32_t classes);


/* Interface for kexts to publish a kpc interface */
struct kpc_driver {
	uint32_t (*get_classes)(void);
	uint32_t (*get_running)(void);
	int      (*set_running)(uint32_t classes);
	int      (*get_cpu_counters)(boolean_t all_cpus, uint32_t classes,
	    int *curcpu, uint64_t *buf);
	int      (*get_curthread_counters)(uint32_t *inoutcount, uint64_t *buf);
	uint32_t (*get_counter_count)(uint32_t classes);
	uint32_t (*get_config_count)(uint32_t classes);
	int      (*get_config)(uint32_t classes, kpc_config_t *current_config);
	int      (*set_config)(uint32_t classes, kpc_config_t *new_config);
	int      (*get_period)(uint32_t classes, uint64_t *period);
	int      (*set_period)(uint32_t classes, uint64_t *period);
};

__END_DECLS

#endif /* !definde(KERN_KPC_H) */

/*
 * Copyright (c) 2005-2006 Apple Computer, Inc. All rights reserved.
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

#ifndef _DTRACE_GLUE_H
#define _DTRACE_GLUE_H

#ifdef KERNEL_BUILD

#include <libkern/libkern.h>
#include <kern/locks.h>
#include <kern/debug.h>
#include <kern/thread_call.h>
#include <kern/thread.h>
#include <machine/atomic.h>
#include <machine/machine_routines.h>
#include <sys/syslog.h>
#include <sys/ucred.h>
#include <stdarg.h>
#include <mach/kmod.h>

#if defined(__i386__) || defined(__x86_64__)
#include <i386/mp.h>
#endif

/*
 * cmn_err
 */
#define CE_NOTE         1       /* notice		*/
#define CE_WARN         2       /* warning		*/

extern void cmn_err( int, const char *, ... );

/*
 * pid/proc
 */

/* Solaris proc_t is the struct. Darwin's proc_t is a pointer to it. */
#define proc_t struct proc /* Steer clear of the Darwin typedef for proc_t */
#define curproc ((struct proc *)current_proc()) /* Called from probe context, must blacklist */

proc_t* sprlock(pid_t pid);
void sprunlock(proc_t *p);

void dtrace_sprlock(proc_t *p);
void dtrace_sprunlock(proc_t *p);

/*
 * uread/uwrite
 */

int uread(proc_t *p, void *buf, user_size_t len, user_addr_t a);
int uwrite(proc_t *p, void *buf, user_size_t len, user_addr_t a);

/*
 * fuword / suword
 */

int fuword8(user_addr_t, uint8_t *);
int fuword16(user_addr_t, uint16_t *);
int fuword32(user_addr_t, uint32_t *);
int fuword64(user_addr_t, uint64_t *);

void fuword32_noerr(user_addr_t, uint32_t *);
void fuword64_noerr(user_addr_t, uint64_t *);

int suword64(user_addr_t, uint64_t value);
int suword32(user_addr_t, uint32_t value);

/*
 * cpuvar
 */
extern lck_mtx_t cpu_lock;
extern lck_mtx_t cyc_lock;
extern lck_mtx_t mod_lock;

/*
 * wrap_timer_call: wrapper of timer_call for cyclic timers.
 */
struct wrap_timer_call;

/*
 * Per-CPU data.
 */
typedef struct dtrace_cpu {
	processorid_t   cpu_id;                    /* CPU number */
	struct dtrace_cpu *cpu_next;                 /* next existing CPU */
	lck_rw_t        cpu_ft_lock;               /* DTrace: fasttrap lock */
	uintptr_t       cpu_dtrace_caller;         /* DTrace: caller, if any */
	hrtime_t        cpu_dtrace_chillmark;      /* DTrace: chill mark time */
	hrtime_t        cpu_dtrace_chilled;        /* DTrace: total chill time */
	boolean_t       cpu_dtrace_invop_underway; /* DTrace gaurds against invalid op re-entrancy */

	/* Local cyclic timers on this CPU */
	LIST_HEAD(cyc_list_head, wrap_timer_call) cpu_cyc_list;
} dtrace_cpu_t;

extern dtrace_cpu_t *cpu_list;

/*
 * The cpu_core structure consists of per-CPU state available in any context.
 * On some architectures, this may mean that the page(s) containing the
 * NCPU-sized array of cpu_core structures must be locked in the TLB -- it
 * is up to the platform to assure that this is performed properly.  Note that
 * the structure is sized to avoid false sharing.
 */
#define CPU_CACHE_COHERENCE_SIZE        64

typedef struct cpu_core {
	uint64_t        cpuc_dtrace_illval;     /* DTrace illegal value */
	lck_mtx_t       cpuc_pid_lock;          /* DTrace pid provider lock */
	uint16_t        cpuc_dtrace_flags;      /* DTrace flags */
	uint64_t        cpuc_missing_tos;       /* Addr. of top most stack frame if missing */
	uint8_t         cpuc_pad[CPU_CACHE_COHERENCE_SIZE - sizeof(uint64_t) - sizeof(lck_mtx_t) - sizeof(uint16_t) - sizeof(uint64_t)];        /* padding */
} cpu_core_t;

extern cpu_core_t *cpu_core;

extern unsigned int dtrace_max_cpus;            /* max number of enabled cpus */
#define NCPU        dtrace_max_cpus

extern int cpu_number(void); /* From #include <kern/cpu_number.h>. Called from probe context, must blacklist. */

#define CPU             (&(cpu_list[cpu_number()]))     /* Pointer to current CPU */
#define CPU_ON_INTR(cpup) ml_at_interrupt_context() /* always invoked on current cpu */

/*
 * Routines used to register interest in cpu's being added to or removed
 * from the system.
 */
typedef enum {
	CPU_INIT,
	CPU_CONFIG,
	CPU_UNCONFIG,
	CPU_ON,
	CPU_OFF,
	CPU_CPUPART_IN,
	CPU_CPUPART_OUT
} cpu_setup_t;

typedef int cpu_setup_func_t(cpu_setup_t, int, void *);

extern void register_cpu_setup_func(cpu_setup_func_t *, void *);
extern void unregister_cpu_setup_func(cpu_setup_func_t *, void *);

/*
 * CPU_DTRACE
 */

/*
 * DTrace flags.
 */
#define CPU_DTRACE_NOFAULT      0x0001  /* Don't fault */
#define CPU_DTRACE_DROP         0x0002  /* Drop this ECB */
#define CPU_DTRACE_BADADDR      0x0004  /* DTrace fault: bad address */
#define CPU_DTRACE_BADALIGN     0x0008  /* DTrace fault: bad alignment */
#define CPU_DTRACE_DIVZERO      0x0010  /* DTrace fault: divide by zero */
#define CPU_DTRACE_ILLOP        0x0020  /* DTrace fault: illegal operation */
#define CPU_DTRACE_NOSCRATCH    0x0040  /* DTrace fault: out of scratch */
#define CPU_DTRACE_KPRIV        0x0080  /* DTrace fault: bad kernel access */
#define CPU_DTRACE_UPRIV        0x0100  /* DTrace fault: bad user access */
#define CPU_DTRACE_TUPOFLOW     0x0200  /* DTrace fault: tuple stack overflow */
#define CPU_DTRACE_USTACK_FP    0x0400  /* pid provider hint to ustack() */
#define CPU_DTRACE_ENTRY        0x0800  /* pid provider hint to ustack() */
#define CPU_DTRACE_BADSTACK 0x1000  /* DTrace fault: bad stack */

#define CPU_DTRACE_FAULT        (CPU_DTRACE_BADADDR | CPU_DTRACE_BADALIGN | \
	                        CPU_DTRACE_DIVZERO | CPU_DTRACE_ILLOP | \
	                        CPU_DTRACE_NOSCRATCH | CPU_DTRACE_KPRIV | \
	                        CPU_DTRACE_UPRIV | CPU_DTRACE_TUPOFLOW | \
	                        CPU_DTRACE_BADSTACK)
#define CPU_DTRACE_ERROR        (CPU_DTRACE_FAULT | CPU_DTRACE_DROP)

/*
 * Loadable Modules
 */

/* Keep the compiler happy */
struct dtrace_module_symbols;

/* Solaris' modctl structure, greatly simplified, shadowing parts of xnu kmod structure. */
typedef struct modctl {
	struct modctl   *mod_next;
	struct modctl   *mod_stale;     // stale module chain
	uint32_t        mod_id;         // the kext unique identifier
	char            mod_modname[KMOD_MAX_NAME];
	int             mod_loadcnt;
	char            mod_loaded;
	uint16_t        mod_flags;      // See flags below
	int             mod_nenabled;   // # of enabled DTrace probes in module
	vm_address_t    mod_address;    // starting address (of Mach-o header blob)
	vm_size_t       mod_size;       // total size (of blob)
	UUID            mod_uuid;
	struct dtrace_module_symbols* mod_user_symbols;
} modctl_t;

/* Definitions for mod_flags */
#define MODCTL_IS_MACH_KERNEL                   0x01  // This module represents /mach_kernel
#define MODCTL_HAS_KERNEL_SYMBOLS               0x02  // Kernel symbols (nlist) are available
#define MODCTL_FBT_PROBES_PROVIDED              0x04  // fbt probes have been provided
#define MODCTL_FBT_INVALID                      0x08  // Module is invalid for fbt probes
#define MODCTL_SDT_PROBES_PROVIDED              0x10  // sdt probes have been provided
#define MODCTL_SDT_INVALID                      0x20  // Module is invalid for sdt probes
#define MODCTL_HAS_UUID                         0x40  // Module has UUID
#define MODCTL_FBT_PROVIDE_BLACKLISTED_PROBES   0x200 // fbt provider must provide blacklisted probes
#define MODCTL_FBT_BLACKLISTED_PROBES_PROVIDED  0x400 // fbt blacklisted probes have been provided
#define MODCTL_IS_STATIC_KEXT                   0x800 // module is a static kext

/* Simple/singular mod_flags accessors */
#define MOD_IS_MACH_KERNEL(mod)                 (mod->mod_flags & MODCTL_IS_MACH_KERNEL)
#define MOD_HAS_KERNEL_SYMBOLS(mod)             (mod->mod_flags & MODCTL_HAS_KERNEL_SYMBOLS)
#define MOD_HAS_USERSPACE_SYMBOLS(mod)          (mod->mod_user_symbols) /* No point in duplicating state in the flags bits */
#define MOD_FBT_PROBES_PROVIDED(mod)            (mod->mod_flags & MODCTL_FBT_PROBES_PROVIDED)
#define MOD_FBT_INVALID(mod)                    (mod->mod_flags & MODCTL_FBT_INVALID)
#define MOD_SDT_PROBES_PROVIDED(mod)            (mod->mod_flags & MODCTL_SDT_PROBES_PROVIDED)
#define MOD_SDT_INVALID(mod)                    (mod->mod_flags & MODCTL_SDT_INVALID)
#define MOD_HAS_UUID(mod)                       (mod->mod_flags & MODCTL_HAS_UUID)
#define MOD_FBT_BLACKLISTED_PROBES_PROVIDED(mod) (mod->mod_flags & MODCTL_FBT_BLACKLISTED_PROBES_PROVIDED)
#define MOD_FBT_PROVIDE_BLACKLISTED_PROBES(mod) (mod->mod_flags & MODCTL_FBT_PROVIDE_BLACKLISTED_PROBES)
#define MOD_IS_STATIC_KEXT(mod)                 (mod->mod_flags & MODCTL_IS_STATIC_KEXT)

/* Compound accessors */
#define MOD_FBT_BLACKLISTED_PROBES_DONE(mod)    (MOD_FBT_BLACKLISTED_PROBES_PROVIDED(mod) || !MOD_FBT_PROVIDE_BLACKLISTED_PROBES(mod))
#define MOD_FBT_DONE(mod)                       ((MOD_FBT_PROBES_PROVIDED(mod) && MOD_FBT_BLACKLISTED_PROBES_DONE(mod)) || MOD_FBT_INVALID(mod))
#define MOD_SDT_DONE(mod)                       (MOD_SDT_PROBES_PROVIDED(mod) || MOD_SDT_INVALID(mod))
#define MOD_SYMBOLS_DONE(mod)                   (MOD_FBT_DONE(mod) && MOD_SDT_DONE(mod))

extern modctl_t *dtrace_modctl_list;

extern int dtrace_addr_in_module(void*, struct modctl*);

/*
 * cred_t
 */
/* Privileges */
#define PRIV_DTRACE_KERNEL        3
#define PRIV_DTRACE_PROC          4
#define PRIV_DTRACE_USER          5
#define PRIV_PROC_OWNER          30
#define PRIV_PROC_ZONE           35
#define PRIV_ALL                        (-1)    /* All privileges required */

/* Privilege sets */
#define PRIV_EFFECTIVE            0

typedef struct ucred cred_t;
#define cr_suid cr_svuid
#define cr_sgid cr_svgid

extern cred_t *dtrace_CRED(void); /* Safe to call from probe context. */
#define CRED() kauth_cred_get() /* Can't be called from probe context! */
extern int PRIV_POLICY_CHOICE(void *, int, int);
extern int PRIV_POLICY_ONLY(void *, int, int);
extern uid_t crgetuid(const cred_t *);
#define crgetzoneid(x) ((zoneid_t)0)

/*
 * "cyclic"
 */
#define CY_LOW_LEVEL            0
#define CY_HIGH_LEVEL           2
#define CY_LEVELS                       3

typedef uintptr_t cyclic_id_t;
typedef cyclic_id_t *cyclic_id_list_t;
typedef uint16_t cyc_level_t;
typedef void (*cyc_func_t)(void *);

#define CYCLIC_NONE             ((cyclic_id_t)0)

typedef struct cyc_time {
	hrtime_t cyt_when;
	hrtime_t cyt_interval;
} cyc_time_t;

typedef struct cyc_handler {
	cyc_func_t cyh_func;
	void *cyh_arg;
	cyc_level_t cyh_level;
} cyc_handler_t;

typedef struct cyc_omni_handler {
	void (*cyo_online)(void *, dtrace_cpu_t *, cyc_handler_t *, cyc_time_t *);
	void (*cyo_offline)(void *, dtrace_cpu_t *, void *);
	void *cyo_arg;
} cyc_omni_handler_t;

extern void dtrace_install_cpu_hooks(void);

extern cyclic_id_t cyclic_add(cyc_handler_t *, cyc_time_t *);
extern void cyclic_remove(cyclic_id_t);

extern cyclic_id_list_t cyclic_add_omni(cyc_omni_handler_t *);
extern void cyclic_remove_omni(cyclic_id_list_t);

extern cyclic_id_t cyclic_timer_add(cyc_handler_t *, cyc_time_t *);
extern void cyclic_timer_remove(cyclic_id_t);

/*
 * ddi
 */

#define DDI_SUCCESS                     0
#define DDI_FAILURE                     -1

#define DDI_PSEUDO "ddi_pseudo"

typedef enum {
	DDI_DETACH = 0,
	DDI_SUSPEND = 1,
	DDI_PM_SUSPEND = 2,
	DDI_HOTPLUG_DETACH = 3          /* detach, don't try to auto-unconfig */
} ddi_detach_cmd_t;

#define DDI_PROP_SUCCESS        0

#define DDI_PROP_DONTPASS   1
typedef uint_t major_t;
typedef uint_t minor_t;

typedef struct __dev_info *dev_info_t;

extern int ddi_driver_major(dev_info_t *);

extern int ddi_create_minor_node(dev_info_t *, const char *, int, minor_t, const char *, int);
extern void ddi_remove_minor_node(dev_info_t *, char *);

extern major_t getemajor(dev_t);
extern minor_t getminor(dev_t);

/*
 * Kernel Debug Interface
 */
extern void debug_enter(char *);

/*
 * DTrace specific zone allocation
 */

/*
 * kmem
 */

#define KM_SLEEP        0x00000000
#define KM_NOSLEEP      0x00000001

typedef struct vmem vmem_t;
typedef struct kmem_cache kmem_cache_t;

#define kmem_free dt_kmem_free /* Avoid clash with Darwin's kmem_free */
#define kmem_free_aligned dt_kmem_free_aligned

#define kmem_alloc(size, kmflag) \
	({ VM_ALLOC_SITE_STATIC(0, 0); \
	dt_kmem_alloc_site(size, kmflag, &site); })

extern void *dt_kmem_alloc_site(size_t, int, vm_allocation_site_t*);
extern void dt_kmem_free(void *, size_t);

#define kmem_zalloc(size, kmflag) \
	({ VM_ALLOC_SITE_STATIC(0, 0); \
	dt_kmem_zalloc_site(size, kmflag, &site); })

extern void *dt_kmem_zalloc_site(size_t, int, vm_allocation_site_t*);

#define kmem_alloc_aligned(size, align, kmflag) \
	({ VM_ALLOC_SITE_STATIC(0, 0); \
	dt_kmem_alloc_aligned_site(size, align, kmflag, &site); })
extern void *dt_kmem_alloc_aligned_site(size_t, size_t, int, vm_allocation_site_t*);

#define kmem_zalloc_aligned(size, align, kmflag) \
	({ VM_ALLOC_SITE_STATIC(0, 0); \
	dt_kmem_zalloc_aligned_site(size, align, kmflag, &site); })
extern void *dt_kmem_zalloc_aligned_site(size_t, size_t, int, vm_allocation_site_t*);

extern void dt_kmem_free_aligned(void*, size_t);

extern kmem_cache_t *
    kmem_cache_create(const char *, size_t, size_t, int (*)(void *, void *, int),
    void (*)(void *, void *), void (*)(void *), void *, vmem_t *, int);
extern void *kmem_cache_alloc(kmem_cache_t *, int);
extern void kmem_cache_free(kmem_cache_t *, void *);
extern void kmem_cache_destroy(kmem_cache_t *);

/*
 * kthread
 */

typedef struct _kthread kthread_t; /* For dtrace_vtime_switch(), dtrace_panicked and dtrace_errthread */

/*
 * proc
 */


#define DATAMODEL_ILP32 0x00100000
#define DATAMODEL_LP64  0x00200000

#define DATAMODEL_NONE  0

#if     defined(__LP64__)
#define DATAMODEL_NATIVE        DATAMODEL_LP64
#else
#define DATAMODEL_NATIVE        DATAMODEL_ILP32
#endif  /* __LP64__ */

typedef unsigned int model_t; /* For dtrace_instr_size_isa() prototype in <sys/dtrace.h> */

/*
 * vmem
 */

#define VMC_IDENTIFIER  0x00040000      /* not backed by memory */
#define VM_SLEEP        0x00000000      /* same as KM_SLEEP */
#define VM_BESTFIT      0x00000100

extern void *vmem_alloc(vmem_t *, size_t, int);
extern vmem_t *vmem_create(const char *, void *, size_t, size_t, void *,
    void *, vmem_t *, size_t, int);
extern void vmem_destroy(vmem_t *);
extern void vmem_free(vmem_t *vmp, void *vaddr, size_t size);

/*
 * Miscellaneous
 */

typedef uintptr_t pc_t;
typedef uintptr_t greg_t; /* For dtrace_impl.h prototype of dtrace_getfp() */
#if defined(__arm__) || defined(__arm64__)
#define regs arm_saved_state
#endif
extern struct regs *find_user_regs( thread_t thread);
extern vm_offset_t dtrace_get_cpu_int_stack_top(void);

#define panic_quiesce (panic_active())

#define IS_P2ALIGNED(v, a) ((((uintptr_t)(v)) & ((uintptr_t)(a) - 1)) == 0)

extern int vuprintf(const char *, va_list);

extern hrtime_t dtrace_abs_to_nano(uint64_t);

__private_extern__ const char * strstr(const char *, const char *);
const void* bsearch(const void*, const void*, size_t, size_t, int (*compar)(const void *, const void *));

int dtrace_copy_maxsize(void);
int dtrace_buffer_copyout(const void*, user_addr_t, vm_size_t);


#define DTRACE_NCLIENTS 32

#undef proc_t

/*
 * Safe counted string compare against a literal string. The sizeof() intentionally
 * counts the trailing NUL, and so ensures that all the characters in the literal
 * can participate in the comparison.
 */
#define LIT_STRNEQL(s1, lit_s2) (0 == strncmp( (s1), (lit_s2), sizeof((lit_s2)) ))

#define KERNELBASE VM_MIN_KERNEL_ADDRESS
#endif /* KERNEL_BUILD */
#endif /* _DTRACE_GLUE_H */

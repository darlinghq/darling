/*
 * Copyright (c) 2000-2018 Apple Inc. All rights reserved.
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

#ifndef BSD_SYS_KDEBUG_KERNEL_H
#define BSD_SYS_KDEBUG_KERNEL_H

#include <mach/boolean.h>
#include <mach/clock_types.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

#ifdef KERNEL

/*
 * To use kdebug in the kernel:
 *
 * #include <sys/kdebug_kernel.h>
 *
 * #define DBG_NETIPINIT NETDBG_CODE(DBG_NETIP, 1)
 *
 * void
 * ip_init(void)
 * {
 *     KDBG(DBG_NETIPINIT | DBG_FUNC_START, 1, 2, 3, 4);
 *     ...
 *     KDBG(DBG_NETIPINIT);
 *     ...
 *     KDBG(DBG_NETIPINIT | DBG_FUNC_END);
 * }
 */

#pragma mark - kernel tracepoints

/*
 * The KDBG{,_DEBUG,_RELEASE,_FILTERED} macros are the preferred method of
 * making tracepoints.
 *
 * Kernel pointers must be unslid or permuted using VM_KERNEL_UNSLIDE_OR_PERM.
 * Do not trace any sensitive data.
 */

/*
 * Traced on debug and development (and release macOS) kernels.
 */
#define KDBG(x, ...) KDBG_(, x, ## __VA_ARGS__, 4, 3, 2, 1, 0)

/*
 * Traced on debug and development (and release macOS) kernels if explicitly
 * requested.  Omitted from tracing without a typefilter.
 */
#define KDBG_FILTERED(x, ...) KDBG_(_FILTERED, x, ## __VA_ARGS__, 4, 3, 2, 1, 0)

#ifdef KERNEL_PRIVATE

/*
 * Traced on debug and development (and release macOS) kernels, even if the
 * process filter would reject it.
 */
#define KDBG_RELEASE_NOPROCFILT(x, ...) \
	        KDBG_(_RELEASE_NOPROCFILT, x, ## __VA_ARGS__, 4, 3, 2, 1, 0)

#endif /* KERNEL_PRIVATE */

/*
 * Traced on debug, development, and release kernels.
 *
 * Only use this tracepoint if the events are required for a shipping trace
 * tool.
 */
#define KDBG_RELEASE(x, ...) KDBG_(_RELEASE, x, ## __VA_ARGS__, 4, 3, 2, 1, 0)

/*
 * Traced only on debug kernels.
 */
#define KDBG_DEBUG(x, ...) KDBG_(_DEBUG, x, ## __VA_ARGS__, 4, 3, 2, 1, 0)

#pragma mark - kernel API

#ifdef KERNEL_PRIVATE

/*
 * kernel_debug_string provides the same functionality as the
 * kdebug_trace_string syscall as a KPI.  str_id is an in/out
 * parameter that, if it's pointing to a string ID of 0, will
 * receive a generated ID.  If it provides a value in str_id,
 * then that will be used, instead.
 *
 * Returns an errno indicating the type of failure.
 */
int kernel_debug_string(uint32_t debugid, uint64_t *str_id, const char *str);

/*
 * kernel_debug_disable disables event logging, but leaves any buffers
 * intact.
 */
void kernel_debug_disable(void);

#endif /* KERNEL_PRIVATE */

/*
 * Returns true if kdebug is using continuous time for its events, and false
 * otherwise.
 */
bool kdebug_using_continuous_time(void);

/*
 * Returns true if kdebug will log an event with the provided debugid, and
 * false otherwise.
 */
bool kdebug_debugid_enabled(uint32_t debugid);

/*
 * Returns true only if the debugid is explicitly enabled by filters.  Returns
 * false otherwise, including when no filters are active.
 */
bool kdebug_debugid_explicitly_enabled(uint32_t debugid);

uint32_t kdebug_commpage_state(void);

#pragma mark - IOP tracing

/*
 * Definitions to support IOP tracing.
 */

typedef enum {
	/* Trace is now enabled; no arguments.  */
	KD_CALLBACK_KDEBUG_ENABLED,
	/* Trace is now disabled; no arguments.  */
	KD_CALLBACK_KDEBUG_DISABLED,
	/*
	 * Request the latest entries from the IOP and block until complete; no
	 * arguments.
	 */
	KD_CALLBACK_SYNC_FLUSH,
	/*
	 * The typefilter is enabled; a read-only pointer to the typefilter is
	 * provided, valid only while in the callback.
	 */
	KD_CALLBACK_TYPEFILTER_CHANGED,
} kd_callback_type;

typedef void (*kd_callback_fn) (void *context, kd_callback_type reason,
    void *arg);

struct kd_callback {
	kd_callback_fn func;
	void *context;
	/* name of IOP, NUL-terminated */
	char iop_name[8];
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
 * Caveats:
 * Note that not all callback calls will indicate a change in
 * state (e.g. disabling trace twice would send two disable
 * notifications).
 */
int kernel_debug_register_callback(kd_callback_t callback);

void kernel_debug_enter(uint32_t coreid, uint32_t debugid, uint64_t timestamp,
    uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4,
    uintptr_t threadid);

#pragma mark - internals

#define KDBG_(f, x, a, b, c, d, n, ...) KDBG##n(f, x, a, b, c, d)
#define KDBG0(f, x, a, b, c, d) KERNEL_DEBUG_CONSTANT##f(x, 0, 0, 0, 0, 0)
#define KDBG1(f, x, a, b, c, d) KERNEL_DEBUG_CONSTANT##f(x, a, 0, 0, 0, 0)
#define KDBG2(f, x, a, b, c, d) KERNEL_DEBUG_CONSTANT##f(x, a, b, 0, 0, 0)
#define KDBG3(f, x, a, b, c, d) KERNEL_DEBUG_CONSTANT##f(x, a, b, c, 0, 0)
#define KDBG4(f, x, a, b, c, d) KERNEL_DEBUG_CONSTANT##f(x, a, b, c, d, 0)

#ifdef XNU_KERNEL_PRIVATE
#define KDBG_IMPROBABLE __improbable
#else
#define KDBG_IMPROBABLE
#endif

extern unsigned int kdebug_enable;

/*
 * The kernel debug configuration level.  These values control which events are
 * compiled in under different build configurations.
 *
 * Infer the supported kernel debug event level from config option.  Use
 * (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) as a guard to protect unaudited debug
 * code.
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
/*
 * Currently, all other kernel configurations (development, etc) build with
 * KDEBUG_LEVEL_STANDARD.
 */
#endif

/*
 * KERNEL_DEBUG_CONSTANT_FILTERED events are omitted from tracing unless they
 * are explicitly requested in the typefilter.  They are not emitted when
 * tracing without a typefilter.
 */
#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD)
#define KERNEL_DEBUG_CONSTANT_FILTERED(x, a, b, c, d, ...)           \
	do {                                                             \
	        if (KDBG_IMPROBABLE(kdebug_enable & ~KDEBUG_ENABLE_PPT)) {   \
	                kernel_debug_filtered((x), (uintptr_t)(a), (uintptr_t)(b),  \
	                        (uintptr_t)(c), (uintptr_t)(d)); \
	        }                                                            \
	} while (0)
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) */
#define KERNEL_DEBUG_CONSTANT_FILTERED(type, x, a, b, c, d, ...) do {} while (0)
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) */

#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST)
#define KERNEL_DEBUG_CONSTANT_RELEASE_NOPROCFILT(x, a, b, c, d, ...)   \
	do {                                                               \
	        if (KDBG_IMPROBABLE(kdebug_enable & ~KDEBUG_ENABLE_PPT)) {     \
	                kernel_debug_flags((x), (uintptr_t)(a), (uintptr_t)(b),    \
	                        (uintptr_t)(c), (uintptr_t)(d), KDBG_FLAG_NOPROCFILT); \
	        }                                                              \
	} while (0)
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST) */
#define KERNEL_DEBUG_CONSTANT_RELEASE_NOPROCFILT(x, a, b, c, d, ...) \
	do { } while (0)
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST) */


#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD)
#define KERNEL_DEBUG_CONSTANT(x, a, b, c, d, e)                               \
	do {                                                                      \
	        if (KDBG_IMPROBABLE(kdebug_enable & ~KDEBUG_ENABLE_PPT)) {            \
	                kernel_debug((x), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), \
	                        (uintptr_t)(d),(uintptr_t)(e));                               \
	        }                                                                     \
	} while (0)

/*
 * DO NOT USE THIS MACRO -- it breaks fundamental assumptions about ktrace and
 * is only meant to be used by the pthread kext and other points in the kernel
 * where the thread ID must be provided explicitly.
 */
#define KERNEL_DEBUG_CONSTANT1(x, a, b, c, d, e)                               \
	do {                                                                       \
	        if (KDBG_IMPROBABLE(kdebug_enable & ~KDEBUG_ENABLE_PPT)) {             \
	                kernel_debug1((x), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), \
	                (uintptr_t)(d), (uintptr_t)(e));                                   \
	        }                                                                      \
	} while (0)

#define KERNEL_DEBUG_EARLY(x, a, b, c, d)                                 \
	do {                                                                  \
	        kernel_debug_early((uint32_t)(x), (uintptr_t)(a), (uintptr_t)(b), \
	                (uintptr_t)(c), (uintptr_t)(d));                              \
	} while (0)
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) */
#define KERNEL_DEBUG_CONSTANT(x, a, b, c, d, e) do {} while (0)
#define KERNEL_DEBUG_CONSTANT1(x, a, b, c, d, e) do {} while (0)
#define KERNEL_DEBUG_EARLY(x, a, b, c, d) do {} while (0)
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_STANDARD) */

/*
 * KERNEL_DEBUG_CONSTANT_IST (in-system trace) events provide an audited subset
 * of tracepoints for userland system tracing tools.  This tracing level was
 * created by 8857227 to protect fairplayd and other PT_DENY_ATTACH processes.
 * It has two effects: only KERNEL_DEBUG_CONSTANT_IST() traces are emitted and
 * any PT_DENY_ATTACH processes will only emit basic traces as defined by the
 * kernel_debug_filter() routine.
 */
#define KERNEL_DEBUG_CONSTANT_RELEASE(x, a, b, c, d, e) \
	KERNEL_DEBUG_CONSTANT_IST(~KDEBUG_ENABLE_PPT, x, a, b, c, d, 0)

#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST)
#define KERNEL_DEBUG_CONSTANT_IST(type, x, a, b, c, d, e)                     \
	do {                                                                      \
	        if (KDBG_IMPROBABLE(kdebug_enable & (type))) {                        \
	                kernel_debug((x), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), \
	                        (uintptr_t)(d), 0);                                           \
	        }                                                                     \
	} while (0)
#define KERNEL_DEBUG_CONSTANT_IST1(x, a, b, c, d, e)                     \
	do {                                                                       \
	        if (KDBG_IMPROBABLE(kdebug_enable)) {                         \
	                kernel_debug1((x), (uintptr_t)(a), (uintptr_t)(b), (uintptr_t)(c), \
	                        (uintptr_t)(d), (uintptr_t)(e));                               \
	        }                                                                      \
	} while (0)
#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST) */
#define KERNEL_DEBUG_CONSTANT_IST(type, x, a, b, c, d, e) do {} while (0)
#define KERNEL_DEBUG_CONSTANT_IST1(x, a, b, c, d, e) do {} while (0)
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_IST) */

#if NO_KDEBUG
#define __kdebug_constant_only __unused
#endif

/*
 * KERNEL_DEBUG events are only traced for DEBUG kernels.
 */
#define KERNEL_DEBUG_CONSTANT_DEBUG(x, a, b, c, d, e) \
	KERNEL_DEBUG(x, a, b, c, d, e)

#if (KDEBUG_LEVEL >= KDEBUG_LEVEL_FULL)
#define __kdebug_only

#undef KERNEL_DEBUG
#define KERNEL_DEBUG(x, a, b, c, d, e)                                  \
	do {                                                                \
	        if (KDBG_IMPROBABLE(kdebug_enable & ~KDEBUG_ENABLE_PPT)) {      \
	                kernel_debug((uint32_t)(x), (uintptr_t)(a), (uintptr_t)(b), \
	                        (uintptr_t)(c), (uintptr_t)(d), (uintptr_t)(e));        \
	        }                                                               \
	} while (0)

/*
 * DO NOT USE THIS MACRO -- see warning above for KERNEL_DEBUG_CONSTANT1.
 */
#define KERNEL_DEBUG1(x, a, b, c, d, e)                                  \
	do {                                                                 \
	        if (KDBG_IMPROBABLE(kdebug_enable & ~KDEBUG_ENABLE_PPT)) {       \
	                kernel_debug1((uint32_t)(x), (uintptr_t)(a), (uintptr_t)(b), \
	                        (uintptr_t)(c), (uintptr_t)(d), (uintptr_t)(e));         \
	        }                                                                \
	} while (0)

#else /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_FULL) */
#define __kdebug_only __unused

#undef KERNEL_DEBUG
#define KERNEL_DEBUG(x, a, b, c, d, e) do {} while (0)
#define KERNEL_DEBUG1(x, a, b, c, d, e) do {} while (0)
#endif /* (KDEBUG_LEVEL >= KDEBUG_LEVEL_FULL) */

void kernel_debug(uint32_t debugid, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);

void kernel_debug1(uint32_t debugid, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4, uintptr_t arg5);

#define KDBG_FLAG_FILTERED 0x01
#define KDBG_FLAG_NOPROCFILT 0x02

void kernel_debug_flags(uint32_t debugid, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4, uint64_t flags);

void kernel_debug_filtered(uint32_t debugid, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4);

#pragma mark - xnu API

#ifdef XNU_KERNEL_PRIVATE
/* Used in early boot to log events. */
void kernel_debug_early(uint32_t  debugid, uintptr_t arg1, uintptr_t arg2,
    uintptr_t arg3, uintptr_t arg4);
/* Used in early boot to log strings spanning only a single tracepoint. */
void kernel_debug_string_early(const char *message);
/* Used to trace strings within kdebug tracepoints on arbitrary eventids. */
void kernel_debug_string_simple(uint32_t eventid, const char *str);
/* Only used by ktrace to reset kdebug.  ktrace_lock must be held. */
extern void kdebug_reset(void);

void kdbg_dump_trace_to_file(const char *);
void kdebug_init(unsigned int n_events, char *filterdesc, bool wrapping);
void kdebug_trace_start(unsigned int n_events, const char *filterdesc,
    bool wrapping, bool at_wake);
void kdebug_free_early_buf(void);
void release_storage_unit(int cpu, uint32_t storage_unit);
bool allocate_storage_unit(int cpu);

struct proc;
void kdbg_trace_data(struct proc *proc, long *arg_pid, long *arg_uniqueid);
void kdbg_trace_string(struct proc *proc, long *arg1, long *arg2, long *arg3,
    long *arg4);

#define KDBG_VFS_LOOKUP_FLAG_LOOKUP 0x01
#define KDBG_VFS_LOOKUP_FLAG_NOPROCFILT 0x02
void kdebug_vfs_lookup(long *dbg_parms, int dbg_namelen, void *dp,
    uint32_t flags);

#endif /* XNU_KERNEL_PRIVATE */

#ifdef KERNEL_PRIVATE

#define NUMPARMS 23
void kdebug_lookup_gen_events(long *dbg_parms, int dbg_namelen, void *dp,
    bool lookup);

#pragma mark - EnergyTracing

#define KERNEL_DBG_IST_SANE KDBG_RELEASE
#define ENTR_KDTRACEFUNC KDBG_RELEASE

// value is int64_t, quality is uint32_t
#define KERNEL_ENERGYTRACE(opcode, lifespan, id, quality, value)        \
	    ENTR_KDTRACE(kEnTrCompKernel, opcode, lifespan, id,         \
	                 quality, value)
#define KERNEL_ENTR_ASSOCIATE(par_opcode, par_act_id, sub_opcode, sub_act_id) \
	    ENTR_KDASSOCIATE(kEnTrCompKernel, par_opcode, par_act_id,   \
	                     kEnTrCompKernel, sub_opcode, sub_act_id)

#endif /* KERNEL_PRIVATE */

#endif /* KERNEL */

__END_DECLS

#endif /* !defined(BSD_SYS_KDEBUG_KERNEL_H) */

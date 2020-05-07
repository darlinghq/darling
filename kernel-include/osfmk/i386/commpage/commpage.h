/*
 * Copyright (c) 2003-2019 Apple Inc. All rights reserved.
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

#ifndef _I386_COMMPAGE_H
#define _I386_COMMPAGE_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#include <mach/boolean.h>
#include <mach/vm_types.h>
#include <machine/cpu_capabilities.h>
#endif /* __ASSEMBLER__ */

/* When trying to acquire a spinlock or mutex, we will spin in
 * user mode for awhile, before entering the kernel to relinquish.
 * MP_SPIN_TRIES is the initial value of _COMM_PAGE_SPIN_COUNT.
 * The idea is that _COMM_PAGE_SPIN_COUNT will be adjusted up or
 * down as the machine is plugged in/out, etc.
 * At present spinlocks do not use _COMM_PAGE_SPIN_COUNT.
 * They use MP_SPIN_TRIES directly.
 */
#define MP_SPIN_TRIES   1000


/* The following macro is used to generate the 64-bit commpage address for a given
 * routine, based on its 32-bit address.  This is used in the kernel to compile
 * the 64-bit commpage.  Since the kernel can be a 32-bit object, cpu_capabilities.h
 * only defines the 32-bit address.
 */
#define _COMM_PAGE_32_TO_64( ADDRESS )  ( ADDRESS + _COMM_PAGE64_START_ADDRESS - _COMM_PAGE32_START_ADDRESS )


#ifdef  __ASSEMBLER__

#define COMMPAGE_DESCRIPTOR_NAME(label)  _commpage_ ## label

#define COMMPAGE_DESCRIPTOR_FIELD_POINTER .quad
#define COMMPAGE_DESCRIPTOR_REFERENCE(label) \
	.quad COMMPAGE_DESCRIPTOR_NAME(label)

#define COMMPAGE_FUNCTION_START(label, codetype, alignment) \
.text								;\
.code ## codetype						;\
.align alignment, 0x90						;\
L ## label ## :

#define COMMPAGE_DESCRIPTOR(label, address)                      \
L ## label ## _end:						;\
.set L ## label ## _size, L ## label ## _end - L ## label	;\
.const_data							;\
.private_extern COMMPAGE_DESCRIPTOR_NAME(label)			;\
COMMPAGE_DESCRIPTOR_NAME(label) ## :				;\
    COMMPAGE_DESCRIPTOR_FIELD_POINTER	L ## label              ;\
    .long				L ## label ## _size	;\
    .long				address			;\
.text


/* COMMPAGE_CALL(target,from,start)
 *
 * This macro compiles a relative near call to one
 * commpage routine from another.
 * The assembler cannot handle this directly because the code
 * is not being assembled at the address at which it will execute.
 * The alternative to this macro would be to use an
 * indirect call, which is slower because the target of an
 * indirect branch is poorly predicted.
 * The macro arguments are:
 *	target = the commpage routine we are calling
 *	from   = the commpage routine we are in now
 *	start  = the label at the start of the code for this func
 * This is admitedly ugly and fragile.  Is there a better way?
 */
#define COMMPAGE_CALL(target, from, start)                        \
	COMMPAGE_CALL_INTERNAL(target,from,start,__LINE__)

#define COMMPAGE_CALL_INTERNAL(target, from, start, unique)        \
	.byte 0xe8						;\
.set UNIQUEID(unique), L ## start - . + target - from - 4	;\
	.long	UNIQUEID(unique)

#define UNIQUEID(name)  L ## name

/* COMMPAGE_JMP(target,from,start)
 *
 * This macro perform a jump to another commpage routine.
 * Used to return from the PFZ by jumping via a return outside the PFZ.
 */
#define COMMPAGE_JMP(target, from, start)                         \
	jmp      L ## start - from + target

#else /* __ASSEMBLER__ */

/* Each potential commpage routine is described by one of these.
 * Note that the COMMPAGE_DESCRIPTOR macro (above), used in
 * assembly language, must agree with this.
 */

typedef struct  commpage_descriptor     {
	void                *code_address;                      // address of code
	uint32_t            code_length;                        // length in bytes
	uint32_t            commpage_address;                   // put at this address (_COMM_PAGE_BCOPY etc)
} commpage_descriptor;


/* Warning: following structure must match the layout of the commpage.  */
/* This is the data starting at _COMM_PAGE_TIME_DATA_START, ie for nanotime() and gettimeofday() */

typedef volatile struct commpage_time_data      {
	uint64_t        nt_tsc_base;                            // _COMM_PAGE_NT_TSC_BASE
	uint32_t        nt_scale;                               // _COMM_PAGE_NT_SCALE
	uint32_t        nt_shift;                               // _COMM_PAGE_NT_SHIFT
	uint64_t        nt_ns_base;                             // _COMM_PAGE_NT_NS_BASE
	uint32_t        nt_generation;                          // _COMM_PAGE_NT_GENERATION
	uint32_t        gtod_generation;                        // _COMM_PAGE_GTOD_GENERATION
	uint64_t        gtod_ns_base;                           // _COMM_PAGE_GTOD_NS_BASE
	uint64_t        gtod_sec_base;                          // _COMM_PAGE_GTOD_SEC_BASE
} commpage_time_data;

extern  char    *commPagePtr32;                         // virt address of 32-bit commpage in kernel map
extern  char    *commPagePtr64;                         // ...and of 64-bit commpage

extern  void    commpage_set_timestamp(uint64_t abstime, uint64_t sec, uint64_t frac, uint64_t scale, uint64_t tick_per_sec);
#define commpage_disable_timestamp() commpage_set_timestamp( 0, 0, 0, 0, 0 );
extern  void    commpage_set_nanotime(uint64_t tsc_base, uint64_t ns_base, uint32_t scale, uint32_t shift);
extern  void    commpage_set_memory_pressure(unsigned int  pressure);
extern  void    commpage_set_spin_count(unsigned int  count);
extern  void    commpage_sched_gen_inc(void);
extern  void    commpage_update_active_cpus(void);
extern  void    commpage_update_mach_approximate_time(uint64_t abstime);
extern  void    commpage_update_mach_continuous_time(uint64_t sleeptime);
extern  void    commpage_update_boottime(uint64_t boottime_usec);
extern  void    commpage_update_kdebug_state(void);
extern  void    commpage_update_atm_diagnostic_config(uint32_t);
extern  void    commpage_update_dof(boolean_t enabled);
extern  void    commpage_update_dyld_flags(uint64_t value);
extern  void    commpage_post_ucode_update(void);

extern  uint32_t        commpage_is_in_pfz32(uint32_t);
extern  uint32_t        commpage_is_in_pfz64(addr64_t);

#endif  /* __ASSEMBLER__ */

#endif /* _I386_COMMPAGE_H */

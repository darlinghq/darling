/*
 * Copyright (c) 2003-2008 Apple Computer, Inc. All rights reserved.
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

#ifndef _ARM_COMMPAGE_H
#define _ARM_COMMPAGE_H

#ifndef __ASSEMBLER__
#include <stdint.h>
#endif /* __ASSEMBLER__ */

extern void     commpage_set_timestamp(uint64_t tbr, uint64_t secs, uint64_t frac, uint64_t scale, uint64_t tick_per_sec);
#define commpage_disable_timestamp() commpage_set_timestamp( 0, 0, 0, 0, 0 );
extern  void    commpage_set_memory_pressure( unsigned int pressure );
extern  void    commpage_update_active_cpus(void);
extern  void    commpage_set_spin_count(unsigned int  count);
extern  void    commpage_update_timebase(void);
extern  void    commpage_update_mach_approximate_time(uint64_t);
extern  void    commpage_update_kdebug_state(void);
extern  void    commpage_update_atm_diagnostic_config(uint32_t);
extern  void    commpage_update_mach_continuous_time(uint64_t sleeptime);
extern  void    commpage_update_multiuser_config(uint32_t);
extern  void    commpage_update_boottime(uint64_t boottime_usec);
extern  void    commpage_set_remotetime_params(double rate, uint64_t base_local_ts, uint64_t base_remote_ts);
extern uint64_t commpage_increment_cpu_quiescent_counter(void);
extern  void    commpage_update_dof(boolean_t enabled);
extern  void    commpage_update_dyld_flags(uint64_t value);

#endif /* _ARM_COMMPAGE_H */

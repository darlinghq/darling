/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

#ifndef _SYS_SFI_H_
#define _SYS_SFI_H_

#include <stdint.h>
#include <sys/types.h>
#include <mach/sfi_class.h>
/*
 * Selective Forced Idle (SFI) is a mechanism for doing
 * phase-aligned modulation of runnable tasks
 */

/* Flags for use with sfi_process_set_flags() */
#define SFI_PROCESS_SET_MANAGED		0x00000001
#define SFI_PROCESS_SET_UNMANAGED	0x00000002

#define SFI_PROCESS_SET_MANAGED_MASK	0x00000003

#ifndef KERNEL
/*
 * The system_set_sfi_window() call can be used to set "Selective
 * Forced Idle"  window for the system. sfi_window_usec is the
 * interval in microseconds and when the "Selective Forced Idle" is
 * active its "off phase" starts every sfi_window_usec.  A nonzero
 * value lower than MIN_SFI_WINDOW_USEC will be automatically changed
 * to MIN_SFI_WINDOW_USEC. The actual window size used is
 * implementation dependent and may be longer.
 *
 * system_get_sfi_window() can be used to determine the actual value.
 *
 * A value of 0 for sfi_window_usec can be used to disable "Selective
 * Forced Idle".
 */
int system_set_sfi_window(uint64_t sfi_window_usec);

int system_get_sfi_window(uint64_t *sfi_window_usec);

/*
 * sfi_set_class_offtime() can be used to set the "off time" interval
 * for all threads placed in a "Selective Forced Idle" class class_id.
 * A thread placed in class_id will not execute for offtime_usec
 * microseconds at the beginning of each "Selective Forced Idle"
 * window.  A nonzero value lower than MIN_OFFTIME_USEC will be
 * automatically changed to MIN_OFFTIME_USEC. The actual "off time"
 * used is implementation dependent and may be
 * longer. sfi_get_class_offtime() can be used to determine the actual
 * value.
 * 
 * A value of 0 for offtime_usec can be used to disable "Selective
 * Forced Idle" for all the threads placed in class_id.
 *
 * A value of offtime_usec greater than the value of system wide
 * "Selective Forced Idle" window will cause failure.
 */
int sfi_set_class_offtime(sfi_class_id_t class_id, uint64_t offtime_usec);

int sfi_get_class_offtime(sfi_class_id_t class_id, uint64_t *offtime_usec);

/*
 * The sfi_process_set_class() can be used to place all the threads in
 * a process ID pid in the "Selective Forced Idle" managed class, or clear
 * the managed state.
 */

int sfi_process_set_flags(pid_t pid, uint32_t flags);

int sfi_process_get_flags(pid_t pid, uint32_t *flags);

#endif /* !KERNEL */

#if PRIVATE

/* This is the private system call interface between Libsyscall and xnu */
#define SFI_CTL_OPERATION_SFI_SET_WINDOW	0x00000001
#define SFI_CTL_OPERATION_SFI_GET_WINDOW	0x00000002
#define SFI_CTL_OPERATION_SET_CLASS_OFFTIME	0x00000003
#define SFI_CTL_OPERATION_GET_CLASS_OFFTIME	0x00000004

#define SFI_PIDCTL_OPERATION_PID_SET_FLAGS	0x00000001
#define SFI_PIDCTL_OPERATION_PID_GET_FLAGS	0x00000002

int __sfi_ctl(uint32_t operation, uint32_t sfi_class, uint64_t time, uint64_t *out_time);
int __sfi_pidctl(uint32_t operation, pid_t pid, uint32_t sfi_flags, uint32_t *out_sfi_flags);

#endif /* PRIVATE */

#endif	/* _SYS_SFI_H_ */

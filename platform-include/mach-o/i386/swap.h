/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <architecture/byte_order.h>
#include <mach/i386/thread_status.h>

extern void swap_i386_thread_state(
    i386_thread_state_t *cpu,
    enum NXByteOrder target_byte_order);

/* current i386 thread states */
#if i386_THREAD_STATE == 1
extern void swap_i386_float_state(
    struct i386_float_state *fpu,
    enum NXByteOrder target_byte_order);

extern void swap_i386_exception_state(
    i386_exception_state_t *exc,
    enum NXByteOrder target_byte_order);
#endif /* i386_THREAD_STATE == 1 */

/* i386 thread states on older releases */
#if i386_THREAD_STATE == -1
extern void swap_i386_thread_fpstate(
    i386_thread_fpstate_t *fpu,
    enum NXByteOrder target_byte_order);

extern void swap_i386_thread_exceptstate(
    i386_thread_exceptstate_t *exc,
    enum NXByteOrder target_byte_order);

extern void swap_i386_thread_cthreadstate(
    i386_thread_cthreadstate_t *user,
    enum NXByteOrder target_byte_order);
#endif /* i386_THREAD_STATE == -1 */

#ifdef x86_THREAD_STATE64
extern void swap_x86_thread_state64(
    x86_thread_state64_t *cpu,
    enum NXByteOrder target_byte_order);

extern void swap_x86_state_hdr(
    x86_state_hdr_t *hdr,
    enum NXByteOrder target_byte_order);

extern void swap_x86_float_state64(
    x86_float_state64_t *fpu,
    enum NXByteOrder target_byte_order);

extern void swap_x86_exception_state64(
    x86_exception_state64_t *exc,
    enum NXByteOrder target_byte_order);

extern void swap_x86_thread_state(
    x86_thread_state_t *cpu,
    enum NXByteOrder target_byte_order);

extern void swap_x86_float_state(
    x86_float_state_t *fpu,
    enum NXByteOrder target_byte_order);

extern void swap_x86_exception_state(
    x86_exception_state_t *exc,
    enum NXByteOrder target_byte_order);

extern void swap_x86_debug_state32(
    x86_debug_state32_t *debug,
    enum NXByteOrder target_byte_order);

extern void swap_x86_debug_state64(
    x86_debug_state64_t *debug,
    enum NXByteOrder target_byte_order);

extern void swap_x86_debug_state(
    x86_debug_state_t *debug,
    enum NXByteOrder target_byte_order);
#endif /* x86_THREAD_STATE64 */

/*
 * Copyright (c) 2016-2019 Apple Inc. All rights reserved.
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

#ifndef KERN_BACKTRACE_H
#define KERN_BACKTRACE_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

/*!
 * @function backtrace
 *
 * @abstract backtrace the current thread's kernel stack
 *
 * @discussion Backtrace the kernel stack of the current thread, storing up
 * to btlen return addresses in bt.  Returns the number of return addresses
 * stored and sets was_truncated to true if it is non-NULL and the backtrace was
 * truncated to fit in the provided space.  The backtrace starts at the calling
 * function.  A zero will be stored after the return addresses in the buffer,
 * if space allows.
 *
 * @param bt Clients must provide a buffer in which to store the return
 * addresses.
 *
 * @param btlen Along with the buffer, its length (in terms of uintptr_t) must
 * also be provided.
 *
 * @param was_truncated Optionally, clients can provide a boolean out-parameter
 * that will be set to true if the backtrace was truncated due to a lack of
 * buffer space.
 *
 * @return The number of return addresses written to bt is returned.  The
 * function cannot return an error.
 */
unsigned int backtrace(uintptr_t *bt, unsigned int btlen, bool *was_truncated)
__attribute__((noinline));

/*!
 * @function backtrace_from
 *
 * @abstract backtrace the current thread's kernel stack from a frame pointer
 *
 * @discussion Backtrace the kernel stack of the current thread from the given
 * frame pointer startfp, storing up to btlen return addresses in bt.  Returns
 * the number of return addresses written and sets trunc to true if trunc is
 * non-NULL and the backtrace was truncated to fit in the provided space.  The
 * frame pointer provided must point to a valid frame on the current thread's
 * stack.
 *
 * @param startfp The frame pointer to start backtracing from is required, and
 * must be point to a valid frame on the current thread's stack.
 *
 * @seealso backtrace
 */
unsigned int backtrace_frame(uintptr_t *bt, unsigned int btlen, void *startfp,
    bool *was_truncated)
__attribute__((noinline, not_tail_called));

/*!
 * @function backtrace_interrupted
 *
 * @abstract backtrace the interrupted context
 *
 * @discussion Backtrace the kernel stack of the interrupted thread, storing up
 * to btlen return addresses in bt.  This function must be called from interrupt
 * context.
 *
 * @seealso backtrace
 */
unsigned int backtrace_interrupted(uintptr_t *bt, unsigned int btlen,
    bool *was_truncated);

/*!
 * @function backtrace_user
 *
 * @abstract backtrace the current thread's user space stack
 *
 * @discussion Backtrace the user stack of the current thread, storing up to
 * btlen return addresses in bt.  This function cannot be called on a kernel
 * thread, nor can it be called from interrupt context or with interrupts
 * disabled.
 *
 * @param btwritten On success, the number of return addresses written is stored
 * here.
 *
 * @param user64 On success, true is stored here if user space was running in
 * 64-bit mode, and false is stored otherwise.
 *
 * @return Returns 0 on success and an errno value on error.
 *
 * @seealso backtrace
 */
int backtrace_user(uintptr_t *bt, unsigned int btlen, unsigned int *btwritten,
    bool *user64, bool *was_truncated);

/*
 * @function backtrace_thread_user
 *
 * @abstract backtrace a given thread's user space stack
 *
 * @discussion Backtrace the user stack of the given thread, storing up to btlen
 * return addresses in bt.  This function cannot be called on a kernel thread,
 * nor can it be called from interrupt context or with interrupts disabled.
 *
 * @param thread The user thread to backtrace is required.
 *
 * @see backtrace_user
 */
int backtrace_thread_user(void *thread, uintptr_t *bt, unsigned int btlen,
    unsigned int *btwritten, bool *user64, bool *was_truncated);

__END_DECLS

#endif /* !defined(KERN_BACKTRACE_H) */

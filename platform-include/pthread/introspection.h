/*
 * Copyright (c) 2013, 2016 Apple Inc. All rights reserved.
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

#ifndef __PTHREAD_INTROSPECTION__
#define __PTHREAD_INTROSPECTION__

#include <sys/cdefs.h>
#include <pthread/pthread.h>
#include <Availability.h>

/*!
 * @header
 *
 * @abstract
 * Introspection API for libpthread.
 *
 * This should only be used for introspection and debugging tools.  Do not rely
 * on it in shipping code.
 */

__BEGIN_DECLS

/*!
 * @typedef pthread_introspection_hook_t
 *
 * @abstract
 * A function pointer called at various points in a PThread's lifetime.  The
 * function must be able to be called in contexts with invalid thread state.
 *
 * @param event
 * One of the events in pthread_introspection_event_t.
 *
 * @param thread
 * pthread_t associated with the event.
 *
 * @param addr
 * Address associated with the event, e.g. stack address.
 *
 * @param size
 * Size associated with the event, e.g. stack size.
 */
typedef void (*pthread_introspection_hook_t)(unsigned int event,
		pthread_t thread, void *addr, size_t size);

/*!
 * @enum pthread_introspection_event_t
 *
 * @constant PTHREAD_INTROSPECTION_THREAD_CREATE
 * pthread_t was created.
 *
 * @constant PTHREAD_INTROSPECTION_THREAD_START
 * Thread has started and stack was allocated.
 *
 * @constant PTHREAD_INTROSPECTION_THREAD_TERMINATE
 * Thread is about to be terminated and stack will be deallocated.
 *
 * @constant PTHREAD_INTROSPECTION_THREAD_DESTROY
 * pthread_t is about to be destroyed.
 */
enum {
	PTHREAD_INTROSPECTION_THREAD_CREATE = 1,
	PTHREAD_INTROSPECTION_THREAD_START,
	PTHREAD_INTROSPECTION_THREAD_TERMINATE,
	PTHREAD_INTROSPECTION_THREAD_DESTROY,
};

/*!
 * @function pthread_introspection_hook_install
 *
 * @abstract
 * Install introspection hook function into libpthread.
 *
 * @discussion
 * The caller is responsible for implementing chaining to the hook that was
 * previously installed (if any).
 *
 * @param hook
 * Pointer to hook function.
 *
 * @result
 * Previously installed hook function or NULL.
 */

__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0)
__attribute__((__nonnull__, __warn_unused_result__))
extern pthread_introspection_hook_t
pthread_introspection_hook_install(pthread_introspection_hook_t hook);

__END_DECLS

#endif

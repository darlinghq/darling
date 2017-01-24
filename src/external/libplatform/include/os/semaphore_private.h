/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
 *
 * @APPLE_APACHE_LICENSE_HEADER_START@
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @APPLE_APACHE_LICENSE_HEADER_END@
 */

#ifndef __OS_SEMAPHORE_PRIVATE__
#define __OS_SEMAPHORE_PRIVATE__

#include <Availability.h>
#include <stdint.h>
#include <os/base_private.h>
#include <os/tsd.h>

OS_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

#define OS_SEMAPHORE_SPI_VERSION 20130313

typedef uintptr_t os_semaphore_t;

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_WARN_RESULT OS_NOTHROW
os_semaphore_t _os_semaphore_create(void);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_NOTHROW
void _os_semaphore_dispose(os_semaphore_t);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_NOTHROW
void _os_semaphore_wait(os_semaphore_t);

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_NOTHROW
void _os_semaphore_signal(os_semaphore_t);

OS_WARN_RESULT OS_NOTHROW
__header_always_inline os_semaphore_t
os_get_cached_semaphore(void)
{
	os_semaphore_t sema;
	sema = (os_semaphore_t)_os_tsd_get_direct(__TSD_SEMAPHORE_CACHE);
	if (os_unlikely(!sema)) {
		return _os_semaphore_create();
	}
	_os_tsd_set_direct(__TSD_SEMAPHORE_CACHE, 0);
	return sema;
}

OS_NOTHROW
__header_always_inline void
os_put_cached_semaphore(os_semaphore_t sema)
{
	os_semaphore_t old_sema;
	old_sema = (os_semaphore_t)_os_tsd_get_direct(__TSD_SEMAPHORE_CACHE);
	_os_tsd_set_direct(__TSD_SEMAPHORE_CACHE, (void*)sema);
	if (os_unlikely(old_sema)) {
		return _os_semaphore_dispose(old_sema);
	}
}

OS_NOTHROW
__header_always_inline void
os_semaphore_wait(os_semaphore_t sema)
{
	return _os_semaphore_wait(sema);
}

OS_NOTHROW
__header_always_inline void
os_semaphore_signal(os_semaphore_t sema)
{
	return _os_semaphore_signal(sema);
}

__END_DECLS

OS_ASSUME_NONNULL_END

#endif // __OS_SEMAPHORE_PRIVATE__

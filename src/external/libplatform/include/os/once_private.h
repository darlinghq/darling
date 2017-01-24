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

#ifndef __OS_ONCE_PRIVATE__
#define __OS_ONCE_PRIVATE__

#include <Availability.h>
#include <os/base_private.h>

OS_ASSUME_NONNULL_BEGIN

__BEGIN_DECLS

#define OS_ONCE_SPI_VERSION 20130313

OS_SWIFT_UNAVAILABLE("Swift has lazy init")
typedef long os_once_t;

__OSX_AVAILABLE_STARTING(__MAC_10_9,__IPHONE_7_0)
OS_EXPORT OS_NONNULL1 OS_NONNULL3 OS_NOTHROW
OS_SWIFT_UNAVAILABLE("Swift has lazy init")
void
_os_once(os_once_t *predicate, void *_Nullable context, os_function_t function);

OS_NONNULL1 OS_NONNULL3 OS_NOTHROW
__header_always_inline void
os_once(os_once_t *predicate, void *_Nullable context, os_function_t function)
{
	if (OS_EXPECT(*predicate, ~0l) != ~0l) {
		_os_once(predicate, context, function);
		OS_COMPILER_CAN_ASSUME(*predicate == ~0l);
	} else {
		os_compiler_barrier();
	}
}

/* This SPI is *strictly* for the use of pthread_once only. This is not
 * safe in general use of os_once.
 */
__OSX_AVAILABLE_STARTING(__MAC_10_9, __IPHONE_7_0)
OS_EXPORT OS_NONNULL1 OS_NOTHROW
OS_SWIFT_UNAVAILABLE("Swift has lazy init")
void
__os_once_reset(os_once_t *val);

__END_DECLS

OS_ASSUME_NONNULL_END

#endif // __OS_ONCE_PRIVATE__

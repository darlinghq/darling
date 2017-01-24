/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
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

/*
 * IMPORTANT: This header file describes INTERNAL interfaces to libplatform
 * which are subject to change in future releases of Mac OS X. Any applications
 * relying on these interfaces WILL break.
 */

#ifndef __OS_YIELD__
#define __OS_YIELD__

#include <TargetConditionals.h>

#pragma mark -
#pragma mark _os_wait_until

#if OS_ATOMIC_UP
#define _os_wait_until(c) do { \
		int _spins = 0; \
		while (unlikely(!(c))) { \
			_spins++; \
			_os_preemption_yield(_spins); \
		} } while (0)
#elif TARGET_OS_EMBEDDED
// <rdar://problem/15508918>
#ifndef OS_WAIT_SPINS
#define OS_WAIT_SPINS 1024
#endif
#define _os_wait_until(c) do { \
		int _spins = -(OS_WAIT_SPINS); \
		while (unlikely(!(c))) { \
			if (unlikely(_spins++ >= 0)) { \
				_os_preemption_yield(_spins); \
			} else { \
				os_hardware_pause(); \
			} \
		} } while (0)
#else
#define _os_wait_until(c) do { \
		while (!(c)) { \
			os_hardware_pause(); \
		} } while (0)
#endif

#pragma mark -
#pragma mark os_hardware_pause

#if defined(__x86_64__) || defined(__i386__)
#define os_hardware_pause() __asm__("pause")
#elif (defined(__arm__) && defined(_ARM_ARCH_7) && defined(__thumb__)) || \
		defined(__arm64__)
#define os_hardware_pause() __asm__("yield")
#define os_hardware_wfe()   __asm__("wfe")
#else
#define os_hardware_pause() __asm__("")
#endif

#pragma mark -
#pragma mark _os_preemption_yield

#if defined(SWITCH_OPTION_OSLOCK_DEPRESS) && !(TARGET_IPHONE_SIMULATOR && \
		IPHONE_SIMULATOR_HOST_MIN_VERSION_REQUIRED < 1090)
#define OS_YIELD_THREAD_SWITCH_OPTION SWITCH_OPTION_OSLOCK_DEPRESS
#else
#define OS_YIELD_THREAD_SWITCH_OPTION SWITCH_OPTION_DEPRESS
#endif
#define _os_preemption_yield(n) thread_switch(MACH_PORT_NULL, \
		OS_YIELD_THREAD_SWITCH_OPTION, (mach_msg_timeout_t)(n))

#endif // __OS_YIELD__

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

#include "lock_internal.h"

#ifdef OS_VARIANT_SELECTOR

#if TARGET_OS_EMBEDDED
OS_VARIANT_UPMP_RESOLVER(OSAtomicAdd32Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicIncrement32Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicDecrement32Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicAdd64Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicIncrement64Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicDecrement64Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicAnd32Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicAnd32OrigBarrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicOr32Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicOr32OrigBarrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicXor32Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicXor32OrigBarrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicCompareAndSwap32Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicCompareAndSwap64Barrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicCompareAndSwapIntBarrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicCompareAndSwapLongBarrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicCompareAndSwapPtrBarrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicTestAndSetBarrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicTestAndClearBarrier)
OS_VARIANT_UPMP_RESOLVER(OSAtomicEnqueue)
OS_VARIANT_UPMP_RESOLVER(OSAtomicDequeue)
OS_VARIANT_UPMP_RESOLVER(OSMemoryBarrier)

OS_VARIANT_UPMP_RESOLVER(OSSpinLockLock)
OS_VARIANT_UPMP_RESOLVER(OSSpinLockTry)
OS_VARIANT_UPMP_RESOLVER(OSSpinLockUnlock)
OS_VARIANT_UPMP_RESOLVER(spin_lock)
OS_VARIANT_UPMP_RESOLVER(spin_lock_try)
OS_VARIANT_UPMP_RESOLVER(spin_unlock)
OS_VARIANT_UPMP_RESOLVER(_spin_lock)
OS_VARIANT_UPMP_RESOLVER(_spin_lock_try)
OS_VARIANT_UPMP_RESOLVER(_spin_unlock)

OS_VARIANT_UPMP_RESOLVER(os_unfair_lock_lock)
OS_VARIANT_UPMP_RESOLVER(os_unfair_lock_lock_with_options)
OS_VARIANT_UPMP_RESOLVER(os_unfair_lock_trylock)
OS_VARIANT_UPMP_RESOLVER(os_unfair_lock_unlock)
OS_VARIANT_UPMP_RESOLVER(os_unfair_lock_lock_no_tsd_4libpthread)
OS_VARIANT_UPMP_RESOLVER(os_unfair_lock_unlock_no_tsd_4libpthread)
OS_VARIANT_UPMP_RESOLVER(os_unfair_lock_lock_with_options_4Libc)
OS_VARIANT_UPMP_RESOLVER(os_unfair_lock_unlock_4Libc)
OS_VARIANT_UPMP_RESOLVER(_os_nospin_lock_lock)
OS_VARIANT_UPMP_RESOLVER(_os_nospin_lock_trylock)
OS_VARIANT_UPMP_RESOLVER(_os_nospin_lock_unlock)

OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_spin_lock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_spin_trylock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_spin_unlock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_handoff_lock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_handoff_trylock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_handoff_unlock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_unfair_lock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_unfair_trylock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_unfair_unlock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_nospin_lock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_nospin_trylock)
OS_VARIANT_UPMP_RESOLVER_INTERNAL(_os_lock_nospin_unlock)

OS_VARIANT_UPMP_RESOLVER(_os_once)
OS_VARIANT_UPMP_RESOLVER(__os_once_reset)
#endif // TARGET_OS_EMBEDDED

#endif // OS_VARIANT_SELECTOR

#ifdef OS_LOCK_VARIANT_SELECTOR

#define WFE_RESOLVER(s) \
	_OS_VARIANT_RESOLVER(s, hidden, \
		uint32_t *_c = (void*)(uintptr_t)_COMM_PAGE_CPU_CAPABILITIES; \
		if (*_c & kHasEvent) { \
			extern void OS_VARIANT(s, wfe)(void); \
			return &OS_VARIANT(s, wfe); \
		} else { \
			extern void OS_VARIANT(s, mp)(void); \
			return &OS_VARIANT(s, mp); \
		})

WFE_RESOLVER(_OSSpinLockLockSlow)

#endif // OS_LOCK_VARIANT_SELECTOR


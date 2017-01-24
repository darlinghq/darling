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

#ifndef __OS_RESOLVER_H__
#define __OS_RESOLVER_H__

#include <TargetConditionals.h>

#ifdef OS_VARIANT_SELECTOR

#if TARGET_OS_EMBEDDED
#define OSAtomicAdd32Barrier \
		OS_VARIANT(OSAtomicAdd32Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicIncrement32Barrier \
		OS_VARIANT(OSAtomicIncrement32Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicDecrement32Barrier \
		OS_VARIANT(OSAtomicDecrement32Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicAdd64Barrier \
		OS_VARIANT(OSAtomicAdd64Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicIncrement64Barrier \
		OS_VARIANT(OSAtomicIncrement64Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicDecrement64Barrier \
		OS_VARIANT(OSAtomicDecrement64Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicAnd32Barrier \
		OS_VARIANT(OSAtomicAnd32Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicAnd32OrigBarrier \
		OS_VARIANT(OSAtomicAnd32OrigBarrier, OS_VARIANT_SELECTOR)
#define OSAtomicOr32Barrier \
		OS_VARIANT(OSAtomicOr32Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicOr32OrigBarrier \
		OS_VARIANT(OSAtomicOr32OrigBarrier, OS_VARIANT_SELECTOR)
#define OSAtomicXor32Barrier \
		OS_VARIANT(OSAtomicXor32Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicXor32OrigBarrier \
		OS_VARIANT(OSAtomicXor32OrigBarrier, OS_VARIANT_SELECTOR)
#define OSAtomicCompareAndSwap32Barrier \
		OS_VARIANT(OSAtomicCompareAndSwap32Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicCompareAndSwap64Barrier \
		OS_VARIANT(OSAtomicCompareAndSwap64Barrier, OS_VARIANT_SELECTOR)
#define OSAtomicCompareAndSwapIntBarrier \
		OS_VARIANT(OSAtomicCompareAndSwapIntBarrier, OS_VARIANT_SELECTOR)
#define OSAtomicCompareAndSwapLongBarrier \
		OS_VARIANT(OSAtomicCompareAndSwapLongBarrier, OS_VARIANT_SELECTOR)
#define OSAtomicCompareAndSwapPtrBarrier \
		OS_VARIANT(OSAtomicCompareAndSwapPtrBarrier, OS_VARIANT_SELECTOR)
#define OSAtomicTestAndSetBarrier \
		OS_VARIANT(OSAtomicTestAndSetBarrier, OS_VARIANT_SELECTOR)
#define OSAtomicTestAndClearBarrier \
		OS_VARIANT(OSAtomicTestAndClearBarrier, OS_VARIANT_SELECTOR)
#define OSAtomicEnqueue \
		OS_VARIANT(OSAtomicEnqueue, OS_VARIANT_SELECTOR)
#define OSAtomicDequeue \
		OS_VARIANT(OSAtomicDequeue, OS_VARIANT_SELECTOR)
#define OSMemoryBarrier \
		OS_VARIANT(OSMemoryBarrier, OS_VARIANT_SELECTOR)

#define OSSpinLockLock \
		OS_VARIANT(OSSpinLockLock, OS_VARIANT_SELECTOR)
#define OSSpinLockTry \
		OS_VARIANT(OSSpinLockTry, OS_VARIANT_SELECTOR)
#define OSSpinLockUnlock \
		OS_VARIANT(OSSpinLockUnlock, OS_VARIANT_SELECTOR)
#define spin_lock \
		OS_VARIANT(spin_lock, OS_VARIANT_SELECTOR)
#define spin_lock_try \
		OS_VARIANT(spin_lock_try, OS_VARIANT_SELECTOR)
#define spin_unlock \
		OS_VARIANT(spin_unlock, OS_VARIANT_SELECTOR)
#define _spin_lock \
		OS_VARIANT(_spin_lock, OS_VARIANT_SELECTOR)
#define _spin_lock_try \
		OS_VARIANT(_spin_lock_try, OS_VARIANT_SELECTOR)
#define _spin_unlock \
		OS_VARIANT(_spin_unlock, OS_VARIANT_SELECTOR)
#define os_unfair_lock_lock \
		OS_VARIANT(os_unfair_lock_lock, OS_VARIANT_SELECTOR)
#define os_unfair_lock_lock_with_options \
		OS_VARIANT(os_unfair_lock_lock_with_options, OS_VARIANT_SELECTOR)
#define os_unfair_lock_trylock \
		OS_VARIANT(os_unfair_lock_trylock, OS_VARIANT_SELECTOR)
#define os_unfair_lock_unlock \
		OS_VARIANT(os_unfair_lock_unlock, OS_VARIANT_SELECTOR)
#define os_unfair_lock_lock_no_tsd_4libpthread \
		OS_VARIANT(os_unfair_lock_lock_no_tsd_4libpthread, OS_VARIANT_SELECTOR)
#define os_unfair_lock_unlock_no_tsd_4libpthread \
		OS_VARIANT(os_unfair_lock_unlock_no_tsd_4libpthread, OS_VARIANT_SELECTOR)
#define os_unfair_lock_lock_with_options_4Libc \
		OS_VARIANT(os_unfair_lock_lock_with_options_4Libc, OS_VARIANT_SELECTOR)
#define os_unfair_lock_unlock_4Libc \
		OS_VARIANT(os_unfair_lock_unlock_4Libc, OS_VARIANT_SELECTOR)
#define _os_nospin_lock_lock \
		OS_VARIANT(_os_nospin_lock_lock, OS_VARIANT_SELECTOR)
#define _os_nospin_lock_trylock \
		OS_VARIANT(_os_nospin_lock_trylock, OS_VARIANT_SELECTOR)
#define _os_nospin_lock_unlock \
		OS_VARIANT(_os_nospin_lock_unlock, OS_VARIANT_SELECTOR)
#define _os_once \
		OS_VARIANT(_os_once, OS_VARIANT_SELECTOR)
#define __os_once_reset \
		OS_VARIANT(__os_once_reset, OS_VARIANT_SELECTOR)
#endif // TARGET_OS_EMBEDDED
#endif // OS_VARIANT_SELECTOR

#ifdef OS_LOCK_VARIANT_SELECTOR
#define _OSSpinLockLockSlow \
		OS_VARIANT(_OSSpinLockLockSlow, OS_LOCK_VARIANT_SELECTOR)
#endif // OS_LOCK_VARIANT_SELECTOR

#endif // __OS_RESOLVER_H__

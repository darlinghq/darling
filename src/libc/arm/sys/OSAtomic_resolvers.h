/*
 * Copyright (c) 2010 Apple Inc. All rights reserved.
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

/*
 * This header file gets included by OSAtomic_resolvers.c and OSAtomic.s
 * in order to create the appropriate resolvers for both libsystem_c.dylib
 * and dyld's libc.a
 */

makeResolver_up_mp(OSAtomicAdd32Barrier)
makeResolver_up_mp(OSAtomicOr32Barrier)
makeResolver_up_mp(OSAtomicOr32OrigBarrier)
makeResolver_up_mp(OSAtomicAnd32Barrier)
makeResolver_up_mp(OSAtomicAnd32OrigBarrier)
makeResolver_up_mp(OSAtomicXor32Barrier)
makeResolver_up_mp(OSAtomicXor32OrigBarrier)
makeResolver_up_mp(OSAtomicCompareAndSwap32Barrier)
makeResolver_up_mp(OSAtomicCompareAndSwapIntBarrier)
makeResolver_up_mp(OSAtomicCompareAndSwapLongBarrier)
makeResolver_up_mp(OSAtomicCompareAndSwapPtrBarrier)
makeResolver_up_mp(OSAtomicTestAndSetBarrier)
makeResolver_up_mp(OSAtomicTestAndClearBarrier)
makeResolver_up_mp(OSMemoryBarrier)
makeResolver_up_mp(OSAtomicEnqueue)
makeResolver_up_mp(OSAtomicDequeue)
makeResolver_up_mp(OSAtomicAdd64Barrier)
makeResolver_up_mp(OSAtomicCompareAndSwap64Barrier)

makeResolver_up_mp_wfe(spin_lock)
makeResolver_up_mp_wfe(_spin_lock)
makeResolver_up_mp_wfe(OSSpinLockLock)

makeResolver_up_mp(spin_lock_try)
makeResolver_up_mp(_spin_lock_try)
makeResolver_up_mp(OSSpinLockTry)

makeResolver_up_mp(spin_unlock)
makeResolver_up_mp(_spin_unlock)
makeResolver_up_mp(OSSpinLockUnlock)

/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2004-2012 Apple Inc. All rights reserved.
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


#ifndef __DYLDLOCK__
#define __DYLDLOCK__

//
// This file contains the synchronization utilities used to make dyld APIs be thread safe.
//
// The implementation of all dyld API's must hold acquire global lock (in libSystem)
// before calling into dyld proper, and release the lock after returning from dyld.
// This is done using the macros DYLD_LOCK_THIS_BLOCK.
// Example:
//
//  void dyld_api() {
//		DYLD_LOCK_THIS_BLOCK;
//		// free to do stuff here 
//		// that accesses dyld internal data structures
//	}
//
//  void dyld_api_state_free() {
//		DYLD_NO_LOCK_THIS_BLOCK
//		// can only do stuff here 
//		// that does not require locking
//	}
//

#define DYLD_LOCK_THIS_BLOCK			LockHelper _dyld_lock;
#define DYLD_NO_LOCK_THIS_BLOCK

// used by dyld wrapper functions in libSystem
class __attribute__((visibility("hidden"))) LockHelper
{
public:
	LockHelper();
	~LockHelper();
};


// to initialize
extern void dyldGlobalLockInitialize()		__attribute__((visibility("hidden")));
extern void dyldGlobalLockAcquire()			__attribute__((visibility("hidden")));
extern void dyldGlobalLockRelease()			__attribute__((visibility("hidden")));

#endif // __DYLDLOCK__


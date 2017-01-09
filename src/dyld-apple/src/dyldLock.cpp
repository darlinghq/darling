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

#include <pthread.h>

#include "dyldLock.h"



static pthread_mutex_t	sGlobalMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;

// <rdar://problem/6361143> Need a way to determine if a gdb call to dlopen() would block
int	__attribute__((visibility("hidden")))			_dyld_global_lock_held = 0;


LockHelper::LockHelper() 
{ 
	dyldGlobalLockAcquire();
}

LockHelper::~LockHelper() 
{ 
	dyldGlobalLockRelease();
}

void dyldGlobalLockAcquire() 
{
	pthread_mutex_lock(&sGlobalMutex);
	++_dyld_global_lock_held;
}

void dyldGlobalLockRelease() 
{
	--_dyld_global_lock_held;
	pthread_mutex_unlock(&sGlobalMutex);
}



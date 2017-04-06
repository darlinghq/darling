/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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
#include <stdio.h>  // fprintf(), NULL
#include <stdlib.h> // exit(), EXIT_SUCCESS
#include <string.h>
#include <dlfcn.h>
#include <mach-o/dyld_priv.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

typedef int (*FooProc)();

int main()
{
	void* handle = dlopen("libfoo.dylib", RTLD_LAZY);
	if ( handle == NULL ) {
		FAIL("interpose-dynamic-dlsym: dlopen() error: %s", dlerror());
		return 0;
	}
	FooProc dlsym_foo = (FooProc)dlsym(handle, "foo");
	if ( dlsym_foo == NULL ) {
		FAIL("interpose-dynamic-dlsym: dlsym() error: %s", dlerror());
		return 0;
	}

  int result = (*dlsym_foo)();
  if ( result == 10 )
      PASS("interpose-dynamic-dlsym");
  else
      FAIL("interpose-dynamic-dlsym");
  
	return 0;
}

/*
 * Copyright (c) 2005 Apple Computer, Inc. All rights reserved.
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
#include <dlfcn.h>
#include <string.h>

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()




static void* openWithModeGetSymbol(const char* path, int mode, const char* symbol)
{
	void* handle = dlopen(path, mode);
	if ( handle == NULL ) {
		const char* msg = dlerror();
		if ( ((mode & RTLD_LOCAL) != 0) && (strstr(msg, "RTLD_LOCAL") != NULL) )
			XFAIL("dlopen(\"%s\") failed: %s", path, msg);
		else
			FAIL("dlopen(\"%s\") failed: %s", path, msg);
		exit(0);
	}
	
	void* sym = dlsym(handle, symbol);
	if ( sym == NULL ) {
		FAIL("dlsym(handle, \"%s\") failed", symbol);
		exit(0);
	}
	return sym;
}

static void trySO(const char* path) 
{
#if LOCAL_FIRST
	void* symLocal = openWithModeGetSymbol(path, RTLD_LOCAL, "foo");
	void* symGlobal = openWithModeGetSymbol(path, RTLD_GLOBAL, "foo");
#else
	void* symGlobal = openWithModeGetSymbol(path, RTLD_GLOBAL, "foo");
	void* symLocal = openWithModeGetSymbol(path, RTLD_LOCAL, "foo");
#endif
	if ( symLocal != symGlobal ) {
		FAIL("global load after local load failed");
		exit(0);
	}
}


int main(int argc, const char* argv[])
{
	trySO("foo.bundle");
 	trySO("foo.dylib");
  
	PASS("dlopen-local-and-global");
	return EXIT_SUCCESS;
}

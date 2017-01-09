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

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()


///
/// When dlopen(NULL, RTLD_FIRST) is called, 
/// any dlsym() looks against that handle should only look in 
/// the main executable, and not subsequent images.
///


struct info
{
	const char* path;
	void*		handle;
};
typedef struct info info;

static info dlopen_or_fail(const char* path, int options)
{
	info result;
	result.path = path;
	result.handle = dlopen(path, options);
	if ( result.handle == NULL ) {
		FAIL("dlsym-NULL-RTLD_FIRST: dlopen(\"%s\") failed: %s", path, dlerror());
		exit(0);
	}
	//fprintf(stderr, "dlopen(%s, 0x%0X) => %p\n", path, options, result.handle);
	return result;
}

static void dlsym_should_fail(info hp, const char* symbol)
{
	void* sym = dlsym(hp.handle, symbol);
	if ( sym != NULL ) {
		FAIL("dlsym-NULL-RTLD_FIRST: dlsym(handle-%s, \"%s\") should have failed", hp.path, symbol);
		exit(0);
	}
}

static void dlsym_should_succeed(info hp, const char* symbol)
{
	void* sym = dlsym(hp.handle, symbol);
	if ( sym == NULL ) {
		FAIL("dlsym-NULL-RTLD_FIRST: dlsym(handle-%s, \"%s\") failed", hp.path, symbol);
		exit(0);
	}
}


int main()
{
	int result;
	info mainFirst		= dlopen_or_fail(NULL, RTLD_FIRST);
	info mainDefault	= dlopen_or_fail(NULL, 0);
		
	dlsym_should_succeed(mainFirst, "main_foo");
	dlsym_should_fail(mainFirst, "foo");
	
	dlsym_should_succeed(mainDefault, "main_foo");
	dlsym_should_succeed(mainDefault, "foo");
	
	result = dlclose(mainFirst.handle);
	if ( result != 0 ) {
		FAIL("dlsym-NULL-RTLD_FIRST: dlclose(mainFirst.handle) failed: %s", dlerror());
		exit(0);
	}
	result = dlclose(mainDefault.handle);
	if ( result != 0 ) {
		FAIL("dlsym-NULL-RTLD_FIRST: dlclose(mainDefault.handle) failed: %s", dlerror());
		exit(0);
	}

	PASS("dlsym-NULL-RTLD_FIRST");
	return EXIT_SUCCESS;
}


void main_foo() {}



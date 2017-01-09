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
/// When dlopen() is called with the RTLD_FIRST option,
/// any dlsym() looks against that handle should only look in 
/// that handle, and not subsequent images.
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
		FAIL("dlopen-RTLD_FIRST: dlopen(\"%s\") failed: %s", path, dlerror());
		exit(0);
	}
	//fprintf(stderr, "dlopen(%s, 0x%0X) => %p\n", path, options, result.handle);
	return result;
}

static void dlsym_should_fail(info hp, const char* symbol)
{
	void* sym = dlsym(hp.handle, symbol);
	if ( sym != NULL ) {
		FAIL("dlopen-RTLD_FIRST: dlsym(handle-%s, \"%s\") should have failed", hp.path, symbol);
		exit(0);
	}
}

static void dlsym_should_succeed(info hp, const char* symbol)
{
	void* sym = dlsym(hp.handle, symbol);
	if ( sym == NULL ) {
		FAIL("dlopen-RTLD_FIRST: dlsym(handle-%s, \"%s\") failed", hp.path, symbol);
		exit(0);
	}
}


int main()
{
	info libFooFirst = dlopen_or_fail("libfoo.dylib", RTLD_FIRST);
	info libFoo      = dlopen_or_fail("libfoo.dylib", 0);
	
	info libBarFirst = dlopen_or_fail("libbar.dylib", RTLD_FIRST);
	info libBar      = dlopen_or_fail("libbar.dylib", 0);
	
	dlsym_should_succeed(libFooFirst, "foo");
	dlsym_should_fail(libFooFirst, "base");
	dlsym_should_fail(libFooFirst, "bar");
	
	dlsym_should_succeed(libFoo, "foo");
	dlsym_should_succeed(libFoo, "base");
			
	dlsym_should_succeed(libBarFirst, "bar");
	dlsym_should_succeed(libBarFirst, "base");	// libbar re-exports libbase
	dlsym_should_fail(libBarFirst, "foo");
	
	dlsym_should_succeed(libBar, "bar");
	dlsym_should_succeed(libBar, "base");



	info bundleFooFirst = dlopen_or_fail("foo.bundle", RTLD_FIRST);
	info bundleFoo      = dlopen_or_fail("foo.bundle", 0);
	
	info bundleBarFirst = dlopen_or_fail("bar.bundle", RTLD_FIRST);
	info bundleBar      = dlopen_or_fail("bar.bundle", 0);
	
	dlsym_should_succeed(bundleFooFirst, "foo");
	dlsym_should_fail(bundleFooFirst, "base");
	dlsym_should_fail(bundleFooFirst, "bar");
	
	dlsym_should_succeed(bundleFoo, "foo");
	dlsym_should_succeed(bundleFoo, "base");
			
	dlsym_should_succeed(bundleBarFirst, "bar");
	dlsym_should_fail(bundleBarFirst, "base");
	dlsym_should_fail(bundleBarFirst, "foo");
	
	dlsym_should_succeed(bundleBar, "bar");
	dlsym_should_succeed(bundleBar, "base");


	PASS("dlsym-RTLD_FIRST bundle and dylib");
	return EXIT_SUCCESS;
}

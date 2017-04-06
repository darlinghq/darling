/*
 * Copyright (c) 2007-2010 Apple Inc. All rights reserved.
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
#include <stdbool.h>
#include <string.h>
#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

#include <_simple.h>

///
/// verify parameters passed to initializer are same as passed to main()
/// verify that apple[0] parameter is correct by comparing to argv[1]
///

static int			initializer_argc = 0;
static const char**	initializer_argv = NULL;
static const char**	initializer_env = NULL;
static const char**	initializer_apple = NULL;


__attribute__((constructor))
void init(int argc, const char* argv[], const char* env[], const char* apple[])
{
	initializer_argc = argc;
	initializer_argv = argv;
	initializer_env = env;
	initializer_apple = apple;
}

int
main(int argc, const char* argv[], const char* env[], const char* apple[])
{
	if ( argc != initializer_argc ) {
		FAIL("crt-apple argc changed");
		exit(EXIT_SUCCESS);
	}
	
	if ( argv != initializer_argv ) {
		FAIL("crt-apple argv changed");
		exit(EXIT_SUCCESS);
	}
		
	if ( env != initializer_env ) {
		FAIL("crt-apple envp changed");
		exit(EXIT_SUCCESS);
	}
		
	if ( apple != initializer_apple ) {
		FAIL("crt-apple apple changed");
		exit(EXIT_SUCCESS);
	}
		
	const char* execPath = _simple_getenv(apple, "executable_path");
    if ( execPath == NULL )
		FAIL("crt-apple apple[] missing executable_path=");
	else if ( strcmp(execPath, argv[1]) == 0 )
		PASS("crt-apple %s", execPath);
	else
		FAIL("crt-apple %s", execPath);
		
	return EXIT_SUCCESS;
}


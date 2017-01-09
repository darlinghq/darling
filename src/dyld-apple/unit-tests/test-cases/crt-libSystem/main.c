/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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
#include <crt_externs.h>
#include <mach-o/ldsyms.h>
#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

struct ProgramVars
{
	const void*		mh;
	int*			NXArgcPtr;
	char***			NXArgvPtr;
	char***			environPtr;
	char**			__prognamePtr;
};
static const struct ProgramVars* sVars;

// global variables defeined in crt1.o
extern char** NXArgv;
extern int NXArgc;
extern char** environ;
extern char* __progname;


int
main(int argc, const char* argv[])
{
	bool success = true;

	if ( _NSGetArgv() != &NXArgv ) {
		FAIL("crt-libSystem _NSGetArgv() != &NXArgv (%p!=%p) for %s", _NSGetArgv(), &NXArgv, argv[0]);
		success = false;
	}
	
	if ( _NSGetArgc() != &NXArgc ) {
		FAIL("crt-libSystem _NSGetArgc() != &NXArgc (%p!=%p) for %s", _NSGetArgc(), &NXArgc, argv[0]);
		success = false;
	}
	
	if ( _NSGetEnviron() != &environ ) {
		FAIL("crt-libSystem _NSGetEnviron() != &environv (%p!=%p) for %s", _NSGetEnviron(), &environ, argv[0]);
		success = false;
	}
	
	if ( _NSGetProgname() != &__progname ) {
		FAIL("crt-libSystem _NSGetProgname() != &__progname (%p!=%p) for %s", _NSGetProgname(), &__progname, argv[0]);
		success = false;
	}
	
	if ( _NSGetMachExecuteHeader() != &_mh_execute_header ) {
		FAIL("crt-libSystem _NSGetMachExecuteHeader() != &_mh_execute_headerv (%p!=%p) for %s", _NSGetMachExecuteHeader(), &_mh_execute_header, argv[0]);
		success = false;
	}
	
	if ( sVars->NXArgvPtr != &NXArgv ) {
		FAIL("crt-libSystem sVars->NXArgvPtr != &NXArg (%p!=%p) for %s", sVars->NXArgvPtr, &NXArgv, argv[0]);
		success = false;
	}

	if ( sVars->NXArgcPtr != &NXArgc ) {
		FAIL("crt-libSystem sVars->NXArgcPtr != &NXArgc (%p!=%p) for %s", sVars->NXArgcPtr, &NXArgc, argv[0]);
		success = false;
	}
	
	if ( sVars->environPtr != &environ ) {
		FAIL("crt-libSystem sVars->environPtr != &environ (%p!=%p) for %s", sVars->environPtr, &environ, argv[0]);
		success = false;
	}
	
	if ( sVars->__prognamePtr != &__progname ) {
		FAIL("crt-libSystem sVars->__prognamePtr != &__progname (%p!=%p) for %s", sVars->__prognamePtr, &__progname, argv[0]);
		success = false;
	}
	
	if ( sVars->mh != &_mh_execute_header ) {
		FAIL("crt-libSystem sVars->mh != &_mh_execute_header (%p!=%p) for %s", sVars->mh, &_mh_execute_header, argv[0]);
		success = false;
	}
	
	if ( success )
		PASS("crt-libSystem");
		
	return EXIT_SUCCESS;
}



void __attribute__((constructor))
myInit(int argc, const char* argv[], const char* envp[], const char* apple[], const struct ProgramVars* vars)
{
	sVars = vars;
}






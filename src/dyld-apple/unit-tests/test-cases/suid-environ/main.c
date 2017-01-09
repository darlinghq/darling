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
#include <string.h> // strcmp(), strncmp()

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

//
// binaries set to run as some other user id never see any DYLD_ environment variables
//

int main(int argc, const char* argv[], const char* envp[], const char* apple[])
{
	// verify no DYLD_ variables
	const char** p;
	for(p = envp; *p != NULL; p++) {
		//fprintf(stderr, "%s\n", *p);
		if ( strncmp(*p, "DYLD_", 5) == 0 ) {
			FAIL("suid-environ: found %s", *p);
			return EXIT_SUCCESS;
		}
	}
	// verify same as apple parameter
	++p;
	if ( apple != p ) {
		FAIL("suid-environ: apple parameter not at end of envp");
		return EXIT_SUCCESS;
	}
	
	// verify apple parameter is not NULL and ends in main
	if ( *apple == NULL ) {
		FAIL("suid-environ: apple parameter is empty");
		return EXIT_SUCCESS;
	}
	if ( strstr(*apple, "/main") == NULL ) {
		FAIL("suid-environ: apple parameter is not path to main");
		return EXIT_SUCCESS;
	}
	
	PASS("suid-environ");
	return EXIT_SUCCESS;
}

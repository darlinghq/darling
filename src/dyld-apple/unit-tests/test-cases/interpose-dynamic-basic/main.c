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

extern const struct mach_header __dso_handle;


#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

typedef char* (*DupProc)(const char*);

char* mystrdup(const char* in)
{
	return "hello";
}

static const struct dyld_interpose_tuple sTable[] = { {&mystrdup, &strdup} };

int main()
{
	const char* preCall = strdup("123");
  DupProc preProc = &strdup;
  
  dyld_dynamic_interpose(&__dso_handle, sTable, 1);
  
	const char* postCall = strdup("123");
  DupProc postProc = &strdup;
  
  if ( strcmp(preCall, "123") != 0 ) {
      FAIL("interpose-dynamic-basic control strdup failed");
      return EXIT_SUCCESS;
  }
  
  if ( strcmp(postCall, "hello") != 0 ) {
      FAIL("interpose-dynamic-basic interposed lazy strdup failed");
      return EXIT_SUCCESS;
  }
  
  if ( preProc == postProc ) {
      FAIL("interpose-dynamic-basic interposed non-lazy strdup failed");
      return EXIT_SUCCESS;
  }
    
  PASS("interpose-dynamic-basic");
  
	return EXIT_SUCCESS;
}

/*
 * Copyright (c) 2006-2012 Apple Inc. All rights reserved.
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
#include <stdlib.h> // EXIT_SUCCESS
#include <mach-o/dyld.h>
#include <sys/sysctl.h>
#include <unistd.h>

#include "test.h"

//
// This builds an executable that needs > 2GB of stack
//


char* keepAlive;	// to keep compiler from optimizing away stack variable

// keep recursing until desired stack size achieved
void foo(unsigned long long stackSize, char* stackStart) 
{
	char buffer[32*1024*1024];
	keepAlive = buffer;
	// only recursive if there is enough room for next buffer
	intptr_t freeStackSpace = (buffer - sizeof(buffer)) - (stackStart - stackSize);
	//fprintf(stderr, "&buffer=%p, stackStart=%p, freeStackSpace=0x%lx\n", buffer, stackStart, freeStackSpace); 
	if ( freeStackSpace < sizeof(buffer) )
		return;
	else
		foo(stackSize, stackStart);
}


int
main()
{
	char start;
  foo(STACK_SIZE, &start);	
	return EXIT_SUCCESS;
}



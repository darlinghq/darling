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

#include "test.h" // PASS(), FAIL(), XPASS(), XFAIL()

static int    my_argc = 0;
static char **my_argv = NULL;
static char **my_envp = NULL;
static char **my_appl = NULL;

void
__attribute__((constructor))
my_init(int argc, char **argv, char **envp, char **appl)
{
  my_argc = argc;
  my_argv = argv;
  my_envp = envp;
  my_appl = appl;
}

int
main(int argc, char **argv, char **envp, char**appl)
{
  if(argc == my_argc
  && argv == my_argv
  && envp == my_envp
  && appl == my_appl)
  {
    PASS("initializer/constructor was called with arguments");
  }
  else
  {
    FAIL("initializer/constructor was not called with arguments");
  }
  return EXIT_SUCCESS;
}

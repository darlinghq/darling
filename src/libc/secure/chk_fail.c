/*
 * Copyright (c) 2007-2013 Apple Inc. All rights reserved.
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

#include <syslog.h>
#include <sys/sysctl.h>
#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>
#include <TargetConditionals.h>
#include <mach-o/dyld_priv.h>
#include "secure.h"

extern void __abort(void) __dead2;

void
__attribute__ ((noreturn))
__chk_fail (void)
{
  //const char message[] = "[%d] detected buffer overflow";

  //syslog(LOG_CRIT, message, getpid());

  const char message[] = "detected buffer overflow";
  write(2, message, sizeof(message)-1);

  __abort();
}

static void __chk_assert_no_overlap_callback(const struct mach_header* mh, intptr_t vmaddr_slide)
{
	if (__chk_assert_no_overlap)
	{
		if (dyld_get_sdk_version(mh) < DYLD_MACOSX_VERSION_10_9)
			__chk_assert_no_overlap = 0;
	}
}

void
__chk_init (void)
{
  if (dyld_get_program_sdk_version() >= DYLD_MACOSX_VERSION_10_9)
  {
    _dyld_register_func_for_add_image(&__chk_assert_no_overlap_callback);
    __chk_assert_no_overlap = 1;
  }
  else
    __chk_assert_no_overlap = 0;
}

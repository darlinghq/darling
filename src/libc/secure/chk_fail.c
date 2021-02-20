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

#include <os/assumes.h>
#include <stdint.h>
#include <TargetConditionals.h>
#include "crt_externs.h"

#ifndef PR_13085474_CHECK
#define PR_13085474_CHECK TARGET_OS_OSX
#endif

#if PR_13085474_CHECK
/* Some shipped applications fail this check and were tested against
 * versions of these functions that supported overlapping buffers.
 *
 * We would rather let such applications run, using the old memmove
 * implementation, than abort() because they can't use the new
 * implementation.
 */

#include <libkern/OSAtomic.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_priv.h>
#if TARGET_OS_OSX
#define START_VERSION dyld_platform_version_macOS_10_9
#else
#error "This platform should not build with PR_13085474_CHECK=1"
#endif
#endif /* !PR_13085474_CHECK */

/* For PR_13085474_CHECK set, we initialize __chk_assert_no_overlap to
 * 1 initially and then reset it to 0 if the main image of the process
 * was linked earlier than 10.9.
 *
 * If PR_13085474_CHECK is zero, then we never do any sdk version checking
 * and always do overlap checks.
 */
__attribute__ ((visibility ("hidden")))
uint32_t __chk_assert_no_overlap = 1;

#if PR_13085474_CHECK
static bool
__chk_assert_sdk_pre_start(const struct mach_header *mh) {
  return (dyld_get_active_platform() == PLATFORM_MACOS &&
      !dyld_sdk_at_least(mh, START_VERSION));
}
#endif

__attribute__ ((visibility ("hidden")))
void __chk_init(void) {
#if PR_13085474_CHECK
  if (__chk_assert_sdk_pre_start((const struct mach_header *)
        _NSGetMachExecuteHeader())) {
    __chk_assert_no_overlap = 0;
  }
#endif
}

__attribute__ ((visibility ("hidden")))
__attribute__ ((noreturn))
void
__chk_fail_overflow (void) {
  os_crash("detected buffer overflow");
}

__attribute__ ((visibility ("hidden")))
__attribute__ ((noreturn))
void
__chk_fail_overlap (void) {
  os_crash("detected source and destination buffer overlap");
}

__attribute__ ((visibility ("hidden")))
void
__chk_overlap (const void *_a, size_t an, const void *_b, size_t bn) {
  uintptr_t a = (uintptr_t)_a;
  uintptr_t b = (uintptr_t)_b;

  if (__builtin_expect(an == 0 || bn == 0, 0)) {
    return;
  } else if (__builtin_expect(a == b, 0)) {
    __chk_fail_overlap();
  } else if (a < b) {
    if (__builtin_expect(a + an > b, 0))
      __chk_fail_overlap();
  } else { // b < a
    if (__builtin_expect(b + bn > a, 0))
      __chk_fail_overlap();
  }
}

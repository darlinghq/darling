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

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <machine/cpu_capabilities.h>
#include <sys/kdebug.h>
#include <sys/kdebug_signpost.h>
#include <sys/errno.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>

extern int __kdebug_typefilter(void** addr, size_t* size);
extern int __kdebug_trace64(uint32_t code, uint64_t arg1, uint64_t arg2,
	uint64_t arg3, uint64_t arg4);
extern uint64_t __kdebug_trace_string(uint32_t debugid, uint64_t str_id,
	const char *str);

static int kdebug_signpost_internal(uint32_t debugid, uintptr_t arg1,
	uintptr_t arg2, uintptr_t arg3, uintptr_t arg4);

/*
 * GENERAL API DESIGN NOTE!
 *
 * Trace API's are expected to avoid performing checks until tracing has
 * been enabled. This includes checks that might cause error codes to be
 * returned.
 *
 * Trace invocations via wrapper and syscall must have the same behavior.
 *
 * Note that the userspace API is chosing to optimize fastpath, non-error
 * performance by eliding validation of each debugid. This means that error
 * cases which could have been caught in userspace will make a syscall
 * before returning with the correct error code. This tradeoff in performance
 * is intentional.
 */

void *
kdebug_typefilter(void)
{
	static void* typefilter;

	/* We expect kdebug_typefilter_bitmap to be valid (the if is not executed) */
	if (__builtin_expect(!typefilter, 0)) {
		// Map the typefilter if it can be mapped.
		void* ptr = NULL;
		size_t ptr_size = 0;

		if (__kdebug_typefilter(&ptr, &ptr_size) == 0) {
			void* old_value = NULL;
			if (ptr && !atomic_compare_exchange_strong((void* _Atomic volatile *)&typefilter, &old_value, ptr)) {
				mach_vm_deallocate(mach_task_self(), (mach_vm_offset_t)ptr, KDBG_TYPEFILTER_BITMAP_SIZE);
			}
		}
	}

	return typefilter;
}

bool
kdebug_is_enabled(uint32_t debugid)
{
	uint32_t state = *((volatile uint32_t *)(uintptr_t)(_COMM_PAGE_KDEBUG_ENABLE));

	if (state == 0) {
		return FALSE;
	}

	if ((state & KDEBUG_COMMPAGE_ENABLE_TYPEFILTER) > 0) {
		/*
		 * Typefilter rules...
		 *
		 * If no typefilter is available (even if due to error),
		 * debugids are allowed. 
		 *
		 * The typefilter will always allow DBG_TRACE; this is a kernel
		 * invariant. There is no need for an explicit check here.
		 *
		 * NOTE: The typefilter will always allow DBG_TRACE, but
		 * it is not legal to inject DBG_TRACE via kdebug_trace.
		 * Attempts to do so will not be detected here, but will be
		 * detected in the kernel, and an error will be returned. Per
		 * the API design note at the top of this file, this is a
		 * deliberate choice.
		 */
		uint8_t* typefilter = kdebug_typefilter();
		if (typefilter && isset(typefilter, KDBG_EXTRACT_CSC(debugid)) == 0) {
			return FALSE;
		}
	}

	return TRUE;
}

int
kdebug_trace(uint32_t debugid, uint64_t arg1, uint64_t arg2, uint64_t arg3,
             uint64_t arg4)
{
	if (!kdebug_is_enabled(debugid)) {
		return 0;
	}

	return __kdebug_trace64(debugid, arg1, arg2, arg3, arg4);
}

uint64_t
kdebug_trace_string(uint32_t debugid, uint64_t str_id, const char *str)
{
	if (!kdebug_is_enabled(debugid)) {
		return 0;
	}

	if ((int64_t)str_id == -1) {
		errno = EINVAL;
		return (uint64_t)-1;
	}

	if (str_id == 0 && str == NULL) {
		errno = EINVAL;
		return (uint64_t)-1;
	}

	return __kdebug_trace_string(debugid, str_id, str);
}

static int
kdebug_signpost_internal(uint32_t debugid, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
	if (KDBG_EXTRACT_CSC(debugid) != 0) {
		errno = EINVAL;
		return -1;
	}

	debugid |= APPSDBG_CODE(DBG_APP_SIGNPOST, 0);

	return kdebug_trace(debugid, arg1, arg2, arg3, arg4);
}

int
kdebug_signpost(uint32_t code, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
	return kdebug_signpost_internal(code << KDBG_CODE_OFFSET, arg1, arg2, arg3, arg4);
}

int
kdebug_signpost_start(uint32_t code, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
	return kdebug_signpost_internal((code << KDBG_CODE_OFFSET) | DBG_FUNC_START, arg1, arg2, arg3, arg4);
}

int
kdebug_signpost_end(uint32_t code, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3, uintptr_t arg4)
{
	return kdebug_signpost_internal((code << KDBG_CODE_OFFSET) | DBG_FUNC_END, arg1, arg2, arg3, arg4);
}

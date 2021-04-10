/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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
#include <sys/reason.h>
#include <sys/types.h>
#include <stdint.h>
#include <signal.h>
#include <os/reason_private.h>
#include <unistd.h>

/* Crash simulation */

extern int pthread_current_stack_contains_np(const void *, unsigned long);
int
__darwin_check_fd_set_overflow(int n, const void *fd_set, int unlimited_select)
{
	if (n < 0) {
		os_fault_with_payload(OS_REASON_LIBSYSTEM, OS_REASON_LIBSYSTEM_CODE_FAULT,
		    &n, sizeof(n), "FD_SET underflow", 0);
		return 0;
	}

	if (n >= __DARWIN_FD_SETSIZE) {
		if (pthread_current_stack_contains_np((const void *) fd_set, sizeof(struct fd_set))) {
			if (!unlimited_select) {
				os_fault_with_payload(OS_REASON_LIBSYSTEM, OS_REASON_LIBSYSTEM_CODE_FAULT,
				    &n, sizeof(n), "FD_SET overflow", 0);
				return 0;
			} else {
				return 1;
			}
		} else {
			return 1;
		}
	}

	return 1;
}

/* System call entry points */
int __terminate_with_payload(int pid, uint32_t reason_namespace, uint64_t reason_code,
    void *payload, uint32_t payload_size, const char *reason_string,
    uint64_t reason_flags);

void __abort_with_payload(uint32_t reason_namespace, uint64_t reason_code,
    void *payload, uint32_t payload_size, const char *reason_string,
    uint64_t reason_flags);

static void abort_with_payload_wrapper_internal(uint32_t reason_namespace, uint64_t reason_code,
    void *payload, uint32_t payload_size, const char *reason_string,
    uint64_t reason_flags) __attribute__((noreturn, cold));

/* System call wrappers */
int
terminate_with_reason(int pid, uint32_t reason_namespace, uint64_t reason_code,
    const char *reason_string, uint64_t reason_flags)
{
	return __terminate_with_payload(pid, reason_namespace, reason_code, 0, 0,
	           reason_string, reason_flags);
}

int
terminate_with_payload(int pid, uint32_t reason_namespace, uint64_t reason_code,
    void *payload, uint32_t payload_size,
    const char *reason_string, uint64_t reason_flags)
{
	return __terminate_with_payload(pid, reason_namespace, reason_code, payload,
	           payload_size, reason_string, reason_flags);
}

static void
abort_with_payload_wrapper_internal(uint32_t reason_namespace, uint64_t reason_code,
    void *payload, uint32_t payload_size, const char *reason_string,
    uint64_t reason_flags)
{
	sigset_t unmask_signal;

	/* Try to unmask SIGABRT before trapping to the kernel */
	sigemptyset(&unmask_signal);
	sigaddset(&unmask_signal, SIGABRT);
	sigprocmask(SIG_UNBLOCK, &unmask_signal, NULL);

	__abort_with_payload(reason_namespace, reason_code, payload, payload_size,
	    reason_string, reason_flags);

	/* If sending a SIGABRT failed, we fall back to SIGKILL */
	terminate_with_payload(getpid(), reason_namespace, reason_code, payload, payload_size,
	    reason_string, reason_flags | OS_REASON_FLAG_ABORT);

	__builtin_unreachable();
}

void
abort_with_reason(uint32_t reason_namespace, uint64_t reason_code, const char *reason_string,
    uint64_t reason_flags)
{
	abort_with_payload_wrapper_internal(reason_namespace, reason_code, 0, 0, reason_string, reason_flags);
}

void
abort_with_payload(uint32_t reason_namespace, uint64_t reason_code, void *payload,
    uint32_t payload_size, const char *reason_string,
    uint64_t reason_flags)
{
	abort_with_payload_wrapper_internal(reason_namespace, reason_code, payload, payload_size,
	    reason_string, reason_flags);
}

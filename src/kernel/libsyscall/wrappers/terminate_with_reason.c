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
#include <unistd.h>

/* System call entry points */
int __terminate_with_payload(int pid, uint32_t reason_namespace, uint64_t reason_code,
				void *payload, uint32_t payload_size, const char *reason_string,
				uint64_t reason_flags);

void __abort_with_payload(uint32_t reason_namespace, uint64_t reason_code,
				void *payload, uint32_t payload_size, const char *reason_string,
				uint64_t reason_flags);

static void abort_with_payload_wrapper_internal(uint32_t reason_namespace, uint64_t reason_code,
				void *payload, uint32_t payload_size, const char *reason_string,
				uint64_t reason_flags) __attribute__((noreturn));

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

static void abort_with_payload_wrapper_internal(uint32_t reason_namespace, uint64_t reason_code,
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

	/* If sending a SIGABRT failed, we try to fall back to SIGKILL */
	terminate_with_payload(getpid(), reason_namespace, reason_code, payload, payload_size,
			reason_string, reason_flags);

	/* Last resort, let's use SIGTRAP (SIGILL on i386) */
	sigemptyset(&unmask_signal);
	sigaddset(&unmask_signal, SIGTRAP);
	sigaddset(&unmask_signal, SIGILL);
	sigprocmask(SIG_UNBLOCK, &unmask_signal, NULL);

	__builtin_trap();
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


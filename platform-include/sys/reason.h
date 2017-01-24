/*
 * Copyright (c) 2016 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef _REASON_H_
#define _REASON_H_

#include <stdint.h>

__BEGIN_DECLS

#ifdef KERNEL

#include <kern/kern_cdata.h>

#ifdef XNU_KERNEL_PRIVATE
#include <kern/locks.h>

typedef struct os_reason {
	decl_lck_mtx_data(,       	osr_lock)
	int				osr_refcount;
	uint32_t			osr_namespace;
	uint64_t			osr_code;
	uint64_t			osr_flags;
	uint32_t			osr_bufsize;
	struct kcdata_descriptor	osr_kcd_descriptor;
	char 				*osr_kcd_buf;
} *os_reason_t;

#define OS_REASON_NULL ((os_reason_t) 0)

/* We only include 800 bytes of the exit reason description to not blow through the panic buffer */
#define LAUNCHD_PANIC_REASON_STRING_MAXLEN "800"

void os_reason_init(void);

os_reason_t build_userspace_exit_reason(uint32_t reason_namespace, uint64_t reason_code, user_addr_t payload, uint32_t payload_size,
					user_addr_t reason_string, uint64_t reason_flags);
char *launchd_exit_reason_get_string_desc(os_reason_t exit_reason);

#else /* XNU_KERNEL_PRIVATE */

typedef void * os_reason_t;

#endif /* XNU_KERNEL_PRIVATE */

os_reason_t os_reason_create(uint32_t osr_namespace, uint64_t osr_code);
int os_reason_alloc_buffer(os_reason_t cur_reason, uint32_t osr_bufsize);
struct kcdata_descriptor * os_reason_get_kcdata_descriptor(os_reason_t cur_reason);
void os_reason_ref(os_reason_t cur_reason);
void os_reason_free(os_reason_t cur_reason);

#endif /* KERNEL */

/*
 * Reason namespaces.
 */
#define OS_REASON_INVALID       0
#define OS_REASON_JETSAM        1
#define OS_REASON_SIGNAL        2
#define OS_REASON_CODESIGNING   3
#define OS_REASON_HANGTRACER    4
#define OS_REASON_TEST          5
#define OS_REASON_DYLD          6
#define OS_REASON_LIBXPC        7
#define OS_REASON_OBJC          8
#define OS_REASON_EXEC          9
#define OS_REASON_SPRINGBOARD   10
#define OS_REASON_TCC           11
#define OS_REASON_REPORTCRASH   12
#define OS_REASON_COREANIMATION 13
#define OS_REASON_AGGREGATED    14

/*
 * Update whenever new OS_REASON namespaces are added.
 */
#define OS_REASON_MAX_VALID_NAMESPACE OS_REASON_AGGREGATED

#define OS_REASON_BUFFER_MAX_SIZE 5120

#define OS_REASON_FLAG_NO_CRASH_REPORT          0x1  /* Don't create a crash report */
#define OS_REASON_FLAG_GENERATE_CRASH_REPORT    0x2  /* Create a crash report - the default for userspace requests */
#define OS_REASON_FLAG_FROM_USERSPACE           0x4  /* Reason created from a userspace syscall */
#define OS_REASON_FLAG_FAILED_DATA_COPYIN       0x8  /* We failed to copyin data from userspace */
#define OS_REASON_FLAG_PAYLOAD_TRUNCATED        0x10 /* The payload was truncated because it was longer than allowed */
#define OS_REASON_FLAG_BAD_PARAMS               0x20 /* Invalid parameters were passed involved with creating this reason */
#define OS_REASON_FLAG_CONSISTENT_FAILURE       0x40 /* Whatever caused this reason to be created will happen again */
#define OS_REASON_FLAG_ONE_TIME_FAILURE         0x80 /* Whatever caused this reason to be created was a one time issue */

/*
 * Set of flags that are allowed to be passed from userspace
 */
#define OS_REASON_FLAG_MASK_ALLOWED_FROM_USER	(OS_REASON_FLAG_CONSISTENT_FAILURE | OS_REASON_FLAG_ONE_TIME_FAILURE | OS_REASON_FLAG_NO_CRASH_REPORT)

/*
 * Macros to encode the exit reason namespace and first 32 bits of code in exception code
 * which is used by Report Crash as a hint. It should be only used as a hint since it
 * looses higher 32 bits of exit reason code.
 */
#define ENCODE_OSR_NAMESPACE_TO_MACH_EXCEPTION_CODE(code, osr_namespace) \
	(code) = (code) | (((osr_namespace) & ((uint64_t)UINT32_MAX)) << 32)
#define ENCODE_OSR_CODE_TO_MACH_EXCEPTION_CODE(code, osr_code) \
	(code) = (code) | ((osr_code) & ((uint64_t)UINT32_MAX))

#ifndef KERNEL
/*
 * abort_with_reason: Used to exit the current process and pass along
 *                    specific information about why it is being terminated.
 *
 * Inputs:              args->reason_namespace - OS_REASON namespace specified for the reason
 *                      args->reason_code - code in the specified namespace for the reason
 *                      args->reason_string - additional string formatted information about the request
 *                      args->reason_flags - options requested for how the process should be terminated (see OS_REASON_FLAG_* above).
 *
 * Outputs:             Does not return.
 */
void abort_with_reason(uint32_t reason_namespace, uint64_t reason_code, const char *reason_string, uint64_t reason_flags) __attribute__((noreturn));

/*
 * abort_with_payload: Used to exit the current process and pass along
 *                     specific information about why it is being terminated. The payload pointer
 *                     should point to structured data that can be interpreted by the consumer of
 *                     exit reason information.
 *
 * Inputs:              args->reason_namespace - OS_REASON namespace specified for the reason
 *                      args->reason_code - code in the specified namespace for the reason
 *                      args->payload - pointer to payload structure in user space
 *                      args->payload_size - length of payload buffer (this will be truncated to EXIT_REASON_PAYLOAD_MAX_LEN)
 *                      args->reason_string - additional string formatted information about the request
 *                      args->reason_flags - options requested for how the process should be terminated (see OS_REASON_FLAG_* above).
 *
 * Outputs:             Does not return.
 */
void abort_with_payload(uint32_t reason_namespace, uint64_t reason_code, void *payload, uint32_t payload_size, const char *reason_string,
                                uint64_t reason_flags) __attribute__((noreturn));

/*
 * terminate_with_reason: Used to terminate a specific process and pass along
 *                        specific information about why it is being terminated.
 *
 * Inputs:              args->pid - the PID of the process to be terminated
 *                      args->reason_namespace - OS_REASON namespace specified for the reason
 *                      args->reason_code - code in the specified namespace for the reason
 *                      args->reason_string - additional string formatted information about the request
 *                      args->reason_flags - options requested for how the process should be terminated (see OS_REASON_FLAG_* above)
 *
 * Outputs:             EINVAL if the PID requested is the same as that of the calling process, invalid or the namespace provided is invalid.
 *                      ESRCH if we couldn't find a live process with the requested PID
 *                      EPERM if the caller is not privileged enough to kill the process with the requested PID
 *                      returns 0 otherwise
 */
int terminate_with_reason(int pid, uint32_t reason_namespace, uint64_t reason_code, const char *reason_string, uint64_t reason_flags);

/*
 * terminate_with_payload: Used to terminate a specific process and pass along
 *                         specific information about why it is being terminated. The payload pointer
 *                         should point to structured data that can be interpreted by the consumer of
 *                         exit reason information.
 *
 * Inputs:              args->pid - the PID of the process to be terminated.
 *                      args->reason_namespace - OS_REASON namespace specified for the reason
 *                      args->reason_code - code in the specified namespace for the reason
 *                      args->payload - pointer to payload structure in user space
 *                      args->payload_size - length of payload buffer (this will be truncated to EXIT_REASON_PAYLOAD_MAX_LEN)
 *                      args->reason_string - additional string formatted information about the request
 *                      args->reason_flags - options requested for how the process should be terminated (see OS_REASON_FLAG_* above)
 *
 * Outputs:             EINVAL if the PID requested is the same as that of the calling process, is invalid or the namespace provided is invalid.
 *                      ESRCH if we couldn't find a live process with the requested PID
 *                      EPERM if the caller is not privileged enough to kill the process with the requested PID
 *                      returns 0 otherwise
 */
int terminate_with_payload(int pid, uint32_t reason_namespace, uint64_t reason_code, void *payload, uint32_t payload_size,
                                const char *reason_string, uint64_t reason_flags);
#endif /* KERNEL */

/*
 * codesigning exit reasons
 */
#define CODESIGNING_EXIT_REASON_TASKGATED_INVALID_SIG 1
#define CODESIGNING_EXIT_REASON_INVALID_PAGE          2
#define CODESIGNING_EXIT_REASON_TASK_ACCESS_PORT      3

/*
 * exec path specific exit reasons
 */
#define EXEC_EXIT_REASON_BAD_MACHO          1
#define EXEC_EXIT_REASON_SUGID_FAILURE      2
#define EXEC_EXIT_REASON_ACTV_THREADSTATE   3
#define EXEC_EXIT_REASON_STACK_ALLOC        4
#define EXEC_EXIT_REASON_APPLE_STRING_INIT  5
#define EXEC_EXIT_REASON_COPYOUT_STRINGS    6
#define EXEC_EXIT_REASON_COPYOUT_DYNLINKER  7
#define EXEC_EXIT_REASON_SECURITY_POLICY    8
#define EXEC_EXIT_REASON_TASKGATED_OTHER    9
#define EXEC_EXIT_REASON_FAIRPLAY_DECRYPT   10
#define EXEC_EXIT_REASON_DECRYPT            11
#define EXEC_EXIT_REASON_UPX                12

__END_DECLS

#endif /* _REASON_H_ */

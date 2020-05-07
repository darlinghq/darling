/*
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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

#ifndef _SYS_CSR_H_
#define _SYS_CSR_H_

#include <stdint.h>
#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

#ifdef __APPLE_API_PRIVATE

typedef uint32_t csr_config_t;
typedef uint32_t csr_op_t;

/* Rootless configuration flags */
#define CSR_ALLOW_UNTRUSTED_KEXTS               (1 << 0)
#define CSR_ALLOW_UNRESTRICTED_FS               (1 << 1)
#define CSR_ALLOW_TASK_FOR_PID                  (1 << 2)
#define CSR_ALLOW_KERNEL_DEBUGGER               (1 << 3)
#define CSR_ALLOW_APPLE_INTERNAL                (1 << 4)
#define CSR_ALLOW_DESTRUCTIVE_DTRACE    (1 << 5) /* name deprecated */
#define CSR_ALLOW_UNRESTRICTED_DTRACE   (1 << 5)
#define CSR_ALLOW_UNRESTRICTED_NVRAM    (1 << 6)
#define CSR_ALLOW_DEVICE_CONFIGURATION  (1 << 7)
#define CSR_ALLOW_ANY_RECOVERY_OS       (1 << 8)
#define CSR_ALLOW_UNAPPROVED_KEXTS      (1 << 9)
#define CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE    (1 << 10)

#define CSR_VALID_FLAGS (CSR_ALLOW_UNTRUSTED_KEXTS | \
	                 CSR_ALLOW_UNRESTRICTED_FS | \
	                 CSR_ALLOW_TASK_FOR_PID | \
	                 CSR_ALLOW_KERNEL_DEBUGGER | \
	                 CSR_ALLOW_APPLE_INTERNAL | \
	                 CSR_ALLOW_UNRESTRICTED_DTRACE | \
	                 CSR_ALLOW_UNRESTRICTED_NVRAM | \
	                 CSR_ALLOW_DEVICE_CONFIGURATION | \
	                 CSR_ALLOW_ANY_RECOVERY_OS | \
	                 CSR_ALLOW_UNAPPROVED_KEXTS | \
	                 CSR_ALLOW_EXECUTABLE_POLICY_OVERRIDE)

#define CSR_ALWAYS_ENFORCED_FLAGS (CSR_ALLOW_DEVICE_CONFIGURATION | CSR_ALLOW_ANY_RECOVERY_OS)

/* CSR capabilities that a booter can give to the system */
#define CSR_CAPABILITY_UNLIMITED                                (1 << 0)
#define CSR_CAPABILITY_CONFIG                                   (1 << 1)
#define CSR_CAPABILITY_APPLE_INTERNAL                   (1 << 2)

#define CSR_VALID_CAPABILITIES (CSR_CAPABILITY_UNLIMITED | CSR_CAPABILITY_CONFIG | CSR_CAPABILITY_APPLE_INTERNAL)

#ifdef PRIVATE
/* Private system call interface between Libsyscall and xnu */

/* Syscall flavors */
enum csr_syscalls {
	CSR_SYSCALL_CHECK,
	CSR_SYSCALL_GET_ACTIVE_CONFIG,
};

#endif /* PRIVATE */

__BEGIN_DECLS

#ifdef XNU_KERNEL_PRIVATE
void csr_init(void);
#endif

/* Syscalls */
int csr_check(csr_config_t mask);
int csr_get_active_config(csr_config_t *config);

__END_DECLS

#endif /* __APPLE_API_PRIVATE */

#endif /* _SYS_CSR_H_ */

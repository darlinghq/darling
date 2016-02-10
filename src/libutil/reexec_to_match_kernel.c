/*
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
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

#include <sys/cdefs.h>

#include <spawn.h>
#include <errno.h>
#include <crt_externs.h>
#include <mach/mach.h>
#include <mach-o/loader.h>
#include <mach-o/dyld.h>
#include <sys/sysctl.h>
#include <stdlib.h>
#include <stdio.h>

#include "libutil.h"

static cpu_type_t current_program_arch(void);
static cpu_type_t current_kernel_arch(void);
static int reexec(cpu_type_t cputype, const char *guardenv);

#define kReExecToMatchKernel "REEXEC_TO_MATCH_KERNEL"
#define kReExecToMatchLP64 "REEXEC_TO_MATCH_LP64NESS"

int reexec_to_match_kernel(void)
{
	cpu_type_t kernarch, progarch;
	char *alreadyenv;
	
	alreadyenv = getenv(kReExecToMatchKernel);
	if (alreadyenv) {
		/* we've done this at least once, assume
		   another try won't help */
		return 0;
	}

	kernarch = current_kernel_arch();
	progarch = current_program_arch();

	if (kernarch == 0) {
		/* could not determine kernel arch */
		errno = EINVAL;
		return -1;
	}

	if (kernarch == progarch) {
		/* nothing to do here */
		return 0;
	}

	/* Now we need to re-exec */
	return reexec(kernarch, kReExecToMatchKernel);
}

int reexec_to_match_lp64ness(bool isLP64)
{
	cpu_type_t kernarch, progarch, targetarch;
	char *alreadyenv;
	
	alreadyenv = getenv(kReExecToMatchLP64);
	if (alreadyenv) {
		/* we've done this at least once, assume
		   another try won't help */
		return 0;
	}

	kernarch = current_kernel_arch();
	progarch = current_program_arch();

	if (kernarch == 0) {
		/* could not determine kernel arch */
		errno = EINVAL;
		return -1;
	}

	if (isLP64) {
		targetarch = kernarch | CPU_ARCH_ABI64;
	} else {
		targetarch = kernarch & ~CPU_ARCH_ABI64;
	}

	if (targetarch == progarch) {
		/* nothing to do here */
		return 0;
	}

	/* Now we need to re-exec */
	return reexec(targetarch, kReExecToMatchLP64);
}

static cpu_type_t current_program_arch(void)
{
	cpu_type_t current_arch = (_NSGetMachExecuteHeader())->cputype;

	return current_arch;
}

static cpu_type_t current_kernel_arch(void)
{
	struct host_basic_info  hi;
	unsigned int            size;
	kern_return_t           kret;
	cpu_type_t				current_arch;
	int						ret, mib[4];
	size_t					len;
	struct kinfo_proc		kp;
	
	size = sizeof(hi)/sizeof(int);
	kret = host_info(mach_host_self(), HOST_BASIC_INFO, (host_info_t)&hi, &size);
	if (kret != KERN_SUCCESS) {
		return 0;
	}

	current_arch = hi.cpu_type;

	/* Now determine if the kernel is running in 64-bit mode */
	mib[0] = CTL_KERN;
	mib[1] = KERN_PROC;
	mib[2] = KERN_PROC_PID;
	mib[3] = 0; /* kernproc, pid 0 */
	len = sizeof(kp);
	ret = sysctl(mib, sizeof(mib)/sizeof(mib[0]), &kp, &len, NULL, 0);
	if (ret == -1) {
		return 0;
	}

	if (kp.kp_proc.p_flag & P_LP64) {
		current_arch |= CPU_ARCH_ABI64;
	}

	return current_arch;
}

static int reexec(cpu_type_t cputype, const char *guardenv)
{
	posix_spawnattr_t  attr;
	int                ret, envcount;
	size_t             copied = 0;
	char			   **argv, **oldenvp, **newenvp;
	char			   execpath[MAXPATHLEN+1];
	uint32_t		   execsize;
	char               guardstr[32];

	argv = *_NSGetArgv();
	oldenvp = *_NSGetEnviron();
	for (envcount = 0; oldenvp[envcount]; envcount++);
	// if there are 4 elements and a NULL, envcount will be 4

	newenvp = calloc(envcount+2, sizeof(newenvp[0]));
	for (envcount = 0; oldenvp[envcount]; envcount++) {
		newenvp[envcount] = oldenvp[envcount];
	}

	snprintf(guardstr, sizeof(guardstr), "%s=1", guardenv);
	newenvp[envcount++] = guardstr;
	newenvp[envcount] = NULL;

	execsize = (uint32_t)sizeof(execpath);
	ret = _NSGetExecutablePath(execpath, &execsize);
	if (ret != 0) {
		return -1;
	}

	ret = posix_spawnattr_init(&attr);
	if (ret != 0) {
		return -1;
	}
	ret = posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETEXEC);
	if (ret != 0) {
		return -1;
	}
	ret = posix_spawnattr_setbinpref_np(&attr, 1, &cputype, &copied);
	if (ret != 0 || copied != 1) {
		return -1;
	}

#if 0
	fprintf(stderr, "reexec: %s (arch=%d)\n", execpath, cputype);
	for (envcount=0; newenvp[envcount]; envcount++) {
		fprintf(stderr, "env[%d] = %s\n", envcount, newenvp[envcount]);
	}
	for (envcount=0; argv[envcount]; envcount++) {
		fprintf(stderr, "argv[%d] = %s\n", envcount, argv[envcount]);
	}
#endif

	ret = posix_spawn(NULL, execpath, NULL, &attr, argv, newenvp);
	if (ret != 0) {
		errno = ret;
		return -1;
	}

	/* should not be reached */
	return 0;
}

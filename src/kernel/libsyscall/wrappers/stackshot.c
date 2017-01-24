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
#include <sys/stackshot.h>
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

/*
 * System call entry point
 */
int __stack_snapshot_with_config(int stackshot_config_version, user_addr_t stackshot_config, size_t stackshot_config_size);

/*
 * stackshot_config_create:	create and initialize the arguments for a stackshot
 *
 * Outputs:			NULL if malloc fails
 *				a pointer to a new stackshot_config_t on success
 */
stackshot_config_t *
stackshot_config_create(void)
{
	stackshot_config_t *s_config;

	s_config = malloc(sizeof(stackshot_config_t));
	if (s_config == NULL) {
		return NULL;
	}

	s_config->sc_pid = -1;
	s_config->sc_flags = 0;
	s_config->sc_delta_timestamp = 0;
	s_config->sc_buffer = 0;
	s_config->sc_size = 0;

	return s_config;
}

/*
 * stackshot_config_set_pid:	set the PID to be traced
 *
 * Inputs:			stackshot_config - a pointer to the stackshot_config_t we want to update
 *				pid - process id of process to be traced, or -1 for the entire system
 *
 * Outputs:			EINVAL if the passed stackshot_config pointer is NULL
 *				0 on success
 */
int
stackshot_config_set_pid(stackshot_config_t *stackshot_config, int pid)
{
	stackshot_config_t *s_config;

	if (stackshot_config == NULL) {
		return EINVAL;
	}

	s_config = (stackshot_config_t *) stackshot_config;
	s_config->sc_pid = pid;

	return 0;
}

/*
 * stackshot_config_set_flags:	set the flags to be passed for the stackshot
 *
 * Inputs:			stackshot_config - a pointer to the stackshot_config_t we want to update
 *				flags - flags to pass to stackshot
 *
 * Outputs:			EINVAL if the passed stackshot_config pointer is NULL
 *				0 on success
 */
int
stackshot_config_set_flags(stackshot_config_t *stackshot_config, uint32_t flags)
{
	stackshot_config_t *s_config;

	if (stackshot_config == NULL) {
		return EINVAL;
	}

	s_config = (stackshot_config_t *) stackshot_config;
	s_config->sc_flags = flags;

	return 0;
}

/*
 * stackshot_capture_with_config:	take a stackshot with the provided config
 *
 * Inputs:				stackshot_config - a pointer to the stackshot_config_t we want to use
 *
 * Outputs:				EINVAL if the passed stackshot_config pointer is NULL, a caller is trying
 *						to reuse a config without deallocating its buffer or if there is a
 *						problem with the arguments
 *					EFAULT if there was a problem with accessing the arguments from the kernel
 *					EPERM if the caller is not privileged
 *					ENOTSUP if the caller is passing a stackshot config version that is not
 *						supported by the kernel (indicates libsyscall:kernel mismatch),
 *						or if the caller is requesting unsupported flags
 *					ENOMEM if the kernel is unable to allocate memory
 *					ENOSPC if the caller doesn't have enough space in their address space for
 *						the kernel to remap the buffer
 *					ENOENT if the caller is requesting an existing buffer that doesn't exist
 *						or the target PID isn't found
 *					0 on success
 */
int
stackshot_capture_with_config(stackshot_config_t *stackshot_config)
{
	int ret;
	stackshot_config_t *s_config;

	if (stackshot_config == NULL) {
		return EINVAL;
	}

	s_config = (stackshot_config_t *) stackshot_config;
	if (s_config->sc_buffer != 0)  {
		return EINVAL;
	}

	s_config->sc_out_buffer_addr = &s_config->sc_buffer;
	s_config->sc_out_size_addr = &s_config->sc_size;
	ret = __stack_snapshot_with_config(STACKSHOT_CONFIG_TYPE, s_config, sizeof(stackshot_config_t));
	
	if (ret != 0) {
		ret = errno;
		s_config->sc_buffer = 0;
		s_config->sc_size = 0;
	}

	return ret;
}

/*
 * stackshot_config_get_stackshot_buffer:	get a pointer to the buffer containing the stackshot
 *
 * Inputs:					stackshot_config - a pointer to a stackshot_config_t
 *
 * Outputs:					NULL if the passed stackshot_config is NULL or if its buffer is NULL
 *						a pointer to the buffer containing the stackshot on success
 */
void *
stackshot_config_get_stackshot_buffer(stackshot_config_t *stackshot_config)
{
	stackshot_config_t *s_config;

	if (stackshot_config == NULL) {
		return NULL;
	}
	s_config = (stackshot_config_t *) stackshot_config;

	return ((void *)s_config->sc_buffer);
}

/*
 * stackshot_config_get_stackshot_size:	get the size of the stackshot buffer
 *
 * Inputs:  stackshot_config - a pointer to a stackshot_config_t
 *
 * Outputs: -1 if the passed stackshot config is NULL or there is no buffer
 *             the length of the stackshot buffer on success
 */
uint32_t
stackshot_config_get_stackshot_size(stackshot_config_t * stackshot_config)
{
	if (stackshot_config == NULL || (void *)stackshot_config->sc_buffer == NULL) {
		return -1;
	}

	return stackshot_config->sc_size;
}

/*
 * stackshot_config_set_size_hint: set the size of the stackshot buffer
 *
 * Inputs:  stackshot_config - a pointer to a stackshot_config_t
 *          suggested_size - hint for size allocation of stackshot
 *
 * Outputs:  -1  if the passed stackshot config is NULL or there is existing stackshot buffer set.
 *              the length of the stackshot buffer on success.
 */
int
stackshot_config_set_size_hint(stackshot_config_t *stackshot_config, uint32_t suggested_size)
{
	if (stackshot_config == NULL || (void *)stackshot_config->sc_buffer != NULL) {
		return -1;
	}

	stackshot_config->sc_size = suggested_size;

	return 0;
}

/*
 * stackshot_config_set_delta_timestamp: set the timestamp to use as the basis for the delta stackshot
 *
 * This timestamp will be used along with STACKSHOT_COLLECT_DELTA_SNAPSHOT flag to collect delta stackshots
 *
 * Inputs:  stackshot_config - a pointer to a stackshot_config_t
 *          delta_timestamp - timestamp in MachAbsoluteTime units to be used as the basis for a delta stackshot
 *
 * Outputs:  -1  if the passed stackshot config is NULL or there is existing stackshot buffer set.
 *           0 on success
 */
int
stackshot_config_set_delta_timestamp(stackshot_config_t *stackshot_config, uint64_t delta_timestamp)
{
	if (stackshot_config == NULL || (void *)stackshot_config->sc_buffer != NULL) {
		return -1;
	}

	stackshot_config->sc_delta_timestamp = delta_timestamp;

	return 0;
}


/*
 * stackshot_config_dealloc_buffer:  dealloc the stackshot buffer and reset the size so that a
 *   stackshot_config_t can be used again
 *
 * Inputs:   stackshot_config - a pointer to a stackshot_config_t
 *
 * Outputs:  EINVAL if the passed stackshot_config is NULL or if its buffer is NULL
 *           0 otherwise
 */
int
stackshot_config_dealloc_buffer(stackshot_config_t *stackshot_config)
{
	stackshot_config_t *s_config;

	if (stackshot_config == NULL) {
		return EINVAL;
	}
	s_config = (stackshot_config_t *) stackshot_config;

	if (s_config->sc_size && s_config->sc_buffer) {
		mach_vm_deallocate(mach_task_self(), (mach_vm_offset_t)s_config->sc_buffer, (mach_vm_size_t)s_config->sc_size);
	}

	s_config->sc_buffer = 0;
	s_config->sc_size = 0;

	return 0;
}

/*
 * stackshot_config_dealloc:	dealloc the stackshot buffer and the stackshot config
 *
 * Inputs:			stackshot_config - a pointer to a stackshot_config_t
 *
 * Outputs:			EINVAL if the passed stackshot_cofnig is NULL
 *				0 otherwise
 */
int
stackshot_config_dealloc(stackshot_config_t *stackshot_config)
{
	stackshot_config_t *s_config;

	if (stackshot_config == NULL) {
		return EINVAL;
	}
	s_config = (stackshot_config_t *) stackshot_config;

	if (s_config->sc_size && s_config->sc_buffer) {
		mach_vm_deallocate(mach_task_self(), (mach_vm_offset_t)s_config->sc_buffer, (mach_vm_size_t)s_config->sc_size);
	}

	s_config->sc_buffer = 0;
	s_config->sc_size = 0;

	free(s_config);
	return 0;
}

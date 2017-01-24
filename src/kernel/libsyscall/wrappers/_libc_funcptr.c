/*
 * Copyright (c) 2010-2014 Apple Inc. All rights reserved.
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

#include "_libkernel_init.h"
extern _libkernel_functions_t _libkernel_functions;
extern void mig_os_release(void* ptr);

__attribute__((visibility("hidden")))
void *
malloc(size_t size)
{
	return _libkernel_functions->malloc(size);
}

__attribute__((visibility("hidden")))
void
free(void *ptr)
{
	return _libkernel_functions->free(ptr);
}

__attribute__((visibility("hidden")))
void *
realloc(void *ptr, size_t size)
{
	return _libkernel_functions->realloc(ptr, size);
}

__attribute__((visibility("hidden")))
void *
reallocf(void *ptr, size_t size)
{
	void *nptr = realloc(ptr, size);
	if (!nptr && ptr)
		free(ptr);
	return (nptr);
}

__attribute__((visibility("hidden")))
void
_pthread_exit_if_canceled(int error)
{
	return _libkernel_functions->_pthread_exit_if_canceled(error);
}

__attribute__((visibility("hidden")))
void
_pthread_set_self(void *ptr __attribute__((__unused__))) {}

__attribute__((visibility("hidden")))
void
_pthread_clear_qos_tsd(mach_port_t thread_port)
{
	if (_libkernel_functions->version >= 3 &&
			_libkernel_functions->pthread_clear_qos_tsd) {
		return _libkernel_functions->pthread_clear_qos_tsd(thread_port);
	}
}

/*
 * mach/mach.h voucher_mach_msg API
 */

static const struct _libkernel_voucher_functions
		_libkernel_voucher_functions_empty;
static _libkernel_voucher_functions_t _libkernel_voucher_functions =
		&_libkernel_voucher_functions_empty;

kern_return_t
__libkernel_voucher_init(_libkernel_voucher_functions_t fns)
{
	_libkernel_voucher_functions = fns;
	return KERN_SUCCESS;
}

boolean_t
voucher_mach_msg_set(mach_msg_header_t *msg)
{
	if (_libkernel_voucher_functions->voucher_mach_msg_set) {
		return _libkernel_voucher_functions->voucher_mach_msg_set(msg);
	}
	return 0;
}

void
voucher_mach_msg_clear(mach_msg_header_t *msg)
{
	if (_libkernel_voucher_functions->voucher_mach_msg_clear) {
		return _libkernel_voucher_functions->voucher_mach_msg_clear(msg);
	}
}

voucher_mach_msg_state_t
voucher_mach_msg_adopt(mach_msg_header_t *msg)
{
	if (_libkernel_voucher_functions->voucher_mach_msg_adopt) {
		return _libkernel_voucher_functions->voucher_mach_msg_adopt(msg);
	}
	return VOUCHER_MACH_MSG_STATE_UNCHANGED;
}

void
voucher_mach_msg_revert(voucher_mach_msg_state_t state)
{
	if (_libkernel_voucher_functions->voucher_mach_msg_revert) {
		return _libkernel_voucher_functions->voucher_mach_msg_revert(state);
	}
}


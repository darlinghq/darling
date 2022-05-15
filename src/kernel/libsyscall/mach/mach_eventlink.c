/*
 * Copyright (c) 2019 Apple Computer, Inc. All rights reserved.
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

#include <mach/boolean.h>
#include <mach/message.h>
#include <mach/kern_return.h>
#include <mach/mach_traps.h>
#include <mach/mach_types.h>
#include <mach/clock_types.h>
#include <mach/mach_eventlink_types.h>

/*
 * __mach_eventlink* calls are bsd syscalls instead of mach traps because
 * they need to return a 64 bit value in register and mach traps currently
 * does not allow 64 bit return values.
 */
uint64_t
__mach_eventlink_signal(
	mach_port_t         eventlink_port,
	uint64_t            signal_count);

uint64_t
__mach_eventlink_wait_until(
	mach_port_t                          eventlink_port,
	uint64_t                             wait_signal_count,
	uint64_t                             deadline,
	kern_clock_id_t                      clock_id,
	mach_eventlink_signal_wait_option_t  option);

uint64_t
__mach_eventlink_signal_wait_until(
	mach_port_t                          eventlink_port,
	uint64_t                             wait_count,
	uint64_t                             signal_count,
	uint64_t                             deadline,
	kern_clock_id_t                      clock_id,
	mach_eventlink_signal_wait_option_t  option);

kern_return_t
mach_eventlink_signal(
	mach_port_t         eventlink_port,
	uint64_t            signal_count)
{
	uint64_t retval = __mach_eventlink_signal(eventlink_port, signal_count);

	return decode_eventlink_error_from_retval(retval);
}

kern_return_t
mach_eventlink_wait_until(
	mach_port_t                          eventlink_port,
	uint64_t                             *wait_count_ptr,
	mach_eventlink_signal_wait_option_t  option,
	kern_clock_id_t                      clock_id,
	uint64_t                             deadline)
{
	uint64_t retval;

	retval = __mach_eventlink_wait_until(eventlink_port, *wait_count_ptr,
	    deadline, clock_id, option);

	*wait_count_ptr = decode_eventlink_count_from_retval(retval);
	return decode_eventlink_error_from_retval(retval);
}

kern_return_t
mach_eventlink_signal_wait_until(
	mach_port_t                          eventlink_port,
	uint64_t                             *wait_count_ptr,
	uint64_t                             signal_count,
	mach_eventlink_signal_wait_option_t  option,
	kern_clock_id_t                      clock_id,
	uint64_t                             deadline)
{
	uint64_t retval;
	retval = __mach_eventlink_signal_wait_until(eventlink_port, *wait_count_ptr,
	    signal_count, deadline, clock_id, option);
	*wait_count_ptr = decode_eventlink_count_from_retval(retval);
	return decode_eventlink_error_from_retval(retval);
}

/*
 * Copyright (c) 2018 Apple Inc. All rights reserved.
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
#include <mach/mach.h>
#include <mach/mach_traps.h>
#include <mach/mach_port.h>
#include <mach/mach_right.h>


#pragma mark Utilities
#define _mach_assert(__op, __kr) \
	do { \
	        if (kr != KERN_SUCCESS) { \
	                __builtin_trap(); \
	        } \
	} while (0)

#pragma mark API
mach_right_recv_t
mach_right_recv_construct(mach_right_flags_t flags,
    mach_right_send_t *_Nullable sr, uintptr_t ctx)
{
	kern_return_t kr = KERN_FAILURE;
	mach_port_t p = MACH_PORT_NULL;
	mach_port_options_t opts = {
		.flags = MPO_CONTEXT_AS_GUARD,
		.mpl = {
			.mpl_qlimit = MACH_PORT_QLIMIT_BASIC,
		},
	};

	if (flags & MACH_RIGHT_RECV_FLAG_UNGUARDED) {
		opts.flags &= (~MPO_CONTEXT_AS_GUARD);
	}
	if (sr) {
		opts.flags |= MPO_INSERT_SEND_RIGHT;
	}

	kr = mach_port_construct(mach_task_self(), &opts, ctx, &p);
	_mach_assert("construct recv right", kr);

	if (sr) {
		sr->mrs_name = p;
	}

	return mach_right_recv(p);
}

void
mach_right_recv_destruct(mach_right_recv_t r, mach_right_send_t *s,
    uintptr_t ctx)
{
	kern_return_t kr = KERN_FAILURE;
	mach_port_delta_t srd = 0;

	if (s) {
		if (r.mrr_name != s->mrs_name) {
			__builtin_trap();
		}

		srd = -1;
	}

	kr = mach_port_destruct(mach_task_self(), r.mrr_name, srd, ctx);
	_mach_assert("destruct recv right", kr);
}

mach_right_send_t
mach_right_send_create(mach_right_recv_t r)
{
	kern_return_t kr = KERN_FAILURE;

	kr = mach_port_insert_right(mach_task_self(), r.mrr_name, r.mrr_name,
	    MACH_MSG_TYPE_MAKE_SEND);
	_mach_assert("create send right", kr);

	return mach_right_send(r.mrr_name);
}

mach_right_send_t
mach_right_send_retain(mach_right_send_t s)
{
	kern_return_t kr = KERN_FAILURE;
	mach_right_send_t rs = MACH_RIGHT_SEND_NULL;

	kr = mach_port_mod_refs(mach_task_self(), s.mrs_name,
	    MACH_PORT_RIGHT_SEND, 1);
	switch (kr) {
	case 0:
		rs = s;
		break;
	case KERN_INVALID_RIGHT:
		rs.mrs_name = MACH_PORT_DEAD;
		break;
	case KERN_INVALID_NAME:
	// mach_port_mod_refs() will return success when given either
	// MACH_PORT_DEAD or MACH_PORT_NULL with send or send-once right
	// operations, so this is always fatal.
	default:
		_mach_assert("retain send right", kr);
	}

	return rs;
}

void
mach_right_send_release(mach_right_send_t s)
{
	kern_return_t kr = KERN_FAILURE;

	kr = mach_port_mod_refs(mach_task_self(), s.mrs_name,
	    MACH_PORT_RIGHT_SEND, -1);
	switch (kr) {
	case 0:
		break;
	case KERN_INVALID_RIGHT:
		kr = mach_port_mod_refs(mach_task_self(), s.mrs_name,
		    MACH_PORT_RIGHT_DEAD_NAME, -1);
		_mach_assert("release dead name", kr);
		break;
	default:
		_mach_assert("release send right", kr);
	}
}

mach_right_send_once_t
mach_right_send_once_create(mach_right_recv_t r)
{
	mach_msg_type_name_t right = 0;
	mach_port_t so = MACH_PORT_NULL;
	kern_return_t kr = mach_port_extract_right(mach_task_self(), r.mrr_name,
	    MACH_MSG_TYPE_MAKE_SEND_ONCE, &so, &right);
	_mach_assert("create send-once right", kr);

	return mach_right_send_once(so);
}

void
mach_right_send_once_consume(mach_right_send_once_t so)
{
	kern_return_t kr = KERN_FAILURE;

	kr = mach_port_mod_refs(mach_task_self(), so.mrso_name,
	    MACH_PORT_RIGHT_SEND_ONCE, -1);
	switch (kr) {
	case 0:
		break;
	case KERN_INVALID_RIGHT:
		kr = mach_port_mod_refs(mach_task_self(), so.mrso_name,
		    MACH_PORT_RIGHT_DEAD_NAME, -1);
		_mach_assert("release dead name", kr);
		break;
	default:
		_mach_assert("consume send-once right", kr);
	}
}

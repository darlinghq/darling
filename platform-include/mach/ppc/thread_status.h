/*
 * Copyright (c) 2000-2005 Apple Computer, Inc. All rights reserved.
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
/*
 * @OSF_COPYRIGHT@
 */

#ifndef	_MACH_PPC_THREAD_STATUS_H_
#define _MACH_PPC_THREAD_STATUS_H_

#include <mach/ppc/_structs.h>
#include <mach/message.h>

/*
 * ppc_thread_state is the structure that is exported to user threads for 
 * use in status/mutate calls.  This structure should never change.
 *
 */

#define PPC_THREAD_STATE        1
#define PPC_FLOAT_STATE         2
#define PPC_EXCEPTION_STATE		3
#define PPC_VECTOR_STATE		4
#define PPC_THREAD_STATE64		5
#define PPC_EXCEPTION_STATE64	6
#define THREAD_STATE_NONE		7
	       
/*
 * VALID_THREAD_STATE_FLAVOR is a platform specific macro that when passed
 * an exception flavor will return whether that is a defined flavor for
 * that platform.
 * The macro must be manually updated to include all of the valid exception
 * flavors as defined above.
 */
#define VALID_THREAD_STATE_FLAVOR(x)       \
        ((x == PPC_THREAD_STATE)        || \
         (x == PPC_FLOAT_STATE)         || \
	 (x == PPC_EXCEPTION_STATE)     	|| \
         (x == PPC_VECTOR_STATE)        || \
         (x == PPC_THREAD_STATE64)      || \
         (x == PPC_EXCEPTION_STATE64)   || \
         (x == THREAD_STATE_NONE))

typedef _STRUCT_PPC_THREAD_STATE	ppc_thread_state_t;
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
typedef _STRUCT_PPC_THREAD_STATE64	ppc_thread_state64_t;
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */
typedef _STRUCT_PPC_FLOAT_STATE		ppc_float_state_t;
typedef _STRUCT_PPC_VECTOR_STATE	ppc_vector_state_t;

/*
 * saved state structure
 *
 * This structure corresponds to the saved state. 
 *
 */

#ifdef	MACH__POSIX_C_SOURCE_PRIVATE

#include <ppc/savearea.h>

typedef struct savearea				ppc_saved_state_t;

#else	/* MACH__POSIX_C_SOURCE_PRIVATE */

typedef struct ppc_thread_state			ppc_saved_state_t;

#endif	/* MACH__POSIX_C_SOURCE_PRIVATE */

/*
 * ppc_exception_state
 *
 * This structure corresponds to some additional state of the user
 * registers as saved in the PCB upon kernel entry. They are only
 * available if an exception is passed out of the kernel, and even
 * then not all are guaranteed to be updated.
 *
 * Some padding is included in this structure which allows space for
 * servers to store temporary values if need be, to maintain binary
 * compatiblity.
 */

/* Exception state for 32-bit thread (on 32-bit processor) */
/* Still available on 64-bit processors, but may fall short */
/* of covering the full potential state (hi half available). */

typedef _STRUCT_PPC_EXCEPTION_STATE	ppc_exception_state_t;
#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
typedef _STRUCT_PPC_EXCEPTION_STATE64	ppc_exception_state64_t;
#endif /* (_POSIX_C_SOURCE && !_DARWIN_C_SOURCE) */

/*
 * Save State Flags
 */

#define PPC_THREAD_STATE_COUNT ((mach_msg_type_number_t) \
   (sizeof(ppc_thread_state_t) / sizeof(int)))

#define PPC_THREAD_STATE64_COUNT ((mach_msg_type_number_t) \
   (sizeof(ppc_thread_state64_t) / sizeof(int)))

#define PPC_EXCEPTION_STATE_COUNT ((mach_msg_type_number_t) \
   (sizeof(ppc_exception_state_t) / sizeof(int)))

#define PPC_EXCEPTION_STATE64_COUNT ((mach_msg_type_number_t) \
   (sizeof(ppc_exception_state64_t) / sizeof(int)))

#define PPC_FLOAT_STATE_COUNT ((mach_msg_type_number_t) \
   (sizeof(ppc_float_state_t) / sizeof(int)))

#define PPC_VECTOR_STATE_COUNT ((mach_msg_type_number_t) \
   (sizeof(ppc_vector_state_t) / sizeof(int)))

/*
 * Machine-independent way for servers and Mach's exception mechanism to
 * choose the most efficient state flavor for exception RPC's:
 */
#define MACHINE_THREAD_STATE		PPC_THREAD_STATE
#define MACHINE_THREAD_STATE_COUNT	PPC_THREAD_STATE_COUNT

/*
 * Largest state on this machine:
 */
#define THREAD_MACHINE_STATE_MAX	THREAD_STATE_MAX

#endif /* _MACH_PPC_THREAD_STATUS_H_ */

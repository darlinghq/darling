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
#ifndef __MACH_RIGHT_PRIVATE_H
#define __MACH_RIGHT_PRIVATE_H

#include <os/base.h>
#include <mach/mach.h>
#include <mach/port.h>
#include <mach/mach_port.h>
#include <sys/cdefs.h>
#include <stdbool.h>

__BEGIN_DECLS;

/*!
 * @typedef mach_right_recv_t
 * A type representing the receive right to a Mach port.
 */
typedef struct _mach_right_recv {
	mach_port_t mrr_name;
} mach_right_recv_t;

/*!
 * @const MACH_RIGHT_RECV_NULL
 * A convenience initializer for a receive right object.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define MACH_RIGHT_RECV_NULL ((mach_right_recv_t){MACH_PORT_NULL})
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define MACH_RIGHT_RECV_NULL (mach_right_recv_t{MACH_PORT_NULL})
#elif defined(__cplusplus)
#define MACH_RIGHT_RECV_NULL \
	        (mach_right_recv_t((mach_right_recv_t){MACH_PORT_NULL}))
#else
#define MACH_RIGHT_RECV_NULL {MACH_PORT_NULL}
#endif

/*!
 * @typedef mach_right_send_t
 * A type representing a send right to a Mach port.
 */
typedef struct _mach_right_send {
	mach_port_t mrs_name;
} mach_right_send_t;

/*!
 * @const MACH_RIGHT_SEND_NULL
 * A convenience initializer for a send right object.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define MACH_RIGHT_SEND_NULL ((mach_right_send_t){MACH_PORT_NULL})
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define MACH_RIGHT_SEND_NULL (mach_right_send_t{MACH_PORT_NULL})
#elif defined(__cplusplus)
#define MACH_RIGHT_SEND_NULL \
	        (mach_right_send_t((mach_right_send_t){MACH_PORT_NULL}))
#else
#define MACH_RIGHT_SEND_NULL {MACH_PORT_NULL}
#endif

/*!
 * @typedef mach_right_send_once_t
 * A type representing a send-once right to a Mach port.
 */
typedef struct _mach_right_send_once {
	mach_port_t mrso_name;
} mach_right_send_once_t;

/*!
 * @const MACH_RIGHT_SEND_ONCE_NULL
 * A convenience initializer for a send-once right object.
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define MACH_RIGHT_SEND_ONCE_NULL ((mach_right_send_once_t){MACH_PORT_NULL})
#elif defined(__cplusplus) && __cplusplus >= 201103L
#define MACH_RIGHT_SEND_ONCE_NULL (mach_right_send_once_t{MACH_PORT_NULL})
#elif defined(__cplusplus)
#define MACH_RIGHT_SEND_ONCE_NULL \
	        (mach_right_send_once_t((mach_right_send_once_t){MACH_PORT_NULL}))
#else
#define MACH_RIGHT_SEND_ONCE_NULL {MACH_PORT_NULL}
#endif

/*!
 * @function mach_right_recv
 * Wraps a port name as a receive right object.
 *
 * @param pn
 * The port name. If this name is valid but does not represent a receive right,
 * the behavior of mach_right_recv_* implementations is undefined.
 *
 * @result
 * A new receive right object.
 */
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline mach_right_recv_t
mach_right_recv(mach_port_name_t pn)
{
	mach_right_recv_t mrr = {pn};
	return mrr;
}

/*!
 * @function mach_right_send
 * Wraps a port name as a send right object.
 *
 * @param pn
 * The port name. If this name is valid but does not represent a send right, the
 * behavior of mach_right_send_* implementations is undefined.
 *
 * @result
 * A new send right object.
 */
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline mach_right_send_t
mach_right_send(mach_port_name_t pn)
{
	mach_right_send_t mrs = {pn};
	return mrs;
}

/*!
 * @function mach_right_send_valid
 * Checks if the given send right object is valid.
 *
 * @param mrs
 * The send right object to check.
 *
 * @result
 * A Boolean indicating whether the right is valid.
 */
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline bool
mach_right_send_valid(mach_right_send_t mrs)
{
	return MACH_PORT_VALID(mrs.mrs_name);
}

/*!
 * @function mach_right_send_once
 * Wraps a port name as a send-once right object.
 *
 * @param pn
 * The port name. If this name is valid but does not represent a send-once
 * right, the behavior of mach_right_send_once_* implementations is undefined.
 *
 * @result
 * A new send-once right object.
 */
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline mach_right_send_once_t
mach_right_send_once(mach_port_name_t pn)
{
	mach_right_send_once_t mrso = {pn};
	return mrso;
}

/*!
 * @function mach_right_send_once_valid
 * Checks if the given send-once right object is valid.
 *
 * @param mrso
 * The send-once right object to check.
 *
 * @result
 * A Boolean indicating whether the right is valid.
 */
OS_ALWAYS_INLINE OS_WARN_RESULT
static inline bool
mach_right_send_once_valid(mach_right_send_once_t mrso)
{
	return MACH_PORT_VALID(mrso.mrso_name);
}

/*!
 * @typedef mach_right_flags_t
 * Flags influencing the behavior of a constructed Mach port.
 *
 * @const MACH_RIGHT_RECV_INIT
 * No flags set. This value is suitable for initialization purposes.
 *
 * @const MACH_RIGHT_RECV_UNGUARDED
 * The given context should not serve as a guard for the underlying port's
 * destruction.
 */
OS_ENUM(mach_right_flags, uint64_t,
    MACH_RIGHT_RECV_FLAG_INIT = 0,
    MACH_RIGHT_RECV_FLAG_UNGUARDED = (1 << 0),
    );

/*!
 * @function mach_right_recv_construct
 * Allocates a new Mach port and returns the receive right to the caller.
 *
 * @param flags
 * Flags to influence the behavior of the new port.
 *
 * @param sr
 * If non-NULL, will be filled in with the name of a send right which
 * corresponds to the new port. The caller is responsible for disposing of this
 * send right with {@link mach_right_send_release}.
 *
 * @param ctx
 * Context to be associated with the new port. By default, this context must be
 * passed to {@link mach_right_recv_destruct} in order to destroy the underlying
 * port. This requirement may be elided with the
 * {@link MACH_RIGHT_RECV_UNGUARDED} flag.
 *
 * @result
 * A new port handle which refers to the receive right for the newly-created
 * port. The caller is responsible for disposing of this handle with
 * {@link mach_right_recv_destruct}.
 *
 * @discussion
 * The implementation will abort on any failure to allocate a new port object in
 * the kernel. Thus the caller may assert that a new, valid receive right is
 * always returned.
 */
OS_EXPORT OS_WARN_RESULT
mach_right_recv_t
mach_right_recv_construct(mach_right_flags_t flags,
    mach_right_send_t *_Nullable sr, uintptr_t ctx);

/*!
 * @function mach_right_recv_destruct
 * Closes the port referred to by the given receive right.
 *
 * @param r
 * The receive right for the port to manipulate.
 *
 * @param s
 * A pointer to the send right to dispose of. If NULL is given, no attempt will
 * be made to clean up any send right associated with the port. If the name of
 * the given send right does not match the name of the given receive right, the
 * implementation's behavior is undefined.
 *
 * @param ctx
 * The context which guards the underlying port destruction. If the receive
 * right was created with {@link MACH_RIGHT_RECV_UNGUARDED}, this parameter is
 * ignored.
 *
 * @discussion
 * If a send right is passed, the implementation performs the moral equivalent
 * of
 *
 *     mach_right_recv_destruct(r, MACH_PORT_NULL, ctx);
 *     mach_right_send_release(s);
 *
 * except in a more efficient manner, requiring only one system call.
 *
 * The implementation will abort on any failure to dispose of the port. As such,
 * this routine should only be used on ports that are known to be under the
 * caller's complete control.
 */
OS_EXPORT
void
mach_right_recv_destruct(mach_right_recv_t r, mach_right_send_t *_Nullable s,
    uintptr_t ctx);

/*!
 * @function mach_right_send_create
 * Creates a send right to the port referenced by the given receive right.
 *
 * @param r
 * The receive right for the port for which to create the send right.
 *
 * @result
 * The name of the new send right. The caller is responsible for disposing of
 * this send right with {@link mach_right_send_release}.
 *
 * This operation will increment the make-send count of the port referenced by
 * the given receive right.
 *
 * @discussion
 * The implementation will abort on any failure to create the send right. As
 * such, this routine should only be used on ports that are known to be under
 * the caller's complete control.
 */
OS_EXPORT OS_WARN_RESULT
mach_right_send_t
mach_right_send_create(mach_right_recv_t r);

/*!
 * @function mach_right_send_retain
 * Increments the user reference count for the given send right.
 *
 * @param s
 * The send right to manipulate.
 *
 * @result
 * If the reference count was successfully incremented, the given port name is
 * returned. If either MACH_PORT_NULL or MACH_PORT_DEAD are given, the given
 * value is returned. If the given send right became a dead name before or
 * during the attempt to retain the send right, MACH_PORT_DEAD is returned.
 *
 * If the implementation encounters any other failure condition, it will abort.
 */
OS_EXPORT OS_WARN_RESULT
mach_right_send_t
mach_right_send_retain(mach_right_send_t s);

/*!
 * @function mach_right_send_release
 * Decrements the user reference count for the given send right.
 *
 * @param s
 * The send right to manipulate.
 *
 * @discussion
 * If the given send right became a dead name before or during the attempt to
 * release it, the implementation will dispose of that dead name.
 *
 * If the implementation encounters any other failure condition, it will abort.
 */
OS_EXPORT
void
mach_right_send_release(mach_right_send_t s);

/*!
 * @function mach_right_send_once_create
 * Creates a send-once right from the given receive right.
 *
 * @param r
 * The receive right for the port for which to create the send-once right.
 *
 * @result
 * The newly-created send-once right.
 *
 * @discussion
 * The implementation will abort on any failure to allocate a new send-once
 * right, and therefore the caller should only provide a receive right which is
 * under its complete control. The caller may assert that a new, valid send-once
 * right is always returned.
 *
 * The returned send-once right will never share a name with the given receive
 * right. A send-once right must be consumed either by using it to send a
 * message or by consuming it with {@link mach_right_send_once_consume}.
 *
 * The returned right does not support retain/release semantics despite the
 * presence of "create" in the name.
 */
OS_EXPORT OS_WARN_RESULT
mach_right_send_once_t
mach_right_send_once_create(mach_right_recv_t r);

/*!
 * @function mach_right_send_once_consume
 * Consumes the given send-once right.
 *
 * @param so
 * The send-once right to manipulate.
 *
 * @discussion
 * If the given send-once right became a dead name before or during the attempt
 * to release it, the implementation will dispose of that dead name.
 *
 * If the implementation encounters any other failure condition, it will abort.
 *
 * This operation will cause a send-once notification to be delivered to the
 * port to which the send-once right refers unless the right is a dead name, in
 * which case there are no side effects.
 */
OS_EXPORT
void
mach_right_send_once_consume(mach_right_send_once_t so);

__END_DECLS;

#endif // __MACH_RIGHT_PRIVATE_H

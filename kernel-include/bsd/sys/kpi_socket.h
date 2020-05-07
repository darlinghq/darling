/*
 * Copyright (c) 2008-2017 Apple Inc. All rights reserved.
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
/*!
 *       @header kpi_socket.h
 *       This header defines an API for creating and interacting with sockets
 *       in the kernel. It is possible to create sockets in the kernel
 *       without an associated file descriptor. In some cases, a reference to
 *       the socket may be known while the file descriptor is not. These
 *       functions can be used for interacting with sockets in the kernel.
 *       The API is similar to the user space socket API.
 */
#ifndef __KPI_SOCKET__
#define __KPI_SOCKET__

#include <sys/types.h>
#include <sys/kernel_types.h>
#include <sys/socket.h>

#ifndef PRIVATE
#include <Availability.h>
#define __NKE_API_DEPRECATED __API_DEPRECATED("Network Kernel Extension KPI is deprecated", macos(10.4, 10.15))
#else
#define __NKE_API_DEPRECATED
#endif /* PRIVATE */

__BEGIN_DECLS

struct timeval;

/*!
 *       @typedef sock_upcall
 *
 *       @discussion sock_upcall is used by a socket to notify an in kernel
 *               client that data is waiting. Instead of making blocking calls in
 *               the kernel, a client can specify an upcall which will be called
 *               when data is available or the socket is ready for sending.
 *
 *               Calls to your upcall function are not serialized and may be
 *               called concurrently from multiple threads in the kernel.
 *
 *               Your upcall function will be called:
 *                   when there is data more than the low water mark for reading,
 *                   or when there is space for a write,
 *                   or when there is a connection to accept,
 *                   or when a socket is connected,
 *                   or when a socket is closed or disconnected
 *
 *       @param so A reference to the socket that's ready.
 *       @param cookie The cookie passed in when the socket was created.
 *       @param waitf Indicates whether or not it's safe to block.
 */
typedef void (*sock_upcall)(socket_t so, void *cookie, int waitf);

#ifdef KERNEL_PRIVATE
/*!
 *       @typedef sock_evupcall
 *
 *       @discussion sock_evupcall is used by a socket to notify an in kernel
 *               client when an event occurs. Instead of making blocking calls in
 *               the kernel, a client can specify an upcall which will be called
 *               when an event status is available.
 *       @param so A reference to the socket that's ready.
 *       @param cookie The cookie passed in when the socket was created.
 *       @param event Indicates the event as defined by SO_FILT_HINT_*
 */
typedef void (*sock_evupcall)(socket_t so, void *cookie, u_int32_t event);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function sock_accept
 *       @discussion Accepts an incoming connection on a socket. See 'man 2
 *               accept' for more information. Allocating a socket in this manner
 *               creates a socket with no associated file descriptor.
 *       @param so The listening socket you'd like to accept a connection on.
 *       @param from A pointer to a socket address that will be filled in
 *               with the address the connection is from.
 *       @param fromlen Maximum length of from.
 *       @param flags Supports MSG_DONTWAIT and MSG_USEUPCALL. If
 *               MSG_DONTWAIT is set, accept will return EWOULDBLOCK if there are
 *               no connections ready to be accepted. If MSG_USEUPCALL is set,
 *               the created socket will use the same upcall function attached to
 *               the original socket.
 *       @param callback A notifier function to be called when an event
 *               occurs on the socket. This may be NULL.
 *       @param cookie A cookie passed directly to the callback.
 *       @param new_so Upon success, *new_so will be a reference to a new
 *               socket for tracking the connection.
 *       @result 0 on success otherwise the errno error.
 */
#ifdef KERNEL_PRIVATE
extern errno_t sock_accept_internal(socket_t so, struct sockaddr *from, int fromlen,
    int flags, sock_upcall callback, void *cookie, socket_t *new_so);

#define sock_accept(so, from, fromlen, flags, callback, cookie, new_so) \
	sock_accept_internal((so), (from), (fromlen), (flags), (callback), \
	(cookie), (new_so))
#else
extern errno_t sock_accept(socket_t so, struct sockaddr *from, int fromlen,
    int flags, sock_upcall callback, void *cookie, socket_t *new_so)
__NKE_API_DEPRECATED;
#endif /* KERNEL_PRIVATE */

/*!
 *       @function sock_bind
 *       @discussion Binds a socket to a specific address. See 'man 2 bind'
 *               for more information.
 *       @param so The socket to be bound.
 *       @param to The local address the socket should be bound to.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_bind(socket_t so, const struct sockaddr *to)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_connect
 *       @discussion Initiates a connection on the socket. See 'man 2
 *               connect' for more information.
 *       @param so The socket to be connect.
 *       @param to The remote address the socket should connect to.
 *       @param flags Flags for connecting. The only flag supported so far is
 *               MSG_DONTWAIT. MSG_DONTWAIT will perform a non-blocking connect.
 *               sock_connect will return immediately with EINPROGRESS. The
 *               upcall, if supplied, will be called when the connection is
 *               completed.
 *       @result 0 on success, EINPROGRESS for a non-blocking connect that
 *               has not completed, otherwise the errno error.
 */
extern errno_t sock_connect(socket_t so, const struct sockaddr *to, int flags)
__NKE_API_DEPRECATED;

#ifdef KERNEL_PRIVATE
/*
 *       This function was added to support NFS. NFS does something funny,
 *       setting a short timeout and checking to see if it should abort the
 *       connect every two seconds. Ideally, NFS would use the upcall to be
 *       notified when the connect is complete.
 *
 *       If you feel you need to use this function, please contact us to
 *       explain why.
 *
 *       @function sock_connectwait
 *       @discussion Allows a caller to wait on a socket connect.
 *       @param so The socket being connected.
 *       @param tv The amount of time to wait.
 *       @result 0 on success otherwise the errno error. EINPROGRESS will be
 *               returned if the connection did not complete in the timeout
 *               specified.
 */
extern errno_t sock_connectwait(socket_t so, const struct timeval *tv);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function sock_getpeername
 *       @discussion Retrieves the remote address of a connected socket. See
 *               'man 2 getpeername'.
 *       @param so The socket.
 *       @param peername Storage for the peer name.
 *       @param peernamelen Length of storage for the peer name.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_getpeername(socket_t so, struct sockaddr *peername,
    int peernamelen)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_getsockname
 *       @discussion Retrieves the local address of a socket. See 'man 2
 *               getsockname'.
 *       @param so The socket.
 *       @param sockname Storage for the local name.
 *       @param socknamelen Length of storage for the socket name.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_getsockname(socket_t so, struct sockaddr *sockname,
    int socknamelen)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_getsockopt
 *       @discussion Retrieves a socket option. See 'man 2 getsockopt'.
 *       @param so The socket.
 *       @param level Level of the socket option.
 *       @param optname The option name.
 *       @param optval The option value.
 *       @param optlen The length of optval, returns the actual length.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_getsockopt(socket_t so, int level, int optname,
    void *optval, int *optlen)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_ioctl
 *       @discussion Performs an ioctl operation on a socket. See 'man 2 ioctl'.
 *       @param so The socket.
 *       @param request The ioctl name.
 *       @param argp The argument.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_ioctl(socket_t so, unsigned long request, void *argp)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_setsockopt
 *       @discussion Sets a socket option. See 'man 2 setsockopt'.
 *       @param so The socket.
 *       @param level Level of the socket option.
 *       @param optname The option name.
 *       @param optval The option value.
 *       @param optlen The length of optval.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_setsockopt(socket_t so, int level, int optname,
    const void *optval, int optlen)
__NKE_API_DEPRECATED;

#ifdef KERNEL_PRIVATE
/*
 *       This function was added to support AFP setting the traffic class
 *       for a backup stream within a wireless LAN or over link-local address.
 *
 *       If you feel you need to use this function, please contact us to
 *       explain why.
 *
 *       @function sock_settclassopt
 *       @discussion Allows a caller to set the traffic class.
 *       @param so The socket.
 *       @param optval The option value.
 *       @param optlen The length of optval.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_settclassopt(socket_t so, const void* optval, size_t optlen);

/*
 *       This function was added to support AFP getting the traffic class
 *       set on a stream.
 *
 *       This is also a private API, please contact us if you need to use it.
 *
 *       @function sockgettclassopt
 *       @discussion Allows a caller to get the traffic class.
 *       @param so The socket.
 *       @param optval The option value.
 *       @param optlen The length of optval, returns the actual length.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_gettclassopt(socket_t so, void* optval, size_t* optlen);

#ifdef XNU_KERNEL_PRIVATE
extern void socket_set_traffic_mgt_flags_locked(socket_t so, u_int8_t flags);
extern void socket_clear_traffic_mgt_flags_locked(socket_t so, u_int8_t flags);
#endif /* XNU_KERNEL_PRIVATE */
#ifdef BSD_KERNEL_PRIVATE
extern void socket_set_traffic_mgt_flags(socket_t so, u_int8_t flags);
extern void socket_clear_traffic_mgt_flags(socket_t so, u_int8_t flags);
extern errno_t socket_defunct(struct proc *, socket_t so, int);
extern errno_t sock_receive_internal(socket_t, struct msghdr *, mbuf_t *,
    int, size_t *);
#endif /* BSD_KERNEL_PRIVATE */
#endif /* KERNEL_PRIVATE */

/*!
 *       @function sock_listen
 *       @discussion Indicate that the socket should start accepting incoming
 *               connections. See 'man 2 listen'.
 *       @param so The socket.
 *       @param backlog The maximum length of the queue of pending connections.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_listen(socket_t so, int backlog)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_receive
 *       @discussion Receive data from a socket. Similar to recvmsg. See 'man
 *               2 recvmsg' for more information about receiving data.
 *       @param so The socket.
 *       @param msg The msg describing how the data should be received.
 *       @param flags See 'man 2 recvmsg'.
 *       @param recvdlen Number of bytes received, same as return value of
 *               userland recvmsg.
 *       @result 0 on success, EWOULDBLOCK if non-blocking and operation
 *               would cause the thread to block, otherwise the errno error.
 */
extern errno_t sock_receive(socket_t so, struct msghdr *msg, int flags,
    size_t *recvdlen)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_receivembuf
 *       @discussion Receive data from a socket. Similar to sock_receive
 *               though data is returned as a chain of mbufs. See 'man 2 recvmsg'
 *               for more information about receiving data.
 *       @param so The socket.
 *       @param msg The msg describing how the data should be received. May
 *               be NULL. The msg_iov is ignored.
 *       @param data Upon return *data will be a reference to an mbuf chain
 *               containing the data received. This eliminates copying the data
 *               out of the mbufs. Caller is responsible for freeing the mbufs.
 *       @param flags See 'man 2 recvmsg'.
 *       @param recvlen Maximum number of bytes to receive in the mbuf chain.
 *               Upon return, this value will be set to the number of bytes
 *               received, same as return value of userland recvmsg.
 *       @result 0 on success, EWOULDBLOCK if non-blocking and operation
 *               would cause the thread to block, otherwise the errno error.
 */
extern errno_t sock_receivembuf(socket_t so, struct msghdr *msg, mbuf_t *data,
    int flags, size_t *recvlen)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_send
 *       @discussion Send data on a socket. Similar to sendmsg. See 'man 2
 *               sendmsg' for more information about sending data.
 *       @param so The socket.
 *       @param msg The msg describing how the data should be sent. Any
 *               pointers must point to data in the kernel.
 *       @param flags See 'man 2 sendmsg'.
 *       @param sentlen The number of bytes sent.
 *       @result 0 on success, EWOULDBLOCK if non-blocking and operation
 *               would cause the thread to block, otherwise the errno error.
 */
extern errno_t sock_send(socket_t so, const struct msghdr *msg, int flags,
    size_t *sentlen)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_sendmbuf
 *       @discussion Send data in an mbuf on a socket. Similar to sock_send
 *               only the data to be sent is taken from the mbuf chain.
 *       @param so The socket.
 *       @param msg The msg describing how the data should be sent. The
 *               msg_iov is ignored. msg may be NULL.
 *       @param data The mbuf chain of data to send.
 *       @param flags See 'man 2 sendmsg'.
 *       @param sentlen The number of bytes sent.
 *       @result 0 on success, EWOULDBLOCK if non-blocking and operation
 *               would cause the thread to block, otherwise the errno error.
 *               Regardless of return value, the mbuf chain 'data' will be freed.
 */
extern errno_t sock_sendmbuf(socket_t so, const struct msghdr *msg, mbuf_t data,
    int flags, size_t *sentlen)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_shutdown
 *       @discussion Shutdown one or both directions of a connection. See
 *               'man 2 shutdown' for more information.
 *       @param so The socket.
 *       @param how SHUT_RD - shutdown receive.
 *               SHUT_WR - shutdown send.
 *               SHUT_RDWR - shutdown both.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_shutdown(socket_t so, int how)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_socket
 *       @discussion Allocate a socket. Allocating a socket in this manner
 *               creates a socket with no associated file descriptor. For more
 *               information, see 'man 2 socket'.
 *       @param domain The socket domain (PF_INET, etc...).
 *       @param type The socket type (SOCK_STREAM, SOCK_DGRAM, etc...).
 *       @param protocol The socket protocol.
 *       @param callback A notifier function to be called when an event
 *               occurs on the socket. This may be NULL.
 *       @param cookie A cookie passed directly to the callback.
 *       @param new_so Upon success, a reference to the new socket.
 *       @result 0 on success otherwise the errno error.
 */
#ifdef KERNEL_PRIVATE
extern errno_t sock_socket_internal(int domain, int type, int protocol,
    sock_upcall callback, void *cookie, socket_t *new_so);

#define sock_socket(domain, type, protocol, callback, cookie, new_so) \
	sock_socket_internal((domain), (type), (protocol), \
	(callback), (cookie), (new_so))
#else
extern errno_t sock_socket(int domain, int type, int protocol,
    sock_upcall callback, void *cookie, socket_t *new_so)
__NKE_API_DEPRECATED;
#endif /* KERNEL_PRIVATE */

/*!
 *       @function sock_close
 *       @discussion Close the socket.
 *       @param so The socket to close. This should only ever be a socket
 *               created with sock_socket. Closing a socket created in user space
 *               using sock_close may leave a file descriptor pointing to the
 *               closed socket, resulting in undefined behavior.
 */
extern void sock_close(socket_t so)
__NKE_API_DEPRECATED;

#ifdef KERNEL_PRIVATE
/*
 *       @function sock_retain
 *       @discussion Prevents the socket from closing
 *       @param so The socket to close.  Increment a retain count on the
 *               socket, preventing it from being closed when sock_close is
 *               called. This is used when a File Descriptor is passed (and
 *               closed) from userland and the kext wants to keep ownership of
 *               that socket. It is used in conjunction with
 *               sock_release(socket_t so).
 */
extern void sock_retain(socket_t so);

/*
 *       @function sock_release
 *       @discussion Decrement the retain count and close the socket if the
 *               retain count reaches zero.
 *       @param so The socket to release. This is used to release ownership
 *               on a socket acquired with sock_retain. When the last retain
 *               count is reached, this will call sock_close to close the socket.
 */
extern void sock_release(socket_t so);
#endif /* KERNEL_PRIVATE */

/*!
 *       @function sock_setpriv
 *       @discussion Set the privileged bit in the socket. Allows for
 *               operations that require root privileges.
 *       @param so The socket on which to modify the SS_PRIV flag.
 *       @param on Indicate whether or not the SS_PRIV flag should be set.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_setpriv(socket_t so, int on)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_isconnected
 *       @discussion Returns whether or not the socket is connected.
 *       @param so The socket to check.
 *       @result 0 - socket is not connected. 1 - socket is connected.
 */
extern int sock_isconnected(socket_t so)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_isnonblocking
 *       @discussion Returns whether or not the socket is non-blocking. In
 *               the context of this KPI, non-blocking means that functions to
 *               perform operations on a socket will not wait for completion.
 *
 *               To enable or disable blocking, use the FIONBIO ioctl. The
 *               parameter is an int. If the int is zero, the socket will block.
 *               If the parameter is non-zero, the socket will not block.
 *       @result 0 - socket will block. 1 - socket will not block.
 */
extern int sock_isnonblocking(socket_t so)
__NKE_API_DEPRECATED;

/*!
 *       @function sock_gettype
 *       @discussion Retrieves information about the socket. This is the same
 *               information that was used to create the socket. If any of the
 *               parameters following so are NULL, that information is not
 *               retrieved.
 *       @param so The socket to check.
 *       @param domain The domain of the socket (PF_INET, ...). May be NULL.
 *       @param type The socket type (SOCK_STREAM, SOCK_DGRAM, ...). May be NULL.
 *       @param protocol The socket protocol. May be NULL.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_gettype(socket_t so, int *domain, int *type, int *protocol)
__NKE_API_DEPRECATED;

#ifdef KERNEL_PRIVATE
/*
 *       @function sock_nointerrupt
 *       @discussion Disables interrupt on socket buffers (sets SB_NOINTR on
 *               send and receive socket buffers).
 *       @param so The socket to modify.
 *       @param on Indicate whether or not the SB_NOINTR flag should be set.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_nointerrupt(socket_t so, int on);

/*
 *       @function sock_getlistener
 *       @discussion Retrieves the listening socket of a pre-accepted socket,
 *               i.e. a socket which is still in the incomplete/completed list.
 *               Once a socket has been accepted, the information pertaining
 *               to its listener is no longer available.  Therefore, modules
 *               interested in finding out the listening socket should install
 *               the appropriate socket filter callback (sf_attach) which gets
 *               invoked prior to the socket being fully accepted, and call
 *               this routine at such a time to obtain the listener.  Callers
 *               are guaranteed that the listener socket will not go away
 *               during the sf_attach callback, and therefore the value is
 *               safe to be used only in that callback context.  Callers should
 *               therefore take note that the listening socket's lock will be
 *               held throughout the duration of the callback.
 *       @param so The pre-accepted socket.
 *       @result Non-NULL value which indicates the listening socket; otherwise,
 *               NULL if the socket is not in the incomplete/completed list
 *               of a listener.
 */
extern socket_t sock_getlistener(socket_t so);

/*
 *       @function sock_getaddr
 *       @discussion Retrieves the local or remote address of a socket.
 *               This is a composite of sock_getpeername and sock_getsockname,
 *               except that the allocated socket address is returned to the
 *               caller, and that the caller is reponsible for calling
 *               sock_freeaddr once finished with it.
 *       @param so The socket.
 *       @param psockname Pointer to the storage for the socket name.
 *       @param peername 0 for local address, and non-zero for peer address.
 *       @result 0 on success otherwise the errno error.
 */
extern errno_t sock_getaddr(socket_t so, struct sockaddr **psockname,
    int peername);

/*
 *       @function sock_freeaddr
 *       @discussion Frees the socket address allocated by sock_getaddr.
 *       @param sockname The socket name to be freed.
 */
extern void sock_freeaddr(struct sockaddr *sockname);

/*
 *       @function sock_setupcall
 *       @discussion Set the notifier function to be called when an event
 *               occurs on the socket. This may be set to NULL to disable
 *               further notifications. Setting the function does not
 *               affect currently notifications about to be sent or being sent.
 *               Note: When this function is used on a socket passed from
 *               userspace it is crucial to call sock_retain() on the socket
 *               otherwise a callback could be dispatched on a closed socket
 *               and cause a crash.
 *       @param sock The socket.
 *       @param callback The notifier function
 *       @param context A cookie passed directly to the callback
 */
extern errno_t sock_setupcall(socket_t sock, sock_upcall callback,
    void *context);

/*
 *       @function sock_setupcalls
 *       @discussion Set the notifier function to be called when an event
 *               occurs on the socket. This may be set to NULL to disable
 *               further notifications. Setting the function does not
 *               affect currently notifications about to be sent or being sent.
 *               Note: When this function is used on a socket passed from
 *               userspace it is crucial to call sock_retain() on the socket
 *               otherwise a callback could be dispatched on a closed socket
 *               and cause a crash.
 *       @param sock The socket.
 *       @param read_callback The read notifier function
 *       @param read_context A cookie passed directly to the read callback
 *       @param write_callback The write notifier function
 *       @param write_context A cookie passed directly to the write callback
 */
extern errno_t sock_setupcalls(socket_t sock, sock_upcall read_callback,
    void *read_context, sock_upcall write_callback, void *write_context);

/*
 *       @function sock_setupcalls_locked
 *       @discussion The locked version of sock_setupcalls
 *       @param locked: When sets, indicates that the callbacks expect to be
 *                      on a locked socket. Thus, no unlock is done prior to
 *                      calling the callback.
 */
extern void sock_setupcalls_locked(socket_t sock,
    sock_upcall rcallback, void *rcontext,
    sock_upcall wcallback, void *wcontext, int locked);

/*
 *       @function sock_catchevents
 *       @discussion Set the notifier function to be called when an event
 *               occurs on the socket. This may be set to NULL to disable
 *               further notifications. Setting the function does not
 *               affect currently notifications about to be sent or being sent.
 *       @param sock The socket.
 *       @param event_callback The event notifier function
 *       @param event_context A cookie passed directly to the event callback
 *       @param event_mask One or more SO_FILT_HINT_* values OR'ed together,
 *               indicating the registered event(s).
 */
extern errno_t sock_catchevents(socket_t sock, sock_evupcall event_callback,
    void *event_context, u_int32_t event_mask);

extern void sock_catchevents_locked(socket_t sock, sock_evupcall ecallback,
    void *econtext, u_int32_t emask);


/*
 *       @function sock_iskernel
 *       @discussion Returns true if the socket was created by the kernel or
 *               is owned by the kernel.
 *       @param sock The socket.
 *       @result True if the kernel owns the socket.
 */
extern int sock_iskernel(socket_t);
#endif /* KERNEL_PRIVATE */

__END_DECLS
#endif /* __KPI_SOCKET__ */

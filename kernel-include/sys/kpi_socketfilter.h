/*
 * Copyright (c) 2008 Apple Computer, Inc. All rights reserved.
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
	@header kpi_socketfilter.h
	This header defines an API for intercepting communications at the
	socket layer.

	For the most part, socket filters want to do three things: Filter
	data in and out, watch for state changes, and intercept a few calls
	for security. The number of function pointers supplied by a socket
	filter has been significantly reduced. The filter no longer has any
	knowledge of socket buffers. The filter no longer intercepts nearly
	every internal socket call. There are two data filters, an in
	filter, and an out filter. The in filter occurs before data is
	placed in the receive socket buffer. This is done to avoid waking
	the process unnecessarily. The out filter occurs before the data is
	appended to the send socket buffer. This should cover inbound and
	outbound data. For monitoring state changes, we've added a notify
	function that will be called when various events that the filter can
	not intercept occur. In addition, we've added a few functions that a
	filter may use to intercept common operations. These functions are:
	connect (inbound), connect (outbound), bind, set socket option,
	get socket option, and listen. Bind, listen, connect in, and connect
	out could be used together to build a fairly comprehensive firewall
	without having to do much with individual packets.
 */
#ifndef __KPI_SOCKETFILTER__
#define __KPI_SOCKETFILTER__

#include <sys/kernel_types.h>
#include <sys/kpi_socket.h>

struct sockaddr;

/*!
	@enum sflt_flags
	@abstract Constants defining mbuf flags. Only the flags listed below
		can be set or retrieved.
	@constant SFLT_GLOBAL Indicates this socket filter should be
		attached to all new sockets when they're created.
	@constant SFLT_PROG Indicates this socket filter should be attached
		only when request by the application using the SO_NKE socket
		option.
	@constant SFLT_EXTENDED	Indicates that this socket filter utilizes
		the extended fields within the sflt_filter structure.
	@constant SFLT_EXTENDED_REGISTRY Indicates that this socket filter
		wants to attach to all the sockets already present on the
		system. It will also receive notifications for these sockets.
*/
enum {
	SFLT_GLOBAL		= 0x01,
	SFLT_PROG		= 0x02,
	SFLT_EXTENDED		= 0x04,
	SFLT_EXTENDED_REGISTRY	= 0x08
};
typedef	u_int32_t	sflt_flags;

/*!
	@typedef sflt_handle
	@abstract A 4 byte identifier used with the SO_NKE socket option to
		identify the socket filter to be attached.
*/
typedef	u_int32_t	sflt_handle;

/*!
	@enum sflt_event_t
	@abstract Events notify a filter of state changes and other various
		events related to the socket. These events cannot be prevented
		or intercepted, only observed.
	@constant sock_evt_connected Indicates this socket has moved to the
		connected state.
	@constant sock_evt_disconnected Indicates this socket has moved to
		the disconnected state.
	@constant sock_evt_flush_read The read socket buffer has been
		flushed.
	@constant sock_evt_shutdown The read and or write side(s) of the
		connection have been shutdown. The param will point to an
		integer that indicates the direction that has been shutdown. See
		'man 2 shutdown' for more information.
	@constant sock_evt_cantrecvmore Indicates the socket cannot receive
		more data.
	@constant sock_evt_cantsendmore Indicates the socket cannot send
		more data.
	@constant sock_evt_closing Indicates the socket is closing.
	@constant sock_evt_bound Indicates this socket has moved to the
		bound state (only for PF_INET/PF_INET6 domain).
*/
enum {
	sock_evt_connecting		= 1,
	sock_evt_connected		= 2,
	sock_evt_disconnecting		= 3,
	sock_evt_disconnected		= 4,
	sock_evt_flush_read		= 5,
	sock_evt_shutdown		= 6, /* param points to an integer specifying how (read, write, or both) see man 2 shutdown */
	sock_evt_cantrecvmore		= 7,
	sock_evt_cantsendmore		= 8,
	sock_evt_closing		= 9,
	sock_evt_bound			= 10
};
typedef	u_int32_t	sflt_event_t;

/*!
	@enum sflt_data_flag_t
	@abstract Inbound and outbound data filters may handle many
		different types of incoming and outgoing data. These flags help
		distinguish between normal data, out-of-band data, and records.
	@constant sock_data_filt_flag_oob Indicates this data is out-of-band
		data.
	@constant sock_data_filt_flag_record Indicates this data is a
		record. This flag is only ever seen on inbound data.
*/
enum {
	sock_data_filt_flag_oob		= 1,
	sock_data_filt_flag_record	= 2
};
typedef	u_int32_t	sflt_data_flag_t;

__BEGIN_DECLS

/*!
	@typedef sf_unregistered_func

	@discussion sf_unregistered_func is called to notify the filter it
		has been unregistered. This is the last function the stack will
		call and this function will only be called once all other
		function calls in to your filter have completed. Once this
		function has been called, your kext may safely unload.
	@param handle The socket filter handle used to identify this filter.
*/
typedef	void (*sf_unregistered_func)(sflt_handle handle);

/*!
	@typedef sf_attach_func

	@discussion sf_attach_func is called to notify the filter it has
		been attached to a socket. The filter may allocate memory for
		this attachment and use the cookie to track it. This filter is
		called in one of two cases:
		1) You've installed a global filter and a new socket was created.
		2) Your non-global socket filter is being attached using the SO_NKE
		socket option.
	@param cookie Used to allow the socket filter to set the cookie for
		this attachment.
	@param so The socket the filter is being attached to.
	@result If you return a non-zero value, your filter will not be
		attached to this socket.
*/
typedef	errno_t	(*sf_attach_func)(void	**cookie, socket_t so);

/*!
	@typedef sf_detach_func

	@discussion sf_detach_func is called to notify the filter it has
		been detached from a socket. If the filter allocated any memory
		for this attachment, it should be freed. This function will
		be called when the socket is disposed of.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@result If you return a non-zero value, your filter will not be
		attached to this socket.
*/
typedef	void (*sf_detach_func)(void *cookie, socket_t so);

/*!
	@typedef sf_notify_func

	@discussion sf_notify_func is called to notify the filter of various
		state changes and other events occuring on the socket.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param event The type of event that has occurred.
	@param param Additional information about the event.
*/
typedef	void (*sf_notify_func)(void *cookie, socket_t so, sflt_event_t event,
    void *param);

/*!
	@typedef sf_getpeername_func

	@discussion sf_getpeername_func is called to allow a filter to
		to intercept the getpeername function. When called, sa will
		point to a pointer to a socket address that was malloced
		in zone M_SONAME. If you want to replace this address, either
		modify the currenty copy or allocate a new one and free the
		old one.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param sa A pointer to a socket address pointer.
	@result If you return a non-zero value, processing will stop. If
		you return EJUSTRETURN, no further filters will be called
		but a result of zero will be returned to the caller of
		getpeername.
*/
typedef	int (*sf_getpeername_func)(void *cookie, socket_t so,
    struct sockaddr **sa);

/*!
	@typedef sf_getsockname_func

	@discussion sf_getsockname_func is called to allow a filter to
		to intercept the getsockname function. When called, sa will
		point to a pointer to a socket address that was malloced
		in zone M_SONAME. If you want to replace this address, either
		modify the currenty copy or allocate a new one and free the
		old one.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param sa A pointer to a socket address pointer.
	@result If you return a non-zero value, processing will stop. If
		you return EJUSTRETURN, no further filters will be called
		but a result of zero will be returned to the caller of
		getsockname.
*/
typedef	int (*sf_getsockname_func)(void *cookie, socket_t so,
    struct sockaddr **sa);

/*!
	@typedef sf_data_in_func

	@discussion sf_data_in_func is called to filter incoming data. If your
		filter intercepts data for later reinjection, it must queue
		all incoming data to preserve the order of the data. Use
		sock_inject_data_in to later reinject this data if you return
		EJUSTRETURN. Warning: This filter is on the data path. Do not
		spend excesive time. Do not wait for data on another socket.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param from The addres the data is from, may be NULL if the socket
		is connected.
	@param data The data being received. Control data may appear in the
		mbuf chain, be sure to check the mbuf types to find control
		data.
	@param control Control data being passed separately from the data.
	@param flags Flags to indicate if this is out of band data or a
		record.
	@result Return:
		0 - The caller will continue with normal processing of the data.
		EJUSTRETURN - The caller will stop processing the data, the
			data will not be freed.
		Anything Else - The caller will free the data and stop
			processing.
*/
typedef	errno_t	(*sf_data_in_func)(void *cookie, socket_t so,
    const struct sockaddr *from, mbuf_t *data, mbuf_t *control,
    sflt_data_flag_t flags);

/*!
	@typedef sf_data_out_func

	@discussion sf_data_out_func is called to filter outbound data. If
		your filter intercepts data for later reinjection, it must queue
		all outbound data to preserve the order of the data when
		reinjecting. Use sock_inject_data_out to later reinject this
		data.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param from The address the data is from, may be NULL if the socket
		is connected.
	@param data The data being received. Control data may appear in the
		mbuf chain, be sure to check the mbuf types to find control
		data.
	@param control Control data being passed separately from the data.
	@param flags Flags to indicate if this is out of band data or a
		record.
	@result Return:
		0 - The caller will continue with normal processing of the data.
		EJUSTRETURN - The caller will stop processing the data,
			the data will not be freed.
		Anything Else - The caller will free the data and stop
			processing.
*/
typedef	errno_t	(*sf_data_out_func)(void *cookie, socket_t so,
    const struct sockaddr *to, mbuf_t *data, mbuf_t *control,
    sflt_data_flag_t flags);

/*!
	@typedef sf_connect_in_func

	@discussion sf_connect_in_func is called to filter inbound connections.
		A protocol will call this before accepting an incoming
		connection and placing it on the queue of completed connections.
		Warning: This filter is on the data path. Do not spend excesive
		time. Do not wait for data on another socket.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param from The address the incoming connection is from.
	@result Return:
		0 - The caller will continue with normal processing of the
			connection.
		Anything Else - The caller will rejecting the incoming
			connection.
*/
typedef	errno_t	(*sf_connect_in_func)(void *cookie, socket_t so,
    const struct sockaddr *from);

/*!
	@typedef sf_connect_out_func

	@discussion sf_connect_out_func is called to filter outbound
		connections. A protocol will call this before initiating an
		outbound connection.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param to The remote address of the outbound connection.
	@result Return:
		0 - The caller will continue with normal processing of the
			connection.
		Anything Else - The caller will rejecting the outbound
			connection.
*/
typedef	errno_t	(*sf_connect_out_func)(void *cookie, socket_t so,
    const struct sockaddr *to);

/*!
	@typedef sf_bind_func

	@discussion sf_bind_func is called before performing a bind
		operation on a socket.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param to The local address of the socket will be bound to.
	@result Return:
		0 - The caller will continue with normal processing of the bind.
		Anything Else - The caller will rejecting the bind.
*/
typedef	errno_t	(*sf_bind_func)(void *cookie, socket_t so,
    const struct sockaddr *to);

/*!
	@typedef sf_setoption_func

	@discussion sf_setoption_func is called before performing setsockopt
		on a socket.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param opt The socket option to set.
	@result Return:
		0 - The caller will continue with normal processing of the
			setsockopt.
		Anything Else - The caller will stop processing and return
			this error.
*/
typedef	errno_t	(*sf_setoption_func)(void *cookie, socket_t so, sockopt_t opt);

/*!
	@typedef sf_getoption_func

	@discussion sf_getoption_func is called before performing getsockopt
		on a socket.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param opt The socket option to get.
	@result Return:
		0 - The caller will continue with normal processing of the
			getsockopt.
		Anything Else - The caller will stop processing and return
			this error.
*/
typedef	errno_t	(*sf_getoption_func)(void *cookie, socket_t so, sockopt_t opt);

/*!
	@typedef sf_listen_func

	@discussion sf_listen_func is called before performing listen
		on a socket.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@result Return:
		0 - The caller will continue with normal processing of listen.
		Anything Else - The caller will stop processing and return
			this error.
*/
typedef	errno_t	(*sf_listen_func)(void *cookie, socket_t so);

/*!
	@typedef sf_ioctl_func

	@discussion sf_ioctl_func is called before performing an ioctl
		on a socket.

		All undefined ioctls are reserved for future use by Apple. If
		you need to communicate with your kext using an ioctl, please
		use SIOCSIFKPI and SIOCGIFKPI.
	@param cookie Cookie value specified when the filter attach was
		called.
	@param so The socket the filter is attached to.
	@param request The ioctl name.
	@param argp A pointer to the ioctl parameter.
	@result Return:
		0 - The caller will continue with normal processing of
			this ioctl.
		Anything Else - The caller will stop processing and return
			this error.
*/
typedef	errno_t	(*sf_ioctl_func)(void *cookie, socket_t so,
    unsigned long request, const char* argp);

/*!
	@typedef sf_accept_func

	@discussion sf_accept_func is called after a socket is dequeued
		off the completed (incoming) connection list and before
		the file descriptor is associated with it.  A filter can
		utilize this callback to intercept the accepted socket
		in order to examine it, prior to returning the socket to
		the caller of accept.  Such a filter may also choose to
		discard the accepted socket if it wishes to do so.
	@param cookie Cookie value specified when the filter attach was called.
	@param so_listen The listening socket.
	@param so The socket that is about to be accepted.
	@param local The local address of the about to be accepted socket.
	@param remote The remote address of the about to be accepted socket.
	@result Return:
		0 - The caller will continue with normal processing of accept.
		EJUSTRETURN - The to be accepted socket will be disconnected
		    prior to being returned to the caller of accept.  No further
		    control or data operations on the socket will be allowed.
		    This is the recommended return value as it has the least
		    amount of impact, especially to applications which don't
		    check the error value returned by accept.
		Anything Else - The to be accepted socket will be closed and
		    the error will be returned to the caller of accept.
		    Note that socket filter developers are advised to exercise
		    caution when returning non-zero values to the caller,
		    since some applications don't check the error value
		    returned by accept and therefore risk breakage.
 */
typedef errno_t (*sf_accept_func)(void *cookie, socket_t so_listen, socket_t so,
    const struct sockaddr *local, const struct sockaddr *remote);

/*!
	@struct sflt_filter
	@discussion This structure is used to define a socket filter.
	@field sf_handle A value used to find socket filters by
		applications. An application can use this value to specify that
		this filter should be attached when using the SO_NKE socket
		option.
	@field sf_flags Indicate whether this filter should be attached to
		all new sockets or just those that request the filter be
		attached using the SO_NKE socket option. If this filter
		utilizes the socket filter extension fields, it must also
		set SFLT_EXTENDED.
	@field sf_name A name used for debug purposes.
	@field sf_unregistered Your function for being notified when your
		filter has been unregistered.
	@field sf_attach Your function for handling attaches to sockets.
	@field sf_detach Your function for handling detaches from sockets.
	@field sf_notify Your function for handling events. May be null.
	@field sf_data_in Your function for handling incoming data. May be
		null.
	@field sf_data_out Your function for handling outgoing data. May be
		null.
	@field sf_connect_in Your function for handling inbound
		connections. May be null.
	@field sf_connect_out Your function for handling outbound
		connections. May be null.
	@field sf_bind Your function for handling binds. May be null.
	@field sf_setoption Your function for handling setsockopt. May be null.
	@field sf_getoption Your function for handling getsockopt. May be null.
	@field sf_listen Your function for handling listen. May be null.
	@field sf_ioctl Your function for handling ioctls. May be null.
	@field sf_len Length of socket filter extension structure; developers
		must initialize this to sizeof sflt_filter_ext structure.
		This field and all fields following it will only be valid
		if SFLT_EXTENDED flag is set in sf_flags field.
	@field sf_ext_accept Your function for handling inbound connections
		at accept time.  May be null.
	@field sf_ext_rsvd Reserved for future use; you must initialize
		the reserved fields with zeroes.
*/
struct sflt_filter {
	sflt_handle			sf_handle;
	int				sf_flags;
	char				*sf_name;

	sf_unregistered_func		sf_unregistered;
	sf_attach_func			sf_attach;
	sf_detach_func			sf_detach;

	sf_notify_func			sf_notify;
	sf_getpeername_func		sf_getpeername;
	sf_getsockname_func		sf_getsockname;
	sf_data_in_func			sf_data_in;
	sf_data_out_func		sf_data_out;
	sf_connect_in_func		sf_connect_in;
	sf_connect_out_func		sf_connect_out;
	sf_bind_func			sf_bind;
	sf_setoption_func		sf_setoption;
	sf_getoption_func		sf_getoption;
	sf_listen_func			sf_listen;
	sf_ioctl_func			sf_ioctl;
	/*
	 * The following are valid only if SFLT_EXTENDED flag is set.
	 * Initialize sf_ext_len to sizeof sflt_filter_ext structure.
	 * Filters must also initialize reserved fields with zeroes.
	 */
	struct sflt_filter_ext {
		unsigned int		sf_ext_len;
		sf_accept_func		sf_ext_accept;
		void			*sf_ext_rsvd[5];	/* Reserved */
	} sf_ext;
#define	sf_len		sf_ext.sf_ext_len
#define	sf_accept	sf_ext.sf_ext_accept
};

/*!
	@function sflt_register
	@discussion Registers a socket filter. See 'man 2 socket' for a
		desciption of domain, type, and protocol.
	@param filter A structure describing the filter.
	@param domain The protocol domain these filters will be attached to.
	@param type The socket type these filters will be attached to.
	@param protocol The protocol these filters will be attached to.
	@result 0 on success otherwise the errno error.
 */
extern errno_t sflt_register(const struct sflt_filter *filter, int domain,
    int type, int protocol);

/*!
	@function sflt_unregister
	@discussion Unregisters a socket filter. This will not detach the
		socket filter from all sockets it may be attached to at the
		time, it will just prevent the socket filter from being attached
		to any new sockets.
	@param handle The sf_handle of the socket filter to unregister.
	@result 0 on success otherwise the errno error.
 */
extern errno_t sflt_unregister(sflt_handle handle);

/*!
	@function sflt_attach
	@discussion Attaches a socket filter to the specified socket. A
		filter must be registered before it can be attached.
	@param socket The socket the filter should be attached to.
	@param handle The handle of the registered filter to be attached.
	@result 0 on success otherwise the errno error.
 */
extern errno_t sflt_attach(socket_t so, sflt_handle);

/*!
	@function sflt_detach
	@discussion Detaches a socket filter from a specified socket.
	@param socket The socket the filter should be detached from.
	@param handle The handle of the registered filter to be detached.
	@result 0 on success otherwise the errno error.
 */
extern errno_t sflt_detach(socket_t so, sflt_handle);

/* Functions for manipulating sockets */
/*
 * Inject data in to the receive buffer of the socket as if it
 * had come from the network.
 *
 * flags should match sflt_data_flag_t
 */

/*!
	@function sock_inject_data_in
	@discussion Inject data in to the receive buffer of the socket as if
		it had come from the network.
	@param so The socket to inject the data on.
	@param from The address the data is from, only necessary on
		un-connected sockets. A copy of the address will be made, caller
		is responsible for freeing the address after calling this
		function.
	@param data The data and possibly control mbufs.
	@param control The separate control mbufs.
	@param flags Flags indicating the type of data.
	@result 0 on success otherwise the errno error. If the function
		returns an error, the caller is responsible for freeing the
		mbuf.
 */
extern errno_t sock_inject_data_in(socket_t so, const struct sockaddr *from,
    mbuf_t data, mbuf_t control, sflt_data_flag_t flags);

/*!
	@function sock_inject_data_out
	@discussion Inject data in to the send buffer of the socket as if it
		had come from the client.
	@param so The socket to inject the data on.
	@param to The address the data should be sent to, only necessary on
		un-connected sockets. The caller is responsible for freeing the
		to address after sock_inject_data_out returns.
	@param data The data and possibly control mbufs.
	@param control The separate control mbufs.
	@param flags Flags indicating the type of data.
	@result 0 on success otherwise the errno error. The data and control
		values are always freed regardless of return value.
 */
extern errno_t sock_inject_data_out(socket_t so, const struct sockaddr *to,
    mbuf_t data, mbuf_t control, sflt_data_flag_t flags);


/*
 * sockopt_t accessors
 */

enum {
	sockopt_get	= 1,
	sockopt_set	= 2
};
typedef u_int8_t sockopt_dir;

/*!
	@function sockopt_direction
	@discussion Retrieves the direction of the socket option (Get or
		Set).
	@param sopt The socket option.
	@result sock_opt_get or sock_opt_set.
 */
extern sockopt_dir sockopt_direction(sockopt_t sopt);

/*!
	@function sockopt_level
	@discussion Retrieves the socket option level. (SOL_SOCKET, etc).
	@param sopt The socket option.
	@result The socket option level. See man 2 setsockopt
 */
extern int sockopt_level(sockopt_t sopt);

/*!
	@function sockopt_name
	@discussion Retrieves the socket option name. (SO_SNDBUF, etc).
	@param sopt The socket option.
	@result The socket option name. See man 2 setsockopt
 */
extern int sockopt_name(sockopt_t sopt);

/*!
	@function sockopt_valsize
	@discussion Retrieves the size of the socket option data.
	@param sopt The socket option.
	@result The length, in bytes, of the data.
 */
extern size_t sockopt_valsize(sockopt_t sopt);

/*!
	@function sockopt_copyin
	@discussion Copies the data from the socket option to a buffer.
	@param sopt The socket option.
	@param data A pointer to the buffer to copy the data in to.
	@param length The number of bytes to copy.
	@result An errno error or zero upon success.
 */
extern errno_t sockopt_copyin(sockopt_t sopt, void *data, size_t length);

/*!
	@function sockopt_copyout
	@discussion Copies the data from a buffer to a socket option.
	@param sopt The socket option.
	@param data A pointer to the buffer to copy the data out of.
	@param length The number of bytes to copy.
	@result An errno error or zero upon success.
 */
extern errno_t sockopt_copyout(sockopt_t sopt, void *data, size_t length);

__END_DECLS
#endif /* __KPI_SOCKETFILTER__ */

/*
 * Copyright (c) 2000-2004 Apple Computer, Inc. All rights reserved.
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
	@header kern_control.h
	This header defines an API to communicate between a kernel
	extension and a process outside of the kernel.
 */

#ifndef KPI_KERN_CONTROL_H
#define KPI_KERN_CONTROL_H


#include <sys/appleapiopts.h>

/*
 * Define Controller event subclass, and associated events.
 * Subclass of KEV_SYSTEM_CLASS
 */

/*!
	@defined KEV_CTL_SUBCLASS
    @discussion The kernel event subclass for kernel control events.
*/
#define KEV_CTL_SUBCLASS        2

/*!
	@defined KEV_CTL_REGISTERED
    @discussion The event code indicating a new controller was
    	registered. The data portion will contain a ctl_event_data.
*/
#define KEV_CTL_REGISTERED      1       /* a new controller appears */

/*!
	@defined KEV_CTL_DEREGISTERED
    @discussion The event code indicating a controller was unregistered.
    	The data portion will contain a ctl_event_data.
*/
#define KEV_CTL_DEREGISTERED    2       /* a controller disappears */

/*!
	@struct ctl_event_data
	@discussion This structure is used for KEV_CTL_SUBCLASS kernel
		events.
	@field ctl_id The kernel control id.
	@field ctl_unit The kernel control unit.
*/
struct ctl_event_data {
    u_int32_t	ctl_id;			/* Kernel Controller ID */
    u_int32_t 	ctl_unit;
};

/*
 * Controls destined to the Controller Manager.
 */

/*!
	@defined CTLIOCGCOUNT
    @discussion The CTLIOCGCOUNT ioctl can be used to determine the
    	number of kernel controllers registered.
*/
#define CTLIOCGCOUNT    _IOR('N', 2, int)		/* get number of control structures registered */

/*!
	@defined CTLIOCGINFO
    @discussion The CTLIOCGINFO ioctl can be used to convert a kernel
    	control name to a kernel control id.
*/
#define CTLIOCGINFO     _IOWR('N', 3, struct ctl_info)	/* get id from name */


/*!
	@defined MAX_KCTL_NAME
    @discussion Kernel control names must be no longer than
    	MAX_KCTL_NAME.
*/
#define MAX_KCTL_NAME	96

/*
 * Controls destined to the Controller Manager.
 */

/*!
	@struct ctl_info
	@discussion This structure is used with the CTLIOCGINFO ioctl to
		translate from a kernel control name to a control id.
	@field ctl_id The kernel control id, filled out upon return.
	@field ctl_name The kernel control name to find.
*/
struct ctl_info {
    u_int32_t	ctl_id; 				/* Kernel Controller ID  */
    char	ctl_name[MAX_KCTL_NAME];		/* Kernel Controller Name (a C string) */
};


/*!
	@struct sockaddr_ctl
	@discussion The controller address structure is used to establish
		contact between a user client and a kernel controller. The
		sc_id/sc_unit uniquely identify each controller. sc_id is a
		unique identifier assigned to the controller. The identifier can
		be assigned by the system at registration time or be a 32-bit
		creator code obtained from Apple Computer. sc_unit is a unit
		number for this sc_id, and is privately used by the kernel
		controller to identify several instances of the controller.
	@field sc_len The length of the structure.
	@field sc_family AF_SYSTEM.
	@field ss_sysaddr AF_SYS_KERNCONTROL.
	@field sc_id Controller unique identifier.
	@field sc_unit Kernel controller private unit number.
	@field sc_reserved Reserved, must be set to zero.
*/
struct sockaddr_ctl {
    u_char		sc_len;		/* depends on size of bundle ID string */
    u_char		sc_family;	/* AF_SYSTEM */
    u_int16_t 	ss_sysaddr;	/* AF_SYS_KERNCONTROL */
    u_int32_t	sc_id; 		/* Controller unique identifier  */
    u_int32_t 	sc_unit;	/* Developer private unit number */
    u_int32_t 	sc_reserved[5];
};

#ifdef KERNEL

#include <sys/kpi_mbuf.h>

/*!
	@typedef kern_ctl_ref
	@discussion A control reference is used to track an attached kernel
		control. Registering a kernel control will create a kernel
		control reference. This reference is required for sending data
		or removing the kernel control. This reference will be passed to
		callbacks for that kernel control.
*/
typedef void * kern_ctl_ref;

/*!
	@defined CTL_FLAG_PRIVILEGED
    @discussion The CTL_FLAG_PRIVILEGED flag is passed in ctl_flags. If
    	this flag is set, only privileged processes may attach to this
    	kernel control.
*/
#define CTL_FLAG_PRIVILEGED	0x1
/*!
	@defined CTL_FLAG_REG_ID_UNIT
    @discussion The CTL_FLAG_REG_ID_UNIT flag is passed to indicate that
    	the ctl_id specified should be used. If this flag is not
    	present, a unique ctl_id will be dynamically assigned to your
    	kernel control. The CTLIOCGINFO ioctl can be used by the client
    	to find the dynamically assigned id based on the control name
    	specified in ctl_name.
*/
#define CTL_FLAG_REG_ID_UNIT	0x2
/*!
	@defined CTL_FLAG_REG_SOCK_STREAM
    @discussion Use the CTL_FLAG_REG_SOCK_STREAM flag when client need to open 
    	socket of type SOCK_STREAM to communicate with the kernel control. 
    	By default kernel control sockets are of type SOCK_DGRAM.
*/
#define CTL_FLAG_REG_SOCK_STREAM	0x4

/* Data flags for controllers */
/*!
	@defined CTL_DATA_NOWAKEUP
    @discussion The CTL_DATA_NOWAKEUP flag can be used for the enqueue
    	data and enqueue mbuf functions to indicate that the process
    	should not be woken up yet. This is useful when you want to
    	enqueue data using more than one call but only want to wake up
    	the client after all of the data has been enqueued.
*/
#define CTL_DATA_NOWAKEUP	0x1
/*!
	@defined CTL_DATA_EOR
    @discussion The CTL_DATA_EOR flag can be used for the enqueue
    	data and enqueue mbuf functions to mark the end of a record.
*/
#define CTL_DATA_EOR		0x2

__BEGIN_DECLS

/*!
	@typedef ctl_connect_func
	@discussion The ctl_connect_func is used to receive
		notification of a client connecting to the kernel control.
	@param kctlref The control ref for the kernel control the client is
		connecting to.
	@param sac The address used to connect to this control. The field sc_unit 
		contains the unit number of the kernel control instance the client is
		connecting to. If CTL_FLAG_REG_ID_UNIT was set when the kernel control 
		was registered, sc_unit is the ctl_unit of the kern_ctl_reg structure. 
		If CTL_FLAG_REG_ID_UNIT was not set when the kernel control was 
		registered, sc_unit is the dynamically allocated unit number of 
		the new kernel control instance that is used for this connection.
	@param unitinfo A placeholder for a pointer to the optional user-defined
		private data associated with this kernel control instance.  This
		opaque info will be provided to the user when the rest of the
		callback routines are executed.  For example, it can be used
		to pass a pointer to an instance-specific data structure in
		order for the user to keep track of the states related to this
		kernel control instance.
 */
typedef errno_t (*ctl_connect_func)(kern_ctl_ref kctlref, 
									struct sockaddr_ctl *sac,
									void **unitinfo);

/*!
	@typedef ctl_disconnect_func
	@discussion The ctl_disconnect_func is used to receive notification
		that a client has disconnected from the kernel control. This
		usually happens when the socket is closed. If this is the last
		socket attached to your kernel control, you may unregister your
		kernel control from this callback.
	@param kctlref The control ref for the kernel control instance the client has
		disconnected from.
	@param unit The unit number of the kernel control instance the client has
		disconnected from.  
	@param unitinfo The user-defined private data initialized by the
		ctl_connect_func callback.
 */
typedef errno_t (*ctl_disconnect_func)(kern_ctl_ref kctlref, u_int32_t unit, void *unitinfo);

/*!
	@typedef ctl_send_func
	@discussion The ctl_send_func is used to receive data sent from
		the client to the kernel control.
	@param kctlref The control ref of the kernel control.
	@param unit The unit number of the kernel control instance the client has
		connected to.
	@param unitinfo The user-defined private data initialized by the
		ctl_connect_func callback.
	@param m The data sent by the client to the kernel control in an
		mbuf chain. Your function is responsible for releasing the
		mbuf chain.
	@param flags The flags specified by the client when calling
		send/sendto/sendmsg (MSG_OOB/MSG_DONTROUTE).
 */
typedef errno_t (*ctl_send_func)(kern_ctl_ref kctlref, u_int32_t unit, void *unitinfo,
								 mbuf_t m, int flags);

/*!
	@typedef ctl_setopt_func
	@discussion The ctl_setopt_func is used to handle set socket option
		calls for the SYSPROTO_CONTROL option level.
	@param kctlref The control ref of the kernel control.
	@param unit The unit number of the kernel control instance.
	@param unitinfo The user-defined private data initialized by the
		ctl_connect_func callback.
	@param opt The socket option.
	@param data A pointer to the socket option data. The data has
		already been copied in to the kernel for you.
	@param len The length of the socket option data.
 */
typedef errno_t (*ctl_setopt_func)(kern_ctl_ref kctlref, u_int32_t unit, void *unitinfo,
								   int opt, void *data, size_t len);

/*!
	@typedef ctl_getopt_func
	@discussion The ctl_getopt_func is used to handle client get socket
		option requests for the SYSPROTO_CONTROL option level. A buffer
		is allocated for storage and passed to your function. The length
		of that buffer is also passed. Upon return, you should set *len
		to length of the buffer used. In some cases, data may be NULL.
		When this happens, *len should be set to the length you would
		have returned had data not been NULL. If the buffer is too small,
		return an error.
	@param kctlref The control ref of the kernel control.
	@param unit The unit number of the kernel control instance.
	@param unitinfo The user-defined private data initialized by the
		ctl_connect_func callback.
	@param opt The socket option.
	@param data A buffer to copy the results in to. May be NULL, see
		discussion.
	@param len A pointer to the length of the buffer. This should be set
		to the length of the buffer used before returning.
 */
typedef errno_t (*ctl_getopt_func)(kern_ctl_ref kctlref, u_int32_t unit, void *unitinfo,
								   int opt, void *data, size_t *len);

/*!
	@struct kern_ctl_reg
	@discussion This structure defines the properties of a kernel
		control being registered.
	@field ctl_name A Bundle ID string of up to MAX_KCTL_NAME bytes (including the ending zero).
		This string should not be empty.
	@field ctl_id The control ID may be dynamically assigned or it can be a
		32-bit creator code assigned by DTS. 
		For a DTS assigned creator code the CTL_FLAG_REG_ID_UNIT flag must be set. 
		For a dynamically assigned control ID, do not set the CTL_FLAG_REG_ID_UNIT flag.
		The  value of the dynamically assigned control ID is set to this field 
		when the registration succeeds.
	@field ctl_unit A separate unit number to register multiple units that 
		share the same control ID with DTS assigned creator code when 
		the CTL_FLAG_REG_ID_UNIT flag is set.
		This field is ignored for a dynamically assigned control ID.
	@field ctl_flags CTL_FLAG_PRIVILEGED and/or CTL_FLAG_REG_ID_UNIT.
	@field ctl_sendsize Override the default send size. If set to zero,
		the default send size will be used, and this default value 
		is set to this field to be retrieved by the caller. 
	@field ctl_recvsize Override the default receive size. If set to
		zero, the default receive size will be used, and this default value 
		is set to this field to be retrieved by the caller. 
	@field ctl_connect Specify the  function to be called whenever a client
		connects to the kernel control. This field must be specified.
	@field ctl_disconnect Specify a function to be called whenever a
		client disconnects from the kernel control.
	@field ctl_send Specify a function to handle data send from the
		client to the kernel control.
	@field ctl_setopt Specify a function to handle set socket option
		operations for the kernel control.
	@field ctl_getopt Specify a function to handle get socket option
		operations for the kernel control.
*/
struct kern_ctl_reg
{
	/* control information */
	char		ctl_name[MAX_KCTL_NAME];
	u_int32_t	ctl_id;
	u_int32_t	ctl_unit;

    /* control settings */
    u_int32_t	ctl_flags;
    u_int32_t	ctl_sendsize;
    u_int32_t	ctl_recvsize;

    /* Dispatch functions */
    ctl_connect_func	ctl_connect;
    ctl_disconnect_func	ctl_disconnect;
    ctl_send_func		ctl_send;
    ctl_setopt_func		ctl_setopt;
    ctl_getopt_func		ctl_getopt;
};

/*!
	@function ctl_register
	@discussion Register a kernel control. This will enable clients to
		connect to the kernel control using a PF_SYSTEM socket.
	@param userkctl A structure defining the kernel control to be
		attached. The ctl_connect callback must be specified, the other callbacks 
		are optional. If ctl_connect is set to zero, ctl_register fails with 
		the error code EINVAL.
	@param kctlref Upon successful return, the kctlref will contain a
		reference to the attached kernel control. This reference is used
		to unregister the kernel control. This reference will also be
		passed in to the callbacks each time they are called.
	@result 0 - Kernel control was registered.
		EINVAL - The registration structure was not valid.
		ENOMEM - There was insufficient memory.
		EEXIST - A controller with that id/unit is already registered.
 */
errno_t
ctl_register(struct kern_ctl_reg *userkctl, kern_ctl_ref *kctlref);	

/*!
	@function ctl_deregister
	@discussion Unregister a kernel control. A kernel extension must
		unregister it's kernel control(s) before unloading. If a kernel
		control has clients attached, this call will fail.
	@param kctlref The control reference of the control to unregister.
	@result 0 - Kernel control was unregistered.
		EINVAL - The kernel control reference was invalid.
		EBUSY - The kernel control has clients still attached.
 */
errno_t 
ctl_deregister(kern_ctl_ref kctlref);	

/*!
	@function ctl_enqueuedata
	@discussion Send data from the kernel control to the client.
	@param kctlref The control reference of the kernel control.
	@param unit The unit number of the kernel control instance.
	@param data A pointer to the data to send.
	@param len The length of data to send.
	@param flags Send flags. CTL_DATA_NOWAKEUP is currently the only
		supported flag.
	@result 0 - Data was enqueued to be read by the client.
		EINVAL - Invalid parameters.
		EMSGSIZE - The buffer is too large.
		ENOBUFS - The queue is full or there are no free mbufs.
 */
errno_t 
ctl_enqueuedata(kern_ctl_ref kctlref, u_int32_t unit, void *data, size_t len, u_int32_t flags);

/*!
	@function ctl_enqueuembuf
	@discussion Send data stored in an mbuf chain from the kernel
		control to the client. The caller is responsible for freeing
		the mbuf chain if ctl_enqueuembuf returns an error.
	@param kctlref The control reference of the kernel control.
	@param unit The unit number of the kernel control instance.
	@param m An mbuf chain containing the data to send to the client.
	@param flags Send flags. CTL_DATA_NOWAKEUP is currently the only
		supported flag.
	@result 0 - Data was enqueued to be read by the client.
		EINVAL - Invalid parameters.
		ENOBUFS - The queue is full.
 */
errno_t 
ctl_enqueuembuf(kern_ctl_ref kctlref, u_int32_t unit, mbuf_t m, u_int32_t flags);


/*!
	@function ctl_getenqueuespace
	@discussion Retrieve the amount of space currently available for data to be sent 
		from the kernel control to the client.
	@param kctlref The control reference of the kernel control.
	@param unit The unit number of the kernel control instance.
	@param space The address where to return the current space available 
	@result 0 - Success; the amount of space is returned to caller.
		EINVAL - Invalid parameters.
 */
errno_t 
ctl_getenqueuespace(kern_ctl_ref kctlref, u_int32_t unit, size_t *space);

#ifdef KERNEL_PRIVATE
u_int32_t ctl_id_by_name(const char *name);
errno_t ctl_name_by_id(u_int32_t id, char *out_name, size_t maxsize);
#endif /* KERNEL_PRIVATE */

__END_DECLS
#endif /* KERNEL */

#endif /* KPI_KERN_CONTROL_H */


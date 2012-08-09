/*
 * Copyright (c) 2003 Apple Computer, Inc. All rights reserved.
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
/* 
 * Mach Operating System
 * Copyright (c) 1991 Carnegie Mellon University
 * All Rights Reserved.
 * 
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 * 
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 * 
 * Carnegie Mellon requests users of this software to return to
 * 
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 * 
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *	File:	mach/host_special_ports.h
 *
 *	Defines codes for access to host-wide special ports.
 */

#ifndef _MACH_HOST_SPECIAL_PORTS_H_
#define _MACH_HOST_SPECIAL_PORTS_H_

/*
 * Cannot be set or gotten from user space
 */
#define HOST_SECURITY_PORT               0

/*
 * Always provided by kernel (cannot be set from user-space).
 */
#define HOST_PORT                        1
#define HOST_PRIV_PORT                   2
#define HOST_IO_MASTER_PORT              3
#define HOST_MAX_SPECIAL_KERNEL_PORT     7 /* room to grow */

/*
 * Not provided by kernel
 */
#define HOST_DYNAMIC_PAGER_PORT         (1 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_AUDIT_CONTROL_PORT         (2 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_USER_NOTIFICATION_PORT     (3 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_AUTOMOUNTD_PORT            (4 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_LOCKD_PORT                 (5 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_SEATBELT_PORT              (7 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_KEXTD_PORT                 (8 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_CHUD_PORT                  (9 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_UNFREED_PORT		(10 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_AMFID_PORT			(11 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_GSSD_PORT			(12 + HOST_MAX_SPECIAL_KERNEL_PORT)
#define HOST_MAX_SPECIAL_PORT           (13 + HOST_MAX_SPECIAL_KERNEL_PORT)
                                        /* room to grow here as well */

/*
 * Special node identifier to always represent the local node.
 */
#define HOST_LOCAL_NODE	                 -1
			
/*
 * Definitions for ease of use.
 *
 * In the get call, the host parameter can be any host, but will generally
 * be the local node host port. In the set call, the host must the per-node
 * host port for the node being affected.
 */
#define host_get_host_port(host, port)	\
	(host_get_special_port((host), 	\
	HOST_LOCAL_NODE, HOST_PORT, (port)))
#define host_set_host_port(host, port) (KERN_INVALID_ARGUMENT)

#define host_get_host_priv_port(host, port)	\
	(host_get_special_port((host), 		\
	HOST_LOCAL_NODE, HOST_PRIV_PORT, (port)))
#define host_set_host_priv_port(host, port) (KERN_INVALID_ARGUMENT)

#define host_get_io_master_port(host, port)	\
	(host_get_special_port((host), 		\
	HOST_LOCAL_NODE, HOST_IO_MASTER_PORT, (port)))
#define host_set_io_master_port(host, port) (KERN_INVALID_ARGUMENT)

/*
 * User-settable special ports.
 */
#define host_get_dynamic_pager_port(host, port)	\
	(host_get_special_port((host),			\
	HOST_LOCAL_NODE, HOST_DYNAMIC_PAGER_PORT, (port)))
#define host_set_dynamic_pager_port(host, port)	\
	(host_set_special_port((host), HOST_DYNAMIC_PAGER_PORT, (port)))

#define host_get_audit_control_port(host, port)	\
	(host_get_special_port((host), 		\
	HOST_LOCAL_NODE, HOST_AUDIT_CONTROL_PORT, (port)))
#define host_set_audit_control_port(host, port)	\
	(host_set_special_port((host), HOST_AUDIT_CONTROL_PORT, (port)))

#define host_get_user_notification_port(host, port)	\
	(host_get_special_port((host), 			\
	HOST_LOCAL_NODE, HOST_USER_NOTIFICATION_PORT, (port)))
#define host_set_user_notification_port(host, port)	\
	(host_set_special_port((host), HOST_USER_NOTIFICATION_PORT, (port)))

#define host_get_automountd_port(host, port)	\
	(host_get_special_port((host), 		\
	HOST_LOCAL_NODE, HOST_AUTOMOUNTD_PORT, (port)))
#define host_set_automountd_port(host, port)	\
	(host_set_special_port((host), HOST_AUTOMOUNTD_PORT, (port)))

#define host_get_lockd_port(host, port)	\
	(host_get_special_port((host), 			\
	HOST_LOCAL_NODE, HOST_LOCKD_PORT, (port)))
#define host_set_lockd_port(host, port)	\
	(host_set_special_port((host), HOST_LOCKD_PORT, (port)))

#define host_get_kextd_port(host, port)	\
	(host_get_special_port((host), 			\
	HOST_LOCAL_NODE, HOST_KEXTD_PORT, (port)))
#define host_set_kextd_port(host, port)	\
	(host_set_special_port((host), HOST_KEXTD_PORT, (port)))

#define host_get_chud_port(host, port)	\
	(host_get_special_port((host), 			\
	HOST_LOCAL_NODE, HOST_CHUD_PORT, (port)))
#define host_set_chud_port(host, port)	\
	(host_set_special_port((host), HOST_CHUD_PORT, (port)))

#define host_get_unfreed_port(host, port)	\
	(host_get_special_port((host), 			\
	HOST_LOCAL_NODE, HOST_UNFREED_PORT, (port)))
#define host_set_unfreed_port(host, port)	\
	(host_set_special_port((host), HOST_UNFREED_PORT, (port)))

#define host_get_amfid_port(host, port)	\
	(host_get_special_port((host), 			\
	HOST_LOCAL_NODE, HOST_AMFID_PORT, (port)))
#define host_set_amfid_port(host, port)	\
	(host_set_special_port((host), HOST_AMFID_PORT, (port)))

#define host_get_gssd_port(host, port)	\
	(host_get_special_port((host),			\
	HOST_LOCAL_NODE, HOST_GSSD_PORT, (port)))

#define host_set_gssd_port(host, port)	\
	(host_set_special_port((host), HOST_GSSD_PORT, (port)))

#endif	/* _MACH_HOST_SPECIAL_PORTS_H_ */

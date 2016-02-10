/*
 * Copyright (c) 2012, 2013 Apple Inc. All rights reserved.
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

#ifndef _LIBSYSTEMCONFIGURATION_CLIENT_H
#define _LIBSYSTEMCONFIGURATION_CLIENT_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>

// ------------------------------------------------------------

#pragma mark -
#pragma mark [XPC] DNS configuration server

#define	DNSINFO_SERVER_VERSION		20130408

#if	!TARGET_IPHONE_SIMULATOR
#define	DNSINFO_SERVICE_NAME		"com.apple.SystemConfiguration.DNSConfiguration"
#else	// !TARGET_IPHONE_SIMULATOR
#define	DNSINFO_SERVICE_NAME		"com.apple.SystemConfiguration.DNSConfiguration_sim"
#endif	// !TARGET_IPHONE_SIMULATOR

#define	DNSINFO_PROC_NAME		"proc_name"	// string

#define	DNSINFO_REQUEST			"request_op"	// int64

enum {
	DNSINFO_REQUEST_COPY		= 0x10001,
	DNSINFO_REQUEST_ACKNOWLEDGE,
};

#define	DNSINFO_CONFIGURATION		"configuration"	// data
#define	DNSINFO_GENERATION		"generation"	// uint64

// ------------------------------------------------------------

#pragma mark -
#pragma mark [XPC] Network information (nwi) server

#define	NWI_SERVER_VERSION		20130408

#if	!TARGET_IPHONE_SIMULATOR
#define	NWI_SERVICE_NAME		"com.apple.SystemConfiguration.NetworkInformation"
#else	// !TARGET_IPHONE_SIMULATOR
#define	NWI_SERVICE_NAME		"com.apple.SystemConfiguration.NetworkInformation_sim"
#endif	// !TARGET_IPHONE_SIMULATOR

#define	NWI_PROC_NAME			"proc_name"	// string

#define	NWI_REQUEST			"request_op"	// int64

enum {
	NWI_REQUEST_COPY		= 0x20001,
	NWI_REQUEST_ACKNOWLEDGE,
};

#define	NWI_CONFIGURATION		"configuration"	// data
#define	NWI_GENERATION			"generation"	// uint64

// ------------------------------------------------------------

typedef struct {
	_Bool			active;
	xpc_connection_t	connection;
	char			*service_description;
	char			*service_name;
} libSC_info_client_t;

// ------------------------------------------------------------

__BEGIN_DECLS

libSC_info_client_t *
libSC_info_client_create		(
					 dispatch_queue_t	q,
					 const char		*service_name,
					 const char		*service_description
					);

void
libSC_info_client_release		(
					 libSC_info_client_t	*client
					);

xpc_object_t
libSC_send_message_with_reply_sync	(
					 libSC_info_client_t	*client,
					 xpc_object_t		message
					);

__END_DECLS

#endif // _LIBSYSTEMCONFIGURATION_CLIENT_H

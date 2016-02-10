/*
 * Copyright (c) 2012 Apple Inc. All rights reserved.
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

#ifndef _LIBSYSTEMCONFIGURATION_SERVER_H
#define _LIBSYSTEMCONFIGURATION_SERVER_H

#include <Availability.h>
#include <TargetConditionals.h>
#include <sys/cdefs.h>
#include <dispatch/dispatch.h>
#include <xpc/xpc.h>
#include <CoreFoundation/CoreFoundation.h>

// ------------------------------------------------------------

#pragma mark -
#pragma mark [XPC] information server common code

// ------------------------------------------------------------


/*
 * libSC_info_server_t
 *	data (CFData)
 *		stored configuration
 *	generation
 *		generation of the last stored configuration
 *	info (CFDictionary)
 *		key = xpc_connection_t [CFData]
 *		val = [CFData] (process name, last push'd, last ack'd)
 *	inSync_NO, inSync_YES
 *		count of client connections that have ack'd a configuration that
 *		are (or are not) in sync with the last stored generation#
*/
typedef struct {
	CFDataRef		data;
	uint64_t		generation;
	CFMutableDictionaryRef	info;
	int			inSync_NO;	// ack'ing clients not in sync w/generation
	int			inSync_YES;	// ack'ing clients in sync w/generation
} libSC_info_server_t;


__BEGIN_DECLS

// Global server info SPIs

void
_libSC_info_server_init		(
				 libSC_info_server_t	*server_info
				);

Boolean
_libSC_info_server_in_sync	(
				 libSC_info_server_t	*server_info
				);

void
_libSC_info_server_set_data	(
				 libSC_info_server_t	*server_info,
				 CFDataRef		data,
				 uint64_t		generation
				);

// Per-session server info SPIs

void
_libSC_info_server_open		(
				 libSC_info_server_t	*server_info,
				 xpc_connection_t	c
				);

CFDataRef
_libSC_info_server_get_data	(
				 libSC_info_server_t	*server_info,
				 xpc_connection_t	c,
				 uint64_t		*generation
				);

Boolean
_libSC_info_server_acknowledged	(
				 libSC_info_server_t	*server_info,
				 xpc_connection_t	c,
				 uint64_t		generation
				);

Boolean
_libSC_info_server_close	(
				 libSC_info_server_t	*server_info,
				 xpc_connection_t	c
				);

__END_DECLS

#endif // _LIBSYSTEMCONFIGURATION_SERVER_H

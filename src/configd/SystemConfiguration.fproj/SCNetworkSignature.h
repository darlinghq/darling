/*
 * Copyright (c) 2006, 2008, 2011, 2012 Apple Inc. All rights reserved.
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

#ifndef _SCNETWORKSIGNATURE_H
#define _SCNETWORKSIGNATURE_H

#include <Availability.h>
#include <sys/cdefs.h>
#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFArray.h>
#include <netinet/in.h>

__BEGIN_DECLS

/*!
	@header SCNetworkSignature
	@discussion The SCNetworkSignature API provides access to the
		network identification information.  Each routable network
		has an associated signature that is assigned a unique
		identifier.  The unique identifier allows an application
		to associate settings with a particular network
		or set of networks.
 */

/*!
	@function SCNetworkSignatureCopyActiveIdentifiers
	@discussion Find all currently active networks and return a list of
		(string) identifiers, one for each network.
	@param allocator The CFAllocator that should be used to allocate
		memory for the local dynamic store object.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@result A CFArrayRef containing a list of (string) identifiers,
		NULL if no networks are currently active.
 */
CFArrayRef /* of CFStringRef's */
SCNetworkSignatureCopyActiveIdentifiers(CFAllocatorRef alloc)			__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0/*SPI*/);

/*!
	@function SCNetworkSignatureCopyActiveIdentifierForAddress
	@discussion Find the one active network associated with the specified
		address and return the unique (string) identifier that
		represents it.
	@param allocator The CFAllocator that should be used to allocate
		memory for the local dynamic store object.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param addr The address of interest.  Note: currently only AF_INET
		0.0.0.0 is supported, passing anything else always returns
		NULL.
	@result The unique (string) identifier associated with the address,
		NULL if no network is known.
 */
CFStringRef
SCNetworkSignatureCopyActiveIdentifierForAddress(CFAllocatorRef alloc,
						 const struct sockaddr * addr)	__OSX_AVAILABLE_STARTING(__MAC_10_5,__IPHONE_2_0/*SPI*/);

/*!
	@function SCNetworkSignatureCopyIdentifierForConnectedSocket
	@discussion Find the identifier for the given file descriptor
		corresponding to a connected socket.
	@param allocator The CFAllocator that should be used to allocate
		memory for the local dynamic store object.
		This parameter may be NULL in which case the current
		default CFAllocator is used. If this reference is not
		a valid CFAllocator, the behavior is undefined.
	@param sock_fd The socket file descriptor, must be either AF_INET
		or AF_INET6.
	@result The unique (string) identifier associated with the address,
		NULL if no network is known.
 */
CFStringRef
SCNetworkSignatureCopyIdentifierForConnectedSocket(CFAllocatorRef alloc,
						   int sock_fd) __OSX_AVAILABLE_STARTING(__MAC_10_7,__IPHONE_5_0/*SPI*/);

__END_DECLS

#endif	/* _SCNETWORKSIGNATURE_H */

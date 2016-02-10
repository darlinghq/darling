/*
 * Copyright (c) 2002-2009 Apple Inc. All rights reserved.
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

#ifndef _NETDB_ASYNC_H_
#define _NETDB_ASYNC_H_

#include <sys/cdefs.h>
#include <mach/mach.h>
#include <netdb.h>

#define gethostbyname_async_handle_reply gethostbyname_async_handleReply
#define gethostbyaddr_async_handle_reply gethostbyaddr_async_handleReply
#define getipnodebyaddr_async_handle_reply getipnodebyaddr_async_handleReply
#define getipnodebyname_async_handle_reply getipnodebyname_async_handleReply

/* SPI for parallel / fast getaddrinfo */
#define	AI_PARALLEL	0x00000008
#define AI_SRV		0x01000000

__BEGIN_DECLS

 
/*
 * getaddrinfo
 */
typedef void (*getaddrinfo_async_callback)(int32_t status, struct addrinfo *res, void *context);
int32_t getaddrinfo_async_start(mach_port_t *p, const char *nodename, const char *servname, const struct addrinfo *hints, getaddrinfo_async_callback callback, void *context);
int32_t getaddrinfo_async_send(mach_port_t *p, const char *nodename, const char *servname, const struct addrinfo *hints);
int32_t getaddrinfo_async_receive(mach_port_t p, struct addrinfo **res);
int32_t getaddrinfo_async_handle_reply(void *msg);
void getaddrinfo_async_cancel(mach_port_t p);

 
/*
 * getnameinfo
 */
typedef void (*getnameinfo_async_callback)(int32_t status, char *host, char *serv, void *context);
int32_t getnameinfo_async_start(mach_port_t *p, const struct sockaddr *sa, size_t salen, int flags, getnameinfo_async_callback callback, void *context);
int32_t getnameinfo_async_send(mach_port_t *p, const struct sockaddr *sa, size_t salen, int flags);
int32_t getnameinfo_async_receive(mach_port_t p, char **host, char **serv);
int32_t getnameinfo_async_handle_reply(void *msg);
void getnameinfo_async_cancel(mach_port_t p);

/*
 * DNS
 */
typedef void (*dns_async_callback)(int32_t status, char *buf, uint32_t len, struct sockaddr *from, int fromlen, void *context);
int32_t dns_async_start(mach_port_t *p, const char *name, uint16_t dnsclass, uint16_t dnstype, uint32_t do_search, dns_async_callback callback, void *context);
int32_t dns_async_send(mach_port_t *p, const char *name, uint16_t dnsclass, uint16_t dnstype, uint32_t do_search);
int32_t dns_async_receive(mach_port_t p, char **buf, uint32_t *len, struct sockaddr **from, uint32_t *fromlen);
int32_t dns_async_handle_reply(void *msg);
void dns_async_cancel(mach_port_t p);

/*
 * Host lookup
 */

/*
 @typedef gethostbyaddr_async_callback
 @discussion Type of the callback function used when a
	gethostbyaddr_async_start() request is delivered.
 @param hent The resolved host entry.
 @param context The context provided when the request
	was initiated.
 */
typedef void (*gethostbyaddr_async_callback)(struct hostent *hent, void *context);

/*!
 @function gethostbyaddr_async_start
 @description Asynchronously resolves an Internet address
 @param addr The address to be resolved.
 @param len The length, in bytes, of the address.
 @param type
 @param callout The function to be called when the specified
	address has been resolved.
 @param context A user specified context which will be passed
	to the callout function.
 @result A mach reply port which will be sent a message when
	the addr resolution has completed. This message
	should be passed to the gethostbyaddr_async_handleReply
	function. A NULL value indicates that no address
	was specified or some other error occurred which
	prevented the resolution from being started.
 */
mach_port_t
gethostbyaddr_async_start(const char *addr, int len, int type, gethostbyaddr_async_callback callout, void *context);

/*!
 @function gethostbyaddr_async_cancel
 @description Cancel an asynchronous request currently in progress.
 @param port The mach reply port associated with the request to be cancelled.
 */
void gethostbyaddr_async_cancel(mach_port_t port);

/*!
 @function gethostbyaddr_async_handleReply
 @description This function should be called with the Mach message sent
	to the port returned by the call to gethostbyaddr_async_start.
	The reply message will be interpreted and will result in a
	call to the specified callout function.
 @param replyMsg The Mach message.
 */
void gethostbyaddr_async_handleReply(void *replyMsg);

/*
 @typedef gethostbyname_async_callback
 @discussion Type of the callback function used when a
 	gethostbyname_async_start() request is delivered.
 @param hent The resolved host entry.
 @param context The context provided when the request was initiated.
 */
typedef void (*gethostbyname_async_callback)(struct hostent *hent, void *context);

/*!
 @function gethostbyname_async_start
 @description Asynchronously resolves a hostname
 @param name The hostname to be resolved.
 @param callout The function to be called when the specified
	hostname has been resolved.
 @param context A user specified context which will be passed
	to the callout function.
 @result A mach reply port which will be sent a message when
	the name resolution has completed. This message
	should be passed to the gethostbyname_async_handleReply
	function. A NULL value indicates that no hostname
	was specified or some other error occurred which
	prevented the resolution from being started.
 */
mach_port_t gethostbyname_async_start(const char *name, gethostbyname_async_callback callout, void *context);

/*!
 @function gethostbyname_async_cancel
 @description Cancel an asynchronous request currently in progress.
 @param port The mach reply port associated with the request to be cancelled.
 */
void gethostbyname_async_cancel(mach_port_t port);

/*!
 @function gethostbyname_async_handleReply
 @description This function should be called with the Mach message sent
	to the port returned by the call to gethostbyname_async_start.
	The reply message will be interpreted and will result in a
	call to the specified callout function.
 @param replyMsg The Mach message.
 */
void gethostbyname_async_handleReply(void *replyMsg);

/*
 @typedef getipnodebyaddr_async_callback
 @discussion Type of the callback function used when a
	getipnodebyaddr_async_start() request is delivered.
 @param hent The resolved host entry. If not NULL, the caller
	must call freehostent() on the host entry.
 @param error If error code if the resolved host entry is NULL
 @param context The context provided when the request was initiated.
 */
typedef void (*getipnodebyaddr_async_callback)(struct hostent *hent, int error, void *context);

/*!
 @function getipnodebyaddr_async_start
 @description Asynchronously resolves an Internet address
 @param addr The address to be resolved.
 @param len The length, in bytes, of the address.
 @param af The address family
 @param error
 @param callout The function to be called when the specified
	address has been resolved.
 @param context A user specified context which will be passed
	to the callout function.
 @result A mach reply port which will be sent a message when
	the addr resolution has completed. This message
	should be passed to the getipnodebyaddr_async_handleReply
	function. A NULL value indicates that no address
	was specified or some other error occurred which
	prevented the resolution from being started.
 */
mach_port_t getipnodebyaddr_async_start(const void *addr, size_t len, int af, int *error, getipnodebyaddr_async_callback callout, void *context);

/*!
 @function getipnodebyaddr_async_cancel
 @description Cancel an asynchronous request currently in progress.
 @param port The mach reply port associated with the request to be cancelled.
 */
void getipnodebyaddr_async_cancel(mach_port_t port);

/*!
 @function getipnodebyaddr_async_handleReply
 @description This function should be called with the Mach message sent
	to the port returned by the call to getipnodebyaddr_async_start.
	The reply message will be interpreted and will result in a
	call to the specified callout function.
 @param replyMsg The Mach message.
 */
void getipnodebyaddr_async_handleReply(void *replyMsg);


/*
 @typedef getipnodebyname_async_callback
 @discussion Type of the callback function used when a
	getipnodebyname_async_start() request is delivered.
 @param hent The resolved host entry. If not NULL, the caller
	must call freehostent() on the host entry.
 @param error If error code if the resolved host entry is NULL
 @param context The context provided when the request was initiated.
 */
typedef void (*getipnodebyname_async_callback)(struct hostent *hent, int error, void *context);

/*!
 @function getipnodebyname_async_start
 @description Asynchronously resolves a hostname
 @param name The hostname to be resolved.
 @param af
 @param flags
 @param error
 @param callout The function to be called when the specified
	hostname has been resolved.
 @param context A user specified context which will be passed
	to the callout function.
 @result A mach reply port which will be sent a message when
	the name resolution has completed. This message
	should be passed to the getipnodebyname_async_handleReply
	function. A NULL value indicates that no hostname
	was specified or some other error occurred which
	prevented the resolution from being started.
 */
mach_port_t getipnodebyname_async_start(const char *name, int af, int flags, int *error, getipnodebyname_async_callback callout, void *context);

/*!
 @function getipnodebyname_async_cancel
 @description Cancel an asynchronous request currently in progress.
 @param port The mach reply port associated with the request to be cancelled.
 */
void getipnodebyname_async_cancel(mach_port_t port);

/*!
 @function getipnodebyname_async_handleReply
 @description This function should be called with the Mach message sent
	to the port returned by the call to getipnodebyname_async_start.
	The reply message will be interpreted and will result in a
	call to the specified callout function.
 @param replyMsg The Mach message.
 */
void getipnodebyname_async_handleReply(void *replyMsg);

__END_DECLS

#endif /* !_NETDB_ASYNC_H_ */

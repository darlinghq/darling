/*
 * Copyright (c) 2008-2010 Apple Inc.  All rights reserved.
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

#include <si_module.h>

/*
 * SPI to disable / enable modules during Libinfo search.
 * Currently known module names are "cache", "ds", "file", and "mdns".
 * Use flag = 0 to disable a module, flag = 1 to enable.
 */
void si_search_module_set_flags(const char *name, uint32_t flag);

/*
 * Most of these async callbacks get data that's held in thread-specific
 * memory (specific to the callback thread) that will be re-used by that
 * thread automatically on the next call - synchronous or asynchronous -
 * of the same routine.  For example, an async getpwnam call will use a 
 * slot in the caller's thread-specifc data to save the returned struct passwd.
 * A subsequent call to getpwnam will release the previous entry
 *
 * Callers of getaddrinfo must free the returned addrinfo list using freeaddrinfo.
 * Callbacks for async getaddrinfo lookups must also free the returned addrinfo list.
 *
 * Callers of getnameinfo supply their own memory for the node and serv names.
 * Callbacks for async getnameinfo lookups get node and serv names that reside in
 * thread-specific memory that is reclaimed by the libraray.  The results must not be freed.
 */
typedef void (*si_user_async_callback)(struct passwd *, void *context);
typedef void (*si_group_async_callback)(struct group *, void *context);
typedef void (*si_grouplist_async_callback)(struct grouplist_s *, void *context);
typedef void (*si_alias_async_callback)(struct aliasent *, void *context);
typedef void (*si_host_async_callback)(struct hostent *, void *context);
typedef void (*si_ipnode_async_callback)(struct hostent *, int32_t status, void *context);
typedef void (*si_network_async_callback)(struct netent *, void *context);
typedef void (*si_service_async_callback)(struct servent *, void *context);
typedef void (*si_protocol_async_callback)(struct protoent *, void *context);
typedef void (*si_rpc_async_callback)(struct rpcent *, void *context);
typedef void (*si_fs_async_callback)(struct fstab *, void *context);
typedef void (*si_addrinfo_async_callback)(int32_t status, struct addrinfo *res, void *context);
typedef void (*si_nameinfo_async_callback)(int32_t status, char *node, char *serv, void *context);
typedef void (*si_setent_async_callback)(void *context);

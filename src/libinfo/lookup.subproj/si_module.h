/*
 * Copyright (c) 2008-2011 Apple Inc.  All rights reserved.
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

#ifndef __SI_MODULE_H__
#define __SI_MODULE_H__

#include <stdlib.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pwd.h>
#include <grp.h>
#include <uuid/uuid.h>
#include <netdb.h>
#include <aliasdb.h>
#include <fstab.h>
#include <mach/mach.h>
#include <si_data.h>
#include <ils.h>

#define forever for(;;)
#define string_equal(A,B) (strcmp(A,B)==0)
#define string_not_equal(A,B) (strcmp(A,B)!=0)

#define SI_CALL_USER_BYNAME        0
#define SI_CALL_USER_BYUID         1
#define SI_CALL_USER_ALL           2
#define SI_CALL_GROUP_BYNAME       3
#define SI_CALL_GROUP_BYGID        4
#define SI_CALL_GROUP_ALL          5
#define SI_CALL_NETGROUP_BYNAME    6
#define SI_CALL_IN_NETGROUP        7
#define SI_CALL_GROUPLIST          8
#define SI_CALL_ALIAS_BYNAME       9
#define SI_CALL_ALIAS_ALL         10
#define SI_CALL_HOST_BYNAME       11
#define SI_CALL_HOST_BYADDR       12
#define SI_CALL_HOST_ALL          13
#define SI_CALL_NETWORK_BYNAME    14
#define SI_CALL_NETWORK_BYADDR    15
#define SI_CALL_NETWORK_ALL       16
#define SI_CALL_SERVICE_BYNAME    17
#define SI_CALL_SERVICE_BYPORT    18
#define SI_CALL_SERVICE_ALL       19
#define SI_CALL_PROTOCOL_BYNAME   20
#define SI_CALL_PROTOCOL_BYNUMBER 21
#define SI_CALL_PROTOCOL_ALL      22
#define SI_CALL_RPC_BYNAME        23
#define SI_CALL_RPC_BYNUMBER      24
#define SI_CALL_RPC_ALL           25
#define SI_CALL_FS_BYSPEC         26
#define SI_CALL_FS_BYFILE         27
#define SI_CALL_FS_ALL            28
#define SI_CALL_ADDRINFO          29
#define SI_CALL_NAMEINFO          30
#define SI_CALL_IPNODE_BYNAME     31
#define SI_CALL_MAC_BYNAME        32
#define SI_CALL_MAC_BYMAC         33
#define SI_CALL_MAC_ALL           34
#define SI_CALL_DNS_QUERY         35
#define SI_CALL_DNS_SEARCH        36

#define si_call_returns_list(A) \
((A==SI_CALL_USER_ALL)||(A==SI_CALL_GROUP_ALL)||(A==SI_CALL_HOST_ALL)||(A==SI_CALL_NETWORK_ALL)||\
 (A==SI_CALL_SERVICE_ALL)||(A==SI_CALL_PROTOCOL_ALL)||(A==SI_CALL_RPC_ALL)||(A==SI_CALL_FS_ALL)||\
 (A==SI_CALL_ALIAS_ALL)||(A==SI_CALL_NETGROUP_BYNAME)||(A==SI_CALL_ADDRINFO)||(A==SI_CALL_MAC_ALL))

#define si_call_str1_is_buffer(A) \
((A==SI_CALL_HOST_BYADDR)||(A==SI_CALL_NAMEINFO))

#define CATEGORY_INVALID  (-1)
#define CATEGORY_DEFAULT    0
#define CATEGORY_USER       1
#define CATEGORY_GROUP      2
#define CATEGORY_GROUPLIST  3
#define CATEGORY_NETGROUP   4
#define CATEGORY_ALIAS      5
#define CATEGORY_HOST_IPV4  6
#define CATEGORY_HOST_IPV6  7
#define CATEGORY_NETWORK    8
#define CATEGORY_SERVICE    9
#define CATEGORY_PROTOCOL  10
#define CATEGORY_RPC       11
#define CATEGORY_FS        12
#define CATEGORY_MAC       13
#define CATEGORY_NAMEINFO  14
#define CATEGORY_ADDRINFO  15
#define CATEGORY_DNSPACKET 16
#define CATEGORY_SRV       17
#define CATEGORY_COUNT     18

/* convenience */
#define CATEGORY_HOST CATEGORY_HOST_IPV4

#define SEL_ALL 0
#define SEL_NAME 1
#define SEL_NUMBER 2

/* host, getaddrinfo, and getnameinfo status codes */
#define SI_STATUS_NO_ERROR                 0
#define SI_STATUS_H_ERRNO_HOST_NOT_FOUND   1
#define SI_STATUS_H_ERRNO_TRY_AGAIN        2
#define SI_STATUS_H_ERRNO_NO_RECOVERY      3
#define SI_STATUS_H_ERRNO_NO_DATA          4
#define SI_STATUS_INTERNAL                10
#define SI_STATUS_WORKITEM_NOT_FOUND      11
#define SI_STATUS_RETURNS_ITEM            12
#define SI_STATUS_RETURNS_LIST            13
#define SI_STATUS_CALL_IN_PROGRESS        14
#define SI_STATUS_CALL_CANCELLED          15
#define	SI_STATUS_EAI_PLUS_100           100
#define	SI_STATUS_EAI_ADDRFAMILY         101
#define	SI_STATUS_EAI_AGAIN              102
#define	SI_STATUS_EAI_BADFLAGS           103
#define	SI_STATUS_EAI_FAIL               104
#define	SI_STATUS_EAI_FAMILY             105
#define	SI_STATUS_EAI_MEMORY             106
#define	SI_STATUS_EAI_NODATA             107
#define	SI_STATUS_EAI_NONAME             108
#define	SI_STATUS_EAI_SERVICE            109
#define	SI_STATUS_EAI_SOCKTYPE           110
#define	SI_STATUS_EAI_SYSTEM             111
#define SI_STATUS_EAI_BADHINTS           112
#define SI_STATUS_EAI_PROTOCOL           113
#define SI_STATUS_EAI_OVERFLOW           114
#define SI_STATUS_ERRNO_PLUS_200         200

typedef void (*item_async_callback)(si_item_t *, uint32_t, void *);
typedef void (*list_async_callback)(si_list_t *, uint32_t, void *);

typedef struct grouplist_s
{
	char *gl_user;
	int gl_count;
	int gl_gid_siz;
	gid_t *gl_gid;
} si_grouplist_t;

typedef struct addrinfo_s
{
	int ai_flags;
	int ai_family;
	int ai_socktype;
	int ai_protocol;
	uint32_t ai_addrlen;
	socket_data_t ai_addr;
	char *ai_canonname;
} si_addrinfo_t;

typedef struct nameinfo_s
{
	char *ni_node;
	char *ni_serv;
} si_nameinfo_t;

typedef struct mac_s
{
	char *host;
	char *mac;
} si_mac_t;

typedef struct netgrent_s
{
	char *ng_host;
	char *ng_user;
	char *ng_domain;
} si_netgrent_t;

typedef struct dnspacket_s
{
	int dns_packet_len;
	char *dns_packet;
	int dns_server_len;
	struct sockaddr *dns_server;
} si_dnspacket_t;

typedef struct si_srv_s
{
	uint16_t priority;
	uint16_t weight;
	uint16_t port;
	char *target;
} si_srv_t;

struct si_mod_s;

struct si_mod_vtable_s
{
	void (*sim_close)(struct si_mod_s *si);

	int (*sim_is_valid)(struct si_mod_s *si, si_item_t *item);

	si_item_t *(*sim_user_byname)(struct si_mod_s *si, const char *name);
	si_item_t *(*sim_user_byuid)(struct si_mod_s *si, uid_t uid);
	si_item_t *(*sim_user_byuuid)(struct si_mod_s *si, uuid_t uuid);
	si_list_t *(*sim_user_all)(struct si_mod_s *si);

	si_item_t *(*sim_group_byname)(struct si_mod_s *si, const char *name);
	si_item_t *(*sim_group_bygid)(struct si_mod_s *si, gid_t gid);
	si_item_t *(*sim_group_byuuid)(struct si_mod_s *si, uuid_t uuid);
	si_list_t *(*sim_group_all)(struct si_mod_s *si);

	si_item_t *(*sim_grouplist)(struct si_mod_s *si, const char *name, uint32_t count);

	si_list_t *(*sim_netgroup_byname)(struct si_mod_s *si, const char *name);
	int (*sim_in_netgroup)(struct si_mod_s *si, const char *name, const char *host, const char *user, const char *domain);

	si_item_t *(*sim_alias_byname)(struct si_mod_s *si, const char *name);
	si_list_t *(*sim_alias_all)(struct si_mod_s *si);

	si_item_t *(*sim_host_byname)(struct si_mod_s *si, const char *name, int af, const char *interface, uint32_t *err);
	si_item_t *(*sim_host_byaddr)(struct si_mod_s *si, const void *addr, int af, const char *interface, uint32_t *err);
	si_list_t *(*sim_host_all)(struct si_mod_s *si);

	si_item_t *(*sim_network_byname)(struct si_mod_s *si, const char *name);
	si_item_t *(*sim_network_byaddr)(struct si_mod_s *si, uint32_t addr);
	si_list_t *(*sim_network_all)(struct si_mod_s *si);

	si_item_t *(*sim_service_byname)(struct si_mod_s *si, const char *name, const char *proto);
	si_item_t *(*sim_service_byport)(struct si_mod_s *si, int port, const char *proto);
	si_list_t *(*sim_service_all)(struct si_mod_s *si);

	si_item_t *(*sim_protocol_byname)(struct si_mod_s *si, const char *name);
	si_item_t *(*sim_protocol_bynumber)(struct si_mod_s *si, int number);
	si_list_t *(*sim_protocol_all)(struct si_mod_s *si);

	si_item_t *(*sim_rpc_byname)(struct si_mod_s *si, const char *name);
	si_item_t *(*sim_rpc_bynumber)(struct si_mod_s *si, int number);
	si_list_t *(*sim_rpc_all)(struct si_mod_s *si);

	si_item_t *(*sim_fs_byspec)(struct si_mod_s *si, const char *spec);
	si_item_t *(*sim_fs_byfile)(struct si_mod_s *si, const char *file);
	si_list_t *(*sim_fs_all)(struct si_mod_s *si);

	si_item_t *(*sim_mac_byname)(struct si_mod_s *si, const char *name);
	si_item_t *(*sim_mac_bymac)(struct si_mod_s *si, const char *mac);
	si_list_t *(*sim_mac_all)(struct si_mod_s *si);

	si_list_t *(*sim_addrinfo)(struct si_mod_s *si, const void *node, const void *serv, uint32_t family, uint32_t socktype, uint32_t protocol, uint32_t flags, const char *interface, uint32_t *err);
	int (*sim_wants_addrinfo)(struct si_mod_s *si);

	si_item_t *(*sim_nameinfo)(struct si_mod_s *si, const struct sockaddr *sa, int flags, const char *interface, uint32_t *err);

	si_list_t *(*sim_srv_byname)(struct si_mod_s *si, const char *qname, const char *interface, uint32_t *err);

	si_item_t *(*sim_item_call)(struct si_mod_s *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t *err);
	si_list_t *(*sim_list_call)(struct si_mod_s *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t num3, uint32_t num4, uint32_t *err);

	mach_port_t (*sim_async_call)(struct si_mod_s *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t num3, uint32_t num4, void *callback, void *context);
	void (*sim_async_cancel)(mach_port_t p);
	void (*sim_async_handle_reply)(mach_msg_header_t *msg);
};

#define SI_MOD_FLAG_STATIC 0x00000001

typedef struct si_mod_s
{
	char *name;
	uint32_t vers;
	int32_t refcount;
	uint32_t flags;

	void *bundle;
	void *private;
	
	const struct si_mod_vtable_s *vtable;
} si_mod_t;

si_mod_t *si_module_with_name(const char *name);
si_mod_t *si_module_retain(si_mod_t *si);
void si_module_release(si_mod_t *si);
const char *si_module_name(si_mod_t *si);
int si_module_vers(si_mod_t *si);

si_mod_t *si_search(void);

int si_item_match(si_item_t *item, int cat, const void *name, uint32_t num, int which);
int si_item_is_valid(si_item_t *item);

si_item_t *si_user_byname(si_mod_t *si, const char *name);
si_item_t *si_user_byuid(si_mod_t *si, uid_t uid);
si_item_t *si_user_byuuid(si_mod_t *si, uuid_t uuid);
si_list_t *si_user_all(si_mod_t *si);

si_item_t *si_group_byname(si_mod_t *si, const char *name);
si_item_t *si_group_bygid(si_mod_t *si, gid_t gid);
si_item_t *si_group_byuuid(si_mod_t *si, uuid_t uuid);
si_list_t *si_group_all(si_mod_t *si);

si_item_t *si_grouplist(si_mod_t *si, const char *name, uint32_t count);

int si_in_netgroup(struct si_mod_s *si, const char *name, const char *host, const char *user, const char *domain);
si_list_t *si_netgroup_byname(struct si_mod_s *si, const char *name);

si_item_t *si_alias_byname(struct si_mod_s *si, const char *name);
si_list_t *si_alias_all(struct si_mod_s *si);

si_item_t *si_host_byname(si_mod_t *si, const char *name, int af, const char *interface, uint32_t *err);
si_item_t *si_host_byaddr(si_mod_t *si, const void *addr, int af, const char *interface, uint32_t *err);
si_list_t *si_host_all(si_mod_t *si);

si_item_t *si_mac_byname(struct si_mod_s *si, const char *name);
si_item_t *si_mac_bymac(struct si_mod_s *si, const char *mac);
si_list_t *si_mac_all(struct si_mod_s *si);

si_item_t *si_network_byname(si_mod_t *si, const char *name);
si_item_t *si_network_byaddr(si_mod_t *si, uint32_t addr);
si_list_t *si_network_all(si_mod_t *si);

si_item_t *si_service_byname(si_mod_t *si, const char *name, const char *proto);
si_item_t *si_service_byport(si_mod_t *si, int port, const char *proto);
si_list_t *si_service_all(si_mod_t *si);

si_item_t *si_protocol_byname(si_mod_t *si, const char *name);
si_item_t *si_protocol_bynumber(si_mod_t *si, uint32_t number);
si_list_t *si_protocol_all(si_mod_t *si);

si_item_t *si_rpc_byname(si_mod_t *si, const char *name);
si_item_t *si_rpc_bynumber(si_mod_t *si, int number);
si_list_t *si_rpc_all(si_mod_t *si);

si_item_t *si_fs_byspec(si_mod_t *si, const char *spec);
si_item_t *si_fs_byfile(si_mod_t *si, const char *file);
si_list_t *si_fs_all(si_mod_t *si);

int si_wants_addrinfo(si_mod_t *s);
si_list_t *si_addrinfo(si_mod_t *si, const char *node, const char *serv, uint32_t family, uint32_t socktype, uint32_t protocol, uint32_t flags, const char *interface, uint32_t *err);

si_item_t *si_nameinfo(si_mod_t *si, const struct sockaddr *sa, int flags, const char *interface, uint32_t *err);
si_item_t *si_ipnode_byname(si_mod_t *si, const char *name, int family, int flags, const char *interface, uint32_t *err);

si_list_t *si_srv_byname(si_mod_t *si, const char *qname, const char *interface, uint32_t *err);

si_item_t *si_item_call(si_mod_t *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t *err);
si_list_t *si_list_call(si_mod_t *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t num3, uint32_t num4, uint32_t *err);

extern mach_port_t si_async_call(si_mod_t *si, int call, const char *str1, const char *str2, const char *str3, uint32_t num1, uint32_t num2, uint32_t num3, uint32_t num4, void *callback, void *context);
extern void si_async_cancel(mach_port_t p);
extern void si_async_handle_reply(mach_msg_header_t *msg);

char *si_standardize_mac_address(const char *addr);
si_item_t *si_addrinfo_v4(si_mod_t *si, int32_t flags, int32_t sock, int32_t proto, uint16_t port, struct in_addr *addr, uint16_t iface, const char *cname);
si_item_t *si_addrinfo_v6(si_mod_t *si, int32_t flags, int32_t sock, int32_t proto, uint16_t port, struct in6_addr *addr, uint16_t iface, const char *cname);
si_item_t *si_addrinfo_v4_mapped(si_mod_t *si, int32_t flags, int32_t sock, int32_t proto, uint16_t port, struct in_addr *addr, uint16_t iface, const char *cname);
si_list_t *si_addrinfo_list(si_mod_t *si, uint32_t flags, int socktype, int proto, struct in_addr *a4, struct in6_addr *a6, int port, int scopeid, const char *cname4, const char *cname6);
si_list_t *si_addrinfo_list_from_hostent(si_mod_t *si, uint32_t flags, uint32_t socktype, uint32_t proto, uint16_t port, uint16_t scope, const struct hostent *h4, const struct hostent *h6);

int _gai_serv_to_port(const char *serv, uint32_t proto, uint16_t *port);
si_list_t *_gai_simple(si_mod_t *si, const void *nodeptr, const void *servptr, uint32_t family, uint32_t socktype, uint32_t proto, uint32_t flags, const char *interface, uint32_t *err);
int si_inet_config(uint32_t *inet4, uint32_t *inet6);

#endif /* ! __SI_MODULE_H__ */

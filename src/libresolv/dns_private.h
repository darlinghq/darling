#ifndef __DNS_PRIVATE_H__
#define __DNS_PRIVATE_H__

#include <sys/cdefs.h>

#define MDNS_HANDLE_NAME "*MDNS*"

#define DNS_FLAG_DEBUG                    0x00000001
#define DNS_FLAG_CHECK_RESOLVER_DIR       0x00000002
#define DNS_FLAG_HAVE_IPV6_SERVER         0x00000004
#define DNS_FLAG_OK_TO_SKIP_AAAA          0x00000008
#define DNS_FLAG_DEFAULT_RESOLVER         0x00000010
#define DNS_FLAG_FORWARD_TO_MDNSRESPONDER 0x00000020

typedef struct
{
	res_state res;
	char *source;
	char *name;
	uint32_t search_count;
	char **search_list;
	uint16_t port;
	uint32_t flags;
	uint32_t total_timeout;
	uint32_t send_timeout;
	uint32_t search_order;
	uint32_t reserved1;
	void *reserved_pointer1;
} pdns_handle_t;

typedef struct 
{
	pdns_handle_t *pdns_primary;
	uint32_t client_count;
	pdns_handle_t **client;
	uint32_t modtime;
	uint32_t stattime;
	uint32_t stat_latency;
	uint32_t flags;
	int notify_sys_config_token;
	int notify_dir_token;
	int notify_delay_token;
	time_t dns_delay;
	uint32_t reserved1;
	void *reserved_pointer1;
} sdns_handle_t;

typedef struct __dns_handle_private_struct
{
	uint32_t handle_type;
	sdns_handle_t *sdns;
	pdns_handle_t *pdns;
	char *recvbuf;
	uint32_t recvsize;
	uint32_t reserved1;
	uint32_t reserved2;
	void *reserved_pointer1;
	void *reserved_pointer2;
} dns_private_handle_t;


__BEGIN_DECLS

/*
 * Returns the number of nameserver addresses available to the input
 * DNS client.  Returns zero if the input handle is a "Super" DNS handle.
 */
extern uint32_t dns_server_list_count(dns_handle_t d);

/*
 * Returns the nameserver address at the given index.  Returns NULL
 * if the index is out of range.  Caller should free the returned sockaddr.
 */
extern struct sockaddr *dns_server_list_address(dns_handle_t d, uint32_t i);

/*
 * Returns a list of all server addresses for all clients.
 * Caller must free each list entry, and the returned list.
 */
extern void dns_all_server_addrs(dns_handle_t d, struct sockaddr ***addrs, uint32_t *count);

/*
 * Returns the number of names in the search list.
 */
uint32_t dns_search_list_count(dns_handle_t d);

__END_DECLS

#endif /* __DNS_PRIVATE_H__ */

/*
 * Copyright (c) 1999-2007 Apple Inc. All rights reserved.
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

#include "dns.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <netdb.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <errno.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <fcntl.h>
#include <notify.h>
#include <dnsinfo.h>
#include "dns_private.h"
#include "res_private.h"

#define INET_NTOP_AF_INET_OFFSET 4
#define INET_NTOP_AF_INET6_OFFSET 8

#define SEARCH_COUNT_INIT -1

#define DNS_RESOLVER_DIR "/etc/resolver"

#define NOTIFY_DNS_CONTROL_NAME "com.apple.system.dns"
#define DNS_CONTROL_FLAG_DEBUG    0x0000000000000001LL
#define DNS_CONTROL_FLAG_NO_MDNS  0x0000000000000002LL

#define NOTIFY_DIR_NAME "com.apple.system.dns.resolver.dir"
#define DNS_DELAY_NAME "com.apple.system.dns.delay"

#define DNS_DELAY_INTERVAL 4

#define DNS_PRIVATE_HANDLE_TYPE_SUPER 0
#define DNS_PRIVATE_HANDLE_TYPE_PLAIN 1

#define MDNS_MIN_TTL 2

static int dns_control_token = -1;
static int dns_control_mdns = 1;
static int dns_control_debug = 0;
static pthread_mutex_t dns_control_lock = PTHREAD_MUTEX_INITIALIZER;

extern uint32_t notify_monitor_file(int token, const char *name, int flags);

extern void res_client_close(res_state res);
extern res_state res_state_new();
extern int res_nquery_soa_min(res_state statp, const char *name, int class, int type, u_char *answer, int anslen, struct sockaddr *from, int *fromlen, int *min);
extern int res_nsearch_2(res_state statp, const char *name, int class, int type, u_char *answer, int anslen, struct sockaddr *from, uint32_t *fromlen);
extern int __res_nsearch_list_2(res_state statp, const char *name,	int class, int type,  u_char *answer, int anslen, struct sockaddr *from, uint32_t *fromlen, int nsearch, char **search);

extern char *res_next_word(char **p);
extern res_state res_build_start(res_state res);
extern int res_build(res_state res, uint16_t port, uint32_t *nsrch, char *key, char *val);
extern int res_build_sortlist(res_state res, struct in_addr addr, struct in_addr mask);

static void
_pdns_set_name(pdns_handle_t *pdns, const char *name)
{
	int n;

	if (pdns == NULL) return;
	if (name == NULL) return;

	/* only set the name once */
	if (pdns->name != NULL) return;

	/* strip trailing dots */
	n = strlen(name) - 1;
	while ((n >= 0) && (name[n] == '.')) n--;

	if (n < 0) return;

	n++;
	pdns->name = calloc(n + 1, sizeof(char));
	if (pdns->name == NULL) return;
	memcpy(pdns->name, name, n);
}

static pdns_handle_t *
_pdns_build_start(char *name)
{
	pdns_handle_t *pdns;

	pdns = (pdns_handle_t *)calloc(1, sizeof(pdns_handle_t));
	if (pdns == NULL) return NULL;

	pdns->res = res_build_start(NULL);
	if (pdns->res == NULL)
	{
		free(pdns);
		return NULL;
	}

	_pdns_set_name(pdns, name);
	pdns->port = NS_DEFAULTPORT;

	return pdns;
}

static int
_pdns_build_finish(pdns_handle_t *pdns)
{
	uint32_t n;

	if (pdns == NULL) return -1;

	n = pdns->res->nscount;
	if (n == 0) n = 1;

	if (pdns->total_timeout == 0)
	{
		if (pdns->send_timeout == 0) pdns->total_timeout = RES_MAXRETRANS;
		else pdns->total_timeout = pdns->send_timeout * pdns->res->retry * n;
	}

	if (pdns->total_timeout == 0) pdns->res->retrans = RES_MAXRETRANS;
	else pdns->res->retrans = pdns->total_timeout;

	pdns->res->options |= RES_INIT;

	return 0;
}

static int
_pdns_build_sortlist(pdns_handle_t *pdns, struct in_addr addr, struct in_addr mask)
{
	if (pdns == NULL) return -1;
	return res_build_sortlist(pdns->res, addr, mask);
}

static void
_pdns_free(pdns_handle_t *pdns)
{
	int i;

	if (pdns == NULL) return;

	if ((pdns->search_count != SEARCH_COUNT_INIT) && (pdns->search_count > 0) && (pdns->search_list != NULL))
	{
		for (i = 0; i < pdns->search_count; i++) free(pdns->search_list[i]);
		free(pdns->search_list);
	}

	if (pdns->name != NULL) free(pdns->name);
	if (pdns->res != NULL) res_client_close(pdns->res);

	free(pdns);
}

static int
_pdns_build(pdns_handle_t *pdns, char *key, char *val)
{
	struct in6_addr addr6;
	int32_t status;
	char *dupval;

	if (pdns == NULL) return -1;

	/*
	 * Detect IPv6 server addresses.
	 */
	if (((pdns->flags & DNS_FLAG_HAVE_IPV6_SERVER) == 0) && (!strcmp(key, "nameserver")))
	{
		memset(&addr6, 0, sizeof(struct in6_addr));
		status = inet_pton(AF_INET6, val, &addr6);
		if (status == 1) pdns->flags |= DNS_FLAG_HAVE_IPV6_SERVER;
	}

	/*
	 * We handle some keys here.
	 * Other keys get passed on to res_build.
	 */
	if (!strcmp(key, "default"))
	{
		pdns->flags |= DNS_FLAG_DEFAULT_RESOLVER;
		return 0;
	}

	if (!strcmp(key, "port"))
	{
		pdns->port = atoi(val);
		return 0;
	}

	if (!strcmp(key, "search"))
	{
		dupval = strdup(val);
		if (dupval == NULL) return -1;

		if (pdns->search_count == SEARCH_COUNT_INIT) pdns->search_count = 0;
		if (pdns->search_count == 0)
		{
			pdns->search_list = (char **)calloc(1, sizeof(char *));
		}
		else
		{
			pdns->search_list = (char **)reallocf(pdns->search_list, (pdns->search_count + 1) * sizeof(char *));
		}

		if (pdns->search_list == NULL)
		{
			free(dupval);
			_pdns_free(pdns);
			return -1;
		}

		pdns->search_list[pdns->search_count] = dupval;
		pdns->search_count++;
		return 0;
	}

	if (!strcmp(key, "total_timeout"))
	{
		pdns->total_timeout = atoi(val);
		return 0;
	}

	if (!strcmp(key, "timeout"))
	{
		pdns->send_timeout = atoi(val);
		return 0;
	}

	if (!strcmp(key, "search_order"))
	{
		pdns->search_order = atoi(val);
		return 0;
	}

	if (!strcmp(key, "pdns"))
	{
		pdns->flags |= DNS_FLAG_FORWARD_TO_MDNSRESPONDER;
		return 0;
	}

	if (!strcmp(key, "mdns"))
	{
		pdns->flags |= DNS_FLAG_FORWARD_TO_MDNSRESPONDER;
		return 0;
	}

	/* pass on to res_build */
	return res_build(pdns->res, pdns->port, &(pdns->search_count), key, val);
}

static pdns_handle_t *
_pdns_convert_sc(dns_resolver_t *r)
{
	pdns_handle_t *pdns;
	char *val, *p, *x;
	int i;

	pdns = _pdns_build_start(r->domain);
	if (r->domain == NULL) _pdns_build(pdns, "default", NULL);

	p = getenv("RES_RETRY_TIMEOUT");
	if (p != NULL) pdns->send_timeout = atoi(p);

	p = getenv("RES_RETRY");
	if (p != NULL) pdns->res->retry= atoi(p);

	if (r->port != 0)
	{
		val = NULL;
		asprintf(&val, "%hu", r->port);
		if (val == NULL)
		{
			_pdns_free(pdns);
			return NULL;
		}

		_pdns_build(pdns, "port", val);
		free(val);
	}

	if (r->n_nameserver > MAXNS) r->n_nameserver = MAXNS;
	for (i = 0; i < r->n_nameserver; i++)
	{
		if (r->nameserver[i]->sa_family == AF_INET)
		{
			val = calloc(1, 256);
			if (val == NULL)
			{
				_pdns_free(pdns);
				return NULL;
			}

			inet_ntop(AF_INET, (char *)(r->nameserver[i]) + INET_NTOP_AF_INET_OFFSET, val, 256);
			_pdns_build(pdns, "nameserver", val);
			free(val);
		}
		else if (r->nameserver[i]->sa_family == AF_INET6)
		{
			pdns->flags |= DNS_FLAG_HAVE_IPV6_SERVER;
			val = calloc(1, 256);
			if (val == NULL)
			{
				_pdns_free(pdns);
				return NULL;
			}

			inet_ntop(AF_INET6, (char *)(r->nameserver[i]) + INET_NTOP_AF_INET6_OFFSET, val, 256);
			_pdns_build(pdns, "nameserver", val);
			free(val);
		}
	}

	if (r->n_search > MAXDNSRCH) r->n_search = MAXDNSRCH;
	for (i = 0; i < r->n_search; i++)
	{
		val = NULL;
		asprintf(&val, "%s", r->search[i]);
		if (val == NULL)
		{
			_pdns_free(pdns);
			return NULL;
		}

		_pdns_build(pdns, "search", val);
		free(val);
	}

	if (r->timeout > 0)
	{
		val = NULL;
		asprintf(&val, "%d", r->timeout);
		if (val == NULL)
		{
			_pdns_free(pdns);
			return NULL;
		}

		_pdns_build(pdns, "total_timeout", val);
		free(val);
	}

	val = NULL;
	asprintf(&val, "%d", r->search_order);
	if (val == NULL)
	{
		_pdns_free(pdns);
		return NULL;
	}

	_pdns_build(pdns, "search_order", val);
	free(val);

	if (r->n_sortaddr > MAXRESOLVSORT) r->n_sortaddr = MAXRESOLVSORT;
	for (i = 0; i < r->n_sortaddr; i++)
	{
		_pdns_build_sortlist(pdns, r->sortaddr[i]->address, r->sortaddr[i]->mask);
	}

	p = r->options;
	while (NULL != (x = res_next_word(&p)))
	{
		/* search for and process individual options */
		if (!strncmp(x, "ndots:", 6))
		{
			_pdns_build(pdns, "ndots", x+6);
		}

		else if (!strncmp(x, "nibble:", 7))
		{
			_pdns_build(pdns, "nibble", x+7);
		}

		else if (!strncmp(x, "nibble2:", 8))
		{
			_pdns_build(pdns, "nibble2", x+8);
		}

		else if (!strncmp(x, "timeout:", 8))
		{
			_pdns_build(pdns, "timeout", x+8);
		}

		else if (!strncmp(x, "attempts:", 9))
		{
			_pdns_build(pdns, "attempts", x+9);
		}

		else if (!strncmp(x, "bitstring:", 10))
		{
			_pdns_build(pdns, "bitstring", x+10);
		}

		else if (!strncmp(x, "v6revmode:", 10))
		{
			_pdns_build(pdns, "v6revmode", x+10);
		}

		else if (!strcmp(x, "debug"))
		{
			_pdns_build(pdns, "debug", NULL);
		}

		else if (!strcmp(x, "no_tld_query"))
		{
			_pdns_build(pdns, "no_tld_query", NULL);
		}

		else if (!strcmp(x, "inet6"))
		{
			_pdns_build(pdns, "inet6", NULL);
		}

		else if (!strcmp(x, "rotate"))
		{
			_pdns_build(pdns, "rotate", NULL);
		}

		else if (!strcmp(x, "no-check-names"))
		{
			_pdns_build(pdns, "no-check-names", NULL);
		}

#ifdef RES_USE_EDNS0
		else if (!strcmp(x, "edns0"))
		{
			_pdns_build(pdns, "edns0", NULL);
		}
#endif
		else if (!strcmp(x, "a6"))
		{
			_pdns_build(pdns, "a6", NULL);
		}

		else if (!strcmp(x, "dname"))
		{
			_pdns_build(pdns, "dname", NULL);
		}

		else if (!strcmp(x, "default"))
		{
			_pdns_build(pdns, "default", NULL);
		}

		else if (!strcmp(x, "pdns"))
		{
			_pdns_build(pdns, "pdns", NULL);
		}

		else if (!strcmp(x, "mdns"))
		{
			_pdns_build(pdns, "mdns", NULL);
		}
	}

	_pdns_build_finish(pdns);
	return pdns;
}

static pdns_handle_t *
_mdns_primary(dns_resolver_t *r)
{
	pdns_handle_t *pdns;
	char *val, *p;
	int i;

	pdns = _pdns_build_start(r->domain);
	if (r->domain == NULL) _pdns_build(pdns, "default", NULL);

	p = getenv("RES_RETRY_TIMEOUT");
	if (p != NULL) pdns->send_timeout = atoi(p);

	p = getenv("RES_RETRY");
	if (p != NULL) pdns->res->retry= atoi(p);

	if (r->n_search > MAXDNSRCH) r->n_search = MAXDNSRCH;
	for (i = 0; i < r->n_search; i++)
	{
		val = NULL;
		asprintf(&val, "%s", r->search[i]);
		if (val == NULL)
		{
			_pdns_free(pdns);
			return NULL;
		}

		_pdns_build(pdns, "search", val);
		free(val);
	}

	_pdns_build(pdns, "mdns", NULL);

	_pdns_build_finish(pdns);
	return pdns;
}

/*
 * Open a named resolver client from the system config data.
 */
static pdns_handle_t *
_pdns_sc_open(const char *name)
{
	pdns_handle_t *pdns;
	int i;
	dns_config_t *sc_dns;
	dns_resolver_t *sc_res;

	sc_dns = dns_configuration_copy();
	if (sc_dns == NULL) return NULL;

	sc_res = NULL;

	if (name == NULL)
	{
		if (sc_dns->n_resolver != 0) sc_res = sc_dns->resolver[0];
	}
	else
	{
		for (i = 0; (sc_res == NULL) && (i < sc_dns->n_resolver); i++)
		{
			if (sc_dns->resolver[i] == NULL) continue;
			if (sc_dns->resolver[i]->domain == NULL) continue;
			if (!strcasecmp(name, sc_dns->resolver[i]->domain)) sc_res = sc_dns->resolver[i];
		}
	}

	if (sc_res == NULL)
	{
		dns_configuration_free(sc_dns);
		return NULL;
	}

	pdns = (pdns_handle_t *)calloc(1, sizeof(pdns_handle_t));
	if (pdns == NULL)
	{
		dns_configuration_free(sc_dns);
		return NULL;
	}

	pdns = _pdns_convert_sc(sc_res);

	dns_configuration_free(sc_dns);

	if (pdns == NULL) return NULL;

	if (pdns->res == NULL)
	{
		free(pdns);
		return NULL;
	}

	pdns->name = NULL;
	if (pdns->res->defdname[0] != '\0') _pdns_set_name(pdns, pdns->res->defdname);
	else if (name != NULL) _pdns_set_name(pdns, name);

	if (name != NULL) pdns->search_count = SEARCH_COUNT_INIT;

	return pdns;
}

/*
 * Open a named resolver client from file.
 */
static pdns_handle_t *
_pdns_file_open(const char *name)
{
	pdns_handle_t *pdns;
	char *path, buf[1024];
	char *p, *x, *y;
	FILE *fp;

	path = NULL;
	if (name == NULL)
	{
		asprintf(&path, "%s", _PATH_RESCONF);
	}
	else if ((name[0] == '.') || (name[0] == '/'))
	{
		asprintf(&path, "%s", name);
	}
	else
	{
		asprintf(&path, "%s/%s", DNS_RESOLVER_DIR, name);
	}

	if (path == NULL) return NULL;

	fp = fopen(path, "r");
	free(path);
	if (fp == NULL) return NULL;

	pdns = _pdns_build_start(NULL);
	if (pdns == NULL) 
	{
		fclose(fp);
		return NULL;
	}

	p = getenv("RES_RETRY_TIMEOUT");
	if (p != NULL) pdns->send_timeout = atoi(p);

	p = getenv("RES_RETRY");
	if (p != NULL) pdns->res->retry= atoi(p);

	while (fgets(buf, sizeof(buf), fp) != NULL)
	{
		/* skip comments */
		if ((buf[0] == ';') || (buf[0] == '#')) continue;
		p = buf;
		x = res_next_word(&p);
		if (x == NULL) continue;
		if (!strcmp(x, "sortlist"))
		{
			while (NULL != (x = res_next_word(&p)))
			{
				_pdns_build(pdns, "sortlist", x);
			}
		}
		else if (!strcmp(x, "timeout"))
		{
			x = res_next_word(&p);
			if (x != NULL) _pdns_build(pdns, "total_timeout", x);
		}
		else if (!strcmp(x, "options"))
		{
			while (NULL != (x = res_next_word(&p)))
			{
				y = strchr(x, ':');
				if (y != NULL)
				{
					*y = '\0';
					y++;
				}
				_pdns_build(pdns, x, y);
			}
		}
		else 
		{
			y = res_next_word(&p);
			_pdns_build(pdns, x, y);

			if ((!strcmp(x, "domain")) && (pdns->name == NULL)) _pdns_set_name(pdns, y);
		}
	}

	fclose(fp);

	if (pdns->name == NULL) _pdns_set_name(pdns, name);

	_pdns_build_finish(pdns);

	return pdns;
}

/*
 * If there was no search list, use domain name and parent domain components.
 *
 * N.B. This code deals with multiple trailing dots, but does not deal with
 * multiple internal dots, e.g. "foo.....com".  
 */
static void
_pdns_check_search_list(pdns_handle_t *pdns)
{
	int n;
	char *p;

	if (pdns == NULL) return;
	if (pdns->name == NULL) return;
	if (pdns->search_count > 0) return;

	/* Count dots */
	n = 0;
	for (p = pdns->name; *p != '\0'; p++) 
	{
		if (*p == '.') n++;
	}

	/* Back up over any trailing dots and cut them out of the name */
	for (p--; (p >= pdns->name) && (*p == '.'); p--)
	{
		*p = '\0';
		n--;
	}

	/* This will be true if name was all dots */
	if (p < pdns->name) return;

	/* dots are separators, so number of components is one larger */
	n++;

	_pdns_build(pdns, "search", pdns->name);

	/* Include parent domains with at least LOCALDOMAINPARTS components */
	p = pdns->name;
	while (n > LOCALDOMAINPARTS)
	{
		/* Find next component */
		while ((*p != '.') && (*p != '\0')) p++;
		if (*p == '\0') break;
		p++;

		n--;
		_pdns_build(pdns, "search", p);
	}
}

__private_extern__ void
_check_cache(sdns_handle_t *sdns)
{
	int i, n, status, refresh, sc_dns_count;
	DIR *dp;
	struct direct *d;
	pdns_handle_t *c;
	dns_config_t *sc_dns;

	if (sdns == NULL) return;

	refresh = 0;

	if (sdns->stattime == 0) refresh = 1;

	if (refresh == 0)
	{
		if (sdns->notify_sys_config_token == -1) refresh = 1;
		else
		{
			n = 1;
			status = notify_check(sdns->notify_sys_config_token, &n);
			if ((status != NOTIFY_STATUS_OK) || (n == 1)) refresh = 1;
		}
	}

	if (refresh == 0)
	{
		if (sdns->notify_dir_token == -1) refresh = 1;
		else
		{
			n = 1;
			status = notify_check(sdns->notify_dir_token, &n);
			if ((status != NOTIFY_STATUS_OK) || (n == 1)) refresh = 1;
		}
	}

	if (refresh == 0) return;

	/* Free old clients */
	sdns->pdns_primary = NULL;

	for (i = 0; i < sdns->client_count; i++)
	{
		_pdns_free(sdns->client[i]);
	}

	sdns->client_count = 0;
	if (sdns->client != NULL) free(sdns->client);
	sdns->client = NULL;

	/* Fetch clients from System Configuration */
	sc_dns = dns_configuration_copy();

	/* Set up Primary resolver.  It's the one we consult for a search list */
	sc_dns_count = 0;
	if ((sc_dns != NULL) && (sc_dns->n_resolver > 0))
	{
		if (sdns->flags & DNS_FLAG_FORWARD_TO_MDNSRESPONDER)
		{
			sc_dns_count = 1;
			sdns->pdns_primary = _mdns_primary(sc_dns->resolver[0]);
		}
		else
		{
			sc_dns_count = sc_dns->n_resolver;
			sdns->pdns_primary = _pdns_convert_sc(sc_dns->resolver[0]);
		}

		_pdns_check_search_list(sdns->pdns_primary);
	}
	else
	{
		sdns->pdns_primary = _pdns_file_open(_PATH_RESCONF);
	}

	if (sdns->pdns_primary != NULL)
	{
		if ((sdns->flags & DNS_FLAG_DEBUG) && (sdns->pdns_primary->res != NULL)) sdns->pdns_primary->res->options |= RES_DEBUG;
		if (sdns->flags & DNS_FLAG_OK_TO_SKIP_AAAA) sdns->pdns_primary->flags |= DNS_FLAG_OK_TO_SKIP_AAAA;
		sdns->pdns_primary->flags |= DNS_FLAG_DEFAULT_RESOLVER;

		sdns->client = (pdns_handle_t **)calloc(1, sizeof(pdns_handle_t *));
		if (sdns->client == NULL)
		{
			if (sc_dns != NULL) dns_configuration_free(sc_dns);
			return;
		}

		sdns->client[sdns->client_count] = sdns->pdns_primary;
		sdns->client_count++;
	}

	/* Convert System Configuration resolvers */
	for (i = 1; i < sc_dns_count; i++)
	{
		c = _pdns_convert_sc(sc_dns->resolver[i]);
		if (c == NULL) continue;

		if (sdns->flags & DNS_FLAG_DEBUG) c->res->options |= RES_DEBUG;
		if (sdns->flags & DNS_FLAG_OK_TO_SKIP_AAAA) c->flags |= DNS_FLAG_OK_TO_SKIP_AAAA;

		if (sdns->client_count == 0)
		{
			sdns->client = (pdns_handle_t **)calloc(1, sizeof(pdns_handle_t *));
		}
		else
		{
			sdns->client = (pdns_handle_t **)reallocf(sdns->client, (sdns->client_count + 1) * sizeof(pdns_handle_t *));
		}

		if (sdns->client == NULL)
		{
			sdns->client_count = 0;
			dns_configuration_free(sc_dns);
			return;
		}

		sdns->client[sdns->client_count] = c;
		sdns->client_count++;
	}

	if (sc_dns != NULL) dns_configuration_free(sc_dns);

	if (sdns->flags & DNS_FLAG_CHECK_RESOLVER_DIR)
	{
		/* Read /etc/resolvers clients */
		dp = opendir(DNS_RESOLVER_DIR);
		if (dp == NULL)
		{
			sdns->flags &= ~DNS_FLAG_CHECK_RESOLVER_DIR;
		}
		else
		{
			while (NULL != (d = readdir(dp)))
			{
				if (d->d_name[0] == '.') continue;

				c = _pdns_file_open(d->d_name);
				if (c == NULL) continue;
				if (sdns->flags & DNS_FLAG_DEBUG) c->res->options |= RES_DEBUG;
				if (sdns->flags & DNS_FLAG_OK_TO_SKIP_AAAA) c->flags |= DNS_FLAG_OK_TO_SKIP_AAAA;

				if (sdns->client_count == 0)
				{
					sdns->client = (pdns_handle_t **)calloc(1, sizeof(pdns_handle_t *));
				}
				else
				{
					sdns->client = (pdns_handle_t **)reallocf(sdns->client, (sdns->client_count + 1) * sizeof(pdns_handle_t *));
				}

				if (sdns->client == NULL)
				{
					sdns->client_count = 0;
					return;
				}

				sdns->client[sdns->client_count] = c;
				sdns->client_count++;
			}
			closedir(dp);
		}
	}

	sdns->stattime = 1;
}

static uint32_t
_pdns_get_default_handles(sdns_handle_t *sdns, pdns_handle_t ***pdns)
{
	int i, j, k, count;

	if (sdns == NULL) return 0;
	if (pdns == NULL) return 0;

	count = 0;

	for (i = 0; i < sdns->client_count; i++)
	{
		if (sdns->client[i]->flags & DNS_FLAG_DEFAULT_RESOLVER)
		{
			if (count == 0)
			{
				*pdns = (pdns_handle_t **)calloc(1, sizeof(pdns_handle_t *));
			}
			else
			{
				*pdns = (pdns_handle_t **)reallocf((*pdns), (count + 1) * sizeof(pdns_handle_t *));
			}

			if (*pdns == NULL) return 0;

			/* Insert sorted by search_order */
			for (j = 0; j < count; j++)
			{
				if (sdns->client[i]->search_order < (*pdns)[j]->search_order) break;
			}

			for (k = count; k > j; k--) (*pdns)[k] = (*pdns)[k-1];
			(*pdns)[j] = sdns->client[i];
			count++;
		}
	}

	return count;
}

static uint32_t
_pdns_get_handles_for_name(sdns_handle_t *sdns, const char *name, pdns_handle_t ***pdns)
{
	char *p, *vname;
	int i, j, k, count;

	if (sdns == NULL) return 0;
	if (pdns == NULL) return 0;

	if (name == NULL) return _pdns_get_default_handles(sdns, pdns);
	else if (name[0] == '\0') return _pdns_get_default_handles(sdns, pdns);

	count = 0;

	vname = strdup(name);
	i = strlen(vname) - 1;
	if ((i >= 0) && (vname[i] == '.')) vname[i] = '\0';

	p = vname;
	while (p != NULL)
	{
		for (i = 0; i < sdns->client_count; i++)
		{
			if (sdns->client[i]->name == NULL) continue;

			if (!strcasecmp(sdns->client[i]->name, p))
			{
				if (count == 0)
				{
					*pdns = (pdns_handle_t **)calloc(1, sizeof(pdns_handle_t *));
				}
				else
				{
					*pdns = (pdns_handle_t **)reallocf((*pdns), (count + 1) * sizeof(pdns_handle_t *));
				}

				if (*pdns == NULL) return 0;

				/* Insert sorted by search_order */
				for (j = 0; j < count; j++)
				{
					if (sdns->client[i]->search_order < (*pdns)[j]->search_order) break;
				}

				for (k = count; k > j; k--) (*pdns)[k] = (*pdns)[k-1];
				(*pdns)[j] = sdns->client[i];
				count++;
			}
		}

		p = strchr(p, '.');
		if (p != NULL) p++;
	}

	free(vname);

	if (count != 0) return count;

	return _pdns_get_default_handles(sdns, pdns);
}

static void
_pdns_process_res_search_list(pdns_handle_t *pdns)
{
	if (pdns->search_count != SEARCH_COUNT_INIT) return;
	for (pdns->search_count = 0; (pdns->res->dnsrch[pdns->search_count] != NULL) && (pdns->res->dnsrch[pdns->search_count][0] != '\0'); pdns->search_count++);
}

static char *
_pdns_search_list_domain(pdns_handle_t *pdns, uint32_t i)
{
	char *s;

	if (pdns == NULL) return NULL;
	if (pdns->search_count == SEARCH_COUNT_INIT) _pdns_process_res_search_list(pdns);
	if (i >= pdns->search_count) return NULL;

	s = pdns->search_list[i];
	if (s == NULL) return NULL;
	return strdup(s);
}

static void
_dns_open_notify(sdns_handle_t *sdns)
{
	int status, n;

	if (sdns == NULL) return;

	if (sdns->notify_delay_token == -1)
	{
		status = notify_register_check(DNS_DELAY_NAME, &(sdns->notify_delay_token));
		if (status != NOTIFY_STATUS_OK) sdns->notify_delay_token = -1;
		else status = notify_check(sdns->notify_delay_token, &n);
	}

	if (sdns->notify_sys_config_token == -1)
	{
		status = notify_register_check(dns_configuration_notify_key(), &(sdns->notify_sys_config_token));
		if (status != NOTIFY_STATUS_OK) sdns->notify_sys_config_token = -1;
	}

	if (sdns->notify_dir_token == -1)
	{
		status = notify_register_check(NOTIFY_DIR_NAME, &(sdns->notify_dir_token));
		if (status == NOTIFY_STATUS_OK)
		{
			status = notify_monitor_file(sdns->notify_dir_token, "/private/etc/resolver", 0);
			if (status != NOTIFY_STATUS_OK)
			{
				notify_cancel(sdns->notify_dir_token);
				sdns->notify_dir_token = -1;
			}
		}
		else 
		{
			sdns->notify_dir_token = -1;
		}
	}
}

static void
_dns_close_notify(sdns_handle_t *sdns)
{
	if (sdns == NULL) return;

	if (sdns->notify_delay_token != -1) notify_cancel(sdns->notify_delay_token);
	sdns->notify_delay_token = -1;

	if (sdns->notify_sys_config_token != -1) notify_cancel(sdns->notify_sys_config_token);
	sdns->notify_sys_config_token = -1;

	if (sdns->notify_dir_token != -1) notify_cancel(sdns->notify_dir_token);
	sdns->notify_dir_token = -1;
}

dns_handle_t
dns_open(const char *name)
{
	dns_private_handle_t *dns;
	struct stat sb;
	int check, status, local_control;
	uint64_t control;

	dns = (dns_private_handle_t *)calloc(1, sizeof(dns_private_handle_t));
	if (dns == NULL) return NULL;

	/* set up control notification if necessary */
	if (dns_control_token == -1)
	{
		pthread_mutex_lock(&dns_control_lock);
		if (dns_control_token == -1) status = notify_register_check(NOTIFY_DNS_CONTROL_NAME, &dns_control_token);
		pthread_mutex_unlock(&dns_control_lock);
	}

	/* check for dns flags */
	if (dns_control_token != -1)
	{
		pthread_mutex_lock(&dns_control_lock);
		status = notify_check(dns_control_token, &check);
		if ((status == 0) && (check == 1))
		{
			/* notification was triggered */
			status = notify_get_state(dns_control_token, &control);
			if (status == 0) 
			{
				if (control & DNS_CONTROL_FLAG_NO_MDNS) dns_control_mdns = 0;
				if (control & DNS_CONTROL_FLAG_DEBUG) dns_control_debug = 1;
			}
		}

		pthread_mutex_unlock(&dns_control_lock);
	}

	if (name == NULL) local_control = dns_control_mdns;
	else if (!strcmp(name, MDNS_HANDLE_NAME)) local_control = 2;
	else local_control = 0;

	if ((name == NULL) && (local_control == 0))
	{
		dns->handle_type = DNS_PRIVATE_HANDLE_TYPE_SUPER;
		dns->sdns = (sdns_handle_t *)calloc(1, sizeof(sdns_handle_t));
		if (dns->sdns == NULL)
		{
			free(dns);
			return NULL;
		}

		dns->sdns->flags |= DNS_FLAG_CHECK_RESOLVER_DIR;
		dns->sdns->notify_sys_config_token = -1;
		dns->sdns->notify_dir_token = -1;
		dns->sdns->notify_delay_token = -1;
		_dns_open_notify(dns->sdns);

		memset(&sb, 0, sizeof(struct stat));
		dns_set_debug((dns_handle_t)dns, dns_control_debug);

		return (dns_handle_t)dns;
	}

	if (local_control != 0)
	{
		dns->handle_type = DNS_PRIVATE_HANDLE_TYPE_SUPER;
		dns->sdns = (sdns_handle_t *)calloc(1, sizeof(sdns_handle_t));
		if (dns->sdns == NULL)
		{
			free(dns);
			return NULL;
		}

		dns->sdns->flags = DNS_FLAG_FORWARD_TO_MDNSRESPONDER;
		dns->sdns->notify_sys_config_token = -1;
		dns->sdns->notify_dir_token = -1;
		dns->sdns->notify_delay_token = -1;
		if (local_control == 1) _dns_open_notify(dns->sdns);

		memset(&sb, 0, sizeof(struct stat));
		dns_set_debug((dns_handle_t)dns, dns_control_debug);

		return (dns_handle_t)dns;
	}

	dns->handle_type = DNS_PRIVATE_HANDLE_TYPE_PLAIN;

	/*  Look for name in System Configuration first */
	dns->pdns = _pdns_sc_open(name);
	if (dns->pdns == NULL) dns->pdns = _pdns_file_open(name);

	if (dns->pdns == NULL)
	{
		free(dns);
		return NULL;
	}

	dns_set_debug((dns_handle_t)dns, dns_control_debug);
	return (dns_handle_t)dns;
}

/*
 * Release a DNS client handle
 */
void
dns_free(dns_handle_t d)
{
	dns_private_handle_t *dns;
	int i;

	if (d == NULL) return;

	dns = (dns_private_handle_t *)d;

	if (dns->recvbuf != NULL) free(dns->recvbuf);

	if (dns->handle_type == DNS_PRIVATE_HANDLE_TYPE_SUPER)
	{
		if (dns->sdns == NULL) return;

		_dns_close_notify(dns->sdns);

		for (i = 0; i < dns->sdns->client_count; i++)
		{
			_pdns_free(dns->sdns->client[i]);
		}

		dns->sdns->client_count = 0;
		if (dns->sdns->client != NULL) free(dns->sdns->client);

		free(dns->sdns);
	}
	else
	{
		_pdns_free(dns->pdns);
	}

	free(dns);
}

static void
_pdns_debug(pdns_handle_t *pdns, uint32_t flag)
{
	if (pdns == NULL) return;

	if (flag == 0)
	{
		pdns->res->options &= ~RES_DEBUG;
	}
	else
	{
		pdns->res->options |= RES_DEBUG;
	}
}

static void
_sdns_debug(sdns_handle_t *sdns, uint32_t flag)
{
	int i;

	if (sdns == NULL) return;

	if (flag == 0)
	{
		sdns->flags &= ~ DNS_FLAG_DEBUG;

		for (i = 0; i < sdns->client_count; i++)
		{
			sdns->client[i]->res->options &= ~RES_DEBUG;
		}
	}
	else
	{
		sdns->flags |= DNS_FLAG_DEBUG;

		for (i = 0; i < sdns->client_count; i++)
		{
			sdns->client[i]->res->options |= RES_DEBUG;
		}
	}
}

/*
 * Enable / Disable debugging
 */
void
dns_set_debug(dns_handle_t d, uint32_t flag)
{
	dns_private_handle_t *dns;

	if (d == NULL) return;

	dns = (dns_private_handle_t *)d;

	if (dns->handle_type == DNS_PRIVATE_HANDLE_TYPE_SUPER)
	{
		_sdns_debug(dns->sdns, flag);
	}
	else
	{
		_pdns_debug(dns->pdns, flag);
	}
}

/*
 * Returns the number of names in the search list
 */
uint32_t
dns_search_list_count(dns_handle_t d)
{
	dns_private_handle_t *dns;
	pdns_handle_t *pdns;

	if (d == NULL) return 0;

	dns = (dns_private_handle_t *)d;

	if (dns->handle_type == DNS_PRIVATE_HANDLE_TYPE_SUPER)
	{
		_check_cache(dns->sdns);
		pdns = dns->sdns->pdns_primary;
	}
	else
	{
		pdns = dns->pdns;
	}

	if (pdns->search_count == SEARCH_COUNT_INIT) _pdns_process_res_search_list(pdns);
	return pdns->search_count;
}

/* 
 * Returns the domain name at index i in the search list.
 * Returns NULL if there are no names in the search list.
 * Caller must free the returned name.
 */
char *
dns_search_list_domain(dns_handle_t d, uint32_t i)
{
	dns_private_handle_t *dns;
	pdns_handle_t *pdns;

	if (d == NULL) return NULL;

	dns = (dns_private_handle_t *)d;

	if (dns->handle_type == DNS_PRIVATE_HANDLE_TYPE_SUPER)
	{
		_check_cache(dns->sdns);
		pdns = dns->sdns->pdns_primary;
	}
	else
	{
		pdns = dns->pdns;
	}

	return _pdns_search_list_domain(pdns, i);
}

static int
_pdns_delay(sdns_handle_t *sdns)
{
	int status, n, snooze;
	time_t tick;

	if (sdns == NULL) return 0;

	snooze = 0;
	n = 0;

	/* No delay if we are not receiving notifications */
	if (sdns->notify_delay_token == -1) return 0;

	if (sdns->dns_delay == 0)
	{
		status = notify_check(sdns->notify_delay_token, &n);
		if ((status == NOTIFY_STATUS_OK) && (n == 1))
		{
			/*
			 * First thread to hit this condition sleeps for DNS_DELAY_INTERVAL seconds
			 */
			sdns->dns_delay = time(NULL) + DNS_DELAY_INTERVAL;
			snooze = DNS_DELAY_INTERVAL;
		}
	}
	else
	{
		tick = time(NULL);
		/*
		 * Subsequent threads sleep for the remaining duration.
		 * We add one to round up the interval since our granularity is coarse.
		 */
		snooze = 1 + (sdns->dns_delay - tick);
		if (snooze < 0) snooze = 0;
	}

	if (snooze == 0) return 0;

	sleep(snooze);

	/* When exiting, first thread in resets the delay condition */
	if (n == 1) sdns->dns_delay = 0;

	return 0;
}

static int
_pdns_query(sdns_handle_t *sdns, pdns_handle_t *pdns, const char *name, uint32_t class, uint32_t type, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen, int *min)
{
	int n;

	if (name == NULL) return -1;
	if (pdns == NULL) return -1;

	if (pdns->flags & DNS_FLAG_FORWARD_TO_MDNSRESPONDER)
	{
		n = res_query_mDNSResponder(pdns->res, name, class, type, (u_char *)buf, len, from, fromlen);
		if ((n < 0) && (min != NULL)) *min = MDNS_MIN_TTL;
		return n;
	}

	if (pdns->res == NULL) return -1;
	if (pdns->res->nscount == 0) return -1;

	if ((type == ns_t_aaaa) && ((pdns->flags & DNS_FLAG_HAVE_IPV6_SERVER) == 0) && (pdns->flags & DNS_FLAG_OK_TO_SKIP_AAAA)) return -1;

	_pdns_delay(sdns);

	/* BIND_9 API */
	return res_nquery_soa_min(pdns->res, name, class, type, (u_char *)buf, len, from, (int32_t *)fromlen, min);
}

__private_extern__ int
_pdns_search(sdns_handle_t *sdns, pdns_handle_t *pdns, const char *name, uint32_t class, uint32_t type, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen)
{
	char *dot, *qname;
	int append, status;

	if (name == NULL) return -1;
	if (pdns == NULL) return -1;
	if (pdns->res == NULL) return -1;
	if (pdns->res->nscount == 0) return -1;

	if ((type == ns_t_aaaa) && ((pdns->flags & DNS_FLAG_HAVE_IPV6_SERVER) == 0) && (pdns->flags & DNS_FLAG_OK_TO_SKIP_AAAA)) return -1;

	qname = NULL;
	append = 1;

	/*
	 * don't append my name if:
	 * - my name is NULL
	 * - input name is qualified (i.e. not single component)
	 * - there is a search list
	 * - there is a domain name
	 */

	if (pdns->name == NULL) append = 0;

	if (append == 1)
	{
		dot = strrchr(name, '.');
		if (dot != NULL) append = 0;
	}

	if (append == 1)
	{
		if (pdns->search_count == SEARCH_COUNT_INIT) _pdns_process_res_search_list(pdns);
		if (pdns->search_count > 0) append = 0;
	}

	if ((append == 1) && (pdns->res->defdname != NULL) && (pdns->res->defdname[0] != '\0')) append = 0;

	status = -1;
	if (append == 0)
	{
		/* BIND_9 API */
		_pdns_delay(sdns);

		status = __res_nsearch_list_2(pdns->res, name, class, type, (u_char *)buf, len, from, fromlen, pdns->search_count, pdns->search_list);
	}
	else
	{
		_pdns_delay(sdns);

		qname = NULL;
		asprintf(&qname, "%s.%s.", name, pdns->name);
		if (qname == NULL) return -1;

		/* BIND_9 API */
		status = res_nsearch_2(pdns->res, qname, class, type, (u_char *)buf, len, from, fromlen);
		free(qname);
	}

	return status;
}

static int
_sdns_send(sdns_handle_t *sdns, const char *name, uint32_t class, uint32_t type, uint32_t fqdn, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen, int *min)
{
	char *qname;
	pdns_handle_t **pdns;
	uint32_t pdns_count;
	int i, n;
	int m, tmin, minstate;

	pdns = NULL;
	pdns_count = 0;
	n = -1;
	minstate = 0;
	*min = -1;
	m = -1;

	pdns_count = _pdns_get_handles_for_name(sdns, name, &pdns);

	if (pdns_count == 0) return -1;

	qname = NULL;
	asprintf(&qname, "%s%s", name, (fqdn == 0) ? "." : "");
	if (qname == NULL) return -1;

	for (i = 0; i < pdns_count; i++)
	{
		tmin = -1;
		n = _pdns_query(sdns, pdns[i], qname, class, type, buf, len, from, fromlen, &tmin);
		if (n <= 0)
		{
			if (tmin < 0)
			{
				minstate = -1;
			}
			else if (minstate == 0)
			{
				if (m == -1) m = tmin;
				else if (tmin < m) m = tmin;
			}
		}

		if (n > 0) break;
	}

	if (minstate == 0) *min = m;

	free(pdns);
	free(qname);
	return n;
}

__private_extern__ int
_sdns_search(sdns_handle_t *sdns, const char *name, uint32_t class, uint32_t type, uint32_t fqdn, uint32_t recurse, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen, int *min)
{
	pdns_handle_t *primary, **pdns;
	int i, n, ndots, status;
	int m, tmin, minstate;
	char *dot, *qname;
	uint32_t pdns_count;

	if (sdns == NULL) return -1;
	if (name == NULL) return -1;

	/*
	 * A minimum TTL derived from the minimim of all SOA records
	 * that are received with NXDOMAIN or no data is returned to
	 * the caller if every call returns an NXDOMAIN or no data
	 * and a SOA min ttl.  If any call times out or returns some
	 * other error, we return "-1" in the "min" out parameter.
	 * The minstate variable is set to -1 if we must return -1.
	 */
	minstate = 0;
	*min = -1;

	/* m is the lowest of all minima.  -1 is unset */
	m = -1;

	/* ndots is the threshold for trying a qualified name "as is" */
	ndots = 1;
	primary = sdns->pdns_primary;
	if ((primary != NULL) && (primary->res != NULL)) ndots = primary->res->ndots;

	/* count dots in input name, and keep track of the location of the last dot */
	n = 0;
	dot = NULL;

	for (i = 0; name[i] != '\0'; i++)
	{
		if (name[i] == '.')
		{
			n++;
			dot = (char *)(name + i);
		}
	}

	/* the last dot is the last character, name is fully qualified */
	if ((fqdn == 0) && (dot != NULL) && (*(dot + 1) == '\0')) fqdn = 1;

	/*
	 * If n >= ndots, or it's a FQDN, or if it's a PTR query,
	 * we try a query with the name "as is".
	 */
	if ((n >= ndots) || (fqdn == 1) || (type == ns_t_ptr))
	{
		tmin = -1;
		status = _sdns_send(sdns, name, class, type, fqdn, buf, len, from, fromlen, &tmin);
		if (status > 0) return status;

		if (tmin < 0) minstate = -1;
		else m = tmin;
	}

	/* end of the line for FQDNs or PTR queries */
	if ((fqdn == 1) || (type == ns_t_ptr) || (recurse == 0) || (primary == NULL))
	{
		if (minstate == 0) *min = m;
		return -1;
	}

	/* Try appending names from the search list */
	if (primary->search_count == SEARCH_COUNT_INIT) _pdns_process_res_search_list(primary);
	n = primary->search_count;
	if (n > 0)
	{
		/* Try qualifying with each name in the search list */
		for (i = 0; i < n ; i++)
		{
			qname = NULL;
			asprintf(&qname, "%s.%s", name, primary->search_list[i]);
			if (qname == NULL) return -1;

			tmin = -1;
			status = _sdns_search(sdns, qname, class, type, fqdn, 0, buf, len, from, fromlen, &tmin);
			if (status <= 0)
			{
				if (tmin < 0)
				{
					minstate = -1;
				}
				else if (minstate == 0)
				{
					if (m == -1) m = tmin;
					else if (tmin < m) m = tmin;
				}
			}

			free(qname);
			if (status > 0) return status;
		}

		if (minstate == 0) *min = m;
		return -1;
	}

	/*
	 * We get here if the name is not fully qualified (no trailing dot), and there is no search list.
	 * Try each default client, qualifying with that client's name.
	 */
	pdns = NULL;
	pdns_count = _pdns_get_default_handles(sdns, &pdns);
	status = -1;

	if (pdns_count == 0)
	{
		if (minstate == 0) *min = m;
		return -1;
	}

	for (i = 0; i < pdns_count; i++)
	{
		qname = NULL;
		if (pdns[i]->name == NULL) asprintf(&qname, "%s", name);
		else asprintf(&qname, "%s.%s", name, pdns[i]->name);

		/* leave *min at -1 in case of a malloc failure */
		if (qname == NULL) return -1;

		tmin = -1;
		status = _pdns_query(sdns, pdns[i], qname, class, type, buf, len, from, fromlen, &tmin);
		if (status <= 0)
		{
			if (tmin < 0)
			{
				minstate = -1;
			}
			else if (minstate == 0)
			{
				if (m == -1) m = tmin;
				else if (tmin < m) m = tmin;
			}
		}

		free(qname);
		if (status > 0) break;
	}

	free(pdns);

	if (minstate == 0) *min = m;
	return status;
}

int
dns_query(dns_handle_t d, const char *name, uint32_t class, uint32_t type, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen)
{
	dns_private_handle_t *dns;
	int status, unused;

	if (d == NULL) return -1;
	if (name == NULL) return -1;
	dns = (dns_private_handle_t *)d;

	status = -1;
	unused = 0;

	if (dns->handle_type == DNS_PRIVATE_HANDLE_TYPE_SUPER)
	{
		_check_cache(dns->sdns);
		status = _sdns_search(dns->sdns, name, class, type, 1, 1, buf, len, from, fromlen, &unused);
	}
	else
	{
		status = _pdns_query(dns->sdns, dns->pdns, name, class, type, buf, len, from, fromlen, &unused);
	}

	return status;
}


int
dns_search(dns_handle_t d, const char *name, uint32_t class, uint32_t type, char *buf, uint32_t len, struct sockaddr *from, uint32_t *fromlen)
{
	dns_private_handle_t *dns;
	int status, unused;

	if (d == NULL) return -1;
	if (name == NULL) return -1;
	dns = (dns_private_handle_t *)d;

	status = -1;
	unused = 0;

	if (dns->handle_type == DNS_PRIVATE_HANDLE_TYPE_SUPER)
	{
		_check_cache(dns->sdns);
		status = _sdns_search(dns->sdns, name, class, type, 0, 1, buf, len, from, fromlen, &unused);
	}
	else
	{
		status = _pdns_search(dns->sdns, dns->pdns, name, class, type, buf, len, from, fromlen);
	}

	return status;
}

/*
 * PRIVATE
 */

uint32_t
dns_server_list_count(dns_handle_t d)
{
	dns_private_handle_t *dns;
	res_state r;

	if (d == NULL) return 0;
	dns = (dns_private_handle_t *)d;

	if (dns->handle_type != DNS_PRIVATE_HANDLE_TYPE_PLAIN) return 0;

	if (dns->pdns == NULL) return 0;

	r = dns->pdns->res;
	if (r == NULL) return 0;

	return r->nscount;
}

struct sockaddr *
dns_server_list_address(dns_handle_t d, uint32_t i)
{
	dns_private_handle_t *dns;
	res_state r;
	struct sockaddr_storage *s;
	struct sockaddr *sa;

	if (d == NULL) return NULL;
	dns = (dns_private_handle_t *)d;

	if (dns->handle_type != DNS_PRIVATE_HANDLE_TYPE_PLAIN) return NULL;

	if (dns->pdns == NULL) return NULL;

	r = dns->pdns->res;
	if (r == NULL) return NULL;

	if (i >= r->nscount) return NULL;
	sa = get_nsaddr(r, i);
	if (sa == NULL) return NULL;

	s = (struct sockaddr_storage *)calloc(1, sizeof(struct sockaddr_storage));
	if (s == NULL) return NULL;

	memcpy(s, sa, sizeof(struct sockaddr_storage));
	return (struct sockaddr *)s;
}

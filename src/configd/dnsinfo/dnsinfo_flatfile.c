/*
 * Copyright (c) 2009, 2011, 2012, 2014 Apple Inc. All rights reserved.
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

#include <dirent.h>
#include <fcntl.h>
#include <libgen.h>
#include <netdb.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <sys/dir.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "dnsinfo.h"
#include "dnsinfo_private.h"
#include "dnsinfo_create.h"

static uint32_t _dnsinfo_flatfile_flags;

enum {
	TOKEN_DOMAIN,
	TOKEN_FLAGS,
	TOKEN_INTERFACE,
	TOKEN_NAMESERVER,
	TOKEN_OPTIONS,
	TOKEN_PORT,
	TOKEN_SEARCH,
	TOKEN_SEARCH_ORDER,
	TOKEN_SORTLIST,
	TOKEN_TIMEOUT,
	TOKEN_MAX
};

/*
 * tokens
 * The supported configuration token strings and enumerated values.
 */
static const struct {
	const char	*name;
	int		token;
	int		max_count;
} tokens [] = {
	{ "domain",		TOKEN_DOMAIN,		1	},
	{ "flags",		TOKEN_FLAGS,		1	},
	{ "interface",		TOKEN_INTERFACE,	1	},
	{ "nameserver",		TOKEN_NAMESERVER,	MAXNS	},
	{ "options",		TOKEN_OPTIONS,		1	},
	{ "port",		TOKEN_PORT,		1	},
	{ "search",		TOKEN_SEARCH,		1	},
	{ "search_order",	TOKEN_SEARCH_ORDER,	1	},
	{ "sortlist",		TOKEN_SORTLIST,		1	},
	{ "timeout",		TOKEN_TIMEOUT,		1	},
};


/*
 * _dnsinfo_parse_address
 *
 * Parse IP address
 */
static struct sockaddr *
_dnsinfo_parse_address(char *nameserver)
{
	struct addrinfo	*ai;
	struct addrinfo	hints;
	int		res;
	struct sockaddr	*sa	= NULL;

	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_NUMERICHOST;

	res = getaddrinfo(nameserver, NULL, &hints, &ai);
	if (res == 0) {
		if ((ai->ai_family == AF_INET) || (ai->ai_family == AF_INET6)) {
			sa = malloc(ai->ai_addrlen);
			memcpy(sa, ai->ai_addr, ai->ai_addrlen);
		}
		freeaddrinfo(ai);
	}

	return sa;
}


/*
 * _dnsinfo_parse_nameserver
 *
 * Parse arguments to the nameserver token. This is essentially a getaddrinfo(3)
 * with AI_NUMERICHOST. However, if the conversion fails, check if the address
 * contains an optional trailing '.' followed by a numeric port number.  If found,
 * remove the port number and retry the conversion (e.g. 127.0.0.1.55 or ::1.55).
 */
static struct sockaddr *
_dnsinfo_parse_nameserver(char *token)
{
	char		*dot;
	long		number;
	struct sockaddr	*sa;

	sa = _dnsinfo_parse_address(token);
	if (sa != NULL) {
		return sa;
	}

	// if we could not parse address, attempt to remove
	// an optional trailing port number
	dot = strrchr(token, '.');
	if (dot == NULL) {
		return NULL;
	}

	number = strtol(dot + 1, NULL, 10);
	if ((number < 0) || (number > UINT16_MAX)) {
		return NULL;
	}

	*dot = '\0';
	sa = _dnsinfo_parse_address(token);
	if (sa != NULL) {
		in_port_t	port	= htons(number);

		switch (sa->sa_family) {
			case AF_INET :
				/* ALIGN: cast ok, sockaddr was malloc'd */
				((struct sockaddr_in *)(void *)sa)->sin_port = port;
				break;
			case AF_INET6 :
				/* ALIGN: cast ok, sockaddr was malloc'd */
				((struct sockaddr_in6 *)(void *)sa)->sin6_port = port;
				break;
		}
	}

	return sa;
}


/*
 * _dnsinfo_parse_sortaddr
 *
 * Parse arguments to the sortlist token.
 */
static dns_sortaddr_t *
_dnsinfo_parse_sortaddr(char *token)
{
	struct in_addr		addr;
	struct in_addr		mask;
	struct sockaddr		*sa;
	char			*slash;
	dns_sortaddr_t		*sortaddr	= NULL;

	slash = strchr(token, '/');
	if (slash != NULL) {
		*slash = '\0';
	}

	sa = _dnsinfo_parse_address(token);
	if (sa == NULL) {
		// if we could not parse the address
		goto done;
	} else if (sa->sa_family != AF_INET) {
		// if not AF_INET
		goto done;
	} else {
		/* ALIGN: cast ok, sockaddr was malloc'd */
		addr = ((struct sockaddr_in *)(void *)sa)->sin_addr;
		free(sa);
		sa = NULL;
	}

	if (slash != NULL) {
		sa = _dnsinfo_parse_address(slash + 1);
		if (sa == NULL) {
			// if we could not parse the provided mask
			goto done;
		} else if (sa->sa_family != AF_INET) {
			// if mask not AF_INET
			goto done;
		} else {
			/* ALIGN: cast ok, sockaddr was malloc'd */
			mask = ((struct sockaddr_in *)(void *)sa)->sin_addr;
			free(sa);
			sa = NULL;
		}
	} else {
		in_addr_t	a;
		in_addr_t	m;

		a = ntohl(addr.s_addr);
		if (IN_CLASSA(a)) {
			m = IN_CLASSA_NET;
		} else if (IN_CLASSB(a)) {
			m = IN_CLASSB_NET;
		} else if (IN_CLASSC(a)) {
			m = IN_CLASSC_NET;
		} else {
			goto done;
		}

		mask.s_addr = htonl(m);
	}

	sortaddr = malloc(sizeof(*sortaddr));
	sortaddr->address = addr;
	sortaddr->mask    = mask;

    done :

	if (sa != NULL) free(sa);
	return sortaddr;
}


/*
 * _dnsinfo_flatfile_set_flags
 *
 * Set the default resolver flags.
 */
__private_extern__
void
_dnsinfo_flatfile_set_flags(uint32_t flags)
{
	_dnsinfo_flatfile_flags = flags;
	return;
}


static void
_dnsinfo_flatfile_update_flags(dns_create_resolver_t *_resolver)
{
	uint32_t		new_flags;
	uint32_t		old_flags;
	_dns_resolver_buf_t	*resolver	= (_dns_resolver_buf_t *)*_resolver;

	old_flags = ntohl(resolver->resolver.flags);
	new_flags = old_flags | _dnsinfo_flatfile_flags;
	_dns_resolver_set_flags(_resolver, new_flags);
	return;
}


/*
 * _dnsinfo_flatfile_create_resolver
 *
 * Create a new dns resolver configuration from the configuration file at the
 * specified path. (e.g. /etc/resolv.conf or /etc/resolver/apple.com)
 */
static dns_create_resolver_t
_dnsinfo_flatfile_create_resolver(const char *dir, const char *path)
{
	char			*buf;
	uint32_t		config_flags		= 0;
	FILE			*f;
	char			filename[FILENAME_MAX];
	size_t			len			= 0;
	char			*line			= NULL;
	dns_create_resolver_t	res			= NULL;
	const char		*sep			= " \t";
	int			token_count[TOKEN_MAX]	= { 0 };

	filename[0] = 0;
	if (dir != NULL) {
		strlcpy(filename, dir, sizeof(filename));
		strlcat(filename, "/", sizeof(filename));
	}
	strlcat(filename, path, sizeof(filename));

	f = fopen(filename, "r");
	if (f == NULL) return NULL;

	while ((buf = fgetln(f, &len)) != NULL) {
		int	i;
		char	*lineptr;
		int	max_count;
		int	token;
		char	*word;

		if (len == 0) continue;
		if (buf[len-1] == '\n') buf[len-1] = '\0';

		line = reallocf(line, len+1);
		if (line == NULL) continue;

		strncpy(line, buf, len);
		line[len] = '\0';

		// parse the first word of the line (the config token)
		lineptr = line;
		word = strsep(&lineptr, sep);
		if (word == NULL) {
			// if empty line
			continue;
		}
		if (word[0] == ';' || word[0] == '#') {
			// if comment
			continue;
		}

		// translate config token to enumerated value
		token = -1;
		for (i = 0; i < sizeof(tokens) / sizeof(tokens[0]); i++) {
			if (strcasecmp(word, tokens[i].name) == 0) {
				token     = tokens[i].token;
				max_count = tokens[i].max_count;
				break;
			}
		}
		if (token == -1) {
			// if not a recognized token
			continue;
		}

		// parse the next word of the line (the config option)
		word = strsep(&lineptr, sep);
		if (word == NULL) {
			// if no option
			continue;
		}
		if (++token_count[token] > max_count) {
			// if too many options
			continue;
		}

		// create resolver
		if (res == NULL) {
			res = _dns_resolver_create();
			if (res == NULL) {
				// if we could not create a resolver
				goto done;
			}
		}

		switch (token) {
			case TOKEN_DOMAIN: {
				size_t	len;

				len = strlen(word);
				while ((len > 0) && (word[len - 1] == '.')) {
					// trim trailing '.'
					word[--len] = '\0';
				}
				if (len > 0) {
					_dns_resolver_set_domain(&res, word);
				}
				break;
			}

			case TOKEN_FLAGS: {
				while (word != NULL) {
					if (word[0] != '\0') {
						if (strcasecmp(word, "scoped") == 0) {
							config_flags |= DNS_RESOLVER_FLAGS_SCOPED;
						} else if (strcasecmp(word, "a") == 0) {
							config_flags |= DNS_RESOLVER_FLAGS_REQUEST_A_RECORDS;
						} else if (strcasecmp(word, "aaaa") == 0) {
							config_flags |= DNS_RESOLVER_FLAGS_REQUEST_AAAA_RECORDS;
						}
					}
					word = strsep(&lineptr, sep);
				}
				break;
			}

			case TOKEN_INTERFACE: {
				unsigned int	if_index;

				if_index = if_nametoindex(word);
				if (if_index > 0) {
					_dns_resolver_set_if_index(&res, if_index);
				}
				break;
			}

			case TOKEN_NAMESERVER: {
				struct sockaddr	*sa;

				sa = _dnsinfo_parse_nameserver(word);
				if (sa != NULL) {
					_dns_resolver_add_nameserver(&res, sa);
					free(sa);
				}
				break;
			}

			case TOKEN_OPTIONS: {
				char	*options	= NULL;

				while (word != NULL) {
					if (word[0] != '\0') {
						if (options == NULL) {
							options = malloc(len+1);
							if (options == NULL) break;

							strlcpy(options, word, len+1);
						} else {
							strlcat(options, " ", len+1);
							strlcat(options, word, len+1);
						}
					}
					word = strsep(&lineptr, sep);
				}

				if (options != NULL) {
					_dns_resolver_set_options(&res, options);
					free(options);
				}
				break;
			}

			case TOKEN_PORT: {
				long	number	= -1;

				number = strtol(word, NULL, 0);
				if (number < 0 || number > UINT16_MAX) break;
				_dns_resolver_set_port(&res, number);
				break;
			}

			case TOKEN_SEARCH: {
				int	n	= 0;

				// multiple search domains are supported
				while ((word != NULL) && (n++ < MAXDNSRCH)) {
					size_t	len;

					len = strlen(word);
					while ((len > 0) && (word[len - 1] == '.')) {
						// trim trailing '.'
						word[--len] = '\0';
					}
					if (len > 0) {
						_dns_resolver_add_search(&res, word);
					}
					word = strsep(&lineptr, sep);
				}
				break;
			}

			case TOKEN_SEARCH_ORDER: {
				long	number	= -1;

				number = strtol(word, NULL, 0);
				if (number < 0 || number > UINT32_MAX) break;
				_dns_resolver_set_order(&res, (uint32_t)number);
				break;
			}

			case TOKEN_SORTLIST: {
				int	n	= 0;

				while ((word != NULL) && (n++ < MAXRESOLVSORT)) {
					dns_sortaddr_t	*sortaddr;

					sortaddr = _dnsinfo_parse_sortaddr(word);
					if (sortaddr == NULL) break;
					_dns_resolver_add_sortaddr(&res, sortaddr);
					free(sortaddr);
					word = strsep(&lineptr, sep);
				}
				break;
			}

			case TOKEN_TIMEOUT: {
				long	number	= -1;

				number = strtol(word, NULL, 0);
				if (number < 0 || number > UINT32_MAX) break;
				_dns_resolver_set_timeout(&res, (uint32_t)number);
				break;
			}
		}
	}
	if (line != NULL) free(line);

	// set the domain to the basename of the path if not specified
	if ((res != NULL) && (token_count[TOKEN_DOMAIN] == 0)) {
		const char	*domain;

		domain = strrchr(path, '/');
		if (domain == NULL) {
			domain = path;
		} else {
			domain = domain + 1;
		}
		_dns_resolver_set_domain(&res, domain);
	}

	if (res != NULL) {
		// config flags should overwrite any default flags
		if (config_flags != 0) {
			_dns_resolver_set_flags(&res, config_flags);
		} else {
			_dnsinfo_flatfile_update_flags(&res);
		}
	}

    done :

	fclose(f);
	return res;
}


/*
 * _dnsinfo_flatfile_add_resolvers
 *
 * Parse the files in the resolver config directory (/etc/resolver) and add each
 * resolver to the dns config.
 */
__private_extern__
void
_dnsinfo_flatfile_add_resolvers(dns_create_config_t *config)
{
	struct dirent		*de;
	DIR			*dp;
	dns_create_resolver_t	res;

	dp = opendir(_PATH_RESOLVER_DIR);
	if (dp == NULL) {
		return;
	}

	while ((de = readdir(dp)) != NULL) {
		if (strcmp(de->d_name, ".") == 0 ||
		    strcmp(de->d_name, "..") == 0) continue;

		res = _dnsinfo_flatfile_create_resolver(_PATH_RESOLVER_DIR, de->d_name);
		if (res != NULL) {
			_dns_configuration_add_resolver(config, res);
			_dns_resolver_free(&res);
		}
	}

	closedir(dp);
	return;
}


#ifdef	MAIN
#undef	MAIN

#include "dnsinfo_copy.c"

int
main(int argc, char **argv)
{
	uint8_t			*buf;
	dns_config_t		*config;
	dns_create_config_t	create_config;
	_dns_config_buf_t	*config_buf;
	uint32_t		n_config;
	uint32_t		n_padding;
	dns_create_resolver_t	resolver;

	resolver = _dnsinfo_flatfile_create_resolver(NULL, _PATH_RESCONF);

	create_config = _dns_configuration_create();
	_dnsinfo_flatfile_add_resolvers(&create_config);

	config_buf = (_dns_config_buf_t *)create_config;
	n_config  = sizeof(_dns_config_buf_t) + ntohl(config_buf->n_attribute);
	n_padding = ntohl(config_buf->n_padding);
	buf = malloc(n_config + n_padding);
	bcopy((void *)config_buf, buf, n_config);
	bzero(&buf[n_config], n_padding);
	config = expand_config((_dns_config_buf_t *)buf);

	return 0;
}

#endif

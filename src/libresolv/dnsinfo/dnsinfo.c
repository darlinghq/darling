#include "dnsinfo.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static char* remove_comment(char* line);
static char* remove_whitespace(char* line);

/**
 * This is a very simple parser of /etc/resolv.conf.
 * It is not intended to be feature complete.
 * It only serves as an interim solution until we have XPC and configd.
 */

const char *
dns_configuration_notify_key    ()
{
	return NULL;
}

dns_config_t *
dns_configuration_copy		()
{
#if 0
	dns_config_t* config;
	dns_resolver_t* resolver;
	FILE* file;
	char buf[256];
	
	config = (dns_config_t*) malloc(sizeof(dns_config_t));
	memset(config, 0, sizeof(*config));

	file = fopen("/etc/resolv.conf", "r");
	if (file == NULL)
		return config;

	resolver = (dns_resolver_t*) malloc(sizeof(dns_resolver_t));
	memset(resolver, 0, sizeof(*resolver));

	config->n_resolver = 1;
	config->resolver = (dns_resolver_t**) malloc(sizeof(void*));
	config->resolver[0] = resolver;

	while (fgets(buf, sizeof(buf), file) != NULL)
	{
		char* line;

		line = remove_comment(buf);
		if (line == NULL)
			continue;

		line = remove_whitespace(line);
		if (line == NULL || line[0] == 0)
			continue;

		if (strncmp(line, "nameserver", 10) == 0)
		{
			char* srv;
			int is_ipv6;
			struct sockaddr* addr;
			int res;

			srv = line + 11;
			while (isspace(*srv))
				srv++;

			is_ipv6 = strchr(srv, ':') != NULL;

			if (is_ipv6)
			{
				struct sockaddr_in6* a6;

				a6 = (struct sockaddr_in6*) malloc(sizeof(struct sockaddr_in6));
				addr = (struct sockaddr*) a6;

				a6->sin6_family = AF_INET6;
				a6->sin6_port = htons(53);

				res = inet_pton(AF_INET6, srv, &a6->sin6_addr);
			}
			else
			{
				struct sockaddr_in* a4;

				a4 = (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
				addr = (struct sockaddr*) a4;

				a4->sin_family = AF_INET;
				a4->sin_port = htons(53);

				res = inet_pton(AF_INET, srv, &a4->sin_addr);
			}

			if (res == 1)
			{
				resolver->n_nameserver++;

				resolver->nameserver = (struct sockaddr**) realloc(resolver->nameserver,
						sizeof(void*) * resolver->n_nameserver);

				resolver->nameserver[resolver->n_nameserver-1] = addr;
			}
			else
			{
				free(addr);
			}
		}
		else if (strncmp(line, "search", 6) == 0)
		{
			char* dom;

			dom = line + 7;
			while (isspace(*dom))
				dom++;

			resolver->n_search++;
			resolver->search = (char**) realloc(resolver->search,
					sizeof(char*) * resolver->n_search);

			resolver->search[resolver->n_search-1] = strdup(dom);
		}
		else if (strncmp(line, "domain", 6) == 0)
		{
			char* dom;

			dom = line + 7;
			while (isspace(*dom))
				dom++;

			resolver->domain = strdup(dom);
		}
	}

	fclose(file);

	return config;
#else
	return NULL;
#endif
}

void
dns_configuration_free		(dns_config_t	*config)
{
#if 0
	if (config->n_resolver > 0)
	{
		dns_resolver_t* resolver;

		resolver = config->resolver[0];

		free(resolver->domain);

		while (resolver->n_search > 0)
		{
			free(resolver->search[resolver->n_search-1]);
			resolver->n_search--;
		}
		free(resolver->search);

		while (resolver->n_nameserver > 0)
		{
			free(resolver->nameserver[resolver->n_nameserver-1]);
			resolver->n_nameserver--;
		}
		free(resolver->nameserver);
	}

	free(config->resolver);
	free(config);
#endif
}

void
_dns_configuration_ack		(dns_config_t	*config,
				 const char	*bundle_id)
{
}

static char* remove_comment(char* line)
{
	char* p;

	if (line[0] == '#')
		return NULL;

	p = strchr(line, '#');
	if (p != NULL)
		*p = 0;

	return line;
}

static char* remove_whitespace(char* line)
{
	char* end;

	while (*line && isspace(*line))
		line++;

	end = line + strlen(line) - 1;

	while (end > line && isspace(*end))
	{
		*end = 0;
		end--;
	}

	return line;
}


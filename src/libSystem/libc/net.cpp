#include "config.h"
#include "net.h"
#include "common/auto.h"
#include "kernel-bsd/sockets.h"
#include <cstring>
#include <string>
#include <iostream>
#include <arpa/inet.h>

static Darling::MappedFlag g_flags[] = {
	{ DARWIN_AI_ADDRCONFIG, AI_ADDRCONFIG },
	{ DARWIN_AI_ALL, AI_ALL },
	{ DARWIN_AI_CANONNAME, AI_CANONNAME },
	{ DARWIN_AI_NUMERICHOST, AI_NUMERICHOST },
	{ DARWIN_AI_NUMERICSERV, AI_NUMERICSERV },
	{ DARWIN_AI_PASSIVE, AI_PASSIVE },
	{ DARWIN_AI_V4MAPPED, AI_V4MAPPED }
};

int __darwin_getaddrinfo(const char *hostname, const char *servname, const struct __darwin_addrinfo *hints, struct __darwin_addrinfo **res)
{
	struct addrinfo* chints;

	if (hints)
	{
		chints = new struct addrinfo;

		const struct __darwin_addrinfo* next = hints;
		struct addrinfo* cnext = chints;

		while (next)
		{
			memset(cnext, 0, sizeof(*cnext));
			cnext->ai_family = Darling::familyDarwinToLinux(next->ai_family); 
			cnext->ai_flags = Darling::flagsDarwinToNative(g_flags, sizeof(g_flags)/sizeof(g_flags[0]), next->ai_flags);
			cnext->ai_socktype = next->ai_socktype;

			next = next->ai_next;
			
			if (next)
			{
				cnext->ai_next = new struct addrinfo;
				cnext = cnext->ai_next;
			}
			else
				cnext->ai_next = 0;
		}
	}
	else
		chints = 0;
	
	struct addrinfo* nres;
	int rv = getaddrinfo(hostname, servname, chints, &nres);
	
	// delete native hints
	while (chints)
	{
		struct addrinfo* i = chints->ai_next;
		delete chints;
		chints = i;
	}
	
	*res = 0;
	if (!rv)
	{
		// convert the native addrinfo
		struct addrinfo* next = nres;
		struct __darwin_addrinfo* cnext = *res = new __darwin_addrinfo;
		while (next)
		{
			cnext->ai_family = Darling::familyLinuxToDarwin(next->ai_family);
			cnext->ai_flags = Darling::flagsNativeToDarwin(g_flags, sizeof(g_flags)/sizeof(g_flags[0]), next->ai_flags);

			if (next->ai_canonname)
				cnext->ai_canonname = strdup(next->ai_canonname);
			else
				cnext->ai_canonname = 0;

			cnext->ai_socktype = next->ai_socktype;
			cnext->ai_protocol = next->ai_protocol;
			cnext->ai_addrlen = next->ai_addrlen;

			cnext->ai_addr = static_cast<struct sockaddr*>(malloc(cnext->ai_addrlen));
			memcpy(cnext->ai_addr, next->ai_addr, cnext->ai_addrlen);

			Darling::sockaddrFixupOut(cnext->ai_addr, cnext->ai_addrlen);
			next = next->ai_next;

			if (next)
			{
				cnext->ai_next = new __darwin_addrinfo;
				cnext = cnext->ai_next;
			}
			else
				cnext->ai_next = 0;
		}
		freeaddrinfo(nres);
	}
	else
	{
		errnoOut();
	}
	
	return rv;
}


void __darwin_freeaddrinfo(struct __darwin_addrinfo* res)
{
	while (res)
	{
		struct __darwin_addrinfo* n = res->ai_next;
		free(res->ai_canonname);
		free(res->ai_addr);
		delete res;
		res = n;
	}
}

const char* __darwin_inet_ntop(int af, const void* src, char* dst, socklen_t size)
{
	af = Darling::familyDarwinToLinux(af);
	const char* rv = inet_ntop(af, src, dst, size);

	if (!rv)
		errnoOut();

	return rv;
}


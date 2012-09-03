#ifndef LIBC_NET_H
#define LIBC_NET_H
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define DARWIN_AI_ADDRCONFIG 1024
#define DARWIN_AI_ALL 256
#define DARWIN_AI_CANONNAME 2
#define DARWIN_AI_NUMERICHOST 4
#define DARWIN_AI_NUMERICSERV 4096
#define DARWIN_AI_PASSIVE 1
#define DARWIN_AI_V4MAPPED 2048

struct __darwin_addrinfo
{
	int ai_flags;
	int ai_family;
	int ai_socktype;
	int ai_protocol;
	socklen_t ai_addrlen;
	char *ai_canonname;
	struct sockaddr *ai_addr;
	struct __darwin_addrinfo *ai_next;
};


extern "C"
{

int __darwin_getaddrinfo(const char *hostname, const char *servname, const struct __darwin_addrinfo *hints, struct __darwin_addrinfo **res);
void __darwin_freeaddrinfo(struct __darwin_addrinfo* res);
const char* __darwin_inet_ntop(int af, const void* src, char* dst, socklen_t size);

}

#endif


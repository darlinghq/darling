#include "config.h"
#include <sys/socket.h>
#include "sockets.h"
#include "libc/darwin_errno_codes.h"
#include "libc/errno.h"
#include "common/path.h"
#include "common/auto.h"
#include <sys/types.h>
#include <cstring>
#include <cstdlib>
#include <cstring>
#include <poll.h>

static short familyDarwinToLinux(short f)
{
	switch (f)
	{
		case DARWIN_AF_UNSPEC:
			return AF_UNSPEC;
		case DARWIN_AF_UNIX:
			return AF_UNIX;
		case DARWIN_AF_INET:
			return AF_INET;
		//case DARWIN_AF_APPLETALK:
		//	return AF_APPLETALK;
		case DARWIN_AF_IPX:
			return AF_IPX;
		case DARWIN_AF_INET6:
			return AF_INET6;
		default:
			return -1;
	}
}

static short familyLinuxToDarwin(short f)
{
	switch (f)
	{
		case AF_UNSPEC:
			return DARWIN_AF_UNSPEC;
		case AF_UNIX:
			return DARWIN_AF_UNIX;
		case AF_INET:
			return DARWIN_AF_INET;
		//case AF_APPLETALK:
		//	return DARWIN_AF_APPLETALK;
		case AF_IPX:
			return DARWIN_AF_IPX;
		case AF_INET6:
			return DARWIN_AF_INET6;
		default:
			return -1;
	}
}

static bool sockaddrFixupIn(struct sockaddr* addr, socklen_t len)
{
	if (offsetof(struct sockaddr,sa_family) == 0)
	{
		// BSD/Darwin has offset of 1
		memmove(addr, reinterpret_cast<char*>(addr)+1, len-1);
	}
	addr->sa_family = familyDarwinToLinux(addr->sa_family);
	return addr->sa_family != -1;
}

static bool sockaddrFixupOut(struct sockaddr* addr, socklen_t len)
{
	bool ok;
	addr->sa_family = familyLinuxToDarwin(addr->sa_family);
	ok = addr->sa_family != -1;
	
	if (offsetof(struct sockaddr,sa_family) == 0)
	{
		// BSD/Darwin has offset of 1
		memmove(reinterpret_cast<char*>(addr)+1, addr, len-1);
	}
	return ok;
}

int __darwin_select(int fd, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, struct timeval* timeout)
{
	struct timeval backup;
	memcpy(&backup, timeout, sizeof(backup)); // emulate BSD behavior
	
	int rv = ::select(fd, readfds, writefds, exceptfds, timeout);
	if (rv == -1)
		errnoOut();
	
	memcpy(timeout, &backup, sizeof(backup));
	return rv;
}

MAP_FUNCTION3(int,poll,struct pollfd*,nfds_t,int);

int __darwin_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int rv = ::accept(sockfd, addr, addrlen);
	if (rv == -1)
		errnoOut();
	else
		sockaddrFixupOut(addr, *addrlen);
	return rv;
}

MAP_FUNCTION4(ssize_t,send,int,const void*,size_t,int);
MAP_FUNCTION3(ssize_t,sendmsg,int,struct msghdr*,int);

ssize_t __darwin_sendto(int socket, const void *buffer, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
	struct sockaddr* buf = reinterpret_cast<struct sockaddr*>(alloca(dest_len));
	memcpy(buf, dest_addr, dest_len);
	
	if (!sockaddrFixupIn(buf, dest_len))
	{
		errno = DARWIN_EAFNOSUPPORT;
		return -1;
	}
	
	return AutoErrno<ssize_t>(sendto,socket,buffer,length,flags,buf,dest_len);
}

MAP_FUNCTION4(ssize_t,recv,int,void*,size_t,int);
MAP_FUNCTION3(ssize_t,recvmsg,int,struct msghdr*,int); // TODO: check msghdr

ssize_t __darwin_recvfrom(int socket, void * buffer, size_t length, int flags, struct sockaddr * address, socklen_t * address_len)
{
	ssize_t s = AutoErrno<ssize_t>(recvfrom,socket,buffer,length,flags,address,address_len);
	if (s != -1)
		sockaddrFixupOut(address, *address_len);
	return s;
}

// for SOL_SOCKET
static int MapSockOptname(int opt)
{
	switch (opt)
	{
		case DARWIN_SO_DEBUG:
			return SO_DEBUG;
		case DARWIN_SO_ACCEPTCONN:
			return SO_ACCEPTCONN;
		case DARWIN_SO_REUSEADDR:
			return SO_REUSEADDR;
		case DARWIN_SO_KEEPALIVE:
			return SO_KEEPALIVE;
		case DARWIN_SO_DONTROUTE:
			return SO_DONTROUTE;
		case DARWIN_SO_BROADCAST:
			return SO_BROADCAST;
		case DARWIN_SO_LINGER_SEC:
		case DARWIN_SO_LINGER:
			return SO_LINGER;
		//case DARWIN_SO_REUSEPORT:
		//	return SO_REUSEPORT;
		case DARWIN_SO_TIMESTAMP:
			return SO_TIMESTAMP;
		case DARWIN_SO_OOBINLINE:
			return SO_OOBINLINE;
		case DARWIN_SO_SNDBUF:
			return SO_SNDBUF;
		case DARWIN_SO_RCVBUF:
			return SO_RCVBUF;
		case DARWIN_SO_RCVLOWAT:
			return SO_RCVLOWAT;
		case DARWIN_SO_SNDLOWAT:
			return SO_SNDLOWAT;
		case DARWIN_SO_RCVTIMEO:
			return SO_RCVTIMEO;
		case DARWIN_SO_SNDTIMEO:
			return SO_SNDTIMEO;
		case DARWIN_SO_ERROR:
			return SO_ERROR;
		case DARWIN_SO_TYPE:
			return SO_TYPE;
		default:
			return -1;
	}
}

int __darwin_getsockopt(int socket, int level, int option_name, void * option_value, socklen_t * option_len)
{
	int orig_opt = option_name;
	if (level == SOL_SOCKET)
		option_name = MapSockOptname(option_name);
	
	int rv = getsockopt(socket,level,option_name,option_value,option_len);
	if (rv != -1)
	{
		if (level == SOL_SOCKET && orig_opt == DARWIN_SO_LINGER)
		{
			linger* l = reinterpret_cast<linger*>(option_value);
			l->l_linger *= 100; // convert to ticks, 1 tick = 1/100 s
		}
	}
	else
		errnoOut();
	
	return rv;
}

int __darwin_setsockopt(int socket, int level, int option_name, const void *option_value, socklen_t option_len)
{
	int orig_opt = option_name;
	if (level == SOL_SOCKET)
	{
		option_name = MapSockOptname(option_name);
		if (orig_opt == DARWIN_SO_LINGER && option_len == sizeof(linger))
		{
			linger* l = reinterpret_cast<linger*>(alloca(sizeof(linger)));
			const linger* o = reinterpret_cast<const linger*>(option_value);
			l->l_onoff = o->l_onoff;
			l->l_linger = (o->l_linger+99) / 100; // round up to seconds
			option_value = l;
		}
	}
	
	return AutoErrno<int>(setsockopt,socket,level,option_name,option_value,option_len);
}

int __darwin_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	struct sockaddr* ma = reinterpret_cast<struct sockaddr*>(alloca(sizeof(addrlen)));
	
	memcpy(ma, addr, addrlen);
	ma->sa_family = familyDarwinToLinux(addr->sa_family);
	
	if (ma->sa_family == -1)
	{
		errno = DARWIN_EAFNOSUPPORT;
		return -1;
	}
	else
	{
		int rv = ::connect(sockfd, ma, addrlen);
		if (rv == -1)
			errnoOut();
		return rv;
	}
}

int __darwin_socket(int domain, int type, int protocol)
{
	domain = familyDarwinToLinux(domain);
	if (domain == -1)
	{
		errno = DARWIN_EAFNOSUPPORT;
		return -1;
	}
	else
	{
		int rv = ::socket(domain, type, protocol);
		if (rv == -1)
			errnoOut();
		return rv;
	}
}



#include "config.h"
#include <sys/socket.h>
#include "sockets.h"
#include "libc/errno.h"
#include "common/path.h"
#include "common/auto.h"
#include <sys/types.h>
#include <cstring>
#include <cstdlib>
#include <poll.h>

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
MAP_FUNCTION3(int,accept,int,struct sockaddr*,socklen_t*);
MAP_FUNCTION4(ssize_t,send,int,const void*,size_t,int);
MAP_FUNCTION3(ssize_t,sendmsg,int,struct msghdr*,int);

ssize_t __darwin_sendto(int socket, const void *buffer, size_t length, int flags, const struct sockaddr *dest_addr, socklen_t dest_len)
{
	return AutoErrno<ssize_t>(sendto,socket,buffer,length,flags,dest_addr,dest_len);
}

MAP_FUNCTION4(ssize_t,recv,int,void*,size_t,int);
MAP_FUNCTION3(ssize_t,recvmsg,int,struct msghdr*,int);

ssize_t __darwin_recvfrom(int socket, void * buffer, size_t length, int flags, struct sockaddr * address, socklen_t * address_len)
{
	return AutoErrno<ssize_t>(recvfrom,socket,buffer,length,flags,address,address_len);
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

MAP_FUNCTION3(int,connect,int,const struct sockaddr*,socklen_t);
MAP_FUNCTION3(int,socket,int,int,int);


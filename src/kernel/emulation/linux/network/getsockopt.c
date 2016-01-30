#include "getsockopt.h"
#include "../base.h"
#include "../errno.h"
#include <asm/unistd.h>
#include <stddef.h>
#include "../../../../../platform-include/sys/errno.h"
#include "duct.h"

#define LINGER_TICKS_PER_SEC	100 // Is this the right number of ticks per sec?

long sys_getsockopt(int fd, int level, int optname, void* optval, int* optlen)
{
	int ret, linux_level, linux_optname;
	
	linux_level = level;
	linux_optname = optname;
	
	ret = sockopt_bsd_to_linux(&linux_level, &linux_optname, NULL, NULL);
	
	if (ret != 0)
		return ret;

#ifdef __NR_socketcall
	ret = LINUX_SYSCALL(__NR_socketcall, LINUX_SYS_GETSOCKOPT,
			((long[6]) { fd, linux_level, linux_optname, optval, optlen, 0 }));
#else
	ret = LINUX_SYSCALL(__NR_getsockopt, fd, linux_level, linux_optname,
			optval, optlen);
#endif

	if (ret < 0)
		ret = errno_linux_to_bsd(ret);
	else
	{
		if (level == BSD_SOL_SOCKET)
		{
			if (optname == BSD_SO_LINGER_TICKS)
			{
				struct linger* l = (struct linger*) optval;
				l->l_linger *= LINGER_TICKS_PER_SEC;
			}
		}
	}

	return ret;
}

int sockopt_bsd_to_linux(int* level, int* optname, void** optval, void* optbuf)
{
	if (*level == BSD_SOL_SOCKET)
	{
		*level = LINUX_SOL_SOCKET;
		
		switch (*optname)
		{
			case BSD_SO_DEBUG:
				*optname = LINUX_SO_DEBUG; break;
			case BSD_SO_ACCEPTCONN:
				return -ENOTSUP;
			case BSD_SO_REUSEADDR:
				*optname = LINUX_SO_REUSEADDR; break;
			case BSD_SO_KEEPALIVE:
				*optname = LINUX_SO_KEEPALIVE; break;
			case BSD_SO_DONTROUTE:
				*optname = LINUX_SO_DONTROUTE; break;
			case BSD_SO_BROADCAST:
				*optname = LINUX_SO_BROADCAST; break;
			case BSD_SO_USELOOPBACK:
				return -ENOTSUP;
			case BSD_SO_LINGER_TICKS:
			{
				if (optval != NULL)
				{
					struct linger *out, *in;
					
					in = (struct linger*) *optval;
					out = (struct linger*) optbuf;
					out->l_onoff = in->l_onoff;
					out->l_linger = (in->l_linger+LINGER_TICKS_PER_SEC-1)/LINGER_TICKS_PER_SEC;
					*optval = out;
				}
				*optname = LINUX_SO_LINGER;
				break;
			}
			case BSD_SO_OOBINLINE:
				*optname = LINUX_SO_OOBINLINE; break;
			case BSD_SO_REUSEPORT:
				*optname = LINUX_SO_REUSEPORT; break;
			case BSD_SO_SNDBUF:
				*optname = LINUX_SO_SNDBUF; break;
			case BSD_SO_RCVBUF:
				*optname = LINUX_SO_RCVBUF; break;
			case BSD_SO_SNDLOWAT:
				*optname = LINUX_SO_SNDLOWAT; break;
			case BSD_SO_RCVLOWAT:
				*optname = LINUX_SO_RCVLOWAT; break;
			case BSD_SO_SNDTIMEO:
				*optname = LINUX_SO_SNDTIMEO; break;
			case BSD_SO_RCVTIMEO:
				*optname = LINUX_SO_RCVTIMEO; break;
			case BSD_SO_ERROR:
				*optname = LINUX_SO_ERROR; break;
			case BSD_SO_TYPE:
				*optname = LINUX_SO_TYPE; break;
			case BSD_SO_LINGER_SEC:
				*optname = LINUX_SO_LINGER; break;
			default:
				return -EINVAL;
		}
	}
	else if (*level == IPPROTO_TCP)
	{
		switch (*optname)
		{
			case BSD_TCP_NODELAY:
			case BSD_TCP_MAXSEG:
				break;
			case BSD_TCP_KEEPALIVE:
				*optname = LINUX_TCP_KEEPIDLE; break;
			case BSD_TCP_KEEPINTVL:
				*optname = LINUX_TCP_KEEPINTVL; break;
			case BSD_TCP_KEEPCNT:
				*optname = LINUX_TCP_KEEPCNT; break;
			default:
				return -ENOTSUP;
		}
	}
	else if (*level == IPPROTO_IP)
	{
		switch (*optname)
		{
			case BSD_IP_OPTIONS:
				*optname = LINUX_IP_OPTIONS; break;
			case BSD_IP_HDRINCL:
				*optname = LINUX_IP_HDRINCL; break;
			case BSD_IP_TOS:
				*optname = LINUX_IP_TOS; break;
			case BSD_IP_TTL:
				*optname = LINUX_IP_TTL; break;
			case BSD_IP_RECVOPTS:
			case BSD_IP_RECVRETOPTS:
				*optname = LINUX_IP_RECVOPTS; break;
			case BSD_IP_RETOPTS:
				*optname = LINUX_IP_RETOPTS; break;
			case BSD_IP_MULTICAST_IF:
				*optname = LINUX_IP_MULTICAST_IF; break;
			case BSD_IP_MULTICAST_TTL:
				*optname = LINUX_IP_MULTICAST_TTL; break;
			case BSD_IP_MULTICAST_LOOP:
				*optname = LINUX_IP_MULTICAST_LOOP; break;
			case BSD_IP_ADD_MEMBERSHIP:
				*optname = LINUX_IP_ADD_MEMBERSHIP; break;
			case BSD_IP_DROP_MEMBERSHIP:
				*optname = LINUX_IP_DROP_MEMBERSHIP; break;
			case BSD_IP_RECVTTL:
				*optname = LINUX_IP_RECVTTL; break;
			default:
				return -ENOTSUP;
		}
	}
	else if (*level == IPPROTO_IPV6)
	{
		switch (*optname)
		{
			case BSD_IPV6_UNICAST_HOPS:
				*optname = LINUX_IPV6_UNICAST_HOPS; break;
			case BSD_IPV6_MULTICAST_IF:
				*optname = LINUX_IPV6_MULTICAST_IF; break;
			case BSD_IPV6_MULTICAST_HOPS:
				*optname = LINUX_IPV6_MULTICAST_HOPS; break;
			case BSD_IPV6_MULTICAST_LOOP:
				*optname = LINUX_IPV6_MULTICAST_LOOP; break;
			case BSD_IPV6_JOIN_GROUP:
				*optname = LINUX_IPV6_JOIN_GROUP; break;
			case BSD_IPV6_LEAVE_GROUP:
				*optname = LINUX_IPV6_LEAVE_GROUP; break;
			case BSD_IPV6_2292PKTINFO:
				*optname = LINUX_IPV6_2292PKTINFO; break;
			case BSD_IPV6_2292HOPLIMIT:
				*optname = LINUX_IPV6_2292HOPLIMIT; break;
			case BSD_IPV6_2292NEXTHOP:
				*optname = LINUX_IPV6_NEXTHOP; break;
			case BSD_IPV6_2292HOPOPTS:
				*optname = LINUX_IPV6_2292HOPOPTS; break;
			case BSD_IPV6_2292DSTOPTS:
				*optname = LINUX_IPV6_2292DSTOPTS; break;
			case BSD_IPV6_2292RTHDR:
				*optname = LINUX_IPV6_2292RTHDR; break;
			default:
				return -ENOTSUP;
		}
	}
	
	return 0;
}

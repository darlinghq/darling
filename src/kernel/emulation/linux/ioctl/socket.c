#include "socket.h"
#include <sys/errno.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/sockio.h>
#include <netinet6/in6_var.h>

int handle_socket(int fd, unsigned int cmd, void* arg, int* retval)
{
	switch (cmd)
	{
		// TODO: This has to be implemented for container with separate networking
		case SIOCGIFFLAGS:
		case SIOCSIFFLAGS:
		case SIOCAIFADDR: // set IPv4 address
		case SIOCAIFADDR_IN6: // set IPv6 address
			*retval = -ENOTSUP;
			return IOCTL_HANDLED;
	}
	return IOCTL_PASS;
}



#include "duct.h"
#include "../../../../../platform-include/sys/socket.h"

int sfamily_linux_to_bsd(int family)
{
	switch (family)
	{
		case LINUX_PF_LOCAL:
			return PF_LOCAL;
		case LINUX_PF_INET:
			return PF_INET;
		case LINUX_PF_IPX:
			return PF_IPX;
		case LINUX_PF_INET6:
			return PF_INET6;
		default:
			return family;
	}
}

int sfamily_bsd_to_linux(int family)
{
	switch (family)
	{
		case PF_LOCAL:
			return LINUX_PF_LOCAL;
		case PF_INET:
			return LINUX_PF_INET;
		case PF_IPX:
			return LINUX_PF_IPX;
		case PF_INET6:
			return LINUX_PF_INET6;
		default:
			return family;
	}
}

int msgflags_bsd_to_linux(int flags)
{
	int oflags = 0;

	if (flags & BSD_MSG_OOB)
		oflags |= LINUX_MSG_OOB;
	if (flags & BSD_MSG_PEEK)
		oflags |= LINUX_MSG_PEEK;
	if (flags & BSD_MSG_DONTROUTE)
		oflags |= LINUX_MSG_DONTROUTE;
	if (flags & BSD_MSG_EOR)
		oflags |= LINUX_MSG_EOR;
	if (flags & BSD_MSG_TRUNC)
		oflags |= LINUX_MSG_TRUNC;
	if (flags & BSD_MSG_CTRUNC)
		oflags |= LINUX_MSG_CTRUNC;
	if (flags & BSD_MSG_WAITALL)
		oflags |= LINUX_MSG_WAITALL;
	if (flags & BSD_MSG_DONTWAIT)
		oflags |= LINUX_MSG_DONTWAIT;

	return oflags;
}

int msgflags_linux_to_bsd(int flags)
{
	int oflags = 0;

	if (flags & LINUX_MSG_OOB)
		oflags |= BSD_MSG_OOB;
	if (flags & LINUX_MSG_PEEK)
		oflags |= BSD_MSG_PEEK;
	if (flags & LINUX_MSG_DONTROUTE)
		oflags |= BSD_MSG_DONTROUTE;
	if (flags & LINUX_MSG_EOR)
		oflags |= BSD_MSG_EOR;
	if (flags & LINUX_MSG_TRUNC)
		oflags |= BSD_MSG_TRUNC;
	if (flags & LINUX_MSG_CTRUNC)
		oflags |= BSD_MSG_CTRUNC;
	if (flags & LINUX_MSG_WAITALL)
		oflags |= BSD_MSG_WAITALL;
	if (flags & LINUX_MSG_DONTWAIT)
		oflags |= BSD_MSG_DONTWAIT;

	return oflags;
}

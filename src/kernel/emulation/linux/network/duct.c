#include "duct.h"
#include "../errno.h"
#include "../vchroot_expand.h"
#include "../bsdthread/per_thread_wd.h"
#include <sys/socket.h>

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

unsigned long sockaddr_fixup_size_from_bsd(const void* bsd_sockaddr, int bsd_sockaddr_len) {
	return (((struct sockaddr_fixup*)bsd_sockaddr)->bsd_family == PF_LOCAL) ? sizeof(struct sockaddr_fixup) : bsd_sockaddr_len;
};

int sockaddr_fixup_from_bsd(struct sockaddr_fixup* out, const void* bsd_sockaddr, int bsd_sockaddr_len) {
	int ret = bsd_sockaddr_len;

	memcpy(out, bsd_sockaddr, bsd_sockaddr_len);

	out->linux_family = sfamily_bsd_to_linux(out->bsd_family);

	if (out->linux_family == LINUX_PF_LOCAL) {
		struct vchroot_expand_args vc;
		vc.flags = VCHROOT_FOLLOW;
		vc.dfd = get_perthread_wd();

		strcpy(vc.path, out->sun_path);

		ret = vchroot_expand(&vc);
		if (ret < 0)
			return errno_linux_to_bsd(ret);

		strncpy(out->sun_path, vc.path, sizeof(out->sun_path) - 1);
		out->sun_path[sizeof(out->sun_path) - 1] = '\0';
		ret = sizeof(*out) - sizeof(out->sun_path) + strlen(out->sun_path);
	}

	return ret;
};

int sockaddr_fixup_from_linux(struct sockaddr_fixup* out, const void* linux_sockaddr, int linux_sockaddr_len) {
	int ret = linux_sockaddr_len;

	memcpy(out, linux_sockaddr, linux_sockaddr_len);

	out->bsd_family = sfamily_linux_to_bsd(out->linux_family);

	if (out->bsd_family == PF_LOCAL) {
		struct vchroot_unexpand_args vc;
		strcpy(vc.path, out->sun_path);

		ret = vchroot_unexpand(&vc);
		if (ret < 0)
			return errno_linux_to_bsd(ret);

		strncpy(out->sun_path, vc.path, sizeof(out->sun_path) - 1);
		out->sun_path[sizeof(out->sun_path) - 1] = '\0';
		ret = sizeof(*out) - sizeof(out->sun_path) + strlen(out->sun_path) + 1;
	}

	if (ret >= 0) {
		out->bsd_length = ret;
	}

	return ret;
};

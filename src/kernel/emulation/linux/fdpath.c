#include "fdpath.h"
#include "mach/lkm.h"
#include <lkm/api.h>

int fdpath(int fd, char* buf, size_t bufsiz)
{
	struct vchroot_fdpath_args args = {
		.fd = fd,
		.path = buf,
		.maxlen = bufsiz
	};

	return lkm_call(NR_vchroot_fdpath, &args);
}


#include "vchroot_expand.h"
#include <mach/lkm.h>

int vchroot_expand(struct vchroot_expand_args* args)
{
	if (strncmp(args->path, "/Volumes/SystemRoot", 19) == 0)
	{
		memmove(args->path, args->path + 19, strlen(args->path + 19) + 1);
		if (args->path[0] == 0)
		{
			args->path[0] = '/';
			args->path[1] = '\0';
		}

		return 0;
	}

	return lkm_call(NR_vchroot_expand, args);
}

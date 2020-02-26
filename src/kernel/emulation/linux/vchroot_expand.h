#ifndef _VCHROOT_EXPAND_H
#define _VCHROOT_EXPAND_H
#include <lkm/api.h>

int vchroot_expand(struct vchroot_expand_args* args);

struct vchroot_unexpand_args
{
	char path[4096];
};
int vchroot_unexpand(struct vchroot_unexpand_args* args);

#endif


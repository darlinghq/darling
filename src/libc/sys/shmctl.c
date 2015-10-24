/*
 * Copyright (c) 2004 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
#include <unistd.h>
#include <sys/shm.h>

/*
 * Stub function to account for the differences in the ipc_perm structure,
 * while maintaining binary backward compatibility.
 *
 * This is only the legacy behavior.
 */
extern int __shmctl(int, int, void *);

int
shmctl(int shmid, int cmd, struct shmid_ds *ds)
{
	struct __shmid_ds_old	*ds_old = (struct __shmid_ds_old *)ds;
	struct __shmid_ds_new	ds2;
	struct __shmid_ds_new	*ds_new = &ds2;
	int			rv;

#define	_UP_CVT(x)	ds_new-> x = ds_old-> x
#define	_DN_CVT(x)	ds_old-> x = ds_new-> x

	if (cmd == IPC_SET) {
		/* convert before call */
		_UP_CVT(shm_perm.uid);
		_UP_CVT(shm_perm.gid);
		_UP_CVT(shm_perm.cuid);
		_UP_CVT(shm_perm.cgid);
		_UP_CVT(shm_perm.mode);
		ds_new->shm_perm._seq = ds_old->shm_perm.seq;
		ds_new->shm_perm._key = ds_old->shm_perm.key;
		_UP_CVT(shm_segsz);
		_UP_CVT(shm_lpid);
		_UP_CVT(shm_cpid);
		_UP_CVT(shm_nattch);
		_UP_CVT(shm_atime);
		_UP_CVT(shm_dtime);
		_UP_CVT(shm_ctime);
		_UP_CVT(shm_internal);
	}

	rv = __shmctl(shmid, cmd, (void *)ds_new);

	if (cmd == IPC_STAT) {
		/* convert after call */
		_DN_CVT(shm_perm.uid);	/* warning!  precision loss! */
		_DN_CVT(shm_perm.gid);	/* warning!  precision loss! */
		_DN_CVT(shm_perm.cuid);	/* warning!  precision loss! */
		_DN_CVT(shm_perm.cgid);	/* warning!  precision loss! */
		_DN_CVT(shm_perm.mode);
		ds_old->shm_perm.seq = ds_new->shm_perm._seq;
		ds_old->shm_perm.key = ds_new->shm_perm._key;
		_DN_CVT(shm_segsz);
		_DN_CVT(shm_lpid);
		_DN_CVT(shm_cpid);
		_DN_CVT(shm_nattch);
		_DN_CVT(shm_atime);
		_DN_CVT(shm_dtime);
		_DN_CVT(shm_ctime);
		_DN_CVT(shm_internal);
	}

	return (rv);
}

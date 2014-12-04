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
#include <stdarg.h>
#include <sys/msg.h>

/*
 * Stub function to account for the differences in the ipc_perm structure,
 * while maintaining binary backward compatibility.
 *
 * This is only the legacy behavior.
 */
extern int __msgctl(int, int, struct msqid_ds *);

int
msgctl(int msqid, int cmd, struct msqid_ds *ds)
{
	struct __msqid_ds_old	*ds_old = (struct __msqid_ds_old *)ds;
	struct __msqid_ds_new	ds2;
	struct __msqid_ds_new	*ds_new = &ds2;
	int			rv;

#define	_UP_CVT(x)	ds_new-> x = ds_old-> x
#define	_DN_CVT(x)	ds_old-> x = ds_new-> x

	if (cmd == IPC_SET) {
		/* convert before call */
		_UP_CVT(msg_perm.uid);
		_UP_CVT(msg_perm.gid);
		_UP_CVT(msg_perm.cuid);
		_UP_CVT(msg_perm.cgid);
		_UP_CVT(msg_perm.mode);
		ds_new->msg_perm._seq = ds_old->msg_perm.seq;
		ds_new->msg_perm._key = ds_old->msg_perm.key;
		_UP_CVT(msg_first);
		_UP_CVT(msg_last);
		_UP_CVT(msg_cbytes);
		_UP_CVT(msg_qnum);
		_UP_CVT(msg_qbytes);
		_UP_CVT(msg_lspid);
		_UP_CVT(msg_lrpid);
		_UP_CVT(msg_stime);
		_UP_CVT(msg_pad1);	/* binary compatibility */
		_UP_CVT(msg_rtime);
		_UP_CVT(msg_pad2);	/* binary compatibility */
		_UP_CVT(msg_ctime);
		_UP_CVT(msg_pad3);	/* binary compatibility */
		_UP_CVT(msg_pad4[0]);	/* binary compatibility */
		_UP_CVT(msg_pad4[1]);	/* binary compatibility */
		_UP_CVT(msg_pad4[2]);	/* binary compatibility */
		_UP_CVT(msg_pad4[3]);	/* binary compatibility */
	}

	rv = __msgctl(msqid, cmd, ds_new);

	if (cmd == IPC_STAT) {
		/* convert after call */
		_DN_CVT(msg_perm.uid);	/* warning!  precision loss! */
		_DN_CVT(msg_perm.gid);	/* warning!  precision loss! */
		_DN_CVT(msg_perm.cuid);	/* warning!  precision loss! */
		_DN_CVT(msg_perm.cgid);	/* warning!  precision loss! */
		_DN_CVT(msg_perm.mode);
		ds_old->msg_perm.seq = ds_new->msg_perm._seq;
		ds_old->msg_perm.key = ds_new->msg_perm._key;
		_DN_CVT(msg_first);
		_DN_CVT(msg_last);
		_DN_CVT(msg_cbytes);
		_DN_CVT(msg_qnum);
		_DN_CVT(msg_qbytes);
		_DN_CVT(msg_lspid);
		_DN_CVT(msg_lrpid);
		_DN_CVT(msg_stime);
		_DN_CVT(msg_pad1);	/* binary compatibility */
		_DN_CVT(msg_rtime);
		_DN_CVT(msg_pad2);	/* binary compatibility */
		_DN_CVT(msg_ctime);
		_DN_CVT(msg_pad3);	/* binary compatibility */
		_DN_CVT(msg_pad4[0]);	/* binary compatibility */
		_DN_CVT(msg_pad4[1]);	/* binary compatibility */
		_DN_CVT(msg_pad4[2]);	/* binary compatibility */
		_DN_CVT(msg_pad4[3]);	/* binary compatibility */
	}

	return (rv);
}

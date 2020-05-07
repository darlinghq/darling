/*
 * Copyright (c) 2013 Apple Inc. All rights reserved.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. The rights granted to you under the License
 * may not be used to create, or enable the creation or redistribution of,
 * unlawful or unlicensed copies of an Apple operating system, or to
 * circumvent, violate, or enable the circumvention or violation of, any
 * terms of an Apple operating system software license agreement.
 *
 * Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_OSREFERENCE_LICENSE_HEADER_END@
 */

#ifndef __TTY_DEV_H__
#define __TTY_DEV_H__

/*
 * ptmx_ioctl is a pointer to a list of pointers to tty structures which is
 * grown, as necessary, copied, and replaced, but never shrunk.  The ioctl
 * structures themselves pointed to from this list come and go as needed.
 */
struct ptmx_ioctl {
	struct tty      *pt_tty;        /* pointer to ttymalloc()'ed data */
	int             pt_flags;
	struct selinfo  pt_selr;
	struct selinfo  pt_selw;
	u_char          pt_send;
	u_char          pt_ucntl;
	void            *pt_devhandle;  /* cloned slave device handle */
};

#define PF_PKT          0x0008          /* packet mode */
#define PF_STOPPED      0x0010          /* user told stopped */
#define PF_REMOTE       0x0020          /* remote and flow controlled input */
#define PF_NOSTOP       0x0040
#define PF_UCNTL        0x0080          /* user control mode */
#define PF_UNLOCKED     0x0100          /* slave unlock (master open resets) */
#define PF_OPEN_M       0x0200          /* master is open */
#define PF_OPEN_S       0x0400          /* slave is open */

struct tty_dev_t {
	int master;     // master major device number
	int slave;      // slave major device number
	unsigned int    fix_7828447:1,
	    fix_7070978:1,
	    mac_notify:1,
	    open_reset:1,
	    _reserved:28;
#if __LP64__
	int _pad;
#endif

	struct tty_dev_t *next;

	struct ptmx_ioctl *(*open)(int minor, int flags);
	int (*free)(int minor, int flags);
	int (*name)(int minor, char *buffer, size_t size);
	void (*revoke)(int minor, struct tty *tp);
};

extern void tty_dev_register(struct tty_dev_t *dev);

extern int ttnread(struct tty *tp);

#endif // __TTY_DEV_H__

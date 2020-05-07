/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
/*
 * Copyright (c) 1987 NeXT, Inc.
 */

struct consdev {
	char    *cn_name;       /* name of device in dev_name_list */
	int     (*cn_probe)(void);      /* probe and fill in consdev info */
	int     (*cn_init)(void);       /* turn on as console */
	int     (*cn_getc)(void);       /* kernel getchar interface */
	int     (*cn_putc)(void);       /* kernel putchar interface */
	struct  tty *cn_tp;     /* tty structure for console device */
	dev_t   cn_dev;         /* major/minor of device */
	short   cn_pri;         /* pecking order; the higher the better */
};

/* values for cn_pri - reflect our policy for console selection */
#define CN_DEAD         0       /* device doesn't exist */
#define CN_NORMAL       1       /* device exists but is nothing special */
#define CN_INTERNAL     2       /* "internal" bit-mapped display */
#define CN_REMOTE       3       /* serial interface with remote bit set */

/* XXX */
#define CONSMAJOR       0

#ifdef KERNEL

#include <sys/types.h>
#include <sys/conf.h>

extern  struct consdev constab[];
extern  struct consdev *cn_tab;
extern  struct tty *cn_tty;

extern struct tty       *constty;               /* current console device */

int consopen(dev_t, int, int, struct proc *);
int consclose(dev_t, int, int, struct proc *);
int consread(dev_t, struct uio *, int);
int conswrite(dev_t, struct uio *, int);
int consioctl(dev_t, u_long, caddr_t, int, struct proc *);
int consselect(dev_t, int, void *, struct proc *);

/*
 * These really want their own header file, but this is the only one in
 * common, and the km device is the keyboard monitor, so it's technically a
 * part of the console.
 */
int kmopen(dev_t, int, int, struct proc *);
int kmclose(dev_t, int, int, struct proc *);
int kmread(dev_t, struct uio *, int);
int kmwrite(dev_t, struct uio *, int);
int kmioctl(dev_t, u_long, caddr_t, int, struct proc *);
int kmputc(dev_t, char);

#endif

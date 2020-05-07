/*
 * Copyright (c) 2000-2012 Apple Computer, Inc. All rights reserved.
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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)conf.h	8.5 (Berkeley) 1/9/95
 */

#ifndef _SYS_CONF_H_
#define _SYS_CONF_H_ 1

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#include <sys/queue.h>
#include <stdint.h>

/*
 * Definitions of device driver entry switches
 */

struct buf;
struct proc;
struct tty;
struct uio;
struct vnode;

/*
 * Types for d_type.
 * These are returned by ioctl FIODTYPE
 */
#define D_TAPE  1
#define D_DISK  2
#define D_TTY   3

#ifdef KERNEL
/*
 * Device switch function types.
 */
typedef int  open_close_fcn_t(dev_t dev, int flags, int devtype,
    struct proc *p);

typedef struct tty *d_devtotty_t(dev_t dev);

typedef void strategy_fcn_t(struct buf *bp);
typedef int  ioctl_fcn_t(dev_t dev, u_long cmd, caddr_t data,
    int fflag, struct proc *p);
typedef int  dump_fcn_t(void);     /* parameters vary by architecture */
typedef int  psize_fcn_t(dev_t dev);
typedef int  read_write_fcn_t(dev_t dev, struct uio *uio, int ioflag);
typedef int  stop_fcn_t(struct tty *tp, int rw);
typedef int  reset_fcn_t(int uban);
typedef int  select_fcn_t(dev_t dev, int which, void * wql, struct proc *p);
typedef int  mmap_fcn_t(void);

#define d_open_t        open_close_fcn_t
#define d_close_t       open_close_fcn_t
#define d_read_t        read_write_fcn_t
#define d_write_t       read_write_fcn_t
#define d_ioctl_t       ioctl_fcn_t
#define d_stop_t        stop_fcn_t
#define d_reset_t       reset_fcn_t
#define d_select_t      select_fcn_t
#define d_mmap_t        mmap_fcn_t
#define d_strategy_t    strategy_fcn_t

__BEGIN_DECLS
int             enodev(void);
void    enodev_strat(void);
__END_DECLS

/*
 * Versions of enodev() pointer, cast to appropriate function type. For use
 * in empty devsw slots.
 */
#define eno_opcl                ((open_close_fcn_t *)&enodev)
#define eno_strat               ((strategy_fcn_t *)&enodev_strat)
#define eno_ioctl               ((ioctl_fcn_t *)&enodev)
#define eno_dump                ((dump_fcn_t *)&enodev)
#define eno_psize               ((psize_fcn_t *)&enodev)
#define eno_rdwrt               ((read_write_fcn_t *)&enodev)
#define eno_stop                ((stop_fcn_t *)&enodev)
#define eno_reset               ((reset_fcn_t *)&enodev)
#define eno_mmap                ((mmap_fcn_t *)&enodev)
#define eno_select              ((select_fcn_t *)&enodev)

/* For source backward compatibility only! */
#define eno_getc                ((void *)&enodev)
#define eno_putc                ((void *)&enodev)

/*
 * Block device switch table
 */
struct bdevsw {
	open_close_fcn_t        *d_open;
	open_close_fcn_t        *d_close;
	strategy_fcn_t          *d_strategy;
	ioctl_fcn_t             *d_ioctl;
	dump_fcn_t              *d_dump;
	psize_fcn_t             *d_psize;
	int                     d_type;
};


d_devtotty_t    nodevtotty;
d_write_t       nowrite;

#ifdef KERNEL_PRIVATE
extern struct bdevsw bdevsw[];
extern int (*bootcache_contains_block)(dev_t device, u_int64_t blkno);
#endif /* KERNEL_PRIVATE */

/*
 * Contents of empty bdevsw slot.
 */
#define  NO_BDEVICE                                             \
	{ eno_opcl,	eno_opcl,	eno_strat, eno_ioctl,   \
	  eno_dump,	eno_psize,	0       }


/*
 * Character device switch table
 */
struct cdevsw {
	open_close_fcn_t        *d_open;
	open_close_fcn_t        *d_close;
	read_write_fcn_t        *d_read;
	read_write_fcn_t        *d_write;
	ioctl_fcn_t             *d_ioctl;
	stop_fcn_t              *d_stop;
	reset_fcn_t             *d_reset;
	struct  tty             **d_ttys;
	select_fcn_t            *d_select;
	mmap_fcn_t              *d_mmap;
	strategy_fcn_t          *d_strategy;
	void                    *d_reserved_1;
	void                    *d_reserved_2;
	int                     d_type;
};

#ifdef BSD_KERNEL_PRIVATE

extern uint64_t cdevsw_flags[];
#define CDEVSW_SELECT_KQUEUE 0x01
#define CDEVSW_USE_OFFSET    0x02
#define CDEVSW_IS_PTC        0x04
#define CDEVSW_IS_PTS        0x08

struct thread;

typedef struct devsw_lock {
	TAILQ_ENTRY(devsw_lock)         dl_list;
	struct thread                   *dl_thread;
	dev_t                           dl_dev;
	int                             dl_mode;
} *devsw_lock_t;

#endif /* BSD_KERNEL_PRIVATE */


/*
 * Contents of empty cdevsw slot.
 */

#define  NO_CDEVICE                                                      \
    {                                                                   \
	eno_opcl,	eno_opcl,	eno_rdwrt,	eno_rdwrt,      \
	eno_ioctl,	eno_stop,	eno_reset,	0,              \
	(select_fcn_t *)seltrue,	eno_mmap,	eno_strat,	eno_getc,       \
	eno_putc,	0                                               \
    }

#endif /* KERNEL */

#ifdef  KERNEL_PRIVATE
typedef int  l_open_t (dev_t dev, struct tty *tp);
typedef int  l_close_t(struct tty *tp, int flags);
typedef int  l_read_t (struct tty *tp, struct uio *uio, int flag);
typedef int  l_write_t(struct tty *tp, struct uio *uio, int flag);
typedef int  l_ioctl_t(struct tty *tp, u_long cmd, caddr_t data, int flag,
    struct proc *p);
typedef int  l_rint_t (int c, struct tty *tp);
typedef void l_start_t(struct tty *tp);
typedef int  l_modem_t(struct tty *tp, int flag);

/*
 * Line discipline switch table
 */
struct linesw {
	l_open_t        *l_open;
	l_close_t       *l_close;
	l_read_t        *l_read;
	l_write_t       *l_write;
	l_ioctl_t       *l_ioctl;
	l_rint_t        *l_rint;
	l_start_t       *l_start;
	l_modem_t       *l_modem;
};


extern struct linesw linesw[];
extern const int nlinesw;

int ldisc_register(int, struct linesw *);
void ldisc_deregister(int);
#define LDISC_LOAD      -1              /* Loadable line discipline */

#endif /* KERNEL_PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
/*
 * Swap device table
 */
struct swdevt {
	dev_t   sw_dev;
	int     sw_flags;
	int     sw_nblks;
	struct  vnode *sw_vp;
};
#define SW_FREED        0x01
#define SW_SEQUENTIAL   0x02
#define sw_freed        sw_flags        /* XXX compat */

extern struct swdevt swdevt[];

#endif /* BSD_KERNEL_PRIVATE */


#ifdef KERNEL
/*
 * ***_free finds free slot;
 * ***_add adds entries to the devsw table
 * If int arg is -1; finds a free slot
 * Returns the major number if successful
 *  else -1
 */
__BEGIN_DECLS
#ifdef KERNEL_PRIVATE
void devsw_init(void);
extern struct cdevsw cdevsw[];
extern int cdevsw_setkqueueok(int, struct cdevsw*, int);
#endif /* KERNEL_PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
extern void devsw_lock(dev_t, int);
extern void devsw_unlock(dev_t, int);
#endif /* BSD_KERNEL_PRIVATE */

int  bdevsw_isfree(int);
int  bdevsw_add(int, struct bdevsw *);
int  bdevsw_remove(int, struct bdevsw *);
int  cdevsw_isfree(int);
int  cdevsw_add(int, struct cdevsw *);
int  cdevsw_add_with_bdev(int index, struct cdevsw * csw, int bdev);
int  cdevsw_remove(int, struct cdevsw *);
int  isdisk(dev_t, int);
__END_DECLS
#endif /* KERNEL */

#endif /* _SYS_CONF_H_ */

/*-
 * Copyright (c) 2009 Mark Heily <mark@heily.com>
 * Copyright (c) 1999,2000,2001 Jonathan Lemon <jlemon@FreeBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD SVN Revision 197533$
 */

#ifndef _SYS_EVENT_H_
#define _SYS_EVENT_H_

#include <sys/types.h>

#ifdef __KERNEL__
#define intptr_t long
#else
#include <sys/types.h> 
#if defined(_WIN32) && _MSC_VER < 1600 && !defined(__MINGW32__)
# include "../../src/windows/stdint.h"
#else
# include <stdint.h>
#endif
#define LIBKQUEUE       1
#endif

struct timespec;

#define EVFILT_READ		(-1)
#define EVFILT_WRITE		(-2)
#define EVFILT_AIO		(-3)	/* attached to aio requests */
#define EVFILT_VNODE		(-4)	/* attached to vnodes */
#define EVFILT_PROC		(-5)	/* attached to struct proc */
#define EVFILT_SIGNAL		(-6)	/* attached to struct proc */
#define EVFILT_TIMER		(-7)	/* timers */
#define EVFILT_NETDEV		(-8)	/* network devices */
#define EVFILT_FS		(-9)	/* filesystem events */
#define EVFILT_LIO		(-10)	/* attached to lio requests */
#define EVFILT_USER		(-11)	/* User events */
#define EVFILT_SYSCOUNT		11

#define EV_SET(kevp_, a, b, c, d, e, f) do {	\
	struct kevent *kevp = (kevp_);		\
	(kevp)->ident = (a);			\
	(kevp)->filter = (b);			\
	(kevp)->flags = (c);			\
	(kevp)->fflags = (d);			\
	(kevp)->data = (e);			\
	(kevp)->udata = (f);			\
} while(0)

struct kevent {
	uintptr_t	ident;		/* identifier for this event */
	short		filter;		/* filter for event */
	unsigned short flags;
	unsigned int fflags;
	intptr_t	data;
	void		*udata;		/* opaque user data identifier */
};

/* actions */
#define EV_ADD		0x0001		/* add event to kq (implies enable) */
#define EV_DELETE	0x0002		/* delete event from kq */
#define EV_ENABLE	0x0004		/* enable event */
#define EV_DISABLE	0x0008		/* disable event (not reported) */

/* flags */
#define EV_ONESHOT	0x0010		/* only report one occurrence */
#define EV_CLEAR	0x0020		/* clear event state after reporting */
#define EV_RECEIPT	0x0040		/* force EV_ERROR on success, data=0 */
#define EV_DISPATCH	0x0080		/* disable event after reporting */

#define EV_SYSFLAGS	0xF000		/* reserved by system */
#define EV_FLAG1	0x2000		/* filter-specific flag */

/* returned values */
#define EV_EOF		0x8000		/* EOF detected */
#define EV_ERROR	0x4000		/* error, data contains errno */

 /*
  * data/hint flags/masks for EVFILT_USER
  *
  * On input, the top two bits of fflags specifies how the lower twenty four
  * bits should be applied to the stored value of fflags.
  *
  * On output, the top two bits will always be set to NOTE_FFNOP and the
  * remaining twenty four bits will contain the stored fflags value.
  */
#define NOTE_FFNOP	0x00000000		/* ignore input fflags */
#define NOTE_FFAND	0x40000000		/* AND fflags */
#define NOTE_FFOR	0x80000000		/* OR fflags */
#define NOTE_FFCOPY	0xc0000000		/* copy fflags */
#define NOTE_FFCTRLMASK	0xc0000000		/* masks for operations */
#define NOTE_FFLAGSMASK	0x00ffffff

#define NOTE_TRIGGER	0x01000000		/* Cause the event to be
						   triggered for output. */

/*
 * data/hint flags for EVFILT_{READ|WRITE}
 */
#define NOTE_LOWAT	0x0001			/* low water mark */
#undef  NOTE_LOWAT                  /* Not supported on Linux */

/*
 * data/hint flags for EVFILT_VNODE
 */
#define	NOTE_DELETE	0x0001			/* vnode was removed */
#define	NOTE_WRITE	0x0002			/* data contents changed */
#define	NOTE_EXTEND	0x0004			/* size increased */
#define	NOTE_ATTRIB	0x0008			/* attributes changed */
#define	NOTE_LINK	0x0010			/* link count changed */
#define	NOTE_RENAME	0x0020			/* vnode was renamed */
#define	NOTE_REVOKE	0x0040			/* vnode access was revoked */
#undef  NOTE_REVOKE                 /* Not supported on Linux */

/*
 * data/hint flags for EVFILT_PROC
 */
#define	NOTE_EXIT	0x80000000		/* process exited */
#define	NOTE_FORK	0x40000000		/* process forked */
#define	NOTE_EXEC	0x20000000		/* process exec'd */
#define	NOTE_PCTRLMASK	0xf0000000		/* mask for hint bits */
#define	NOTE_PDATAMASK	0x000fffff		/* mask for pid */

/* additional flags for EVFILT_PROC */
#define	NOTE_TRACK	0x00000001		/* follow across forks */
#define	NOTE_TRACKERR	0x00000002		/* could not track child */
#define	NOTE_CHILD	0x00000004		/* am a child process */

/*
 * data/hint flags for EVFILT_NETDEV
 */
#define NOTE_LINKUP	0x0001			/* link is up */
#define NOTE_LINKDOWN	0x0002			/* link is down */
#define NOTE_LINKINV	0x0004			/* link state is invalid */

/* KLUDGE: This is from <sys/mount.h> on FreeBSD and is used by 
           the EVFILT_FS filter. */
/* vfsquery flags */
#define VQ_NOTRESP      0x0001  /* server down */
#define VQ_NEEDAUTH     0x0002  /* server bad auth */
#define VQ_LOWDISK      0x0004  /* we're low on space */
#define VQ_MOUNT        0x0008  /* new filesystem arrived */
#define VQ_UNMOUNT      0x0010  /* filesystem has left */
#define VQ_DEAD         0x0020  /* filesystem is dead, needs force unmount */
#define VQ_ASSIST       0x0040  /* filesystem needs assistance from external
                                   program */
#define VQ_NOTRESPLOCK  0x0080  /* server lockd down */


#ifndef __KERNEL__
#ifdef  __cplusplus
extern "C" {
#endif

#ifdef _WIN32

struct timespec {
    time_t  tv_sec;
    long    tv_nsec;
};

__declspec(dllexport) int
kqueue(void);

__declspec(dllexport) int
kevent(int kq, const struct kevent *changelist, int nchanges,
	    struct kevent *eventlist, int nevents,
	    const struct timespec *timeout);

#ifdef MAKE_STATIC
__declspec(dllexport) int
libkqueue_init();
#endif

#else
int     kqueue(void);
int     kevent(int kq, const struct kevent *changelist, int nchanges,
	    struct kevent *eventlist, int nevents,
	    const struct timespec *timeout);
#ifdef MAKE_STATIC
int     libkqueue_init();
#endif
#endif

#ifdef  __cplusplus
}
#endif
#endif /* !__KERNEL__* */

#endif /* !_SYS_EVENT_H_ */

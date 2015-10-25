/*
 * Copyright (c) 2000-2012 Apple Inc. All rights reserved.
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
/* Copyright (c) 1998, 1999 Apple Computer, Inc. All Rights Reserved */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*-
 * Copyright (c) 1982, 1986, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)socketvar.h	8.3 (Berkeley) 2/19/95
 * $FreeBSD: src/sys/sys/socketvar.h,v 1.46.2.6 2001/08/31 13:45:49 jlemon Exp $
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _SYS_SOCKETVAR_H_
#define	_SYS_SOCKETVAR_H_

#include <sys/appleapiopts.h>
#include <sys/queue.h>			/* for TAILQ macros */
#include <sys/select.h>			/* for struct selinfo */
#include <net/kext_net.h>
#include <sys/ev.h>
#include <sys/cdefs.h>

/*
 * Hacks to get around compiler complaints
 */
struct mbuf;
struct socket_filter_entry;
struct protosw;
struct sockif;
struct sockutil;

#ifdef KERNEL_PRIVATE
/* strings for sleep message: */
extern	char netio[], netcon[], netcls[];
#define	SOCKET_CACHE_ON
#define	SO_CACHE_FLUSH_INTERVAL 1	/* Seconds */
#define	SO_CACHE_TIME_LIMIT	(120/SO_CACHE_FLUSH_INTERVAL) /* Seconds */
#define	SO_CACHE_MAX_FREE_BATCH	50
#define	MAX_CACHED_SOCKETS	60000
#define	TEMPDEBUG		0

/*
 * Kernel structure per socket.
 * Contains send and receive buffer queues,
 * handle on protocol and pointer to protocol
 * private data and error information.
 */
#endif /* KERNEL_PRIVATE */

typedef	u_quad_t so_gen_t;

#ifdef PRIVATE
#define SO_TC_STATS_MAX 4

struct data_stats {
        u_int64_t       rxpackets;
        u_int64_t       rxbytes;
        u_int64_t       txpackets;
        u_int64_t       txbytes;
};
#endif /* PRIVATE */

#ifdef KERNEL_PRIVATE
#ifndef __APPLE__
/* We don't support BSD style socket filters */
struct accept_filter;
#endif

struct socket {
	int	so_zone;		/* zone we were allocated from */
	short	so_type;		/* generic type, see socket.h */
	u_int32_t so_options;		/* from socket call, see socket.h */
	short	so_linger;		/* time to linger while closing */
	short	so_state;		/* internal state flags SS_*, below */
	void	*so_pcb;		/* protocol control block */
	struct	protosw *so_proto;	/* protocol handle */
	/*
	 * Variables for connection queueing.
	 * Socket where accepts occur is so_head in all subsidiary sockets.
	 * If so_head is 0, socket is not related to an accept.
	 * For head socket so_incomp queues partially completed connections,
	 * while so_comp is a queue of connections ready to be accepted.
	 * If a connection is aborted and it has so_head set, then
	 * it has to be pulled out of either so_incomp or so_comp.
	 * We allow connections to queue up based on current queue lengths
	 * and limit on number of queued connections for this socket.
	 */
	struct	socket *so_head;	/* back pointer to accept socket */
	TAILQ_HEAD(, socket) so_incomp;	/* q of partially unaccepted conns */
	TAILQ_HEAD(, socket) so_comp;	/* q of complete unaccepted conns */
	TAILQ_ENTRY(socket) so_list;	/* list of unaccepted connections */
	short	so_qlen;		/* number of unaccepted connections */
	short	so_incqlen;		/* number of unaccepted incomplete
					   connections */
	short	so_qlimit;		/* max number queued connections */
	short	so_timeo;		/* connection timeout */
	u_short	so_error;		/* error affecting connection */
	pid_t	so_pgid;		/* pgid for signals */
	u_int32_t	so_oobmark;		/* chars to oob mark */
#ifndef __APPLE__
	/* We don't support AIO ops */
	TAILQ_HEAD(, aiocblist) so_aiojobq; /* AIO ops waiting on socket */
#endif
	/*
	 * Variables for socket buffering.
	 */
	struct	sockbuf {
		u_int32_t	sb_cc;		/* actual chars in buffer */
		u_int32_t	sb_hiwat;	/* max actual char count */
		u_int32_t	sb_mbcnt;	/* chars of mbufs used */
		u_int32_t	sb_mbmax;	/* max chars of mbufs to use */
		u_int32_t	sb_ctl;		/* non-data chars in buffer */
		u_int32_t	sb_lowat;	/* low water mark */
		struct	mbuf *sb_mb;	/* the mbuf chain */
		struct	mbuf *sb_mbtail; /* the last mbuf in the chain */
		struct	mbuf *sb_lastrecord; /* first mbuf of last record */
#if __APPLE__
		struct  socket *sb_so;  /* socket back ptr for kexts */
#endif
		struct	selinfo sb_sel;	/* process selecting read/write */
		short	sb_flags;	/* flags, see below */
		struct timeval sb_timeo; /* timeout for read/write */
		u_int32_t sb_idealsize; /* Ideal size for the sb based on bandwidth and delay */
		void	*reserved1[4];	/* for future use */
	} so_rcv, so_snd;
#define	SB_MAX		(8192*1024)	/* default for max chars in sockbuf */
#define LOW_SB_MAX	(2*9*1024)	/* lower limit on max socket buffer size, 2 max datagrams */
#define	SB_LOCK		0x01		/* lock on data queue */
#define	SB_WANT		0x02		/* someone is waiting to lock */
#define	SB_WAIT		0x04		/* someone is waiting for data/space */
#define	SB_SEL		0x08		/* someone is selecting */
#define	SB_ASYNC	0x10		/* ASYNC I/O, need signals */
#define	SB_UPCALL	0x20		/* someone wants an upcall */
#define	SB_NOINTR	0x40		/* operations not interruptible */
#define	SB_KNOTE	0x100		/* kernel note attached */
#define SB_USRSIZE	0x200		/* user specified sbreserve */
#ifndef __APPLE__
#define	SB_AIO		0x80		/* AIO operations queued */
#else
#define	SB_NOTIFY	(SB_WAIT|SB_SEL|SB_ASYNC)
#define	SB_DROP		0x400		/* does not accept any more data */
#define	SB_UNIX		0x800		/* UNIX domain socket buffer */
#define	SB_AUTOSIZE	0x1000		/* automatically size socket buffer */
#define	SB_TRIM		0x2000		/* Trim the socket buffer */
#define	SB_RECV		0x8000		/* this is rcv sb */

	caddr_t	so_tpcb;	/* Wisc. protocol control block, used by some kexts */
#endif

	void	(*so_upcall)(struct socket *so, caddr_t arg, int waitf);
	caddr_t	so_upcallarg;		/* Arg for above */
	kauth_cred_t	so_cred;	/* cred of who opened the socket */
	/* NB: generation count must not be first; easiest to make it last. */
	so_gen_t so_gencnt;		/* generation count */
#ifndef __APPLE__
	void	*so_emuldata;		/* private data for emulators */
	struct so_accf {
		struct	accept_filter *so_accept_filter;
		void	*so_accept_filter_arg;	/* saved filter args */
		char	*so_accept_filter_str;	/* saved user args */
	} *so_accf;
#else
	TAILQ_HEAD(, eventqelt) so_evlist;
	int	cached_in_sock_layer;	/* bundled with pcb/pcb.inp_ppcb? */
	struct	socket	*cache_next;
	struct	socket	*cache_prev;
	u_int32_t		cache_timestamp;
	caddr_t		so_saved_pcb;	/* Saved pcb when cacheing */
	struct	mbuf *so_temp;		/* Holding area for outbound frags */
	/* Plug-in support - make the socket interface overridable */
	struct	mbuf *so_tail;
	struct socket_filter_entry *so_filt;	/* NKE hook */
	u_int32_t	so_flags;	/* Flags */
#define	SOF_NOSIGPIPE	0x1
#define	SOF_NOADDRAVAIL	0x2	/* EADDRNOTAVAIL if src addr is gone */
#define	SOF_PCBCLEARING	0x4	/* pru_disconnect done; don't call pru_detach */
#define	SOF_DEFUNCT	0x8	/* socket marked as inactive */
#define	SOF_CLOSEWAIT	0x10	/* blocked in close awaiting some events */
#define SOF_REUSESHAREUID	0x40	/* Allows SO_REUSEADDR/SO_REUSEPORT for multiple so_uid */
#define	SOF_MULTIPAGES	0x80	/* jumbo clusters may be used for sosend */
#define SOF_ABORTED	0x100	/* soabort was already called once on the socket */
#define SOF_OVERFLOW 0x200	/* socket was dropped as overflow of listen queue */
#ifdef __APPLE_API_PRIVATE
#define SOF_NOTIFYCONFLICT 0x400	/* notify that a bind was done on a port already in use */
#endif
#define	SOF_UPCALLCLOSEWAIT 0x800 /* block on close until an upcall returns  */
#define SOF_BINDRANDOMPORT 0x1000 /* Request a randomized port number for the bind */
#define SOF_NPX_SETOPTSHUT 0x2000 /* Non POSIX extension to allow setsockopt(2) after shut down */
#define SOF_RECV_TRAFFIC_CLASS	0x4000	/* Receive traffic class as ancillary data */
#define	SOF_NODEFUNCT	0x8000	/* socket cannot be defunct'd */
#define	SOF_PRIVILEGED_TRAFFIC_CLASS 0x10000 /* traffic class is privileged */
#define SOF_SUSPENDED		0x20000 /* interface output queue is suspended */
#define SOF_INCOMP_INPROGRESS	0x40000 /* incomp socket still being processed */
#define	SOF_NOTSENT_LOWAT	0x80000 /* A different lowat on not sent data has been set */
#define SOF_KNOTE	0x100000 /* socket is on the EV_SOCK klist */
#define SOF_USELRO	0x200000 /* TCP must use LRO on these sockets */
	uint32_t	so_upcallusecount;	/* number of upcalls in progress */
	int	so_usecount;	/* refcounting of socket use */;
	int	so_retaincnt;
	u_int32_t so_filteruse;	/* usecount for the socket filters */
	u_int32_t	so_traffic_mgt_flags;	/* traffic_mgt socket config */
	thread_t	so_send_filt_thread;
	u_int32_t	so_restrictions;

/* for debug pruposes */
#define	SO_LCKDBG_MAX 4	/* number of debug locking Link Registers recorded */
	void	*lock_lr[SO_LCKDBG_MAX];	/* locking calling history */
	int	next_lock_lr;
	void	*unlock_lr[SO_LCKDBG_MAX];	/* unlocking caller history */
	int	next_unlock_lr;
	void *reserved; /* reserved for future use */
#endif /* __APPLE__ */

	struct	label *so_label;	/* MAC label for socket */
	struct	label *so_peerlabel;	/* cached MAC label for socket peer */
	thread_t	so_background_thread;	/* thread that marked this socket background */
	int		so_traffic_class;

	// last process to interact with this socket
	u_int64_t	last_upid;
	pid_t		last_pid;
	struct data_stats	so_tc_stats[SO_TC_STATS_MAX];
	struct klist	so_klist; /* klist for EV_SOCK events */
};

/* Control message accessor in mbufs */

#define _MIN_NXT_CMSGHDR_PTR(cmsg)                              \
	((char *)(cmsg) +                                       \
	    __DARWIN_ALIGN32((__uint32_t)(cmsg)->cmsg_len) +    \
	    __DARWIN_ALIGN32(sizeof(struct cmsghdr)))

#define M_FIRST_CMSGHDR(m)                                                                      \
        ((char *)(m) != (char *)0L && (size_t)(m)->m_len >= sizeof(struct cmsghdr) &&           \
	  (socklen_t)(m)->m_len >= __DARWIN_ALIGN32(((struct cmsghdr *)(void *)(m)->m_data)->cmsg_len) ?\
         (struct cmsghdr *)(void *)(m)->m_data :                                                        \
         (struct cmsghdr *)0L)

#define M_NXT_CMSGHDR(m, cmsg)                                                  \
        ((char *)(cmsg) == (char *)0L ? M_FIRST_CMSGHDR(m) :                    \
            _MIN_NXT_CMSGHDR_PTR(cmsg) > ((char *)(m)->m_data) + (m)->m_len ||  \
            _MIN_NXT_CMSGHDR_PTR(cmsg) < (char *)(m)->m_data ?                  \
                (struct cmsghdr *)0L /* NULL */ :                               \
                (struct cmsghdr *)(void *)((unsigned char *)(cmsg) +            \
                            __DARWIN_ALIGN32((__uint32_t)(cmsg)->cmsg_len)))

#endif /* KERNEL_PRIVATE */

/*
 * Socket state bits.
 */
#define	SS_NOFDREF		0x0001	/* no file table ref any more */
#define	SS_ISCONNECTED		0x0002	/* socket connected to a peer */
#define	SS_ISCONNECTING		0x0004	/* in process of connecting to peer */
#define	SS_ISDISCONNECTING	0x0008	/* in process of disconnecting */
#define	SS_CANTSENDMORE		0x0010	/* can't send more data to peer */
#define	SS_CANTRCVMORE		0x0020	/* can't receive more data from peer */
#define	SS_RCVATMARK		0x0040	/* at mark on input */

#define	SS_PRIV			0x0080	/* privileged for broadcast, raw... */
#define	SS_NBIO			0x0100	/* non-blocking ops */
#define	SS_ASYNC		0x0200	/* async i/o notify */
#define	SS_ISCONFIRMING		0x0400	/* deciding to accept connection req */
#define	SS_INCOMP		0x0800	/* Unaccepted, incomplete connection */
#define	SS_COMP			0x1000	/* unaccepted, complete connection */
#define	SS_ISDISCONNECTED	0x2000	/* socket disconnected from peer */
#define	SS_DRAINING		0x4000	/* close waiting for blocked system
					   calls to drain */
#define	SS_DEFUNCT		0x8000	/* has been fully defunct'd */

#if defined(__LP64__)
#define	_XSOCKET_PTR(x)		u_int32_t
#else
#define	_XSOCKET_PTR(x)		x
#endif

#ifdef PRIVATE
/* Flags returned in data field for EVFILT_SOCK events. */
#define SOCKEV_CONNECTED	0x00000001 /* connected */
#define SOCKEV_DISCONNECTED	0x00000002 /* disconnected */
#endif /* PRIVATE */

#pragma pack(4)

struct xsockbuf {
	u_int32_t	sb_cc;
	u_int32_t	sb_hiwat;
	u_int32_t	sb_mbcnt;
	u_int32_t	sb_mbmax;
	int32_t		sb_lowat;
	short		sb_flags;
	short		sb_timeo;
};

/*
 * Externalized form of struct socket used by the sysctl(3) interface.
 */
struct	xsocket {
	u_int32_t		xso_len;		/* length of this structure */
	_XSOCKET_PTR(struct socket *) xso_so;	/* makes a convenient handle */
	short			so_type;
	short			so_options;
	short			so_linger;
	short			so_state;
	_XSOCKET_PTR(caddr_t)	so_pcb;		/* another convenient handle */
	int				xso_protocol;
	int				xso_family;
	short			so_qlen;
	short			so_incqlen;
	short			so_qlimit;
	short			so_timeo;
	u_short			so_error;
	pid_t			so_pgid;
	u_int32_t		so_oobmark;
	struct xsockbuf	so_rcv;
	struct xsockbuf	so_snd;
	uid_t			so_uid;		/* XXX */
};

#if !CONFIG_EMBEDDED

struct	xsocket64 {
	u_int32_t		xso_len;		/* length of this structure */
	u_int64_t		xso_so;	/* makes a convenient handle */
	short			so_type;
	short			so_options;
	short			so_linger;
	short			so_state;
	u_int64_t		so_pcb;		/* another convenient handle */
	int			xso_protocol;
	int			xso_family;
	short			so_qlen;
	short			so_incqlen;
	short			so_qlimit;
	short			so_timeo;
	u_short			so_error;
	pid_t			so_pgid;
	u_int32_t		so_oobmark;
	struct xsockbuf		so_rcv;
	struct xsockbuf		so_snd;
	uid_t			so_uid;		/* XXX */
};

#endif /* !CONFIG_EMBEDDED */

#ifdef PRIVATE

#define XSO_SOCKET	0x001
#define XSO_RCVBUF	0x002
#define XSO_SNDBUF	0x004
#define XSO_STATS	0x008
#define XSO_INPCB	0x010
#define XSO_TCPCB	0x020

struct	xsocket_n {
	u_int32_t		xso_len;		/* length of this structure */
	u_int32_t		xso_kind;		/* XSO_SOCKET */
	u_int64_t		xso_so;	/* makes a convenient handle */
	short			so_type;
	u_int32_t		so_options;
	short			so_linger;
	short			so_state;
	u_int64_t		so_pcb;		/* another convenient handle */
	int				xso_protocol;
	int				xso_family;
	short			so_qlen;
	short			so_incqlen;
	short			so_qlimit;
	short			so_timeo;
	u_short			so_error;
	pid_t			so_pgid;
	u_int32_t		so_oobmark;
	uid_t			so_uid;		/* XXX */
};

struct xsockbuf_n {
	u_int32_t		xsb_len;		/* length of this structure */
	u_int32_t		xsb_kind;		/* XSO_RCVBUF or XSO_SNDBUF */
	u_int32_t		sb_cc;
	u_int32_t		sb_hiwat;
	u_int32_t		sb_mbcnt;
	u_int32_t		sb_mbmax;
	int32_t			sb_lowat;
	short			sb_flags;
	short			sb_timeo;
};

struct xsockstat_n {
	u_int32_t		xst_len;		/* length of this structure */
	u_int32_t		xst_kind;		/* XSO_STATS */
	struct data_stats	xst_tc_stats[SO_TC_STATS_MAX];
};

#endif /* PRIVATE */

#pragma pack()

#ifdef KERNEL_PRIVATE
/*
 * Macros for sockets and socket buffering.
 */

#define	sbtoso(sb) (sb->sb_so)

/*
 * Functions for sockets and socket buffering.
 * These are macros on FreeBSD. On Darwin the
 * implementation is in bsd/kern/uipc_socket2.c
 */

__BEGIN_DECLS
int	sb_notify(struct sockbuf *sb);
int	sbspace(struct sockbuf *sb);
int	sosendallatonce(struct socket *so);
int	soreadable(struct socket *so);
int	sowriteable(struct socket *so);
void	sballoc(struct sockbuf *sb, struct mbuf *m);
void	sbfree(struct sockbuf *sb, struct mbuf *m);
int	sblock(struct sockbuf *sb, int wf);
void	sbunlock(struct sockbuf *sb, int locked);
void	sorwakeup(struct socket *so);
void	sowwakeup(struct socket *so);
__END_DECLS

/*
 * Socket extension mechanism: control block hooks:
 * This is the "head" of any control block for an extenstion
 * Note: we separate intercept function dispatch vectors from
 *  the NFDescriptor to permit selective replacement during
 *  operation, e.g., to disable some functions.
 */
struct kextcb {
	struct kextcb *e_next;		/* Next kext control block */
	void *e_fcb;			/* Real filter control block */
	struct NFDescriptor *e_nfd;	/* NKE Descriptor */
	/* Plug-in support - intercept functions */
	struct sockif *e_soif;		/* Socket functions */
	struct sockutil *e_sout;	/* Sockbuf utility functions */
};
#define	EXT_NULL	0x0		/* STATE: Not in use */
#define	sotokextcb(so) (so ? so->so_ext : 0)

#ifdef KERNEL
#include <sys/kpi_mbuf.h>

/* Hints for socket event processing */
#define SO_FILT_HINT_LOCKED      0x00000001	/* socket is already locked */
#define SO_FILT_HINT_CONNRESET   0x00000002	/* Reset is received */
#define SO_FILT_HINT_CANTRCVMORE 0x00000004	/* No more data to read */
#define SO_FILT_HINT_CANTSENDMORE 0x00000008	/* Can't write more data */
#define SO_FILT_HINT_TIMEOUT     0x00000010	/* timeout */
#define SO_FILT_HINT_NOSRCADDR   0x00000020	/* No src address available */
#define SO_FILT_HINT_IFDENIED    0x00000040	/* interface denied connection */
#define SO_FILT_HINT_SUSPEND     0x00000080	/* output queue suspended */
#define SO_FILT_HINT_RESUME      0x00000100	/* output queue resumed */
#define SO_FILT_HINT_KEEPALIVE	 0x00000200	/* TCP Keepalive received */

#define SO_FILT_HINT_EV (SO_FILT_HINT_CONNRESET | \
	SO_FILT_HINT_CANTRCVMORE | SO_FILT_HINT_CANTSENDMORE | \
	SO_FILT_HINT_TIMEOUT | SO_FILT_HINT_NOSRCADDR | \
	SO_FILT_HINT_IFDENIED | SO_FILT_HINT_SUSPEND | \
	SO_FILT_HINT_RESUME | SO_FILT_HINT_KEEPALIVE)

/*
 * Argument structure for sosetopt et seq.  This is in the KERNEL
 * section because it will never be visible to user code.
 */
enum sopt_dir { SOPT_GET, SOPT_SET };
struct sockopt {
	enum	sopt_dir sopt_dir; /* is this a get or a set? */
	int	sopt_level;	/* second arg of [gs]etsockopt */
	int	sopt_name;	/* third arg of [gs]etsockopt */
	user_addr_t sopt_val;	/* fourth arg of [gs]etsockopt */
	size_t	sopt_valsize;	/* (almost) fifth arg of [gs]etsockopt */
	struct	proc *sopt_p;	/* calling process or null if kernel */
};

#if SENDFILE
struct sf_buf {
	SLIST_ENTRY(sf_buf) free_list;	/* list of free buffer slots */
	int		refcnt;		/* reference count */
	struct		vm_page *m;	/* currently mapped page */
	vm_offset_t	kva;		/* va of mapping */
};
#endif

#ifdef MALLOC_DECLARE
MALLOC_DECLARE(M_PCB);
MALLOC_DECLARE(M_SONAME);
#endif

extern int	maxsockets;
extern u_int32_t	sb_max;
extern int socket_zone;
extern so_gen_t so_gencnt;
extern int	socket_debug;
extern int sosendjcl;
extern int sosendjcl_ignore_capab;
extern int sodefunctlog;
extern int sothrottlelog;
extern int somaxconn;

struct file;
struct filedesc;
struct mbuf;
struct sockaddr;
struct stat;
struct ucred;
struct uio;
struct knote;
struct so_tcdbg;

#define	SBLASTRECORDCHK(sb, s)	\
	if (socket_debug) sblastrecordchk(sb, s);

#define	SBLASTMBUFCHK(sb, s)	\
	if (socket_debug) sblastmbufchk(sb, s);

#define	SB_EMPTY_FIXUP(sb) {			\
	if ((sb)->sb_mb == NULL) {		\
		(sb)->sb_mbtail = NULL;		\
		(sb)->sb_lastrecord = NULL;	\
	}					\
}

#define SB_MB_CHECK(sb) do {			\
	if (((sb)->sb_mb != NULL && 		\
		(sb)->sb_cc == 0) ||		\
		((sb)->sb_mb == NULL && 	\
		(sb)->sb_cc > 0))		\
		panic("corrupt so_rcv: sb_mb %p sb_cc %d\n", \
			(sb)->sb_mb, (sb)->sb_cc);	\
} while(0)
		 

#define	SODEFUNCTLOG(x)		do { if (sodefunctlog) printf x; } while (0)

#define	SOTHROTTLELOG(x)	do { if (sothrottlelog) printf x; } while (0)

/*
 * For debugging traffic class behaviors
 */
#define SOTCDB_NO_DSCP		0x01	/* Do not set DSCP code in IP header */
#define SOTCDB_NO_MTC		0x02	/* Do not set the mbuf traffic class */
#define SOTCDB_NO_SENDTCPBG	0x04	/* Do not use background TCP CC algorithm for sender */
#define SOTCDB_NO_LCLTST	0x08	/* Do not test for local destination for setting DSCP */
#define SOTCDB_NO_DSCPTST	0x10	/* Overwritte any existing DSCP code */
#define SOTCDB_NO_RECVTCPBG	0x20	/* Do not use throttling on receiver-side of TCP */
#define	SOTCDB_NO_PRIVILEGED	0x40	/* Do not set privileged traffic flag */

extern u_int32_t sotcdb;

/*
 * From uipc_socket and friends
 */
__BEGIN_DECLS
/* Not exported */
extern void socketinit(void) __attribute__((section("__TEXT, initcode")));

/* Exported */
extern struct sockaddr *dup_sockaddr(struct sockaddr *sa, int canwait);
extern int getsock(struct filedesc *fdp, int fd, struct file **fpp);
extern int sockargs(struct mbuf **mp, user_addr_t data, int buflen, int type);

extern int sbappend(struct sockbuf *sb, struct mbuf *m);
extern int sbappendstream(struct sockbuf *sb, struct mbuf *m);
extern int sbappendaddr(struct sockbuf *sb, struct sockaddr *asa,
    struct mbuf *m0, struct mbuf *control, int *error_out);
extern int sbappendcontrol(struct sockbuf *sb, struct mbuf *m0,
    struct mbuf *control, int *error_out);
extern int sbappendrecord(struct sockbuf *sb, struct mbuf *m0);
extern void sbcheck(struct sockbuf *sb);
extern void sblastmbufchk(struct sockbuf *, const char *);
extern void sblastrecordchk(struct sockbuf *, const char *);
extern struct mbuf *sbcreatecontrol(caddr_t p, int size, int type, int level);
extern struct mbuf **sbcreatecontrol_mbuf(caddr_t p, int size, int type, int level, struct mbuf** m);
extern void sbdrop(struct sockbuf *sb, int len);
extern void sbdroprecord(struct sockbuf *sb);
extern void sbflush(struct sockbuf *sb);
extern int sbinsertoob(struct sockbuf *sb, struct mbuf *m0);
extern void sbrelease(struct sockbuf *sb);
extern int sbreserve(struct sockbuf *sb, u_int32_t cc);
extern void sbtoxsockbuf(struct sockbuf *sb, struct xsockbuf *xsb);
extern int sbwait(struct sockbuf *sb);
extern int sb_lock(struct sockbuf *sb);
extern void sb_empty_assert(struct sockbuf *, const char *);

extern int soabort(struct socket *so);
extern int soaccept(struct socket *so, struct sockaddr **nam);
extern int soacceptlock(struct socket *so, struct sockaddr **nam, int dolock);
extern int soacceptfilter(struct socket *so);
extern struct socket *soalloc(int waitok, int dom, int type);
extern int sobind(struct socket *so, struct sockaddr *nam);
extern void socantrcvmore(struct socket *so);
extern void socantsendmore(struct socket *so);
extern int soclose(struct socket *so);
extern int soconnect(struct socket *so, struct sockaddr *nam);
extern int soconnectlock(struct socket *so, struct sockaddr *nam, int dolock);
extern int soconnect2(struct socket *so1, struct socket *so2);
extern int socreate(int dom, struct socket **aso, int type, int proto);
extern void sodealloc(struct socket *so);
extern int sodisconnect(struct socket *so);
extern int sodisconnectlocked(struct socket *so);
extern void sofree(struct socket *so);
extern void soreference(struct socket *so);
extern void sodereference(struct socket *so);
extern void somultipages(struct socket *, boolean_t);
extern int sosetdefunct(struct proc *, struct socket *, int level, boolean_t);
extern int sodefunct(struct proc *, struct socket *, int level);
extern int sogetopt(struct socket *so, struct sockopt *sopt);
extern void sohasoutofband(struct socket *so);
extern void soisconnected(struct socket *so);
extern void soisconnecting(struct socket *so);
extern void soisdisconnected(struct socket *so);
extern void sodisconnectwakeup(struct socket *so);
extern void soisdisconnecting(struct socket *so);
extern int soisthrottled(struct socket *so);
extern int soisprivilegedtraffic(struct socket *so);
extern int solisten(struct socket *so, int backlog);
extern struct socket *sodropablereq(struct socket *head);
extern struct socket *sonewconn(struct socket *head, int connstatus,
    const struct sockaddr *from);
extern int sooptcopyin(struct sockopt *sopt, void *data, size_t len,
    size_t minlen);
extern int sooptcopyout(struct sockopt *sopt, void *data, size_t len);
extern int socket_lock(struct socket *so, int refcount);
extern int socket_unlock(struct socket *so, int refcount);
extern void sofreelastref(struct socket *, int);
extern int sogetaddr_locked(struct socket *, struct sockaddr **, int);
extern const char *solockhistory_nr(struct socket *);
extern void soevent(struct socket *so, long hint);
extern void get_sockev_state(struct socket *, u_int32_t *);

#ifdef BSD_KERNEL_PRIVATE
/* Service class flags used for setting service class on a packet */
#define PKT_SCF_IPV6		0x00000001	/* IPv6 packet */
#define PKT_SCF_TCP_ACK		0x00000002	/* Pure TCP ACK */

extern void set_packet_service_class(struct mbuf *, struct socket *,
    mbuf_svc_class_t, u_int32_t);
extern void so_tc_update_stats(struct mbuf *, struct socket *, mbuf_svc_class_t );
extern mbuf_svc_class_t mbuf_service_class_from_control(struct mbuf *);
extern mbuf_svc_class_t so_tc2msc(int);
extern int so_svc2tc(mbuf_svc_class_t);

extern void set_tcp_stream_priority(struct socket *so);
extern int so_set_traffic_class(struct socket *, int);
extern void so_set_default_traffic_class(struct socket *);
extern int so_set_opportunistic(struct socket *, int);
extern int so_get_opportunistic(struct socket *);
extern int so_set_recv_anyif(struct socket *, int);
extern int so_get_recv_anyif(struct socket *);
extern void socket_tclass_init(void);
extern int so_set_tcdbg(struct socket *, struct so_tcdbg *);
extern int sogetopt_tcdbg(struct socket *, struct sockopt *);
extern void so_recv_data_stat(struct socket *, struct mbuf *, size_t);
extern int so_wait_for_if_feedback(struct socket *);
#endif /* BSD_KERNEL_PRIVATE */

/*
 * XXX; prepare mbuf for (__FreeBSD__ < 3) routines.
 * Used primarily in IPSec and IPv6 code.
 */
extern int soopt_getm(struct sockopt *sopt, struct mbuf **mp);
extern int soopt_mcopyin(struct sockopt *sopt, struct mbuf *m);
extern int soopt_mcopyout(struct sockopt *sopt, struct mbuf *m);

extern int sopoll(struct socket *so, int events, struct ucred *cred, void *wql);
extern int soreceive(struct socket *so, struct sockaddr **paddr,
    struct uio *uio, struct mbuf **mp0, struct mbuf **controlp, int *flagsp);
extern int soreserve(struct socket *so, u_int32_t sndcc, u_int32_t rcvcc);
extern void sorflush(struct socket *so);
extern int sosend(struct socket *so, struct sockaddr *addr, struct uio *uio,
    struct mbuf *top, struct mbuf *control, int flags);

extern int sosetopt(struct socket *so, struct sockopt *sopt);
extern int soshutdown(struct socket *so, int how);
extern int soshutdownlock(struct socket *so, int how);
extern void sotoxsocket(struct socket *so, struct xsocket *xso);
#if !CONFIG_EMBEDDED
extern void sotoxsocket64(struct socket *so, struct xsocket64 *xso);
#endif
extern void sbwakeup(struct sockbuf *sb);
extern void sowakeup(struct socket *so, struct sockbuf *sb);
extern int soioctl(struct socket *so, u_long cmd, caddr_t data, struct proc *p);

#ifndef __APPLE__
/* accept filter functions */
extern int accept_filt_add(struct accept_filter *filt);
extern int accept_filt_del(char *name);
extern struct accept_filter *accept_filt_get(char *name);
#ifdef ACCEPT_FILTER_MOD
extern int accept_filt_generic_mod_event(module_t mod, int event, void *data);
SYSCTL_DECL(_net_inet_accf);
#endif /* ACCEPT_FILTER_MOD */
#endif /* !defined(__APPLE__) */

__END_DECLS

#endif /* KERNEL */
#endif /* KERNEL_PRIVATE */

#endif /* !_SYS_SOCKETVAR_H_ */

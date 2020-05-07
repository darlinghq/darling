/*
 * Copyright (c) 2000-2019 Apple Inc. All rights reserved.
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
/*
 * Copyright (c) 1982, 1986, 1993
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
 *	@(#)protosw.h	8.1 (Berkeley) 6/2/93
 * $FreeBSD: src/sys/sys/protosw.h,v 1.28.2.2 2001/07/03 11:02:01 ume Exp $
 */

#ifndef _SYS_PROTOSW_H_
#define _SYS_PROTOSW_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>

/* XXX: this will go away */
#define PR_SLOWHZ       2               /* 2 slow timeouts per second */

/*
 * The arguments to the ctlinput routine are
 *      (*protosw[].pr_ctlinput)(cmd, sa, arg);
 * where cmd is one of the commands below, sa is a pointer to a sockaddr,
 * and arg is a `void *' argument used within a protocol family.
 */
#define PRC_IFDOWN              0       /* interface transition */
#define PRC_ROUTEDEAD           1       /* select new route if possible ??? */
#define PRC_IFUP                2       /* interface has come back up */
#define PRC_QUENCH2             3       /* DEC congestion bit says slow down */
#define PRC_QUENCH              4       /* some one said to slow down */
#define PRC_MSGSIZE             5       /* message size forced drop */
#define PRC_HOSTDEAD            6       /* host appears to be down */
#define PRC_HOSTUNREACH         7       /* deprecated (use PRC_UNREACH_HOST) */
#define PRC_UNREACH_NET         8       /* no route to network */
#define PRC_UNREACH_HOST        9       /* no route to host */
#define PRC_UNREACH_PROTOCOL    10      /* dst says bad protocol */
#define PRC_UNREACH_PORT        11      /* bad port # */
/* was PRC_UNREACH_NEEDFRAG	12         (use PRC_MSGSIZE) */
#define PRC_UNREACH_SRCFAIL     13      /* source route failed */
#define PRC_REDIRECT_NET        14      /* net routing redirect */
#define PRC_REDIRECT_HOST       15      /* host routing redirect */
#define PRC_REDIRECT_TOSNET     16      /* redirect for type of service & net */
#define PRC_REDIRECT_TOSHOST    17      /* redirect for tos & host */
#define PRC_TIMXCEED_INTRANS    18      /* packet lifetime expired in transit */
#define PRC_TIMXCEED_REASS      19      /* lifetime expired on reass q */
#define PRC_PARAMPROB           20      /* header incorrect */
#define PRC_UNREACH_ADMIN_PROHIB        21     /* packet administrativly prohibited */

#define PRC_NCMDS               22

#define PRC_IS_REDIRECT(cmd)    \
	((cmd) >= PRC_REDIRECT_NET && (cmd) <= PRC_REDIRECT_TOSHOST)

#ifdef BSD_KERNEL_PRIVATE
#include <sys/eventhandler.h>
#endif

#ifdef KERNEL_PRIVATE
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/queue.h>
#include <kern/locks.h>

/* Forward declare these structures referenced from prototypes below. */
struct mbuf;
struct proc;
struct sockaddr;
struct socket;
struct sockopt;
struct socket_filter;
struct uio;
struct ifnet;
#ifdef XNU_KERNEL_PRIVATE
struct domain_old;
struct proc;
#endif /* XNU_KERNEL_PRIVATE */

#pragma pack(4)

#ifdef XNU_KERNEL_PRIVATE
/*
 * Legacy protocol switch table.
 *
 * NOTE: Do not modify this structure, as there are modules outside of xnu
 * which rely on the size and layout for binary compatibility.  This structure
 * is simply used by the exported net_{add,del}_proto_old, pffindproto_old
 * routines, and by the domain_old structure.  Internally, protocol switch
 * tables are stored in the private variant of protosw defined down below.
 */
struct protosw_old {
#else
struct protosw {
#endif /* !XNU_KERNEL_PRIVATE */
	short   pr_type;                /* socket type used for */
	struct  domain *pr_domain;      /* domain protocol a member of */
	short   pr_protocol;            /* protocol number */
	unsigned int pr_flags;          /* see below */
	/*
	 * protocol-protocol hooks
	 */
	void    (*pr_input)             /* input to protocol (from below) */
	(struct mbuf *, int len);
	int     (*pr_output)            /* output to protocol (from above) */
	(struct mbuf *m, struct socket *so);
	void    (*pr_ctlinput)          /* control input (from below) */
	(int, struct sockaddr *, void *, struct ifnet *);
	int     (*pr_ctloutput)         /* control output (from above) */
	(struct socket *, struct sockopt *);
	/*
	 * user-protocol hook
	 */
	void    *pr_ousrreq;
	/*
	 * utility hooks
	 */
	void    (*pr_init)(void);       /* initialization hook */
	void    (*pr_unused)(void);     /* placeholder - fasttimo is removed */
	void    (*pr_unused2)(void);    /* placeholder - slowtimo is removed */
	void    (*pr_drain)(void);      /* flush any excess space possible */
	int     (*pr_sysctl)            /* sysctl for protocol */
	(int *, u_int, void *, size_t *, void *, size_t);
#ifdef XNU_KERNEL_PRIVATE
	struct  pr_usrreqs_old *pr_usrreqs; /* supersedes pr_usrreq() */
#else
	struct  pr_usrreqs *pr_usrreqs; /* supersedes pr_usrreq() */
#endif /* !XNU_KERNEL_PRIVATE */
	int     (*pr_lock)              /* lock function for protocol */
	(struct socket *so, int refcnt, void *debug);
	int     (*pr_unlock)            /* unlock for protocol */
	(struct socket *so, int refcnt, void *debug);
	lck_mtx_t *(*pr_getlock)        /* retrieve protocol lock */
	(struct socket *so, int flags);
	/*
	 * Implant hooks
	 */
	TAILQ_HEAD(, socket_filter) pr_filter_head;
#ifdef XNU_KERNEL_PRIVATE
	struct protosw_old *pr_next;    /* chain for domain */
#else
	struct protosw *pr_next;        /* chain for domain */
#endif /* !XNU_KERNEL_PRIVATE */
	u_int32_t       reserved[1];    /* padding for future use */
};

#pragma pack()

#ifdef XNU_KERNEL_PRIVATE
/*
 * Protocol switch table.
 *
 * Each protocol has a handle initializing one of these structures,
 * which is used for protocol-protocol and system-protocol communication.
 *
 * A protocol is called through the pr_init entry before any other.
 * The system will call the pr_drain entry if it is low on space and this
 * should throw away any non-critical data.
 *
 * Protocols pass data between themselves as chains of mbufs using
 * the pr_input and pr_output hooks.  Pr_input passes data up (towards
 * the users) and pr_output passes it down (towards the interfaces); control
 * information passes up and down on pr_ctlinput and pr_ctloutput.
 * The protocol is responsible for the space occupied by any the
 * arguments to these entries and must dispose it.
 *
 * The userreq routine interfaces protocols to the system and is
 * described below.
 *
 * After a protocol is attached, its pr_domain will be set to the domain
 * which the protocol belongs to, and its pr_protosw will be set to the
 * address of the protosw instance.  The latter is useful for finding
 * the real/original protosw instance, in the event so_proto is altered
 * to point to an alternative/derivative protosw.  E.g. the list of
 * socket filters is only applicable on the original protosw instance.
 *
 * Internal, private and extendable representation of protosw.
 */
struct protosw {
	TAILQ_ENTRY(protosw) pr_entry; /* chain for domain */
	struct  domain *pr_domain;      /* domain protocol a member of */
	struct protosw *pr_protosw;     /* pointer to self */
	u_int16_t pr_type;              /* socket type used for */
	u_int16_t pr_protocol;          /* protocol number */
	u_int32_t pr_flags;             /* see below */
	/*
	 * protocol-protocol hooks
	 */
	void    (*pr_input)             /* input to protocol (from below) */
	(struct mbuf *, int len);
	int     (*pr_output)            /* output to protocol (from above) */
	(struct mbuf *m, struct socket *so);
	void    (*pr_ctlinput)          /* control input (from below) */
	(int, struct sockaddr *, void *, struct ifnet *);
	int     (*pr_ctloutput)         /* control output (from above) */
	(struct socket *, struct sockopt *);
	/*
	 * user-protocol hook
	 */
	struct  pr_usrreqs *pr_usrreqs; /* user request; see list below */
	/*
	 * utility hooks
	 */
	void    (*pr_init)              /* initialization hook */
	(struct protosw *, struct domain *);
	void    (*pr_drain)(void);      /* flush any excess space possible */
	int     (*pr_sysctl)            /* sysctl for protocol */
	(int *, u_int, void *, size_t *, void *, size_t);
	int     (*pr_lock)              /* lock function for protocol */
	(struct socket *so, int refcnt, void *debug);
	int     (*pr_unlock)            /* unlock for protocol */
	(struct socket *so, int refcnt, void *debug);
	lck_mtx_t *(*pr_getlock)        /* retrieve protocol lock */
	(struct socket *so, int flags);
	/*
	 * misc
	 */
	TAILQ_HEAD(, socket_filter) pr_filter_head;
	struct protosw_old *pr_old;

	void    (*pr_update_last_owner) /* update last socket owner */
	(struct socket *so, struct proc *p, struct proc *ep);

	void    (*pr_copy_last_owner) /* copy last socket from listener */
	(struct socket *so, struct socket *head);
};

/*
 * Values for the flags argument of pr_getlock
 */
#define PR_F_WILLUNLOCK 0x01    /* Will unlock (e.g., msleep) after the pr_getlock call */

#endif /* XNU_KERNEL_PRIVATE */

/*
 * Values for pr_flags.
 * PR_ADDR requires PR_ATOMIC;
 * PR_ADDR and PR_CONNREQUIRED are mutually exclusive.
 * PR_IMPLOPCL means that the protocol allows sendto without prior connect,
 *	and the protocol understands the MSG_EOF flag.  The first property is
 *	is only relevant if PR_CONNREQUIRED is set (otherwise sendto is allowed
 *	anyhow).
 */
#define PR_ATOMIC       0x01    /* exchange atomic messages only */
#define PR_ADDR         0x02    /* addresses given with messages */
#define PR_CONNREQUIRED 0x04    /* connection required by protocol */
#define PR_WANTRCVD     0x08    /* want PRU_RCVD calls */
#define PR_RIGHTS       0x10    /* passes capabilities */
#define PR_IMPLOPCL     0x20    /* implied open/close */
#define PR_LASTHDR      0x40    /* enforce ipsec policy; last header */
#define PR_PROTOLOCK    0x80    /* protocol takes care of it's own locking */
#define PR_PCBLOCK      0x100   /* protocol supports per pcb locking */
#define PR_DISPOSE      0x200   /* protocol requires late lists disposal */
#ifdef BSD_KERNEL_PRIVATE
#define PR_INITIALIZED  0x400   /* protocol has been initialized */
#define PR_ATTACHED     0x800   /* protocol is attached to a domain */
#define PR_MULTICONN    0x1000  /* supports multiple connect calls */
#define PR_EVCONNINFO   0x2000  /* protocol generates conninfo event */
#define PR_PRECONN_WRITE        0x4000  /* protocol supports preconnect write */
#define PR_DATA_IDEMPOTENT      0x8000  /* protocol supports idempotent data at connectx-time */
#define PR_OLD          0x10000000 /* added via net_add_proto */

/* pseudo-public domain flags */
#define PRF_USERFLAGS                                                   \
	(PR_ATOMIC|PR_ADDR|PR_CONNREQUIRED|PR_WANTRCVD|PR_RIGHTS|       \
	PR_IMPLOPCL|PR_LASTHDR|PR_PROTOLOCK|PR_PCBLOCK|PR_DISPOSE)
#endif /* BSD_KERNEL_PRIVATE */

#ifdef BSD_KERNEL_PRIVATE
#ifdef PRCREQUESTS
char    *prcrequests[] = {
	"IFDOWN", "ROUTEDEAD", "IFUP", "DEC-BIT-QUENCH2",
	"QUENCH", "MSGSIZE", "HOSTDEAD", "#7",
	"NET-UNREACH", "HOST-UNREACH", "PROTO-UNREACH", "PORT-UNREACH",
	"#12", "SRCFAIL-UNREACH", "NET-REDIRECT", "HOST-REDIRECT",
	"TOSNET-REDIRECT", "TOSHOST-REDIRECT", "TX-INTRANS", "TX-REASS",
	"PARAMPROB", "ADMIN-UNREACH"
};
#endif /* PRCREQUESTS */

/*
 * The arguments to ctloutput are:
 *	(*protosw[].pr_ctloutput)(req, so, level, optname, optval, p);
 * req is one of the actions listed below, so is a (struct socket *),
 * level is an indication of which protocol layer the option is intended.
 * optname is a protocol dependent socket option request,
 * optval is a pointer to a mbuf-chain pointer, for value-return results.
 * The protocol is responsible for disposal of the mbuf chain *optval
 * if supplied,
 * the caller is responsible for any space held by *optval, when returned.
 * A non-zero return from usrreq gives an
 * UNIX error number which should be passed to higher level software.
 */
#define PRCO_GETOPT     0
#define PRCO_SETOPT     1

#define PRCO_NCMDS      2

#ifdef PRCOREQUESTS
char    *prcorequests[] = {
	"GETOPT", "SETOPT",
};
#endif /* PRCOREQUESTS */

/*
 * In earlier BSD network stacks, a single pr_usrreq() function pointer was
 * invoked with an operation number indicating what operation was desired.
 * We now provide individual function pointers which protocols can implement,
 * which offers a number of benefits (such as type checking for arguments).
 * These older constants are still present in order to support TCP debugging.
 */
#define PRU_ATTACH              0       /* attach protocol to up */
#define PRU_DETACH              1       /* detach protocol from up */
#define PRU_BIND                2       /* bind socket to address */
#define PRU_LISTEN              3       /* listen for connection */
#define PRU_CONNECT             4       /* establish connection to peer */
#define PRU_ACCEPT              5       /* accept connection from peer */
#define PRU_DISCONNECT          6       /* disconnect from peer */
#define PRU_SHUTDOWN            7       /* won't send any more data */
#define PRU_RCVD                8       /* have taken data; more room now */
#define PRU_SEND                9       /* send this data */
#define PRU_ABORT               10      /* abort (fast DISCONNECT, DETATCH) */
#define PRU_CONTROL             11      /* control operations on protocol */
#define PRU_SENSE               12      /* return status into m */
#define PRU_RCVOOB              13      /* retrieve out of band data */
#define PRU_SENDOOB             14      /* send out of band data */
#define PRU_SOCKADDR            15      /* fetch socket's address */
#define PRU_PEERADDR            16      /* fetch peer's address */
#define PRU_CONNECT2            17      /* connect two sockets */
/* begin for protocols internal use */
#define PRU_FASTTIMO            18      /* 200ms timeout */
#define PRU_SLOWTIMO            19      /* 500ms timeout */
#define PRU_PROTORCV            20      /* receive from below */
#define PRU_PROTOSEND           21      /* send to below */
/* end for protocol's internal use */
#define PRU_SEND_EOF            22      /* send and close */
#define PRU_NREQ                22

#ifdef PRUREQUESTS
char *prurequests[] = {
	"ATTACH", "DETACH", "BIND", "LISTEN",
	"CONNECT", "ACCEPT", "DISCONNECT", "SHUTDOWN",
	"RCVD", "SEND", "ABORT", "CONTROL",
	"SENSE", "RCVOOB", "SENDOOB", "SOCKADDR",
	"PEERADDR", "CONNECT2", "FASTTIMO", "SLOWTIMO",
	"PROTORCV", "PROTOSEND", "SEND_EOF",
};
#endif /* PRUREQUESTS */
#endif /* BSD_KERNEL_PRIVATE */

struct stat;
struct ucred;
struct uio;
struct recv_msg_elem;

#ifdef XNU_KERNEL_PRIVATE
/*
 * Legacy user-protocol hooks.
 *
 * NOTE: Do not modify this structure, as there are modules outside of xnu
 * which rely on the size and layout for binary compatibility.  This structure
 * is simply used by the protosw_old structure.  Internally, user-protocol
 * hooks use the private variant of pr_usrreqs defined down below.
 */
struct pr_usrreqs_old {
#else
struct pr_usrreqs {
#endif /* !XNU_KERNEL_PRIVATE */
	int     (*pru_abort)(struct socket *so);
	int     (*pru_accept)(struct socket *so, struct sockaddr **nam);
	int     (*pru_attach)(struct socket *so, int proto, struct proc *p);
	int     (*pru_bind)(struct socket *so, struct sockaddr *nam,
	    struct proc *p);
	int     (*pru_connect)(struct socket *so, struct sockaddr *nam,
	    struct proc *p);
	int     (*pru_connect2)(struct socket *so1, struct socket *so2);
	int     (*pru_control)(struct socket *so, u_long cmd, caddr_t data,
	    struct ifnet *ifp, struct proc *p);
	int     (*pru_detach)(struct socket *so);
	int     (*pru_disconnect)(struct socket *so);
	int     (*pru_listen)(struct socket *so, struct proc *p);
	int     (*pru_peeraddr)(struct socket *so, struct sockaddr **nam);
	int     (*pru_rcvd)(struct socket *so, int flags);
	int     (*pru_rcvoob)(struct socket *so, struct mbuf *m, int flags);
	int     (*pru_send)(struct socket *so, int flags, struct mbuf *m,
	    struct sockaddr *addr, struct mbuf *control,
	    struct proc *p);
	int     (*pru_sense)(struct socket *so, void  *sb, int isstat64);
	int     (*pru_shutdown)(struct socket *so);
	int     (*pru_sockaddr)(struct socket *so, struct sockaddr **nam);
	int     (*pru_sosend)(struct socket *so, struct sockaddr *addr,
	    struct uio *uio, struct mbuf *top, struct mbuf *control,
	    int flags);
	int     (*pru_soreceive)(struct socket *so, struct sockaddr **paddr,
	    struct uio *uio, struct mbuf **mp0, struct mbuf **controlp,
	    int *flagsp);
	int     (*pru_sopoll)(struct socket *so, int events,
	    struct ucred *cred, void *);
};

#ifdef XNU_KERNEL_PRIVATE
/*
 * If the ordering here looks odd, that's because it's alphabetical.  These
 * should eventually be merged back into struct protosw.
 *
 * Internal, private and extendable representation of pr_usrreqs.
 *
 * NOTE: When adding new ones, also add default callbacks in pru_sanitize().
 */
struct pr_usrreqs {
	uint32_t pru_flags;     /* see PRUF flags below */
	int     (*pru_abort)(struct socket *);
	int     (*pru_accept)(struct socket *, struct sockaddr **);
	int     (*pru_attach)(struct socket *, int proto, struct proc *);
	int     (*pru_bind)(struct socket *, struct sockaddr *, struct proc *);
	int     (*pru_connect)(struct socket *, struct sockaddr *,
	    struct proc *);
	int     (*pru_connect2)(struct socket *, struct socket *);
	int     (*pru_connectx)(struct socket *, struct sockaddr *,
	    struct sockaddr *, struct proc *, uint32_t,
	    sae_associd_t, sae_connid_t *, uint32_t, void *, uint32_t,
	    struct uio *, user_ssize_t *);
	int     (*pru_control)(struct socket *, u_long, caddr_t,
	    struct ifnet *, struct proc *);
	int     (*pru_detach)(struct socket *);
	int     (*pru_disconnect)(struct socket *);
	int     (*pru_disconnectx)(struct socket *,
	    sae_associd_t, sae_connid_t);
	int     (*pru_listen)(struct socket *, struct proc *);
	int     (*pru_peeraddr)(struct socket *, struct sockaddr **);
	int     (*pru_rcvd)(struct socket *, int);
	int     (*pru_rcvoob)(struct socket *, struct mbuf *, int);
	int     (*pru_send)(struct socket *, int, struct mbuf *,
	    struct sockaddr *, struct mbuf *, struct proc *);
	int     (*pru_send_list)(struct socket *, int, struct mbuf *,
	    struct sockaddr *, struct mbuf *, struct proc *);
#define PRUS_OOB        0x1
#define PRUS_EOF        0x2
#define PRUS_MORETOCOME 0x4
	int     (*pru_sense)(struct socket *, void *, int);
	int     (*pru_shutdown)(struct socket *);
	int     (*pru_sockaddr)(struct socket *, struct sockaddr **);
	int     (*pru_sopoll)(struct socket *, int, struct ucred *, void *);
	int     (*pru_soreceive)(struct socket *, struct sockaddr **,
	    struct uio *, struct mbuf **, struct mbuf **, int *);
	int     (*pru_soreceive_list)(struct socket *, struct recv_msg_elem *, u_int,
	    int *);
	int     (*pru_sosend)(struct socket *, struct sockaddr *,
	    struct uio *, struct mbuf *, struct mbuf *, int);
	int     (*pru_sosend_list)(struct socket *, struct uio **, u_int, int);
	int     (*pru_socheckopt)(struct socket *, struct sockopt *);
	int     (*pru_preconnect)(struct socket *so);
};

/* Values for pru_flags  */
#define PRUF_OLD        0x10000000      /* added via net_add_proto */

#ifdef BSD_KERNEL_PRIVATE
/*
 * For faster access than net_uptime(), bypassing the initialization.
 */
extern u_int64_t _net_uptime;
#endif /* BSD_KERNEL_PRIVATE */
#endif /* XNU_KERNEL_PRIVATE */

__BEGIN_DECLS
extern int pru_abort_notsupp(struct socket *so);
extern int pru_accept_notsupp(struct socket *so, struct sockaddr **nam);
extern int pru_attach_notsupp(struct socket *so, int proto, struct proc *p);
extern int pru_bind_notsupp(struct socket *so, struct sockaddr *nam,
    struct proc *p);
extern int pru_connect_notsupp(struct socket *so, struct sockaddr *nam,
    struct proc *p);
extern int pru_connect2_notsupp(struct socket *so1, struct socket *so2);
#ifdef XNU_KERNEL_PRIVATE
extern int pru_connectx_notsupp(struct socket *, struct sockaddr *,
    struct sockaddr *, struct proc *, uint32_t, sae_associd_t,
    sae_connid_t *, uint32_t, void *, uint32_t, struct uio *, user_ssize_t *);
extern int pru_disconnectx_notsupp(struct socket *, sae_associd_t,
    sae_connid_t);
extern int pru_socheckopt_null(struct socket *, struct sockopt *);
#endif /* XNU_KERNEL_PRIVATE */
extern int pru_control_notsupp(struct socket *so, u_long cmd, caddr_t data,
    struct ifnet *ifp, struct proc *p);
extern int pru_detach_notsupp(struct socket *so);
extern int pru_disconnect_notsupp(struct socket *so);
extern int pru_listen_notsupp(struct socket *so, struct proc *p);
extern int pru_peeraddr_notsupp(struct socket *so, struct sockaddr **nam);
extern int pru_rcvd_notsupp(struct socket *so, int flags);
extern int pru_rcvoob_notsupp(struct socket *so, struct mbuf *m, int flags);
extern int pru_send_notsupp(struct socket *so, int flags, struct mbuf *m,
    struct sockaddr *addr, struct mbuf *control, struct proc *p);
extern int pru_send_list_notsupp(struct socket *so, int flags, struct mbuf *m,
    struct sockaddr *addr, struct mbuf *control, struct proc *p);
extern int pru_sense_null(struct socket *so, void * sb, int isstat64);
extern int pru_shutdown_notsupp(struct socket *so);
extern int pru_sockaddr_notsupp(struct socket *so, struct sockaddr **nam);
extern int pru_sosend_notsupp(struct socket *so, struct sockaddr *addr,
    struct uio *uio, struct mbuf *top, struct mbuf *control, int flags);
extern int pru_sosend_list_notsupp(struct socket *so, struct uio **uio,
    u_int, int flags);
extern int pru_soreceive_notsupp(struct socket *so,
    struct sockaddr **paddr, struct uio *uio, struct mbuf **mp0,
    struct mbuf **controlp, int *flagsp);
extern int pru_soreceive_list_notsupp(struct socket *, struct recv_msg_elem *, u_int,
    int *);
extern int pru_sopoll_notsupp(struct socket *so, int events,
    struct ucred *cred, void *);
#ifdef XNU_KERNEL_PRIVATE
extern void pru_sanitize(struct pr_usrreqs *);
extern void domaininit(void);
extern void domainfin(void);
extern void pfctlinput(int, struct sockaddr *);
extern void pfctlinput2(int, struct sockaddr *, void *);
extern struct protosw *pffindproto_locked(int, int, int);
extern struct protosw *pffindprotonotype(int, int);
extern struct protosw *pffindtype(int, int);
extern struct protosw_old *pffindproto_old(int, int, int);
extern int net_add_proto(struct protosw *, struct domain *, int)
__XNU_INTERNAL(net_add_proto);
extern void net_init_proto(struct protosw *, struct domain *);
extern int net_del_proto(int, int, struct domain *)
__XNU_INTERNAL(net_del_proto);
extern int net_add_proto_old(struct protosw_old *, struct domain_old *);
extern int net_del_proto_old(int, int, struct domain_old *);
extern void net_update_uptime(void);
extern void net_update_uptime_with_time(const struct timeval *);
extern u_int64_t net_uptime(void);
extern u_int64_t net_uptime_ms(void);
extern void net_uptime2timeval(struct timeval *);
extern struct protosw *pffindproto(int family, int protocol, int type)
__XNU_INTERNAL(pffindproto);
#else
extern int net_add_proto(struct protosw *, struct domain *);
extern int net_del_proto(int, int, struct domain *);
extern struct protosw *pffindproto(int family, int protocol, int type);
#endif /* XNU_KERNEL_PRIVATE */
__END_DECLS
#endif /* KERNEL_PRIVATE */
#endif  /* !_SYS_PROTOSW_H_ */

/*
 * Copyright (c) 2000-2018 Apple Inc. All rights reserved.
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
/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
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
 *	@(#)nfsmount.h	8.3 (Berkeley) 3/30/95
 * FreeBSD-Id: nfsmount.h,v 1.13 1997/08/16 19:16:05 wollman Exp $
 */


#ifndef _NFS_NFSMOUNT_H_
#define _NFS_NFSMOUNT_H_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_PRIVATE

/*
 * NFS mount file system attributes
 */
struct nfs_fsattr {
	uint32_t        nfsa_flags;             /* file system flags */
	uint32_t        nfsa_lease;             /* lease time in seconds */
	uint32_t        nfsa_maxname;           /* maximum filename size */
	uint32_t        nfsa_maxlink;           /* maximum # links */
	uint32_t        nfsa_bsize;             /* block size */
	uint32_t        nfsa_pad;               /* UNUSED */
	uint64_t        nfsa_maxfilesize;       /* maximum file size */
	uint64_t        nfsa_maxread;           /* maximum read size */
	uint64_t        nfsa_maxwrite;          /* maximum write size */
	uint64_t        nfsa_files_avail;       /* file slots available */
	uint64_t        nfsa_files_free;        /* file slots free */
	uint64_t        nfsa_files_total;       /* file slots total */
	uint64_t        nfsa_space_avail;       /* disk space available */
	uint64_t        nfsa_space_free;        /* disk space free */
	uint64_t        nfsa_space_total;       /* disk space total */
	uint32_t        nfsa_supp_attr[NFS_ATTR_BITMAP_LEN]; /* attributes supported on this file system */
	uint32_t        nfsa_bitmap[NFS_ATTR_BITMAP_LEN]; /* valid attributes */
};
#define NFS_FSFLAG_LINK                 0x00000001
#define NFS_FSFLAG_SYMLINK              0x00000002
#define NFS_FSFLAG_UNIQUE_FH            0x00000004
#define NFS_FSFLAG_ACL                  0x00000008
#define NFS_FSFLAG_SET_TIME             0x00000010
#define NFS_FSFLAG_CASE_INSENSITIVE     0x00000020
#define NFS_FSFLAG_CASE_PRESERVING      0x00000040
#define NFS_FSFLAG_CHOWN_RESTRICTED     0x00000080
#define NFS_FSFLAG_HOMOGENEOUS          0x00000100
#define NFS_FSFLAG_NO_TRUNC             0x00000200
#define NFS_FSFLAG_NAMED_ATTR           0x00000400
#define NFS_FSFLAG_FHTYPE_MASK          0xFF000000
#define NFS_FSFLAG_FHTYPE_SHIFT         24

/*
 * NFS file system location structures
 */
struct nfs_fs_server {
	char *                  ns_name;                /* name of server */
	char **                 ns_addresses;           /* array of addresses for server */
	uint32_t                ns_addrcount;           /* # of addresses */
};
struct nfs_fs_path {
	char **                 np_components;          /* array of component pointers */
	uint32_t                np_compcount;           /* # components in path */
};
struct nfs_fs_location {
	struct nfs_fs_server ** nl_servers;             /* array of server pointers */
	struct nfs_fs_path      nl_path;                /* file system path */
	uint32_t                nl_servcount;           /* # of servers */
};

struct nfs_location_index {
	uint8_t                 nli_flags;              /* misc flags */
	uint8_t                 nli_loc;                /* location index */
	uint8_t                 nli_serv;               /* server index */
	uint8_t                 nli_addr;               /* address index */
};
#define NLI_VALID       0x01    /* index is valid */

struct nfs_fs_locations {
	struct nfs_fs_path      nl_root;                /* current server's root file system path */
	uint32_t                nl_numlocs;             /* # of locations */
	struct nfs_location_index nl_current;           /* index of current location/server/address */
	struct nfs_fs_location **nl_locations;          /* array of fs locations */
};

/*
 * RPC record marker parsing state
 */
struct nfs_rpc_record_state {
	mbuf_t                  nrrs_m;                 /* mbufs for current record */
	mbuf_t                  nrrs_mlast;
	uint16_t                nrrs_lastfrag;          /* last fragment of record */
	uint16_t                nrrs_markerleft;        /* marker bytes remaining */
	uint32_t                nrrs_fragleft;          /* fragment bytes remaining */
	uint32_t                nrrs_reclen;            /* length of RPC record */
};

/*
 * NFS socket structures
 */
struct nfs_socket {
	lck_mtx_t               nso_lock;               /* nfs socket lock */
	TAILQ_ENTRY(nfs_socket) nso_link;               /* list of sockets */
	struct sockaddr *       nso_saddr;              /* socket address */
	struct sockaddr *       nso_saddr2;             /* additional socket address */
	void *                  nso_wake;               /* address to wake up */
	time_t                  nso_timestamp;
	time_t                  nso_reqtimestamp;       /* last request sent */
	socket_t                nso_so;                 /* socket */
	uint8_t                 nso_sotype;             /* Type of socket */
	uint16_t                nso_flags;              /* NSO_* flags */
	struct nfs_location_index nso_location;         /* location index */
	uint32_t                nso_protocol;           /* RPC protocol */
	uint32_t                nso_version;            /* RPC protocol version */
	uint32_t                nso_pingxid;            /* RPC XID of NULL ping request */
	uint32_t                nso_nfs_min_vers;       /* minimum nfs version for connecting sockets */
	uint32_t                nso_nfs_max_vers;       /* maximum nfs version for connecting sockets */
	int                     nso_error;              /* saved error/status */
	struct nfs_rpc_record_state nso_rrs;            /* RPC record parsing state (TCP) */
};

TAILQ_HEAD(nfssocketlist, nfs_socket);

/* nso_flags */
#define NSO_UPCALL              0x0001                  /* socket upcall in progress */
#define NSO_DEAD                0x0002                  /* socket is dead */
#define NSO_CONNECTING          0x0004                  /* socket is being connected */
#define NSO_CONNECTED           0x0008                  /* socket connection complete */
#define NSO_PINGING             0x0010                  /* socket is being tested */
#define NSO_VERIFIED            0x0020                  /* socket appears functional */
#define NSO_DISCONNECTING       0x0040                  /* socket is being disconnected */

/* NFS connect socket search state */
struct nfs_socket_search {
	struct nfs_location_index nss_startloc;         /* starting location index */
	struct nfs_location_index nss_nextloc;          /* next location index */
	struct nfssocketlist    nss_socklist;           /* list of active sockets */
	time_t                  nss_timestamp;          /* search start time */
	time_t                  nss_last;               /* timestamp of last socket */
	struct nfs_socket *     nss_sock;               /* found socket */
	uint8_t                 nss_sotype;             /* TCP/UDP */
	uint8_t                 nss_sockcnt;            /* # of active sockets */
	in_port_t               nss_port;               /* port # to connect to */
	uint32_t                nss_protocol;           /* RPC protocol */
	uint32_t                nss_version;            /* RPC protocol version */
	uint32_t                nss_flags;              /* (see below) */
	int                     nss_addrcnt;            /* Number addresses to try or left */
	int                     nss_timeo;              /* how long we are willing to wait */
	int                     nss_error;              /* best error we've gotten so far */
};
/* nss_flags */
#define NSS_VERBOSE             0x00000001              /* OK to log info about socket search */
#define NSS_WARNED              0x00000002              /* logged warning about socket search taking a while */
#define NSS_FALLBACK2PMAP       0x00000004              /* Try V4 on NFS_PORT first, if that fails fall back to portmapper */

/*
 * function table for calling version-specific NFS functions
 */
struct nfs_funcs {
	int     (*nf_mount)(struct nfsmount *, vfs_context_t, nfsnode_t *);
	int     (*nf_update_statfs)(struct nfsmount *, vfs_context_t);
	int     (*nf_getquota)(struct nfsmount *, vfs_context_t, uid_t, int, struct dqblk *);
	int     (*nf_access_rpc)(nfsnode_t, u_int32_t *, int, vfs_context_t);
	int     (*nf_getattr_rpc)(nfsnode_t, mount_t, u_char *, size_t, int, vfs_context_t, struct nfs_vattr *, u_int64_t *);
	int     (*nf_setattr_rpc)(nfsnode_t, struct vnode_attr *, vfs_context_t);
	int     (*nf_read_rpc_async)(nfsnode_t, off_t, size_t, thread_t, kauth_cred_t, struct nfsreq_cbinfo *, struct nfsreq **);
	int     (*nf_read_rpc_async_finish)(nfsnode_t, struct nfsreq *, uio_t, size_t *, int *);
	int     (*nf_readlink_rpc)(nfsnode_t, char *, uint32_t *, vfs_context_t);
	int     (*nf_write_rpc_async)(nfsnode_t, uio_t, size_t, thread_t, kauth_cred_t, int, struct nfsreq_cbinfo *, struct nfsreq **);
	int     (*nf_write_rpc_async_finish)(nfsnode_t, struct nfsreq *, int *, size_t *, uint64_t *);
	int     (*nf_commit_rpc)(nfsnode_t, uint64_t, uint64_t, kauth_cred_t, uint64_t);
	int     (*nf_lookup_rpc_async)(nfsnode_t, char *, int, vfs_context_t, struct nfsreq **);
	int     (*nf_lookup_rpc_async_finish)(nfsnode_t, char *, int, vfs_context_t, struct nfsreq *, u_int64_t *, fhandle_t *, struct nfs_vattr *);
	int     (*nf_remove_rpc)(nfsnode_t, char *, int, thread_t, kauth_cred_t);
	int     (*nf_rename_rpc)(nfsnode_t, char *, int, nfsnode_t, char *, int, vfs_context_t);
	int     (*nf_setlock_rpc)(nfsnode_t, struct nfs_open_file *, struct nfs_file_lock *, int, int, thread_t, kauth_cred_t);
	int     (*nf_unlock_rpc)(nfsnode_t, struct nfs_lock_owner *, int, uint64_t, uint64_t, int, thread_t, kauth_cred_t);
	int     (*nf_getlock_rpc)(nfsnode_t, struct nfs_lock_owner *, struct flock *, uint64_t, uint64_t, vfs_context_t);
};

/*
 * The long form of the NFSv4 client ID.
 */
struct nfs_client_id {
	TAILQ_ENTRY(nfs_client_id)      nci_link;       /* list of client IDs */
	char                            *nci_id;        /* client id buffer */
	int                             nci_idlen;      /* length of client id buffer */
};
TAILQ_HEAD(nfsclientidlist, nfs_client_id);
extern struct nfsclientidlist nfsclientids;

/*
 * Mount structure.
 * One allocated on every NFS mount.
 * Holds NFS specific information for mount.
 */
struct nfsmount {
	lck_mtx_t nm_lock;              /* nfs mount lock */
	char *  nm_args;                /* NFS mount args (XDR) */
	uint32_t nm_mattrs[NFS_MATTR_BITMAP_LEN]; /* mount attributes in mount args */
	uint32_t nm_mflags_mask[NFS_MFLAG_BITMAP_LEN]; /* mount flags mask in mount args */
	uint32_t nm_mflags[NFS_MFLAG_BITMAP_LEN]; /* mount flags in mount args */
	uint32_t nm_flags[NFS_MFLAG_BITMAP_LEN]; /* current mount flags (soft, intr, etc...) */
	char *  nm_realm;               /* Kerberos realm to use */
	char *  nm_principal;           /* GSS principal to use on initial mount */
	char *  nm_sprinc;              /* Kerberos principal of the server */
	int     nm_ref;                 /* Reference count on this mount */
	int     nm_state;               /* Internal state flags */
	int     nm_vers;                /* NFS version */
	uint32_t nm_minor_vers;         /* minor version of above */
	uint32_t nm_min_vers;           /* minimum packed version to try */
	uint32_t nm_max_vers;           /* maximum packed version to try */
	const struct nfs_funcs *nm_funcs;/* version-specific functions */
	kauth_cred_t nm_mcred;          /* credential used for the mount */
	mount_t nm_mountp;              /* VFS structure for this filesystem */
	nfsnode_t nm_dnp;               /* root directory nfsnode pointer */
	struct nfs_fs_locations nm_locations; /* file system locations */
	uint32_t nm_numgrps;            /* Max. size of groupslist */
	TAILQ_HEAD(, nfs_gss_clnt_ctx) nm_gsscl;        /* GSS user contexts */
	uint32_t nm_ncentries;          /* GSS expired negative cache entries */
	int     nm_timeo;               /* Init timer for NFSMNT_DUMBTIMR */
	int     nm_retry;               /* Max retries */
	uint32_t nm_rsize;              /* Max size of read rpc */
	uint32_t nm_wsize;              /* Max size of write rpc */
	uint32_t nm_biosize;            /* buffer I/O size */
	uint32_t nm_readdirsize;        /* Size of a readdir rpc */
	uint32_t nm_readahead;          /* Num. of blocks to readahead */
	uint32_t nm_acregmin;           /* reg file min attr cache timeout */
	uint32_t nm_acregmax;           /* reg file max attr cache timeout */
	uint32_t nm_acdirmin;           /* dir min attr cache timeout */
	uint32_t nm_acdirmax;           /* dir max attr cache timeout */
	uint32_t nm_auth;               /* security mechanism flavor being used */
	uint32_t nm_writers;            /* Number of nodes open for writing */
	uint32_t nm_mappers;            /* Number of nodes that have mmapped */
	struct nfs_sec nm_sec;          /* acceptable security mechanism flavors */
	struct nfs_sec nm_servsec;      /* server's acceptable security mechanism flavors */
	struct nfs_etype nm_etype;      /* If using kerberos, the support session key encryption types */
	fhandle_t *nm_fh;               /* initial file handle */
	uint8_t  nm_lockmode;           /* advisory file locking mode */
	/* mount info */
	uint32_t nm_fsattrstamp;        /* timestamp for fs attrs */
	struct nfs_fsattr nm_fsattr;    /* file system attributes */
	uint64_t nm_verf;               /* v3/v4 write verifier */
	union {
		struct {                /* v2/v3 specific fields */
			TAILQ_ENTRY(nfsmount) ldlink; /* chain of mounts registered for lockd use */
			int udp_sent;   /* UDP request send count */
			int udp_cwnd;   /* UDP request congestion window */
			struct nfs_reqqhead udp_cwndq; /* requests waiting on cwnd */
			struct sockaddr *rqsaddr;/* cached rquota socket address */
			uint32_t rqsaddrstamp; /* timestamp of rquota socket address */
		} v3;
		struct {                /* v4 specific fields */
			struct nfs_client_id *longid; /* client ID, long form */
			uint64_t mounttime; /* used as client ID verifier */
			uint64_t clientid; /* client ID, short form */
			thread_call_t renew_timer; /* RENEW timer call */
			nfs_fsid fsid;  /* NFS file system id */
			TAILQ_HEAD(, nfsnode) delegations; /* list of nodes with delegations */
			TAILQ_HEAD(, nfsnode) dreturnq; /* list of nodes with delegations to return */
			TAILQ_ENTRY(nfsmount) cblink; /* chain of mounts registered for callbacks */
			uint32_t cbid;  /* callback channel identifier */
			uint32_t cbrefs; /* # callbacks using this mount */
		} v4;
	} nm_un;
	/* common state */
	TAILQ_HEAD(, nfs_open_owner) nm_open_owners; /* list of open owners */
	uint32_t nm_stateinuse;         /* state in use counter */
	uint32_t nm_stategenid;         /* state generation counter */
	time_t  nm_recover_start;       /* recover start time */
	LIST_HEAD(, nfsnode) nm_monlist; /* list of nodes being monitored */
	/* async I/O queue */
	struct nfs_reqqhead nm_resendq; /* async I/O resend queue */
	struct nfs_reqqhead nm_iodq;    /* async I/O request queue */
	struct nfsiod *nm_niod;         /* nfsiod processing this mount */
	TAILQ_ENTRY(nfsmount) nm_iodlink; /* chain of mounts awaiting nfsiod */
	int     nm_asyncwrites;         /* outstanding async I/O writes */
	/* socket state */
	uint8_t nm_sofamily;            /* (preferred) protocol family of socket */
	uint8_t nm_sotype;              /* (preferred) type of socket */
	in_port_t       nm_nfsport;     /* NFS protocol port */
	in_port_t       nm_mountport;   /* MOUNT protocol port (v2/v3) */
	char    *nm_nfs_localport;      /* Unix domain address (port) for nfs */
	char    *nm_mount_localport;    /* Unix domain address (port) for mountd */
	struct nfs_socket_search *nm_nss; /* current socket search structure */
	struct nfs_socket *nm_nso;      /* current socket */
	struct sockaddr *nm_saddr;      /* Address of server */
	u_short nm_sockflags;           /* socket state flags */
	time_t  nm_deadto_start;        /* dead timeout start time */
	time_t  nm_reconnect_start;     /* reconnect start time */
	int     nm_tprintf_initial_delay;       /* delay first "server down" */
	int     nm_tprintf_delay;       /* delay between "server down" */
	int     nm_deadtimeout;         /* delay between first "server down" and dead set at mount time */
	int     nm_curdeadtimeout;      /* current dead timeout. Adjusted by mount state and mobility */
	int     nm_srtt[4];             /* Timers for RPCs */
	int     nm_sdrtt[4];
	int     nm_timeouts;            /* Request timeouts */
	int     nm_jbreqs;              /* # R_JBTPRINTFMSG requests */
	int     nm_mounterror;          /* status of mount connect */
	TAILQ_ENTRY(nfsmount) nm_pokeq; /* mount poke queue chain */
	thread_t nm_sockthd;            /* socket thread for this mount */
};

/* macro for checking current mount flags */
#define NMFLAG(NMP, F)          NFS_BITMAP_ISSET((NMP)->nm_flags, NFS_MFLAG_ ## F)
/* macros for checking (original) mount attributes/flags */
#define NM_OMATTR_GIVEN(NMP, F) NFS_BITMAP_ISSET((NMP)->nm_mattrs, NFS_MATTR_ ## F)
#define NM_OMFLAG_GIVEN(NMP, F) NFS_BITMAP_ISSET((NMP)->nm_mflags_mask, NFS_MFLAG_ ## F)
#define NM_OMFLAG(NMP, F)       NFS_BITMAP_ISSET((NMP)->nm_mflags, NFS_MFLAG_ ## F)

/*
 * NFS mount state flags (nm_state)
 */
#define NFSSTA_MOUNT_THREAD     0x00000040  /* nfs_mount_connect_thread running */
#define NFSSTA_MONITOR_SCAN     0x00000080  /* scan of monitored nodes in progress */
#define NFSSTA_UNMOUNTING       0x00000100  /* an unmount attempt is in progress */
#define NFSSTA_NEEDSECINFO      0x00000200  /* need to fetch security info */
#define NFSSTA_CLIENTID         0x00000400  /* short client ID is valid */
#define NFSSTA_BIGCOOKIES       0x00000800  /* have seen >32bit dir cookies */
#define NFSSTA_JUKEBOXTIMEO     0x00001000  /* experienced a jukebox timeout */
#define NFSSTA_LOCKTIMEO        0x00002000  /* experienced a lock req timeout */
#define NFSSTA_MOUNTED          0x00004000  /* completely mounted */
#define NFSSTA_LOCKSWORK        0x00008000  /* lock ops have worked. */
#define NFSSTA_TIMEO            0x00010000  /* experienced a timeout. */
#define NFSSTA_FORCE            0x00020000  /* doing a forced unmount. */
#define NFSSTA_HASWRITEVERF     0x00040000  /* Has write verifier for V3 */
#define NFSSTA_GOTPATHCONF      0x00080000  /* Got the V3 pathconf info */
#define NFSSTA_GOTFSINFO        0x00100000  /* Got the V3 fsinfo */
#define NFSSTA_WANTRQUOTA       0x00200000  /* Want rquota address */
#define NFSSTA_RQUOTAINPROG     0x00400000  /* Getting rquota address */
#define NFSSTA_SENDING          0x00800000  /* Sending on socket */
#define NFSSTA_SNDLOCK          0x01000000  /* Send socket lock */
#define NFSSTA_WANTSND          0x02000000  /* Want above */
#define NFSSTA_DEAD             0x04000000  /* mount is dead */
#define NFSSTA_RECOVER          0x08000000  /* mount state needs to be recovered */
#define NFSSTA_RECOVER_EXPIRED  0x10000000  /* mount state expired */
#define NFSSTA_REVOKE           0x20000000  /* need to scan for revoked nodes */
#define NFSSTA_SQUISHY          0x40000000  /* we can ask to be forcibly unmounted */
#define NFSSTA_MOUNT_DRAIN      0x80000000  /* mount is draining references */

/* flags for nm_sockflags */
#define NMSOCK_READY            0x0001  /* socket is ready for use */
#define NMSOCK_CONNECTING       0x0002  /* socket is being connect()ed */
#define NMSOCK_SETUP            0x0004  /* socket/connection is being set up */
#define NMSOCK_UNMOUNT          0x0008  /* unmounted, no more socket activity */
#define NMSOCK_HASCONNECTED     0x0010  /* socket has connected before */
#define NMSOCK_POKE             0x0020  /* socket needs to be poked */
#define NMSOCK_DISCONNECTING    0x0080  /* socket is being disconnected */

/* aliases for version-specific fields */
#define nm_ldlink       nm_un.v3.ldlink
#define nm_sent         nm_un.v3.udp_sent
#define nm_cwnd         nm_un.v3.udp_cwnd
#define nm_cwndq        nm_un.v3.udp_cwndq
#define nm_rqproto      nm_un.v3.rqproto
#define nm_rqsaddr      nm_un.v3.rqsaddr
#define nm_rqsaddrstamp nm_un.v3.rqsaddrstamp
#define nm_longid       nm_un.v4.longid
#define nm_clientid     nm_un.v4.clientid
#define nm_mounttime    nm_un.v4.mounttime
#define nm_fsid         nm_un.v4.fsid
#define nm_renew_timer  nm_un.v4.renew_timer
#define nm_cbid         nm_un.v4.cbid
#define nm_cblink       nm_un.v4.cblink
#define nm_cbrefs       nm_un.v4.cbrefs
#define nm_delegations  nm_un.v4.delegations
#define nm_dreturnq     nm_un.v4.dreturnq

#if defined(KERNEL)
/*
 * Macros to convert from various things to mount structures.
 */
#define VFSTONFS(mp)    ((mp) ? ((struct nfsmount *)vfs_fsprivate(mp)) : NULL)
#define VTONMP(vp)      VFSTONFS(vnode_mount(vp))
#define NFSTONMP(np)    VTONMP(NFSTOV(np))
#define NFSTOMP(np)     (vnode_mount(NFSTOV(np)))

#endif /* KERNEL */

#endif /* __APPLE_API_PRIVATE */
#endif /* _NFS_NFSMOUNT_H_ */

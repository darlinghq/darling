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
 *	@(#)nfsnode.h	8.9 (Berkeley) 5/14/95
 * FreeBSD-Id: nfsnode.h,v 1.24 1997/10/28 14:06:25 bde Exp $
 */


#ifndef _NFS_NFSNODE_H_
#define _NFS_NFSNODE_H_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_PRIVATE
#ifndef _NFS_NFS_H_
#include <nfs/nfs.h>
#endif
#include <sys/kauth.h>
#include <os/refcnt.h>

/*
 * Silly rename structure that hangs off the nfsnode until the name
 * can be removed by nfs_vnop_inactive()
 */
struct nfs_sillyrename {
	kauth_cred_t    nsr_cred;
	struct nfsnode  *nsr_dnp;
	int             nsr_namlen;
	char            nsr_name[20];
};

/*
 * The nfsbuf is the nfs equivalent to a struct buf.
 */
struct nfsbuf {
	LIST_ENTRY(nfsbuf)      nb_hash;        /* hash chain */
	LIST_ENTRY(nfsbuf)      nb_vnbufs;      /* nfsnode's nfsbuf chain */
	TAILQ_ENTRY(nfsbuf)     nb_free;        /* free list position if not active. */
	volatile uint32_t       nb_flags;       /* NB_* flags. */
	volatile uint32_t       nb_lflags;      /* NBL_* flags. */
	os_refcnt_t             nb_refs;        /* outstanding references. */
	uint32_t                nb_bufsize;     /* buffer size */
	daddr64_t               nb_lblkno;      /* logical block number. */
	uint64_t                nb_verf;        /* V3 write verifier */
	int                     nb_commitlevel; /* lowest write commit level */
	time_t                  nb_timestamp;   /* buffer timestamp */
	int                     nb_error;       /* errno value. */
	u_int32_t               nb_valid;       /* valid pages in buf */
	u_int32_t               nb_dirty;       /* dirty pages in buf */
	int                     nb_validoff;    /* offset in buffer of valid region. */
	int                     nb_validend;    /* offset of end of valid region. */
	int                     nb_dirtyoff;    /* offset in buffer of dirty region. */
	int                     nb_dirtyend;    /* offset of end of dirty region. */
	int                     nb_offio;       /* offset in buffer of I/O region. */
	int                     nb_endio;       /* offset of end of I/O region. */
	int                     nb_rpcs;        /* Count of RPCs remaining for this buffer. */
	caddr_t                 nb_data;        /* mapped buffer */
	nfsnode_t               nb_np;          /* nfsnode buffer belongs to */
	kauth_cred_t            nb_rcred;       /* read credentials reference */
	kauth_cred_t            nb_wcred;       /* write credentials reference */
	void *                  nb_pagelist;    /* upl */
};

#define NFS_MAXBSIZE    (32 * PAGE_SIZE)        /* valid/dirty page masks limit buffer size */

#define NFS_A_LOT_OF_NEEDCOMMITS        256                     /* max# uncommitted buffers for a node */
#define NFS_A_LOT_OF_DELAYED_WRITES     MAX(nfsbufcnt/8,512)    /* max# "delwri" buffers in system */

/*
 * These flags are kept in b_lflags...
 * nfs_buf_mutex must be held before examining/updating
 */
#define NBL_BUSY        0x00000001      /* I/O in progress. */
#define NBL_WANTED      0x00000002      /* Process wants this buffer. */

/*
 * These flags are kept in nb_flags and they're (purposefully)
 * very similar to the B_* flags for struct buf.
 * nfs_buf_mutex is not needed to examine/update these.
 */
#define NB_STALEWVERF   0x00000001      /* write verifier changed on us */
#define NB_NEEDCOMMIT   0x00000002      /* buffer needs to be committed */
#define NB_ASYNC        0x00000004      /* Start I/O, do not wait. */
#define NB_CACHE        0x00000020      /* buffer data found in the cache */
#define NB_STABLE       0x00000040      /* write FILESYNC not UNSTABLE */
#define NB_DELWRI       0x00000080      /* delayed write: dirty range needs to be written */
#define NB_DONE         0x00000200      /* I/O completed. */
#define NB_EINTR        0x00000400      /* I/O was interrupted */
#define NB_ERROR        0x00000800      /* I/O error occurred. */
#define NB_INVAL        0x00002000      /* Does not contain valid info. */
#define NB_NCRDAHEAD    0x00004000      /* "nocache readahead" data */
#define NB_NOCACHE      0x00008000      /* Do not cache block after use. */
#define NB_WRITE        0x00000000      /* Write buffer (pseudo flag). */
#define NB_READ         0x00100000      /* Read buffer. */
#define NB_MULTASYNCRPC 0x00200000      /* multiple async RPCs issued for buffer */
#define NB_PAGELIST     0x00400000      /* Buffer describes pagelist I/O. */
#define NB_WRITEINPROG  0x01000000      /* Write in progress. */
#define NB_META         0x40000000      /* buffer contains meta-data. */

/* Flags for operation type in nfs_buf_get() */
#define NBLK_READ       0x00000001      /* buffer for read */
#define NBLK_WRITE      0x00000002      /* buffer for write */
#define NBLK_META       0x00000004      /* buffer for metadata */
#define NBLK_OPMASK     0x00000007      /* operation mask */
/* modifiers for above flags... */
#define NBLK_NOWAIT     0x40000000      /* don't wait on busy buffer */
#define NBLK_ONLYVALID  0x80000000      /* only return cached buffer */

/* These flags are used for nfsbuf iterating */
#define NBI_ITER                0x01    /* iteration in progress */
#define NBI_ITERWANT            0x02    /* waiting to iterate */
#define NBI_CLEAN               0x04    /* requesting clean buffers */
#define NBI_DIRTY               0x08    /* requesting dirty buffers */
#define NBI_NOWAIT              0x10    /* don't block on NBI_ITER */

/* Flags for nfs_buf_acquire */
#define NBAC_NOWAIT             0x01    /* Don't wait if buffer is busy */
#define NBAC_REMOVE             0x02    /* Remove from free list once buffer is acquired */

/* some convenience macros...  */
#define NBOFF(BP)                       ((off_t)(BP)->nb_lblkno * (off_t)(BP)->nb_bufsize)
#define NBPGVALID(BP, P)                 (((BP)->nb_valid >> (P)) & 0x1)
#define NBPGDIRTY(BP, P)                 (((BP)->nb_dirty >> (P)) & 0x1)
#define NBPGVALID_SET(BP, P)             ((BP)->nb_valid |= (1 << (P)))
#define NBPGDIRTY_SET(BP, P)             ((BP)->nb_dirty |= (1 << (P)))

#define NBUFSTAMPVALID(BP)              ((BP)->nb_timestamp != ~0)
#define NBUFSTAMPINVALIDATE(BP)         ((BP)->nb_timestamp = ~0)

#define NFS_BUF_MAP(BP) \
	do { \
	        if (!(BP)->nb_data && nfs_buf_map(BP)) \
	                panic("nfs_buf_map failed"); \
	} while (0)

LIST_HEAD(nfsbuflists, nfsbuf);
TAILQ_HEAD(nfsbuffreehead, nfsbuf);

extern lck_mtx_t *nfs_buf_mutex;
extern int nfsbufcnt, nfsbufmin, nfsbufmax, nfsbufmetacnt, nfsbufmetamax;
extern int nfsbuffreecnt, nfsbuffreemetacnt, nfsbufdelwricnt, nfsneedbuffer;
extern int nfs_nbdwrite;
extern struct nfsbuffreehead nfsbuffree, nfsbufdelwri;

#ifdef NFSBUFDEBUG
#define NFSBUFCNTCHK() \
	do { \
	if (	(nfsbufcnt < 0) || \
	        (nfsbufcnt > nfsbufmax) || \
	        (nfsbufmetacnt < 0) || \
	        (nfsbufmetacnt > nfsbufmetamax) || \
	        (nfsbufmetacnt > nfsbufcnt) || \
	        (nfsbuffreecnt < 0) || \
	        (nfsbuffreecnt > nfsbufmax) || \
	        (nfsbuffreecnt > nfsbufcnt) || \
	        (nfsbuffreemetacnt < 0) || \
	        (nfsbuffreemetacnt > nfsbufmax) || \
	        (nfsbuffreemetacnt > nfsbufcnt) || \
	        (nfsbuffreemetacnt > nfsbufmetamax) || \
	        (nfsbuffreemetacnt > nfsbufmetacnt) || \
	        (nfsbufdelwricnt < 0) || \
	        (nfsbufdelwricnt > nfsbufmax) || \
	        (nfsbufdelwricnt > nfsbufcnt) || \
	        (nfs_nbdwrite < 0) || \
	        (nfs_nbdwrite > nfsbufcnt) || \
	        0) \
	        panic("nfsbuf count error: max %d meta %d cnt %d meta %d free %d meta %d delwr %d bdw %d\n", \
	                nfsbufmax, nfsbufmetamax, nfsbufcnt, nfsbufmetacnt, nfsbuffreecnt, nfsbuffreemetacnt, \
	                nfsbufdelwricnt, nfs_nbdwrite); \
	} while (0)
#else
#define NFSBUFCNTCHK()
#endif

/*
 * NFS directory buffer
 *
 * Each buffer for a directory consists of:
 *
 * - a small header
 * - a packed list of direntry structures
 *   (if RDIRPLUS is enabled, a file handle and attrstamp are
 *   packed after the direntry name.)
 * - free/unused space
 * - if RDIRPLUS is enabled, an array of attributes
 *   that is indexed backwards from the end of the buffer.
 */
struct nfs_dir_buf_header {
	uint16_t        ndbh_flags;     /* flags (see below) */
	uint16_t        ndbh_count;     /* # of entries */
	uint32_t        ndbh_entry_end; /* end offset of direntry data */
	uint32_t        ndbh_ncgen;     /* name cache generation# */
	uint32_t        ndbh_pad;       /* reserved */
};
/* ndbh_flags */
#define NDB_FULL        0x0001  /* buffer has been filled */
#define NDB_EOF         0x0002  /* buffer contains EOF */
#define NDB_PLUS        0x0004  /* buffer contains RDIRPLUS data */

#define NFS_DIR_BUF_FIRST_DIRENTRY(BP) \
	((struct direntry*)((char*)((BP)->nb_data) + sizeof(*ndbhp)))
#define NFS_DIR_BUF_NVATTR(BP, IDX) \
	(&((struct nfs_vattr*)((char*)((BP)->nb_data) + (BP)->nb_bufsize))[-((IDX)+1)])
#define NFS_DIRENTRY_LEN(namlen) \
	((sizeof(struct direntry) + (namlen) - (MAXPATHLEN-1) + 7) & ~7)
#define NFS_DIRENT_LEN(namlen) \
	((sizeof(struct dirent) - (NAME_MAX+1)) + (((namlen) + 1 + 3) &~ 3))
#define NFS_DIRENTRY_NEXT(DP) \
	((struct direntry*)((char*)(DP) + (DP)->d_reclen))
#define NFS_DIR_COOKIE_POTENTIALLY_TRUNCATED(C) \
	((C) && ((((C) >> 32) == 0) || (((C) & 0x80000000ULL) && (((C) >> 32) == 0xffffffff))))
#define NFS_DIR_COOKIE_SAME32(C1, C2) \
	(((C1) & 0xffffffffULL) == ((C2) & 0xffffffffULL))

/*
 * NFS directory cookie cache
 *
 * This structure is used to cache cookie-to-buffer mappings for
 * cookies recently returned from READDIR.  The entries are kept in an
 * array.  The most-recently-used (MRU) list is headed by the entry at
 * index "mru".  The index of the next entry in the list is kept in the
 * "next" array.  (An index value of -1 marks an invalid entry.)
 */
#define NFSNUMCOOKIES           14
struct nfsdmap {
	int8_t          free;                   /* next unused slot */
	int8_t          mru;                    /* head of MRU list */
	int8_t          next[NFSNUMCOOKIES];    /* MRU list links */
	struct {
		uint64_t    key;                /* cookie */
		uint64_t    lbn;                /* lbn of buffer */
	} cookies[NFSNUMCOOKIES];               /* MRU list entries */
};

/*
 * NFS vnode attribute structure
 */
#define NFSTIME_ACCESS  0       /* time of last access */
#define NFSTIME_MODIFY  1       /* time of last modification */
#define NFSTIME_CHANGE  2       /* time file changed */
#define NFSTIME_CREATE  3       /* time file created */
#define NFSTIME_BACKUP  4       /* time of last backup */
#define NFSTIME_ADDED   5       /* time added (FPnfs only) */
#define NFSTIME_COUNT   6

#define NFS_COMPARE_MTIME(TVP, NVAP, CMP) \
	(((TVP)->tv_sec == (NVAP)->nva_timesec[NFSTIME_MODIFY]) ?       \
	 ((TVP)->tv_nsec CMP (NVAP)->nva_timensec[NFSTIME_MODIFY]) :    \
	 ((TVP)->tv_sec CMP (NVAP)->nva_timesec[NFSTIME_MODIFY]))
#define NFS_COPY_TIME(TVP, NVAP, WHICH) \
	do { \
	(TVP)->tv_sec = (NVAP)->nva_timesec[NFSTIME_##WHICH]; \
	(TVP)->tv_nsec = (NVAP)->nva_timensec[NFSTIME_##WHICH]; \
	} while (0)

struct nfs_vattr {
	enum vtype      nva_type;       /* vnode type (for create) */
	uint32_t        nva_mode;       /* file's access mode (and type) */
	uid_t           nva_uid;        /* owner user id */
	gid_t           nva_gid;        /* owner group id */
	guid_t          nva_uuuid;      /* owner user UUID */
	guid_t          nva_guuid;      /* owner group UUID */
	kauth_acl_t     nva_acl;        /* access control list */
	nfs_specdata    nva_rawdev;     /* device the special file represents */
	uint32_t        nva_flags;      /* file flags (see below) */
	uint32_t        nva_maxlink;    /* maximum # of links (v4) */
	uint64_t        nva_nlink;      /* number of references to file */
	uint64_t        nva_fileid;     /* file id */
	nfs_fsid        nva_fsid;       /* file system id */
	uint64_t        nva_size;       /* file size in bytes */
	uint64_t        nva_bytes;      /* bytes of disk space held by file */
	uint64_t        nva_change;     /* change attribute */
	int64_t         nva_timesec[NFSTIME_COUNT];
	int32_t         nva_timensec[NFSTIME_COUNT];
	uint32_t        nva_bitmap[NFS_ATTR_BITMAP_LEN]; /* attributes that are valid */

	/* FPnfs only. */
	uint32_t        nva_bsd_flags;  /* BSD flags */
	uint64_t        nva_parentid;   /* parent file id */
	uint64_t        nva_allocsize;  /* size allocated on disk */
};

/* nva_flags */
#define NFS_FFLAG_ARCHIVED              0x0001
#define NFS_FFLAG_HIDDEN                0x0002
#define NFS_FFLAG_HAS_NAMED_ATTRS       0x0004  /* file has named attributes */
#define NFS_FFLAG_TRIGGER               0x0008  /* node is a trigger/mirror mount point */
#define NFS_FFLAG_TRIGGER_REFERRAL      0x0010  /* trigger is a referral */
#define NFS_FFLAG_IS_ATTR               0x8000  /* file is a named attribute file/directory */
/* FPnfs only */
#define NFS_FFLAG_FPNFS_BSD_FLAGS   0x01000000
#define NFS_FFLAG_FPNFS_PARENTID    0x02000000
#define NFS_FFLAG_FPNFS_ADDEDTIME   0x04000000

/* flags for nfs_getattr() */
#define NGA_CACHED      0x0001  /* use cached attributes (if still valid) */
#define NGA_UNCACHED    0x0002  /* fetch new attributes */
#define NGA_ACL         0x0004  /* fetch ACL */
#define NGA_MONITOR     0x0008  /* vnode monitor attr update poll */
#define NGA_SOFT        0x0010  /* use cached attributes if ETIMEOUT */

/* macros for initting/cleaning up nfs_vattr structures */
#define NVATTR_INIT(NVAP) \
	do { \
	        NFS_CLEAR_ATTRIBUTES((NVAP)->nva_bitmap); \
	        (NVAP)->nva_flags = 0; \
	        (NVAP)->nva_acl = NULL; \
	} while (0)
#define NVATTR_CLEANUP(NVAP) \
	do { \
	        NFS_CLEAR_ATTRIBUTES((NVAP)->nva_bitmap); \
	        if ((NVAP)->nva_acl) { \
	                kauth_acl_free((NVAP)->nva_acl); \
	                (NVAP)->nva_acl = NULL; \
	        } \
	} while (0)

/*
 * macros for detecting node changes
 *
 * These macros help us determine if a file has been changed on the server and
 * thus whether or not we need to invalidate any cached data.
 *
 * For NFSv2/v3, the modification time is used.
 * For NFSv4, the change attribute is used.
 */
#define NFS_CHANGED(VERS, NP, NVAP) \
	        (((VERS) >= NFS_VER4) ? \
	                ((NP)->n_change != (NVAP)->nva_change) : \
	                NFS_COMPARE_MTIME(&(NP)->n_mtime, (NVAP), !=))
#define NFS_CHANGED_NC(VERS, NP, NVAP) \
	        (((VERS) >= NFS_VER4) ? \
	                ((NP)->n_ncchange != (NVAP)->nva_change) : \
	                NFS_COMPARE_MTIME(&(NP)->n_ncmtime, (NVAP), !=))
#define NFS_CHANGED_UPDATE(VERS, NP, NVAP) \
	do { \
	        if ((VERS) >= NFS_VER4) \
	                (NP)->n_change = (NVAP)->nva_change; \
	        else \
	                NFS_COPY_TIME(&(NP)->n_mtime, (NVAP), MODIFY); \
	} while (0)
#define NFS_CHANGED_UPDATE_NC(VERS, NP, NVAP) \
	do { \
	        if ((VERS) >= NFS_VER4) \
	                (NP)->n_ncchange = (NVAP)->nva_change; \
	        else \
	                NFS_COPY_TIME(&(NP)->n_ncmtime, (NVAP), MODIFY); \
	} while (0)


extern lck_grp_t *nfs_open_grp;
extern uint32_t nfs_open_owner_seqnum, nfs_lock_owner_seqnum;

/*
 * NFSv4 open owner structure - one per cred per mount
 */
struct nfs_open_owner {
	TAILQ_ENTRY(nfs_open_owner)     noo_link;       /* List of open owners (on mount) */
	lck_mtx_t                       noo_lock;       /* owner mutex */
	struct nfsmount *               noo_mount;      /* NFS mount */
	os_refcnt_t                     noo_refcnt;     /* # outstanding references */
	uint32_t                        noo_flags;      /* see below */
	kauth_cred_t                    noo_cred;       /* credentials of open owner */
	uint32_t                        noo_name;       /* unique name used otw */
	uint32_t                        noo_seqid;      /* client-side sequence ID */
	TAILQ_HEAD(, nfs_open_file)      noo_opens;      /* list of open files */
};
/* noo_flags */
#define NFS_OPEN_OWNER_LINK     0x1     /* linked into mount's open owner list */
#define NFS_OPEN_OWNER_BUSY     0x2     /* open state-modifying operation in progress */
#define NFS_OPEN_OWNER_WANT     0x4     /* someone else wants to mark busy */

/*
 * NFS open file structure - one per open owner per nfsnode
 */
struct nfs_open_file {
	lck_mtx_t                       nof_lock;               /* open file mutex */
	TAILQ_ENTRY(nfs_open_file)      nof_link;               /* list of open files */
	TAILQ_ENTRY(nfs_open_file)      nof_oolink;             /* list of open owner's open files */
	struct nfs_open_owner *         nof_owner;              /* open owner */
	nfsnode_t                       nof_np;                 /* nfsnode this open is for */
	nfs_stateid                     nof_stateid;            /* open stateid */
	thread_t                        nof_creator;            /* thread that created file */
	uint32_t                        nof_opencnt;            /* open file count */
	uint16_t                        nof_flags;              /* see below */
	uint8_t                         nof_access:4;           /* access mode for this open */
	uint8_t                         nof_deny:4;             /* deny mode for this open */
	uint8_t                         nof_mmap_access:4;      /* mmap open access mode */
	uint8_t                         nof_mmap_deny:4;        /* mmap open deny mode */
	/* counts of access/deny mode open combinations */
	uint32_t                        nof_r;                  /* read opens (deny none) */
	uint32_t                        nof_w;                  /* write opens (deny none) */
	uint32_t                        nof_rw;                 /* read/write opens (deny none) */
	uint32_t                        nof_r_dw;               /* read deny-write opens */
	/* the rest of the counts have a max of 2 (1 for open + 1 for mmap) */
	uint32_t                        nof_w_dw:2;             /* write deny-write opens (max 2) */
	uint32_t                        nof_rw_dw:2;            /* read/write deny-write opens (max 2) */
	uint32_t                        nof_r_drw:2;            /* read deny-read/write opens (max 2) */
	uint32_t                        nof_w_drw:2;            /* write deny-read/write opens (max 2) */
	uint32_t                        nof_rw_drw:2;           /* read/write deny-read/write opens (max 2) */
	/* counts of DELEGATED access/deny mode open combinations */
	uint32_t                        nof_d_w_dw:2;           /* write deny-write opens (max 2) */
	uint32_t                        nof_d_rw_dw:2;          /* read/write deny-write opens (max 2) */
	uint32_t                        nof_d_r_drw:2;          /* read deny-read/write opens (max 2) */
	uint32_t                        nof_d_w_drw:2;          /* write deny-read/write opens (max 2) */
	uint32_t                        nof_d_rw_drw:2;         /* read/write deny-read/write opens (max 2) */
	uint32_t                        nof_d_r;                /* read opens (deny none) */
	uint32_t                        nof_d_w;                /* write opens (deny none) */
	uint32_t                        nof_d_rw;               /* read/write opens (deny none) */
	uint32_t                        nof_d_r_dw;             /* read deny-write opens */
};
/* nof_flags */
#define NFS_OPEN_FILE_BUSY      0x0001  /* open state-modifying operation in progress */
#define NFS_OPEN_FILE_WANT      0x0002  /* someone else wants to mark busy */
#define NFS_OPEN_FILE_CREATE    0x0004  /* has an open(RW) from a "CREATE" call */
#define NFS_OPEN_FILE_NEEDCLOSE 0x0008  /* has an open(R) from an (unopen) VNOP_READ or VNOP_MMAP call */
#define NFS_OPEN_FILE_SETATTR   0x0020  /* has an open(W) to perform a SETATTR(size) */
#define NFS_OPEN_FILE_POSIXLOCK 0x0040  /* server supports POSIX locking semantics */
#define NFS_OPEN_FILE_LOST      0x0080  /* open state has been lost */
#define NFS_OPEN_FILE_REOPEN    0x0100  /* file needs to be reopened */
#define NFS_OPEN_FILE_REOPENING 0x0200  /* file is being reopened */

struct nfs_lock_owner;
/*
 * NFS file lock
 *
 * Each lock request (pending or granted) has an
 * nfs_file_lock structure representing its state.
 */
struct nfs_file_lock {
	TAILQ_ENTRY(nfs_file_lock)      nfl_link;       /* List of locks on nfsnode */
	TAILQ_ENTRY(nfs_file_lock)      nfl_lolink;     /* List of locks held by locker */
	struct nfs_lock_owner *         nfl_owner;      /* lock owner that holds this lock */
	uint64_t                        nfl_start;      /* starting offset */
	uint64_t                        nfl_end;        /* ending offset (inclusive) */
	uint32_t                        nfl_blockcnt;   /* # locks blocked on this lock */
	uint16_t                        nfl_flags;      /* see below */
	uint8_t                         nfl_type;       /* lock type: read/write */
};
/* nfl_flags */
#define NFS_FILE_LOCK_ALLOC             0x01    /* lock was allocated */
#define NFS_FILE_LOCK_STYLE_POSIX       0x02    /* POSIX-style fcntl() lock */
#define NFS_FILE_LOCK_STYLE_FLOCK       0x04    /* flock(2)-style lock */
#define NFS_FILE_LOCK_STYLE_MASK        0x06    /* lock style mask */
#define NFS_FILE_LOCK_WAIT              0x08    /* may block on conflicting locks */
#define NFS_FILE_LOCK_BLOCKED           0x10    /* request is blocked */
#define NFS_FILE_LOCK_DEAD              0x20    /* lock (request) no longer exists */
#define NFS_FILE_LOCK_DELEGATED         0x40    /* lock acquired via delegation */

TAILQ_HEAD(nfs_file_lock_queue, nfs_file_lock);

/*
 * Calculate length of lock range given the endpoints.
 * Note that struct flock has "to EOF" reported as 0 but
 * the NFSv4 protocol has "to EOF" reported as UINT64_MAX.
 */
#define NFS_FLOCK_LENGTH(S, E)  (((E) == UINT64_MAX) ? 0 : ((E) - (S) + 1))
#define NFS_LOCK_LENGTH(S, E)   (((E) == UINT64_MAX) ? UINT64_MAX : ((E) - (S) + 1))

/*
 * NFSv4 lock owner structure - per open owner per process per nfsnode
 *
 * A lock owner is a process + an nfsnode.
 *
 * Note that flock(2) locks technically should have the lock owner be
 * an fglob pointer instead of a process.  However, implementing that
 * correctly would not be trivial.  So, for now, flock(2) locks are
 * essentially treated like whole-file POSIX locks.
 */
struct nfs_lock_owner {
	lck_mtx_t                       nlo_lock;       /* owner mutex */
	TAILQ_ENTRY(nfs_lock_owner)     nlo_link;       /* List of lock owners (on nfsnode) */
	struct nfs_open_owner *         nlo_open_owner; /* corresponding open owner */
	struct nfs_file_lock_queue      nlo_locks;      /* list of locks held */
	struct nfs_file_lock            nlo_alock;      /* most lockers will only ever have one */
	struct timeval                  nlo_pid_start;  /* Start time of process id */
	pid_t                           nlo_pid;        /* lock-owning process ID */
	os_refcnt_t                     nlo_refcnt;     /* # outstanding references */
	uint32_t                        nlo_flags;      /* see below */
	uint32_t                        nlo_name;       /* unique name used otw */
	uint32_t                        nlo_seqid;      /* client-side sequence ID */
	uint32_t                        nlo_stategenid; /* mount state generation ID */
	nfs_stateid                     nlo_stateid;    /* lock stateid */
};
/* nlo_flags */
#define NFS_LOCK_OWNER_LINK     0x1     /* linked into mount's lock owner list */
#define NFS_LOCK_OWNER_BUSY     0x2     /* lock state-modifying operation in progress */
#define NFS_LOCK_OWNER_WANT     0x4     /* someone else wants to mark busy */

/*
 * The nfsnode is the NFS equivalent of an inode.
 * There is a unique nfsnode for each NFS vnode.
 * An nfsnode is 'named' by its file handle. (nget/nfs_node.c)
 * NB: Hopefully the current order of the fields is such that everything will
 *     be well aligned and, therefore, tightly packed.
 */

#define NFS_ACCESS_CACHE_SIZE   3

struct nfsnode {
	lck_mtx_t               n_lock;         /* nfs node lock */
	lck_rw_t                n_datalock;     /* nfs node data lock */
	void                    *n_datalockowner;/* nfs node data lock owner (exclusive) */
	LIST_ENTRY(nfsnode)     n_hash;         /* Hash chain */
	LIST_ENTRY(nfsnode)     n_monlink;      /* list of monitored nodes */
	u_quad_t                n_size;         /* Current size of file */
	u_quad_t                n_newsize;      /* new size of file (pending update) */
	u_int64_t               n_xid;          /* last xid to loadattr */
	struct nfs_vattr        n_vattr;        /* Vnode attribute cache */
	time_t                  n_attrstamp;    /* Attr. cache timestamp */
	time_t                  n_aclstamp;     /* ACL cache timestamp */
	time_t                  n_evtstamp;     /* last vnode event timestamp */
	uint32_t                n_events;       /* pending vnode events */
	u_int8_t                n_access[NFS_ACCESS_CACHE_SIZE + 1];      /* ACCESS cache */
	uid_t                   n_accessuid[NFS_ACCESS_CACHE_SIZE];     /* credentials having access */
	time_t                  n_accessstamp[NFS_ACCESS_CACHE_SIZE];   /* access cache timestamp */
	union {
		struct {
			struct timespec n3_mtime; /* Prev modify time. */
			struct timespec n3_ncmtime; /* namecache modify time. */
		} v3;
		struct {
			uint64_t        n4_change;/* prev change attribute */
			uint64_t        n4_ncchange;/* namecache change attribute */
			u_char          *n4_attrdirfh;/* associated attr directory fh */
			struct timeval  n4_lastio;/* time of most recent I/O on attr */
		} v4;
	} n_un4;
	vnode_t                 n_parent;       /* this node's parent */
	u_char                  *n_fhp;         /* NFS File Handle */
	vnode_t                 n_vnode;        /* associated vnode */
	mount_t                 n_mount;        /* associated mount (NHINIT) */
	int                     n_error;        /* Save write error value */
	union {
		struct timespec ns_atim;        /* Special file times */
		struct timespec nl_rltim;       /* Time of last readlink */
		daddr64_t       nf_lastread;    /* last block# read from (for readahead) */
		uint64_t        nd_cookieverf;  /* Cookie verifier (dir only) */
	} n_un1;
	union {
		struct timespec ns_mtim;        /* Special file times */
		daddr64_t       nf_lastrahead;  /* last block# read ahead */
		uint64_t        nd_eofcookie;   /* Dir. EOF cookie cache */
	} n_un2;
	union {
		struct nfs_sillyrename *nf_silly;/* Ptr to silly rename struct */
		struct nfsdmap *nd_cookiecache; /* dir cookie cache */
	} n_un3;
	uint32_t                n_flag;         /* node flags */
	u_short                 n_fhsize;       /* size in bytes, of fh */
	u_short                 n_hflag;        /* node hash flags */
	u_short                 n_bflag;        /* node buffer flags */
	u_short                 n_mflag;        /* node mount flags */
	u_char                  n_fh[NFS_SMALLFH];/* Small File Handle */
	uint32_t                n_auth;         /* security flavor used for this node */
	struct nfsbuflists      n_cleanblkhd;   /* clean blocklist head */
	struct nfsbuflists      n_dirtyblkhd;   /* dirty blocklist head */
	union {
		int             nf_wrbusy;      /* # threads in write/fsync */
		uint32_t        nd_ncgen;       /* dir name cache generation# */
	} n_un5;
	union {
		int             nf_needcommitcnt;/* # bufs that need committing */
		daddr64_t       nd_lastdbl;     /* last dir buf lookup block# */
	} n_un6;
	int                     n_bufiterflags; /* buf iterator flags */
	union {
		int             nf_numoutput;   /* write I/Os in progress */
		int             nd_trigseq;     /* vnode trigger seq# */
	} n_un7;
	/* open state */
	lck_mtx_t               n_openlock;     /* nfs node open lock */
	uint32_t                n_openflags;    /* open state flags */
	uint32_t                n_openrefcnt;   /* # non-file opens */
	TAILQ_HEAD(, nfs_open_file) n_opens;     /* list of open files */
	/* lock state */
	TAILQ_HEAD(, nfs_lock_owner) n_lock_owners; /* list of lock owners */
	struct nfs_file_lock_queue n_locks;     /* list of locks */
	/* delegation state */
	nfs_stateid             n_dstateid;     /* delegation stateid */
	TAILQ_ENTRY(nfsnode)    n_dlink;        /* delegation list link */
	TAILQ_ENTRY(nfsnode)    n_dreturn;      /* delegation return list link */
	struct kauth_ace        n_dace;         /* delegation ACE */
};

#define NFS_DATA_LOCK_SHARED    1
#define NFS_DATA_LOCK_EXCLUSIVE 2

#define nfstimespeccmp(tvp, uvp, cmp)           \
	(((tvp)->tv_sec == (uvp)->tv_sec) ?     \
	 ((tvp)->tv_nsec cmp (uvp)->tv_nsec) :  \
	 ((tvp)->tv_sec cmp (uvp)->tv_sec))

#define CHECK_NEEDCOMMITCNT(np) \
	do { \
	        if ((np)->n_needcommitcnt < 0) { \
	                printf("nfs: n_needcommitcnt negative\n"); \
	                (np)->n_needcommitcnt = 0; \
	        } \
	} while (0)

#define n_atim                  n_un1.ns_atim
#define n_mtim                  n_un2.ns_mtim
#define n_rltim                 n_un1.nl_rltim
#define n_lastread              n_un1.nf_lastread
#define n_lastrahead            n_un2.nf_lastrahead
#define n_sillyrename           n_un3.nf_silly
#define n_wrbusy                n_un5.nf_wrbusy
#define n_needcommitcnt         n_un6.nf_needcommitcnt
#define n_numoutput             n_un7.nf_numoutput
#define n_cookieverf            n_un1.nd_cookieverf
#define n_eofcookie             n_un2.nd_eofcookie
#define n_cookiecache           n_un3.nd_cookiecache
#define n_ncgen                 n_un5.nd_ncgen
#define n_lastdbl               n_un6.nd_lastdbl
#define n_trigseq               n_un7.nd_trigseq
#define n_mtime                 n_un4.v3.n3_mtime
#define n_ncmtime               n_un4.v3.n3_ncmtime
#define n_change                n_un4.v4.n4_change
#define n_ncchange              n_un4.v4.n4_ncchange
#define n_attrdirfh             n_un4.v4.n4_attrdirfh
#define n_lastio                n_un4.v4.n4_lastio

/*
 * Flags for n_flag
 */
#define NUPDATESIZE     0x00001 /* size of file needs updating */
#define NREVOKE         0x00002 /* node revoked */
#define NMODIFIED       0x00004 /* Might have a modified buffer in bio */
#define NWRITEERR       0x00008 /* Flag write errors so close will know */
#define NNEEDINVALIDATE 0x00010 /* need to call vinvalbuf() */
#define NGETATTRINPROG  0x00020 /* GETATTR RPC in progress */
#define NGETATTRWANT    0x00040 /* waiting for GETATTR RPC */
#define NACC            0x00100 /* Special file accessed */
#define NUPD            0x00200 /* Special file updated */
#define NCHG            0x00400 /* Special file times changed */
#define NNEGNCENTRIES   0x00800 /* directory has negative name cache entries */
#define NBUSY           0x01000 /* node is busy */
#define NBUSYWANT       0x02000 /* waiting on busy node */
#define NISDOTZFS       0x04000 /* a ".zfs" directory */
#define NISDOTZFSCHILD  0x08000 /* a child of a ".zfs" directory */
#define NISMAPPED       0x10000 /* node is mmapped   */
#define NREFRESH        0x20000 /* node's fh needs to be refreshed */
#define NREFRESHWANT    0x40000 /* Waiting for fh to be refreshed */
#define NDISARMTRIGGER  0x80000 /* Ignore node's mirror mount trigger */

/*
 * Flags for n_hflag
 * Note: protected by nfs_node_hash_mutex
 */
#define NHHASHED        0x0001  /* node is in hash table */
#define NHINIT          0x0002  /* node is being initialized */
#define NHLOCKED        0x0004  /* node is locked (initting or deleting) */
#define NHLOCKWANT      0x0008  /* someone wants to lock */

/*
 * Flags for n_bflag
 * Note: protected by nfs_buf_mutex
 */
#define NBFLUSHINPROG   0x0001  /* Avoid multiple calls to nfs_flush() */
#define NBFLUSHWANT     0x0002  /* waiting for nfs_flush() to complete */
#define NBINVALINPROG   0x0004  /* Avoid multiple calls to nfs_vinvalbuf() */
#define NBINVALWANT     0x0008  /* waiting for nfs_vinvalbuf() to complete */

/*
 * Flags for n_mflag
 * Note: protected by nfsmount's nm_lock
 */
#define NMMONSCANINPROG 0x0001  /* monitored node is currently updating attributes */
#define NMMONSCANWANT   0x0002  /* waiting for attribute update to complete */

/*
 * n_openflags
 * Note: protected by n_openlock
 */
#define N_OPENBUSY              0x0001  /* open state is busy - being updated */
#define N_OPENWANT              0x0002  /* someone wants to mark busy */
#define N_DELEG_READ            0x0004  /* we have a read delegation */
#define N_DELEG_WRITE           0x0008  /* we have a write delegation */
#define N_DELEG_MASK            0x000c  /* delegation mask */
#define N_DELEG_RETURN          0x0010  /* delegation queued for return */
#define N_DELEG_RETURNING       0x0020  /* delegation being returned */

/* attr/access/ACL cache timestamp macros */
#define NATTRVALID(np)          ((np)->n_attrstamp != ~0)
#define NATTRINVALIDATE(np)     ((np)->n_attrstamp = ~0)
#define NACCESSVALID(np, slot)  (((slot) >= 0) && ((slot) < NFS_ACCESS_CACHE_SIZE) && ((np)->n_accessstamp[(slot)] != ~0))
#define NACCESSINVALIDATE(np) \
	do { \
	        int __i; \
	        for (__i=0; __i < NFS_ACCESS_CACHE_SIZE; __i++) \
	                (np)->n_accessstamp[__i] = ~0; \
	        (np)->n_access[NFS_ACCESS_CACHE_SIZE] = 0; \
	} while (0)
#define NACLVALID(np)           ((np)->n_aclstamp != ~0)
#define NACLINVALIDATE(np)      ((np)->n_aclstamp = ~0)

/*
 * NFS-specific flags for nfs_vinvalbuf/nfs_flush
 */
#define V_IGNORE_WRITEERR       0x8000

/*
 * Flags for nfs_nget()
 */
#define NG_MARKROOT     0x0001  /* mark vnode as root of FS */
#define NG_MAKEENTRY    0x0002  /* add name cache entry for vnode */
#define NG_NOCREATE     0x0004  /* don't create a new node, return existing one */

/*
 * Convert between nfsnode pointers and vnode pointers
 */
#define VTONFS(vp)      ((nfsnode_t)vnode_fsnode(vp))
#define NFSTOV(np)      ((np)->n_vnode)

/* nfsnode hash table mutex */
extern lck_mtx_t *nfs_node_hash_mutex;

/*
 * printf-like helper macro that also outputs node name.
 */
#define NP(NP, FMT, ...) \
	do { \
	        const char *__vname = (NP) ? vnode_getname(NFSTOV(NP)) : NULL; \
	        printf(FMT " %s\n", ##__VA_ARGS__, __vname ? __vname : "???"); \
	        if (__vname) vnode_putname(__vname); \
	} while (0)

/*
 * nfsiod structures
 */
struct nfsiod {
	TAILQ_ENTRY(nfsiod)     niod_link;      /* List of nfsiods */
	struct nfsmount *       niod_nmp;       /* mount point for this nfsiod */
};
TAILQ_HEAD(nfsiodlist, nfsiod);
TAILQ_HEAD(nfsiodmountlist, nfsmount);
extern struct nfsiodlist nfsiodfree, nfsiodwork;
extern struct nfsiodmountlist nfsiodmounts;
extern lck_mtx_t *nfsiod_mutex;

#if defined(KERNEL)

typedef int     vnop_t(void *);
extern  vnop_t  **fifo_nfsv2nodeop_p;
extern  vnop_t  **nfsv2_vnodeop_p;
extern  vnop_t  **fpnfs_vnodeop_p;
extern  vnop_t  **spec_nfsv2nodeop_p;
#if CONFIG_NFS4
extern  vnop_t  **fifo_nfsv4nodeop_p;
extern  vnop_t  **nfsv4_vnodeop_p;
extern  vnop_t  **spec_nfsv4nodeop_p;
#endif
/*
 * Prototypes for NFS vnode operations
 */
#define nfs_vnop_revoke nop_revoke
int     nfs_vnop_inactive(struct vnop_inactive_args *);
int     nfs_vnop_reclaim(struct vnop_reclaim_args *);

int nfs_node_lock(nfsnode_t);
int nfs_node_lock_internal(nfsnode_t, int);
void nfs_node_lock_force(nfsnode_t);
void nfs_node_unlock(nfsnode_t);
int nfs_node_lock2(nfsnode_t, nfsnode_t);
void nfs_node_unlock2(nfsnode_t, nfsnode_t);
int nfs_node_set_busy(nfsnode_t, thread_t);
int nfs_node_set_busy2(nfsnode_t, nfsnode_t, thread_t);
int nfs_node_set_busy4(nfsnode_t, nfsnode_t, nfsnode_t, nfsnode_t, thread_t);
void nfs_node_clear_busy(nfsnode_t);
void nfs_node_clear_busy2(nfsnode_t, nfsnode_t);
void nfs_node_clear_busy4(nfsnode_t, nfsnode_t, nfsnode_t, nfsnode_t);
void nfs_data_lock(nfsnode_t, int);
void nfs_data_lock_noupdate(nfsnode_t, int);
void nfs_data_lock_internal(nfsnode_t, int, int);
void nfs_data_unlock(nfsnode_t);
void nfs_data_unlock_noupdate(nfsnode_t);
void nfs_data_unlock_internal(nfsnode_t, int);
void nfs_data_update_size(nfsnode_t, int);

/* other stuff */
int nfs_removeit(struct nfs_sillyrename *);
int nfs_nget(mount_t, nfsnode_t, struct componentname *, u_char *, int, struct nfs_vattr *, u_int64_t *, uint32_t, int, nfsnode_t*);
int nfs_mount_is_dirty(mount_t);
void nfs_dir_cookie_cache(nfsnode_t, uint64_t, uint64_t);
int nfs_dir_cookie_to_lbn(nfsnode_t, uint64_t, int *, uint64_t *);
void nfs_invaldir(nfsnode_t);
uint32_t nfs_dir_buf_freespace(struct nfsbuf *, int);

/* nfsbuf functions */
void nfs_nbinit(void);
void nfs_buf_timer(void *, void *);
void nfs_buf_remfree(struct nfsbuf *);
boolean_t nfs_buf_is_incore(nfsnode_t, daddr64_t);
struct nfsbuf * nfs_buf_incore(nfsnode_t, daddr64_t);
int nfs_buf_get(nfsnode_t, daddr64_t, uint32_t, thread_t, int, struct nfsbuf **);
int nfs_buf_upl_setup(struct nfsbuf *bp);
void nfs_buf_upl_check(struct nfsbuf *bp);
void nfs_buf_normalize_valid_range(nfsnode_t, struct nfsbuf *);
int nfs_buf_map(struct nfsbuf *);
void nfs_buf_release(struct nfsbuf *, int);
int nfs_buf_iowait(struct nfsbuf *);
void nfs_buf_iodone(struct nfsbuf *);
void nfs_buf_write_delayed(struct nfsbuf *);
void nfs_buf_check_write_verifier(nfsnode_t, struct nfsbuf *);
void nfs_buf_freeup(int);
void nfs_buf_refget(struct nfsbuf *bp);
void nfs_buf_refrele(struct nfsbuf *bp);
void nfs_buf_drop(struct nfsbuf *);
errno_t nfs_buf_acquire(struct nfsbuf *, int, int, int);
int nfs_buf_iterprepare(nfsnode_t, struct nfsbuflists *, int);
void nfs_buf_itercomplete(nfsnode_t, struct nfsbuflists *, int);

int nfs_bioread(nfsnode_t, uio_t, int, vfs_context_t);
int nfs_buf_readahead(nfsnode_t, int, daddr64_t *, daddr64_t, thread_t, kauth_cred_t);
int nfs_buf_readdir(struct nfsbuf *, vfs_context_t);
int nfs_buf_read(struct nfsbuf *);
void nfs_buf_read_finish(struct nfsbuf *);
int nfs_buf_read_rpc(struct nfsbuf *, thread_t, kauth_cred_t);
void nfs_buf_read_rpc_finish(struct nfsreq *);
int nfs_buf_write(struct nfsbuf *);
void nfs_buf_write_finish(struct nfsbuf *, thread_t, kauth_cred_t);
int nfs_buf_write_rpc(struct nfsbuf *, int, thread_t, kauth_cred_t);
void nfs_buf_write_rpc_finish(struct nfsreq *);
int nfs_buf_write_dirty_pages(struct nfsbuf *, thread_t, kauth_cred_t);

int nfs_flushcommits(nfsnode_t, int);
int nfs_flush(nfsnode_t, int, thread_t, int);
void nfs_buf_delwri_push(int);
void nfs_buf_delwri_service(void);
void nfs_buf_delwri_thread(void *, wait_result_t);

int nfsiod_start(void);
void nfsiod_terminate(struct nfsiod *);
void nfsiod_thread(void);
int nfsiod_continue(int);
void nfs_asyncio_finish(struct nfsreq *);
void nfs_asyncio_resend(struct nfsreq *);
int nfs_async_write_start(struct nfsmount *);
void nfs_async_write_done(struct nfsmount *);

#endif /* KERNEL */

#endif /* __APPLE_API_PRIVATE */
#endif /* _NFS_NFSNODE_H_ */

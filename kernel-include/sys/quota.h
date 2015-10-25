/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Robert Elz at The University of Melbourne.
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
 *	@(#)quota.h
 *	derived from @(#)ufs/ufs/quota.h	8.3 (Berkeley) 8/19/94
 */

#ifndef _SYS_QUOTA_H
#define _SYS_QUOTA_H

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#ifdef KERNEL_PRIVATE
#include <kern/locks.h>
#endif

#include <mach/boolean.h>

#ifdef __APPLE_API_UNSTABLE
/*
 * Definitions for disk quotas imposed on the average user
 * (big brother finally hits UNIX).
 *
 * The following constants define the amount of time given a user before the
 * soft limits are treated as hard limits (usually resulting in an allocation
 * failure). The timer is started when the user crosses their soft limit, it
 * is reset when they go below their soft limit.
 */
#define	MAX_IQ_TIME	(7*24*60*60)	/* seconds in 1 week */
#define	MAX_DQ_TIME	(7*24*60*60)	/* seconds in 1 week */

/*
 * The following constants define the usage of the quota file array in the
 * file system mount structure and dquot array in the inode structure.  The semantics
 * of the elements of these arrays are defined in the routine getinoquota;
 * the remainder of the quota code treats them generically and need not be
 * inspected when changing the size of the array.
 */
#define	MAXQUOTAS	2
#define	USRQUOTA	0	/* element used for user quotas */
#define	GRPQUOTA	1	/* element used for group quotas */

/*
 * Definitions for the default names of the quotas files.
 */
#define INITQFNAMES { \
	"user",		/* USRQUOTA */ \
	"group",	/* GRPQUOTA */ \
	"undefined", \
};
#define	QUOTAFILENAME	".quota"
#define QUOTAOPSNAME    ".quota.ops"
#define	QUOTAGROUP	"operator"

/*
 * Command definitions for the 'quotactl' system call.  The commands are
 * broken into a main command defined below and a subcommand that is used
 * to convey the type of quota that is being manipulated (see above).
 */
#define SUBCMDMASK	0x00ff
#define SUBCMDSHIFT	8
#define	QCMD(cmd, type)	(((cmd) << SUBCMDSHIFT) | ((type) & SUBCMDMASK))

#define	Q_QUOTAON	0x0100	/* enable quotas */
#define	Q_QUOTAOFF	0x0200	/* disable quotas */
#define	Q_GETQUOTA	0x0300	/* get limits and usage */
#define	Q_SETQUOTA	0x0400	/* set limits and usage */
#define	Q_SETUSE	0x0500	/* set usage */
#define	Q_SYNC		0x0600	/* sync disk copy of a filesystems quotas */
#define	Q_QUOTASTAT	0x0700	/* get quota on/off status */

/*
 * The following two structures define the format of the disk
 * quota file (as it appears on disk) - the file contains a
 * header followed by a hash table of dqblk entries.  To find
 * a particular entry, the user or group number (id) is first
 * converted to an index into this table by means of the hash
 * function dqhash1.  If there is a collision at that index
 * location then a second hash value is computed which using
 * dqhash2.  This second hash value is then used as an offset
 * to the next location to probe.  ID = 0 is used to indicate
 * an empty (unused) entry.  So there can never be an entry in
 * the quota file for user 0 or group 0 (which is OK since disk
 * quotas are never enforced for user 0).
 *
 * The setquota system call establishes the vnode for each quota
 * file (a pointer is retained in the filesystem  mount structure).
 */
struct dqfilehdr {
	u_int32_t dqh_magic;		
	u_int32_t dqh_version;		/* == QF_VERSION */
	u_int32_t dqh_maxentries;	/* must be a power of 2 */
	u_int32_t dqh_entrycnt;		/* count of active entries */
	u_int32_t dqh_flags;		/* reserved for now (0) */
	u_int32_t dqh_chktime;		/* time of last quota check */
	u_int32_t dqh_btime;		/* time limit for excessive disk use */
	u_int32_t dqh_itime;		/* time limit for excessive files */
	char      dqh_string[16];	/* tag string */
	u_int32_t dqh_spare[4];		/* pad struct to power of 2 */
};

struct dqblk {
	u_int64_t dqb_bhardlimit;	/* absolute limit on disk bytes alloc */
	u_int64_t dqb_bsoftlimit;	/* preferred limit on disk bytes */
	u_int64_t dqb_curbytes;	        /* current byte count */
	u_int32_t dqb_ihardlimit;	/* maximum # allocated inodes + 1 */
	u_int32_t dqb_isoftlimit;	/* preferred inode limit */
	u_int32_t dqb_curinodes;	/* current # allocated inodes */
	u_int32_t dqb_btime;		/* time limit for excessive disk use */
	u_int32_t dqb_itime;		/* time limit for excessive files */
	u_int32_t dqb_id;		/* identifier (0 for empty entries) */
	u_int32_t dqb_spare[4];		/* pad struct to power of 2 */
};

#ifdef KERNEL_PRIVATE
#include <machine/types.h>	/* user_time_t */
/* LP64 version of struct dqblk.  time_t is a long and must grow when 
 * we're dealing with a 64-bit process.
 * WARNING - keep in sync with struct dqblk
 */

struct user_dqblk {
	u_int64_t dqb_bhardlimit;	/* absolute limit on disk bytes alloc */
	u_int64_t dqb_bsoftlimit;	/* preferred limit on disk bytes */
	u_int64_t dqb_curbytes;	        /* current byte count */
	u_int32_t dqb_ihardlimit;	/* maximum # allocated inodes + 1 */
	u_int32_t dqb_isoftlimit;	/* preferred inode limit */
	u_int32_t dqb_curinodes;	/* current # allocated inodes */
	u_int32_t dqb_btime;		/* time limit for excessive disk use */
	u_int32_t dqb_itime;		/* time limit for excessive files */
	u_int32_t dqb_id;		/* identifier (0 for empty entries) */
	u_int32_t dqb_spare[4];		/* pad struct to power of 2 */
};
#endif  /* KERNEL_PRIVATE */

#define INITQMAGICS { \
	0xff31ff35,	/* USRQUOTA */ \
	0xff31ff27,	/* GRPQUOTA */ \
};

#define QF_VERSION          1
#define QF_STRING_TAG       "QUOTA HASH FILE"

#define QF_USERS_PER_GB     256
#define QF_MIN_USERS        2048
#define QF_MAX_USERS       (2048*1024)

#define QF_GROUPS_PER_GB    32
#define QF_MIN_GROUPS       2048
#define QF_MAX_GROUPS       (256*1024)


/*
 * The primary and secondary multiplicative hash functions are
 * derived from Knuth (vol. 3). They use a prime that is in
 * golden ratio to the machine's word size.
 */
#define dqhash1(id, shift, mask)  \
	((((id) * 2654435761U) >> (shift)) & (mask))

#define dqhash2(id, mask)  \
	(dqhash1((id), 11, (mask)>>1) | 1)

/*
 * Compute a disk offset into a quota file.
 */
#define dqoffset(index)  \
	(sizeof (struct dqfilehdr) + ((index) * sizeof (struct dqblk)))
/*
 * Compute the hash shift value.
 * It is the word size, in bits, minus the hash table size, in bits.
 */
static __inline int dqhashshift(u_int32_t);

static __inline int
dqhashshift(u_int32_t size)
{
	int shift;

	for (shift = 32; size > 1; size >>= 1, --shift)
		continue;
	return (shift);
}


#ifndef KERNEL
__BEGIN_DECLS
int quotactl(const char *, int, int, caddr_t);
__END_DECLS
#endif /* !KERNEL */

#ifdef KERNEL_PRIVATE
#include <sys/queue.h>



/* Quota file info
 */
struct quotafile {
  	lck_mtx_t     qf_lock;	     /* quota file mutex */
	struct vnode *qf_vp;         /* quota file vnode */
	kauth_cred_t  qf_cred;       /* quota file access cred */
	int           qf_shift;      /* primary hash shift */
	int           qf_maxentries; /* size of hash table (power of 2) */
	int           qf_entrycnt;   /* count of active entries */
	u_int32_t     qf_btime;      /* block quota time limit */
	u_int32_t     qf_itime;      /* inode quota time limit */

                                     /* the following 2 fields are protected */
                                     /* by the quota list lock  */
	char          qf_qflags;     /* quota specific flags */
        int	      qf_refcnt;     /* count of dquot refs on this file */
};

/*
 * Flags describing the runtime state of quotas.
 * (in qf_qflags)
 */
#define	QTF_OPENING	0x01	/* Q_QUOTAON  in progress */
#define	QTF_CLOSING	0x02	/* Q_QUOTAOFF in progress */
#define	QTF_WANTED	0x04	/* waiting for change of state */


/*
 * The following structure records disk usage for a user or group on a
 * filesystem. There is one allocated for each quota that exists on any
 * filesystem for the current user or group. A cache is kept of recently
 * used entries.
 */
struct dquot {
	LIST_ENTRY(dquot) dq_hash;	/* hash list */
	TAILQ_ENTRY(dquot) dq_freelist;	/* free list */
	u_int16_t dq_flags;		/* flags, see below */
	u_int16_t dq_cnt_unused;	/* Replaced by dq_cnt below */
        u_int16_t dq_lflags;		/* protected by the quota list lock */
	u_int16_t dq_type;		/* quota type of this dquot */
	u_int32_t dq_id;		/* identifier this applies to */
	u_int32_t dq_index;		/* index into quota file */
	struct	quotafile *dq_qfile;	/* quota file that this is taken from */
	struct	dqblk dq_dqb;		/* actual usage & quotas */
	uint32_t  dq_cnt;		/* count of active references */
};

/*
 * dq_lflags values
 */
#define	DQ_LLOCK	0x01		/* this quota locked (no MODS) */
#define	DQ_LWANT	0x02		/* wakeup on unlock */

/*
 * dq_flags values
 */
#define	DQ_MOD		0x01		/* this quota modified since read */
#define	DQ_FAKE		0x02		/* no limits here, just usage */
#define	DQ_BLKS		0x04		/* has been warned about blk limit */
#define	DQ_INODS	0x08		/* has been warned about inode limit */

/*
 * Shorthand notation.
 */
#define	dq_bhardlimit	dq_dqb.dqb_bhardlimit
#define	dq_bsoftlimit	dq_dqb.dqb_bsoftlimit
#define	dq_curbytes	dq_dqb.dqb_curbytes
#define	dq_ihardlimit	dq_dqb.dqb_ihardlimit
#define	dq_isoftlimit	dq_dqb.dqb_isoftlimit
#define	dq_curinodes	dq_dqb.dqb_curinodes
#define	dq_btime	dq_dqb.dqb_btime
#define	dq_itime	dq_dqb.dqb_itime

/*
 * If the system has never checked for a quota for this file, then it is
 * set to NODQUOT.  Once a write attempt is made the inode pointer is set
 * to reference a dquot structure.
 */
#define	NODQUOT		NULL

/*
 * Flags to chkdq() and chkiq()
 */
#define	FORCE	0x01	/* force usage changes independent of limits */
#define	CHOWN	0x02	/* (advisory) change initiated by chown */


#ifdef XNU_KERNEL_PRIVATE
/*
 * Functions that manage the in-core dquot and the
 * on-disk dqblk data structures.
 */
__BEGIN_DECLS
void	dqfileinit(struct quotafile *);
int	dqfileopen(struct quotafile *, int);
void	dqfileclose(struct quotafile *, int);
void	dqflush(struct vnode *);
int	dqget(u_int32_t, struct quotafile *, int, struct dquot **);
void	dqhashinit(void);
void	dqinit(void);
int	dqisinitialized(void);
void	dqref(struct dquot *);
void	dqrele(struct dquot *);
void	dqreclaim(struct dquot *);
int	dqsync(struct dquot *);
void	dqsync_orphans(struct quotafile *);
void	dqlock(struct dquot *);
void	dqunlock(struct dquot *);

int	qf_get(struct quotafile *, int type);
void	qf_put(struct quotafile *, int type);

__private_extern__ void  munge_dqblk(struct dqblk *dqblkp, struct user_dqblk *user_dqblkp, boolean_t to64);
__END_DECLS
#endif /* XNU_KERNEL_PRIVATE */

#endif /* KERNEL_PRIVATE */

#endif /* __APPLE_API_UNSTABLE */

#endif /* !_SYS_QUOTA_H_ */

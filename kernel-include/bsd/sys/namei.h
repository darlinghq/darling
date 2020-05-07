/*
 * Copyright (c) 2000-2014 Apple Inc. All rights reserved.
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
 * Copyright (c) 1985, 1989, 1991, 1993
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
 *	@(#)namei.h	8.4 (Berkeley) 8/20/94
 */

#ifndef _SYS_NAMEI_H_
#define _SYS_NAMEI_H_

#include <sys/appleapiopts.h>

#ifdef KERNEL
#define LOCKLEAF        0x0004  /* lock inode on return */
#define LOCKPARENT      0x0008  /* want parent vnode returned */
#define WANTPARENT      0x0010  /* want parent vnode returned */

#ifdef KERNEL_PRIVATE
#define CN_SECLUDE_RENAME 0x10000000 /*rename iff ￢(hard-linked ∨ opened ∨ mmaped)*/
#define CN_RAW_ENCRYPTED 0x80000000 /* Look-up is for RO raw encrypted access. */
#endif

#endif // KERNEL

#ifdef BSD_KERNEL_PRIVATE

/* VFS Supports "/..namedfork/rsrc" access. */
#define NAMEDRSRCFORK           NAMEDSTREAMS


#include <sys/queue.h>
#include <sys/uio.h>
#include <sys/vnode.h>
#include <sys/mount.h>
#include <sys/filedesc.h>

#define PATHBUFLEN      256

/*
 * Encapsulation of namei parameters.
 */
struct nameidata {
	/*
	 * Arguments to namei/lookup.
	 */
	user_addr_t ni_dirp;            /* pathname pointer */
	enum    uio_seg ni_segflg;      /* location of pathname */
#if CONFIG_TRIGGERS
	enum    path_operation ni_op;   /* intended operation, see enum path_operation in vnode.h */
#endif /* CONFIG_TRIGGERS */
	/*
	 * Arguments to lookup.
	 */
	struct  vnode *ni_startdir;     /* starting directory */
	struct  vnode *ni_rootdir;      /* logical root directory */
	struct  vnode *ni_usedvp;       /* directory passed in via USEDVP */
	/*
	 * Results: returned from/manipulated by lookup
	 */
	struct  vnode *ni_vp;           /* vnode of result */
	struct  vnode *ni_dvp;          /* vnode of intermediate directory */
	/*
	 * Shared between namei and lookup/commit routines.
	 */
	u_int   ni_pathlen;             /* remaining chars in path */
	char    *ni_next;               /* next location in pathname */
	char    ni_pathbuf[PATHBUFLEN];
	u_long  ni_loopcnt;             /* count of symlinks encountered */

	struct componentname ni_cnd;
	int32_t ni_flag;
	int ni_ncgeneration;            /* For a batched vnop, grab generation beforehand */
};

#define NAMEI_CONTLOOKUP        0x002    /* Continue processing a lookup which was partially processed in a compound VNOP */
#define NAMEI_TRAILINGSLASH     0x004    /* There was at least one trailing slash after last component */
#define NAMEI_UNFINISHED        0x008    /* We broke off a lookup to do a compound op */
/*
 * XXX Hack: we need to encode the intended VNOP in order to
 * be able to include information about which operations a filesystem
 * supports in the decision to break off a lookup early.
 */
#define NAMEI_COMPOUNDOPEN      0x010
#define NAMEI_COMPOUNDREMOVE    0x020
#define NAMEI_COMPOUNDMKDIR     0x040
#define NAMEI_COMPOUNDRMDIR     0x080
#define NAMEI_COMPOUNDRENAME    0x100
#define NAMEI_COMPOUND_OP_MASK (NAMEI_COMPOUNDOPEN | NAMEI_COMPOUNDREMOVE | NAMEI_COMPOUNDMKDIR | NAMEI_COMPOUNDRMDIR | NAMEI_COMPOUNDRENAME)

#ifdef KERNEL
/*
 * namei operational modifier flags, stored in ni_cnd.flags
 * Also includes LOCKLEAF, LOCKPARENT, and WANTPARENT flags, defined above.
 */
#define NOCACHE         0x00000020 /* name must not be left in cache */
#define NOFOLLOW        0x00000000 /* do not follow symbolic links (pseudo) */
/* public FOLLOW	0x00000040    see vnode.h */
#define SHAREDLEAF      0x00000080 /* OK to have shared leaf lock */
#define MODMASK         0x100000fc /* mask of operational modifiers */
/*
 * Namei parameter descriptors.
 *
 * SAVESTART is set only by the callers of namei. It implies SAVENAME
 * plus the addition of saving the parent directory that contains the
 * name in ni_startdir. It allows repeated calls to lookup for the
 * name being sought. The caller is responsible for releasing the
 * buffer and for vrele'ing ni_startdir.
 */
#define SAVENAME        0          /* save pathanme buffer ***obsolete */
#define NOCROSSMOUNT    0x00000100 /* do not cross mount points */
#define RDONLY          0x00000200 /* lookup with read-only semantics */
#define HASBUF          0x00000400 /* has allocated pathname buffer */
#define DONOTAUTH       0x00000800 /* do not authorize during lookup */
#define SAVESTART       0x00001000 /* save starting directory */
/* public ISDOTDOT	0x00002000    see vnode.h */
/* public MAKEENTRY	0x00004000    see vnode.h */
/* public ISLASTCN	0x00008000    see vnode.h */
#define ISSYMLINK       0x00010000 /* symlink needs interpretation */
/* public ISWHITEOUT	0x00020000    see vnode.h */
/* public DOWHITEOUT	0x00040000    see vnode.h */
#define WILLBEDIR       0x00080000 /* new files will be dirs; allow trailing / */
#define AUDITVNPATH1    0x00100000 /* audit the path/vnode info */
#define AUDITVNPATH2    0x00200000 /* audit the path/vnode info */
#define USEDVP          0x00400000 /* start the lookup at ndp.ni_dvp */
#define CN_VOLFSPATH    0x00800000 /* user path was a volfs style path */
#define CN_FIRMLINK_NOFOLLOW    0x01000000 /* Do not follow firm links */
#define UNIONCREATED    0x02000000 /* union fs creation of vnode */
#if NAMEDRSRCFORK
#define CN_WANTSRSRCFORK 0x04000000
#define CN_ALLOWRSRCFORK 0x08000000
#endif // NAMEDRSRCFORK
// CN_SECLUDE_RENAME is defined above as 0x10000000 (SPI)
#define CN_NBMOUNTLOOK  0x20000000 /* do not block for cross mount lookups */
#ifdef BSD_KERNEL_PRIVATE
#define CN_SKIPNAMECACHE        0x40000000      /* skip cache during lookup(), allow FS to handle all components */
#endif
// CN_RAW_ENCRYPTED	is defined above as 0x80000000 (SPI)

/*
 * Initialization of an nameidata structure.
 */

#if CONFIG_TRIGGERS
/* Note: vnode triggers require more precise path operation (ni_op) */

#define NDINIT(ndp, op, pop, flags, segflg, namep, ctx) { \
	(ndp)->ni_cnd.cn_nameiop = op; \
	(ndp)->ni_op = pop; \
	(ndp)->ni_cnd.cn_flags = flags; \
	if ((segflg) == UIO_USERSPACE) { \
	        (ndp)->ni_segflg = ((IS_64BIT_PROCESS(vfs_context_proc(ctx))) ? UIO_USERSPACE64 : UIO_USERSPACE32); \
	} \
	else { \
	        (ndp)->ni_segflg = segflg; \
	} \
	(ndp)->ni_dirp = namep; \
	(ndp)->ni_cnd.cn_context = ctx; \
	(ndp)->ni_flag = 0; \
	(ndp)->ni_cnd.cn_ndp = (ndp); \
}
#else
#define NDINIT(ndp, op, _unused_, flags, segflg, namep, ctx) { \
	(ndp)->ni_cnd.cn_nameiop = op; \
	(ndp)->ni_cnd.cn_flags = flags; \
	if ((segflg) == UIO_USERSPACE) { \
	        (ndp)->ni_segflg = ((IS_64BIT_PROCESS(vfs_context_proc(ctx))) ? UIO_USERSPACE64 : UIO_USERSPACE32); \
	} \
	else { \
	        (ndp)->ni_segflg = segflg; \
	} \
	(ndp)->ni_dirp = namep; \
	(ndp)->ni_cnd.cn_context = ctx; \
	(ndp)->ni_flag = 0; \
	(ndp)->ni_cnd.cn_ndp = (ndp); \
}
#endif /* CONFIG_TRIGGERS */

#endif /* KERNEL */

/*
 * This structure describes the elements in the cache of recent
 * names looked up by namei.
 */
struct  namecache {
	TAILQ_ENTRY(namecache)  nc_entry;       /* chain of all entries */
	TAILQ_ENTRY(namecache)  nc_child;       /* chain of ncp's that are children of a vp */
	union {
		LIST_ENTRY(namecache)  nc_link; /* chain of ncp's that 'name' a vp */
		TAILQ_ENTRY(namecache) nc_negentry; /* chain of ncp's that 'name' a vp */
	} nc_un;
	LIST_ENTRY(namecache)   nc_hash;        /* hash chain */
	vnode_t                 nc_dvp;         /* vnode of parent of name */
	vnode_t                 nc_vp;          /* vnode the name refers to */
	unsigned int            nc_hashval;     /* hashval of stringname */
	const char              *nc_name;       /* pointer to segment name in string cache */
};


#ifdef KERNEL

int     namei(struct nameidata *ndp);
void    nameidone(struct nameidata *);
int     lookup(struct nameidata *ndp);
int     relookup(struct vnode *dvp, struct vnode **vpp,
    struct componentname *cnp);
int     lookup_traverse_union(vnode_t dvp, vnode_t *new_dvp, vfs_context_t ctx);
void    lookup_compound_vnop_post_hook(int error, vnode_t dvp, vnode_t vp, struct nameidata *ndp, int did_create);
void    kdebug_lookup(struct vnode *dp, struct componentname *cnp);

/*
 * namecache function prototypes
 */
void    cache_purgevfs(mount_t mp);
int             cache_lookup_path(struct nameidata *ndp, struct componentname *cnp, vnode_t dp,
    vfs_context_t context, int *dp_authorized, vnode_t last_dp);

void            vnode_cache_authorized_action(vnode_t vp, vfs_context_t context, kauth_action_t action);
void            vnode_uncache_authorized_action(vnode_t vp, kauth_action_t action);
boolean_t       vnode_cache_is_stale(vnode_t vp);
boolean_t       vnode_cache_is_authorized(vnode_t vp, vfs_context_t context, kauth_action_t action);
int             lookup_validate_creation_path(struct nameidata *ndp);
int             namei_compound_available(vnode_t dp, struct nameidata *ndp);

#endif /* KERNEL */

/*
 * Stats on usefulness of namei caches.
 */
struct  nchstats {
	long    ncs_goodhits;           /* hits that we can really use */
	long    ncs_neghits;            /* negative hits that we can use */
	long    ncs_badhits;            /* hits we must drop */
	long    ncs_miss;               /* misses */
	long    ncs_pass2;              /* names found with passes == 2 */
	long    ncs_2passes;            /* number of times we attempt it */
	long    ncs_stolen;
	long    ncs_enters;
	long    ncs_deletes;
	long    ncs_badvid;
};
#endif /* BSD_KERNEL_PRIVATE */

#endif /* !_SYS_NAMEI_H_ */

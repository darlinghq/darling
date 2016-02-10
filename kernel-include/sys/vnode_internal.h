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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1989, 1993
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
 *	@(#)vnode.h	8.17 (Berkeley) 5/20/95
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */

#ifndef _SYS_VNODE_INTERNAL_H_
#define _SYS_VNODE_INTERNAL_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#include <sys/queue.h>
#include <sys/lock.h>

#include <sys/time.h>
#include <sys/uio.h>

#include <sys/vm.h>
#include <sys/systm.h>
#include <kern/locks.h>
#include <vm/vm_kern.h>
#include <sys/vnode.h>
#include <sys/namei.h>
#include <sys/vfs_context.h>
#include <sys/sysctl.h>


struct lockf;
struct label;

LIST_HEAD(buflists, buf);

#if CONFIG_VFS_FUNNEL
struct unsafe_fsnode {
	lck_mtx_t fsnodelock;
	int32_t	  fsnode_count;
	void *	  fsnodeowner;
};
#endif /* CONFIG_VFS_FUNNEL */

#if CONFIG_TRIGGERS
/*
 * VFS Internal (private) trigger vnode resolver info.
 */
struct vnode_resolve {
	lck_mtx_t				vr_lock;   /* protects vnode_resolve_t fields */
	trigger_vnode_resolve_callback_t	vr_resolve_func;
	trigger_vnode_unresolve_callback_t	vr_unresolve_func;
	trigger_vnode_rearm_callback_t		vr_rearm_func;
	trigger_vnode_reclaim_callback_t	vr_reclaim_func;
	void *					vr_data;   /* private data for resolver */
	uint32_t				vr_flags;
	uint32_t				vr_lastseq;
};
typedef struct vnode_resolve *vnode_resolve_t;

/* private vr_flags */
#define VNT_RESOLVED        (1UL << 31)
#define VNT_VFS_UNMOUNTED   (1UL << 30)
#define VNT_EXTERNAL	    (1UL << 29)

#endif /* CONFIG_TRIGGERS */

/*
 * Reading or writing any of these items requires holding the appropriate lock.
 * v_freelist is locked by the global vnode_list_lock
 * v_mntvnodes is locked by the mount_lock
 * v_nclinks and v_ncchildren are protected by the global name_cache_lock
 * v_cleanblkhd and v_dirtyblkhd and v_iterblkflags are locked via the global buf_mtxp
 * the rest of the structure is protected by the vnode_lock
 */
struct vnode {
	lck_mtx_t v_lock;			/* vnode mutex */
	TAILQ_ENTRY(vnode) v_freelist;		/* vnode freelist */
	TAILQ_ENTRY(vnode) v_mntvnodes;		/* vnodes for mount point */
        LIST_HEAD(, namecache) v_nclinks;	/* name cache entries that name this vnode */
        LIST_HEAD(, namecache) v_ncchildren;	/* name cache entries that regard us as there parent */
        vnode_t	 v_defer_reclaimlist;		/* in case we have to defer the reclaim to avoid recursion */
        uint32_t v_listflag;			/* flags protected by the vnode_list_lock (see below) */
	uint32_t v_flag;			/* vnode flags (see below) */
	uint16_t v_lflag;			/* vnode local and named ref flags */
	uint8_t	 v_iterblkflags;		/* buf iterator flags */
	uint8_t	 v_references;			/* number of times io_count has been granted */
	int32_t	 v_kusecount;			/* count of in-kernel refs */
	int32_t	 v_usecount;			/* reference count of users */
	int32_t	 v_iocount;			/* iocounters */
	void *   v_owner;			/* act that owns the vnode */
	uint16_t v_type;			/* vnode type */
	uint16_t v_tag;				/* type of underlying data */
	uint32_t v_id;				/* identity of vnode contents */
	union {
		struct mount	*vu_mountedhere;/* ptr to mounted vfs (VDIR) */
		struct socket	*vu_socket;	/* unix ipc (VSOCK) */
		struct specinfo	*vu_specinfo;	/* device (VCHR, VBLK) */
		struct fifoinfo	*vu_fifoinfo;	/* fifo (VFIFO) */
	        struct ubc_info *vu_ubcinfo;	/* valid for (VREG) */
	} v_un;
	struct	buflists v_cleanblkhd;		/* clean blocklist head */
	struct	buflists v_dirtyblkhd;		/* dirty blocklist head */
	struct klist v_knotes;			/* knotes attached to this vnode */
        /*
	 * the following 4 fields are protected
	 * by the name_cache_lock held in 
	 * excluive mode
	 */
        kauth_cred_t	v_cred;			/* last authorized credential */
        kauth_action_t	v_authorized_actions;	/* current authorized actions for v_cred */
        int		v_cred_timestamp;	/* determine if entry is stale for MNTK_AUTH_OPAQUE */
        int		v_nc_generation;	/* changes when nodes are removed from the name cache */
        /*
	 * back to the vnode lock for protection
	 */
	int32_t		v_numoutput;			/* num of writes in progress */
	int32_t		v_writecount;			/* reference count of writers */
	const char *v_name;			/* name component of the vnode */
	vnode_t v_parent;			/* pointer to parent vnode */
	struct lockf	*v_lockf;		/* advisory lock list head */
#if CONFIG_VFS_FUNNEL
        struct unsafe_fsnode *v_unsafefs;	/* pointer to struct used to lock */
#else 
	int32_t		v_reserved1;
#ifdef __LP64__
	int32_t		v_reserved2;
#endif
#endif /* CONFIG_VFS_FUNNEL */
	int 	(**v_op)(void *);		/* vnode operations vector */
	mount_t v_mount;			/* ptr to vfs we are in */
	void *	v_data;				/* private data for fs */
#if CONFIG_MACF
	struct label *v_label;			/* MAC security label */
#endif
#if CONFIG_TRIGGERS
	vnode_resolve_t v_resolve;		/* trigger vnode resolve info (VDIR only) */
#endif /* CONFIG_TRIGGERS */
};

#define	v_mountedhere	v_un.vu_mountedhere
#define	v_socket	v_un.vu_socket
#define	v_specinfo	v_un.vu_specinfo
#define	v_fifoinfo	v_un.vu_fifoinfo
#define v_ubcinfo	v_un.vu_ubcinfo


/*
 * v_iterblkflags
 */
#define VBI_ITER		0x1
#define VBI_ITERWANT		0x2
#define VBI_CLEAN		0x4
#define VBI_DIRTY		0x8
#define VBI_NEWBUF		0x10

/*
 * v_listflag
 */
#define VLIST_RAGE		  0x01		/* vnode is currently in the rapid age list */
#define VLIST_DEAD		  0x02		/* vnode is currently in the dead list */
#define VLIST_ASYNC_WORK	  0x04		/* vnode is currently on the deferred async work queue */

/*
 * v_lflags
 */
#define	VL_SUSPENDED 	0x0001		/* vnode is suspended */
#define	VL_DRAIN	0x0002		/* vnode is being drained */
#define	VL_TERMINATE	0x0004		/* vnode is in the process of being recycled */
#define	VL_TERMWANT	0x0008		/* there's a waiter  for recycle finish (vnode_getiocount)*/
#define	VL_DEAD		0x0010		/* vnode is dead, cleaned of filesystem-specific info */
#define	VL_MARKTERM	0x0020		/* vnode should be recycled when no longer referenced */
#define VL_NEEDINACTIVE	0x0080		/* delay VNOP_INACTIVE until iocount goes to 0 */

#define	VL_LABEL	0x0100		/* vnode is marked for labeling */
#define	VL_LABELWAIT	0x0200		/* vnode is marked for labeling */
#define	VL_LABELED	0x0400		/* vnode is labeled */
#define	VL_LWARNED	0x0800
#define VL_HASSTREAMS	0x1000		/* vnode has had at least one associated named stream vnode (may not have one any longer) */

#define	VNAMED_UBC	0x2000		/* ubc named reference */
#define VNAMED_MOUNT	0x4000		/* mount point named reference */
#define	VNAMED_FSHASH	0x8000		/* FS hash named reference */

/*
 * v_flags
 */
#define	VROOT		0x000001	/* root of its file system */
#define	VTEXT		0x000002	/* vnode is a pure text prototype */
#define	VSYSTEM		0x000004	/* vnode being used by kernel */
#define	VISTTY		0x000008	/* vnode represents a tty */
#define	VRAGE		0x000010	/* vnode is in rapid age state */
#define VBDEVVP		0x000020        /* vnode created by bdevvp */
#define VDEVFLUSH	0x000040        /* device vnode after vflush */
#define	VMOUNT		0x000080	/* mount operation in progress */
#define	VBWAIT		0x000100	/* waiting for output to complete */
#define VSHARED_DYLD	0x000200	/* vnode is a dyld shared cache file */
#define	VNOCACHE_DATA	0x000400	/* don't keep data cached once it's been consumed */
#define	VSTANDARD	0x000800	/* vnode obtained from common pool */
#define	VAGE		0x001000	/* Insert vnode at head of free list */
#define	VRAOFF		0x002000	/* read ahead disabled */
#define	VNCACHEABLE	0x004000	/* vnode is allowed to be put back in name cache */
#if NAMEDSTREAMS
#define	VISSHADOW	0x008000	/* vnode is a shadow file */
#endif
#define	VSWAP		0x010000	/* vnode is being used as swapfile */
#define	VTHROTTLED	0x020000	/* writes or pageouts have been throttled */
		/* wakeup tasks waiting when count falls below threshold */
#define	VNOFLUSH	0x040000	/* don't vflush() if SKIPSYSTEM */
#define	VLOCKLOCAL	0x080000	/* this vnode does adv locking in vfs */
#define	VISHARDLINK	0x100000	/* hard link needs special processing on lookup and in volfs */
#define	VISUNION	0x200000	/* union special processing */
#if NAMEDSTREAMS
#define	VISNAMEDSTREAM	0x400000	/* vnode is a named stream (eg HFS resource fork) */
#endif
#define VOPENEVT        0x800000        /* if process is P_CHECKOPENEVT, then or in the O_EVTONLY flag on open */
#define VNEEDSSNAPSHOT 0x1000000

/*
 * Global vnode data.
 */
extern	struct vnode *rootvnode;	/* root (i.e. "/") vnode */

#ifdef CONFIG_IMGSRC_ACCESS
#define MAX_IMAGEBOOT_NESTING	2
extern	struct vnode *imgsrc_rootvnodes[];
#endif /* CONFIG_IMGSRC_ACCESS */


/*
 * Mods for exensibility.
 */

/*
 * Flags for vdesc_flags:
 */
#define VDESC_MAX_VPS		16
/* Low order 16 flag bits are reserved for willrele flags for vp arguments. */
#define VDESC_VP0_WILLRELE	0x0001
#define VDESC_VP1_WILLRELE	0x0002
#define VDESC_VP2_WILLRELE	0x0004
#define VDESC_VP3_WILLRELE	0x0008
#define VDESC_NOMAP_VPP		0x0100
#define VDESC_VPP_WILLRELE	0x0200

/*
 * VDESC_NO_OFFSET is used to identify the end of the offset list
 * and in places where no such field exists.
 */
#define VDESC_NO_OFFSET -1

/*
 * This structure describes the vnode operation taking place.
 */
struct vnodeop_desc {
	int	vdesc_offset;		/* offset in vector--first for speed */
	const char *vdesc_name;		/* a readable name for debugging */
	int	vdesc_flags;		/* VDESC_* flags */

	/*
	 * These ops are used by bypass routines to map and locate arguments.
	 * Creds and procs are not needed in bypass routines, but sometimes
	 * they are useful to (for example) transport layers.
	 * Nameidata is useful because it has a cred in it.
	 */
	int	*vdesc_vp_offsets;	/* list ended by VDESC_NO_OFFSET */
	int	vdesc_vpp_offset;	/* return vpp location */
	int	vdesc_cred_offset;	/* cred location, if any */
	int	vdesc_proc_offset;	/* proc location, if any */
	int	vdesc_componentname_offset; /* if any */
	int	vdesc_context_offset;	/* context location, if any */
	/*
	 * Finally, we've got a list of private data (about each operation)
	 * for each transport layer.  (Support to manage this list is not
	 * yet part of BSD.)
	 */
	caddr_t	*vdesc_transports;
};

/*
 * A list of all the operation descs.
 */
extern struct vnodeop_desc *vnodeop_descs[];


/*
 * This macro is very helpful in defining those offsets in the vdesc struct.
 *
 * This is stolen from X11R4.  I ingored all the fancy stuff for
 * Crays, so if you decide to port this to such a serious machine,
 * you might want to consult Intrisics.h's XtOffset{,Of,To}.
 */
#define VOPARG_OFFSET(p_type,field) \
        ((int) (((char *) (&(((p_type)NULL)->field))) - ((char *) NULL)))
#define VOPARG_OFFSETOF(s_type,field) \
	VOPARG_OFFSET(s_type*,field)
#define VOPARG_OFFSETTO(S_TYPE,S_OFFSET,STRUCT_P) \
	((S_TYPE)(((char*)(STRUCT_P))+(S_OFFSET)))



/*
 * VOCALL calls an op given an ops vector.  We break it out because BSD's
 * vclean changes the ops vector and then wants to call ops with the old
 * vector.
 */
#define VOCALL(OPSV,OFF,AP) (( *((OPSV)[(OFF)])) (AP))

/*
 * This call works for vnodes in the kernel.
 */
#define VCALL(VP,OFF,AP) VOCALL((VP)->v_op,(OFF),(AP))
#define VDESC(OP) (& __CONCAT(OP,_desc))
#define VOFFSET(OP) (VDESC(OP)->vdesc_offset)

struct ostat;

#define BUILDPATH_NO_FS_ENTER 0x1 /* Use cache values, do not enter file system */
#define BUILDPATH_CHECKACCESS 0x2 /* Check if parents have search rights */
#define BUILDPATH_CHECK_MOVED 0x4 /* Return EAGAIN if the parent hierarchy is modified */

int	build_path(vnode_t first_vp, char *buff, int buflen, int *outlen, int flags, vfs_context_t ctx);

int 	bdevvp(dev_t dev, struct vnode **vpp);
void	cvtstat(struct stat *st, struct ostat *ost);
void	vprint(const char *label, struct vnode *vp);


__private_extern__ int is_package_name(const char *name, int len);
__private_extern__ int set_package_extensions_table(user_addr_t data, int nentries, int maxwidth);
int 	vn_rdwr_64(enum uio_rw rw, struct vnode *vp, uint64_t base,
	    		int64_t len, off_t offset, enum uio_seg segflg,
			int ioflg, kauth_cred_t cred, int64_t *aresid,
			struct proc *p);
#if CONFIG_MACF
int	vn_setlabel (struct vnode *vp, struct label *intlabel,
	    vfs_context_t context);
#endif
void	fifo_printinfo(struct vnode *vp);
int 	vn_open(struct nameidata *ndp, int fmode, int cmode);
int	vn_open_modflags(struct nameidata *ndp, int *fmode, int cmode);
int	vn_open_auth(struct nameidata *ndp, int *fmode, struct vnode_attr *);
int 	vn_close(vnode_t, int flags, vfs_context_t ctx);
errno_t vn_remove(vnode_t dvp, vnode_t *vpp, struct nameidata *ndp, int32_t flags, struct vnode_attr *vap, vfs_context_t ctx);
errno_t vn_rename(struct vnode *fdvp, struct vnode **fvpp, struct componentname *fcnp, struct vnode_attr *fvap,
       	     struct vnode *tdvp, struct vnode **tvpp, struct componentname *tcnp, struct vnode_attr *tvap,
       	     uint32_t flags, vfs_context_t ctx);

void	lock_vnode_and_post(vnode_t, int);

#define post_event_if_success(_vp, _error, _event) \
	do { \
		if (0 == (_error)) {	\
			lock_vnode_and_post((_vp), (_event)); \
		} \
	} while (0) 
		
/* Authorization subroutines */
int	vn_authorize_open_existing(vnode_t vp, struct componentname *cnp, int fmode, vfs_context_t ctx, void *reserved);
int	vn_authorize_create(vnode_t, struct componentname *, struct vnode_attr *, vfs_context_t, void*); 
int	vn_attribute_prepare(vnode_t dvp, struct vnode_attr *vap, uint32_t *defaulted_fieldsp, vfs_context_t ctx);
void	vn_attribute_cleanup(struct vnode_attr *vap, uint32_t defaulted_fields);
int	vn_authorize_unlink(vnode_t dvp, vnode_t vp, struct componentname *cnp, vfs_context_t ctx, void *reserved);
int 	vn_authorize_rename(struct vnode *fdvp, struct vnode *fvp, struct componentname *fcnp,
       		struct vnode *tdvp, struct vnode *tvp, struct componentname *tcnp,
            	vfs_context_t ctx, void *reserved);
int	vn_authorize_rmdir(vnode_t dvp, vnode_t vp, struct componentname *cnp, vfs_context_t ctx, void *reserved);

typedef int (*vn_create_authorizer_t)(vnode_t, struct componentname *, struct vnode_attr *, vfs_context_t, void*);
int vn_authorize_mkdir(vnode_t, struct componentname *, struct vnode_attr *, vfs_context_t, void*);
int vn_authorize_null(vnode_t, struct componentname *, struct vnode_attr *, vfs_context_t, void*);
/* End of authorization subroutines */

#define VN_CREATE_NOAUTH		(1<<0)
#define VN_CREATE_NOINHERIT		(1<<1)
#define VN_CREATE_UNION			(1<<2)
#define	VN_CREATE_NOLABEL		(1<<3)
#define	VN_CREATE_DOOPEN		(1<<4)	/* Open file if a batched operation is available */
errno_t vn_create(vnode_t, vnode_t *, struct nameidata *, struct vnode_attr *, uint32_t, int, uint32_t*, vfs_context_t);
int	vn_mkdir(vnode_t dvp, vnode_t *vpp, struct nameidata *ndp, struct vnode_attr *vap, vfs_context_t ctx);
int 	vn_rmdir(vnode_t dvp, vnode_t *vpp, struct nameidata *ndp, struct vnode_attr *vap, vfs_context_t ctx);

int	vn_getxattr(vnode_t, const char *, uio_t, size_t *, int, vfs_context_t);
int	vn_setxattr(vnode_t, const char *, uio_t, int, vfs_context_t);
int	vn_removexattr(vnode_t, const char *, int, vfs_context_t);
int	vn_listxattr(vnode_t, uio_t, size_t *, int, vfs_context_t);

int	default_getxattr(vnode_t, const char *, uio_t, size_t *, int, vfs_context_t);
int	default_setxattr(vnode_t, const char *, uio_t, int, vfs_context_t);
int	default_removexattr(vnode_t, const char *, int, vfs_context_t);

int check_appledouble_header(vnode_t, vfs_context_t);

#if NAMEDSTREAMS
errno_t  vnode_getnamedstream(vnode_t, vnode_t *, const char *, enum nsoperation, int, vfs_context_t);
errno_t  vnode_makenamedstream(vnode_t, vnode_t *, const char *, int, vfs_context_t);
errno_t  vnode_removenamedstream(vnode_t, vnode_t, const char *, int, vfs_context_t);
errno_t  vnode_flushnamedstream(vnode_t vp, vnode_t svp, vfs_context_t context);
errno_t  vnode_relenamedstream(vnode_t vp, vnode_t svp, vfs_context_t context);
#endif


void	nchinit(void) __attribute__((section("__TEXT, initcode")));
int	resize_namecache(uint32_t newsize);
void	name_cache_lock_shared(void);
void	name_cache_lock(void);
void	name_cache_unlock(void);
void	cache_enter_with_gen(vnode_t dvp, vnode_t vp, struct componentname *cnp, int gen);
const char *cache_enter_create(vnode_t dvp, vnode_t vp, struct componentname *cnp);

int vn_pathconf(vnode_t, int, int32_t *, vfs_context_t);
extern int nc_disabled; 	

#define	vnode_lock_convert(v)	lck_mtx_convert_spin(&(v)->v_lock)

void	vnode_lock_spin(vnode_t);


void	vnode_list_lock(void);
void	vnode_list_unlock(void);

#define VNODE_REF_FORCE	0x1
int	vnode_ref_ext(vnode_t, int, int);

void	vnode_rele_ext(vnode_t, int, int);
void	vnode_rele_internal(vnode_t, int, int, int);
#ifdef BSD_KERNEL_PRIVATE
int	vnode_getalways(vnode_t);
int 	vget_internal(vnode_t, int, int);
errno_t vnode_getiocount(vnode_t, unsigned int, int);
#endif /* BSD_KERNEL_PRIVATE */
int	vnode_get_locked(vnode_t);
int	vnode_put_locked(vnode_t);

int	vnode_issock(vnode_t);
int	vnode_isaliased(vnode_t);

void	unlock_fsnode(vnode_t, int *);
int	lock_fsnode(vnode_t, int *);

errno_t	vnode_resume(vnode_t);
errno_t	vnode_suspend(vnode_t);


errno_t	vnode_size(vnode_t, off_t *, vfs_context_t);
errno_t	vnode_setsize(vnode_t, off_t, int ioflag, vfs_context_t);
int	vnode_setattr_fallback(vnode_t vp, struct vnode_attr *vap, vfs_context_t ctx);
int	vnode_isspec(vnode_t vp);


#ifdef BSD_KERNEL_PRIVATE

typedef uint32_t compound_vnop_id_t;
#define	COMPOUND_VNOP_OPEN		0x01
#define	COMPOUND_VNOP_MKDIR		0x02
#define	COMPOUND_VNOP_RENAME		0x04
#define	COMPOUND_VNOP_REMOVE		0x08
#define	COMPOUND_VNOP_RMDIR		0x10

int 	vnode_compound_rename_available(vnode_t vp);
int 	vnode_compound_rmdir_available(vnode_t vp);
int 	vnode_compound_mkdir_available(vnode_t vp);
int 	vnode_compound_remove_available(vnode_t vp);
int 	vnode_compound_open_available(vnode_t vp);
int    	vnode_compound_op_available(vnode_t, compound_vnop_id_t);
#endif /* BSD_KERNEL_PRIVATE */

void vn_setunionwait(vnode_t);
void vn_checkunionwait(vnode_t);
void vn_clearunionwait(vnode_t, int);

void SPECHASH_LOCK(void);
void SPECHASH_UNLOCK(void);

void	vnode_authorize_init(void) __attribute__((section("__TEXT, initcode")));

void	vfsinit(void) __attribute__((section("__TEXT, initcode")));
void vnode_lock(vnode_t);
void vnode_unlock(vnode_t);

/*
 * XXX exported symbols; should be static
 */
void	vfs_op_init(void) __attribute__((section("__TEXT, initcode")));
void	vfs_opv_init(void) __attribute__((section("__TEXT, initcode")));
int	vfs_sysctl(int *name, uint32_t namelen, user_addr_t oldp, size_t *oldlenp,
		user_addr_t newp, size_t newlen, struct proc *p);
int	sysctl_vnode(struct sysctl_oid *oidp, void *arg1, int arg2, struct sysctl_req *req);

#ifdef BSD_KERNEL_PRIVATE
void vnode_setneedinactive(vnode_t);
int 	vnode_hasnamedstreams(vnode_t); /* Does this vnode have associated named streams? */

void nspace_proc_exit(struct proc *p);

#if CONFIG_TRIGGERS
/* VFS Internal Vnode Trigger Interfaces (Private) */
int vnode_trigger_resolve(vnode_t, struct nameidata *, vfs_context_t);
void vnode_trigger_rearm(vnode_t, vfs_context_t);
void vfs_nested_trigger_unmounts(mount_t, int, vfs_context_t);
#endif /* CONFIG_TRIGGERS */

#endif /* BSD_KERNEL_PRIVATE */

#endif /* !_SYS_VNODE_INTERNAL_H_ */

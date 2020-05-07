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
 * Copyright (c) 1989, 1991, 1993
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
 *	@(#)mount.h	8.21 (Berkeley) 5/20/95
 */
/*
 * NOTICE: This file was modified by SPARTA, Inc. in 2005 to introduce
 * support for mandatory and extensible security protections.  This notice
 * is included in support of clause 2.2 (b) of the Apple Public License,
 * Version 2.0.
 */


#ifndef _SYS_MOUNT_H_
#define _SYS_MOUNT_H_

#include <sys/appleapiopts.h>
#include <sys/cdefs.h>
#include <sys/attr.h>           /* needed for vol_capabilities_attr_t */
#include <os/base.h>

#ifndef KERNEL
#include <stdint.h>
#include <sys/ucred.h>
#include <sys/queue.h>          /* XXX needed for user builds */
#include <Availability.h>
#else
#include <sys/kernel_types.h>
#include <uuid/uuid.h>
#endif

#include <sys/_types/_fsid_t.h> /* file system id type */

/*
 * file system statistics
 */

#define MFSNAMELEN      15      /* length of fs type name, not inc. null */
#define MFSTYPENAMELEN  16      /* length of fs type name including null */

#if __DARWIN_64_BIT_INO_T
#define MNAMELEN        MAXPATHLEN      /* length of buffer for returned name */
#else /* ! __DARWIN_64_BIT_INO_T */
#define MNAMELEN        90              /* length of buffer for returned name */
#endif /* __DARWIN_64_BIT_INO_T */

#define MNT_EXT_ROOT_DATA_VOL      0x00000001      /* Data volume of root volume group */

#define __DARWIN_STRUCT_STATFS64 { \
	uint32_t	f_bsize;        /* fundamental file system block size */ \
	int32_t		f_iosize;       /* optimal transfer block size */ \
	uint64_t	f_blocks;       /* total data blocks in file system */ \
	uint64_t	f_bfree;        /* free blocks in fs */ \
	uint64_t	f_bavail;       /* free blocks avail to non-superuser */ \
	uint64_t	f_files;        /* total file nodes in file system */ \
	uint64_t	f_ffree;        /* free file nodes in fs */ \
	fsid_t		f_fsid;         /* file system id */ \
	uid_t		f_owner;        /* user that mounted the filesystem */ \
	uint32_t	f_type;         /* type of filesystem */ \
	uint32_t	f_flags;        /* copy of mount exported flags */ \
	uint32_t	f_fssubtype;    /* fs sub-type (flavor) */ \
	char		f_fstypename[MFSTYPENAMELEN];   /* fs type name */ \
	char		f_mntonname[MAXPATHLEN];        /* directory on which mounted */ \
	char		f_mntfromname[MAXPATHLEN];      /* mounted filesystem */ \
	uint32_t    f_flags_ext;    /* extended flags */ \
	uint32_t	f_reserved[7];  /* For future use */ \
}

#if !__DARWIN_ONLY_64_BIT_INO_T

struct statfs64 __DARWIN_STRUCT_STATFS64;

#endif /* !__DARWIN_ONLY_64_BIT_INO_T */

#if __DARWIN_64_BIT_INO_T

struct statfs __DARWIN_STRUCT_STATFS64;

#else /* !__DARWIN_64_BIT_INO_T */

/*
 * LP64 - WARNING - must be kept in sync with struct user_statfs in mount_internal.h.
 */
struct statfs {
	short   f_otype;                /* TEMPORARY SHADOW COPY OF f_type */
	short   f_oflags;               /* TEMPORARY SHADOW COPY OF f_flags */
	long    f_bsize;                /* fundamental file system block size */
	long    f_iosize;               /* optimal transfer block size */
	long    f_blocks;               /* total data blocks in file system */
	long    f_bfree;                /* free blocks in fs */
	long    f_bavail;               /* free blocks avail to non-superuser */
	long    f_files;                /* total file nodes in file system */
	long    f_ffree;                /* free file nodes in fs */
	fsid_t  f_fsid;                 /* file system id */
	uid_t   f_owner;                /* user that mounted the filesystem */
	short   f_reserved1;    /* spare for later */
	short   f_type;                 /* type of filesystem */
	long    f_flags;                /* copy of mount exported flags */
	long    f_reserved2[2]; /* reserved for future use */
	char    f_fstypename[MFSNAMELEN]; /* fs type name */
	char    f_mntonname[MNAMELEN];  /* directory on which mounted */
	char    f_mntfromname[MNAMELEN];/* mounted filesystem */
	char    f_reserved3;    /* For alignment */
	long    f_reserved4[4]; /* For future use */
};

#endif /* __DARWIN_64_BIT_INO_T */

#pragma pack(4)

struct vfsstatfs {
	uint32_t        f_bsize;        /* fundamental file system block size */
	size_t          f_iosize;       /* optimal transfer block size */
	uint64_t        f_blocks;       /* total data blocks in file system */
	uint64_t        f_bfree;        /* free blocks in fs */
	uint64_t        f_bavail;       /* free blocks avail to non-superuser */
	uint64_t        f_bused;        /* free blocks avail to non-superuser */
	uint64_t        f_files;        /* total file nodes in file system */
	uint64_t        f_ffree;        /* free file nodes in fs */
	fsid_t          f_fsid;         /* file system id */
	uid_t           f_owner;        /* user that mounted the filesystem */
	uint64_t        f_flags;        /* copy of mount exported flags */
	char            f_fstypename[MFSTYPENAMELEN];/* fs type name inclus */
	char            f_mntonname[MAXPATHLEN];/* directory on which mounted */
	char            f_mntfromname[MAXPATHLEN];/* mounted filesystem */
	uint32_t        f_fssubtype;     /* fs sub-type (flavor) */
	void            *f_reserved[2];         /* For future use == 0 */
};

#pragma pack()

#ifdef KERNEL
/*
 * Kernel level support for the VFS_GETATTR(), VFS_SETATTR() for use in
 * implementation of filesystem KEXTs, and by the vfs_getattr() and
 * vfs_setattr() KPIs.
 */

#define VFSATTR_INIT(s)                 ((s)->f_supported = (s)->f_active = 0LL)
#define VFSATTR_SET_SUPPORTED(s, a)     ((s)->f_supported |= VFSATTR_ ## a)
#define VFSATTR_IS_SUPPORTED(s, a)      ((s)->f_supported & VFSATTR_ ## a)
#define VFSATTR_CLEAR_ACTIVE(s, a)      ((s)->f_active &= ~VFSATTR_ ## a)
#define VFSATTR_IS_ACTIVE(s, a)         ((s)->f_active & VFSATTR_ ## a)
#define VFSATTR_ALL_SUPPORTED(s)        (((s)->f_active & (s)->f_supported) == (s)->f_active)
#define VFSATTR_WANTED(s, a)            ((s)->f_active |= VFSATTR_ ## a)
#define VFSATTR_RETURN(s, a, x)         do { (s)-> a = (x); VFSATTR_SET_SUPPORTED(s, a);} while(0)

#define VFSATTR_f_objcount              (1LL<<  0)
#define VFSATTR_f_filecount             (1LL<<  1)
#define VFSATTR_f_dircount              (1LL<<  2)
#define VFSATTR_f_maxobjcount           (1LL<<  3)
#define VFSATTR_f_bsize                 (1LL<< 4)
#define VFSATTR_f_iosize                (1LL<<  5)
#define VFSATTR_f_blocks                (1LL<<  6)
#define VFSATTR_f_bfree                 (1LL<<  7)
#define VFSATTR_f_bavail                (1LL<<  8)
#define VFSATTR_f_bused                 (1LL<<  9)
#define VFSATTR_f_files                 (1LL<< 10)
#define VFSATTR_f_ffree                 (1LL<< 11)
#define VFSATTR_f_fsid                  (1LL<< 12)
#define VFSATTR_f_owner                 (1LL<< 13)
#define VFSATTR_f_capabilities          (1LL<< 14)
#define VFSATTR_f_attributes            (1LL<< 15)
#define VFSATTR_f_create_time           (1LL<< 16)
#define VFSATTR_f_modify_time           (1LL<< 17)
#define VFSATTR_f_access_time           (1LL<< 18)
#define VFSATTR_f_backup_time           (1LL<< 19)
#define VFSATTR_f_fssubtype             (1LL<< 20)
#define VFSATTR_f_vol_name              (1LL<< 21)
#define VFSATTR_f_signature             (1LL<< 22)
#define VFSATTR_f_carbon_fsid           (1LL<< 23)
#define VFSATTR_f_uuid                  (1LL<< 24)
#define VFSATTR_f_quota         (1LL<< 25)
#define VFSATTR_f_reserved              (1LL<< 26)


/*
 * Argument structure.
 */
#pragma pack(4)
/*
 * Note: the size of the vfs_attr structure can change.
 * A kext should only reference the fields that are
 * marked as active; it should not depend on the actual
 * size of the structure or attempt to copy it.
 */
struct vfs_attr {
	uint64_t        f_supported;
	uint64_t        f_active;

	uint64_t        f_objcount;     /* number of filesystem objects in volume */
	uint64_t        f_filecount;    /* ... files */
	uint64_t        f_dircount;     /* ... directories */
	uint64_t        f_maxobjcount;  /* maximum number of filesystem objects */

	uint32_t        f_bsize;        /* block size for the below size values */
	size_t          f_iosize;       /* optimal transfer block size */
	uint64_t        f_blocks;       /* total data blocks in file system */
	uint64_t        f_bfree;        /* free blocks in fs */
	uint64_t        f_bavail;       /* free blocks avail to non-superuser */
	uint64_t        f_bused;        /* blocks in use */
	uint64_t        f_files;        /* total file nodes in file system */
	uint64_t        f_ffree;        /* free file nodes in fs */
	fsid_t          f_fsid;         /* file system id */
	uid_t           f_owner;        /* user that mounted the filesystem */

	vol_capabilities_attr_t f_capabilities;
	vol_attributes_attr_t f_attributes;

	struct timespec f_create_time;  /* creation time */
	struct timespec f_modify_time;  /* last modification time */
	struct timespec f_access_time;  /* time of last access */
	struct timespec f_backup_time;  /* last backup time */

	uint32_t        f_fssubtype;    /* filesystem subtype */

	char            *f_vol_name;    /* volume name */

	uint16_t        f_signature;    /* used for ATTR_VOL_SIGNATURE, Carbon's FSVolumeInfo.signature */
	uint16_t        f_carbon_fsid;  /* same as Carbon's FSVolumeInfo.filesystemID */
	uuid_t          f_uuid;         /* file system UUID (version 3 or 5), available in 10.6 and later */
	uint64_t        f_quota;        /* total quota data blocks in file system */
	uint64_t        f_reserved;     /* total reserved data blocks in file system */
};

#pragma pack()

#endif  /* KERNEL */

/*
 * User specifiable flags.
 *
 * Unmount uses MNT_FORCE flag.
 */
#define MNT_RDONLY      0x00000001      /* read only filesystem */
#define MNT_SYNCHRONOUS 0x00000002      /* file system written synchronously */
#define MNT_NOEXEC      0x00000004      /* can't exec from filesystem */
#define MNT_NOSUID      0x00000008      /* don't honor setuid bits on fs */
#define MNT_NODEV       0x00000010      /* don't interpret special files */
#define MNT_UNION       0x00000020      /* union with underlying filesystem */
#define MNT_ASYNC       0x00000040      /* file system written asynchronously */
#define MNT_CPROTECT    0x00000080      /* file system supports content protection */

/*
 * NFS export related mount flags.
 */
#define MNT_EXPORTED    0x00000100      /* file system is exported */

/*
 * Denotes storage which can be removed from the system by the user.
 */

#define MNT_REMOVABLE   0x00000200

/*
 * MAC labeled / "quarantined" flag
 */
#define MNT_QUARANTINE  0x00000400      /* file system is quarantined */

/*
 * Flags set by internal operations.
 */
#define MNT_LOCAL       0x00001000      /* filesystem is stored locally */
#define MNT_QUOTA       0x00002000      /* quotas are enabled on filesystem */
#define MNT_ROOTFS      0x00004000      /* identifies the root filesystem */
#define MNT_DOVOLFS     0x00008000      /* FS supports volfs (deprecated flag in Mac OS X 10.5) */


#define MNT_DONTBROWSE  0x00100000      /* file system is not appropriate path to user data */
#define MNT_IGNORE_OWNERSHIP 0x00200000 /* VFS will ignore ownership information on filesystem objects */
#define MNT_AUTOMOUNTED 0x00400000      /* filesystem was mounted by automounter */
#define MNT_JOURNALED   0x00800000      /* filesystem is journaled */
#define MNT_NOUSERXATTR 0x01000000      /* Don't allow user extended attributes */
#define MNT_DEFWRITE    0x02000000      /* filesystem should defer writes */
#define MNT_MULTILABEL  0x04000000      /* MAC support for individual labels */
#define MNT_NOATIME             0x10000000      /* disable update of file access time */
#define MNT_SNAPSHOT    0x40000000 /* The mount is a snapshot */
#define MNT_STRICTATIME 0x80000000      /* enable strict update of file access time */
#ifdef BSD_KERNEL_PRIVATE
/* #define MNT_IMGSRC_BY_INDEX 0x20000000 see sys/imgsrc.h */
#endif /* BSD_KERNEL_PRIVATE */

/* backwards compatibility only */
#define MNT_UNKNOWNPERMISSIONS MNT_IGNORE_OWNERSHIP


/*
 * XXX I think that this could now become (~(MNT_CMDFLAGS))
 * but the 'mount' program may need changing to handle this.
 */
#define MNT_VISFLAGMASK (MNT_RDONLY	| MNT_SYNCHRONOUS | MNT_NOEXEC	| \
	                MNT_NOSUID	| MNT_NODEV	| MNT_UNION	| \
	                MNT_ASYNC	| MNT_EXPORTED	| MNT_QUARANTINE | \
	                MNT_LOCAL	| MNT_QUOTA | MNT_REMOVABLE | \
	                MNT_ROOTFS	| MNT_DOVOLFS	| MNT_DONTBROWSE | \
	                MNT_IGNORE_OWNERSHIP | MNT_AUTOMOUNTED | MNT_JOURNALED | \
	                MNT_NOUSERXATTR | MNT_DEFWRITE	| MNT_MULTILABEL | \
	                MNT_NOATIME | MNT_STRICTATIME | MNT_SNAPSHOT | MNT_CPROTECT)
/*
 * External filesystem command modifier flags.
 * Unmount can use the MNT_FORCE flag.
 * XXX These are not STATES and really should be somewhere else.
 * External filesystem control flags.
 */
#define MNT_UPDATE      0x00010000      /* not a real mount, just an update */
#define MNT_NOBLOCK     0x00020000      /* don't block unmount if not responding */
#define MNT_RELOAD      0x00040000      /* reload filesystem data */
#define MNT_FORCE       0x00080000      /* force unmount or readonly change */
#define MNT_CMDFLAGS    (MNT_UPDATE|MNT_NOBLOCK|MNT_RELOAD|MNT_FORCE)



/*
 * Sysctl CTL_VFS definitions.
 *
 * Second level identifier specifies which filesystem. Second level
 * identifier VFS_GENERIC returns information about all filesystems.
 */
#define VFS_GENERIC             0       /* generic filesystem information */
#define VFS_NUMMNTOPS           1       /* int: total num of vfs mount/unmount operations */
/*
 * Third level identifiers for VFS_GENERIC are given below; third
 * level identifiers for specific filesystems are given in their
 * mount specific header files.
 */
#define VFS_MAXTYPENUM  1       /* int: highest defined filesystem type */
#define VFS_CONF        2       /* struct: vfsconf for filesystem given
	                         *  as next argument */

/*
 * Flags for various system call interfaces.
 *
 * waitfor flags to vfs_sync() and getfsstat()
 */
#define MNT_WAIT        1       /* synchronized I/O file integrity completion */
#define MNT_NOWAIT      2       /* start all I/O, but do not wait for it */
#define MNT_DWAIT       4       /* synchronized I/O data integrity completion */
#ifdef KERNEL
/* only for VFS_SYNC */
#define MNT_VOLUME      8       /* sync on a single mounted filesystem  */
#endif


#if !defined(KERNEL) && !defined(_KERN_SYS_KERNELTYPES_H_) /* also defined in kernel_types.h */
struct mount;
typedef struct mount * mount_t;
struct vnode;
typedef struct vnode * vnode_t;
#endif

/* Reserved fields preserve binary compatibility */
struct vfsconf {
	uint32_t vfc_reserved1;         /* opaque */
	char    vfc_name[MFSNAMELEN];   /* filesystem type name */
	int     vfc_typenum;            /* historic filesystem type number */
	int     vfc_refcount;           /* number mounted of this type */
	int     vfc_flags;              /* permanent flags */
	uint32_t vfc_reserved2;         /* opaque */
	uint32_t vfc_reserved3;         /* opaque */
};

struct vfsidctl {
	int             vc_vers;        /* should be VFSIDCTL_VERS1 (below) */
	fsid_t          vc_fsid;        /* fsid to operate on. */
	void            *vc_ptr;        /* pointer to data structure. */
	size_t          vc_len;         /* sizeof said structure. */
	u_int32_t       vc_spare[12];   /* spare (must be zero). */
};


/* vfsidctl API version. */
#define VFS_CTL_VERS1   0x01

#ifdef KERNEL
struct user_vfsidctl {
	int                              vc_vers;       /* should be VFSIDCTL_VERS1 (below) */
	fsid_t                   vc_fsid;               /* fsid to operate on. */
	user_addr_t              vc_ptr __attribute((aligned(8)));      /* pointer to data structure. */
	user_size_t              vc_len;                /* sizeof said structure. */
	u_int32_t                vc_spare[12];  /* spare (must be zero). */
};

struct user32_vfsidctl {
	int                              vc_vers;       /* should be VFSIDCTL_VERS1 (below) */
	fsid_t                   vc_fsid;               /* fsid to operate on. */
	user32_addr_t    vc_ptr;                /* pointer to data structure. */
	user32_size_t    vc_len;                /* sizeof said structure. */
	u_int32_t                vc_spare[12];  /* spare (must be zero). */
};

union union_vfsidctl { /* the fields vc_vers and vc_fsid are compatible */
	struct user32_vfsidctl vc32;
	struct user_vfsidctl vc64;
};

#endif /* KERNEL */

/*
 * New style VFS sysctls, do not reuse/conflict with the namespace for
 * private sysctls.
 */
#define VFS_CTL_STATFS  0x00010001      /* statfs */
#define VFS_CTL_UMOUNT  0x00010002      /* unmount */
#define VFS_CTL_QUERY   0x00010003      /* anything wrong? (vfsquery) */
#define VFS_CTL_NEWADDR 0x00010004      /* reconnect to new address */
#define VFS_CTL_TIMEO   0x00010005      /* set timeout for vfs notification */
#define VFS_CTL_NOLOCKS 0x00010006      /* disable file locking */
#define VFS_CTL_SADDR   0x00010007      /* get server address */
#define VFS_CTL_DISC    0x00010008      /* server disconnected */
#define VFS_CTL_SERVERINFO  0x00010009  /* information about fs server */
#define VFS_CTL_NSTATUS 0x0001000A      /* netfs mount status */

struct vfsquery {
	u_int32_t       vq_flags;
	u_int32_t       vq_spare[31];
};

struct vfs_server {
	int32_t  vs_minutes;                    /* minutes until server goes down. */
	u_int8_t vs_server_name[MAXHOSTNAMELEN * 3]; /* UTF8 server name to display (null terminated) */
};

/*
 * NetFS mount status - returned by VFS_CTL_NSTATUS
 */
struct netfs_status {
	u_int32_t       ns_status;              // Current status of mount (vfsquery flags)
	char            ns_mountopts[512];      // Significant mount options
	uint32_t        ns_waittime;            // Time waiting for reply (sec)
	uint32_t        ns_threadcount;         // Number of threads blocked on network calls
	uint64_t        ns_threadids[0];        // Thread IDs of those blocked threads
};

/* vfsquery flags */
#define VQ_NOTRESP      0x0001  /* server down */
#define VQ_NEEDAUTH     0x0002  /* server bad auth */
#define VQ_LOWDISK      0x0004  /* we're low on space */
#define VQ_MOUNT        0x0008  /* new filesystem arrived */
#define VQ_UNMOUNT      0x0010  /* filesystem has left */
#define VQ_DEAD         0x0020  /* filesystem is dead, needs force unmount */
#define VQ_ASSIST       0x0040  /* filesystem needs assistance from external program */
#define VQ_NOTRESPLOCK  0x0080  /* server lockd down */
#define VQ_UPDATE       0x0100  /* filesystem information has changed */
#define VQ_VERYLOWDISK  0x0200  /* file system has *very* little disk space left */
#define VQ_SYNCEVENT    0x0400  /* a sync just happened (not set by kernel starting Mac OS X 10.9) */
#define VQ_SERVEREVENT  0x0800  /* server issued notification/warning */
#define VQ_QUOTA        0x1000  /* a user quota has been hit */
#define VQ_NEARLOWDISK          0x2000  /* Above lowdisk and below desired disk space */
#define VQ_DESIRED_DISK         0x4000  /* the desired disk space */
#define VQ_FREE_SPACE_CHANGE    0x8000  /* free disk space has significantly changed */
#define VQ_FLAG10000    0x10000  /* placeholder */


#ifdef KERNEL

/* Structure for setting device IO parameters per mount point */
struct vfsioattr {
	u_int32_t       io_maxreadcnt;          /* Max. byte count for read */
	u_int32_t       io_maxwritecnt;         /* Max. byte count for write */
	u_int32_t       io_segreadcnt;          /* Max. segment count for read */
	u_int32_t       io_segwritecnt;         /* Max. segment count for write */
	u_int32_t       io_maxsegreadsize;      /* Max. segment read size  */
	u_int32_t       io_maxsegwritesize;     /* Max. segment write size */
	u_int32_t       io_devblocksize;        /* the underlying device block size */
	u_int32_t       io_flags;                       /* flags for underlying device */
	union {
		int64_t io_max_swappin_available;
		// On 32 bit architectures, we don't have any spare
		void *io_reserved[2];
	};
};

#define VFS_IOATTR_FLAGS_FUA                            0x00000001      /* Write-through cache supported */
#define VFS_IOATTR_FLAGS_UNMAP                          0x00000002      /* Unmap (trim) supported */
#define VFS_IOATTR_FLAGS_SWAPPIN_SUPPORTED      0x00000010      /* Pinning swap file supported */

/*
 * Filesystem Registration information
 */
#define VFS_TBLTHREADSAFE               0x0001  /* Only threadsafe filesystems are supported */
#define VFS_TBLFSNODELOCK               0x0002  /* Only threadsafe filesystems are supported */
#define VFS_TBLNOTYPENUM                0x0008
#define VFS_TBLLOCALVOL                 0x0010
#define VFS_TBL64BITREADY               0x0020
#define VFS_TBLNATIVEXATTR              0x0040
#define VFS_TBLDIRLINKS                 0x0080
#define VFS_TBLUNMOUNT_PREFLIGHT        0x0100  /* does a preflight check before unmounting */
#define VFS_TBLGENERICMNTARGS           0x0200  /* force generic mount args for local fs */
#define VFS_TBLREADDIR_EXTENDED         0x0400  /* fs supports VNODE_READDIR_EXTENDED */
#define VFS_TBLNOMACLABEL               0x1000
#define VFS_TBLVNOP_PAGEINV2            0x2000
#define VFS_TBLVNOP_PAGEOUTV2           0x4000
#define VFS_TBLVNOP_NOUPDATEID_RENAME   0x8000  /* vfs should not call vnode_update_ident on rename */
#define VFS_TBLVNOP_SECLUDE_RENAME      0x10000
#define VFS_TBLCANMOUNTROOT             0x20000


struct vfs_fsentry {
	struct vfsops * vfe_vfsops;     /* vfs operations */
	int             vfe_vopcnt;     /* # of vnodeopv_desc being registered (reg, spec, fifo ...) */
	struct vnodeopv_desc ** vfe_opvdescs; /* null terminated;  */
	int                     vfe_fstypenum;  /* historic filesystem type number */
	char            vfe_fsname[MFSNAMELEN]; /* filesystem type name */
	uint32_t        vfe_flags;              /* defines the FS capabilities */
	void *              vfe_reserv[2];/* reserved for future use; set this to zero*/
};



struct vfsops {
	/*!
	 *  @function vfs_mount
	 *  @abstract Perform filesystem-specific operations required for mounting.
	 *  @discussion Typical operations include setting the mount-specific data with vfs_setfsprivate().
	 *  Note that if a mount call fails, the filesystem must clean up any state it has constructed, because
	 *  vfs-level mount code will not clean it up.
	 *  @param mp Mount structure for the newly mounted filesystem.
	 *  @param devvp Device that the filesystem is mounted from.
	 *  @param data Filesystem-specific data passed down from userspace.
	 *  @param context Context to authenticate for mount.
	 *  @return 0 for success, else an error code.  Once success is returned, the filesystem should be ready to go active;
	 *  VFS will not ask again.
	 */
	int  (*vfs_mount)(struct mount *mp, vnode_t devvp, user_addr_t data, vfs_context_t context);

	/*!
	 *  @function vfs_start
	 *  @abstract Mark a mount as ready to be used.
	 *  @discussion After receiving this calldown, a filesystem will be hooked into the mount list and should expect
	 *  calls down from the VFS layer.
	 *  @param mp Mount structure being activated.
	 *  @param flags Unused.
	 *  @param context Context to authenticate for mount.
	 *  @return Return value is ignored.
	 */
	int  (*vfs_start)(struct mount *mp, int flags, vfs_context_t context);

	/*!
	 *  @function vfs_unmount
	 *  @abstract Perform filesystem-specific cleanup as part of unmount.
	 *  @discussion If the unmount downcall succeeds, VFS considers itself authorized to destroy all
	 *  state related to the mount.
	 *  @param mp Mount structure to unmount.
	 *  @param mntflags MNT_FORCE indicates that we wish to unmount even if there are active vnodes.
	 *  @param context Context to authenticate for unmount.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_unmount)(struct mount *mp, int mntflags, vfs_context_t context);

	/*!
	 *  @function vfs_root
	 *  @abstract Get the root vnode of a filesystem.
	 *  @discussion Upon success, should return with an iocount held on the root vnode which the caller will
	 *  drop with vnode_put().
	 *  @param mp Mount for which to get the root.
	 *  @param vpp Destination for root vnode.
	 *  @param context Context to authenticate for getting the root.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_root)(struct mount *mp, struct vnode **vpp, vfs_context_t context);

	/*!
	 *  @function vfs_quotactl
	 *  @abstract Manipulate quotas for a volume.
	 *  @param mp Mount for which to manipulate quotas.
	 *  @param cmds Detailed in "quotactl" manual page.
	 *  @param uid Detailed in "quotactl" manual page.
	 *  @param arg Detailed in "quotactl" manual page.
	 *  @param context Context to authenticate for changing quotas.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_quotactl)(struct mount *mp, int cmds, uid_t uid, caddr_t arg, vfs_context_t context);

	/*!
	 *  @function vfs_getattr
	 *  @abstract Get filesystem attributes.
	 *  @discussion See VFSATTR_RETURN, VFSATTR_ACTIVE, VFSATTR_SET_SUPPORTED, VFSATTR_WANTED macros.
	 *  @param mp Mount for which to get parameters.
	 *  @param vfa Container for specifying which attributes are desired and which attributes the filesystem
	 *  supports, as well as for returning results.
	 *  @param ctx Context to authenticate for getting filesystem attributes.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_getattr)(struct mount *mp, struct vfs_attr *, vfs_context_t context);
/*	int  (*vfs_statfs)(struct mount *mp, struct vfsstatfs *sbp, vfs_context_t context);*/

	/*!
	 *  @function vfs_sync
	 *  @abstract Flush all filesystem data to backing store.
	 *  @discussion vfs_sync will be called as part of the sync() system call and during unmount.
	 *  @param mp Mountpoint to sync.
	 *  @param waitfor MNT_WAIT: flush synchronously, waiting for all data to be written before returning. MNT_NOWAIT: start I/O but do not wait for it.
	 *  @param ctx Context to authenticate for the sync.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_sync)(struct mount *mp, int waitfor, vfs_context_t context);

	/*!
	 *  @function vfs_vget
	 *  @abstract Get a vnode by file id (inode number).
	 *  @discussion This routine is chiefly used to build paths to vnodes.  Result should be turned with an iocount that the
	 *  caller will drop with vnode_put().
	 *  @param mp Mount against which to look up inode number.
	 *  @param ino File ID for desired file, as found through a readdir.
	 *  @param vpp Destination for vnode.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_vget)(struct mount *mp, ino64_t ino, struct vnode **vpp, vfs_context_t context);

	/*!
	 *  @function vfs_fhtovp
	 *  @abstract Get the vnode corresponding to a file handle.
	 *  @discussion Filesystems can return handles to files which are independent of their (transient) vnode identities.
	 *  vfs_thtovp converts that persistent handle back to a vnode.  The vnode should be returned with an iocount which
	 *  the caller will drop with vnode_put().
	 *  @param mp Mount against which to look up file handle.
	 *  @param fhlen Size of file handle structure, as returned by vfs_vptofh.
	 *  @param fhp Pointer to handle.
	 *  @param vpp Destination for vnode.
	 *  @param ctx Context against which to authenticate the file-handle conversion.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_fhtovp)(struct mount *mp, int fhlen, unsigned char *fhp, struct vnode **vpp,
	    vfs_context_t context);

	/*!
	 *  @function vfs_vptofh
	 *  @abstract Get a persistent handle corresponding to a vnode.
	 *  @param mp Mount against which to convert the vnode to a handle.
	 *  @param fhlen Size of buffer provided for handle; set to size of actual handle returned.
	 *  @param fhp Pointer to buffer in which to place handle data.
	 *  @param ctx Context against which to authenticate the file-handle request.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_vptofh)(struct vnode *vp, int *fhlen, unsigned char *fhp, vfs_context_t context);

	/*!
	 *  @function vfs_init
	 *  @abstract Prepare a filesystem for having instances mounted.
	 *  @discussion This routine is called once, before any particular instance of a filesystem
	 *  is mounted; it allows the filesystem to initialize whatever global data structures
	 *  are shared across all mounts.  If this returns successfully, a filesystem should be ready to have
	 *  instances mounted.
	 *  @param vfsconf Configuration information.  Currently, the only useful data are the filesystem name,
	 *  typenum, and flags.  The flags field will be either 0 or MNT_LOCAL.  Many filesystems ignore this
	 *  parameter.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_init)(struct vfsconf *);

	/*!
	 *  @function vfs_sysctl
	 *  @abstract Broad interface for querying and controlling filesystem.
	 *  @discussion VFS defines VFS_CTL_QUERY as a generic status request which is answered
	 *  with the VQ_* macros in a "struct vfsquery."
	 *  A filesystem may also define implementation-specific commands.  See "man 3 sysctl"
	 *  for the meaning of sysctl parameters.
	 *  @param context Context against which to authenticate command.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_sysctl)(int *, u_int, user_addr_t, size_t *, user_addr_t, size_t, vfs_context_t context);

	/*!
	 *  @function vfs_setattr
	 *  @abstract Set filesystem attributes.
	 *  @discussion The other side of the vfs_getattr coin.  Currently only called to set volume name.
	 *  @param mp Mount on which to set attributes.
	 *  @param vfa VFS attribute structure containing requested attributes to set and their values.  Currently
	 *  will only be called with f_vol_name set.
	 *  @param context Context against which to authenticate attribute change.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_setattr)(struct mount *mp, struct vfs_attr *, vfs_context_t context);

	/*!
	 *  @function vfs_ioctl
	 *  @abstract File system control operations.
	 *  @discussion  Unlike vfs_sysctl, this is specific to a particular volume.
	 *  @param mp The mount to execute the command on.
	 *  @param command Identifier for action to take.  The command used here
	 *  should be in the same namespace as VNOP ioctl commands.
	 *  @param data Pointer to data; this can be an integer constant (of 32 bits
	 *  only) or an address to be read from or written to, depending on "command."
	 *  If it is an address, it is valid and resides in the kernel; callers of
	 *  VFS_IOCTL() are responsible for copying to and from userland.
	 *  @param flags Reserved for future use, set to zero
	 *  @param ctx Context against which to authenticate ioctl request.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_ioctl)(struct mount *mp, u_long command, caddr_t data,
	    int flags, vfs_context_t context);

	/*!
	 *  @function vfs_vget_snapdir
	 *  @abstract Get the vnode for the snapshot directory of a filesystem.
	 *  @discussion Upon success, should return with an iocount held on the root vnode which the caller will
	 *  drop with vnode_put().
	 *  @param mp Mount for which to get the root.
	 *  @param vpp Destination for snapshot directory vnode.
	 *  @param context Context to authenticate for getting the snapshot directory.
	 *  @return 0 for success, else an error code.
	 */
	int  (*vfs_vget_snapdir)(struct mount *mp, struct vnode **vpp, vfs_context_t context);
	void *vfs_reserved5;
	void *vfs_reserved4;
	void *vfs_reserved3;
	void *vfs_reserved2;
	void *vfs_reserved1;
};

#ifdef KERNEL

/*
 * Commands for vfs_ioctl. While they are encoded the same way as for ioctl(2),
 * there is no generic interface for them from userspace like ioctl(2).
 */
struct fs_snapshot_mount_args {
	mount_t sm_mp;
	struct componentname *sm_cnp;
};

#define VFSIOC_MOUNT_SNAPSHOT  _IOW('V', 1, struct fs_snapshot_mount_args)

struct fs_snapshot_revert_args {
	struct componentname *sr_cnp;
};
#define VFSIOC_REVERT_SNAPSHOT  _IOW('V', 2, struct fs_snapshot_revert_args)

struct fs_snapshot_root_args {
	struct componentname *sr_cnp;
};
#define VFSIOC_ROOT_SNAPSHOT  _IOW('V', 3, struct fs_snapshot_root_args)

typedef struct fs_role_mount_args {
	mount_t root_mp;
	uint32_t mount_role;
} fs_role_mount_args_t;

OS_ENUM(vfs_roles, uint32_t,
    VFS_SYSTEM_ROLE = 1,
    VFS_VM_ROLE = 8,
    VFS_DATA_ROLE = 64);

#define VFSIOC_MOUNT_BYROLE  _IOW('V', 4, fs_role_mount_args_t)

#endif /* KERNEL */

/*
 * flags passed into vfs_iterate
 */
#ifdef PRIVATE
#define VFS_ITERATE_TAIL_FIRST  (1 << 0)
#define VFS_ITERATE_CB_DROPREF  (1 << 1)        // Callback will drop the iterref
#define VFS_ITERATE_NOSKIP_UNMOUNT  (1 << 2)    /* Callback will be made on FS in unmount.
	                                        * The callback cannot make any calls
	                                        * into the Filesystem when this is set. */
#endif /* PRIVATE */

/*
 * return values from callback
 */
#define VFS_RETURNED            0       /* done with vnode, reference can be dropped */
#define VFS_RETURNED_DONE       1       /* done with vnode, reference can be dropped, terminate iteration */
#define VFS_CLAIMED             2       /* don't drop reference */
#define VFS_CLAIMED_DONE        3       /* don't drop reference, terminate iteration */


__BEGIN_DECLS
#ifdef BSD_KERNEL_PRIVATE
extern int VFS_MOUNT(mount_t, vnode_t, user_addr_t, vfs_context_t);
extern int VFS_START(mount_t, int, vfs_context_t);
extern int VFS_UNMOUNT(mount_t, int, vfs_context_t);
extern int VFS_ROOT(mount_t, vnode_t *, vfs_context_t);
extern int VFS_QUOTACTL(mount_t, int, uid_t, caddr_t, vfs_context_t);
extern int VFS_GETATTR(mount_t, struct vfs_attr *, vfs_context_t);
extern int VFS_SETATTR(mount_t, struct vfs_attr *, vfs_context_t);
extern int VFS_SYNC(mount_t, int, vfs_context_t);
extern int VFS_VGET(mount_t, ino64_t, vnode_t *, vfs_context_t);
extern int VFS_FHTOVP(mount_t, int, unsigned char *, vnode_t *, vfs_context_t);
extern int VFS_VPTOFH(vnode_t, int *, unsigned char *, vfs_context_t);
extern int VFS_IOCTL(mount_t mp, u_long command, caddr_t data,
    int flags, vfs_context_t context);
extern int VFS_VGET_SNAPDIR(mount_t, vnode_t *, vfs_context_t);
#endif /* BSD_KERNEL_PRIVATE */
/*
 * prototypes for exported VFS operations
 */

/*!
 *  @function vfs_fsadd
 *  @abstract Register a filesystem with VFS.
 *  @discussion Typically called by a filesystem Kernel Extension when it is loaded.
 *  @param vfe Filesystem information: table of vfs operations, list of vnode operation tables,
 *  filesystem type number (can be omitted with VFS_TBLNOTYPENUM flag), name, flags.
 *  @param handle Opaque handle which will be passed to vfs_fsremove.
 *  @return 0 for success, else an error code.
 */
int vfs_fsadd(struct vfs_fsentry *vfe, vfstable_t *handle);

/*!
 *  @function vfs_fsremove
 *  @abstract Unregister a filesystem with VFS.
 *  @discussion Typically called by a filesystem Kernel Extension when it is unloaded.
 *  @param handle Handle which was returned by vfs_fsadd.
 *  @return 0 for success, else an error code.
 */
int vfs_fsremove(vfstable_t handle);

/*!
 *  @function vfs_iterate
 *  @abstract Iterate over all mountpoints with a callback.  Used, for example, by sync().
 *  @param flags Unused.
 *  @param callout Function which takes a mount and arbitrary passed-in "arg," and returns one of VFS_RETURNED_DONE or VFS_CLAIMED_DONE: end
 *  iteration and return success.  VFS_RETURNED or VFS_CLAIMED: continue iterating. Anything else: continue iterating.
 *  @param arg Arbitrary data to pass to callback.
 *  @return 0 for success, else an error code.
 */
int     vfs_iterate(int flags, int (*callout)(struct mount *, void *), void *arg);

/*!
 *  @function vfs_init_io_attributes
 *  @abstract Set I/O attributes on a mountpoint based on device properties.
 *  @param devvp Block device vnode from which a filesystem is being mounted.
 *  @param mp Mountpoint whose I/O parameters to initialize.
 *  @return 0 for success, else an error code.
 */
int     vfs_init_io_attributes(vnode_t devvp, mount_t mp);

/*!
 *  @function vfs_flags
 *  @abstract Retrieve mount flags.
 *  @discussion Results will be in the bitwise "OR" of MNT_VISFLAGMASK and MNT_CMDFLAGS.
 *  @param mp Mount whose flags to grab.
 *  @return Flags.
 */
uint64_t vfs_flags(mount_t mp);

/*!
 *  @function vfs_setflags
 *  @abstract Set flags on a mount.
 *  @discussion Sets mount flags to the bitwise "OR" of their current value and the specified bits. Often
 *  used by a filesystem as part of the mount process.
 *  @param mp Mount whose flags to set.
 *  @param flags Flags to activate.  Must be in the bitwise "OR" of MNT_VISFLAGMASK and MNT_CMDFLAGS.
 */
void    vfs_setflags(mount_t mp, uint64_t flags);

/*!
 *  @function vfs_clearflags
 *  @abstract Clear flags on a mount.
 *  @discussion Sets mount flags to the bitwise "AND" of their current value and the complement of the specified bits.
 *  @param mp Mount whose flags to set.
 *  @param flags Flags to deactivate.  Must be in the bitwise "OR" of MNT_VISFLAGMASK and MNT_CMDFLAGS.
 */
void    vfs_clearflags(mount_t mp, uint64_t flags);

/*!
 *  @function vfs_issynchronous
 *  @abstract Determine if writes to a filesystem occur synchronously.
 *  @param mp Mount to test.
 *  @return Nonzero if writes occur synchronously, else 0.
 */
int     vfs_issynchronous(mount_t mp);

/*!
 *  @function vfs_iswriteupgrade
 *  @abstract Determine if a filesystem is mounted read-only but a request has been made to upgrade
 *  to read-write.
 *  @param mp Mount to test.
 *  @return Nonzero if a request has been made to update from read-only to read-write, else 0.
 */
int     vfs_iswriteupgrade(mount_t mp);

/*!
 *  @function vfs_isupdate
 *  @abstract Determine if a mount update is in progress.
 *  @param mp Mount to test.
 *  @return Nonzero if a mount update is in progress, 0 otherwise.
 */
int     vfs_isupdate(mount_t mp);

/*!
 *  @function vfs_isreload
 *  @abstract Determine if a reload of filesystem data is in progress.  This can only be the case
 *  for a read-only filesystem; all data is brought in from secondary storage.
 *  @param mp Mount to test.
 *  @return Nonzero if a request has been made to reload data, else 0.
 */
int     vfs_isreload(mount_t mp);

/*!
 *  @function vfs_isforce
 *  @abstract Determine if a forced unmount is in progress.
 *  @discussion A forced unmount invalidates open files.
 *  @param mp Mount to test.
 *  @return Nonzero if a request has been made to forcibly unmount, else 0.
 */
int     vfs_isforce(mount_t mp);

/*!
 *  @function vfs_isunmount
 *  @abstract Determine if an unmount is in progress.
 *  @discussion This is an unsynchronized snapshot of the mount state.  It should only be called
 *  if the mount is known to be valid, e.g. there are known to be live files on that volume.
 *  @param mp Mount to test.
 *  @return Nonzero if an unmount is in progress, else zero.
 */
int     vfs_isunmount(mount_t mp);

/*!
 *  @function vfs_isrdonly
 *  @abstract Determine if a filesystem is mounted read-only.
 *  @param mp Mount to test.
 *  @return Nonzero if filesystem is mounted read-only, else 0.
 */
int     vfs_isrdonly(mount_t mp);

/*!
 *  @function vfs_isrdwr
 *  @abstract Determine if a filesystem is mounted with writes enabled.
 *  @param mp Mount to test.
 *  @return Nonzero if filesystem is mounted read-write, else 0.
 */
int     vfs_isrdwr(mount_t mp);

/*!
 *  @function vfs_authopaque
 *  @abstract Determine if a filesystem's authorization decisions occur remotely.
 *  @param mp Mount to test.
 *  @return Nonzero if filesystem authorization is controlled remotely, else 0.
 */
int     vfs_authopaque(mount_t mp);

/*!
 *  @function vfs_authopaqueaccess
 *  @abstract Check if a filesystem is marked as having reliable remote VNOP_ACCESS support.
 *  @param mp Mount to test.
 *  @return Nonzero if VNOP_ACCESS is supported remotely, else 0.
 */
int     vfs_authopaqueaccess(mount_t mp);

/*!
 *  @function vfs_setauthopaque
 *  @abstract Mark a filesystem as having authorization decisions controlled remotely.
 *  @param mp Mount to mark.
 */
void    vfs_setauthopaque(mount_t mp);

/*!
 *  @function vfs_setauthopaqueaccess
 *  @abstract Mark a filesystem as having remote VNOP_ACCESS support.
 *  @param mp Mount to mark.
 */
void    vfs_setauthopaqueaccess(mount_t mp);

/*!
 *  @function vfs_clearauthopaque
 *  @abstract Mark a filesystem as not having remote authorization decisions.
 *  @param mp Mount to mark.
 */
void    vfs_clearauthopaque(mount_t mp);

/*!
 *  @function vfs_clearauthopaque
 *  @abstract Mark a filesystem as not having remote VNOP_ACCESS support.
 *  @param mp Mount to mark.
 */
void    vfs_clearauthopaqueaccess(mount_t mp);

/*!
 *  @function vfs_setextendedsecurity
 *  @abstract Mark a filesystem as supporting security controls beyond POSIX permissions.
 *  @discussion Specific controls include ACLs, file owner UUIDs, and group UUIDs.
 *  @param mp Mount to test.
 */
void    vfs_setextendedsecurity(mount_t mp);

/*!
 *  @function vfs_clearextendedsecurity
 *  @abstract Mark a filesystem as NOT supporting security controls beyond POSIX permissions.
 *  @discussion Specific controls include ACLs, file owner UUIDs, and group UUIDs.
 *  @param mp Mount to test.
 */
void    vfs_clearextendedsecurity(mount_t mp);

/*!
 *  @function vfs_setnoswap
 *  @abstract Mark a filesystem as unable to use swap files.
 *  @param mp Mount to mark.
 */
void    vfs_setnoswap(mount_t mp);

/*!
 *  @function vfs_clearnoswap
 *  @abstract Mark a filesystem as capable of using swap files.
 *  @param mp Mount to mark.
 */
void    vfs_clearnoswap(mount_t mp);

/*!
 *  @function vfs_setlocklocal
 *  @abstract Mark a filesystem as using VFS-level advisory locking support.
 *  @discussion Advisory locking operations will not call down to the filesystem if this flag is set.
 *  @param mp Mount to mark.
 */
void    vfs_setlocklocal(mount_t mp);

/*!
 *  @function vfs_authcache_ttl
 *  @abstract Determine the time-to-live of cached authorized credentials for files in this filesystem.
 *  @discussion If a filesystem is set to allow caching credentials, the VFS layer can authorize
 *  previously-authorized actions from the same vfs_context_t without calling down to the filesystem (though
 *  it will not deny based on the cache).
 *  @param mp Mount for which to check cache lifetime.
 *  @return  Cache lifetime in seconds.  CACHED_RIGHT_INFINITE_TTL indicates that credentials never expire.
 */
int     vfs_authcache_ttl(mount_t mp);

/*!
 *  @function vfs_setauthcache_ttl
 *  @abstract Enable credential caching and set  time-to-live of cached authorized credentials for files in this filesystem.
 *  @discussion If a filesystem is set to allow caching credentials, the VFS layer can authorize
 *  previously-authorized actions from the same vfs_context_t without calling down to the filesystem (though
 *  it will not deny based on the cache).
 *  @param mp Mount for which to set cache lifetime.
 */
void    vfs_setauthcache_ttl(mount_t mp, int ttl);

/*!
 *  @function vfs_clearauthcache_ttl
 *  @abstract Remove time-to-live controls for cached credentials on a filesytem.  Filesystems with remote authorization
 *  decisions (opaque) will still have KAUTH_VNODE_SEARCH rights cached for a default of CACHED_LOOKUP_RIGHT_TTL seconds.
 *  @param mp Mount for which to clear cache lifetime.
 */
void    vfs_clearauthcache_ttl(mount_t mp);

/*
 * return value from vfs_cachedrights_ttl if
 * neither MNTK_AUTH_OPAQUE | MNTK_AUTH_CACHE_TTL
 * is set in mnt_kern_flag.. it indicates
 * that no TTL is being applied to the vnode rights cache
 */
#define CACHED_RIGHT_INFINITE_TTL       ~0

/*!
 *  @function vfs_maxsymlen
 *  @abstract Get the maximum length of a symbolic link on a filesystem.
 *  @param mp Mount from which to get symlink length cap.
 *  @return Max symlink length.
 */
uint32_t vfs_maxsymlen(mount_t mp);

/*!
 *  @function vfs_setmaxsymlen
 *  @abstract Set the maximum length of a symbolic link on a filesystem.
 *  @param mp Mount on which to set symlink length cap.
 *  @param symlen Length to set.
 */
void    vfs_setmaxsymlen(mount_t mp, uint32_t symlen);

/*!
 *  @function vfs_fsprivate
 *  @abstract Get filesystem-private mount data.
 *  @discussion A filesystem generally has an internal mount structure which it attaches to the VFS-level mount structure
 *  as part of the mounting process.
 *  @param mp Mount for which to get private data.
 *  @return Private data.
 */
void *  vfs_fsprivate(mount_t mp);

/*!
 *  @function vfs_setfsprivate
 *  @abstract Set filesystem-private mount data.
 *  @discussion A filesystem generally has an internal mount structure which it attaches to the VFS-level mount structure
 *  as part of the mounting process.
 *  @param mp Mount for which to set private data.
 */
void    vfs_setfsprivate(mount_t mp, void *mntdata);

/*!
 *  @function vfs_statfs
 *  @abstract Get information about filesystem status.
 *  @discussion Each filesystem has a struct vfsstatfs associated with it which is updated as events occur; this function
 *  returns a pointer to it.  Note that the data in the structure will continue to change over time and also that it may
 *  be quite stale if vfs_update_vfsstat has not been called recently.
 *  @param mp Mount for which to get vfsstatfs pointer.
 *  @return Pointer to vfsstatfs.
 */
struct vfsstatfs *      vfs_statfs(mount_t mp);
#define VFS_USER_EVENT          0
#define VFS_KERNEL_EVENT        1

/*!
 *  @function vfs_update_vfsstat
 *  @abstract Update cached filesystem status information in the VFS mount structure.
 *  @discussion Each filesystem has a struct vfsstatfs associated with it which is updated as events occur; this function
 *  updates it so that the structure pointer returned by vfs_statfs() returns a pointer to fairly recent data.
 *  @param mp Mount for which to update cached status information.
 *  @param ctx Context to authenticate against for call down to filesystem.
 *  @param eventtype VFS_USER_EVENT: need for update is driven by user-level request; perform additional authentication.
 *  VFS_KERNEL_EVENT: need for update is driven by in-kernel events.  Skip extra authentication.
 *  @return 0 for success, or an error code for authentication failure or problem with call to filesystem to
 *  request information.
 */
int     vfs_update_vfsstat(mount_t mp, vfs_context_t ctx, int eventtype);

/*!
 *  @function vfs_typenum
 *  @abstract Get (archaic) filesystem type number.
 *  @discussion Filesystem type numbers are an old construct; most filesystems just get a number assigned based on
 *  the order in which they are registered with the system.
 *  @param mp Mount for which to get type number.
 *  @return Type number.
 */
int     vfs_typenum(mount_t mp);

/*!
 *  @function vfs_name
 *  @abstract Copy filesystem name into a buffer.
 *  @discussion Get filesystem name; this refers to the filesystem type of which a mount is an instantiation,
 *  rather than a name specific to the mountpoint.
 *  @param mp Mount for which to get name.
 *  @param buffer Destination for name; length should be at least MFSNAMELEN.
 */
void    vfs_name(mount_t mp, char *buffer);

/*!
 *  @function vfs_devblocksize
 *  @abstract Get the block size of the device underlying a mount.
 *  @param mp Mount for which to get block size.
 *  @return Block size.
 */
int     vfs_devblocksize(mount_t mp);

/*!
 *  @function vfs_ioattr
 *  @abstract Get I/O attributes associated with a mounpoint.
 *  @param mp Mount for which to get attributes.  If NULL, system defaults are filled into ioattrp.
 *  @param ioattrp Destination for results.
 */
void    vfs_ioattr(mount_t mp, struct vfsioattr *ioattrp);

/*!
 *  @function vfs_setioattr
 *  @abstract Set I/O attributes associated with a mounpoint.
 *  @param mp Mount for which to set attributes.
 *  @param ioattrp Structure containing I/O parameters; all fields must be filled in.
 */
void    vfs_setioattr(mount_t mp, struct vfsioattr *ioattrp);

/*!
 *  @function vfs_64bitready
 *  @abstract Check if the filesystem associated with a mountpoint is marked ready for interaction with 64-bit user processes.
 *  @param mp Mount to test.
 *  @return Nonzero if filesystem is ready for 64-bit; 0 otherwise.
 */
int     vfs_64bitready(mount_t mp);


#define LK_NOWAIT 1
/*!
 *  @function vfs_busy
 *  @abstract "Busy" a mountpoint.
 *  @discussion vfs_busy() will "busy" a mountpoint, preventing unmounts from taking off, by taking its reader-writer lock
 *  in a shared manner.  If a mount is dead,
 *  it will fail; if an unmount is in progress, depending on flags, it will either fail immediately or block
 *  until the unmount completes (then failing if the unmount has succeeded, or potentially succeeding if unmounting failed).
 *  A successful vfs_busy() must be followed by a vfs_unbusy() to release the lock on the mount.
 *  @param mp Mount to busy.
 *  @param flags LK_NOWAIT: fail with ENOENT if an unmount is in progress.
 *  @return 0 for success, with a lock held; an error code otherwise, with no lock held.
 */
int     vfs_busy(mount_t mp, int flags);

/*!
 *  @function vfs_unbusy
 *  @abstract "Unbusy" a mountpoint by releasing its read-write lock.
 *  @discussion A successful vfs_busy() must be followed by a vfs_unbusy() to release the lock on the mount.
 *  @param mp Mount to unbusy.
 */
void    vfs_unbusy(mount_t mp);

/*!
 *  @function vfs_getnewfsid
 *  @abstract Generate a unique filesystem ID for a mount and store it in the mount structure.
 *  @discussion Filesystem IDs are returned as part of "struct statfs."  This function is typically
 *  called as part of file-system specific mount code (i.e. through VFS_MOUNT).
 *  @param mp Mount to set an ID for.
 */
void    vfs_getnewfsid(struct mount *mp);

/*!
 *  @function vfs_getvfs
 *  @abstract Given a filesystem ID, look up a mount structure.
 *  @param fsid Filesystem ID to look up.
 *  @return Mountpoint if found, else NULL.  Note unmounting mountpoints can be returned.
 */
mount_t vfs_getvfs(fsid_t *fsid);

/*!
 *  @function vfs_mountedon
 *  @abstract Check whether a given block device has a filesystem mounted on it.
 *  @discussion Note that this is NOT a check for a covered vnode (the directory upon which
 *  a filesystem is mounted)--it is a test for whether a block device is being used as the source
 *  of a filesystem.  Note that a block device marked as being mounted on cannot be opened.
 *  @param vp The vnode to test.
 *  @return EBUSY if vnode is indeed the source of a filesystem; 0 if it is not.
 */
int     vfs_mountedon(struct vnode *vp);

/*!
 *  @function vfs_unmountbyfsid
 *  @abstract Find a filesystem by ID and unmount it.
 *  @param fsid ID of filesystem to unmount, as found through (for example) statfs.
 *  @param flags MNT_FORCE: forcibly invalidate files open on the mount (though in-flight I/O operations
 *  will be allowed to complete).
 *  @param ctx Context against which to authenticate unmount operation.
 *  @return 0 for succcess, nonero for failure.
 */
int     vfs_unmountbyfsid(fsid_t *fsid, int flags, vfs_context_t ctx);

/*!
 *  @function vfs_event_signal
 *  @abstract Post a kqueue-style event on a filesystem (EVFILT_FS).
 *  @param fsid Unused.
 *  @param event Events to post.
 *  @param data Unused.
 */
void    vfs_event_signal(fsid_t *fsid, u_int32_t event, intptr_t data);

/*!
 *  @function vfs_event_init
 *  @abstract This function should not be called by kexts.
 */
void    vfs_event_init(void); /* XXX We should not export this */

/*!
 *  @function vfs_set_root_unmount_cleanly
 *  @abstract This function should be called by the root file system
 *  when it is being mounted if the file system state is consistent.
 */
void vfs_set_root_unmounted_cleanly(void);

#ifdef KERNEL_PRIVATE
int     vfs_getbyid(fsid_t *fsid, ino64_t ino, vnode_t *vpp, vfs_context_t ctx);
int     vfs_getattr(mount_t mp, struct vfs_attr *vfa, vfs_context_t ctx);
int     vfs_setattr(mount_t mp, struct vfs_attr *vfa, vfs_context_t ctx);
int     vfs_extendedsecurity(mount_t);
mount_t vfs_getvfs_by_mntonname(char *);
vnode_t vfs_vnodecovered(mount_t mp); /* Returns vnode with an iocount that must be released with vnode_put() */
vnode_t vfs_devvp(mount_t mp); /* Please see block comment with implementation */
int vfs_nativexattrs(mount_t mp);  /* whether or not the FS supports EAs natively */
void *  vfs_mntlabel(mount_t mp); /* Safe to cast to "struct label*"; returns "void*" to limit dependence of mount.h on security headers.  */
void    vfs_setcompoundopen(mount_t mp);
uint64_t vfs_throttle_mask(mount_t mp);
int vfs_isswapmount(mount_t mp);
boolean_t vfs_context_is_dataless_manipulator(vfs_context_t);

struct vnode_trigger_info;

/*!
 *  @function vfs_addtrigger
 *  @abstract Create an "external" trigger vnode: look up a vnode and mark it as
 *  a trigger.  Can only safely be called in the context of a callback set by
 *  vfs_settriggercallback().  May only be used on a file which is not already
 *  marked as a trigger.
 *  @param relpath Path relative to root of mountpoint at which to mark trigger.
 *  @param vtip Information about trigger; analogous to "vnode_trigger_param"
 *       argument to vnode_create.
 *  @param ctx Authorization context.
 */
int     vfs_addtrigger(mount_t mp, const char *relpath, struct vnode_trigger_info *vtip, vfs_context_t ctx);


/*!
 *  @enum vfs_trigger_callback_op_t
 *  @abstract Operation to perform after an attempted unmount (successful or otherwise).
 *  @constant VTC_REPLACE Unmount failed: attempt to replace triggers.  Only valid
 *       VFS operation to perform in this context is vfs_addtrigger().
 *  @constant VTC_RELEASE Unmount succeeded: release external triggering context.
 */
typedef enum {
	VTC_REPLACE,
	VTC_RELEASE
} vfs_trigger_callback_op_t;

/*!
 *  @typedef vfs_trigger_callback_t
 *  @abstract Callback to be passed to vfs_settriggercallback() and invoked from
 *       unmount context.
 *  @param mp Mountpoint on which unmount is occurring.
 *  @param op Operation (see vfs_trigger_callback_op_t)
 *  @param data Context passed to vfs_settriggercallback()
 *  @param ctx Authorization context in which unmount is occurring.
 */
typedef void vfs_trigger_callback_t(mount_t mp, vfs_trigger_callback_op_t op, void *data, vfs_context_t ctx);

/*!
 *  @function vfs_settriggercallback
 *  @abstract Install a callback to be called after unmount attempts on a volume,
 *  to restore triggers for failed unmounts and release state for successful ones.
 *  @discussion Installs a callback which will be called in two situations: a
 *  failed unmount where vnodes may have been reclaimed and a successful unmount.
 *  Gives an external trigger-marking entity an opportunity to replace triggers
 *  which may have been reclaimed.  The callback can only be installed (not
 *  cleared), and only one callback can be installed.  The callback will be called
 *  with a read-write lock held on the mount point; in the VTC_REPLACE case, the
 *  <em>only</em> valid VFS operation to perform in the context of the callback is
 *  vfs_addtrigger() on the mountpoint in question.  This rwlock is held in order
 *  to attempt to provide some modicum of coverage from lookups which might find
 *  missing trigger vnodes and receive spurious ENOENTs.  Note that this
 *  protection is incomplete--current working directories, or traversals up into a
 *  volume via ".." may still find missing triggers.  As of this writing, no
 *  serialization mechanism exists to do better than this.
 *  When the "op" is VTC_RELEASE, the mountpoint is going away, and the only valid
 *  VFS operation is to free the  private data pointer if needed.  The callback
 *  will be called immediately, with VTC_REPLACE, from vfs_settriggercallback(),
 *  if installation is successful.
 *  @param fsid FSID for filesystem in question.
 *  @param vtc Callback pointer.
 *  @param data Context pointer to be passed to callback.
 *  @param flags Currently unused.
 *  @param ctx Authorization context.
 *  @return 0 for success.  EBUSY if a trigger has already been installed.
 */
int     vfs_settriggercallback(fsid_t *fsid, vfs_trigger_callback_t vtc, void *data, uint32_t flags, vfs_context_t ctx);

/* tags a volume as not supporting extended readdir for NFS exports */
void mount_set_noreaddirext(mount_t);

/*!
 *  @function vfs_get_statfs64
 *  @abstract Get the same information as vfs_statfs(), but in a format suitable
 *  for copying to userland.
 */
void vfs_get_statfs64(struct mount *mp, struct statfs64 *sfs);

#endif  /* KERNEL_PRIVATE */
__END_DECLS

#endif /* KERNEL */

#ifndef KERNEL

/*
 * Generic file handle
 */
#define NFS_MAX_FH_SIZE         NFSV4_MAX_FH_SIZE
#define NFSV4_MAX_FH_SIZE       128
#define NFSV3_MAX_FH_SIZE       64
#define NFSV2_MAX_FH_SIZE       32
struct fhandle {
	unsigned int    fh_len;                         /* length of file handle */
	unsigned char   fh_data[NFS_MAX_FH_SIZE];       /* file handle value */
};
typedef struct fhandle  fhandle_t;


__BEGIN_DECLS
int     fhopen(const struct fhandle *, int);
int     fstatfs(int, struct statfs *) __DARWIN_INODE64(fstatfs);
#if !__DARWIN_ONLY_64_BIT_INO_T
int     fstatfs64(int, struct statfs64 *) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_5, __MAC_10_6, __IPHONE_NA, __IPHONE_NA);
#endif /* !__DARWIN_ONLY_64_BIT_INO_T */
int     getfh(const char *, fhandle_t *);
int     getfsstat(struct statfs *, int, int) __DARWIN_INODE64(getfsstat);
#if !__DARWIN_ONLY_64_BIT_INO_T
int     getfsstat64(struct statfs64 *, int, int) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_5, __MAC_10_6, __IPHONE_NA, __IPHONE_NA);
#endif /* !__DARWIN_ONLY_64_BIT_INO_T */
int     getmntinfo(struct statfs **, int) __DARWIN_INODE64(getmntinfo);
int     getmntinfo_r_np(struct statfs **, int) __DARWIN_INODE64(getmntinfo_r_np)
__OSX_AVAILABLE(10.13) __IOS_AVAILABLE(11.0)
__TVOS_AVAILABLE(11.0) __WATCHOS_AVAILABLE(4.0);
#if !__DARWIN_ONLY_64_BIT_INO_T
int     getmntinfo64(struct statfs64 **, int) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_5, __MAC_10_6, __IPHONE_NA, __IPHONE_NA);
#endif /* !__DARWIN_ONLY_64_BIT_INO_T */
int     mount(const char *, const char *, int, void *);
int     fmount(const char *, int, int, void *) __OSX_AVAILABLE(10.13) __IOS_AVAILABLE(11.0) __TVOS_AVAILABLE(11.0) __WATCHOS_AVAILABLE(4.0);
int     statfs(const char *, struct statfs *) __DARWIN_INODE64(statfs);
#if !__DARWIN_ONLY_64_BIT_INO_T
int     statfs64(const char *, struct statfs64 *) __OSX_AVAILABLE_BUT_DEPRECATED(__MAC_10_5, __MAC_10_6, __IPHONE_NA, __IPHONE_NA);
#endif /* !__DARWIN_ONLY_64_BIT_INO_T */
int     unmount(const char *, int);
int     getvfsbyname(const char *, struct vfsconf *);
__END_DECLS

#endif /* KERNEL */
#endif /* !_SYS_MOUNT_H_ */

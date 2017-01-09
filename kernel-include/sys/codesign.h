/*
 * Copyright (c) 2006 Apple Computer, Inc. All rights reserved.
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

#ifndef _SYS_CODESIGN_H_
#define _SYS_CODESIGN_H_

/* code signing attributes of a process */
#define	CS_VALID		0x0000001	/* dynamically valid */
#define CS_ADHOC		0x0000002	/* ad hoc signed */
#define CS_GET_TASK_ALLOW	0x0000004	/* has get-task-allow entitlement */
#define CS_INSTALLER		0x0000008	/* has installer entitlement */

#define	CS_HARD			0x0000100	/* don't load invalid pages */
#define	CS_KILL			0x0000200	/* kill process if it becomes invalid */
#define CS_CHECK_EXPIRATION	0x0000400	/* force expiration checking */
#define CS_RESTRICT		0x0000800	/* tell dyld to treat restricted */
#define CS_ENFORCEMENT		0x0001000	/* require enforcement */
#define CS_REQUIRE_LV		0x0002000	/* require library validation */
#define CS_ENTITLEMENTS_VALIDATED	0x0004000	/* code signature permits restricted entitlements */

#define	CS_ALLOWED_MACHO	(CS_ADHOC | CS_HARD | CS_KILL | CS_CHECK_EXPIRATION | CS_RESTRICT | CS_ENFORCEMENT | CS_REQUIRE_LV)

#define CS_EXEC_SET_HARD	0x0100000	/* set CS_HARD on any exec'ed process */
#define CS_EXEC_SET_KILL	0x0200000	/* set CS_KILL on any exec'ed process */
#define CS_EXEC_SET_ENFORCEMENT	0x0400000	/* set CS_ENFORCEMENT on any exec'ed process */
#define CS_EXEC_SET_INSTALLER	0x0800000	/* set CS_INSTALLER on any exec'ed process */

#define CS_KILLED		0x1000000	/* was killed by kernel for invalidity */
#define CS_DYLD_PLATFORM	0x2000000	/* dyld used to load this is a platform binary */
#define CS_PLATFORM_BINARY	0x4000000	/* this is a platform binary */
#define CS_PLATFORM_PATH	0x8000000	/* platform binary by the fact of path (osx only) */
#define CS_DEBUGGED     	0x10000000  /* process is currently or has previously been debugged and allowed to run with invalid pages */
#define CS_SIGNED           0x20000000  /* process has a signature (may have gone invalid) */
#define CS_DEV_CODE         0x40000000  /* code is dev signed, cannot be loaded into prod signed code (will go away with rdar://problem/28322552) */
	
#define CS_ENTITLEMENT_FLAGS	(CS_GET_TASK_ALLOW | CS_INSTALLER)

/* MAC flags used by F_ADDFILESIGS_* */
#define MAC_VNODE_CHECK_DYLD_SIM 0x1   /* tells the MAC framework that dyld-sim is being loaded */

/* csops  operations */
#define	CS_OPS_STATUS		0	/* return status */
#define	CS_OPS_MARKINVALID	1	/* invalidate process */
#define	CS_OPS_MARKHARD		2	/* set HARD flag */
#define	CS_OPS_MARKKILL		3	/* set KILL flag (sticky) */
#ifdef KERNEL_PRIVATE
/* CS_OPS_PIDPATH		4	*/
#endif
#define	CS_OPS_CDHASH		5	/* get code directory hash */
#define CS_OPS_PIDOFFSET	6	/* get offset of active Mach-o slice */
#define CS_OPS_ENTITLEMENTS_BLOB 7	/* get entitlements blob */
#define CS_OPS_MARKRESTRICT	8	/* set RESTRICT flag (sticky) */
#define CS_OPS_SET_STATUS	9	/* set codesign flags */
#define CS_OPS_BLOB		10	/* get codesign blob */
#define CS_OPS_IDENTITY		11	/* get codesign identity */
#define CS_OPS_CLEARINSTALLER	12	/* clear INSTALLER flag */

/*
 * Magic numbers used by Code Signing
 */
enum {
	CSMAGIC_REQUIREMENT = 0xfade0c00,		/* single Requirement blob */
	CSMAGIC_REQUIREMENTS = 0xfade0c01,		/* Requirements vector (internal requirements) */
	CSMAGIC_CODEDIRECTORY = 0xfade0c02,		/* CodeDirectory blob */
	CSMAGIC_EMBEDDED_SIGNATURE = 0xfade0cc0, /* embedded form of signature data */
	CSMAGIC_EMBEDDED_SIGNATURE_OLD = 0xfade0b02,	/* XXX */
	CSMAGIC_EMBEDDED_ENTITLEMENTS = 0xfade7171,	/* embedded entitlements */
	CSMAGIC_DETACHED_SIGNATURE = 0xfade0cc1, /* multi-arch collection of embedded signatures */
	CSMAGIC_BLOBWRAPPER = 0xfade0b01,	/* CMS Signature, among other things */
	
	CS_SUPPORTSSCATTER = 0x20100,
	CS_SUPPORTSTEAMID = 0x20200,

	CSSLOT_CODEDIRECTORY = 0,				/* slot index for CodeDirectory */
	CSSLOT_INFOSLOT = 1,
	CSSLOT_REQUIREMENTS = 2,
	CSSLOT_RESOURCEDIR = 3,
	CSSLOT_APPLICATION = 4,
	CSSLOT_ENTITLEMENTS = 5,

	CSSLOT_ALTERNATE_CODEDIRECTORIES = 0x1000, /* first alternate CodeDirectory, if any */
	CSSLOT_ALTERNATE_CODEDIRECTORY_MAX = 5,		/* max number of alternate CD slots */
	CSSLOT_ALTERNATE_CODEDIRECTORY_LIMIT = CSSLOT_ALTERNATE_CODEDIRECTORIES + CSSLOT_ALTERNATE_CODEDIRECTORY_MAX, /* one past the last */

	CSSLOT_SIGNATURESLOT = 0x10000,			/* CMS Signature */

	CSTYPE_INDEX_REQUIREMENTS = 0x00000002,		/* compat with amfi */
	CSTYPE_INDEX_ENTITLEMENTS = 0x00000005,		/* compat with amfi */

	CS_HASHTYPE_SHA1 = 1,
	CS_HASHTYPE_SHA256 = 2,
	CS_HASHTYPE_SHA256_TRUNCATED = 3,
	CS_HASHTYPE_SHA384 = 4,

	CS_SHA1_LEN = 20,
	CS_SHA256_TRUNCATED_LEN = 20,

	CS_CDHASH_LEN = 20,						/* always - larger hashes are truncated */
	CS_HASH_MAX_SIZE = 48, /* max size of the hash we'll support */
};


#define KERNEL_HAVE_CS_CODEDIRECTORY 1
#define KERNEL_CS_CODEDIRECTORY_HAVE_PLATFORM 1

/*
 * C form of a CodeDirectory.
 */
typedef struct __CodeDirectory {
	uint32_t magic;					/* magic number (CSMAGIC_CODEDIRECTORY) */
	uint32_t length;				/* total length of CodeDirectory blob */
	uint32_t version;				/* compatibility version */
	uint32_t flags;					/* setup and mode flags */
	uint32_t hashOffset;			/* offset of hash slot element at index zero */
	uint32_t identOffset;			/* offset of identifier string */
	uint32_t nSpecialSlots;			/* number of special hash slots */
	uint32_t nCodeSlots;			/* number of ordinary (code) hash slots */
	uint32_t codeLimit;				/* limit to main image signature range */
	uint8_t hashSize;				/* size of each hash in bytes */
	uint8_t hashType;				/* type of hash (cdHashType* constants) */
	uint8_t platform;				/* platform identifier; zero if not platform binary */
	uint8_t	pageSize;				/* log2(page size in bytes); 0 => infinite */
	uint32_t spare2;				/* unused (must be zero) */
	/* Version 0x20100 */
	uint32_t scatterOffset;				/* offset of optional scatter vector */
	/* Version 0x20200 */
	uint32_t teamOffset;				/* offset of optional team identifier */
	/* followed by dynamic content as located by offset fields above */
} CS_CodeDirectory;

/*
 * Structure of an embedded-signature SuperBlob
 */

typedef struct __BlobIndex {
	uint32_t type;					/* type of entry */
	uint32_t offset;				/* offset of entry */
} CS_BlobIndex;

typedef struct __SC_SuperBlob {
	uint32_t magic;					/* magic number */
	uint32_t length;				/* total length of SuperBlob */
	uint32_t count;					/* number of index entries following */
	CS_BlobIndex index[];			/* (count) entries */
	/* followed by Blobs in no particular order as indicated by offsets in index */
} CS_SuperBlob;

#define KERNEL_HAVE_CS_GENERICBLOB 1
typedef struct __SC_GenericBlob {
	uint32_t magic;				/* magic number */
	uint32_t length;			/* total length of blob */
	char data[];
} CS_GenericBlob;

typedef struct __SC_Scatter {
	uint32_t count;			// number of pages; zero for sentinel (only)
	uint32_t base;			// first page number
	uint64_t targetOffset;		// offset in target
	uint64_t spare;			// reserved
} SC_Scatter;


#ifndef KERNEL

#include <sys/types.h>
#include <mach/message.h>

__BEGIN_DECLS
/* code sign operations */
int csops(pid_t pid, unsigned int  ops, void * useraddr, size_t usersize);
int csops_audittoken(pid_t pid, unsigned int  ops, void * useraddr, size_t usersize, audit_token_t * token);
__END_DECLS

#else /* !KERNEL */

#include <mach/vm_types.h>

#include <sys/cdefs.h>
#include <sys/_types/_off_t.h>

struct vnode;
struct cs_blob;
struct fileglob;

__BEGIN_DECLS
int	cs_valid(struct proc *);
int	cs_enforcement(struct proc *);
int	cs_require_lv(struct proc *);
int	cs_system_require_lv(void);
uint32_t cs_entitlement_flags(struct proc *p);
int	cs_entitlements_blob_get(struct proc *, void **, size_t *);
int	cs_restricted(struct proc *);
uint8_t * cs_get_cdhash(struct proc *);

struct cs_blob * csproc_get_blob(struct proc *);
struct cs_blob * csvnode_get_blob(struct vnode *, off_t);
void		 csvnode_print_debug(struct vnode *);

off_t			csblob_get_base_offset(struct cs_blob *);
vm_size_t		csblob_get_size(struct cs_blob *);
vm_address_t	csblob_get_addr(struct cs_blob *);
const char *	csblob_get_teamid(struct cs_blob *);
const char *	csblob_get_identity(struct cs_blob *);
const uint8_t *	csblob_get_cdhash(struct cs_blob *);
int				csblob_get_platform_binary(struct cs_blob *);
unsigned int	csblob_get_flags(struct cs_blob *blob);

int		csblob_get_entitlements(struct cs_blob *, void **, size_t *);

const CS_GenericBlob *
		csblob_find_blob(struct cs_blob *, uint32_t, uint32_t);
const CS_GenericBlob *
		csblob_find_blob_bytes(const uint8_t *, size_t, uint32_t, uint32_t);
void *          csblob_entitlements_dictionary_copy(struct cs_blob *csblob);
void            csblob_entitlements_dictionary_set(struct cs_blob *csblob, void * entitlements);

/*
 * Mostly convenience functions below
 */

const 	char * csproc_get_teamid(struct proc *);
const 	char * csvnode_get_teamid(struct vnode *, off_t);
int 	csproc_get_platform_binary(struct proc *);
int csproc_get_prod_signed(struct proc *);
const 	char * csfg_get_teamid(struct fileglob *);
int	csfg_get_path(struct fileglob *, char *, int *);
int 	csfg_get_platform_binary(struct fileglob *);
uint8_t * csfg_get_cdhash(struct fileglob *, uint64_t, size_t *);
int csfg_get_prod_signed(struct fileglob *);

extern int cs_debug;

#ifdef XNU_KERNEL_PRIVATE

void	cs_init(void);
int	cs_allow_invalid(struct proc *);
int	cs_invalid_page(addr64_t vaddr, boolean_t *cs_killed);
int	csproc_get_platform_path(struct proc *);

#if !SECURE_KERNEL
extern int cs_enforcement_panic;
#endif

#endif /* XNU_KERNEL_PRIVATE */


__END_DECLS



#endif /* KERNEL */

#endif /* _SYS_CODESIGN_H_ */

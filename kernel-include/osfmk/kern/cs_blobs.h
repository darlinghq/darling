/*
 * Copyright (c) 2017 Apple Computer, Inc. All rights reserved.
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

#ifndef _KERN_CODESIGN_H_
#define _KERN_CODESIGN_H_

/* code signing attributes of a process */
#define CS_VALID                    0x00000001  /* dynamically valid */
#define CS_ADHOC                    0x00000002  /* ad hoc signed */
#define CS_GET_TASK_ALLOW           0x00000004  /* has get-task-allow entitlement */
#define CS_INSTALLER                0x00000008  /* has installer entitlement */

#define CS_FORCED_LV                0x00000010  /* Library Validation required by Hardened System Policy */
#define CS_INVALID_ALLOWED          0x00000020  /* (macOS Only) Page invalidation allowed by task port policy */

#define CS_HARD                     0x00000100  /* don't load invalid pages */
#define CS_KILL                     0x00000200  /* kill process if it becomes invalid */
#define CS_CHECK_EXPIRATION         0x00000400  /* force expiration checking */
#define CS_RESTRICT                 0x00000800  /* tell dyld to treat restricted */

#define CS_ENFORCEMENT              0x00001000  /* require enforcement */
#define CS_REQUIRE_LV               0x00002000  /* require library validation */
#define CS_ENTITLEMENTS_VALIDATED   0x00004000  /* code signature permits restricted entitlements */
#define CS_NVRAM_UNRESTRICTED       0x00008000  /* has com.apple.rootless.restricted-nvram-variables.heritable entitlement */

#define CS_RUNTIME                                      0x00010000  /* Apply hardened runtime policies */

#define CS_ALLOWED_MACHO            (CS_ADHOC | CS_HARD | CS_KILL | CS_CHECK_EXPIRATION | \
	                             CS_RESTRICT | CS_ENFORCEMENT | CS_REQUIRE_LV | CS_RUNTIME)

#define CS_EXEC_SET_HARD            0x00100000  /* set CS_HARD on any exec'ed process */
#define CS_EXEC_SET_KILL            0x00200000  /* set CS_KILL on any exec'ed process */
#define CS_EXEC_SET_ENFORCEMENT     0x00400000  /* set CS_ENFORCEMENT on any exec'ed process */
#define CS_EXEC_INHERIT_SIP         0x00800000  /* set CS_INSTALLER on any exec'ed process */

#define CS_KILLED                   0x01000000  /* was killed by kernel for invalidity */
#define CS_DYLD_PLATFORM            0x02000000  /* dyld used to load this is a platform binary */
#define CS_PLATFORM_BINARY          0x04000000  /* this is a platform binary */
#define CS_PLATFORM_PATH            0x08000000  /* platform binary by the fact of path (osx only) */

#define CS_DEBUGGED                 0x10000000  /* process is currently or has previously been debugged and allowed to run with invalid pages */
#define CS_SIGNED                   0x20000000  /* process has a signature (may have gone invalid) */
#define CS_DEV_CODE                 0x40000000  /* code is dev signed, cannot be loaded into prod signed code (will go away with rdar://problem/28322552) */
#define CS_DATAVAULT_CONTROLLER     0x80000000  /* has Data Vault controller entitlement */

#define CS_ENTITLEMENT_FLAGS        (CS_GET_TASK_ALLOW | CS_INSTALLER | CS_DATAVAULT_CONTROLLER | CS_NVRAM_UNRESTRICTED)

/* executable segment flags */

#define CS_EXECSEG_MAIN_BINARY          0x1                     /* executable segment denotes main binary */
#define CS_EXECSEG_ALLOW_UNSIGNED       0x10            /* allow unsigned pages (for debugging) */
#define CS_EXECSEG_DEBUGGER                     0x20            /* main binary is debugger */
#define CS_EXECSEG_JIT                          0x40            /* JIT enabled */
#define CS_EXECSEG_SKIP_LV                      0x80            /* OBSOLETE: skip library validation */
#define CS_EXECSEG_CAN_LOAD_CDHASH      0x100           /* can bless cdhash for execution */
#define CS_EXECSEG_CAN_EXEC_CDHASH      0x200           /* can execute blessed cdhash */

/*
 * Magic numbers used by Code Signing
 */
enum {
	CSMAGIC_REQUIREMENT = 0xfade0c00,               /* single Requirement blob */
	CSMAGIC_REQUIREMENTS = 0xfade0c01,              /* Requirements vector (internal requirements) */
	CSMAGIC_CODEDIRECTORY = 0xfade0c02,             /* CodeDirectory blob */
	CSMAGIC_EMBEDDED_SIGNATURE = 0xfade0cc0, /* embedded form of signature data */
	CSMAGIC_EMBEDDED_SIGNATURE_OLD = 0xfade0b02,    /* XXX */
	CSMAGIC_EMBEDDED_ENTITLEMENTS = 0xfade7171,     /* embedded entitlements */
	CSMAGIC_DETACHED_SIGNATURE = 0xfade0cc1, /* multi-arch collection of embedded signatures */
	CSMAGIC_BLOBWRAPPER = 0xfade0b01,       /* CMS Signature, among other things */

	CS_SUPPORTSSCATTER = 0x20100,
	CS_SUPPORTSTEAMID = 0x20200,
	CS_SUPPORTSCODELIMIT64 = 0x20300,
	CS_SUPPORTSEXECSEG = 0x20400,

	CSSLOT_CODEDIRECTORY = 0,                               /* slot index for CodeDirectory */
	CSSLOT_INFOSLOT = 1,
	CSSLOT_REQUIREMENTS = 2,
	CSSLOT_RESOURCEDIR = 3,
	CSSLOT_APPLICATION = 4,
	CSSLOT_ENTITLEMENTS = 5,

	CSSLOT_ALTERNATE_CODEDIRECTORIES = 0x1000, /* first alternate CodeDirectory, if any */
	CSSLOT_ALTERNATE_CODEDIRECTORY_MAX = 5,         /* max number of alternate CD slots */
	CSSLOT_ALTERNATE_CODEDIRECTORY_LIMIT = CSSLOT_ALTERNATE_CODEDIRECTORIES + CSSLOT_ALTERNATE_CODEDIRECTORY_MAX, /* one past the last */

	CSSLOT_SIGNATURESLOT = 0x10000,                 /* CMS Signature */
	CSSLOT_IDENTIFICATIONSLOT = 0x10001,
	CSSLOT_TICKETSLOT = 0x10002,

	CSTYPE_INDEX_REQUIREMENTS = 0x00000002,         /* compat with amfi */
	CSTYPE_INDEX_ENTITLEMENTS = 0x00000005,         /* compat with amfi */

	CS_HASHTYPE_SHA1 = 1,
	CS_HASHTYPE_SHA256 = 2,
	CS_HASHTYPE_SHA256_TRUNCATED = 3,
	CS_HASHTYPE_SHA384 = 4,

	CS_SHA1_LEN = 20,
	CS_SHA256_LEN = 32,
	CS_SHA256_TRUNCATED_LEN = 20,

	CS_CDHASH_LEN = 20,                                             /* always - larger hashes are truncated */
	CS_HASH_MAX_SIZE = 48, /* max size of the hash we'll support */

/*
 * Currently only to support Legacy VPN plugins, and Mac App Store
 * but intended to replace all the various platform code, dev code etc. bits.
 */
	CS_SIGNER_TYPE_UNKNOWN = 0,
	CS_SIGNER_TYPE_LEGACYVPN = 5,
	CS_SIGNER_TYPE_MAC_APP_STORE = 6,
};

#define KERNEL_HAVE_CS_CODEDIRECTORY 1
#define KERNEL_CS_CODEDIRECTORY_HAVE_PLATFORM 1

/*
 * C form of a CodeDirectory.
 */
typedef struct __CodeDirectory {
	uint32_t magic;                                 /* magic number (CSMAGIC_CODEDIRECTORY) */
	uint32_t length;                                /* total length of CodeDirectory blob */
	uint32_t version;                               /* compatibility version */
	uint32_t flags;                                 /* setup and mode flags */
	uint32_t hashOffset;                    /* offset of hash slot element at index zero */
	uint32_t identOffset;                   /* offset of identifier string */
	uint32_t nSpecialSlots;                 /* number of special hash slots */
	uint32_t nCodeSlots;                    /* number of ordinary (code) hash slots */
	uint32_t codeLimit;                             /* limit to main image signature range */
	uint8_t hashSize;                               /* size of each hash in bytes */
	uint8_t hashType;                               /* type of hash (cdHashType* constants) */
	uint8_t platform;                               /* platform identifier; zero if not platform binary */
	uint8_t pageSize;                               /* log2(page size in bytes); 0 => infinite */
	uint32_t spare2;                                /* unused (must be zero) */

	char end_earliest[0];

	/* Version 0x20100 */
	uint32_t scatterOffset;                 /* offset of optional scatter vector */
	char end_withScatter[0];

	/* Version 0x20200 */
	uint32_t teamOffset;                    /* offset of optional team identifier */
	char end_withTeam[0];

	/* Version 0x20300 */
	uint32_t spare3;                                /* unused (must be zero) */
	uint64_t codeLimit64;                   /* limit to main image signature range, 64 bits */
	char end_withCodeLimit64[0];

	/* Version 0x20400 */
	uint64_t execSegBase;                   /* offset of executable segment */
	uint64_t execSegLimit;                  /* limit of executable segment */
	uint64_t execSegFlags;                  /* executable segment flags */
	char end_withExecSeg[0];

	/* followed by dynamic content as located by offset fields above */
} CS_CodeDirectory
__attribute__ ((aligned(1)));

/*
 * Structure of an embedded-signature SuperBlob
 */

typedef struct __BlobIndex {
	uint32_t type;                                  /* type of entry */
	uint32_t offset;                                /* offset of entry */
} CS_BlobIndex
__attribute__ ((aligned(1)));

typedef struct __SC_SuperBlob {
	uint32_t magic;                                 /* magic number */
	uint32_t length;                                /* total length of SuperBlob */
	uint32_t count;                                 /* number of index entries following */
	CS_BlobIndex index[];                   /* (count) entries */
	/* followed by Blobs in no particular order as indicated by offsets in index */
} CS_SuperBlob
__attribute__ ((aligned(1)));

#define KERNEL_HAVE_CS_GENERICBLOB 1
typedef struct __SC_GenericBlob {
	uint32_t magic;                                 /* magic number */
	uint32_t length;                                /* total length of blob */
	char data[];
} CS_GenericBlob
__attribute__ ((aligned(1)));

typedef struct __SC_Scatter {
	uint32_t count;                                 // number of pages; zero for sentinel (only)
	uint32_t base;                                  // first page number
	uint64_t targetOffset;                  // offset in target
	uint64_t spare;                                 // reserved
} SC_Scatter
__attribute__ ((aligned(1)));


#endif /* _KERN_CODESIGN_H */

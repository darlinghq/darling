/*
 * Copyright (c) 2009 Apple Inc. All rights reserved.
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

#ifndef _SYS_CPROTECT_H_
#define	_SYS_CPROTECT_H_

#ifdef __cplusplus
extern "C" {
#endif

#if KERNEL_PRIVATE

#include <sys/cdefs.h>
#include <sys/content_protection.h>
#include <sys/kernel_types.h>
#include <crypto/aes.h>

#define CP_IV_KEYSIZE 20			/* 16x8 = 128, but SHA1 pushes 20 bytes so keep space for that */
#define CP_MAX_KEYSIZE 32		/* 8x4 = 32, 32x8 = 256 */
#define CP_MAX_WRAPPEDKEYSIZE  128	/* The size of the largest allowed key */
#define CP_INITIAL_WRAPPEDKEYSIZE 40
#define CP_V2_WRAPPEDKEYSIZE 40		/* Size of the wrapped key in a v2 EA */

/* lock events from AppleKeyStore */
#define CP_LOCKED_STATE 0		/* Device is locked */
#define CP_UNLOCKED_STATE 1		/* Device is unlocked */

#define CP_LOCKED_KEYCHAIN 0	
#define CP_UNLOCKED_KEYCHAIN 1

/* For struct cprotect: cp_flags */
#define CP_NEEDS_KEYS		0x1		/* File needs persistent keys */
#define CP_KEY_FLUSHED		0x2		/* File's unwrapped key has been purged from memory */
#define CP_NO_XATTR			0x4		/* Key info has not been saved as EA to the FS */
#define CP_OFF_IV_ENABLED  	0x8		/* Only go down relative IV route if this flag is set */

#define CP_RELOCATION_INFLIGHT 0x10	/* File with offset IVs is in the process of being relocated. */

/* Content Protection VNOP Operation flags */
#define CP_READ_ACCESS 	0x1
#define CP_WRITE_ACCESS 0x2

/* 
 * Check for this version when deciding to enable features
 * For Apex, CP_CURRENT_MAJOR_VERS = 2.0
 * For Telluride, CP_CURRENT_MAJOR_VERS = 4.0
 */
#define CONTENT_PROTECTION_XATTR_NAME	"com.apple.system.cprotect"
#define CP_NEW_MAJOR_VERS 4
#define CP_PREV_MAJOR_VERS 2
#define CP_MINOR_VERS 0

typedef struct cprotect *cprotect_t;
typedef struct cp_wrap_func *cp_wrap_func_t;
typedef struct cp_global_state *cp_global_state_t;
typedef struct cp_xattr *cp_xattr_t;

typedef struct cnode * cnode_ptr_t;
//forward declare the struct.
struct hfsmount;

/* The wrappers are invoked by the AKS kext */
typedef int wrapper_t(uint32_t properties, uint64_t file_id, void *key_bytes, size_t key_length, void *wrapped_data, size_t *wrapped_length);
typedef	int unwrapper_t(uint32_t properties, void *wrapped_data, size_t wrapped_data_length, void *key_bytes, size_t *key_length);

/* 
 * Runtime-only structure containing the content protection status 
 * for the given file.  This is contained within the cnode 
 * This is passed down to IOStorageFamily via the bufattr struct
 *
 ******************************************************
 * Some Key calculation information for offset based IV
 ******************************************************
 * Kf  = original 256 bit per file key
 * Kiv = SHA1(Kf), use full Kf, but truncate Kiv to 128 bits
 * Kiv can be cached in the cprotect, so it only has to be calculated once for the file init
 *
 * IVb = Encrypt(Kiv, offset)
 *
 */
struct cprotect {
	uint32_t	cp_flags;
	uint32_t	cp_pclass;
	aes_encrypt_ctx	cp_cache_iv_ctx;
	uint32_t	cp_cache_key_len;
	uint8_t		cp_cache_key[CP_MAX_KEYSIZE];
	uint32_t	cp_persistent_key_len;
	uint8_t		cp_persistent_key[];
};

struct cp_wrap_func {
	wrapper_t	*wrapper;
	unwrapper_t	*unwrapper;
};

struct cp_global_state {
	uint8_t	wrap_functions_set;
	uint8_t	lock_state;
	u_int16_t reserved;
};

/*
 * On-disk structure written as the per-file EA payload 
 * All on-disk multi-byte fields for the CP XATTR must be stored
 * little-endian on-disk.  This means they must be endian swapped to
 * L.E on getxattr() and converted to LE on setxattr().	
 */
struct cp_xattr_v2 {
	u_int16_t xattr_major_version;
	u_int16_t xattr_minor_version;
	u_int32_t flags;
	u_int32_t persistent_class;
	u_int32_t key_size;
	uint8_t   persistent_key[CP_V2_WRAPPEDKEYSIZE];
};

struct cp_xattr_v4 {
	u_int16_t xattr_major_version;
	u_int16_t xattr_minor_version;
	u_int32_t flags;
	u_int32_t persistent_class;
	u_int32_t key_size;
	u_int32_t reserved1;
	u_int32_t reserved2;
	u_int32_t reserved3;
	u_int32_t reserved4;
	u_int32_t reserved5;
	uint8_t   persistent_key[CP_MAX_WRAPPEDKEYSIZE];
};

/* Same is true for the root EA, all fields must be written little endian. */
struct cp_root_xattr {
	u_int16_t major_version;
	u_int16_t minor_version;
	u_int64_t flags;
	u_int32_t reserved1;
	u_int32_t reserved2;
	u_int32_t reserved3;
	u_int32_t reserved4;
};


/* 
 * Functions to check the status of a CP and to query 
 * the containing filesystem to see if it is supported.
 */
int cp_vnode_getclass(vnode_t, int *);
int cp_vnode_setclass(vnode_t, uint32_t);
int cp_vnode_transcode(vnode_t);

int cp_key_store_action(int);
int cp_register_wraps(cp_wrap_func_t);

int cp_entry_init(cnode_ptr_t, struct mount *);
int cp_entry_create_keys(struct cprotect **entry_ptr, struct cnode *dcp, struct hfsmount *hfsmp, 
		uint32_t input_class, uint32_t fileid, mode_t cmode);
int cp_entry_gentempkeys(struct cprotect **entry_ptr, struct hfsmount *hfsmp);
void cp_entry_destroy(struct cprotect **entry_ptr);

cnode_ptr_t cp_get_protected_cnode(vnode_t);
int cp_handle_vnop(vnode_t, int, int);
int cp_fs_protected (mount_t);
int cp_getrootxattr (struct hfsmount *hfsmp, struct cp_root_xattr *outxattr);
int cp_setrootxattr (struct hfsmount *hfsmp, struct cp_root_xattr *newxattr);
int cp_setxattr(struct cnode *cp, struct cprotect *entry, struct hfsmount *hfsmp, uint32_t fileid, int options);
int cp_update_mkb (struct cprotect *entry, uint32_t fileid);
int cp_handle_relocate (cnode_ptr_t cp, struct hfsmount *hfsmp);
int cp_handle_open(struct vnode *vp, int mode);
int cp_get_root_major_vers (struct vnode *vp, uint32_t *level);

#if 0
int cp_isdevice_locked (void);
#endif

#endif	/* KERNEL_PRIVATE */

#ifdef __cplusplus
};
#endif

#endif /* !_SYS_CPROTECT_H_ */

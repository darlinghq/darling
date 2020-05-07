/*
 * Copyright (c) 2000-2013 Apple Inc. All rights reserved.
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

#include <kern/kern_types.h>
#include <kern/locks.h>
#include <kern/kalloc.h>
#include <vm/vm_kern.h>
#include <mach/kern_return.h>
#include <kern/queue.h>
#include <vm/vm_pageout.h>
#include <vm/vm_protos.h>
#include <vm/vm_compressor.h>
#include <libkern/crypto/aes.h>
#include <kern/host_statistics.h>

#if CONFIG_EMBEDDED

#define MIN_SWAP_FILE_SIZE              (64 * 1024 * 1024)

#define MAX_SWAP_FILE_SIZE              (128 * 1024 * 1024)

#else /* CONFIG_EMBEDDED */

#define MIN_SWAP_FILE_SIZE              (256 * 1024 * 1024)

#define MAX_SWAP_FILE_SIZE              (1 * 1024 * 1024 * 1024)

#endif /* CONFIG_EMBEDDED */

#define COMPRESSED_SWAP_CHUNK_SIZE      (C_SEG_BUFSIZE)

#define VM_SWAPFILE_HIWATER_SEGS        (MIN_SWAP_FILE_SIZE / COMPRESSED_SWAP_CHUNK_SIZE)

#define SWAPFILE_RECLAIM_THRESHOLD_SEGS ((17 * (MAX_SWAP_FILE_SIZE / COMPRESSED_SWAP_CHUNK_SIZE)) / 10)
#define SWAPFILE_RECLAIM_MINIMUM_SEGS   ((13 * (MAX_SWAP_FILE_SIZE / COMPRESSED_SWAP_CHUNK_SIZE)) / 10)


#define SWAP_FILE_NAME          "/private/var/vm/swapfile"
#define SWAPFILENAME_LEN        (int)(strlen(SWAP_FILE_NAME))


#define SWAP_SLOT_MASK          0x1FFFFFFFF
#define SWAP_DEVICE_SHIFT       33

extern int              vm_num_swap_files;

struct swapfile;
lck_grp_attr_t  vm_swap_data_lock_grp_attr;
lck_grp_t       vm_swap_data_lock_grp;
lck_attr_t      vm_swap_data_lock_attr;
lck_mtx_ext_t   vm_swap_data_lock_ext;
lck_mtx_t       vm_swap_data_lock;

void vm_swap_init(void);
boolean_t vm_swap_create_file(void);


struct swapout_io_completion {
	int          swp_io_busy;
	int          swp_io_done;
	int          swp_io_error;

	uint32_t     swp_c_size;
	c_segment_t  swp_c_seg;

	struct swapfile *swp_swf;
	uint64_t        swp_f_offset;

	struct upl_io_completion swp_upl_ctx;
};
void vm_swapout_iodone(void *, int);


static void vm_swapout_finish(c_segment_t, uint64_t, uint32_t, kern_return_t);
kern_return_t vm_swap_put_finish(struct swapfile *, uint64_t *, int);
kern_return_t vm_swap_put(vm_offset_t, uint64_t*, uint32_t, c_segment_t, struct swapout_io_completion *);

void vm_swap_flush(void);
void vm_swap_reclaim(void);
void vm_swap_encrypt(c_segment_t);
uint64_t vm_swap_get_total_space(void);
uint64_t vm_swap_get_used_space(void);
uint64_t vm_swap_get_free_space(void);

struct vnode;
extern void vm_swapfile_open(const char *path, struct vnode **vp);
extern void vm_swapfile_close(uint64_t path, struct vnode *vp);
extern int vm_swapfile_preallocate(struct vnode *vp, uint64_t *size, boolean_t *pin);
extern uint64_t vm_swapfile_get_blksize(struct vnode *vp);
extern uint64_t vm_swapfile_get_transfer_size(struct vnode *vp);
extern int vm_swapfile_io(struct vnode *vp, uint64_t offset, uint64_t start, int npages, int flags, void *upl_ctx);

#if CONFIG_FREEZE
boolean_t vm_swap_max_budget(uint64_t *);
int vm_swap_vol_get_budget(struct vnode* vp, uint64_t *freeze_daily_budget);
#endif /* CONFIG_FREEZE */

#if RECORD_THE_COMPRESSED_DATA
extern int vm_record_file_write(struct vnode *vp, uint64_t offset, char *buf, int size);
#endif

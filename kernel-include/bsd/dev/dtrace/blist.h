/*
 * Copyright (c) 1998 Matthew Dillon.  Terms of use and redistribution in all
 * forms are covered by the BSD copyright in the file "/usr/src/COPYRIGHT".
 *
 * Implements bitmap resource lists.
 *
 *	Usage:
 *		blist = blist_create(blocks)
 *		(void)  blist_destroy(blist)
 *		blkno = blist_alloc(blist, count)
 *		(void)  blist_free(blist, blkno, count)
 *		(void)  blist_resize(&blist, count, freeextra)
 *
 *
 *	Notes:
 *		on creation, the entire list is marked reserved.  You should
 *		first blist_free() the sections you want to make available
 *		for allocation before doing general blist_alloc()/free()
 *		ops.
 *
 *		SWAPBLK_NONE is returned on failure.  This module is typically
 *		capable of managing up to (2^31) blocks per blist, though
 *		the memory utilization would be insane if you actually did
 *		that.  Managing something like 512MB worth of 4K blocks
 *		eats around 32 KBytes of memory.
 *
 * $FreeBSD: src/sys/sys/blist.h,v 1.2 1999/08/28 00:51:33 peter Exp $
 */

#ifndef _SYS_BLIST_H_
#define _SYS_BLIST_H_

#define LOG2(v)         (((u_daddr_t)(v) >= 0x80000000U) ? 31 : \
	                ((u_daddr_t)(v) >= 0x40000000U) ? 30 : \
	                ((u_daddr_t)(v) >= 0x20000000U) ? 29 : \
	                ((u_daddr_t)(v) >= 0x10000000U) ? 28 : \
	                ((u_daddr_t)(v) >= 0x08000000U) ? 27 : \
	                ((u_daddr_t)(v) >= 0x04000000U) ? 26 : \
	                ((u_daddr_t)(v) >= 0x02000000U) ? 25 : \
	                ((u_daddr_t)(v) >= 0x01000000U) ? 24 : \
	                ((u_daddr_t)(v) >= 0x00800000U) ? 23 : \
	                ((u_daddr_t)(v) >= 0x00400000U) ? 22 : \
	                ((u_daddr_t)(v) >= 0x00200000U) ? 21 : \
	                ((u_daddr_t)(v) >= 0x00100000U) ? 20 : \
	                ((u_daddr_t)(v) >= 0x00080000U) ? 19 : \
	                ((u_daddr_t)(v) >= 0x00040000U) ? 18 : \
	                ((u_daddr_t)(v) >= 0x00020000U) ? 17 : \
	                ((u_daddr_t)(v) >= 0x00010000U) ? 16 : \
	                ((u_daddr_t)(v) >= 0x00008000U) ? 15 : \
	                ((u_daddr_t)(v) >= 0x00004000U) ? 14 : \
	                ((u_daddr_t)(v) >= 0x00002000U) ? 13 : \
	                ((u_daddr_t)(v) >= 0x00001000U) ? 12 : \
	                ((u_daddr_t)(v) >= 0x00000800U) ? 11 : \
	                ((u_daddr_t)(v) >= 0x00000400U) ? 10 : \
	                ((u_daddr_t)(v) >= 0x00000200U) ? 9 : \
	                ((u_daddr_t)(v) >= 0x00000100U) ? 8 : \
	                ((u_daddr_t)(v) >= 0x00000080U) ? 7 : \
	                ((u_daddr_t)(v) >= 0x00000040U) ? 6 : \
	                ((u_daddr_t)(v) >= 0x00000020U) ? 5 : \
	                ((u_daddr_t)(v) >= 0x00000010U) ? 4 : \
	                ((u_daddr_t)(v) >= 0x00000008U) ? 3 : \
	                ((u_daddr_t)(v) >= 0x00000004U) ? 2 : \
	                ((u_daddr_t)(v) >= 0x00000002U) ? 1 : 0)

/*
 * blmeta and bl_bitmap_t MUST be a power of 2 in size.
 */

typedef struct blmeta {
	union {
		daddr_t     bmu_avail;  /* space available under us	*/
		u_daddr_t   bmu_bitmap; /* bitmap if we are a leaf	*/
	} u;
	daddr_t         bm_bighint;     /* biggest contiguous block hint*/
} blmeta_t;

typedef struct blist {
	daddr_t         bl_blocks;      /* area of coverage		*/
	daddr_t         bl_radix;       /* coverage radix		*/
	daddr_t         bl_skip;        /* starting skip		*/
	daddr_t         bl_free;        /* number of free blocks	*/
	blmeta_t        *bl_root;       /* root of radix tree		*/
	daddr_t         bl_rootblks;    /* daddr_t blks allocated for tree */
} *blist_t;

#define BLIST_META_RADIX        16
#define BLIST_META_RADIX_SHIFT  LOG2(BLIST_META_RADIX)
#define BLIST_BMAP_RADIX        (sizeof(u_daddr_t)*8)
#define BLIST_BMAP_RADIX_SHIFT  LOG2(BLIST_BMAP_RADIX)

#define BLIST_MAX_ALLOC         BLIST_BMAP_RADIX

extern blist_t blist_create(daddr_t blocks);
extern void blist_destroy(blist_t blist);
extern daddr_t blist_alloc(blist_t blist, daddr_t count);
extern void blist_free(blist_t blist, daddr_t blkno, daddr_t count);
extern void blist_print(blist_t blist);
extern void blist_resize(blist_t *pblist, daddr_t count, int freenew);

#endif  /* _SYS_BLIST_H_ */

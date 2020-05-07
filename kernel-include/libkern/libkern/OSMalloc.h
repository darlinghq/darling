/*
 * Copyright (c) 2003-2004 Apple Computer, Inc. All rights reserved.
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

#ifndef LIBKERN_OSMALLOC_h
#define LIBKERN_OSMALLOC_h

#include <sys/cdefs.h>

__BEGIN_DECLS

#include <stdint.h>
#ifdef  MACH_KERNEL_PRIVATE
#include <kern/queue.h>
#endif

/*!
 * @header
 *
 * @abstract
 * This header declares the OSMalloc memory-allocation KPI.
 *
 * @discussion
 * Kernel extensions can use these functions to allocate and deallocate
 * memory blocks that are tracked under named tags.
 * A kernel extension can create whatever tags it needs,
 * but typically just creates one with its bundle identifier.
 *
 * Tags are required; attempting to use these functions without one
 * will result in a panic.
 *
 * <b>Use Restrictions</b>
 *
 * None of the OSMalloc functions are safe to call
 * in a primary interrupt handler.
 */

#ifdef  MACH_KERNEL_PRIVATE

#define OSMT_MAX_NAME  (64)

typedef struct _OSMallocTag_ {
	queue_chain_t   OSMT_link;
	uint32_t        OSMT_refcnt;
	uint32_t        OSMT_state;
	uint32_t        OSMT_attr;
	char            OSMT_name[OSMT_MAX_NAME];
} * OSMallocTag;

#define OSMT_VALID_MASK   0xFFFF0000
#define OSMT_VALID        0xDEAB0000
#define OSMT_RELEASED     0x00000001

/*! @parseOnly */
#define OSMT_ATTR_PAGEABLE  0x01

#else
/*!
 * @typedef OSMallocTag
 *
 * @abstract
 * An opaque type used to track memory allocations.
 */
typedef struct __OSMallocTag__ * OSMallocTag;


/*!
 * @typedef OSMallocTag_t
 *
 * @abstract
 * See <code>@link OSMallocTag OSMallocTag@/link</code>.
 */
typedef struct __OSMallocTag__ * OSMallocTag_t;
#endif

/*!
 * @define OSMT_DEFAULT
 *
 * @abstract
 * Indicates that an <code>@link OSMallocTag OSMallocTag@/link</code>
 * be created with default attributes.
 *
 * @discussion
 * An <code>@link OSMallocTag OSMallocTag@/link</code> created
 * with this attribute allocates all blocks in wired memory.
 */
#define OSMT_DEFAULT   0x00


/*!
 * @define OSMT_PAGEABLE
 *
 * @abstract
 * Indicates that an <code>@link OSMallocTag OSMallocTag@/link</code>
 * should allocate pageable memory when possible.
 *
 * @discussion
 * An <code>@link OSMallocTag OSMallocTag@/link</code> created
 * with this attribute allocates blocks of a full page size or larger
 * in pageable memory,
 * and blocks smaller than a full page size in wired memory.
 */
#define OSMT_PAGEABLE  0x01


/*!
 * @function OSMalloc_Tagalloc
 *
 * @abstract
 * Creates a tag for use with OSMalloc functions.
 *
 * @param name   The name of the tag to create.
 * @param flags  A bitmask that controls allocation behavior; see description.
 *
 * @result
 * An opaque tag to be used with OSMalloc functions for tracking memory usage.
 *
 * @discussion
 * OSMalloc tags can have arbitrary names of a length up to 63 characters.
 * Calling this function twice with the same name
 * creates two tags, which share that name.
 *
 * <code>flags</code> can be the bitwise OR of the following flags:
 * <ul>
 *    <li><code>@link OSMT_DEFAULT OSMT_DEFAULT@/link</code> -
 *        allocations are wired. This is the 'zero' bitmask value and
 *        is overridden by any other flag specified.</li>
 *    <li><code>@link OSMT_PAGEABLE OSMT_PAGEABLE@/link</code> -
 *        allocations of a full page size or greater are pageable;
 *        allocations smaller than a page are wired.</li>
 * </ul>
 */
extern OSMallocTag OSMalloc_Tagalloc(
	const char * name,
	uint32_t    flags);


/*!
 * @function OSMalloc_Tagfree
 *
 * @abstract
 * Frees a tag used with OSMalloc functions.
 *
 * @param tag  The <code>@link OSMallocTag OSMallocTag@/link</code> to free.
 *
 * @discussion
 * OSMalloc tags must not be freed
 * while any memory blocks allocated
 * with them still exist.
 * Any OSMalloc function called on those blocks
 * will result in a panic.
 */
extern void OSMalloc_Tagfree(OSMallocTag tag);


/*!
 * @function OSMalloc
 *
 * @abstract
 * Allocates a block of memory associated
 * with a given <code>@link OSMallocTag OSMallocTag@/link</code>.
 *
 * @param size  The size of the memory block to allocate.
 * @param tag   The <code>@link OSMallocTag OSMallocTag@/link</code>
 *              under which to allocate the memory.
 *
 * @result
 * A pointer to the memory on success, <code>NULL</code> on failure.
 *
 * @discussion
 * If <code>tag</code> was created with the
 * <code>@link OSMT_PAGEABLE OSMT_PAGEABLE@/link</code>
 * attribute <i>and</i> <code>size</code>
 * is a full page or larger, the allocated memory is pageable;
 * otherwise it is wired.
 */
extern void * OSMalloc(
	uint32_t    size,
	OSMallocTag tag) __attribute__((alloc_size(1)));


/*!
 * @function OSMalloc_nowait
 *
 * @abstract
 * Equivalent to <code>@link OSMalloc_noblock OSMalloc_noblock@/link</code>.
 */
extern void * OSMalloc_nowait(
	uint32_t    size,
	OSMallocTag tag) __attribute__((alloc_size(1)));


/*!
 * @function OSMalloc_noblock
 *
 * @abstract
 * Allocates a block of memory associated
 * with a given <code>@link OSMallocTag OSMallocTag@/link</code>,
 * returning <code>NULL</code> if it would block.
 *
 * @param size  The size of the memory block to allocate.
 * @param tag   The <code>@link OSMallocTag OSMallocTag@/link</code>
 *              under which to allocate the memory.
 *
 * @result
 * A pointer to the memory on success, <code>NULL</code> on failure
 * or if allocation would block.
 *
 * @discussion
 * If <code>tag</code> was created with the
 * <code>@link OSMT_PAGEABLE OSMT_PAGEABLE@/link</code>
 * attribute <i>and</i> <code>size</code>
 * is a full page or larger, the allocated memory is pageable;
 * otherwise it is wired.
 *
 * This function is guaranteed not to block.
 */
extern void * OSMalloc_noblock(
	uint32_t    size,
	OSMallocTag tag) __attribute__((alloc_size(1)));


/*!
 * @function OSFree
 *
 * @abstract
 * Frees a block of memory allocated by <code>@link OSMalloc OSMalloc@/link</code>.
 *
 * @param addr  A pointer to the memory block to free.
 * @param size  The size of the memory block to free.
 * @param tag   The <code>@link OSMallocTag OSMallocTag@/link</code>
 *              with which <code>addr</code> was originally allocated.
 */
extern void OSFree(
	void      * addr,
	uint32_t    size,
	OSMallocTag tag);

#ifdef XNU_KERNEL_PRIVATE
/*!
 * @function OSMalloc_size
 *
 * @abstract
 * Returns the size of a block of memory allocated by <code>@link OSMalloc OSMalloc@/link</code>.
 *
 * @param addr A pointer to the memory block allocated via OSMalloc.
 */
extern uint32_t OSMalloc_size(
	void    * addr);
#endif /* XNU_KERNEL_PRIVATE */

__END_DECLS

#endif  /* LIBKERN_OSMALLOC_h */

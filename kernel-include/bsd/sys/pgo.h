/*
 * Copyright (c) 2014 Apple Computer, Inc. All rights reserved.
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

#ifndef _SYS_PGO_H_
#define _SYS_PGO_H_

#include <sys/_types.h>
#include <sys/_types/_ssize_t.h>
#include <stdint.h>
#include <uuid/uuid.h>

#define PGO_HIB (1)
#define PGO_WAIT_FOR_UNLOAD (2)
#define PGO_METADATA (4)
#define PGO_RESET_ALL (8)

#define PGO_ALL_FLAGS (PGO_HIB | PGO_WAIT_FOR_UNLOAD | PGO_METADATA | PGO_RESET_ALL)


/**
 * This is a serialization format for metadata related to a profile data buffer.
 *
 * If metadata is present, this footer will appear at the end of the file, so
 * the last four bytes of the file will be the ASCII string "meta".
 *
 * The metadata is stored in a environment-string style buffer.  The buffer
 * consists of key-value pairs, which are delimited by null bytes.  Each
 * key-value pair is a string of the form "FOO=bar".  Everything before the
 * first equal sign is the key, everything after is the value.
 *
 * All members are in network byte order.
 */
struct pgo_metadata_footer {
	/**
	 * number of pairs.
	 *
	 * This should be htonl(n), where n is the number of key-value pairs in the
	 * metadata buffer
	 */
	uint32_t number_of_pairs;

	/**
	 * pointer to the metadata buffer
	 *
	 * This should be htonl(offset), where offset is the backwards offset from
	 * the end of the file to the metadata buffer.
	 */
	uint32_t  offset_to_pairs;

	/**
	 * magic number
	 *
	 * This should be  htonl(0x6d657461);
	 */
	uint32_t magic;
};

#ifndef KERNEL

ssize_t grab_pgo_data(
	uuid_t *uuid,
	int flags,
	unsigned char *buffer,
	ssize_t size);


#endif

#ifdef XNU_KERNEL_PRIVATE
kern_return_t do_pgo_reset_counters(void);
#endif


#endif

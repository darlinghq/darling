/*
 * Copyright (c) 2015-2017 Apple Computer, Inc. All rights reserved.
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

#ifndef _NET_IF_FAKE_VAR_H_
#define _NET_IF_FAKE_VAR_H_     1

#include <stdint.h>

#ifdef KERNEL_PRIVATE
__private_extern__ void
if_fake_init(void);
#endif /* KERNEL_PRIVATE */

/*
 * SIOCSDRVSPEC
 */
enum {
	IF_FAKE_S_CMD_NONE              = 0,
	IF_FAKE_S_CMD_SET_PEER          = 1,
	IF_FAKE_S_CMD_SET_MEDIA         = 2,
	IF_FAKE_S_CMD_SET_DEQUEUE_STALL = 3,
};

/*
 * SIOCGDRVSPEC
 */
enum {
	IF_FAKE_G_CMD_NONE              = 0,
	IF_FAKE_G_CMD_GET_PEER          = 1,
};

#define IF_FAKE_MEDIA_LIST_MAX  27

struct if_fake_media {
	int32_t         iffm_current;
	uint32_t        iffm_count;
	uint32_t        iffm_reserved[3];
	int32_t         iffm_list[IF_FAKE_MEDIA_LIST_MAX];
};

struct if_fake_request {
	uint64_t        iffr_reserved[4];
	union {
		char    iffru_buf[128];         /* stable size */
		struct if_fake_media    iffru_media;
		char    iffru_peer_name[IFNAMSIZ]; /* if name, e.g. "en0" */
		/*
		 * control dequeue stall. 0: disable dequeue stall, else
		 * enable dequeue stall.
		 */
		uint32_t        iffru_dequeue_stall;
	} iffr_u;
#define iffr_peer_name  iffr_u.iffru_peer_name
#define iffr_media      iffr_u.iffru_media
#define iffr_dequeue_stall      iffr_u.iffru_dequeue_stall
};

#endif /* _NET_IF_FAKE_VAR_H_ */

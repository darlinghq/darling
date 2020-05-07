/*
 * Copyright (c) 2008-2013 Apple Inc. All rights reserved.
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

/*	$FreeBSD: src/sys/netinet6/esp.h,v 1.2.2.2 2001/07/03 11:01:49 ume Exp $	*/
/*	$KAME: esp.h,v 1.16 2000/10/18 21:28:00 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * RFC1827/2406 Encapsulated Security Payload.
 */

#ifndef _NETINET6_ESP_H_
#define _NETINET6_ESP_H_
#include <sys/appleapiopts.h>
#include <net/multi_layer_pkt_log.h>

struct esp {
	u_int32_t       esp_spi;        /* ESP */
	/*variable size, 32bit bound*/	/* Initialization Vector */
	/*variable size*/		/* Payload data */
	/*variable size*/		/* padding */
	/*8bit*/			/* pad size */
	/*8bit*/			/* next header */
	/*8bit*/			/* next header */
	/*variable size, 32bit bound*/	/* Authentication data (new IPsec) */
};

struct newesp {
	u_int32_t       esp_spi;        /* ESP */
	u_int32_t       esp_seq;        /* Sequence number */
	/*variable size*/		/* (IV and) Payload data */
	/*variable size*/		/* padding */
	/*8bit*/			/* pad size */
	/*8bit*/			/* next header */
	/*8bit*/			/* next header */
	/*variable size, 32bit bound*/	/* Authentication data */
};

struct esptail {
	u_int8_t        esp_padlen;     /* pad length */
	u_int8_t        esp_nxt;        /* Next header */
	/*variable size, 32bit bound*/	/* Authentication data (new IPsec)*/
};

#ifdef BSD_KERNEL_PRIVATE
struct secasvar;

struct esp_algorithm {
	size_t padbound;        /* pad boundary, in byte */
	int ivlenval;           /* iv length, in byte */
	int (*mature)(struct secasvar *);
	int keymin;     /* in bits */
	int keymax;     /* in bits */
	int (*schedlen)(const struct esp_algorithm *);
	const char *name;
	int (*ivlen)(const struct esp_algorithm *, struct secasvar *);
	int (*decrypt)(struct mbuf *, size_t,
	    struct secasvar *, const struct esp_algorithm *, int);
	int (*encrypt)(struct mbuf *, size_t, size_t,
	    struct secasvar *, const struct esp_algorithm *, int);
	/* not supposed to be called directly */
	int (*schedule)(const struct esp_algorithm *, struct secasvar *);
	int (*blockdecrypt)(const struct esp_algorithm *,
	    struct secasvar *, u_int8_t *, u_int8_t *);
	int (*blockencrypt)(const struct esp_algorithm *,
	    struct secasvar *, u_int8_t *, u_int8_t *);
	/* For Authenticated Encryption Methods */
	size_t icvlen;
	int (*finalizedecrypt)(struct secasvar *, u_int8_t *, uint);
	int (*finalizeencrypt)(struct secasvar *, u_int8_t *, uint);
};

extern os_log_t esp_mpkl_log_object;

extern const struct esp_algorithm *esp_algorithm_lookup(int);
extern int esp_max_ivlen(void);

/* crypt routines */
extern int esp4_output(struct mbuf *, struct secasvar *);
extern void esp4_input(struct mbuf *, int off);
extern struct mbuf *esp4_input_extended(struct mbuf *, int off, ifnet_t interface);
extern size_t esp_hdrsiz(struct ipsecrequest *);

extern int esp_schedule(const struct esp_algorithm *, struct secasvar *);
extern int esp_auth(struct mbuf *, size_t, size_t,
    struct secasvar *, u_char *);

extern void esp_init(void);
#endif /* BSD_KERNEL_PRIVATE */

#endif /* _NETINET6_ESP_H_ */

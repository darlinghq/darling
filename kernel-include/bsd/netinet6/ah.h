/*	$FreeBSD: src/sys/netinet6/ah.h,v 1.3.2.2 2001/07/03 11:01:49 ume Exp $	*/
/*	$KAME: ah.h,v 1.13 2000/10/18 21:28:00 itojun Exp $	*/

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
 * RFC1826/2402 authentication header.
 */

#ifndef _NETINET6_AH_H_
#define _NETINET6_AH_H_
#include <sys/appleapiopts.h>

struct ah {
	u_int8_t        ah_nxt;         /* Next Header */
	u_int8_t        ah_len;         /* Length of data, in 32bit */
	u_int16_t       ah_reserve;     /* Reserved for future use */
	u_int32_t       ah_spi;         /* Security parameter index */
	/* variable size, 32bit bound*/	/* Authentication data */
};

struct newah {
	u_int8_t        ah_nxt;         /* Next Header */
	u_int8_t        ah_len;         /* Length of data + 1, in 32bit */
	u_int16_t       ah_reserve;     /* Reserved for future use */
	u_int32_t       ah_spi;         /* Security parameter index */
	u_int32_t       ah_seq;         /* Sequence number field */
	/* variable size, 32bit bound*/	/* Authentication data */
};

#ifdef BSD_KERNEL_PRIVATE
struct secasvar;

struct ah_algorithm_state {
	struct secasvar *sav;
	void* foo;      /*per algorithm data - maybe*/
};

struct ah_algorithm {
	int (*sumsiz)(struct secasvar *);
	int (*mature)(struct secasvar *);
	int keymin;     /* in bits */
	int keymax;     /* in bits */
	const char *name;
	int (*init)(struct ah_algorithm_state *, struct secasvar *);
	void (*update)(struct ah_algorithm_state *, caddr_t, size_t);
	void (*result)(struct ah_algorithm_state *, caddr_t, size_t);
};

#define AH_MAXSUMSIZE   64 // sha2-512's output size

extern const struct ah_algorithm *ah_algorithm_lookup(int);

/* cksum routines */
extern int ah_hdrlen(struct secasvar *);

extern size_t ah_hdrsiz(struct ipsecrequest *);
extern void ah4_input(struct mbuf *, int);
extern int ah4_output(struct mbuf *, struct secasvar *);
extern int ah4_calccksum(struct mbuf *, caddr_t, size_t,
    const struct ah_algorithm *, struct secasvar *);
#endif /* BSD_KERNEL_PRIVATE */

#endif /* _NETINET6_AH_H_ */

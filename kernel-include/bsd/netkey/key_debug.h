/*	$KAME: key_debug.h,v 1.7 2000/07/04 04:08:16 itojun Exp $	*/

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

#ifndef _NETKEY_KEY_DEBUG_H_
#define _NETKEY_KEY_DEBUG_H_
#include <sys/appleapiopts.h>

/* debug flags */
#define KEYDEBUG_STAMP          0x00000001 /* path */
#define KEYDEBUG_DATA           0x00000002 /* data */
#define KEYDEBUG_DUMP           0x00000004 /* dump */

#define KEYDEBUG_KEY            0x00000010 /* key processing */
#define KEYDEBUG_ALG            0x00000020 /* ciph & auth algorithm */
#define KEYDEBUG_IPSEC          0x00000040 /* ipsec processing */

#define KEYDEBUG_KEY_STAMP      (KEYDEBUG_KEY | KEYDEBUG_STAMP)
#define KEYDEBUG_KEY_DATA       (KEYDEBUG_KEY | KEYDEBUG_DATA)
#define KEYDEBUG_KEY_DUMP       (KEYDEBUG_KEY | KEYDEBUG_DUMP)
#define KEYDEBUG_ALG_STAMP      (KEYDEBUG_ALG | KEYDEBUG_STAMP)
#define KEYDEBUG_ALG_DATA       (KEYDEBUG_ALG | KEYDEBUG_DATA)
#define KEYDEBUG_ALG_DUMP       (KEYDEBUG_ALG | KEYDEBUG_DUMP)
#define KEYDEBUG_IPSEC_STAMP    (KEYDEBUG_IPSEC | KEYDEBUG_STAMP)
#define KEYDEBUG_IPSEC_DATA     (KEYDEBUG_IPSEC | KEYDEBUG_DATA)
#define KEYDEBUG_IPSEC_DUMP     (KEYDEBUG_IPSEC | KEYDEBUG_DUMP)

#if 0
#define KEYDEBUG(lev, arg) \
	do { if ((key_debug_level & (lev)) == (lev)) { arg; } } while (0)
#else
#define KEYDEBUG(lev, arg)
#endif

struct sadb_msg;
struct sadb_ext;
extern void kdebug_sadb(struct sadb_msg *);
extern void kdebug_sadb_x_policy(struct sadb_ext *);

#ifdef BSD_KERNEL_PRIVATE
extern u_int32_t key_debug_level;

struct secpolicy;
struct secpolicyindex;
struct secasindex;
struct secasvar;
struct secreplay;
struct mbuf;
extern void kdebug_secpolicy(struct secpolicy *);
extern void kdebug_secpolicyindex(struct secpolicyindex *);
extern void kdebug_secasindex(struct secasindex *);
extern void kdebug_secasv(struct secasvar *);
extern void kdebug_mbufhdr(struct mbuf *);
extern void kdebug_mbuf(struct mbuf *);
#endif /* BSD_KERNEL_PRIVATE */

struct sockaddr;
extern void kdebug_sockaddr(struct sockaddr *);

extern void ipsec_hexdump(caddr_t, int);
extern void ipsec_bindump(caddr_t, int);


#endif /* _NETKEY_KEY_DEBUG_H_ */

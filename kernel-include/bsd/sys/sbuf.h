/*-
 * Copyright (c) 2000 Poul-Henning Kamp and Dag-Erling Co•dan Sm¿rgrav
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *      $FreeBSD: /repoman/r/ncvs/src/sys/sys/sbuf.h,v 1.14 2004/07/09 11:35:30 des Exp $
 */

#ifndef _SYS_SBUF_H_
#define _SYS_SBUF_H_

#include <sys/_types.h>
#include <stdarg.h>

/*
 * Structure definition
 */
struct sbuf {
	char            *s_buf;         /* storage buffer */
	void            *s_unused;      /* binary compatibility. */
	int              s_size;        /* size of storage buffer */
	int              s_len;         /* current length of string */
#define SBUF_FIXEDLEN   0x00000000      /* fixed length buffer (default) */
#define SBUF_AUTOEXTEND 0x00000001      /* automatically extend buffer */
#define SBUF_USRFLAGMSK 0x0000ffff      /* mask of flags the user may specify */
#define SBUF_DYNAMIC    0x00010000      /* s_buf must be freed */
#define SBUF_FINISHED   0x00020000      /* set by sbuf_finish() */
#define SBUF_OVERFLOWED 0x00040000      /* sbuf overflowed */
#define SBUF_DYNSTRUCT  0x00080000      /* sbuf must be freed */
	int              s_flags;       /* flags */
};

__BEGIN_DECLS
/*
 * API functions
 */
struct sbuf     *sbuf_new(struct sbuf *, char *, int, int);
void             sbuf_clear(struct sbuf *);
int              sbuf_setpos(struct sbuf *, int);
int              sbuf_bcat(struct sbuf *, const void *, size_t);
int              sbuf_bcpy(struct sbuf *, const void *, size_t);
int              sbuf_cat(struct sbuf *, const char *);
int              sbuf_cpy(struct sbuf *, const char *);
int              sbuf_printf(struct sbuf *, const char *, ...) __printflike(2, 3);
int              sbuf_vprintf(struct sbuf *, const char *, va_list) __printflike(2, 0);
int              sbuf_putc(struct sbuf *, int);
int              sbuf_trim(struct sbuf *);
int              sbuf_overflowed(struct sbuf *);
void             sbuf_finish(struct sbuf *);
char            *sbuf_data(struct sbuf *);
int              sbuf_len(struct sbuf *);
int              sbuf_done(struct sbuf *);
void             sbuf_delete(struct sbuf *);

#ifdef KERNEL
struct uio;
struct sbuf     *sbuf_uionew(struct sbuf *, struct uio *, int *);
int              sbuf_bcopyin(struct sbuf *, const void *, size_t);
int              sbuf_copyin(struct sbuf *, const void *, size_t);
#endif
__END_DECLS

#endif

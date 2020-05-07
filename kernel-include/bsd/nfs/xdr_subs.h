/*
 * Copyright (c) 2000-2011 Apple Inc. All rights reserved.
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
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1989, 1993
 *     The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Rick Macklem at The University of Guelph.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *     This product includes software developed by the University of
 *     California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *     @(#)xdr_subs.h  8.3 (Berkeley) 3/30/95
 * FreeBSD-Id: xdr_subs.h,v 1.9 1997/02/22 09:42:53 peter Exp $
 */

#ifndef _NFS_XDR_SUBS_H_
#define _NFS_XDR_SUBS_H_

#include <sys/appleapiopts.h>

#ifdef __APPLE_API_PRIVATE
/*
 * Macros used for conversion to/from xdr representation by nfs...
 * These use the MACHINE DEPENDENT routines ntohl, htonl
 * As defined by "XDR: External Data Representation Standard" RFC1014
 *
 * To simplify the implementation, we use ntohl/htonl even on big-endian
 * machines, and count on them being `#define'd away.  Some of these
 * might be slightly more efficient as quad_t copies on a big-endian,
 * but we cannot count on their alignment anyway.
 */

#define fxdr_unsigned(t, v)     ((t)ntohl((uint32_t)(v)))
#define txdr_unsigned(v)        (htonl((uint32_t)(v)))

#define fxdr_hyper(f, t) { \
	((uint32_t *)(t))[_QUAD_HIGHWORD] = ntohl(((uint32_t *)(f))[0]); \
	((uint32_t *)(t))[_QUAD_LOWWORD] = ntohl(((uint32_t *)(f))[1]); \
}
#define txdr_hyper(f, t) { \
	((uint32_t *)(t))[0] = htonl(((uint32_t *)(f))[_QUAD_HIGHWORD]); \
	((uint32_t *)(t))[1] = htonl(((uint32_t *)(f))[_QUAD_LOWWORD]); \
}


/*
 * xdrbuf
 *
 * generalized functionality for managing the building/dissecting of XDR data
 */
typedef enum xdrbuf_type {
	XDRBUF_NONE   = 0,
	XDRBUF_BUFFER = 1,
} xdrbuf_type;

struct xdrbuf {
	union {
		struct {
			char *                  xbb_base;       /* base address of buffer */
			uint32_t                xbb_size;       /* size of buffer */
			uint32_t                xbb_len;        /* length of data in buffer */
		} xb_buffer;
	} xb_u;
	char *          xb_ptr;         /* pointer to current position */
	size_t          xb_left;        /* bytes remaining in current buffer */
	size_t          xb_growsize;    /* bytes to allocate when growing */
	xdrbuf_type     xb_type;        /* type of xdr buffer */
	uint32_t        xb_flags;       /* XB_* (see below) */
};

#define XB_CLEANUP      0x0001  /* needs cleanup */

#define XDRWORD         4       /* the basic XDR building block is a 4 byte (32 bit) word */
#define xdr_rndup(a)    (((a)+3)&(~0x3))        /* round up to XDRWORD size */
#define xdr_pad(a)      (xdr_rndup(a) - (a))    /* calculate round up padding */

void xb_init(struct xdrbuf *, xdrbuf_type);
void xb_init_buffer(struct xdrbuf *, char *, size_t);
void xb_cleanup(struct xdrbuf *);
void *xb_malloc(size_t);
void xb_free(void *);
int xb_grow(struct xdrbuf *);
void xb_set_cur_buf_len(struct xdrbuf *);
char *xb_buffer_base(struct xdrbuf *);
int xb_advance(struct xdrbuf *, uint32_t);
int xb_offset(struct xdrbuf *);
int xb_seek(struct xdrbuf *, uint32_t);
int xb_add_bytes(struct xdrbuf *, const char *, uint32_t, int);
int xb_get_bytes(struct xdrbuf *, char *, uint32_t, int);

#ifdef _NFS_XDR_SUBS_FUNCS_

/*
 * basic initialization of xdrbuf structure
 */
void
xb_init(struct xdrbuf *xbp, xdrbuf_type type)
{
	bzero(xbp, sizeof(*xbp));
	xbp->xb_type = type;
	xbp->xb_flags |= XB_CLEANUP;
}

/*
 * initialize a single-buffer xdrbuf
 */
void
xb_init_buffer(struct xdrbuf *xbp, char *buf, size_t buflen)
{
	xb_init(xbp, XDRBUF_BUFFER);
	xbp->xb_u.xb_buffer.xbb_base = buf;
	xbp->xb_u.xb_buffer.xbb_size = buflen;
	xbp->xb_u.xb_buffer.xbb_len = buflen;
	xbp->xb_growsize = 512;
	xbp->xb_ptr = buf;
	xbp->xb_left = buflen;
	if (buf) { /* when using an existing buffer, xb code should skip cleanup */
		xbp->xb_flags &= ~XB_CLEANUP;
	}
}

/*
 * get the pointer to the single-buffer xdrbuf's buffer
 */
char *
xb_buffer_base(struct xdrbuf *xbp)
{
	return xbp->xb_u.xb_buffer.xbb_base;
}

/*
 * clean up any resources held by an xdrbuf
 */
void
xb_cleanup(struct xdrbuf *xbp)
{
	if (!(xbp->xb_flags & XB_CLEANUP)) {
		return;
	}
	switch (xbp->xb_type) {
	case XDRBUF_BUFFER:
		if (xbp->xb_u.xb_buffer.xbb_base) {
			xb_free(xbp->xb_u.xb_buffer.xbb_base);
		}
		break;
	default:
		break;
	}
	xbp->xb_flags &= ~XB_CLEANUP;
}

/*
 * set the length of valid data in the current buffer to
 * be up to the current location within the buffer
 */
void
xb_set_cur_buf_len(struct xdrbuf *xbp)
{
	switch (xbp->xb_type) {
	case XDRBUF_BUFFER:
		xbp->xb_u.xb_buffer.xbb_len = xbp->xb_ptr - xbp->xb_u.xb_buffer.xbb_base;
		break;
	default:
		break;
	}
}

/*
 * advance forward through existing data in xdrbuf
 */
int
xb_advance(struct xdrbuf *xbp, uint32_t len)
{
	uint32_t tlen;

	while (len) {
		if (xbp->xb_left <= 0) {
			return EBADRPC;
		}
		tlen = MIN(xbp->xb_left, len);
		if (tlen) {
			xbp->xb_ptr += tlen;
			xbp->xb_left -= tlen;
			len -= tlen;
		}
	}
	return 0;
}

/*
 * Calculate the current offset in the XDR buffer.
 */
int
xb_offset(struct xdrbuf *xbp)
{
	uint32_t offset = 0;

	switch (xbp->xb_type) {
	case XDRBUF_BUFFER:
		offset = xbp->xb_ptr - xbp->xb_u.xb_buffer.xbb_base;
		break;
	default:
		break;
	}

	return offset;
}

/*
 * Seek to the given offset in the existing data in the XDR buffer.
 */
int
xb_seek(struct xdrbuf *xbp, uint32_t offset)
{
	switch (xbp->xb_type) {
	case XDRBUF_BUFFER:
		xbp->xb_ptr = xbp->xb_u.xb_buffer.xbb_base + offset;
		xbp->xb_left = xbp->xb_u.xb_buffer.xbb_len - offset;
		break;
	default:
		break;
	}

	return 0;
}

/*
 * allocate memory
 */
void *
xb_malloc(size_t size)
{
	void *buf = NULL;

#ifdef KERNEL
	MALLOC(buf, void *, size, M_TEMP, M_WAITOK);
#else
	buf = malloc(size);
#endif
	return buf;
}
/*
 * free a chunk of memory allocated with xb_malloc()
 */
void
xb_free(void *buf)
{
#ifdef KERNEL
	FREE(buf, M_TEMP);
#else
	free(buf);
#endif
}

/*
 * Increase space available for new data in XDR buffer.
 */
int
xb_grow(struct xdrbuf *xbp)
{
	char *newbuf, *oldbuf;
	size_t newsize, oldsize;

	switch (xbp->xb_type) {
	case XDRBUF_BUFFER:
		oldsize = xbp->xb_u.xb_buffer.xbb_size;
		oldbuf = xbp->xb_u.xb_buffer.xbb_base;
		newsize = oldsize + xbp->xb_growsize;
		if (newsize < oldsize) {
			return ENOMEM;
		}
		newbuf = xb_malloc(newsize);
		if (newbuf == NULL) {
			return ENOMEM;
		}
		if (oldbuf != NULL) {
			bcopy(oldbuf, newbuf, oldsize);
			xb_free(oldbuf);
		}
		xbp->xb_u.xb_buffer.xbb_base = newbuf;
		xbp->xb_u.xb_buffer.xbb_size = newsize;
		xbp->xb_ptr = newbuf + oldsize;
		xbp->xb_left = xbp->xb_growsize;
		break;
	default:
		break;
	}

	return 0;
}

/*
 * xb_add_bytes()
 *
 * Add "count" bytes of opaque data pointed to by "buf" to the given XDR buffer.
 */
int
xb_add_bytes(struct xdrbuf *xbp, const char *buf, uint32_t count, int nopad)
{
	uint32_t len, tlen;
	int error;

	len = nopad ? count : xdr_rndup(count);

	/* copy in "count" bytes and zero out any pad bytes */
	while (len) {
		if (xbp->xb_left <= 0) {
			/* need more space */
			if ((error = xb_grow(xbp))) {
				return error;
			}
			if (xbp->xb_left <= 0) {
				return ENOMEM;
			}
		}
		tlen = MIN(xbp->xb_left, len);
		if (tlen) {
			if (count) {
				if (tlen > count) {
					tlen = count;
				}
				bcopy(buf, xbp->xb_ptr, tlen);
			} else {
				bzero(xbp->xb_ptr, tlen);
			}
			xbp->xb_ptr += tlen;
			xbp->xb_left -= tlen;
			len -= tlen;
			if (count) {
				buf += tlen;
				count -= tlen;
			}
		}
	}
	return 0;
}

/*
 * xb_get_bytes()
 *
 * Get "count" bytes of opaque data from the given XDR buffer.
 */
int
xb_get_bytes(struct xdrbuf *xbp, char *buf, uint32_t count, int nopad)
{
	uint32_t len, tlen;

	len = nopad ? count : xdr_rndup(count);

	/* copy in "count" bytes and zero out any pad bytes */
	while (len) {
		if (xbp->xb_left <= 0) {
			return ENOMEM;
		}
		tlen = MIN(xbp->xb_left, len);
		if (tlen) {
			if (count) {
				if (tlen > count) {
					tlen = count;
				}
				bcopy(xbp->xb_ptr, buf, tlen);
			}
			xbp->xb_ptr += tlen;
			xbp->xb_left -= tlen;
			len -= tlen;
			if (count) {
				buf += tlen;
				count -= tlen;
			}
		}
	}
	return 0;
}

#endif /* _NFS_XDR_SUBS_FUNCS_ */


/*
 * macros for building XDR data
 */

/* finalize the data that has been added to the buffer */
#define xb_build_done(E, XB) \
	do { \
	        if (E) break; \
	        xb_set_cur_buf_len(XB); \
	} while (0)

/* add a 32-bit value */
#define xb_add_32(E, XB, VAL) \
	do { \
	        uint32_t __tmp; \
	        if (E) break; \
	        __tmp = txdr_unsigned(VAL); \
	        (E) = xb_add_bytes((XB), (void*)&__tmp, XDRWORD, 0); \
	} while (0)

/* add a 64-bit value */
#define xb_add_64(E, XB, VAL) \
	do { \
	        uint64_t __tmp1, __tmp2; \
	        if (E) break; \
	        __tmp1 = (VAL); \
	        txdr_hyper(&__tmp1, &__tmp2); \
	        (E) = xb_add_bytes((XB), (char*)&__tmp2, 2 * XDRWORD, 0); \
	} while (0)

/* add an array of XDR words */
#define xb_add_word_array(E, XB, A, LEN) \
	do { \
	        uint32_t __i; \
	        xb_add_32((E), (XB), (LEN)); \
	        for (__i=0; __i < (uint32_t)(LEN); __i++) \
	                xb_add_32((E), (XB), (A)[__i]); \
	} while (0)
#define xb_add_bitmap(E, XB, B, LEN)    xb_add_word_array((E), (XB), (B), (LEN))

/* add a file handle */
#define xb_add_fh(E, XB, FHP, FHLEN) \
	do { \
	        xb_add_32((E), (XB), (FHLEN)); \
	        if (E) break; \
	        (E) = xb_add_bytes((XB), (char*)(FHP), (FHLEN), 0); \
	} while (0)

/* add a string */
#define xb_add_string(E, XB, S, LEN) \
	do { \
	        xb_add_32((E), (XB), (LEN)); \
	        if (E) break; \
	        (E) = xb_add_bytes((XB), (const char*)(S), (LEN), 0); \
	} while (0)


/*
 * macros for decoding XDR data
 */

/* skip past data in the buffer */
#define xb_skip(E, XB, LEN) \
	do { \
	        if (E) break; \
	        (E) = xb_advance((XB), (LEN)); \
	} while (0)

/* get a 32-bit value */
#define xb_get_32(E, XB, LVAL) \
	do { \
	        uint32_t __tmp; \
	        if (E) break; \
	        (E) = xb_get_bytes((XB), (char*)&__tmp, XDRWORD, 0); \
	        if (E) break; \
	        (LVAL) = fxdr_unsigned(uint32_t, __tmp); \
	} while (0)

/* get a 64-bit value */
#define xb_get_64(E, XB, LVAL) \
	do { \
	        uint64_t __tmp; \
	        if (E) break; \
	        (E) = xb_get_bytes((XB), (char*)&__tmp, 2 * XDRWORD, 0); \
	        if (E) break; \
	        fxdr_hyper(&__tmp, &(LVAL)); \
	} while (0)

/* get an array of XDR words (of a given expected/maximum length) */
#define xb_get_word_array(E, XB, A, LEN) \
	do { \
	        uint32_t __len = 0, __i; \
	        xb_get_32((E), (XB), __len); \
	        if (E) break; \
	        for (__i=0; __i < MIN(__len, (uint32_t)(LEN)); __i++) \
	                xb_get_32((E), (XB), (A)[__i]); \
	        if (E) break; \
	        for (; __i < __len; __i++) \
	                xb_skip((E), (XB), XDRWORD); \
	        for (; __i < (uint32_t)(LEN); __i++) \
	                (A)[__i] = 0; \
	        (LEN) = __len; \
	} while (0)
#define xb_get_bitmap(E, XB, B, LEN)    xb_get_word_array((E), (XB), (B), (LEN))

#endif /* __APPLE_API_PRIVATE */
#endif /* _NFS_XDR_SUBS_H_ */

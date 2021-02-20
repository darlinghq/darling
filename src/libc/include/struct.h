/*-
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
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
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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
 *	@(#)struct.h	8.1 (Berkeley) 6/2/93
 */

#ifndef _STRUCT_H_
#define	_STRUCT_H_

/* Offset of the field in the structure. */
#define	fldoff(name, field) \
	((int)&(((struct name *)0)->field))

/* Size of the field in the structure. */
#define	fldsiz(name, field) \
	(sizeof(((struct name *)0)->field))

/* Address of the structure from a field. */
#define	strbase(name, addr, field) \
	((struct name *)((char *)(addr) - fldoff(name, field)))

/*
 * countof() cannot be safely used in a _Static_assert statement, so we provide
 * an unsafe variant that does not verify the input array is statically-defined.
 */
#define countof_unsafe(arr) \
	(sizeof(arr) / sizeof(arr[0]))

/* Number of elements in a statically-defined array */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && __GNUC__
#define countof(arr) ({ \
	_Static_assert( \
			!__builtin_types_compatible_p(typeof(arr), typeof(&(arr)[0])), \
			"array must be statically defined"); \
	(sizeof(arr) / sizeof(arr[0])); \
})
#else
#define countof(arr) \
	countof_unsafe(arr)
#endif

/* Length of a statically-defined string (does not include null terminator) */
#define lenof(str) \
	(sizeof(str) - 1)

/* Last index of a statically-defined array */
#define lastof(arr) \
	(countof(arr) - 1)

#endif /* !_STRUCT_H_ */

/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
/* Copyright (c) 1995 NeXT Computer, Inc. All Rights Reserved */
/*
 * Copyright (c) 1990, 1982, 1985, 1986, 1988, 1993, 1994
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
 *	@(#)ranlib.h	8.1 (Berkeley) 6/2/93
 */
#ifndef _RANLIB_H_
#define _RANLIB_H_

#include <sys/types.h>		/* off_t */

/*
 * There are two known orders of table of contents for archives.  The first is
 * the order ranlib(1) originally produced and still produces without any
 * options.  This table of contents has the archive member name "__.SYMDEF"
 * This order has the ranlib structures in the order the objects appear in the
 * archive and the symbol names of those objects in the order of symbol table.
 * The second know order is sorted by symbol name and is produced with the -s
 * option to ranlib(1).  This table of contents has the archive member name
 * "__.SYMDEF SORTED" and many programs (notably the 1.0 version of ld(1) can't
 * tell the difference between names because of the imbedded blank in the name
 * and works with either table of contents).  This second order is used by the
 * post 1.0 link editor to produce faster linking.  The original 1.0 version of
 * ranlib(1) gets confused when it is run on a archive with the second type of
 * table of contents because it and ar(1) which it uses use different ways to
 * determined the member name (ar(1) treats all blanks in the name as
 * significant and ranlib(1) only checks for the first one).
 */
#define SYMDEF		"__.SYMDEF"
#define SYMDEF_SORTED	"__.SYMDEF SORTED"

#define	RANLIBMAG	"__.SYMDEF"	/* archive file name */
#define	RANLIBSKEW	3		/* creation time offset */

/*
 * Structure of the __.SYMDEF table of contents for an archive.
 * __.SYMDEF begins with a long giving the size in bytes of the ranlib
 * structures which immediately follow, and then continues with a string
 * table consisting of a long giving the number of bytes of strings which
 * follow and then the strings themselves.  The ran_strx fields index the
 * string table whose first byte is numbered 0.
 */
struct	ranlib {
	union {
		off_t	ran_strx;	/* string table index of */
		char	*ran_name;	/* symbol defined by */
	} ran_un;
	off_t	ran_off;		/* library member at this offset */
};
#endif /* ! _RANLIB_H_ */

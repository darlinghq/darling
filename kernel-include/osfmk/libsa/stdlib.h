/*
 * Copyright (c) 2000-2006 Apple Computer, Inc. All rights reserved.
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
/*
 * @OSF_COPYRIGHT@
 *
 */
/*
 * HISTORY
 *
 * Revision 1.1.1.1  1998/09/22 21:05:51  wsanchez
 * Import of Mac OS X kernel (~semeria)
 *
 * Revision 1.1.1.1  1998/03/07 02:25:35  wsanchez
 * Import of OSF Mach kernel (~mburg)
 *
 * Revision 1.1.4.1  1997/02/21  15:43:19  barbou
 *      Removed "size_t" definition, include "types.h" instead.
 *      [1997/02/21  15:36:24  barbou]
 *
 * Revision 1.1.2.3  1996/09/30  10:14:34  bruel
 *      Added strtol and strtoul prototypes.
 *      [96/09/30            bruel]
 *
 * Revision 1.1.2.2  1996/09/23  15:06:22  bruel
 *      removed bzero and bcopy definitions.
 *      [96/09/23            bruel]
 *
 * Revision 1.1.2.1  1996/09/17  16:56:24  bruel
 *      created from standalone mach servers.
 *      [96/09/17            bruel]
 *
 * $EndLog$
 */

#ifndef _MACH_STDLIB_H_
#define _MACH_STDLIB_H_

#include <types.h>

#ifndef NULL
#define NULL    (void *)0
#endif

extern int     atoi(const char *);
extern int     atoi_term(char *, char **);
extern char    *itoa(int, char *);

extern void     free(void *);
extern void     *malloc(size_t);
extern void     *realloc(void *, size_t);

extern char     *getenv(const char *);

extern void     exit(int);

extern long int strtol(const char *, char **, int);
extern unsigned long int strtoul(const char *, char **, int);

#endif /* _MACH_STDLIB_H_ */

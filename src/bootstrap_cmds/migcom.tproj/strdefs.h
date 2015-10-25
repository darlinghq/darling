/*
 * Copyright (c) 1999, 2008 Apple Inc. All rights reserved.
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
/*
 * Mach Operating System
 * Copyright (c) 1991,1990 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS
 * CONDITION.  CARNEGIE MELLON DISCLAIMS ANY LIABILITY OF ANY KIND FOR
 * ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie the
 * rights to redistribute these changes.
 */
/*
 * 91/02/05  17:55:57  mrt
 *  Changed to new Mach copyright
 *  [91/02/01  17:56:03  mrt]
 *
 * 90/06/02  15:05:49  rpd
 *  Created for new IPC.
 *  [90/03/26  21:13:56  rpd]
 *
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 *  Extensive revamping.  Added polymorphic arguments.
 *  Allow multiple variable-sized inline arguments in messages.
 *
 * 15-Jun-87  David Black (dlb) at Carnegie-Mellon University
 *  Fixed strNULL to be the null string instead of the null string
 *  pointer.
 *
 * 27-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *  Created.
 */

#ifndef STRDEFS_H
#define STRDEFS_H

#include <string.h>
#include <mach/boolean.h>

typedef char *string_t;
typedef string_t identifier_t;

#define MAX_STR_LEN 200

#define strNULL ((string_t) 0)

extern string_t strmake( char *string );
extern string_t strconcat( string_t left, string_t right );
extern string_t strphrase( string_t left, string_t right );
extern void strfree( string_t string );

#define streql(a, b)  (strcmp((a), (b)) == 0)

extern char *strbool( boolean_t bool );
extern char *strstring( string_t string );
extern char *toupperstr( char *string );

#endif  /* STRDEFS_H */

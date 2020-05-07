/*
 * Copyright (c) 2000 Apple Computer, Inc. All rights reserved.
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
 */
/*
 * HISTORY
 *
 * Revision 1.1.1.1  1998/09/22 21:05:30  wsanchez
 * Import of Mac OS X kernel (~semeria)
 *
 * Revision 1.1.1.1  1998/03/07 02:25:46  wsanchez
 * Import of OSF Mach kernel (~mburg)
 *
 * Revision 1.2.6.1  1994/09/23  02:40:41  ezf
 *      change marker to not FREE
 *      [1994/09/22  21:41:56  ezf]
 *
 * Revision 1.2.2.2  1993/06/09  02:42:32  gm
 *      Added to OSF/1 R1.3 from NMK15.0.
 *      [1993/06/02  21:17:31  jeffc]
 *
 * Revision 1.2  1993/04/19  16:37:56  devrcs
 *      ansi C conformance changes
 *      [1993/02/02  18:53:54  david]
 *
 * Revision 1.1  1992/09/30  02:31:51  robert
 *      Initial revision
 *
 * $EndLog$
 */
/* CMU_HIST */
/*
 * Revision 2.3  91/05/14  16:58:02  mrt
 *      Correcting copyright
 *
 * Revision 2.2  91/02/05  17:35:10  mrt
 *      Changed to new Mach copyright
 *      [91/02/01  17:17:32  mrt]
 *
 * Revision 2.1  89/08/03  16:03:38  rwd
 * Created.
 *
 * Revision 2.3  89/02/25  18:39:26  gm0w
 *      Changes for cleanup.
 *
 *  4-Mar-87  Michael Young (mwyoung) at Carnegie-Mellon University
 *	Added MSG_TYPE_RPC.
 *
 *  22-Dec-86 Mary Thompson
 *	defined MSG_TYPE_CAMELOT, and MSG_TYPE_ENCRYPTED
 *
 */
/* CMU_ENDHIST */
/*
 * Mach Operating System
 * Copyright (c) 1991,1990,1989,1988,1987 Carnegie Mellon University
 * All Rights Reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS"
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
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */
/*
 */
/*
 *    This file defines user msg types that may be ored into
 *    the msg_type field in a msg header. Values 0-5 are reserved
 *    for use by the kernel and are defined in message.h.
 *
 */

#ifndef MSG_TYPE_H_
#define MSG_TYPE_H_

#define MSG_TYPE_CAMELOT        (1 << 6)
#define MSG_TYPE_ENCRYPTED      (1 << 7)
#define MSG_TYPE_RPC            (1 << 8)        /* Reply expected */

#include <mach/message.h>

#endif  /* MSG_TYPE_H_ */

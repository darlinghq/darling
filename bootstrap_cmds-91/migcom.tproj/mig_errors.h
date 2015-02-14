/*
 * Copyright (c) 1999 Apple Computer, Inc. All rights reserved.
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
 * Copyright (c) 1987 Carnegie-Mellon University
 * All rights reserved.  The CMU software License Agreement specifies
 * the terms and conditions for use and redistribution.
 */
/*
 * Mach Interface Generator errors
 *
 * $Header: /Users/Shared/bootstrap_cmds/bootstrap_cmds/migcom.tproj/mig_errors.h,v 1.1.1.2 2000/01/11 00:36:18 wsanchez Exp $
 *
 * HISTORY
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 *  Extensive revamping.  Added polymorphic arguments.
 *  Allow multiple variable-sized inline arguments in messages.
 *
 * 28-Apr-88  Bennet Yee (bsy) at Carnegie-Mellon University
 *  Put mig_symtab back.
 *
 *  2-Dec-87  David Golub (dbg) at Carnegie-Mellon University
 *  Added MIG_ARRAY_TOO_LARGE.
 *
 * 25-May-87  Richard Draves (rpd) at Carnegie-Mellon University
 *  Added definition of death_pill_t.
 *
 * 31-Jul-86  Michael Young (mwyoung) at Carnegie-Mellon University
 *  Created.
 */
#ifndef _MIG_ERRORS_H
#define _MIG_ERRORS_H

#include <mach/kern_return.h>
#include <mach/message.h>

#define MIG_TYPE_ERROR      -300    /* Type check failure */
#define MIG_REPLY_MISMATCH  -301    /* Wrong return message ID */
#define MIG_REMOTE_ERROR    -302    /* Server detected error */
#define MIG_BAD_ID          -303    /* Bad message ID */
#define MIG_BAD_ARGUMENTS   -304    /* Server found wrong arguments */
#define MIG_NO_REPLY        -305    /* Server shouldn't reply */
#define MIG_EXCEPTION       -306    /* Server raised exception */
#define MIG_ARRAY_TOO_LARGE -307    /* User specified array not large enough
                                       to hold returned array */

typedef struct {
  msg_header_t  Head;
  msg_type_t    RetCodeType;
  kern_return_t RetCode;
} death_pill_t;

typedef struct mig_symtab {
  char  *ms_routine_name;
  int   ms_routine_number;
#ifdef hc
  void
#else
  int
#endif
        (*ms_routine)();
} mig_symtab_t;

#endif  _MIG_ERRORS_H

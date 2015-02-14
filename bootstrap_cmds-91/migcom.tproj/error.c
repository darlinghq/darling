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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "global.h"
#include "error.h"

extern int lineno;
extern char *yyinname;

static char *program;
__private_extern__
int mig_errors = 0;

/*ARGSUSED*/
/*VARARGS1*/
void
fatal(char *format, ...)
{
  va_list pvar;
  va_start(pvar, format);
  fprintf(stderr, "%s: fatal: \"%s\", line %d: ", program, yyinname, lineno-1);
  (void) vfprintf(stderr, format, pvar);
  fprintf(stderr, "\n");
  va_end(pvar);
  exit(1);
}

__private_extern__
/*ARGSUSED*/
/*VARARGS1*/
void
warn(char *format, ...)
{
  va_list pvar;
  va_start(pvar, format);
  if (!BeQuiet && (mig_errors == 0)) {
    fprintf(stderr, "\"%s\", line %d: warning: ", yyinname, lineno-1);
    (void) vfprintf(stderr, format, pvar);
    fprintf(stderr, "\n");
  }
  va_end(pvar);
}

/*ARGSUSED*/
/*VARARGS1*/
void
error(char *format, ...)
{
  va_list pvar;
  va_start(pvar, format);
  fprintf(stderr, "\"%s\", line %d: ", yyinname, lineno-1);
  (void) vfprintf(stderr, format, pvar);
  fprintf(stderr, "\n");
  va_end(pvar);
  mig_errors++;
}

void
set_program_name(char *name)
{
  program = name;
}

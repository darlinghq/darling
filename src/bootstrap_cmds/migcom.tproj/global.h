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

#ifndef _GLOBAL_H
#define _GLOBAL_H

#include "type.h"

extern boolean_t BeQuiet;   /* no warning messages */
extern boolean_t BeVerbose; /* summarize types, routines */
extern boolean_t BeDebug;   /* enters in the debug mode */
extern boolean_t UseMsgRPC;
extern boolean_t GenSymTab;
extern boolean_t UseEventLogger;
extern boolean_t BeLint;
extern boolean_t BeAnsiC;
extern boolean_t CheckNDR;
extern boolean_t PackMsg;
extern boolean_t UseSplitHeaders;
extern boolean_t ShortCircuit;
extern boolean_t UseRPCTrap;
extern boolean_t TestRPCTrap;
extern boolean_t IsVoucherCodeAllowed;

extern boolean_t IsKernelUser;
extern boolean_t IsKernelServer;

extern string_t RCSId;

extern string_t SubsystemName;
extern u_int SubsystemBase;

extern string_t MsgOption;
extern string_t WaitTime;
extern string_t SendTime;
extern string_t ErrorProc;
extern string_t ServerPrefix;
extern string_t UserPrefix;
extern string_t ServerDemux;
extern string_t ServerImpl;
extern string_t ServerSubsys;
extern int MaxMessSizeOnStack;
extern int UserTypeLimit;

extern int yylineno;
extern string_t yyinname;

extern void init_global();

extern string_t UserFilePrefix;
extern string_t UserHeaderFileName;
extern string_t ServerHeaderFileName;
extern string_t InternalHeaderFileName;
extern string_t DefinesHeaderFileName;
extern string_t UserFileName;
extern string_t ServerFileName;

extern void more_global();

extern char NewCDecl[];
extern char LintLib[];

#endif  /* _GLOBAL_H */

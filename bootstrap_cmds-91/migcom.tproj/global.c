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

#include "strdefs.h"
#include "global.h"
#include "error.h"
#include "mig_machine.h"

boolean_t BeQuiet = FALSE;
boolean_t BeVerbose = FALSE;
boolean_t UseMsgRPC = TRUE;
boolean_t GenSymTab = FALSE;
boolean_t UseEventLogger = FALSE;
boolean_t BeLint = FALSE;
boolean_t BeAnsiC = TRUE;
boolean_t CheckNDR = FALSE;
boolean_t PackMsg = PACK_MESSAGES;
boolean_t UseSplitHeaders = FALSE;
boolean_t ShortCircuit = FALSE;
boolean_t UseRPCTrap = FALSE;
boolean_t TestRPCTrap= FALSE;
boolean_t IsVoucherCodeAllowed = TRUE;

boolean_t IsKernelUser = FALSE;
boolean_t IsKernelServer = FALSE;

string_t RCSId = strNULL;

string_t SubsystemName = strNULL;
u_int SubsystemBase = 0;

string_t MsgOption = strNULL;
string_t WaitTime = strNULL;
string_t SendTime = strNULL;
string_t ErrorProc = "MsgError";
string_t ServerPrefix = "";
string_t UserPrefix = "";
string_t ServerDemux = strNULL;
string_t ServerImpl = strNULL;
string_t ServerSubsys = strNULL;
int MaxMessSizeOnStack = -1;    /* by default, always on stack */
int UserTypeLimit = -1;         /* by default, assume unlimited size. */

string_t yyinname;

char NewCDecl[] = "(defined(__STDC__) || defined(c_plusplus))";
char LintLib[] = "defined(LINTLIBRARY)";

void
init_global()
{
    yyinname = strmake("<no name yet>");
}

string_t UserFilePrefix = strNULL;
string_t UserHeaderFileName = strNULL;
string_t ServerHeaderFileName = strNULL;
string_t InternalHeaderFileName = strNULL;
string_t DefinesHeaderFileName = strNULL;
string_t UserFileName = strNULL;
string_t ServerFileName = strNULL;
string_t GenerationDate = strNULL;

void
more_global()
{
  if (SubsystemName == strNULL)
    fatal("no SubSystem declaration");

  if (UserHeaderFileName == strNULL)
    UserHeaderFileName = strconcat(SubsystemName, ".h");
  else if (streql(UserHeaderFileName, "/dev/null"))
    UserHeaderFileName = strNULL;

  if (UserFileName == strNULL)
    UserFileName = strconcat(SubsystemName, "User.c");
  else if (streql(UserFileName, "/dev/null"))
    UserFileName = strNULL;

  if (ServerFileName == strNULL)
    ServerFileName = strconcat(SubsystemName, "Server.c");
  else if (streql(ServerFileName, "/dev/null"))
    ServerFileName = strNULL;

  if (ServerDemux == strNULL)
    ServerDemux = strconcat(SubsystemName, "_server");

  if (ServerImpl == strNULL)
    ServerImpl = strconcat(SubsystemName, "_impl");

  if (ServerSubsys == strNULL) {
    if (ServerPrefix != strNULL)
      ServerSubsys = strconcat(ServerPrefix, SubsystemName);
    else
      ServerSubsys = SubsystemName;
    ServerSubsys = strconcat(ServerSubsys, "_subsystem");
  }
}

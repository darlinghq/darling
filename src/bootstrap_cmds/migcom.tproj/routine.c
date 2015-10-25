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

#include "type.h"

#include <mach/message.h>
#include <mach/kern_return.h>
#include "mig_machine.h"
#include "error.h"
#include "alloc.h"
#include "global.h"
#include "routine.h"
#include "write.h"

u_int rtNumber = 0;

static void rtSizeDelta();

routine_t *
rtAlloc(void)
{
  register routine_t *new;

  new = (routine_t *) calloc(1, sizeof *new);
  if (new == rtNULL)
    fatal("rtAlloc(): %s", strerror(errno));
  new->rtNumber = rtNumber++;
  new->rtName = strNULL;
  new->rtErrorName = strNULL;
  new->rtUserName = strNULL;
  new->rtServerName = strNULL;

  return new;
}

void
rtSkip(void)
{
  rtNumber++;
}

argument_t *
argAlloc(void)
{
  extern void KPD_error();

  static argument_t prototype =
  {
    strNULL,      /* identifier_t argName */
    argNULL,      /* argument_t *argNext */
    akNone,       /* arg_kind_t argKind */
    itNULL,       /* ipc_type_t *argType */
    argKPD_NULL,  /* mach_msg_descriptor_type_t argKPD_Type */
    KPD_error,    /* KPD discipline for templates */
    KPD_error,    /* KPD discipline for initializing */
    KPD_error,    /* KPD discipline for packing */
    KPD_error,    /* KPD discipline for extracting */
    KPD_error,    /* KPD discipline for type checking */
    strNULL,      /* string_t argVarName */
    strNULL,      /* string_t argMsgField */
    strNULL,      /* string_t argTTName */
    strNULL,      /* string_t argPadName */
    strNULL,      /* string_t argSuffix */
    flNone,       /* ipc_flags_t argFlags */
    d_NO,         /* dealloc_t argDeallocate */
    FALSE,        /* boolean_t argCountInOut */
    rtNULL,       /* routine_t *argRoutine */
    argNULL,      /* argument_t *argCount */
    argNULL,      /* argument_t *argSubCount */
    argNULL,      /* argument_t *argCInOut */
    argNULL,      /* argument_t *argPoly */
    argNULL,      /* argument_t *argDealloc */
    argNULL,      /* argument_t *argSameCount */
    argNULL,      /* argument_t *argParent */
    1,            /* int argMultiplier */
    0,            /* int argRequestPos */
    0,            /* int argReplyPos */
    FALSE,        /* boolean_t argByReferenceUser */
    FALSE         /* boolean_t argByReferenceServer */
  };
  register argument_t *new;

  new = (argument_t *) malloc(sizeof *new);
  if (new == argNULL)
    fatal("argAlloc(): %s", strerror(errno));
  *new = prototype;
  return new;
}

routine_t *
rtMakeRoutine(identifier_t name, argument_t *args)
{
  register routine_t *rt = rtAlloc();

  rt->rtName = name;
  rt->rtKind = rkRoutine;
  rt->rtArgs = args;

  return rt;
}

routine_t *
rtMakeSimpleRoutine(identifier_t name, argument_t *args)
{
  register routine_t *rt = rtAlloc();

  rt->rtName = name;
  rt->rtKind = rkSimpleRoutine;
  rt->rtArgs = args;

  return rt;
}

char *
rtRoutineKindToStr(routine_kind_t rk)
{
  switch (rk) {

    case rkRoutine:
      return "Routine";

    case rkSimpleRoutine:
      return "SimpleRoutine";

    default:
      fatal("rtRoutineKindToStr(%d): not a routine_kind_t", rk);
      /*NOTREACHED*/
      return strNULL;
  }
}

static void
rtPrintArg(register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;

  if (!akCheck(arg->argKind, akbUserArg|akbServerArg) ||
      (akIdent(arg->argKind) == akeCount) ||
      (akIdent(arg->argKind) == akeDealloc) ||
      (akIdent(arg->argKind) == akeNdrCode) ||
      (akIdent(arg->argKind) == akePoly))
    return;

  printf("\n\t");

  switch (akIdent(arg->argKind)) {

    case akeRequestPort:
      printf("RequestPort");
      break;

    case akeReplyPort:
      printf("ReplyPort");
      break;

    case akeWaitTime:
      printf("WaitTime");
      break;

    case akeSendTime:
      printf("SendTime");
      break;

    case akeMsgOption:
      printf("MsgOption");
      break;

    case akeMsgSeqno:
      printf("MsgSeqno\t");
      break;

    case akeSecToken:
      printf("SecToken\t");
      break;

    case akeAuditToken:
      printf("AuditToken\t");
      break;

    case akeContextToken:
      printf("ContextToken\t");
      break;

    case akeImplicit:
      printf("Implicit\t");
      break;

    default:
      if (akCheck(arg->argKind, akbRequest)) {
        if (akCheck(arg->argKind, akbSend))
          printf("In");
        else
          printf("(In)");
      }
      if (akCheck(arg->argKind, akbReply)) {
        if (akCheck(arg->argKind, akbReturn))
          printf("Out");
        else
          printf("(Out)");
      }
      printf("\t");
  }

  printf("\t%s: %s", arg->argName, it->itName);

  if (arg->argDeallocate == d_YES)
    printf(", Dealloc");
  else if (arg->argDeallocate == d_MAYBE)
    printf(", Dealloc[]");

  if (arg->argCountInOut)
    printf(", CountInOut");

  if (arg->argFlags & flSameCount)
    printf(", SameCount");

  if (arg->argFlags & flPhysicalCopy)
    printf(", PhysicalCopy");

  if (arg->argFlags & flRetCode)
    printf(", PhysicalCopy");

  if (arg->argFlags & flOverwrite)
    printf(", Overwrite");

  if (arg->argFlags & flAuto)
    printf(", Auto");

  if (arg->argFlags & flConst)
    printf(", Const");
}

void
rtPrintRoutine(register routine_t *rt)
{
  register argument_t *arg;

  printf("%s (%d) %s(", rtRoutineKindToStr(rt->rtKind), rt->rtNumber, rt->rtName);

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)
    rtPrintArg(arg);

  printf(")\n");
  printf("\n");
}

/*
 * Determines appropriate value of msg-simple for the message.
 * One version for both In & Out.
 */

static void
rtCheckSimple(argument_t *args, u_int mask, boolean_t *simple)
{
  register argument_t *arg;
  boolean_t MustBeComplex = FALSE;

  for (arg = args; arg != argNULL; arg = arg->argNext)
    if (akCheck(arg->argKind, mask)) {
      register ipc_type_t *it = arg->argType;

      if (IS_KERN_PROC_DATA(it))
        MustBeComplex = TRUE;
    }

  *simple = !MustBeComplex;
}

static void
rtCheckFit(routine_t *rt, u_int mask, boolean_t *fitp, boolean_t *uselimp, u_int *knownp)
{
  boolean_t uselim = FALSE;
  argument_t *arg;
  u_int size = sizeof(mach_msg_header_t);

  if (!rt->rtSimpleRequest)
    machine_alignment(size,sizeof(mach_msg_body_t));
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)
    if (akCheck(arg->argKind, mask)) {
      register ipc_type_t *it = arg->argType;

      machine_alignment(size, it->itMinTypeSize);
      if (it->itNative)
        uselim = TRUE;
      else if (IS_VARIABLE_SIZED_UNTYPED(it)) {
        machine_alignment(size, it->itTypeSize);
        size += it->itPadSize;
      }
    }
  *knownp = size;
  if (MaxMessSizeOnStack == -1) {
    *fitp = TRUE;
    *uselimp = FALSE;
  }
  else if (size > MaxMessSizeOnStack) {
    *fitp = FALSE;
    *uselimp = FALSE;
  }
  else if (!uselim) {
    *fitp = TRUE;
    *uselimp = FALSE;
  }
  else if (UserTypeLimit == -1) {
    *fitp = FALSE;
    *uselimp = FALSE;
  }
  else if (size + UserTypeLimit > MaxMessSizeOnStack) {
    *fitp = FALSE;
    *uselimp = TRUE;
  }
  else {
    *fitp = TRUE;
    *uselimp = TRUE;
  }
}

static void
rtFindHowMany(routine_t *rt)
{
  register argument_t *arg;
  int multiplier = 1;
  boolean_t test;

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    register ipc_type_t *it = arg->argType;

    if (IS_MULTIPLE_KPD(it)) {
      if (!it->itVarArray)
        multiplier = it->itKPD_Number;
      test = !it->itVarArray && !it->itElement->itVarArray;
      it = it->itElement;
    }
    else
      test = !it->itVarArray;

    if (akCheck(arg->argKind, akbSendKPD)) {

      if (it->itInLine)
        rt->rtCountPortsIn += it->itNumber * multiplier;
      else if (it->itPortType) {
        if (test)
          rt->rtCountOolPortsIn += it->itNumber * multiplier;
      }
      else {
        if (test)
          rt->rtCountOolIn += (it->itNumber * it->itSize + 7)/8 * multiplier;
      }
    }
    if (akCheckAll(arg->argKind, akbReturnKPD)) {
      if (it->itInLine)
        rt->rtCountPortsOut += it->itNumber * multiplier;
      else if (it->itPortType) {
        if (test)
          rt->rtCountOolPortsOut += it->itNumber * multiplier;
      }
      else {
        if (test)
          rt->rtCountOolOut += ((it->itNumber * it->itSize + 7)/8) * multiplier;
      }
    }
  }
}

boolean_t
rtCheckMask(argument_t *args, u_int mask)
{
  register argument_t *arg;

  for (arg = args; arg != argNULL; arg = arg->argNext)
    if (akCheckAll(arg->argKind, mask))
      return TRUE;
  return FALSE;
}

boolean_t
rtCheckMaskFunction(argument_t *args, u_int mask, boolean_t (*func)(/* argument_t *arg */))
{
  register argument_t *arg;

  for (arg = args; arg != argNULL; arg = arg->argNext)
    if (akCheckAll(arg->argKind, mask))
      if ((*func)(arg))
        return TRUE;
  return FALSE;
}


int
rtCountKPDs(argument_t *args, u_int mask)
{
  register argument_t *arg;
  int count = 0;

  for (arg = args; arg != argNULL; arg = arg->argNext)
    if (akCheckAll(arg->argKind, mask))
      count += arg->argType->itKPD_Number;
  return count;
}

int
rtCountFlags(argument_t *args, u_int flag)
{
  register argument_t *arg;
  int count = 0;

  for (arg = args; arg != argNULL; arg = arg->argNext)
    if (arg->argFlags & flag)
      count++;
  return count;
}

int
rtCountArgDescriptors(argument_t *args, int *argcount)
{
  register argument_t *arg;
  int count = 0;

  if (argcount)
    *argcount = 0;
  for (arg = args; arg != argNULL; arg = arg->argNext)
    if (akCheck(arg->argKind, akbServerArg)) {
      if (RPCFixedArray(arg) ||
          RPCPort(arg) ||
          RPCVariableArray(arg) ||
          RPCPortArray(arg)) {
        count++;
        if (argcount)
          (*argcount)++;
      }
      else {
        if (argcount) {
          if (arg->argType->itStruct && arg->argType->itNumber &&
              (arg->argType->itSize >= 32))
            *argcount += arg->argType->itNumber * (arg->argType->itSize / 32);
          else
            (*argcount)++;
        }
      }
    }
  return count;
}

int
rtCountMask(argument_t *args, u_int mask)
{
  register argument_t *arg;
  int count = 0;

  for (arg = args; arg != argNULL; arg = arg->argNext)
    if (akCheckAll(arg->argKind, mask))
      count++;
  return count;
}

/* arg->argType may be NULL in this function */

static void
rtDefaultArgKind(routine_t *rt, argument_t *arg)
{
  if ((arg->argKind == akNone) && (rt->rtRequestPort == argNULL))
    arg->argKind = akRequestPort;

  if (arg->argKind == akNone)
    arg->argKind = akIn;
}

/*
 * Initializes arg->argDeallocate,
 * arg->argCountInOut from arg->argFlags
 * and perform consistency check over the
 * flags.
 */

static ipc_flags_t
rtProcessDeallocFlag(register ipc_type_t *it, register ipc_flags_t flags, register arg_kind_t kind, dealloc_t *what, string_t name)
{

  /* only one of flDealloc, flNotDealloc, flMaybeDealloc */

  if (flags & flMaybeDealloc) {
    if (flags & (flDealloc|flNotDealloc)) {
      warn("%s: Dealloc and NotDealloc ignored with Dealloc[]", name);
      flags &= ~(flDealloc|flNotDealloc);
    }
  }

  if ((flags&(flDealloc|flNotDealloc)) == (flDealloc|flNotDealloc)) {
    warn("%s: Dealloc and NotDealloc cancel out", name);
    flags &= ~(flDealloc|flNotDealloc);
  }

  if (((IsKernelServer && akCheck(kind, akbReturn)) ||
       (IsKernelUser && akCheck(kind, akbSend))) &&
      (flags & flDealloc)) {
    /*
     *  For a KernelServer interface and an Out argument,
     *  or a KernelUser interface and an In argument,
     *  we avoid a possible spurious warning about the deallocate bit.
     *  For compatibility with Mach 2.5, the deallocate bit
     *  may need to be enabled on some inline arguments.
     */

    *what= d_YES;
  }
  else if (flags & (flMaybeDealloc|flDealloc)) {
    /* only give semantic warnings if the user specified something */
    if (it->itInLine && !it->itPortType) {
      warn("%s: Dealloc is ignored: it is meaningless  for that type of argument", name);
      flags &= ~(flMaybeDealloc|flDealloc);
    }
    else
      *what = (flags & flMaybeDealloc) ? d_MAYBE : d_YES;
  }
  return flags;
}

static void
rtProcessSameCountFlag(register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  register ipc_flags_t flags = arg->argFlags;
  string_t name = arg->argVarName;
  static argument_t *old_arg;

  if (flags & flSameCount)  {
    if (!it->itVarArray) {
      warn("%s: SameCount is ignored - the argument is not variable", name);
      flags &= ~flSameCount;
    }
    if (old_arg) {
      if (old_arg->argParent)
        old_arg = old_arg->argParent;
      if (old_arg->argSameCount)
        old_arg = old_arg->argSameCount;

      if (!old_arg->argType->itVarArray) {
        warn("%s: SameCount is ignored - adjacent argument is not variable", name);
        flags &= ~flSameCount;
      }

#define SAMECOUNT_MASK akeBITS|akbSend|akbReturn|akbRequest|akbReply|akbUserArg|akbServerArg
      if (akCheck(old_arg->argKind, SAMECOUNT_MASK) !=
          akCheck(arg->argKind, SAMECOUNT_MASK) ||
          old_arg->argCountInOut != arg->argCountInOut) {
        warn("%s: SameCount is ignored - inconsistencies with the adjacent argument\n", name);
        flags &= ~flSameCount;
      }
      arg->argSameCount = old_arg;
    }
    arg->argFlags = flags;
  }
  old_arg = arg;
}

static ipc_flags_t
rtProcessCountInOutFlag(register ipc_type_t *it, register ipc_flags_t flags, register arg_kind_t kind, boolean_t *what, string_t name)
{
  if (flags & flCountInOut) {
    if (!akCheck(kind, akbReply)) {
      warn("%s: CountInOut is ignored: argument must be Out\n", name);
      flags &= ~flCountInOut;
    }
    else if (!it->itVarArray || !it->itInLine) {
      warn("%s: CountInOut is ignored: argument isn't variable or in-line\n", name);
      flags &= ~flCountInOut;
    }
    else
      *what = TRUE;
  }
  return flags;
}

static ipc_flags_t
rtProcessPhysicalCopyFlag(register ipc_type_t *it, register ipc_flags_t flags, register arg_kind_t kind, string_t name)
{
  if (flags & flPhysicalCopy) {
    if (it->itInLine) {
      warn("%s: PhysicalCopy is ignored, argument copied inline anyway", name);
      flags &= ~flPhysicalCopy;
    }
    if (it->itPortType) {
      warn("%s: PhysicalCopy is ignored, it does not apply to ports and array of ports", name);
      flags &= ~flPhysicalCopy;
    }
  }
  return flags;
}

static void
rtProcessRetCodeFlag(register argument_t *thisarg)
{
  register ipc_type_t *it = thisarg->argType;
  register ipc_flags_t flags = thisarg->argFlags;
  string_t name = thisarg->argVarName;
  routine_t *thisrout = thisarg->argRoutine;

  if (flags & flRetCode) {
    if (!it->itInLine || !it->itStruct ||
        it->itSize != 32 || it->itNumber != 1) {
      warn("%s: RetCode is ignored - the type doesn't match a MIG RetCode", name);
      flags &= ~flRetCode;
    }
    else if (thisrout->rtKind != rkSimpleRoutine) {
      fatal("%s: RetCode is allowed only for SimpleRoutines", name);
    }
    else if (thisrout->rtRetCArg != argNULL) {
      warn("%s: RetCode is ignored - only one argument can be flagged as RetCode", name);
      flags &= ~flRetCode;
    }
    else {
      thisrout->rtRetCArg = thisarg;
    }
    thisarg->argFlags = flags;
  }
}

static ipc_flags_t
rtProcessOverwriteFlag(register ipc_type_t *it, register ipc_flags_t flags, register arg_kind_t kind, string_t name)
{
  if (flags & flOverwrite)
    if (it->itInLine || it->itMigInLine ||
        /* among In, Out, InOut, we want only the Out! */
        !akCheck(kind, akbReturn) || akCheck(kind, akbSend)) {
      warn("%s: Overwrite is ignored - it must be Out AND Ool!", name);
      flags &= ~flOverwrite;
    }
  return flags;
}

static void
rtDetectKPDArg(argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char *string;

  if  (IS_KERN_PROC_DATA(it)) {
    if (akCheck(arg->argKind, akbSendBody)) {
      arg->argKind = akRemFeature(arg->argKind, akbSendBody);
      arg->argKind = akAddFeature(arg->argKind, akbSendKPD);
    }
    if (akCheck(arg->argKind, akbReturnBody)) {
      arg->argKind = akRemFeature(arg->argKind, akbReturnBody);
      arg->argKind = akAddFeature(arg->argKind, akbReturnKPD);
    }
    if (it->itInLine) {
      string = "mach_msg_port_descriptor_t";
      arg->argKPD_Type = MACH_MSG_PORT_DESCRIPTOR;
    }
    else if (it->itPortType) {
      string = "mach_msg_ool_ports_descriptor_t";
      arg->argKPD_Type = MACH_MSG_OOL_PORTS_DESCRIPTOR;
    }
    else {
      string = "mach_msg_ool_descriptor_t";
      arg->argKPD_Type = MACH_MSG_OOL_DESCRIPTOR;
    }
    it->itKPDType = string;
  }
}

static void
rtAugmentArgKind(argument_t *arg)
{
  register ipc_type_t *it = arg->argType;

  /* akbVariable means variable-sized inline */

  if (IS_VARIABLE_SIZED_UNTYPED(it)) {
    if (akCheckAll(arg->argKind, akbRequest|akbReply))
      error("%s: Inline variable-sized arguments can't be InOut", arg->argName);
    arg->argKind = akAddFeature(arg->argKind, akbVariable);
  }
  if (IS_OPTIONAL_NATIVE(it))
    arg->argKind = akAddFeature(arg->argKind, akbVariable);

  /*
   * Need to use a local variable in the following cases:
   *  1) There is a translate-out function & the argument is being
   *     returned.  We need to translate it before it hits the message.
   *  2) There is a translate-in function & the argument is
   *     sent and returned.  We need a local variable for its address.
   *  3) There is a destructor function, which will be used
   *     (SendRcv and not ReturnSnd), and there is a translate-in
   *     function whose value must be saved for the destructor.
   *  4) This is Complex KPD (array of KPD), and as such it has to
   *     be copied to a local array in input and output
   *  5) Both poly and dealloc generate warnings compile time, because
   *     we attempt to take address of bit-field structure member
   */

  if (
      ((it->itOutTrans != strNULL) && akCheck(arg->argKind, akbReturnSnd)) ||
      ((it->itInTrans != strNULL) && akCheckAll(arg->argKind, akbSendRcv|akbReturnSnd)) ||
      ((it->itDestructor != strNULL) && akCheck(arg->argKind, akbSendRcv) && !akCheck(arg->argKind, akbReturnSnd) && (it->itInTrans != strNULL)) ||
      (IS_MULTIPLE_KPD(it)) ||
      ((akIdent(arg->argKind) == akePoly) && akCheck(arg->argKind, akbReturnSnd))  ||
      ((akIdent(arg->argKind) == akeDealloc) && akCheck(arg->argKind, akbReturnSnd))
     ) {
    arg->argKind = akRemFeature(arg->argKind, akbReplyCopy);
    arg->argKind = akAddFeature(arg->argKind, akbVarNeeded);
  }
}

/*
 * The Suffix allows to handle KPDs as normal data.
 * it is used in InArgMsgField.
 */
static void
rtSuffixExtArg(register argument_t *args)
{
  register argument_t *arg;
  register char *subindex;
  char string[MAX_STR_LEN];

  for (arg = args; arg != argNULL; arg = arg->argNext)  {
    if (akCheck(arg->argKind, akbSendKPD | akbReturnKPD)) {
      if (IS_MULTIPLE_KPD(arg->argType))
        subindex = "[0]";
      else
        subindex = "";
      switch (arg->argKPD_Type) {

        case MACH_MSG_PORT_DESCRIPTOR:
          (void)sprintf(string, "%s.name", subindex);
          break;

        case MACH_MSG_OOL_DESCRIPTOR:
        case MACH_MSG_OOL_PORTS_DESCRIPTOR:
          (void)sprintf(string, "%s.address", subindex);
          break;

        default:
          error("Type of kernel processed data unknown\n");
      }
      arg->argSuffix = strconcat(arg->argMsgField, string);
      /* see above the list of VarNeeded cases */
      /*
       * argCount has been removed from the VarNeeded list,
       * because VarSize arrays have their Count in the untyped
       * section of the message, and because it is not possible
       * to move anything in-line/out-of-line
       */
    }
    else if (akIdent(arg->argKind) == akePoly &&
               akCheck(arg->argParent->argKind, akbSendKPD | akbReturnKPD)) {
      register argument_t *par_arg = arg->argParent;

      if (IS_MULTIPLE_KPD(par_arg->argType))
        subindex = "[0]";
      else
        subindex = "";
      switch (par_arg->argKPD_Type) {

        case MACH_MSG_PORT_DESCRIPTOR:
        case MACH_MSG_OOL_PORTS_DESCRIPTOR:
          (void)sprintf(string, "%s.disposition", subindex);
          arg->argSuffix = strconcat(par_arg->argMsgField, string);
          break;

        default:
          error("Type of kernel processed data inconsistent\n");
      }
    }
    else if (akIdent(arg->argKind) == akeDealloc &&
               akCheck(arg->argParent->argKind, akbSendKPD | akbReturnKPD)) {
      register argument_t *par_arg = arg->argParent;

      if (IS_MULTIPLE_KPD(par_arg->argType))
        subindex = "[0]";
      else
        subindex = "";
      switch (par_arg->argKPD_Type) {

        case MACH_MSG_OOL_DESCRIPTOR:
        case MACH_MSG_OOL_PORTS_DESCRIPTOR:
          (void)sprintf(string, "%s.deallocate", subindex);
          arg->argSuffix = strconcat(par_arg->argMsgField, string);
          break;

        default:
          error("Type of kernel processed data inconsistent\n");
      }
    }
  }
}

/* arg->argType may be NULL in this function */

static void
rtCheckRoutineArg(routine_t *rt, argument_t *arg)
{
  switch (akIdent(arg->argKind)) {

    case akeRequestPort:
      if (rt->rtRequestPort != argNULL)
        warn("multiple RequestPort args in %s; %s won't be used", rt->rtName, rt->rtRequestPort->argName);
      rt->rtRequestPort = arg;
      break;

    case akeReplyPort:
      if (rt->rtReplyPort != argNULL)
        warn("multiple ReplyPort args in %s; %s won't be used", rt->rtName, rt->rtReplyPort->argName);
      rt->rtReplyPort = arg;
      break;

    case akeWaitTime:
      if (rt->rtWaitTime != argNULL)
        warn("multiple WaitTime/SendTime type args in %s; %s won't be used", rt->rtName, rt->rtWaitTime->argName);
      rt->rtWaitTime = arg;
      break;

    case akeSendTime:
      if (rt->rtWaitTime != argNULL)
	      if (akIdent(rt->rtWaitTime->argKind) == akeWaitTime) {
		      warn("SendTime type argument after a WaitTime in %s; SendTime %s won't be used", rt->rtName, arg->argName);
		      break;
	      } else 
		      warn("multiple SendTime type args in %s; %s won't be used", rt->rtName, rt->rtWaitTime->argName);
      rt->rtWaitTime = arg;
      break;

    case akeMsgOption:
      if (rt->rtMsgOption != argNULL)
        warn("multiple MsgOption args in %s; %s won't be used", rt->rtName, rt->rtMsgOption->argName);
      rt->rtMsgOption = arg;
      break;

    default:
      break;
  }
}

/* arg->argType may be NULL in this function */

static void
rtSetArgDefaults(routine_t *rt, register argument_t *arg)
{
  arg->argRoutine = rt;
  if (arg->argVarName == strNULL)
    arg->argVarName = arg->argName;
  if (arg->argMsgField == strNULL)
    switch(akIdent(arg->argKind)) {

      case akeRequestPort:
        arg->argMsgField = "Head.msgh_request_port";
        break;

      case akeReplyPort:
        arg->argMsgField = "Head.msgh_reply_port";
        break;

      case akeNdrCode:
        arg->argMsgField = "NDR";
        break;

      case akeSecToken:
        arg->argMsgField = "msgh_sender";
        break;

      case akeAuditToken:
        arg->argMsgField = "msgh_audit";
        break;

      case akeContextToken:
        arg->argMsgField = "msgh_context";
        break;

      case akeMsgSeqno:
        arg->argMsgField = "msgh_seqno";
        break;

      case akeImplicit:
        /* the field is set directly by Yacc */
        break;

      default:
        arg->argMsgField = arg->argName;
        break;
  }

  if (arg->argTTName == strNULL)
    arg->argTTName = strconcat(arg->argName, "Template");
  if (arg->argPadName == strNULL)
    arg->argPadName = strconcat(arg->argName, "Pad");

  /*
   *  The poly args for the request and reply ports have special defaults,
   *  because their msg-type-name values aren't stored in normal fields.
   */

  if ((rt->rtRequestPort != argNULL) &&
      (rt->rtRequestPort->argPoly == arg) &&
      (arg->argType != itNULL)) {
    arg->argMsgField = "Head.msgh_bits";
    arg->argType->itInTrans = "MACH_MSGH_BITS_REQUEST";
  }

  if ((rt->rtReplyPort != argNULL) &&
      (rt->rtReplyPort->argPoly == arg) &&
      (arg->argType != itNULL)) {
    arg->argMsgField = "Head.msgh_bits";
    arg->argType->itInTrans = "MACH_MSGH_BITS_REPLY";
  }
}

static void
rtAddCountArg(register argument_t *arg)
{
  register argument_t *count, *master;
  register ipc_type_t *it = arg->argType;

  count = argAlloc();

  if (IS_MULTIPLE_KPD(it) && it->itElement->itVarArray) {
    count->argName = strconcat(arg->argName, "Subs");
    count->argType = itMakeSubCountType(it->itKPD_Number, it->itVarArray, arg->argVarName);
    count->argKind = akeSubCount;
    arg->argSubCount = count;
  }
  else {
    count->argName = strconcat(arg->argName, "Cnt");
    count->argType = itMakeCountType();
    count->argKind = akeCount;
    arg->argCount = count;
    if (arg->argParent != argNULL)  {
      /* this is the case where we are at the second level of recursion:
       we want the Parent to access it through argCount */
      arg->argParent->argCount = count;
    }
  }
  master = (arg->argParent != argNULL) ? arg->argParent : arg;
  if (IS_MULTIPLE_KPD(master->argType))
    count->argMultiplier = 1;
  else
    count->argMultiplier = it->itElement->itNumber;
  count->argParent = arg;
  count->argNext = arg->argNext;
  arg->argNext = count;

  if (arg->argType->itString) {
    /* C String gets no Count argument on either side, but are still in the msg */
    count->argKind = akAddFeature(count->argKind, akCheck(arg->argKind, akbSend) ? akbSendRcv : akbReturnRcv);
    count->argVarName = (char *)0;
  }
  else {
    /*
     * Count arguments have to be present on the message body (NDR encoded)
     * akbVariable has to be turned down, has it foul the algorithm
     * for detecting the in-line variable sized arrays
     */
    count->argKind |= akAddFeature(akbUserArg|akbServerArg, (arg->argKind) & ~akeBITS);
    count->argKind = akRemFeature(count->argKind, akbVariable|akbVarNeeded);
    if (IS_VARIABLE_SIZED_UNTYPED(arg->argType))
    /*
     * Count arguments for the above types are explicitly declared
     * BEFORE the variable (with those bits, they would come afterwards)
     */
    count->argKind = akRemFeature(count->argKind, akbRequest|akbReply);
  }
}

static void
rtAddCountInOutArg(register argument_t *arg)
{
  register argument_t *count;

  /*
   *  The user sees a single count variable.  However, to get the
   *  count passed from user to server for variable-sized inline OUT
   *  arrays, we need two count arguments internally.  This is
   *  because the count value lives in different message fields (and
   *  is scaled differently) in the request and reply messages.
   *
   *  The two variables have the same name to simplify code generation.
   *
   *  This variable has a null argParent field because it has akbRequest.
   *  For example, see rtCheckVariable.
   */

  count = argAlloc();
  count->argName = strconcat(arg->argName, "Cnt");
  count->argType = itMakeCountType();
  count->argParent = argNULL;
  count->argNext = arg->argNext;
  arg->argNext = count;
  (count->argCInOut = arg->argCount)->argCInOut = count;
  count->argKind = akCountInOut;
}

static void
rtAddPolyArg(register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  register argument_t *poly;
  arg_kind_t akbsend, akbreturn;

  poly = argAlloc();
  poly->argName = strconcat(arg->argName, "Poly");
  poly->argType = itMakePolyType();
  poly->argParent = arg;
  poly->argNext = arg->argNext;
  arg->argNext = poly;
  arg->argPoly = poly;

  /*
   * akbsend is bits added if the arg is In;
   * akbreturn is bits added if the arg is Out.
   * The mysterious business with KernelServer subsystems:
   * when packing Out arguments, they use OutNames instead
   * of InNames, and the OutName determines if they are poly-in
   * as well as poly-out.
   */

  akbsend = akbSend;
  akbreturn = akbReturn;

  if (it->itInName == MACH_MSG_TYPE_POLYMORPHIC) {
    akbsend |= akbUserArg|akbSendSnd;
    if (!IsKernelServer)
      akbreturn |= akbServerArg|akbReturnSnd;
  }
  if (it->itOutName == MACH_MSG_TYPE_POLYMORPHIC) {
    akbsend |= akbServerArg|akbSendRcv;
    akbreturn |= akbUserArg|akbReturnRcv;
    if (IsKernelServer)
      akbreturn |= akbServerArg|akbReturnSnd;
  }

  poly->argKind = akPoly;
  if (akCheck(arg->argKind, akbSend))
    poly->argKind = akAddFeature(poly->argKind, akCheck(arg->argKind, akbsend));
  if (akCheck(arg->argKind, akbReturn))
    poly->argKind = akAddFeature(poly->argKind, akCheck(arg->argKind, akbreturn));
}

static void
rtAddDeallocArg(register argument_t *arg)
{
  register argument_t *dealloc;

  dealloc = argAlloc();
  dealloc->argName = strconcat(arg->argName, "Dealloc");
  dealloc->argType = itMakeDeallocType();
  dealloc->argParent = arg;
  dealloc->argNext = arg->argNext;
  arg->argNext = dealloc;
  arg->argDealloc = dealloc;

  /*
   * Dealloc flag can only be associated to KPDs.
   */

  dealloc->argKind = akeDealloc;
  if (akCheck(arg->argKind, akbSend))
    dealloc->argKind = akAddFeature(dealloc->argKind, akCheck(arg->argKind, akbUserArg|akbSend|akbSendSnd));
  if (akCheck(arg->argKind, akbReturn)) {
    dealloc->argKind = akAddFeature(dealloc->argKind, akCheck(arg->argKind, akbServerArg|akbReturn|akbReturnSnd));
    dealloc->argByReferenceServer = TRUE;
  }
}

static void
rtCheckRoutineArgs(routine_t *rt)
{
  register argument_t *arg;

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    register ipc_type_t *it = arg->argType;

    rtDefaultArgKind(rt, arg);
    rtCheckRoutineArg(rt, arg);

    /* need to set argTTName before adding implicit args */
    rtSetArgDefaults(rt, arg);

    /* the arg may not have a type (if there was some error in parsing it),
       in which case we don't want to do these steps. */

    if (it != itNULL) {
      arg->argFlags = rtProcessDeallocFlag(it, arg->argFlags, arg->argKind, &arg->argDeallocate, arg->argVarName);
      arg->argFlags = rtProcessCountInOutFlag(it, arg->argFlags, arg->argKind, &arg->argCountInOut, arg->argVarName);
      rtProcessSameCountFlag(arg);
      arg->argFlags = rtProcessPhysicalCopyFlag(it, arg->argFlags, arg->argKind, arg->argVarName);
      rtProcessRetCodeFlag(arg);
      arg->argFlags = rtProcessOverwriteFlag(it, arg->argFlags, arg->argKind, arg->argVarName);
      rtAugmentArgKind(arg);

      /* args added here will get processed in later iterations */
      /* order of args is 'arg poly countinout count dealloc' */

      if (arg->argDeallocate == d_MAYBE)
        rtAddDeallocArg(arg);
      if (it->itVarArray || (IS_MULTIPLE_KPD(it) && it->itElement->itVarArray))
        rtAddCountArg(arg);
      if (arg->argCountInOut)
        rtAddCountInOutArg(arg);
      if ((it->itInName == MACH_MSG_TYPE_POLYMORPHIC) || (it->itOutName == MACH_MSG_TYPE_POLYMORPHIC))
        rtAddPolyArg(arg);
      /*
       * Detects whether the arg has to become part of the
       * Kernel Processed Data section; if yes, define the proper
       * itUserKPDType, itServerKPDType
       */
      rtDetectKPDArg(arg);
    }
  }
}

boolean_t
rtCheckTrailerType(register argument_t *arg)
{
  if (akIdent(arg->argKind) == akeSecToken ||
      akIdent(arg->argKind) == akeAuditToken ||
      akIdent(arg->argKind) == akeContextToken )
    itCheckTokenType(arg->argVarName, arg->argType);

  if (akIdent(arg->argKind) == akeMsgSeqno)
    itCheckIntType(arg->argVarName, arg->argType);
  /*
   * if the built-in are not used, we cannot match
   * the type/size of the desciption provided by the user
   * with the one defined in message.h.
   */

  return TRUE;
}

static void
rtCheckArgTypes(routine_t *rt)
{
  if (rt->rtRequestPort == argNULL)
    error("%s %s doesn't have a server port argument", rtRoutineKindToStr(rt->rtKind), rt->rtName);

  if ((rt->rtRequestPort != argNULL) &&
      (rt->rtRequestPort->argType != itNULL))
    itCheckRequestPortType(rt->rtRequestPort->argName, rt->rtRequestPort->argType);

  if ((rt->rtReplyPort != argNULL) &&
      (rt->rtReplyPort->argType != itNULL))
    itCheckReplyPortType(rt->rtReplyPort->argName, rt->rtReplyPort->argType);

  if ((rt->rtWaitTime != argNULL) &&
      (rt->rtWaitTime->argType != itNULL))
    itCheckIntType(rt->rtWaitTime->argName, rt->rtWaitTime->argType);

  if ((rt->rtMsgOption != argNULL) &&
      (rt->rtMsgOption->argType != itNULL))
    itCheckIntType(rt->rtMsgOption->argName, rt->rtMsgOption->argType);

  if ((IsKernelServer && rt->rtServerImpl) ||
      (IsKernelUser   && rt->rtUserImpl))
    fatal("Implicit data is not supported in the KernelUser and KernelServer modes");
  /* rtCheckTrailerType will hit a fatal() if something goes wrong */
  if (rt->rtServerImpl)
    rtCheckMaskFunction(rt->rtArgs, akbServerImplicit, rtCheckTrailerType);
  if (rt->rtUserImpl)
    rtCheckMaskFunction(rt->rtArgs, akbUserImplicit, rtCheckTrailerType);
}

/*
 * Check for arguments which are missing seemingly needed functions.
 * We make this check here instead of in itCheckDecl, because here
 * we can take into account what kind of argument the type is
 * being used with.
 *
 * These are warnings, not hard errors, because mig will generate
 * reasonable code in any case.  The generated code will work fine
 * if the ServerType and TransType are really the same, even though
 * they have different names.
 */

static void
rtCheckArgTrans(routine_t *rt)
{
  register argument_t *arg;

  /* the arg may not have a type (if there was some error in parsing it) */

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    register ipc_type_t *it = arg->argType;

    if ((it != itNULL) && !streql(it->itServerType, it->itTransType)) {
      if (akCheck(arg->argKind, akbSendRcv) && (it->itInTrans == strNULL))
        warn("%s: argument has no in-translation function", arg->argName);

      if (akCheck(arg->argKind, akbReturnSnd) && (it->itOutTrans == strNULL))
        warn("%s: argument has no out-translation function", arg->argName);
    }
  }
}

/*
 * Adds an implicit return-code argument.  It exists in the reply message,
 * where it is the first piece of data (After the NDR format label)..
 */

static void
rtAddRetCode(routine_t *rt)
{
  register argument_t *arg = argAlloc();

  arg->argName = "RetCode";
  arg->argType = itRetCodeType;
  arg->argKind = akRetCode;
  rt->rtRetCode = arg;

  arg->argNext = rt->rtArgs;
  rt->rtArgs = arg;
}

/*
 * Process the Return Code.
 * The MIG protocol says that RetCode != 0 are only sent through
 * mig_reply_error_t structures. Therefore, there is no need
 * for reserving a RetCode in a complex Reply message.
 */
static void
rtProcessRetCode(routine_t *rt)
{
  if (!rt->rtOneWay && !rt->rtSimpleReply) {
    register argument_t *arg = rt->rtRetCode;

    arg->argKind = akRemFeature(arg->argKind, akbReply);
    /* we want the RetCode to be a local variable instead */
    arg->argKind = akAddFeature(arg->argKind, akbVarNeeded);
  }
  if (rt->rtRetCArg != argNULL && !rt->rtSimpleRequest) {
    register argument_t *arg = rt->rtRetCArg;

    arg->argKind = akeRetCode|akbUserArg|akbServerArg|akbSendRcv;
  }
}

/*
 * Adds an implicit NDR argument.  It exists in the reply message,
 * where it is the first piece of data.
 */

static void
rtAddNdrCode(routine_t *rt)
{
  register argument_t *arg = argAlloc();

  arg->argName = "NDR_record";
  arg->argType = itNdrCodeType;
  arg->argKind = akeNdrCode;
  rt->rtNdrCode = arg;

  /* add at beginning, so ndr-code is first in the reply message  */
  arg->argNext = rt->rtArgs;
  rt->rtArgs = arg;
}

/*
 * Process the NDR Code.
 * We stick a NDR format label iff there is untyped data
 */
static void
rtProcessNdrCode(routine_t *rt)
{
  register argument_t *ndr = rt->rtNdrCode;
  argument_t *arg;
  boolean_t found;

  /* akbSendSnd|akbSendBody initialize the NDR format label */
#define ndr_send akbRequest|akbSend|akbSendSnd|akbSendBody
  /* akbReplyInit initializes the NDR format label */
#define ndr_rcv  akbReply|akbReplyInit|akbReturn|akbReturnBody

  ndr->argKind = akAddFeature(ndr->argKind, ndr_send|ndr_rcv);

  for (found = FALSE, arg = ndr->argNext; arg != argNULL; arg = arg->argNext)
    if (akCheck(arg->argKind, akbSendRcv|akbSendBody) &&
       !akCheck(arg->argKind, akbServerImplicit) && !arg->argType->itPortType &&
       (!arg->argParent || akIdent(arg->argKind) == akeCount ||
       akIdent(arg->argKind) == akeCountInOut)) {
      arg->argKind = akAddFeature(arg->argKind, akbSendNdr);
      found = TRUE;
    }
  if (!found)
    ndr->argKind = akRemFeature(ndr->argKind, ndr_send);

  found = FALSE;
  if (!rt->rtOneWay)
    for (arg = ndr->argNext; arg != argNULL; arg = arg->argNext)
      if ((arg == rt->rtRetCode && akCheck(arg->argKind, akbReply)) ||
          (arg != rt->rtRetCode &&
          akCheck(arg->argKind, akbReturnRcv|akbReturnBody) &&
          !akCheck(arg->argKind, akbUserImplicit) && !arg->argType->itPortType &&
          (!arg->argParent || akIdent(arg->argKind) == akeCount ||
          akIdent(arg->argKind) == akeCountInOut))) {
        arg->argKind = akAddFeature(arg->argKind, akbReturnNdr);
        found = TRUE;
      }
  if (!found && !akCheck(rt->rtRetCode->argKind, akbReply))
    ndr->argKind = akRemFeature(ndr->argKind, ndr_rcv);
}

/*
 *  Adds a dummy WaitTime argument to the function.
 *  This argument doesn't show up in any C argument lists;
 *  it implements the global WaitTime statement.
 */

static void
rtAddWaitTime(routine_t *rt, identifier_t name, arg_kind_t kind)
{
  register argument_t *arg = argAlloc();
  argument_t **loc;

  arg->argName = "dummy WaitTime arg";
  arg->argVarName = name;
  arg->argType = itWaitTimeType;
  arg->argKind = kind;
  rt->rtWaitTime = arg;

  /* add wait-time after msg-option, if possible */

  if (rt->rtMsgOption != argNULL)
    loc = &rt->rtMsgOption->argNext;
  else
    loc = &rt->rtArgs;

  arg->argNext = *loc;
  *loc = arg;

  rtSetArgDefaults(rt, arg);
}

/*
 *  Adds a dummy MsgOption argument to the function.
 *  This argument doesn't show up in any C argument lists;
 *  it implements the global MsgOption statement.
 */

static void
rtAddMsgOption(routine_t *rt, identifier_t name)
{
  register argument_t *arg = argAlloc();
  argument_t **loc;

  arg->argName = "dummy MsgOption arg";
  arg->argVarName = name;
  arg->argType = itMsgOptionType;
  arg->argKind = akeMsgOption;
  rt->rtMsgOption = arg;

  /* add msg-option after msg-seqno */

  loc = &rt->rtArgs;

  arg->argNext = *loc;
  *loc = arg;

  rtSetArgDefaults(rt, arg);
}

/*
 * Process the MsgOption Code.
 * We must add the information to post a receive with the right
 * Trailer options.
 */
static void
rtProcessMsgOption(routine_t *rt)
{
  register argument_t *msgop = rt->rtMsgOption;
  register argument_t *arg;
  boolean_t sectoken = FALSE;
  boolean_t audittoken = FALSE;
  boolean_t contexttoken = FALSE;

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)
    if (akCheckAll(arg->argKind, akbReturn|akbUserImplicit)) {
      if (akIdent(arg->argKind) == akeSecToken)
        sectoken = TRUE;
      else if (akIdent(arg->argKind) == akeAuditToken)
        audittoken = TRUE;
      else if (akIdent(arg->argKind) == akeContextToken)
        contexttoken = TRUE;
    }

  if (contexttoken == TRUE)
    msgop->argVarName = strconcat(msgop->argVarName, "|MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_CTX)");
  else if (audittoken == TRUE)
    msgop->argVarName = strconcat(msgop->argVarName, "|MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_AUDIT)");
  else if (sectoken == TRUE)
    msgop->argVarName = strconcat(msgop->argVarName, "|MACH_RCV_TRAILER_ELEMENTS(MACH_RCV_TRAILER_SENDER)");
  /* other implicit data received by the user will be handled here */
}

/*
 *  Adds a dummy reply port argument to the function.
 */

static void
rtAddDummyReplyPort(routine_t *rt, ipc_type_t *type)
{
  register argument_t *arg = argAlloc();
  argument_t **loc;

  arg->argName = "dummy ReplyPort arg";
  arg->argVarName = "dummy ReplyPort arg";
  arg->argType = type;
  arg->argKind = akeReplyPort;
  rt->rtReplyPort = arg;

  /* add the reply port after the request port */

  if (rt->rtRequestPort != argNULL)
    loc = &rt->rtRequestPort->argNext;
  else
    loc = &rt->rtArgs;

  arg->argNext = *loc;
  *loc = arg;

  rtSetArgDefaults(rt, arg);
}


/*
 * At least one overwrite keyword has been detected:
 * we tag all the OOL entries (ports + data) with
 * akbOverwrite which will tell us that we have to
 * fill a KPD entry in the message-template
 */
static void
rtCheckOverwrite(register routine_t *rt)
{
  register argument_t *arg;
  register int howmany = rt->rtOverwrite;

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    register ipc_type_t *it = arg->argType;

    if (akCheck(arg->argKind, akbReturnKPD) && !it->itInLine) {
      /* among OUT args, we want OOL, OOL ports and MigInLine */
      arg->argKind = akAddFeature(arg->argKind, akbOverwrite);
      if (arg->argFlags & flOverwrite)
        howmany--;
      if (!howmany)
        return;
    }
  }
}

/*
 * Initializes argRequestPos, argReplyPos, rtMaxRequestPos, rtMaxReplyPos,
 * rtNumRequestVar, rtNumReplyVar, and adds akbVarNeeded to those arguments
 * that need it because of variable-sized inline considerations.
 *
 * argRequestPos and argReplyPos get -1 if the value shouldn't be used.
 */
static void
rtCheckVariable(register routine_t *rt)
{
  register argument_t *arg;
  int NumRequestVar = 0;
  int NumReplyVar = 0;
  int MaxRequestPos = 0;
  int MaxReplyPos = 0;

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    register argument_t *parent = arg->argParent;

    /*
     * We skip KPDs. We have to make sure that the KPDs count
     * present in the message body follow the RequestPos/ReplyPos logic
     * The rest of the parameters are defaulted to have
     * Arg{Request, Reply}Pos = 0
     */
    if (parent == argNULL || akCheck(parent->argKind, akbSendKPD|akbReturnKPD)) {
      if (akCheckAll(arg->argKind, akbSend|akbSendBody)) {
        arg->argRequestPos = NumRequestVar;
        MaxRequestPos = NumRequestVar;
        if (akCheck(arg->argKind, akbVariable))
          NumRequestVar++;
      }
      if (akCheckAll(arg->argKind, akbReturn|akbReturnBody)) {
        arg->argReplyPos = NumReplyVar;
        MaxReplyPos = NumReplyVar;
        if (akCheck(arg->argKind, akbVariable))
          NumReplyVar++;
      }
    }
    else {
      arg->argRequestPos = parent->argRequestPos;
      arg->argReplyPos = parent->argReplyPos;
    }
    /*
     printf("Var %s Kind %x RequestPos %d\n", arg->argVarName, arg->argKind, arg->argRequestPos);
     printf("* Var %s Kind %x ReplyPos %d\n", arg->argVarName, arg->argKind, arg->argReplyPos);
     */

    /* Out variables that follow a variable-sized field
       need VarNeeded or ReplyCopy; they can't be stored
       directly into the reply message. */

    if (akCheckAll(arg->argKind, akbReturnSnd|akbReturnBody) &&
        !akCheck(arg->argKind, akbReplyCopy|akbVarNeeded) &&
        (arg->argReplyPos > 0))
      arg->argKind = akAddFeature(arg->argKind, akbVarNeeded);
  }

  rt->rtNumRequestVar = NumRequestVar;
  rt->rtNumReplyVar = NumReplyVar;
  rt->rtMaxRequestPos = MaxRequestPos;
  rt->rtMaxReplyPos = MaxReplyPos;
}

/*
 * Adds akbDestroy where needed.
 */

static void
rtCheckDestroy(register routine_t *rt)
{
  register argument_t *arg;

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    register ipc_type_t *it = arg->argType;

    if(akCheck(arg->argKind, akbSendRcv) &&
       !akCheck(arg->argKind, akbReturnSnd) &&
       (it->itDestructor != strNULL || IS_MIG_INLINE_EMUL(it))) {
      arg->argKind = akAddFeature(arg->argKind, akbDestroy);
    }
    if (argIsIn(arg) && akCheck(arg->argKind, akbSendKPD|akbReturnKPD) &&
        arg->argKPD_Type == MACH_MSG_OOL_DESCRIPTOR &&
        (arg->argFlags & flAuto))
      arg->argKind = akAddFeature(arg->argKind, akbDestroy);
  }
}

/*
 * Sets ByReferenceUser and ByReferenceServer.
 */

static void
rtAddByReference(register routine_t *rt)
{
  register argument_t *arg;

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    register ipc_type_t *it = arg->argType;

    if (akCheck(arg->argKind, akbReturnRcv) && it->itStruct) {
      arg->argByReferenceUser = TRUE;

      /*
       *  A CountInOut arg itself is not akbReturnRcv,
       *  so we need to set argByReferenceUser specially.
       */

      if (arg->argCInOut != argNULL)
        arg->argCInOut->argByReferenceUser = TRUE;
    }

    if ((akCheck(arg->argKind, akbReturnSnd) ||
         (akCheck(arg->argKind, akbServerImplicit) &&
          akCheck(arg->argKind, akbReturnRcv) &&
          akCheck(arg->argKind, akbSendRcv)))
        && it->itStruct) {
      arg->argByReferenceServer = TRUE;
    }
  }
}

/*
 * This procedure can be executed only when all the akb* and ake* have
 * been set properly (when rtAddCountArg is executed, akbVarNeeded
 * might not be set yet - see rtCheckVariable)
 */
void
rtAddSameCount(register routine_t *rt)
{
  register argument_t *arg;

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)
    if (arg->argFlags & flSameCount) {
      register ipc_type_t *it = arg->argType;
      register argument_t *tmp_count;
      register argument_t *my_count = arg->argCount;
      register argument_t *ref_count = arg->argSameCount->argCount;

      tmp_count = argAlloc();
      *tmp_count = *ref_count;
      /*
       * if our count is a akbVarNeeded, we need to copy this
       * attribute to the master count!
       */
      tmp_count->argKind = akeSameCount;
      ref_count->argKind = akAddFeature(ref_count->argKind, akCheck(my_count->argKind, akbVarNeeded));
      tmp_count->argKind = akAddFeature(tmp_count->argKind, akCheck(my_count->argKind, akbVarNeeded));
      tmp_count->argNext = my_count->argNext;
      tmp_count->argMultiplier = my_count->argMultiplier;
      tmp_count->argType = my_count->argType;
      tmp_count->argParent = arg;
      /* we don't need more */
      arg->argCount = tmp_count;
      arg->argNext = tmp_count;
      /* for these args, Cnt is not akbRequest, and therefore size is embedded */
      if (IS_VARIABLE_SIZED_UNTYPED(it))
        it->itMinTypeSize = 0;
      tmp_count->argType->itMinTypeSize = 0;
      tmp_count->argType->itTypeSize = 0;
    }
}

void
rtCheckRoutine(register routine_t *rt)
{
  /* Initialize random fields. */

  rt->rtErrorName = ErrorProc;
  rt->rtOneWay = (rt->rtKind == rkSimpleRoutine);
  rt->rtServerName = strconcat(ServerPrefix, rt->rtName);
  rt->rtUserName = strconcat(UserPrefix, rt->rtName);

  /* Add implicit arguments. */

  rtAddRetCode(rt);
  rtAddNdrCode(rt);

  /* Check out the arguments and their types.  Add count, poly
   implicit args.  Any arguments added after rtCheckRoutineArgs
   should have rtSetArgDefaults called on them. */

  rtCheckRoutineArgs(rt);

  /* Add dummy WaitTime and MsgOption arguments, if the routine
   doesn't have its own args and the user specified global values. */

  if (rt->rtReplyPort == argNULL) {
    if (rt->rtOneWay)
      rtAddDummyReplyPort(rt, itZeroReplyPortType);
    else
      rtAddDummyReplyPort(rt, itRealReplyPortType);
  }
  if (rt->rtMsgOption == argNULL) {
    if (MsgOption == strNULL)
      rtAddMsgOption(rt, "MACH_MSG_OPTION_NONE");
    else
      rtAddMsgOption(rt, MsgOption);
  }
  if (rt->rtWaitTime == argNULL) {
	  if (WaitTime != strNULL)
		  rtAddWaitTime(rt, WaitTime, akeWaitTime);
	  else if (SendTime != strNULL)
		  rtAddWaitTime(rt, SendTime, akeSendTime);
  }


  /* Now that all the arguments are in place, do more checking. */

  rtCheckArgTypes(rt);
  rtCheckArgTrans(rt);

  if (rt->rtOneWay) {
    if (rtCheckMask(rt->rtArgs, akbReturn) || rt->rtUserImpl)
      error("%s %s has OUT argument", rtRoutineKindToStr(rt->rtKind), rt->rtName);
  }

  /* If there were any errors, don't bother calculating more info
   that is only used in code generation anyway.  Therefore,
   the following functions don't have to worry about null types. */

  if (mig_errors > 0)
    fatal("%d errors found. Abort.\n", mig_errors);

  rt->rtServerImpl  = rtCountMask(rt->rtArgs, akbServerImplicit);
  rt->rtUserImpl = rtCountMask(rt->rtArgs, akbUserImplicit);
  /*
   * ASSUMPTION:
   * kernel cannot change a message from simple to complex,
   * therefore SimpleSendReply and SimpleRcvReply become SimpleReply
   */
  rtCheckSimple(rt->rtArgs, akbRequest, &rt->rtSimpleRequest);
  rtCheckSimple(rt->rtArgs, akbReply, &rt->rtSimpleReply);

  rt->rtRequestKPDs = rtCountKPDs(rt->rtArgs, akbSendKPD);
  rt->rtReplyKPDs = rtCountKPDs(rt->rtArgs, akbReturnKPD);
  /*
   * Determine how many overwrite parameters we have:
   *  # of Overwrite args -> rt->rtOverwrite
   *  flOverwrite -> the arg has to be overwritten
   *  akbOverwrite -> the arg has to be declared in the message-template
   *  (only as a placeholder if !flOverwrite).
   */
  if ((rt->rtOverwrite = rtCountFlags(rt->rtArgs, flOverwrite))) {
    rtCheckOverwrite(rt);
    rt->rtOverwriteKPDs = rtCountKPDs(rt->rtArgs, akbReturnKPD|akbOverwrite);
    if (IsKernelUser)
      fatal("Overwrite option(s) do not match with the KernelUser personality\n");
  }

  rtCheckFit(rt, akbRequest, &rt->rtRequestFits, &rt->rtRequestUsedLimit, &rt->rtRequestSizeKnown);
  rtCheckFit(rt, akbReply, &rt->rtReplyFits, &rt->rtReplyUsedLimit, &rt->rtReplySizeKnown);

  rtCheckVariable(rt);
  rtCheckDestroy(rt);
  rtAddByReference(rt);
  rtSuffixExtArg(rt->rtArgs);
  rtAddSameCount(rt);
  rtProcessRetCode(rt);
  rtProcessNdrCode(rt);
  if (rt->rtUserImpl)
    rtProcessMsgOption(rt);

  rt->rtNoReplyArgs = !rtCheckMask(rt->rtArgs, akbReturnSnd);

  if (UseEventLogger)
  /* some more info's are needed for Event logging/Stats */
    rtFindHowMany(rt);
}

void
rtMinRequestSize(FILE *file, routine_t *rt, char *str)
{
  fprintf(file, "(mach_msg_size_t)(sizeof(%s)", str);
  rtSizeDelta(file, akbRequest, rt);
  fprintf(file, ")");
}

void
rtMinReplySize(FILE *file, routine_t *rt, char *str)
{
  fprintf(file, "(mach_msg_size_t)(sizeof(%s)", str);
  rtSizeDelta(file, akbReply, rt);
  fprintf(file, ")");
}

static void
rtSizeDelta(FILE *file, u_int mask, routine_t *rt)
{
  argument_t *arg;
  u_int min_size = sizeof(mach_msg_header_t);
  u_int max_size;
  boolean_t output = FALSE;

  if (!rt->rtSimpleRequest)
    machine_alignment(min_size, sizeof(mach_msg_body_t));
  max_size = min_size;
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)
    if (akCheck(arg->argKind, mask)) {
      register ipc_type_t *it = arg->argType;

      machine_alignment(min_size, it->itMinTypeSize);
      machine_alignment(max_size, it->itMinTypeSize);

      if (IS_VARIABLE_SIZED_UNTYPED(it)) {
        machine_alignment(max_size, it->itTypeSize);
        max_size += it->itPadSize;
      }
      if (IS_OPTIONAL_NATIVE(it)) {
        if (output)
          fprintf(file, " + ");
        else {
          output = TRUE;
          fprintf(file, " - (");
        }
        fprintf(file, "_WALIGNSZ_(%s)", it->itUserType);
      }
    }
  if (min_size != max_size) {
    if (output)
      fprintf(file, " + ");
    else
      fprintf(file, " - ");
    fprintf(file, "%d", max_size - min_size);
  }
  if (output)
    fprintf(file, ")");
}


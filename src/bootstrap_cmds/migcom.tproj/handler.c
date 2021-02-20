/*
 * Copyright (c) 1999-2018 Apple Inc. All rights reserved.
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
/********************************************************
 *  Abstract:
 *  routines to write pieces of the Handler module.
 *  exports WriteHandler which directs the writing of
 *  the Handler module
 *
 *
 *  $Header: /Users/Shared/bootstrap_cmds/bootstrap_cmds/migcom.tproj/handler.c,v 1.2 2005/02/06 07:28:59 lindak Exp $
 *
 * HISTORY
 * 03-Jul-97  Daniel Wade (danielw) at Apple
 *      Generated code is now ANSI C compliant
 *
 * 10-Sep-91  Gregg Kellogg (gk) at NeXT
 *  Created.
 *******************************************************/

#include <mach/message.h>
#include "write.h"
#include "utils.h"
#include "global.h"

static void
WriteIncludes(FILE *file)
{
  fprintf(file, "#define EXPORT_BOOLEAN\n");
  fprintf(file, "#include <mach/boolean.h>\n");
  fprintf(file, "#include <mach/message.h>\n");
  fprintf(file, "#include <mach/mig_errors.h>\n");
  if (IsCamelot) {
    fprintf(file, "#include <cam/camelot_types.h>\n");
    fprintf(file, "#include <mach/msg_type.h>\n");
  }
  fprintf(file, "#include \"%s\"\n", ServerHeaderFileName);
  fprintf(file, "\n");
}

static void
WriteGlobalDecls(FILE *file)
{
  fprintf(file, "#define novalue void\n");
  fprintf(file, "\n");

  if (RCSId != strNULL)
    WriteRCSDecl(file, strconcat(SubsystemName, "_handler"), RCSId);

  /* Used for locations in the request message, *not* reply message.
   Reply message locations aren't dependent on IsKernel. */

  if (IsKernel)
  {
    fprintf(file, "#define msg_request_port\tmsg_remote_port\n");
    fprintf(file, "#define msg_reply_port\t\tmsg_local_port\n");
  }
  else
  {
    fprintf(file, "#define msg_request_port\tmsg_local_port\n");
    fprintf(file, "#define msg_reply_port\t\tmsg_remote_port\n");
  }
}

static void
WriteProlog(FILE *file)
{
  fprintf(file, "/* Module %s */\n", SubsystemName);
  fprintf(file, "\n");

  WriteIncludes(file);
  WriteBogusDefines(file);
  WriteGlobalDecls(file);
}


static void
WriteSymTabEntries(FILE *file  statement_t *stats)
{
  statement_t *stat;
  u_int current = 0;

  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine) {
      int num = stat->stRoutine->rtNumber;
      char      *name = stat->stRoutine->rtName;
      while (++current <= num)
        fprintf(file,"\t\t\t{ \"\", 0, 0 },\n");
      fprintf(file, "\t{ \"%s\", %d, _X%s },\n",
              name,
              SubsystemBase + current - 1,
              name);
    }
  while (++current <= rtNumber)
    fprintf(file,"\t{ \"\", 0, 0 },\n");
}

static void
WriteArrayEntries(FILE *file, statement_t *stats)
{
  u_int current = 0;
  statement_t *stat;

  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine)
    {
      routine_t *rt = stat->stRoutine;

      while (current++ < rt->rtNumber)
        fprintf(file, "\t\t\t0,\n");
      fprintf(file, "\t\t\t_X%s,\n", rt->rtName);
    }
  while (current++ < rtNumber)
    fprintf(file, "\t\t\t0,\n");
}

static void
WriteEpilog(FILE *file, statement_t *stats)
{
  statement_t *stat;
  u_int MaxReply = 0;

  for (stat = stats; stat != stNULL; stat = stat->stNext) {
    if (stat->stKind != skRoutine)
      continue;
    if (stat->stRoutine->rtMaxReplySize > MaxReply)
      MaxReply = stat->stRoutine->rtMaxReplySize;
  }

  fprintf(file, "\n");

  fprintf(file, "kern_return_t %s (\n", ServerProcName);
  fprintf(file, "\tmsg_header_t *InHeadP,\n\t%s_t *%s)\n", SubsystemName, SubsystemName);

  fprintf(file, "{\n");
  fprintf(file, "\tchar OutBuf[%d];\n", MaxReply);
  fprintf(file, "\tmsg_header_t *InP =  InHeadP;\n");

  if (IsCamelot)
    fprintf(file, "\tcamelot_death_pill_t *OutP = (camelot_death_pill_t *) OutBuf;\n");
  else
    fprintf(file, "\tdeath_pill_t *OutP = (death_pill_t *) OutBuf;\n");

  fprintf(file, "\n");

  WriteStaticDecl(file, itRetCodeType, itRetCodeType->itDeallocate, itRetCodeType->itLongForm, "RetCodeType");
  fprintf(file, "\n");

  if (IsCamelot)
  {
    WriteStaticDecl(file, itDummyType, itDummyType->itDeallocate, itDummyType->itLongForm, "DummyType");
    fprintf(file, "\n");
    WriteStaticDecl(file, itTidType, itTidType->itDeallocate, itTidType->itLongForm, "TidType");
    fprintf(file, "\n");
  }

  fprintf(file, "\tOutP->Head.msg_simple = TRUE;\n");
  fprintf(file, "\tOutP->Head.msg_size = (mach_msg_size_t)sizeof(*OutP);\n");
  fprintf(file, "\tOutP->Head.msg_type = InP->msg_type;\n");
  fprintf(file, "\tOutP->Head.msg_local_port = PORT_NULL;\n");
  fprintf(file, "\tOutP->Head.msg_remote_port = InP->msg_reply_port;\n");
  fprintf(file, "\tOutP->Head.msg_id = InP->msg_id + 100;\n");
  fprintf(file, "\n");
  WritePackMsgType(file, itRetCodeType, itRetCodeType->itDeallocate, itRetCodeType->itLongForm, "OutP->RetCodeType", "RetCodeType");
  fprintf(file, "\tOutP->RetCode = MIG_BAD_ID;\n");
  fprintf(file, "\n");

  if (IsCamelot)
  {
    WritePackMsgType(file, itDummyType, itDummyType->itDeallocate, itDummyType->itLongForm, "OutP->DummyType", "DummyType");
    fprintf(file, "\t/* dummy doesn't need a value */\n");
    fprintf(file, "\n");
    WritePackMsgType(file, itTidType, itTidType->itDeallocate, itTidType->itLongForm, "OutP->TidType", "TidType");
    fprintf(file, "\tOutP->Tid = ((camelot_death_pill_t *)InP)->Tid;\n");
    fprintf(file, "\n");
  }

  fprintf(file, "\tif ((InP->msg_id > %d) || (InP->msg_id < %d))\n", SubsystemBase + rtNumber - 1, SubsystemBase);
  fprintf(file, "\t\treturn OutP->RetCode;\n");
  fprintf(file, "\telse {\n");
  fprintf(file, "\t\ttypedef novalue (*SERVER_STUB_PROC) (\n");
  fprintf(file, "\t\t\tmsg_header_t *,\n"
          "\t\t\tmsg_header_t *,\n"
          "\t\t\t%s_t *);\n", SubsystemName);
  fprintf(file, "\t\tstatic const SERVER_STUB_PROC routines[] = {\n");

  WriteArrayEntries(file, stats);

  fprintf(file, "\t\t};\n");
  fprintf(file, "\n");

  /* Call appropriate routine */
  fprintf(file, "\t\tif (routines[InP->msg_id - %d])\n", SubsystemBase);
  fprintf(file, "\t\t\t(routines[InP->msg_id - %d]) (\n"
          "\t\t\t\tInP, &OutP->Head, %s);\n",
          SubsystemBase, SubsystemName);
  fprintf(file, "\t\t else\n");
  fprintf(file, "\t\t\treturn MIG_BAD_ID;\n");

  fprintf(file, "\t}\n");

  fprintf(file, "\tif (OutP->RetCode == MIG_NO_REPLY)\n");
  fprintf(file, "\t\treturn KERN_SUCCESS;\n");
  fprintf(file, "\treturn msg_send(&OutP->Head,\n");
  fprintf(file, "\t\t%s->timeout >= 0 ? SEND_TIMEOUT : MSG_OPTION_NONE,\n", SubsystemName);
  fprintf(file, "\t\t%s->timeout);\n", SubsystemName);
  fprintf(file, "}\n");

  /* symtab */

  if (GenSymTab) {
    fprintf(file,"\nmig_symtab_t _%sSymTab[] = {\n",SubsystemName);
    WriteSymTabEntries(file,stats);
    fprintf(file,"};\n");
    fprintf(file,"int _%sSymTabBase = %d;\n",SubsystemName,SubsystemBase);
    fprintf(file,"int _%sSymTabEnd = %d;\n",SubsystemName,SubsystemBase+rtNumber);
  }
}

/*
 *  Returns the return type of the server-side work function.
 *  Suitable for "extern %s serverfunc()".
 */
static char *
HandlerSideType(routine_t *rt)
{
  if (rt->rtServerReturn == argNULL)
    return "void";
  else
    return rt->rtServerReturn->argType->itTransType;
}

static void
WriteLocalVarDecl(FILE *file, argument_t *arg)
{
  ipc_type_t *it = arg->argType;

  if (it->itInLine && it->itVarArray)
  {
    ipc_type_t *btype = it->itElement;

    fprintf(file, "\t%s %s[%d]", btype->itTransType, arg->argVarName, it->itNumber/btype->itNumber);
  }
  else
    fprintf(file, "\t%s %s", it->itTransType, arg->argVarName);
}

static void
_WriteHandlerVarDecl(FILE *file, argument_t *arg)
{
  fprintf(file, "%s %s%s", arg->argType->itTransType, argByReferenceServer(arg) ? "*" : "", arg->argVarName);
}

/*
 *  Writes the local variable declarations which are always
 *  present:  InP, OutP, the server-side work function.
 */
static void
WriteVarDecls(FILE *file, routine_t *rt)
{
  int i;

  fprintf(file, "\tRequest *In0P = (Request *) InHeadP;\n");
  for (i = 1; i <= rt->rtMaxRequestPos; i++)
    fprintf(file, "\tRequest *In%dP;\n", i);
  fprintf(file, "\tReply *OutP = (Reply *) OutHeadP;\n");
  fprintf(file, "\n");

  fprintf(file, "#if\t__MigTypeCheck\n");
  fprintf(file, "\tboolean_t msg_simple;\n");
  fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  fprintf(file, "\n");

  fprintf(file, "\tunsigned int msg_size;\n");

  /* if either request or reply is variable, we need msg_size_delta */
  if ((rt->rtNumRequestVar > 0) || (rt->rtNumReplyVar > 0))
    fprintf(file, "\tunsigned int msg_size_delta;\n");

  fprintf(file, "\n");
}

static void
WriteMsgError(FILE *file, argument_t *arg, char *error)
{
  if (arg == argNULL)
    fprintf(file, "\t\t{ OutP->RetCode = %s; return; }\n", error);
  else {
    fprintf(file, "\t\t{ OutP->RetCode = %s; goto punt%d; }\n", error, arg->argPuntNum);
    fprintf(file, "#define\tlabel_punt%d\n", arg->argPuntNum);
  }
}

static void
WriteReplyInit(FILE *file, routine_t *rt)
{
  fprintf(file, "\n");
  fprintf(file, "\tmsg_size = %d;\t\n", rt->rtReplySize);
  if (rt->rtNumReplyVar > 0)
    fprintf(file, "\t/* Maximum reply size %d */\n", rt->rtMaxReplySize);
}

static void
WriteReplyHead(FILE *file, routine_t *rt)
{
  fprintf(file, "\n");
  if (rt->rtMaxReplyPos > 0)
    fprintf(file, "\tOutP = (Reply *) OutHeadP;\n");

  fprintf(file, "\tOutP->Head.msg_simple = %s;\n", strbool(rt->rtSimpleSendReply));
  fprintf(file, "\tOutP->Head.msg_size = msg_size;\n");
}

static void
WriteCheckHead(FILE *file, routine_t *rt)
{
  fprintf(file, "#if\t__MigTypeCheck\n");
  fprintf(file, "\tmsg_size = In0P->Head.msg_size;\n");
  fprintf(file, "\tmsg_simple = In0P->Head.msg_simple;\n");

  if (rt->rtNumRequestVar > 0) {
    fprintf(file, "\tif ((msg_size < %d)", rt->rtRequestSize);
    fprintf(file, " || (msg_size > %d)", rt->rtMaxRequestSize);
  }
  else
    fprintf(file, "\tif ((msg_size != %d)", rt->rtRequestSize);
  if (rt->rtSimpleCheckRequest)
    fprintf(file, " || (msg_simple != %s)", strbool(rt->rtSimpleReceiveRequest));
  fprintf(file, ")\n");
  WriteMsgError(file, argNULL, "MIG_BAD_ARGUMENTS");
  fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  fprintf(file, "\n");
}

static void
WriteTypeCheck(FILE *file, argument_t *arg)
{
  ipc_type_t *it = arg->argType;
  routine_t *rt = arg->argRoutine;

  fprintf(file, "#if\t__MigTypeCheck\n");
  if (akCheck(arg->argKind, akbQuickCheck))
  {
    fprintf(file, "#if\tUseStaticMsgType\n");
    fprintf(file, "\tif (* (int *) &In%dP->%s != * (int *) &%sCheck)\n", arg->argRequestPos, arg->argTTName, arg->argVarName);
    fprintf(file, "#else\t/* UseStaticMsgType */\n");
  }
  fprintf(file, "\tif ((In%dP->%s%s.msg_type_inline != %s) ||\n", arg->argRequestPos, arg->argTTName, arg->argLongForm ? ".msg_type_header" : "", strbool(it->itInLine));
  fprintf(file, "\t    (In%dP->%s%s.msg_type_longform != %s) ||\n", arg->argRequestPos, arg->argTTName, arg->argLongForm ? ".msg_type_header" : "", strbool(arg->argLongForm));
  if (it->itOutName == MSG_TYPE_POLYMORPHIC)
  {
    if (!rt->rtSimpleCheckRequest)
      fprintf(file, "\t    (MSG_TYPE_PORT_ANY(In%dP->%s.msg_type_%sname) && msg_simple) ||\n", arg->argRequestPos, arg->argTTName, arg->argLongForm ? "long_" : "");
  }
  else
    fprintf(file, "\t    (In%dP->%s.msg_type_%sname != %s) ||\n", arg->argRequestPos, arg->argTTName, arg->argLongForm ? "long_" : "", it->itOutNameStr);
  if (!it->itVarArray)
    fprintf(file, "\t    (In%dP->%s.msg_type_%snumber != %d) ||\n", arg->argRequestPos, arg->argTTName, arg->argLongForm ? "long_" : "", it->itNumber);
  fprintf(file, "\t    (In%dP->%s.msg_type_%ssize != %d))\n",  arg->argRequestPos, arg->argTTName, arg->argLongForm ? "long_" : "", it->itSize);
  if (akCheck(arg->argKind, akbQuickCheck))
    fprintf(file, "#endif\t/* UseStaticMsgType */\n");
  WriteMsgError(file, arg, "MIG_BAD_ARGUMENTS");
  fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  fprintf(file, "\n");
}

static void
WriteCheckMsgSize(FILE *file, argument_t *arg)
{
  routine_t *rt = arg->argRoutine;
  ipc_type_t *btype = arg->argType->itElement;
  argument_t *count = arg->argCount;
  boolean_t NoMoreArgs, LastVarArg;

  /* If there aren't any more In args after this, then
     msg_size_delta value will only get used by TypeCheck code,
     so put the assignment under the TypeCheck conditional. */

  NoMoreArgs = arg->argRequestPos == rt->rtMaxRequestPos;

  /* If there aren't any more variable-sized arguments after this,
   then we must check for exact msg-size and we don't need
   to update msg_size. */

  LastVarArg = arg->argRequestPos+1 == rt->rtNumRequestVar;

  if (NoMoreArgs)
    fprintf(file, "#if\t__MigTypeCheck\n");

  /* calculate the actual size in bytes of the data field.  note
     that this quantity must be a multiple of four.  hence, if
     the base type size isn't a multiple of four, we have to
     round up.  note also that btype->itNumber must
     divide btype->itTypeSize (see itCalculateSizeInfo). */

  if (btype->itTypeSize % 4 != 0)
    fprintf(file, "\tmsg_size_delta = (%d * In%dP->%s + 3) & ~3;\n", btype->itTypeSize/btype->itNumber, arg->argRequestPos, count->argMsgField);
  else
    fprintf(file, "\tmsg_size_delta = %d * In%dP->%s;\n", btype->itTypeSize/btype->itNumber, arg->argRequestPos, count->argMsgField);

  if (!NoMoreArgs)
    fprintf(file, "#if\t__MigTypeCheck\n");

  /* Don't decrement msg_size until we've checked it won't underflow. */

  if (LastVarArg)
    fprintf(file, "\tif (msg_size != %d + msg_size_delta)\n", rt->rtRequestSize);
  else
    fprintf(file, "\tif (msg_size < %d + msg_size_delta)\n", rt->rtRequestSize);
  WriteMsgError(file, arg, "MIG_BAD_ARGUMENTS");

  if (!LastVarArg)
    fprintf(file, "\tmsg_size -= msg_size_delta;\n");

  fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  fprintf(file, "\n");
}

static void
WriteExtractArgValue(FILE *file,  argument_t *arg)
{
  ipc_type_t *it = arg->argType;

  if (arg->argMultiplier > 1)
    WriteCopyType(file, it, FALSE, "%s /* %d %s %d */", "/* %s */ In%dP->%s / %d", arg->argVarName, arg->argRequestPos, arg->argMsgField, arg->argMultiplier);
  else if (it->itInTrans != strNULL)
    WriteCopyType(file, it, FALSE, "%s /* %s %d %s */", "/* %s */ %s(In%dP->%s)", arg->argVarName, it->itInTrans, arg->argRequestPos, arg->argMsgField);
  else
    WriteCopyType(file, it, FALSE, "%s /* %d %s */", "/* %s */ In%dP->%s", arg->argVarName, arg->argRequestPos, arg->argMsgField);
  fprintf(file, "\n");
}

static void
WriteInitializeCount(FILE *file, argument_t *arg)
{
  ipc_type_t *ptype = arg->argParent->argType;
  ipc_type_t *btype = ptype->itElement;

  /*
   *  Initialize 'count' argument for variable-length inline OUT parameter
   *  with maximum allowed number of elements.
   */

  fprintf(file, "\t%s = %d;\n", arg->argVarName, ptype->itNumber/btype->itNumber);
  fprintf(file, "\n");
}

static void
WriteExtractArg(FILE *file, argument_t *arg)
{
  if (akCheck(arg->argKind, akbRequest))
    WriteTypeCheck(file, arg);

  if (akCheckAll(arg->argKind, akbVariable|akbRequest))
    WriteCheckMsgSize(file, arg);

  if (akCheckAll(arg->argKind, akbSendRcv|akbVarNeeded))
    WriteExtractArgValue(file, arg);

  if ((akIdent(arg->argKind) == akeCount) && akCheck(arg->argKind, akbReturnSnd)) {
    ipc_type_t *ptype = arg->argParent->argType;

    if (ptype->itInLine && ptype->itVarArray)
      WriteInitializeCount(file, arg);
  }

  /* This assumes that the count argument directly follows the
   associated variable-sized argument and any other implicit
   arguments it may have. */

  if ((akIdent(arg->argKind) == akeCount) && akCheck(arg->argKind, akbSendRcv) && (arg->argRequestPos < arg->argRoutine->rtMaxRequestPos)) {
    ipc_type_t *ptype = arg->argParent->argType;

    if (ptype->itInLine && ptype->itVarArray) {
      fprintf(file, "\tIn%dP = (Request *) ((char *) In%dP + msg_size_delta - %d);\n", arg->argRequestPos+1, arg->argRequestPos, ptype->itTypeSize + ptype->itPadSize);
      fprintf(file, "\n");
    }
  }
}

static void
WriteHandlerCallArg(FILE *file, argument_t *arg)
{
  ipc_type_t *it = arg->argType;
  boolean_t NeedClose = FALSE;

  if (argByReferenceServer(arg))
    fprintf(file, "&");

  if ((it->itInTrans != strNULL) && akCheck(arg->argKind, akbSendRcv) && !akCheck(arg->argKind, akbVarNeeded)) {
    fprintf(file, "%s(", it->itInTrans);
    NeedClose = TRUE;
  }

  if (akCheck(arg->argKind, akbVarNeeded))
    fprintf(file, "%s", arg->argVarName);
  else if (akCheck(arg->argKind, akbSendRcv))
    fprintf(file, "In%dP->%s", arg->argRequestPos, arg->argMsgField);
  else
    fprintf(file, "OutP->%s", arg->argMsgField);

  if (NeedClose)
    fprintf(file, ")");

  if (!argByReferenceServer(arg) && (arg->argMultiplier > 1))
    fprintf(file, " / %d", arg->argMultiplier);
}

static void
WriteDestroyArg(FILE *file, argument_t *arg)
{
  ipc_type_t *it = arg->argType;

  fprintf(file, "#ifdef\tlabel_punt%d\n", arg->argPuntNum+1);
  fprintf(file, "#undef\tlabel_punt%d\n", arg->argPuntNum+1);
  fprintf(file, "punt%d:\n", arg->argPuntNum+1);
  fprintf(file, "#endif\t/* label_punt%d */\n", arg->argPuntNum+1);

  if (akCheck(arg->argKind, akbVarNeeded))
    fprintf(file, "\t%s(%s);\n", it->itDestructor, arg->argVarName);
  else
    fprintf(file, "\t%s(In%dP->%s);\n", it->itDestructor, arg->argRequestPos, arg->argMsgField);
}

static void
WriteHandlerCall(FILE *file, routine_t *rt)
{
  boolean_t NeedClose = FALSE;

  fprintf(file, "\tif (%s->%s == 0)\n", SubsystemName, rt->rtName);
  WriteMsgError(file, argNULL, "MIG_BAD_ID");

  fprintf(file, "\t");
  if (rt->rtServerReturn != argNULL) {
    argument_t *arg = rt->rtServerReturn;
    ipc_type_t *it = arg->argType;

    if (rt->rtOneWay)
      fprintf(file, "(void) ");
    else
      fprintf(file, "OutP->%s = ", arg->argMsgField);
    if (it->itOutTrans != strNULL) {
      fprintf(file, "%s(", it->itOutTrans);
      NeedClose = TRUE;
    }
  }
  fprintf(file, "(*%s->%s)(%s->arg", SubsystemName, rt->rtName, SubsystemName);
  WriteListSkipFirst(file, rt->rtArgs, WriteHandlerCallArg, akbServerArg,
                     ", ", "");
  if (NeedClose)
    fprintf(file, ")");
  fprintf(file, ");\n");
}

static void
WriteGetReturnValue(FILE *file, routine_t *rt)
{
  fprintf(file, "\t" "OutP->%s = %s;\n", rt->rtRetCode->argMsgField, rt->rtOneWay ? "MIG_NO_REPLY" : "KERN_SUCCESS");
}

static void
WriteCheckReturnValue(FILE *file, routine_t *rt)
{
  fprintf(file, "\tif (OutP->%s != KERN_SUCCESS)\n", rt->rtRetCode->argMsgField);
  fprintf(file, "\t\treturn;\n");
}

static void
WritePackArgType(FILE *file, argument_t *arg)
{
  fprintf(file, "\n");

  WritePackMsgType(file, arg->argType, arg->argDeallocate, arg->argLongForm, "OutP->%s", "%s", arg->argTTName);
}

static void
WritePackArgValue(FILE *file, argument_t *arg)
{
  ipc_type_t *it = arg->argType;

  fprintf(file, "\n");

  if (it->itInLine && it->itVarArray) {
    argument_t *count = arg->argCount;
    ipc_type_t *btype = it->itElement;

    /* Note btype->itNumber == count->argMultiplier */

    fprintf(file, "\tbcopy((char *) %s, (char *) OutP->%s, ", arg->argVarName, arg->argMsgField);
    fprintf(file, "%d * %s);\n", btype->itTypeSize, count->argVarName);
  }
  else if (arg->argMultiplier > 1)
    WriteCopyType(file, it, TRUE, "OutP->%s /* %d %s */", "/* %s */ %d * %s", arg->argMsgField, arg->argMultiplier, arg->argVarName);
  else if (it->itOutTrans != strNULL)
    WriteCopyType(file, it, TRUE, "OutP->%s /* %s %s */", "/* %s */ %s(%s)", arg->argMsgField, it->itOutTrans, arg->argVarName);
  else
    WriteCopyType(file, it, TRUE, "OutP->%s /* %s */", "/* %s */ %s", arg->argMsgField, arg->argVarName);
}

static void
WriteCopyArgValue(FILE *file, argument_t *arg)
{
  fprintf(file, "\n");
  WriteCopyType(file, arg->argType, TRUE, "/* %d */ OutP->%s", "In%dP->%s", arg->argRequestPos, arg->argMsgField);
}

static void
WriteAdjustMsgSize(FILE *file, argument_t *arg)
{
  ipc_type_t *ptype = arg->argParent->argType;
  ipc_type_t *btype = ptype->itElement;

  fprintf(file, "\n");

  /* calculate the actual size in bytes of the data field.
     note that this quantity must be a multiple of four.
     hence, if the base type size isn't a multiple of four,
     we have to round up. */

  if (btype->itTypeSize % 4 != 0)
    fprintf(file, "\tmsg_size_delta = (%d * %s + 3) & ~3;\n", btype->itTypeSize, arg->argVarName);
  else
    fprintf(file, "\tmsg_size_delta = %d * %s;\n", btype->itTypeSize, arg->argVarName);

  fprintf(file, "\tmsg_size += msg_size_delta;\n");

  /* Don't bother moving OutP unless there are more Out arguments. */
  if (arg->argReplyPos < arg->argRoutine->rtMaxReplyPos) {
    fprintf(file, "\tOutP = (Reply *) ((char *) OutP + ");
    fprintf(file, "msg_size_delta - %d);\n", ptype->itTypeSize + ptype->itPadSize);
  }
}

static void
WritePackArg(FILE *file, argument_t *arg)
{
  if (akCheck(arg->argKind, akbReplyInit))
    WritePackArgType(file, arg);

  if (akCheckAll(arg->argKind, akbReturnSnd|akbVarNeeded))
    WritePackArgValue(file, arg);

  if (akCheck(arg->argKind, akbReplyCopy))
    WriteCopyArgValue(file, arg);

  if ((akIdent(arg->argKind) == akeCount) && akCheck(arg->argKind, akbReturnSnd)) {
    ipc_type_t *ptype = arg->argParent->argType;

    if (ptype->itInLine && ptype->itVarArray)
      WriteAdjustMsgSize(file, arg);
  }
}

static void
WriteFieldDecl(FILE *file, argument_t *arg)
{
  WriteFieldDeclPrim(file, arg, FetchServerType);
}

static void
WriteRoutine(FILE *file, routine_t *rt)
{
  fprintf(file, "\n");

  fprintf(file, "/* %s %s */\n", rtRoutineKindToStr(rt->rtKind), rt->rtName);
  fprintf(file, "mig_internal novalue _X%s (\n", rt->rtName);
  fprintf(file, "\tmsg_header_t *InHeadP,\n"
          "\tmsg_header_t *OutHeadP,\n"
          "\t%s_t *%s)\n", SubsystemName, SubsystemName);

  fprintf(file, "{\n");
  WriteStructDecl(file, rt->rtArgs, WriteFieldDecl, akbRequest, "Request");
  WriteStructDecl(file, rt->rtArgs, WriteFieldDecl, akbReply, "Reply");

  WriteVarDecls(file, rt);

  WriteList(file, rt->rtArgs, WriteCheckDecl, akbQuickCheck|akbRequest, "\n", "\n");
  WriteList(file, rt->rtArgs, WriteTypeDecl, akbReplyInit, "\n", "\n");

  WriteList(file, rt->rtArgs, WriteLocalVarDecl, akbVarNeeded, ";\n", ";\n\n");

  WriteCheckHead(file, rt);

  WriteList(file, rt->rtArgs, WriteExtractArg, akbNone, "", "");

  WriteHandlerCall(file, rt);
  WriteGetReturnValue(file, rt);

  /* In reverse order so we can jump into the middle. */

  WriteReverseList(file, rt->rtArgs, WriteDestroyArg, akbDestroy, "", "");
  fprintf(file, "#ifdef\tlabel_punt0\n");
  fprintf(file, "#undef\tlabel_punt0\n");
  fprintf(file, "punt0:\n");
  fprintf(file, "#endif\t/* label_punt0 */\n");

  if (rt->rtOneWay)
    fprintf(file, "\t;\n");
  else {
    WriteCheckReturnValue(file, rt);
    WriteReplyInit(file, rt);
    WriteList(file, rt->rtArgs, WritePackArg, akbNone, "", "");
    WriteReplyHead(file, rt);
  }

  fprintf(file, "}\n");
}

void
WriteHandler(FILE *file, statement_t *stats)
{
  statement_t *stat;

  WriteProlog(file);
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    switch (stat->stKind) {

      case skRoutine:
        WriteRoutine(file, stat->stRoutine);
        break;

      case skImport:
      case skSImport:
        WriteImport(file, stat->stFileName);
        break;

      case skUImport:
        break;

      default:
        fatal("WriteHandler(): bad statement_kind_t (%d)", (int) stat->stKind);
  }
  WriteEpilog(file, stats);
}


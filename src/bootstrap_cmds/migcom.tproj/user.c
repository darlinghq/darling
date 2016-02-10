/*
 * Copyright (c) 1999-2003, 2008-2009 Apple Inc. All rights reserved.
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

#include <stdlib.h>
#include <assert.h>

#include <mach/message.h>
#include "write.h"
#include "error.h"
#include "utils.h"
#include "global.h"

#ifndef USE_IMMEDIATE_SEND_TIMEOUT
#define USE_IMMEDIATE_SEND_TIMEOUT 0
#endif

char *MessAllocRoutine = "mig_user_allocate";
char *MessFreeRoutine = "mig_user_deallocate";

char stRetCode[] = "ReturnValue";
char stRetNone[] = "";

void WriteLogDefines();
void WriteIdentificationString();

static void
WriteKPD_Iterator(FILE *file, boolean_t in, boolean_t overwrite, boolean_t varying, argument_t *arg, boolean_t bracket)
{
  register ipc_type_t *it = arg->argType;
  char string[MAX_STR_LEN];
  
  fprintf(file, "\t{\n");
  fprintf(file, "\t    register\t%s\t*ptr;\n", it->itKPDType);
  fprintf(file, "\t    register int\ti");
  if (varying && !in)
    fprintf(file, ", j");
  fprintf(file, ";\n\n");
  
  if (in)
    sprintf(string, "InP");
  else if (overwrite)
    sprintf(string, "InOvTemplate");
  else
    sprintf(string, "Out%dP", arg->argRequestPos);
  
  fprintf(file, "\t    ptr = &%s->%s[0];\n", string, arg->argMsgField);
  
  if (varying) {
    register argument_t *count = arg->argCount;
    register char *cref = count->argByReferenceUser ? "*" : "";
    
    if (in || overwrite) {
      fprintf(file, "\t    if (%s%s > %d)\n", cref, count->argVarName, it->itKPD_Number);
      WriteReturnMsgError(file, arg->argRoutine, TRUE, arg, "MIG_ARRAY_TOO_LARGE");
      fprintf(file, "\t    for (i = 0; i < %s%s; ptr++, i++) %s\n", cref, count->argVarName, (bracket) ? "{" : "");
    }
    else {
      fprintf(file, "\t    j = min(Out%dP->%s, %s%s);\n", count->argReplyPos, count->argVarName, cref, count->argVarName);
      fprintf(file, "\t    for (i = 0; i < j; ptr++, i++) %s\n",(bracket) ? "{" : "");
}
  }
  else
    fprintf(file, "\t    for (i = 0; i < %d; ptr++, i++) %s\n", it->itKPD_Number, (bracket) ? "{" : "");
}

/*************************************************************
 * Writes the standard includes. The subsystem specific
 * includes  are in <SubsystemName>.h and writen by
 * header:WriteHeader. Called by WriteProlog.
 *************************************************************/
static void
WriteMyIncludes(FILE *file, statement_t *stats)
{
#ifdef MIG_KERNEL_PORT_CONVERSION
  if (IsKernelServer)
  {
    /*
     * We want to get the user-side definitions of types
     * like task_t, ipc_space_t, etc. in mach/mach_types.h.
     */
    
    fprintf(file, "#undef\tMACH_KERNEL\n");
    
    if (InternalHeaderFileName != strNULL)
    {
      register char *cp;
      
      /* Strip any leading path from InternalHeaderFileName. */
      cp = strrchr(InternalHeaderFileName, '/');
      if (cp == 0)
        cp = InternalHeaderFileName;
      else
        cp++; /* skip '/' */
      fprintf(file, "#include \"%s\"\n", cp);
    }
  }
#endif
  
  if (UserHeaderFileName == strNULL || UseSplitHeaders)
    WriteIncludes(file, TRUE, FALSE);
  if (UserHeaderFileName != strNULL)
  {
    register char *cp;
    
    /* Strip any leading path from UserHeaderFileName. */
    cp = strrchr(UserHeaderFileName, '/');
    if (cp == 0)
      cp = UserHeaderFileName;
    else
      cp++; /* skip '/' */
    fprintf(file, "#include \"%s\"\n", cp);
  }
  if (UseSplitHeaders)
    WriteImplImports(file, stats, TRUE);
  
  if (UseEventLogger) {
    if (IsKernelUser) {
      fprintf(file, "#if\t__MigKernelSpecificCode\n");
      fprintf(file, "#include <mig_debug.h>\n");
      fprintf(file, "#endif\t/* __MigKernelSpecificCode */\n");
    }
    fprintf(file, "#if  MIG_DEBUG\n");
    fprintf(file, "#include <mach/mig_log.h>\n");
    fprintf(file, "#endif /* MIG_DEBUG */\n");
  }
  if (BeLint) {
    fprintf(file, "/* LINTLIBRARY */\n");
  }
  fprintf(file, "\n");
  if (!BeAnsiC) {
    fprintf(file, "#if\t%s\n", NewCDecl);
    fprintf(file, "#else\t/* %s */\n", NewCDecl);
    fprintf(file, "extern mach_port_t mig_get_reply_port();\n");
    fprintf(file, "extern void mig_dealloc_reply_port();\n");
    fprintf(file, "extern char *%s();\n", MessAllocRoutine);
    fprintf(file, "extern void %s();\n", MessFreeRoutine);
    fprintf(file, "#endif\t/* %s */\n", NewCDecl);
  }
  fprintf(file, "\n");
}

static void
WriteGlobalDecls(FILE *file)
{
  if (RCSId != strNULL)
    WriteRCSDecl(file, strconcat(SubsystemName, "_user"), RCSId);
  
  fprintf(file, "#define msgh_request_port\tmsgh_remote_port\n");
  fprintf(file, "#define msgh_reply_port\t\tmsgh_local_port\n");
  fprintf(file, "\n");
  if (UseEventLogger)
    WriteLogDefines(file, "MACH_MSG_LOG_USER");
  fprintf(file, "\n");
}

static void
WriteOneMachErrorDefine(FILE *file, char *name, boolean_t timeout)
{
  fprintf(file, "#ifndef\t%s\n", name);
  fprintf(file, "#define\t%s(_R_) { \\\n", name);
  fprintf(file, "\tswitch (_R_) { \\\n");
  fprintf(file, "\tcase MACH_SEND_INVALID_DATA: \\\n");
  fprintf(file, "\tcase MACH_SEND_INVALID_DEST: \\\n");
  fprintf(file, "\tcase MACH_SEND_INVALID_HEADER: \\\n");
  fprintf(file, "\t\tmig_put_reply_port(InP->Head.msgh_reply_port); \\\n");
  fprintf(file, "\t\tbreak; \\\n");
  if (timeout) {
	  fprintf(file, "\tcase MACH_SEND_TIMED_OUT: \\\n");
	  fprintf(file, "\tcase MACH_RCV_TIMED_OUT: \\\n");
  }
  fprintf(file, "\tdefault: \\\n");
  fprintf(file, "\t\tmig_dealloc_reply_port(InP->Head.msgh_reply_port); \\\n");
  fprintf(file, "\t} \\\n}\n");
  fprintf(file, "#endif\t/* %s */\n", name);
  fprintf(file, "\n");
}

static void
WriteMachErrorDefines(FILE *file)
{
  WriteOneMachErrorDefine(file, "__MachMsgErrorWithTimeout", TRUE);
  WriteOneMachErrorDefine(file, "__MachMsgErrorWithoutTimeout", FALSE);
}

static void
WriteMIGCheckDefines(FILE *file)
{
  fprintf(file, "#define\t__MIG_check__Reply__%s_subsystem__ 1\n", SubsystemName);
  fprintf(file, "\n");
}
  
static void
WriteNDRDefines(FILE *file)
{
  fprintf(file, "#define\t__NDR_convert__Reply__%s_subsystem__ 1\n", SubsystemName);
  fprintf(file, "#define\t__NDR_convert__mig_reply_error_subsystem__ 1\n");
  fprintf(file, "\n");
}

/*************************************************************
 * Writes the standard #includes, #defines, and
 * RCS declaration. Called by WriteUser.
 *************************************************************/
static void
WriteProlog(FILE *file, statement_t *stats)
{
  WriteIdentificationString(file);
  WriteMIGCheckDefines(file);
  if (CheckNDR)
	  WriteNDRDefines(file);
  WriteMyIncludes(file, stats);
  WriteBogusDefines(file);
  WriteMachErrorDefines(file);
  WriteApplDefaults(file, "Send");
  WriteGlobalDecls(file);
}

/*ARGSUSED*/
static void
WriteEpilog(FILE *file)
{
  /* nothing to see here, move along... */
}

static string_t
WriteHeaderPortType(argument_t *arg)
{
  if (arg->argType->itInName == MACH_MSG_TYPE_POLYMORPHIC)
    return arg->argPoly->argVarName;
  else
    return arg->argType->itInNameStr;
}

static void
WriteRequestHead(FILE *file, routine_t *rt)
{
  if (rt->rtRetCArg != argNULL && !rt->rtSimpleRequest)
    fprintf(file, "ready_to_send:\n");
  
  if (rt->rtMaxRequestPos > 0) {
    if (rt->rtOverwrite)
      fprintf(file, "\tInP = &MessRequest;\n");
    else
      fprintf(file, "\tInP = &Mess%sIn;\n", (rtMessOnStack(rt) ? "." : "->"));
  }
  
  fprintf(file, "\tInP->Head.msgh_bits =");
  if (rt->rtRetCArg == argNULL && !rt->rtSimpleRequest)
    fprintf(file, " MACH_MSGH_BITS_COMPLEX|");
  fprintf(file, "\n");
  fprintf(file, "\t\tMACH_MSGH_BITS(%s, %s);\n", WriteHeaderPortType(rt->rtRequestPort), WriteHeaderPortType(rt->rtReplyPort));
  if (rt->rtRetCArg != argNULL && !rt->rtSimpleRequest) {
    fprintf(file, "\tif (!%s)\n", rt->rtRetCArg->argVarName);
    fprintf(file, "\t\tInP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;\n");
  }
  
  
  fprintf(file, "\t/* msgh_size passed as argument */\n");
  
  /*
   * KernelUser stubs need to cast the request and reply ports
   * from ipc_port_t to mach_port_t.
   */
  
#ifdef MIG_KERNEL_PORT_CONVERSION
  if (IsKernelUser)
    fprintf(file, "\tInP->%s = (mach_port_t) %s;\n", rt->rtRequestPort->argMsgField, rt->rtRequestPort->argVarName);
  else
#endif
    fprintf(file, "\tInP->%s = %s;\n", rt->rtRequestPort->argMsgField, rt->rtRequestPort->argVarName);
  
  if (akCheck(rt->rtReplyPort->argKind, akbUserArg)) {
#ifdef MIG_KERNEL_PORT_CONVERSION
    if (IsKernelUser)
      fprintf(file, "\tInP->%s = (mach_port_t) %s;\n", rt->rtReplyPort->argMsgField, rt->rtReplyPort->argVarName);
    else
#endif
      fprintf(file, "\tInP->%s = %s;\n", rt->rtReplyPort->argMsgField, rt->rtReplyPort->argVarName);
  }
  else if (rt->rtOneWay)
    fprintf(file, "\tInP->%s = MACH_PORT_NULL;\n", rt->rtReplyPort->argMsgField);
  else
    fprintf(file, "\tInP->%s = mig_get_reply_port();\n", rt->rtReplyPort->argMsgField);
  
  fprintf(file, "\tInP->Head.msgh_id = %d;\n", rt->rtNumber + SubsystemBase);


  if (IsVoucherCodeAllowed && !IsKernelUser && !IsKernelServer) {
    fprintf(file, "\t\n/* BEGIN VOUCHER CODE */\n\n");
    fprintf(file, "#ifdef USING_VOUCHERS\n");
    fprintf(file, "\tif (voucher_mach_msg_set != NULL) {\n");
    fprintf(file, "\t\tvoucher_mach_msg_set(&InP->Head);\n");
    fprintf(file, "\t}\n");
    fprintf(file, "#endif // USING_VOUCHERS\n");
    fprintf(file, "\t\n/* END VOUCHER CODE */\n");
  }
}

/*************************************************************
 *  Writes declarations for the message types, variables
 *  and return  variable if needed. Called by WriteRoutine.
 *************************************************************/
static void
WriteVarDecls(FILE *file, routine_t *rt)
{
  register int i;
  
  if (rt->rtOverwrite) {
    fprintf(file, "\tRequest MessRequest;\n");
    fprintf(file, "\tRequest *InP = &MessRequest;\n\n");
    
    fprintf(file, "\tunion {\n");
    fprintf(file, "\t\tOverwriteTemplate In;\n");
    fprintf(file, "\t\tReply Out;\n");
    fprintf(file, "\t} MessReply;\n");
    
    fprintf(file, "\tOverwriteTemplate *InOvTemplate = &MessReply.In;\n");
    fprintf(file, "\tReply *Out0P = &MessReply.Out;\n");
    for (i = 1; i <= rt->rtMaxReplyPos; i++)
      fprintf(file, "\t" "Reply *Out%dP = NULL;\n", i);
  }
  else {
    if (rtMessOnStack(rt))
      fprintf(file, "\tunion {\n");
    else
      fprintf(file, "\tunion %sMessU {\n", rt->rtName);
    fprintf(file, "\t\tRequest In;\n");
    if (!rt->rtOneWay)
      fprintf(file, "\t\tReply Out;\n");
    if (rtMessOnStack(rt))
      fprintf(file, "\t} Mess;\n");
    else
      fprintf(file, "\t} *Mess = (union %sMessU *) %s(sizeof(*Mess));\n",
              rt->rtName, MessAllocRoutine);
    fprintf(file, "\n");
    
    fprintf(file, "\tRequest *InP = &Mess%sIn;\n", (rtMessOnStack(rt) ? "." : "->"));
    if (!rt->rtOneWay) {
      fprintf(file, "\tReply *Out0P = &Mess%sOut;\n", (rtMessOnStack(rt) ? "." : "->"));
      for (i = 1; i <= rt->rtMaxReplyPos; i++)
        fprintf(file, "\t" "Reply *Out%dP = NULL;\n", i);
    }
  }
  
  fprintf(file, "\n");
  
  fprintf(file, "\tmach_msg_return_t msg_result;\n");
  
  /* if request is variable, we need msgh_size_delta and msgh_size */
  if (rt->rtNumRequestVar > 0)
    fprintf(file, "\tunsigned int msgh_size;\n");
  if (rt->rtMaxRequestPos > 0)
    fprintf(file, "\tunsigned int msgh_size_delta;\n");
  if (rt->rtNumRequestVar > 1 || rt->rtMaxRequestPos > 0)
    fprintf(file, "\n");
  
  if (rt->rtUserImpl) {
    fprintf(file, "\tmach_msg_max_trailer_t *TrailerP;\n");
    fprintf(file, "#if\t__MigTypeCheck\n");
    fprintf(file, "\tunsigned int trailer_size;\n");
    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  }
  fprintf(file, "\n");
  fprintf(file, "#ifdef\t__MIG_check__Reply__%s_t__defined\n", rt->rtName);
  fprintf(file, "\tkern_return_t check_result;\n");
  fprintf(file, "#endif\t/* __MIG_check__Reply__%s_t__defined */\n", rt->rtName);
  fprintf(file, "\n");
  WriteApplMacro(file, "Send", "Declare", rt);
  fprintf(file, "\n");
}

static void
WriteReturn(FILE *file, routine_t *rt, char *before, char *value, char *after)
{
  if (rtMessOnStack(rt)) {
    if (value != stRetCode) {
      /* get the easy case (no braces needed) out of the way */
      fprintf(file, "%sreturn%s%s;%s", before, (*value ? " " : ""), value, after);
      return;
    }
    else {
      fprintf(file, "%s{\n", before);
      fprintf(file, "%s\treturn Out0P->RetCode;\n%s}%s", before, before, after);
      return;
    }
  }
  
  if (value == stRetCode) {
    fprintf(file, "%s{\n%s\t%s ReturnValue;\n", before, before, ReturnTypeStr(rt));
    fprintf(file, "%s\tReturnValue = Out0P->RetCode;\n%s\t", before, before);
  }
  else {
    fprintf(file, "%s{ ", before);
  }
  
  fprintf(file, "%s((char *) Mess, sizeof(*Mess)); ", MessFreeRoutine);
  
  if (value == stRetCode)
    fprintf(file, "return ReturnValue;\n%s}%s", before, after);
  else if (value == stRetNone)
    fprintf(file, "return; }%s", after);
  else
    fprintf(file, "return %s; }%s", value, after);
}

static void
WriteRetCodeArg(FILE *file, register routine_t *rt)
{
  if (rt->rtRetCArg != argNULL && !rt->rtSimpleRequest) {
    register argument_t *arg = rt->rtRetCArg;
    
    fprintf(file, "\tif (%s) {\n", arg->argVarName);
    fprintf(file, "\t\t((mig_reply_error_t *)InP)->RetCode = %s;\n", arg->argVarName);
    fprintf(file, "\t\t((mig_reply_error_t *)InP)->NDR = NDR_record;\n");
    fprintf(file, "\t\tgoto ready_to_send;\n");
    fprintf(file, "\t}\n\n");
  }
}

/*************************************************************
 *   Writes the logic to check for a message send timeout, and
 *   deallocate any relocated ool data so as not to leak.
 *************************************************************/
static void
WriteMsgCheckForTimeout(FILE *file, routine_t *rt)
{
  if (rt->rtWaitTime != argNULL) {    /* no reason to test for timeout if no timeout was specified... */
    argument_t  *arg_ptr;
    fputs("\n\t"    "if (msg_result == MACH_SEND_TIMED_OUT) {" "\n", file);
    
    // iterate over arg list
    for (arg_ptr = rt->rtArgs; arg_ptr != NULL; arg_ptr = arg_ptr->argNext) {
      
      //  if argument contains ool data
      if (akCheck(arg_ptr->argKind, akbSendKPD) && arg_ptr->argKPD_Type == MACH_MSG_OOL_DESCRIPTOR) {
        //    generate code to test current arg address vs. address before the msg_send call
        //    if not at the same address, mig_deallocate the argument
        fprintf(file, "\t\tif((vm_offset_t) InP->%s.address != (vm_offset_t) %s)\n",
		arg_ptr->argVarName, arg_ptr->argVarName);
        fprintf(file, "\t\t\t"   "mig_deallocate((vm_offset_t) InP->%s.address, "
		"(vm_size_t) InP->%s.size);\n", arg_ptr->argVarName, arg_ptr->argVarName);
      }
    }
    
    fputs("\t"      "}" "\n\n", file);
  }
  return;
}

/*************************************************************
 *   Writes the send call when there is to be no subsequent
 *   receive. Called by WriteRoutine SimpleRoutines
 *************************************************************/
static void
WriteMsgSend(FILE *file, routine_t *rt)
{
  char *SendSize = "";
  char string[MAX_STR_LEN];
  
  if (rt->rtNumRequestVar == 0)
    SendSize = "(mach_msg_size_t)sizeof(Request)";
  else
    SendSize = "msgh_size";
  
  if (rt->rtRetCArg != argNULL && !rt->rtSimpleRequest) {
    sprintf(string, "(%s) ? (mach_msg_size_t)sizeof(mig_reply_error_t) : ", rt->rtRetCArg->argVarName);
    SendSize = strconcat(string, SendSize);
  }
  
  if (IsKernelUser) {
    fprintf(file, "#if\t__MigKernelSpecificCode\n");
    fprintf(file, "\tmsg_result = mach_msg_send_from_kernel(");
    fprintf(file, "&InP->Head, %s);\n", SendSize);
    fprintf(file, "#else\n");
  }
  fprintf(file, "\tmsg_result = mach_msg("
          "&InP->Head, MACH_SEND_MSG|%s%s, %s, 0, MACH_PORT_NULL, %s, MACH_PORT_NULL);\n",
          rt->rtWaitTime !=argNULL ? "MACH_SEND_TIMEOUT|" : "",
          rt->rtMsgOption->argVarName,
          SendSize,
          rt->rtWaitTime != argNULL ? rt->rtWaitTime->argVarName:"MACH_MSG_TIMEOUT_NONE");
  
  if (IsKernelUser) {
    fprintf(file, "#endif /* __MigKernelSpecificCode */\n");
  }
  
  WriteApplMacro(file, "Send", "After", rt);
  
  WriteMsgCheckForTimeout(file, rt);
  
  WriteReturn(file, rt, "\t", "msg_result", "\n");
}

/*************************************************************
 *  Writes to code to check for error returns from receive.
 *  Called by WriteMsgSendReceive and WriteMsgRPC
 *************************************************************/
static void
WriteMsgCheckReceive(FILE *file, routine_t *rt, char *success)
{
  fprintf(file, "\tif (msg_result != %s) {\n", success);
  if (!akCheck(rt->rtReplyPort->argKind, akbUserArg))
  {
    /* If we aren't using a user-supplied reply port, then
       deallocate the reply port when it is invalid or
       for TIMED_OUT errors. */
#ifdef DeallocOnAnyError
    fprintf(file, "\t\tmig_dealloc_reply_port(InP->Head.msgh_reply_port);\n");
#else
    if (rt->rtWaitTime != argNULL)
      fprintf(file, "\t\t__MachMsgErrorWithTimeout(msg_result);\n");
    else
      fprintf(file, "\t\t__MachMsgErrorWithoutTimeout(msg_result);\n");
#endif
  }
  WriteReturnMsgError(file, rt, TRUE, argNULL, "msg_result");
  fprintf(file, "\t}\n");
}

/*************************************************************
 *  Writes the send and receive calls and code to check
 *  for errors. Normally the rpc code is generated instead
 *  although, the subsytem can be compiled with the -R option
 *  which will cause this code to be generated. Called by
 *  WriteRoutine if UseMsgRPC option is false.
 *************************************************************/
static void
WriteMsgSendReceive(FILE *file, routine_t *rt)
{
  char *SendSize = "";
  char string[MAX_STR_LEN];
  
  if (rt->rtNumRequestVar == 0)
    SendSize = "(mach_msg_size_t)sizeof(Request)";
  else
    SendSize = "msgh_size";
  
  if (rt->rtRetCArg != argNULL && !rt->rtSimpleRequest) {
    sprintf(string, "(%s) ? (mach_msg_size_t)sizeof(mig_reply_error_t) : ", rt->rtRetCArg->argVarName);
    SendSize = strconcat(string, SendSize);
  }
  
  /* IsKernelUser to be done! */
  fprintf(file, "\tmsg_result = mach_msg(&InP->Head, MACH_SEND_MSG|%s%s, %s, 0, ", rt->rtWaitTime != argNULL ? "MACH_SEND_TIMEOUT|" : "", rt->rtMsgOption->argVarName, SendSize);
  fprintf(file, " MACH_PORT_NULL, %s, MACH_PORT_NULL);\n",
#if !USE_IMMEDIATE_SEND_TIMEOUT
          (rt->rtWaitTime != argNULL) ? rt->rtWaitTime->argVarName :
#endif
          "MACH_MSG_TIMEOUT_NONE");
  fprintf(file, "\tif (msg_result != MACH_MSG_SUCCESS)\n");
  WriteReturnMsgError(file, rt, TRUE, argNULL, "msg_result");
  fprintf(file, "\n");
  
  fprintf(file, "\tmsg_result = mach_msg(&Out0P->Head, MACH_RCV_MSG|%s%s%s, 0, (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_local_port, %s, MACH_PORT_NULL);\n",
          rt->rtUserImpl != 0 ? "MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0)|" : "",
          (rt->rtWaitTime != argNULL && akIdent(rt->rtWaitTime->argKind) == akeWaitTime) ? "MACH_RCV_TIMEOUT|" : "",
          rt->rtMsgOption->argVarName,
          (rt->rtWaitTime != argNULL && akIdent(rt->rtWaitTime->argKind) == akeWaitTime) ? rt->rtWaitTime->argVarName : "MACH_MSG_TIMEOUT_NONE");
  WriteApplMacro(file, "Send", "After", rt);
  WriteMsgCheckReceive(file, rt, "MACH_MSG_SUCCESS");
  fprintf(file, "\n");
}

/*************************************************************
 *  Writes the rpc call and the code to check for errors.
 *  This is the default code to be generated. Called by WriteRoutine
 *  for all routine types except SimpleRoutine.
 *************************************************************/
static void
WriteMsgRPC(FILE *file, routine_t *rt)
{
  char *SendSize = "";
  char string[MAX_STR_LEN];
  
  if (rt->rtNumRequestVar == 0)
    SendSize = "(mach_msg_size_t)sizeof(Request)";
  else
    SendSize = "msgh_size";
  
  if (rt->rtRetCArg != argNULL && !rt->rtSimpleRequest) {
    sprintf(string, "(%s) ? (mach_msg_size_t)sizeof(mig_reply_error_t) : ", rt->rtRetCArg->argVarName);
    SendSize = strconcat(string, SendSize);
  }
  
  if (IsKernelUser) {
    fprintf(file, "#if\t(__MigKernelSpecificCode) || (_MIG_KERNELSPECIFIC_CODE_)\n");
    fprintf(file, "\tmsg_result = mach_msg_rpc_from_kernel(&InP->Head, %s, (mach_msg_size_t)sizeof(Reply));\n", SendSize);
    fprintf(file, "#else\n");
  }
  if (rt->rtOverwrite) {
    fprintf(file, "\tmsg_result = mach_msg_overwrite(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|MACH_RCV_OVERWRITE|%s%s%s, %s, (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, %s, MACH_PORT_NULL, ",
            rt->rtUserImpl != 0 ? "MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0)|" : "",
            rt->rtWaitTime != argNULL ? 
	        (akIdent(rt->rtWaitTime->argKind) == akeWaitTime ? "MACH_SEND_TIMEOUT|MACH_RCV_TIMEOUT|" : "MACH_SEND_TIMEOUT|") : "",
            rt->rtMsgOption->argVarName,
            SendSize,
            rt->rtWaitTime != argNULL? rt->rtWaitTime->argVarName : "MACH_MSG_TIMEOUT_NONE");
    fprintf(file, " &InOvTemplate->Head, (mach_msg_size_t)sizeof(OverwriteTemplate));\n");
  }
  else {
    fprintf(file, "\tmsg_result = mach_msg(&InP->Head, MACH_SEND_MSG|MACH_RCV_MSG|%s%s%s, %s, (mach_msg_size_t)sizeof(Reply), InP->Head.msgh_reply_port, %s, MACH_PORT_NULL);\n",
            rt->rtUserImpl != 0 ? "MACH_RCV_TRAILER_TYPE(MACH_MSG_TRAILER_FORMAT_0)|" : "",
            rt->rtWaitTime != argNULL ?
	        (akIdent(rt->rtWaitTime->argKind) == akeWaitTime ? "MACH_SEND_TIMEOUT|MACH_RCV_TIMEOUT|" : "MACH_SEND_TIMEOUT|") : "",
            rt->rtMsgOption->argVarName,
            SendSize,
            rt->rtWaitTime != argNULL? rt->rtWaitTime->argVarName : "MACH_MSG_TIMEOUT_NONE");
  }
  if (IsKernelUser)
    fprintf(file,"#endif /* __MigKernelSpecificCode */\n");
  WriteApplMacro(file, "Send", "After", rt);
  
  WriteMsgCheckForTimeout(file, rt);
  
  WriteMsgCheckReceive(file, rt, "MACH_MSG_SUCCESS");
  fprintf(file, "\n");
}

/*
 * argKPD_Pack discipline for Port types.
 */
static void
WriteKPD_port(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char *subindex = "";
  char *recast = "";
  char firststring[MAX_STR_LEN];
  char string[MAX_STR_LEN];
  register char *ref = arg->argByReferenceUser ? "*" : "";
  ipc_type_t *real_it;
  
  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, TRUE, FALSE, it->itVarArray, arg, TRUE);
    (void)sprintf(firststring, "\t*ptr");
    (void)sprintf(string, "\tptr->");
    subindex = "[i]";
    real_it = it->itElement;
  }
  else {
    (void)sprintf(firststring, "InP->%s", arg->argMsgField);
    (void)sprintf(string, "InP->%s.", arg->argMsgField);
    real_it = it;
  }
  
#ifdef MIG_KERNEL_PORT_CONVERSION
  if (IsKernelUser && streql(real_it->itUserType, "ipc_port_t"))
    recast = "(mach_port_t)";
#endif
  fprintf(file, "#if\tUseStaticTemplates\n");
  fprintf(file, "\t%s = %s;\n", firststring, arg->argTTName);
  /* ref is required also in the Request part, because of inout parameters */
  fprintf(file, "\t%sname = %s%s%s%s;\n", string, recast, ref, arg->argVarName, subindex);
  if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbSendSnd)) {
    register argument_t *poly = arg->argPoly;
    
    fprintf(file, "\t%sdisposition = %s%s;\n", string, poly->argByReferenceUser ? "*" : "", poly->argVarName);
  }
  fprintf(file, "#else\t/* UseStaticTemplates */\n");
  fprintf(file, "\t%sname = %s%s%s%s;\n", string, recast, ref, arg->argVarName, subindex);
  if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbSendSnd)) {
    register argument_t *poly = arg->argPoly;
    
    fprintf(file, "\t%sdisposition = %s%s;\n", string, poly->argByReferenceUser ? "*" : "", poly->argVarName);
  }
  else
    fprintf(file, "\t%sdisposition = %s;\n", string, it->itInNameStr);
  fprintf(file, "\t%stype = MACH_MSG_PORT_DESCRIPTOR;\n", string);
  fprintf(file, "#endif\t/* UseStaticTemplates */\n");
  if (IS_MULTIPLE_KPD(it))  {
    fprintf(file, "\t    }\n");
    if (it->itVarArray) {
      fprintf(file, "\t    for (i = %s; i < %d; ptr++, i++) {\n", arg->argCount->argVarName, it->itKPD_Number);
      /* fill the rest of the statically allocated KPD entries with MACH_PORT_NULL */
      fprintf(file, "#if\tUseStaticTemplates\n");
      fprintf(file, "\t%s = %s;\n", firststring, arg->argTTName);
      fprintf(file, "#else\t/* UseStaticTemplates */\n");
      fprintf(file, "\t%sname = MACH_PORT_NULL;\n", string);
      fprintf(file, "\t%stype = MACH_MSG_PORT_DESCRIPTOR;\n", string);
      fprintf(file, "#endif\t/* UseStaticTemplates */\n");
      fprintf(file, "\t    }\n");
    }
    fprintf(file, "\t}\n");
  }
  fprintf(file, "\n");
}

static void
WriteKPD_ool_varsize(FILE *file, register argument_t *arg, char *who, char *where, boolean_t iscomplex)
{
  register ipc_type_t *it = arg->argType;
  register argument_t *count;
  register char *cref;
  
  if (iscomplex) {
    it = it->itElement;
    count = arg->argSubCount;
  }
  else
    count = arg->argCount;
  cref = count->argByReferenceUser ? "*" : "";
  
  /* size has to be expressed in bytes! */
  if (count->argMultiplier > 1 || it->itSize > 8)
    fprintf(file, "\t%s->%s = %s%s%s * %d;\n", who, where, cref, count->argVarName, (iscomplex)? "[i]" : "", count->argMultiplier * it->itSize / 8);
  else
    fprintf(file, "\t%s->%s = %s%s%s;\n", who, where, cref, count->argVarName, (iscomplex)? "[i]" : "");
}

/*
 * argKPD_Pack discipline for out-of-line types.
 */
static void
WriteKPD_ool(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  register char *ref = arg->argByReferenceUser ? "*" : "";
  char firststring[MAX_STR_LEN];
  char string[MAX_STR_LEN];
  boolean_t VarArray;
  u_int howmany, howbig;
  char *subindex;
  
  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, TRUE, FALSE, it->itVarArray, arg, TRUE);
    (void)sprintf(firststring, "\t*ptr");
    (void)sprintf(string, "\tptr->");
    VarArray = it->itElement->itVarArray;
    howmany = it->itElement->itNumber;
    howbig = it->itElement->itSize;
    subindex = "[i]";
  }
  else {
    (void)sprintf(firststring, "InP->%s", arg->argMsgField);
    (void)sprintf(string, "InP->%s.", arg->argMsgField);
    VarArray = it->itVarArray;
    howmany = it->itNumber;
    howbig = it->itSize;
    subindex = "";
  }
  
  fprintf(file, "#if\tUseStaticTemplates\n");
  
  fprintf(file, "\t%s = %s;\n", firststring, arg->argTTName);
  fprintf(file, "\t%saddress = (void *)(%s%s%s);\n", string, ref, arg->argVarName, subindex);
  if (VarArray) {
    if (IS_MULTIPLE_KPD(it))
      WriteKPD_ool_varsize(file, arg, "\tptr", "size", TRUE);
    else
      WriteKPD_ool_varsize(file, arg, "InP", strconcat(arg->argMsgField, ".size"), FALSE);
  }
  
  if (arg->argDeallocate == d_MAYBE)
    fprintf(file, "\t%sdeallocate =  %s;\n", string, arg->argDealloc->argVarName);
  
  fprintf(file, "#else\t/* UseStaticTemplates */\n");
  
  fprintf(file, "\t%saddress = (void *)(%s%s%s);\n", string, ref, arg->argVarName, subindex);
  if (VarArray)
    if (IS_MULTIPLE_KPD(it))
      WriteKPD_ool_varsize(file, arg, "\tptr", "size", TRUE);
    else
      WriteKPD_ool_varsize(file, arg, "InP", strconcat(arg->argMsgField, ".size"), FALSE);
    else
      fprintf(file, "\t%ssize = %d;\n", string, (howmany * howbig + 7)/8);
  if (arg->argDeallocate == d_MAYBE)
    fprintf(file, "\t%sdeallocate =  %s;\n", string, arg->argDealloc->argVarName);
  else
    fprintf(file, "\t%sdeallocate =  %s;\n", string, (arg->argDeallocate == d_YES) ? "TRUE" : "FALSE");
  fprintf(file, "\t%scopy = %s;\n", string, (arg->argFlags & flPhysicalCopy) ? "MACH_MSG_PHYSICAL_COPY" : "MACH_MSG_VIRTUAL_COPY");
#ifdef ALIGNMENT
  fprintf(file, "\t%salignment = MACH_MSG_ALIGN_%d;\n", string, (it->itElement->itSize < 8) ? 1 : it->itElement->itSize / 8);
#endif
  fprintf(file, "\t%stype = MACH_MSG_OOL_DESCRIPTOR;\n", string);
  
  fprintf(file, "#endif\t/* UseStaticTemplates */\n");
  if (IS_MULTIPLE_KPD(it)) {
    fprintf(file, "\t    }\n");
    if (it->itVarArray) {
      fprintf(file, "\t    for (i = %s; i < %d; ptr++, i++) {\n", arg->argCount->argVarName, it->itKPD_Number);
      /* fill the rest of the statically allocated KPD entries with size NULL */
      fprintf(file, "#if\tUseStaticTemplates\n");
      fprintf(file, "\t%s = %s;\n", firststring, arg->argTTName);
      if (!VarArray)
        fprintf(file, "\t%ssize = 0;\n", string);
      /* otherwise the size in the template would be != 0! */
      fprintf(file, "#else\t/* UseStaticTemplates */\n");
      fprintf(file, "\t%ssize = 0;\n", string);
      fprintf(file, "\t%stype = MACH_MSG_OOL_DESCRIPTOR;\n", string);
      fprintf(file, "#endif\t/* UseStaticTemplates */\n");
      fprintf(file, "\t    }\n");
    }
    fprintf(file, "\t}\n");
  }
  fprintf(file, "\n");
}

/*
 * argKPD_Pack discipline for out-of-line Port types.
 */
static void
WriteKPD_oolport(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  register char *ref = arg->argByReferenceUser ? "*" : "";
  register argument_t *count;
  boolean_t VarArray;
  string_t howstr;
  u_int howmany;
  char *subindex;
  char firststring[MAX_STR_LEN];
  char string[MAX_STR_LEN];
  
  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, TRUE, FALSE, it->itVarArray, arg, TRUE);
    (void)sprintf(firststring, "\t*ptr");
    (void)sprintf(string, "\tptr->");
    VarArray = it->itElement->itVarArray;
    howmany = it->itElement->itNumber;
    howstr = it->itElement->itInNameStr;
    count = arg->argSubCount;
    subindex = "[i]";
  }
  else {
    (void)sprintf(firststring, "InP->%s", arg->argMsgField);
    (void)sprintf(string, "InP->%s.", arg->argMsgField);
    VarArray = it->itVarArray;
    howmany = it->itNumber;
    howstr = it->itInNameStr;
    count = arg->argCount;
    subindex = "";
  }
  
  fprintf(file, "#if\tUseStaticTemplates\n");
  
  fprintf(file, "\t%s = %s;\n", firststring, arg->argTTName);
  fprintf(file, "\t%saddress = (void *)(%s%s%s);\n", string, ref, arg->argVarName, subindex);
  if (VarArray)
    fprintf(file, "\t%scount = %s%s%s;\n", string, count->argByReferenceUser ? "*" : "", count->argVarName, subindex);
  if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbSendSnd)) {
    register argument_t *poly = arg->argPoly;
    register char *pref = poly->argByReferenceUser ? "*" : "";
    
    fprintf(file, "\t%sdisposition = %s%s;\n", string, pref, poly->argVarName);
  }
  if (arg->argDeallocate == d_MAYBE)
    fprintf(file, "\t%sdeallocate =  %s;\n", string, arg->argDealloc->argVarName);
  
  fprintf(file, "#else\t/* UseStaticTemplates */\n");
  
  fprintf(file, "\t%saddress = (void *)(%s%s%s);\n", string, ref, arg->argVarName, subindex);
  if (VarArray)
    fprintf(file, "\t%scount = %s%s%s;\n", string, count->argByReferenceUser ? "*" : "", count->argVarName, subindex);
  else
    fprintf(file, "\t%scount = %d;\n", string, howmany);
  if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbSendSnd)) {
    register argument_t *poly = arg->argPoly;
    register char *pref = poly->argByReferenceUser ? "*" : "";
    
    fprintf(file, "\t%sdisposition = %s%s;\n", string, pref, poly->argVarName);
  }
  else
    fprintf(file, "\t%sdisposition = %s;\n", string, howstr);
  if (arg->argDeallocate == d_MAYBE)
    fprintf(file, "\t%sdeallocate =  %s;\n", string, arg->argDealloc->argVarName);
  else
    fprintf(file, "\t%sdeallocate =  %s;\n", string, (arg->argDeallocate == d_YES) ? "TRUE" : "FALSE");
  fprintf(file, "\t%stype = MACH_MSG_OOL_PORTS_DESCRIPTOR;\n", string);
  
  fprintf(file, "#endif\t/* UseStaticTemplates */\n");
  fprintf(file, "\n");
  
  if (IS_MULTIPLE_KPD(it)) {
    fprintf(file, "\t    }\n");
    if (it->itVarArray) {
      fprintf(file, "\t    for (i = %s; i < %d; ptr++, i++) {\n", arg->argCount->argVarName, it->itKPD_Number);
      /* fill the rest of the statically allocated KPD entries with size NULL */
      fprintf(file, "#if\tUseStaticTemplates\n");
      fprintf(file, "\t%s = %s;\n", firststring, arg->argTTName);
      if (!VarArray)
        fprintf(file, "\t%scount = 0;\n", string);
      /* otherwise the size in the template would be != 0! */
      fprintf(file, "#else\t/* UseStaticTemplates */\n");
      fprintf(file, "\t%scount = 0;\n", string);
      fprintf(file, "\t%stype = MACH_MSG_OOL_PORTS_DESCRIPTOR;\n", string);
      fprintf(file, "#endif\t/* UseStaticTemplates */\n");
      fprintf(file, "\t    }\n");
    }
    fprintf(file, "\t}\n");
  }
  fprintf(file, "\n");
}

static void
WriteOverwriteTemplate(FILE *file, routine_t *rt)
{
  register argument_t *arg;
  char string[MAX_STR_LEN];
  char *subindex = "";
  boolean_t finish = FALSE;
  
  fprintf(file, "\t/* Initialize the template for overwrite */\n");
  fprintf(file, "\tInOvTemplate->msgh_body.msgh_descriptor_count = %d;\n", rt->rtOverwriteKPDs);
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)  {
    register ipc_type_t *it = arg->argType;
    char *ref = arg->argByReferenceUser ? "*" : "";
    argument_t *count;
    char *cref;
    boolean_t VarIndex;
    u_int howmany, howbig;
    
    if (akCheck(arg->argKind, akbOverwrite)) {
      if (arg->argFlags & flOverwrite) {
        if (IS_MULTIPLE_KPD(it)) {
          WriteKPD_Iterator(file, FALSE, TRUE, it->itVarArray, arg, TRUE);
          if (it->itVarArray)
            finish = TRUE;
          sprintf(string, "\tptr->");
          subindex = "[i]";
          count = arg->argSubCount;
          VarIndex = it->itElement->itVarArray;
          howmany = it->itElement->itNumber;
          howbig = it->itElement->itSize;
        }
        else {
          sprintf(string, "InOvTemplate->%s.", arg->argMsgField);
          subindex = "";
          count = arg->argCount;
          VarIndex = it->itVarArray;
          howmany = it->itNumber;
          howbig = it->itSize;
        }
        
        fprintf(file, "\t%saddress = (void *) %s%s%s;\n", string, ref, arg->argVarName, subindex);
        
        if (it->itPortType) {
          fprintf(file, "\t%scount = ", string);
          if (VarIndex) {
            cref = count->argByReferenceUser ? "*" : "";
            fprintf(file, "%s%s%s;\n", cref, count->argVarName, subindex);
          }
          else
            fprintf(file, "%d;\n", howmany);
        }
        else {
          fprintf(file, "\t%ssize = ", string);
          if (VarIndex) {
            cref = count->argByReferenceUser ? "*" : "";
            if (count->argMultiplier > 1 || howbig > 8)
              fprintf(file, "%s%s%s * %d;\n", cref, count->argVarName, subindex, count->argMultiplier * howbig / 8);
            else
              fprintf(file, "%s%s%s;\n", cref, count->argVarName, subindex);
          }
          else
            fprintf(file, "\t%ssize = %d;\n", string, (howmany * howbig + 7)/8);
        }
        fprintf(file, "\t%scopy = MACH_MSG_OVERWRITE;\n", string);
        fprintf(file, "\t%stype = MACH_MSG_OOL_%sDESCRIPTOR;\n", string, (it->itPortType) ? "PORTS_" : "");
        if (IS_MULTIPLE_KPD(it))
          fprintf(file, "\t    }\n");
        if (finish) {
          fprintf(file, "\t    for (i = %s%s; i < %d; ptr++, i++) {\n", (arg->argCount->argByReferenceUser) ? "*" : "", arg->argCount->argVarName, it->itKPD_Number);
          fprintf(file, "\t\tptr->copy = MACH_MSG_ALLOCATE;\n");
          fprintf(file, "\t\tptr->type = MACH_MSG_OOL_%sDESCRIPTOR;\n", (it->itPortType) ? "PORTS_" : "");
          fprintf(file, "\t    }\n");
        }
        if (IS_MULTIPLE_KPD(it))
          fprintf(file, "\t}\n");
      }
      else {
        /* just a placeholder */
        if (IS_MULTIPLE_KPD(it)) {
          WriteKPD_Iterator(file, FALSE, TRUE, FALSE, arg, TRUE);
          fprintf(file, "\t\tptr->copy = MACH_MSG_ALLOCATE;\n");
          fprintf(file, "\t\tptr->type = MACH_MSG_OOL_%sDESCRIPTOR;\n", (it->itPortType) ? "PORTS_" : "");
          fprintf(file, "\t    }\n\t}\n");
        }
        else {
          fprintf(file, "\tInOvTemplate->%s.copy = MACH_MSG_ALLOCATE;\n", arg->argMsgField);
          /* not sure whether this is needed */
          fprintf(file, "\tInOvTemplate->%s.type = MACH_MSG_OOL_%sDESCRIPTOR;\n", arg->argMsgField, (it->itPortType) ? "PORTS_" : "");
        }
      }
    }
  }
  fprintf(file, "\n");
}

/*************************************************************
 *  Writes code to copy an argument into the request message.
 *  Called by WriteRoutine for each argument that is to placed
 *  in the request message.
 *************************************************************/

static void
WritePackArgValueNormal(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  register char *ref = (arg->argByReferenceUser ||
                        it->itNativePointer) ? "*" : "";
  
  if (IS_VARIABLE_SIZED_UNTYPED(it) || it->itNoOptArray) {
    if (it->itString) {
      /*
       * Copy variable-size C string with mig_strncpy.
       * Save the string length (+ 1 for trailing 0)
       * in the argument`s count field.
       */
      fprintf(file, "\tInP->%s = mig_strncpy(InP->%s, %s, %d);\n", arg->argCount->argMsgField, arg->argMsgField, arg->argVarName, it->itNumber);
    }
    else if (it->itNoOptArray)
      fprintf(file, "\t(void)memcpy((char *) InP->%s, (const char *) %s%s, %d);\n", arg->argMsgField, ref, arg->argVarName, it->itTypeSize);
    else {
      
      /*
       * Copy in variable-size inline array with (void)memcpy,
       * after checking that number of elements doesn`t
       * exceed declared maximum.
       */
      register argument_t *count = arg->argCount;
      register char *countRef = count->argByReferenceUser ? "*" : "";
      register ipc_type_t *btype = it->itElement;
      
      /* Note btype->itNumber == count->argMultiplier */
      
      if (akIdent(arg->argKind) != akeSubCount) {
        /* we skip the SubCount case, as we have already taken care of */
        fprintf(file, "\tif (%s%s > %d) {\n", countRef, count->argVarName, it->itNumber/btype->itNumber);
        WriteReturnMsgError(file, arg->argRoutine, TRUE, arg, "MIG_ARRAY_TOO_LARGE");
        fprintf(file, "\t}\n");
      }
      
      fprintf(file, "\t(void)memcpy((char *) InP->%s, (const char *) %s%s, ", arg->argMsgField, ref, arg->argVarName);
      if (btype->itTypeSize > 1)
        fprintf(file, "%d * ", btype->itTypeSize);
      fprintf(file, "%s%s);\n", countRef, count->argVarName);
    }
  }
  else if (IS_OPTIONAL_NATIVE(it)) {
    fprintf(file, "\tif ((InP->__Present__%s = (%s != %s))) {\n", arg->argMsgField, arg->argVarName, it->itBadValue);
    WriteCopyType(file, it, "\tInP->%s.__Real__%s", "/* %s%s */ %s%s", arg->argMsgField, arg->argMsgField, ref, arg->argVarName);
    fprintf(file, "\t}\n");
  }
  else
    WriteCopyType(file, it, "InP->%s", "/* %s */ %s%s", arg->argMsgField, ref, arg->argVarName);
  fprintf(file, "\n");
}

/*
 * Calculate the size of a variable-length message field.
 */
static void
WriteArgSizeVariable(FILE *file,  register argument_t *arg, ipc_type_t *ptype)
{
  register int bsize = ptype->itElement->itTypeSize;
  register argument_t *count = arg->argCount;
  
  if (PackMsg == FALSE) {
    fprintf(file, "%d", ptype->itTypeSize + ptype->itPadSize);
    return;
  }
  
  /* If the base type size of the data field isn`t a multiple of 4,
   we have to round up. */
  if (bsize % itWordAlign != 0)
    fprintf(file, "_WALIGN_");
  fprintf(file, "(");
  if (bsize > 1)
    fprintf(file, "%d * ", bsize);
  if (ptype->itString)
  /* get count from descriptor in message */
    fprintf(file, "InP->%s", count->argMsgField);
  else
  /* get count from argument */
    fprintf(file, "%s%s", count->argByReferenceUser ? "*" : "", count->argVarName);
  fprintf(file, ")");
}

static void
WriteArgSizeOptional(FILE *file, argument_t *arg, ipc_type_t *ptype)
{
  fprintf(file, "(InP->__Present__%s ? _WALIGNSZ_(%s) : 0)", arg->argVarName, ptype->itUserType);
}

static void
WriteArgSize(FILE *file, argument_t *arg)

{
  ipc_type_t *ptype = arg->argType;
  
  if (IS_OPTIONAL_NATIVE(ptype))
    WriteArgSizeOptional(file, arg, ptype);
  else
    WriteArgSizeVariable(file, arg, ptype);
}

/*
 * Adjust message size and advance request pointer.
 * Called after packing a variable-length argument that
 * has more arguments following.
 */
static void
WriteAdjustMsgSize(FILE *file, register argument_t *arg)
{
  register ipc_type_t *ptype = arg->argType;
  
  /* There are more In arguments.  We need to adjust msgh_size
     and advance InP, so we save the size of the current field
     in msgh_size_delta. */
  
  fprintf(file, "\tmsgh_size_delta = ");
  WriteArgSize(file, arg);
  fprintf(file, ";\n");
  
  if (arg->argRequestPos == 0) {
    /* First variable-length argument.  The previous msgh_size value
       is the minimum request size. */
    
    fprintf(file, "\tmsgh_size = ");
    rtMinRequestSize(file, arg->argRoutine, "Request");
    fprintf(file, " + msgh_size_delta;\n");
  }
  else
    fprintf(file, "\tmsgh_size += msgh_size_delta;\n");
  
  if (PackMsg == TRUE) {
    fprintf(file, "\tInP = (Request *) ((pointer_t) InP + msgh_size_delta - ");
    if (IS_OPTIONAL_NATIVE(ptype))
      fprintf(file, "_WALIGNSZ_(%s)", ptype->itUserType);
    else
      fprintf(file, "%d", ptype->itTypeSize + ptype->itPadSize);
    fprintf(file, ");\n\n");
  }
}

/*
 * Calculate the size of the message.  Called after the
 * last argument has been packed.
 */
static void
WriteFinishMsgSize(FILE *file, register argument_t *arg)
{
  /* No more In arguments.  If this is the only variable In
     argument, the previous msgh_size value is the minimum
     request size. */
  
  if (arg->argRequestPos == 0) {
    fprintf(file, "\tmsgh_size = ");
    rtMinRequestSize(file, arg->argRoutine, "Request");
    fprintf(file, " + (");
    WriteArgSize(file, arg);
    fprintf(file, ");\n");
  }
  else {
    fprintf(file, "\tmsgh_size += ");
    WriteArgSize(file, arg);
    fprintf(file, ";\n");
  }
}

static void
WriteInitializeCount(FILE *file, register argument_t *arg)
{
  register ipc_type_t *ptype = arg->argCInOut->argParent->argType;
  register ipc_type_t *btype = ptype->itElement;
  
  fprintf(file, "\tif (%s%s < %d)\n", arg->argByReferenceUser ? "*" : "", arg->argVarName, ptype->itNumber/btype->itNumber);
  fprintf(file, "\t\tInP->%s = %s%s;\n", arg->argMsgField, arg->argByReferenceUser ? "*" : "", arg->argVarName);
  fprintf(file, "\telse\n");
  fprintf(file, "\t\tInP->%s = %d;\n", arg->argMsgField, ptype->itNumber/btype->itNumber);
  fprintf(file, "\n");
}

/*
 * Generate code to fill in all of the request arguments and their
 * message types.
 */
static void
WriteRequestArgs(FILE *file, register routine_t *rt)
{
  register argument_t *arg;
  register argument_t *lastVarArg;
  
  /*
   * 1. The Kernel Processed Data
   */
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)
    if (akCheckAll(arg->argKind, akbSendSnd|akbSendKPD))
      (*arg->argKPD_Pack)(file, arg);
  
  /*
   * 2. The Data Stream
   */
  lastVarArg = argNULL;
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)  {
    /*
     * Adjust message size and advance message pointer if
     * the last request argument was variable-length and the
     * request position will change.
     */
    if (lastVarArg != argNULL &&
        lastVarArg->argRequestPos < arg->argRequestPos) {
      WriteAdjustMsgSize(file, lastVarArg);
      lastVarArg = argNULL;
    }
    
    if ((akIdent(arg->argKind) == akeCountInOut) &&
        akCheck(arg->argKind, akbSendSnd))
      WriteInitializeCount(file, arg);
    else if (akCheckAll(arg->argKind, akbSendSnd|akbSendBody))
      WritePackArgValueNormal(file, arg);
    /*
     * Remember whether this was variable-length.
     */
    if (akCheckAll(arg->argKind, akbSendSnd|akbSendBody|akbVariable))
      lastVarArg = arg;
  }
  /*
   * Finish the message size.
   */
  if (lastVarArg != argNULL)
    WriteFinishMsgSize(file, lastVarArg);
}

/*************************************************************
 *  Writes code to check that the return msgh_id is correct and that
 *  the size of the return message is correct. Called by
 *  WriteRoutine.
 *************************************************************/
static void
WriteCheckIdentity(FILE *file, routine_t *rt)
{
  fprintf(file, "\tif (Out0P->Head.msgh_id != %d) {\n", rt->rtNumber + SubsystemBase + 100);
  fprintf(file, "\t    if (Out0P->Head.msgh_id == MACH_NOTIFY_SEND_ONCE)\n");
  fprintf(file, "\t\t{ return MIG_SERVER_DIED; }\n");
  fprintf(file, "\t    else\n");
  fprintf(file, "\t\t{ return MIG_REPLY_MISMATCH; }\n");
  fprintf(file, "\t}\n");
  fprintf(file, "\n");
  if (!rt->rtSimpleReply)
    fprintf(file, "\tmsgh_simple = !(Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX);\n");
  fprintf(file, "#if\t__MigTypeCheck\n");
  
  if (!rt->rtNoReplyArgs)
    fprintf(file, "\tmsgh_size = Out0P->Head.msgh_size;\n\n");
  
  if (rt->rtSimpleReply) {
    /* Expecting a simple message.  We can factor out the check for
     * a simple message, since the error reply message is also simple.
     */
    fprintf(file, "\tif ((Out0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||\n");
    if (rt->rtNoReplyArgs)
      fprintf(file, "\t    (Out0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Reply)))\n");
    else {
      /*
       * We have an error iff:
       * 1) the message size is not the one expected AND
       * 2) the message size is also different from sizeof(mig_reply_error_t)
       *    or the RetCode == KERN_SUCCESS
       */
      if (rt->rtNumReplyVar > 0)  {
        fprintf(file, "\t    ((msgh_size > (mach_msg_size_t)sizeof(__Reply) || msgh_size < ");
        rtMinReplySize(file, rt, "__Reply");
        fprintf(file, ") &&\n");
      }
      else
        fprintf(file, "\t    ((msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&\n");
      fprintf(file, "\t     (msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||\n");
      fprintf(file, "\t      Out0P->RetCode == KERN_SUCCESS)))\n");
    }
  }
  else {
    /* Expecting a complex message. */
    
    fprintf(file, "\t" "if ((msgh_simple || Out0P->msgh_body.msgh_descriptor_count != %d ||\n", rt->rtReplyKPDs);
    if (rt->rtNumReplyVar > 0) {
      fprintf(file, "\t    msgh_size < ");
      rtMinReplySize(file, rt, "__Reply");
      fprintf(file, " || msgh_size > (mach_msg_size_t)sizeof(__Reply)) &&\n");
    }
    else
      fprintf(file, "\t    msgh_size != (mach_msg_size_t)sizeof(__Reply)) &&\n");
    fprintf(file, "\t    (!msgh_simple || msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||\n");
    fprintf(file, "\t    ((mig_reply_error_t *)Out0P)->RetCode == KERN_SUCCESS))\n");
  }
  fprintf(file, "\t\t{ return MIG_TYPE_ERROR ; }\n");
  fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  fprintf(file, "\n");
}

/*************************************************************
 *  Write code to generate error handling code if the RetCode
 *  argument of a Routine is not KERN_SUCCESS.
 *************************************************************/
static void
WriteRetCodeCheck(FILE *file, routine_t *rt)
{
  if (rt->rtSimpleReply)
    fprintf(file, "\tif (Out0P->RetCode != KERN_SUCCESS) {\n");
  else
    fprintf(file, "\tif (msgh_simple) {\n");
  if (CheckNDR) {
    fprintf(file, "#ifdef\t__NDR_convert__mig_reply_error_t__defined\n");
    fprintf(file, "\t\t__NDR_convert__mig_reply_error_t((mig_reply_error_t *)Out0P);\n");
    fprintf(file, "#endif\t/* __NDR_convert__mig_reply_error_t__defined */\n");
  }
  fprintf(file, "\t\treturn ((mig_reply_error_t *)Out0P)->RetCode;\n");
  fprintf(file, "\t}\n");
  fprintf(file, "\n");
}

/*
 * argKPD_TypeCheck discipline for Port types.
 */
static void
WriteTCheckKPD_port(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char *tab = "";
  char string[MAX_STR_LEN];
  boolean_t close = FALSE;
  
  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, FALSE, FALSE, arg, TRUE);
    (void)sprintf(string, "ptr->");
    tab = "\t";
    close = TRUE;
  }
  else
    (void)sprintf(string, "Out%dP->%s.", arg->argReplyPos, arg->argMsgField);
  fprintf(file, "\t%sif (%stype != MACH_MSG_PORT_DESCRIPTOR", tab, string);
  if (arg->argPoly == argNULL && !it->itVarArray)
  /* we can't check disposition when poly or VarArray,
     (because some of the entries could be empty) */
    fprintf(file, " ||\n\t%s    %sdisposition != %s", tab, string, it->itOutNameStr);
  fprintf(file,
          ") {\n"
          "\t\t%s" "return MIG_TYPE_ERROR;\n"
          "\t%s" "}\n"
        , tab, tab);
  if (close)
    fprintf(file, "\t    }\n\t}\n");
}

/*
 * argKPD_TypeCheck discipline for out-of-line types.
 */
static void
WriteTCheckKPD_ool(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char *tab, string[MAX_STR_LEN];
  boolean_t test;
  u_int howmany, howbig;
  
  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, FALSE, FALSE, arg, TRUE);
    tab = "\t";
    sprintf(string, "ptr->");
    howmany = it->itElement->itNumber;
    howbig = it->itElement->itSize;
    test = !it->itVarArray && !it->itElement->itVarArray;
  }
  else {
    tab = "";
    sprintf(string, "Out%dP->%s.", arg->argReplyPos, arg->argMsgField);
    howmany = it->itNumber;
    howbig = it->itSize;
    test = !it->itVarArray;
  }
  
  fprintf(file, "\t%sif (%stype != MACH_MSG_OOL_DESCRIPTOR", tab, string);
  if (test)
  /* if VarArray we may use no-op; if itElement->itVarArray size might change */
    fprintf(file, " ||\n\t%s    %ssize != %d", tab, string, (howmany * howbig + 7)/8);
  fprintf(file,
          ") {\n"
          "\t\t%s" "return MIG_TYPE_ERROR;\n"
          "\t%s" "}\n"
          , tab, tab);
  if (IS_MULTIPLE_KPD(it))
    fprintf(file, "\t    }\n\t}\n");
}

/*
 * argKPD_TypeCheck discipline for out-of-line Port types.
 */
static void
WriteTCheckKPD_oolport(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char *tab, string[MAX_STR_LEN];
  boolean_t test;
  u_int howmany;
  char *howstr;
  
  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, FALSE, FALSE, arg, TRUE);
    tab = "\t";
    sprintf(string, "ptr->");
    howmany = it->itElement->itNumber;
    test = !it->itVarArray && !it->itElement->itVarArray;
    howstr = it->itElement->itOutNameStr;
  }
  else {
    tab = "";
    sprintf(string, "Out%dP->%s.", arg->argReplyPos, arg->argMsgField);
    howmany = it->itNumber;
    test = !it->itVarArray;
    howstr = it->itOutNameStr;
  }
  
  fprintf(file, "\t%sif (%stype != MACH_MSG_OOL_PORTS_DESCRIPTOR", tab, string);
  if (test)
    /* if VarArray we may use no-op; if itElement->itVarArray size might change */
    fprintf(file, " ||\n\t%s    %scount != %d", tab, string, howmany);
  if (arg->argPoly == argNULL)
    fprintf(file, " ||\n\t%s    %sdisposition != %s", tab, string, howstr);
  fprintf(file, ") {\n"
          "\t\t%s" "return MIG_TYPE_ERROR;\n"
          "\t%s" "}\n"
          ,tab, tab);
  if (IS_MULTIPLE_KPD(it))
    fprintf(file, "\t    }\n\t}\n");
}

/*************************************************************
 *  Writes code to check that the type of each of the arguments
 *  in the reply message is what is expected. Called by
 *  WriteRoutine for each out && typed argument in the reply message.
 *************************************************************/
static void
WriteTypeCheck(FILE *file, register argument_t *arg)
{
  fprintf(file, "#if\t__MigTypeCheck\n");
  (*arg->argKPD_TypeCheck)(file, arg);
  fprintf(file, "#endif\t/* __MigTypeCheck */\n");
}


/*
 * argKPD_Extract discipline for Port types.
 */
static void
WriteExtractKPD_port(FILE *file,  register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  register char *ref = arg->argByReferenceUser ? "*" : "";
  register char *subindex;
  register char *recast = "";
  ipc_type_t *real_it;
  
  real_it = (IS_MULTIPLE_KPD(it)) ? it->itElement : it;
#ifdef MIG_KERNEL_PORT_CONVERSION
  if (IsKernelUser && streql(real_it->itUserType, "ipc_port_t"))
    recast = "(mach_port_t)";
#endif
  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, FALSE, it->itVarArray, arg, FALSE);
    
    fprintf(file, "\t\t%s[i] = %sptr->name;\n", arg->argVarName, recast);
    if (it->itVarArray) {
      register argument_t *count = arg->argCount;
      register char *cref = count->argByReferenceUser ? "*" : "";
      
      fprintf(file, "\t    if (Out%dP->%s >",count->argReplyPos, count->argVarName);
      if (arg->argCountInOut) {
        fprintf(file, " %s%s)\n", cref, count->argVarName);
      }
      else {
        fprintf(file, " %d)\n", it->itNumber/it->itElement->itNumber);
      }
      WriteReturnMsgError(file, arg->argRoutine, TRUE, arg, "MIG_ARRAY_TOO_LARGE");
    }
    fprintf(file, "\t}\n");
    subindex = "[0]";
  }
  else {
    fprintf(file, "\t%s%s = %sOut%dP->%s.name;\n", ref, arg->argVarName, recast, arg->argReplyPos, arg->argMsgField);
    subindex = "";
  }
  
  if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbReturnRcv)) {
    register argument_t *poly = arg->argPoly;
    register char *pref = poly->argByReferenceUser ? "*" : "";
    
    fprintf(file, "\t%s%s = Out%dP->%s%s.disposition;\n", pref, poly->argVarName, arg->argReplyPos, arg->argMsgField, subindex);
  }
}

/*
 * argKPD_Extract discipline for out-of-line types.
 */
static void
WriteExtractKPD_ool(FILE *file, register argument_t *arg)
{
  register char *ref = arg->argByReferenceUser ? "*" : "";
  register ipc_type_t *it = arg->argType;
  
  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, FALSE, it->itVarArray, arg, FALSE);
    fprintf(file, "\t\t%s[i] = ptr->address;\n", arg->argVarName);
    fprintf(file, "\t}\n");
  }
  else
    fprintf(file, "\t%s%s = (%s)(Out%dP->%s.address);\n", ref, arg->argVarName, arg->argType->itUserType, arg->argReplyPos, arg->argMsgField);
  /*
   *  In case of variable sized arrays,
   *  the count field will be retrieved from the untyped
   *  section of the message
   */
}

/*
 * argKPD_Extract discipline for out-of-line Port types.
 */
static void
WriteExtractKPD_oolport(FILE *file, register argument_t *arg)
{
  register char *ref = arg->argByReferenceUser ? "*" : "";
  register ipc_type_t *it = arg->argType;
  char *subindex;
  
  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, FALSE, it->itVarArray, arg, FALSE);
    fprintf(file, "\t\t%s[i] = ptr->address;\n", arg->argVarName);
    fprintf(file, "\t}\n");
    subindex = "[0]";
  }
  else {
    fprintf(file, "\t%s%s = (%s)(Out%dP->%s.address);\n", ref, arg->argVarName, arg->argType->itUserType, arg->argReplyPos, arg->argMsgField);
    subindex = "";
  }
  /*
   *  In case of variable sized arrays,
   *  the count field will be retrieved from the untyped
   *  section of the message
   */
  if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbReturnRcv)) {
    register argument_t *poly = arg->argPoly;
    register char *pref = poly->argByReferenceUser ? "*" : "";
    
    fprintf(file, "\t%s%s = Out%dP->%s%s.disposition;\n", pref, poly->argVarName, arg->argReplyPos, arg->argMsgField, subindex);
  }
}

/*************************************************************
 *  Write code to copy an argument from the reply message
 *  to the parameter. Called by WriteRoutine for each argument
 *  in the reply message.
 *************************************************************/

static void
WriteExtractArgValueNormal(FILE *file, register argument_t *arg)
{
  register ipc_type_t *argType = arg->argType;
  register char *ref = arg->argByReferenceUser ? "*" : "";
  char who[20];
  
  if (akCheck(arg->argKind, akbUserImplicit))
    sprintf(who, "TrailerP");
  else
    sprintf(who, "Out%dP", arg->argReplyPos);
  
  if (IS_VARIABLE_SIZED_UNTYPED(argType) || argType->itNoOptArray) {
    if (argType->itString) {
      /*
       * Copy out variable-size C string with mig_strncpy.
       */
      fprintf(file, "\t(void) mig_strncpy(%s%s, %s->%s, %d);\n", ref, arg->argVarName, who, arg->argMsgField, argType->itNumber);
    }
    else if (argType->itNoOptArray)
      fprintf(file, "\t(void)memcpy((char *) %s%s, (const char *) %s->%s, %d);\n", ref, arg->argVarName, who, arg->argMsgField, argType->itTypeSize);
    else {
      
      /*
       * Copy out variable-size inline array with (void)memcpy,
       * after checking that number of elements doesn`t
       * exceed user`s maximum.
       */
      register argument_t *count = arg->argCount;
      register char *countRef = count->argByReferenceUser ? "*" : "";
      register ipc_type_t *btype = argType->itElement;
      
      /* Note count->argMultiplier == btype->itNumber */
      /* Note II: trailer logic isn't supported in this case */
      fprintf(file, "\tif (Out%dP->%s", count->argReplyPos, count->argMsgField);
      if (arg->argCountInOut) {
        fprintf(file, " > %s%s) {\n", countRef, count->argVarName);
      }
      else {
        fprintf(file, " > %d) {\n", argType->itNumber/btype->itNumber);
      }
      
      /*
       * If number of elements is too many for user receiving area,
       * fill user`s area as much as possible.  Return the correct
       * number of elements.
       */
      fprintf(file, "\t\t(void)memcpy((char *) %s%s, (const char *) Out%dP->%s, ", ref, arg->argVarName, arg->argReplyPos, arg->argMsgField);
      if (btype->itTypeSize > 1)
        fprintf(file, "%d * ", btype->itTypeSize);
      if (arg->argCountInOut) {
        fprintf(file, " %s%s);\n", countRef, count->argVarName);
      }
      else {
        fprintf(file, " %d);\n", argType->itNumber/btype->itNumber);
      }
      fprintf(file, "\t\t%s%s = Out%dP->%s", countRef, count->argVarName, count->argReplyPos, count->argMsgField);
      fprintf(file, ";\n");
      WriteReturnMsgError(file, arg->argRoutine, TRUE, arg, "MIG_ARRAY_TOO_LARGE");
      
      fprintf(file, "\t}\n");
      
      fprintf(file, "\t(void)memcpy((char *) %s%s, (const char *) Out%dP->%s, ", ref, arg->argVarName, arg->argReplyPos, arg->argMsgField);
      if (btype->itTypeSize > 1)
        fprintf(file, "%d * ", btype->itTypeSize);
      fprintf(file, "Out%dP->%s);\n", count->argReplyPos, count->argMsgField);
    }
  }
  else
    WriteCopyType(file, argType, "%s%s", "/* %s%s */ %s->%s", ref, arg->argVarName, who, arg->argMsgField);
  fprintf(file, "\n");
}

static void
WriteCalcArgSize(FILE *file, register argument_t *arg)
{
  register ipc_type_t *ptype = arg->argType;
  register ipc_type_t *btype = ptype->itElement;
  argument_t *count = arg->argCount;
  int multiplier = btype->itTypeSize;
  
  /* If the base type size of the data field isn`t a multiple of 4,
     we have to round up. */
  if (btype->itTypeSize % itWordAlign != 0)
    fprintf(file, "_WALIGN_(");
  
  fprintf(file, "Out%dP->%s", count->argReplyPos, count->argMsgField);
  if (multiplier > 1)
    fprintf(file, " * %d", multiplier);

  if (btype->itTypeSize % itWordAlign != 0)
    fprintf(file, ")");
}

static void
WriteCheckArgSize(FILE *file, routine_t *rt, argument_t *arg, const char *comparator)
{
  register ipc_type_t *ptype = arg->argType;
  register ipc_type_t *btype = ptype->itElement;
  argument_t *count = arg->argCount;
  int multiplier = btype->itTypeSize;
  
  fprintf(file, "\tif (((msgh_size - ");
  rtMinReplySize(file, rt, "__Reply");
  fprintf(file, ")");
  if (multiplier > 1)
	  fprintf(file, " / %d", multiplier);
  fprintf(file, "< Out%dP->%s) ||\n", count->argReplyPos, count->argMsgField);
  fprintf(file, "\t    (msgh_size %s ", comparator);
  rtMinReplySize(file, rt, "__Reply");
  fprintf(file, " + ");
  WriteCalcArgSize(file, arg);
  fprintf(file, ")");
  fprintf(file, ")\n\t\t{ return MIG_TYPE_ERROR ; }\n");
}


/* NDR Conversion routines */


void
WriteReplyNDRConvertIntRepArgCond(FILE *file, argument_t *arg)
{
  routine_t *rt = arg->argRoutine;
  
  fprintf(file, "defined(__NDR_convert__int_rep__Reply__%s_t__%s__defined)", rt->rtName, arg->argMsgField);
}

void
WriteReplyNDRConvertCharRepArgCond(FILE *file, argument_t *arg)
{
  routine_t *rt = arg->argRoutine;
  
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) !=akeCountInOut && akIdent(arg->argKind) != akeRetCode)
    fprintf(file, "defined(__NDR_convert__char_rep__Reply__%s_t__%s__defined)", rt->rtName, arg->argMsgField);
  else
    fprintf(file, "0");
}

void
WriteReplyNDRConvertFloatRepArgCond(FILE *file, argument_t *arg)
{
  routine_t *rt = arg->argRoutine;
  
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) !=akeCountInOut && akIdent(arg->argKind) != akeRetCode)
    fprintf(file, "defined(__NDR_convert__float_rep__Reply__%s_t__%s__defined)", rt->rtName, arg->argMsgField);
  else
    fprintf(file, "0");
}

void
WriteReplyNDRConvertIntRepArgDecl(FILE *file, argument_t *arg)
{
  WriteNDRConvertArgDecl(file, arg, "int_rep", "Reply");
}

void
WriteReplyNDRConvertCharRepArgDecl(FILE *file, argument_t *arg)
{
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) !=akeCountInOut && akIdent(arg->argKind) != akeRetCode)
    WriteNDRConvertArgDecl(file, arg, "char_rep", "Reply");
}

void
WriteReplyNDRConvertFloatRepArgDecl(FILE *file, argument_t *arg)
{
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) !=akeCountInOut && akIdent(arg->argKind) != akeRetCode)
    WriteNDRConvertArgDecl(file, arg, "float_rep", "Reply");
}



void
WriteReplyNDRConvertArgUse(FILE *file, argument_t *arg, char *convert)
{
  routine_t *rt = arg->argRoutine;
  argument_t *count = arg->argCount;
  char argname[MAX_STR_LEN];
  
  if ((akIdent(arg->argKind) == akeCount || akIdent(arg->argKind) == akeCountInOut) &&
      (arg->argParent && akCheck(arg->argParent->argKind, akbReturnNdr)))
    return;
  
  if (arg->argKPD_Type == MACH_MSG_OOL_DESCRIPTOR) {
    if (count && !arg->argSameCount && !strcmp(convert, "int_rep")) {
      fprintf(file, "#if defined(__NDR_convert__int_rep__Reply__%s_t__%s__defined)\n", rt->rtName, count->argMsgField);
      fprintf(file, "\t\t__NDR_convert__int_rep__Reply__%s_t__%s(&Out%dP->%s, Out%dP->NDR.int_rep);\n", rt->rtName, count->argMsgField, count->argReplyPos, count->argMsgField, count->argReplyPos);
      fprintf(file, "#endif\t/* __NDR_convert__int_rep__Reply__%s_t__%s__defined */\n", rt->rtName, count->argMsgField);
    }
    
    sprintf(argname, "(%s)(Out%dP->%s.address)", FetchServerType(arg->argType), arg->argReplyPos, arg->argMsgField);
  }
  else {
    sprintf(argname, "&Out%dP->%s", arg->argReplyPos, arg->argMsgField);
  }
  
  fprintf(file, "#if defined(__NDR_convert__%s__Reply__%s_t__%s__defined)\n", convert, rt->rtName, arg->argMsgField);
  fprintf(file, "\t\t__NDR_convert__%s__Reply__%s_t__%s(%s, Out0P->NDR.%s", convert, rt->rtName, arg->argMsgField, argname, convert);
  if (count)
    fprintf(file, ", Out%dP->%s", count->argReplyPos, count->argMsgField);
  fprintf(file, ");\n");
  fprintf(file, "#endif /* __NDR_convert__%s__Reply__%s_t__%s__defined */\n", convert, rt->rtName, arg->argMsgField);
}

void
WriteReplyNDRConvertIntRepOneArgUse(FILE *file, argument_t *arg)
{
  routine_t *rt = arg->argRoutine;
  
  fprintf(file, "#if defined(__NDR_convert__int_rep__Reply__%s_t__%s__defined)\n", rt->rtName, arg->argMsgField);
  fprintf(file, "\tif (Out0P->NDR.int_rep != NDR_record.int_rep)\n");
  fprintf(file, "\t\t__NDR_convert__int_rep__Reply__%s_t__%s(&Out%dP->%s, Out%dP->NDR.int_rep);\n", rt->rtName, arg->argMsgField, arg->argReplyPos, arg->argMsgField, arg->argReplyPos);
  fprintf(file, "#endif\t/* __NDR_convert__int_rep__Reply__%s_t__%s__defined */\n", rt->rtName, arg->argMsgField);
}

void
WriteReplyNDRConvertIntRepArgUse(FILE *file, argument_t *arg)
{
  WriteReplyNDRConvertArgUse(file, arg, "int_rep");
}

void
WriteReplyNDRConvertCharRepArgUse(FILE *file, argument_t *arg)
{
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) !=akeCountInOut && akIdent(arg->argKind) != akeRetCode)
    WriteReplyNDRConvertArgUse(file, arg, "char_rep");
}

void
WriteReplyNDRConvertFloatRepArgUse(FILE *file, argument_t *arg)
{
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) !=akeCountInOut && akIdent(arg->argKind) != akeRetCode)
    WriteReplyNDRConvertArgUse(file, arg, "float_rep");
}

static void
WriteCheckMsgSize(FILE *file, register argument_t *arg)
{
  register routine_t *rt = arg->argRoutine;
  
  /* If there aren't any more Out args after this, then
     we can use the msgh_size_delta value directly in
     the TypeCheck conditional. */
  
  if (CheckNDR && arg->argCount && !arg->argSameCount)
    WriteReplyNDRConvertIntRepOneArgUse(file, arg->argCount);
  
  if (arg->argReplyPos == rt->rtMaxReplyPos) {
    fprintf(file, "#if\t__MigTypeCheck\n");

    /*
     * emit code to verify that the server-code-provided count does not exceed the maximum count allowed by the type.
     */
    fprintf(file, "\t" "if ( Out%dP->%s > %d )\n", arg->argCount->argReplyPos, arg->argCount->argMsgField, arg->argType->itNumber);
    fputs("\t\t" "return MIG_TYPE_ERROR;\n", file);
    /* ...end... */
    
    WriteCheckArgSize(file, rt, arg, "!=");

    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  }
  else {
    /* If there aren't any more variable-sized arguments after this,
       then we must check for exact msg-size and we don't need
       to update msgh_size. */
    
    boolean_t LastVarArg = arg->argReplyPos+1 == rt->rtNumReplyVar;
    
    /* calculate the actual size in bytes of the data field.  note
       that this quantity must be a multiple of four.  hence, if
       the base type size isn't a multiple of four, we have to
       round up.  note also that btype->itNumber must
       divide btype->itTypeSize (see itCalculateSizeInfo). */
    
    fprintf(file, "\tmsgh_size_delta = ");
    WriteCalcArgSize(file, arg);
    fprintf(file, ";\n");
    fprintf(file, "#if\t__MigTypeCheck\n");

    /*
     * emit code to verify that the server-code-provided count does not exceed the maximum count allowed by the type.
     */
    fprintf(file, "\t" "if ( Out%dP->%s > %d )\n", arg->argCount->argReplyPos, arg->argCount->argMsgField, arg->argType->itNumber);
    fputs("\t\t" "return MIG_TYPE_ERROR;\n", file);
    /* ...end... */
    
    WriteCheckArgSize(file, rt, arg, LastVarArg ? "!=" : "<");

    if (!LastVarArg)
      fprintf(file, "\tmsgh_size -= msgh_size_delta;\n");
    
    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  }
  fprintf(file, "\n");
}

void
WriteAdjustReplyMsgPtr(FILE *file, register argument_t *arg)
{
  register ipc_type_t *ptype = arg->argType;
  
  fprintf(file, "\t*Out%dPP = Out%dP = (__Reply *) ((pointer_t) Out%dP + msgh_size_delta - %d);\n\n",
          arg->argReplyPos+1, arg->argReplyPos +1, arg->argReplyPos, ptype->itTypeSize + ptype->itPadSize);
}

static void
WriteReplyArgs(FILE *file, register routine_t *rt)
{
  register argument_t *arg;
  
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    if (akCheckAll(arg->argKind, akbReturnRcv|akbReturnBody)) {
      WriteExtractArgValueNormal(file, arg);
    }
    else if (akCheckAll(arg->argKind, akbReturnRcv|akbReturnKPD)) {
      /*
       * KPDs have argReplyPos 0, therefore they escape the above logic
       */
      (*arg->argKPD_Extract)(file, arg);
    }
    else if (akCheck(arg->argKind, akbUserImplicit)) {
      WriteExtractArgValueNormal(file, arg);
    }
  }
}

/*************************************************************
 *  Writes code to return the return value. Called by WriteRoutine
 *  for routines and functions.
 *************************************************************/
static void
WriteReturnValue(FILE *file, routine_t *rt)
{
  /* If returning RetCode, we have already checked that it is KERN_SUCCESS */
  WriteReturn(file, rt, "\t", "KERN_SUCCESS", "\n");
}

/*************************************************************
 *  Writes the elements of the message type declaration: the
 *  msg_type structure, the argument itself and any padding
 *  that is required to make the argument a multiple of 4 bytes.
 *  Called by WriteRoutine for all the arguments in the request
 *  message first and then the reply message.
 *************************************************************/
static void
WriteFieldDecl(FILE *file, argument_t *arg)
{
  if (akCheck(arg->argKind, akbSendKPD) ||
      akCheck(arg->argKind, akbReturnKPD))
    WriteFieldDeclPrim(file, arg, FetchKPDType);
  else
    WriteFieldDeclPrim(file, arg, FetchUserType);
}

/* Fill in the string with an expression that refers to the size
 * of the specified array:
 */
static void
GetArraySize(register argument_t *arg, char *size)
{
  register ipc_type_t *it = arg->argType;
  
  if (it->itVarArray) {
    if (arg->argCount->argByReferenceUser) {
      sprintf(size, "*%s", arg->argCount->argVarName);
    }
    else
      sprintf(size, "%s", arg->argCount->argVarName);
  }
  else {
    sprintf(size, "%d", (it->itNumber * it->itSize + 7) / 8);
  }
}


static void
WriteRPCPortDisposition(FILE *file, register argument_t *arg)
{
  /*
   * According to the MIG specification, the port disposition could be different
   * on input and output. If we stay with this then a new bitfield will have
   * to be added. Right now the port disposition is the same for in and out cases.
   */
  switch(arg->argType->itInName) {

    case  MACH_MSG_TYPE_MOVE_RECEIVE:
      fprintf(file, " | MACH_RPC_MOVE_RECEIVE");
      break;
      
    case  MACH_MSG_TYPE_MOVE_SEND:
      fprintf(file, " | MACH_RPC_MOVE_SEND");
      break;
      
    case  MACH_MSG_TYPE_MOVE_SEND_ONCE:
      fprintf(file, " | MACH_RPC_MOVE_SEND_ONCE");
      break;
      
    case  MACH_MSG_TYPE_COPY_SEND:
      fprintf(file, " | MACH_RPC_COPY_SEND");
      break;
      
    case  MACH_MSG_TYPE_MAKE_SEND:
      fprintf(file, " | MACH_RPC_MAKE_SEND");
      break;
      
    case  MACH_MSG_TYPE_MAKE_SEND_ONCE:
      fprintf(file, " | MACH_RPC_MAKE_SEND_ONCE");
      break;
  }
}

static void
WriteRPCArgDescriptor(FILE *file, register argument_t *arg, int offset)
{
  fprintf(file, "            {\n                0 ");
  if (RPCPort(arg)) {
    fprintf(file, "| MACH_RPC_PORT ");
    if (arg->argType->itNumber > 1)
      fprintf(file, "| MACH_RPC_ARRAY ");
    if (arg->argType->itVarArray)
      fprintf(file, "| MACH_RPC_VARIABLE ");
    WriteRPCPortDisposition(file, arg);
  }
  else if (RPCPortArray(arg)) {
    fprintf(file, "| MACH_RPC_PORT_ARRAY ");
    if (arg->argType->itVarArray)
      fprintf(file, "| MACH_RPC_VARIABLE ");
    WriteRPCPortDisposition(file, arg);
  }
  else if (RPCFixedArray(arg))
    fprintf(file, "| MACH_RPC_ARRAY_FIXED ");
  else if (RPCVariableArray(arg))
    fprintf(file, "| MACH_RPC_ARRAY_VARIABLE ");
  if (argIsIn(arg))
    fprintf(file, " | MACH_RPC_IN ");
  if (argIsOut(arg))
    fprintf(file, " | MACH_RPC_OUT ");
  if ((! arg->argType->itInLine) && (! arg->argType->itMigInLine))
    fprintf(file, " | MACH_RPC_POINTER ");
  if (arg->argFlags & flDealloc)
    fprintf(file, " | MACH_RPC_DEALLOCATE ");
  if (arg->argFlags & flPhysicalCopy)
    fprintf(file, " | MACH_RPC_PHYSICAL_COPY ");
  fprintf(file, ",\n");
  fprintf(file, "                %d,\n", (arg->argType->itSize / 8));
  fprintf(file, "                %d,\n", arg->argType->itNumber);
  fprintf(file, "                %d,\n            },\n", offset);
}

void
WriteRPCRoutineDescriptor(FILE *file, register routine_t *rt, int arg_count, int descr_count, string_t stub_routine, string_t sig_array)
{
  fprintf(file, "          { (mig_impl_routine_t) 0,\n\
          (mig_stub_routine_t) %s, ", stub_routine);
  fprintf(file, "%d, %d, %s}", arg_count, descr_count, sig_array);
}

void
WriteRPCRoutineArgDescriptor(FILE *file, register routine_t *rt)
{
  register argument_t *arg;
  int offset = 0;
  int size = 0;
  
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    boolean_t compound = arg->argType->itStruct && arg->argType->itInLine;
    
    if (RPCPort(arg) || RPCPortArray(arg) ||
        RPCFixedArray(arg) || RPCVariableArray(arg)) {
      WriteRPCArgDescriptor(file, arg, offset);
      size = 4;
    }
    if (! size) {
      if (compound)
        size = arg->argType->itNumber * (arg->argType->itSize / 8);
      else
        size = (arg->argType->itSize / 8);
    }
    if (akCheck(arg->argKind, akbServerArg))
      offset += size;
    size = 0;
  }
}


static void
WriteRPCSignature(FILE *file, register routine_t *rt)
{
  int arg_count = 0;
  int descr_count = 0;
  
  fprintf(file, "    kern_return_t rtn;\n");
  descr_count = rtCountArgDescriptors(rt->rtArgs, &arg_count);
  fprintf(file, "    const static struct\n    {\n");
  fprintf(file, "        struct rpc_routine_descriptor rd;\n");
  fprintf(file, "        struct rpc_routine_arg_descriptor rad[%d];\n", descr_count);
  fprintf(file, "    } sig =\n    {\n");
  WriteRPCRoutineDescriptor(file, rt, arg_count, descr_count, "0", "sig.rad, 0");
  fprintf(file, ",\n");
  fprintf(file, "        {\n");
  WriteRPCRoutineArgDescriptor(file, rt);
  fprintf(file, "\n        }\n");
  fprintf(file, "\n    };\n\n");
}

static void
WriteRPCCall(FILE *file, register routine_t *rt)
{
  register argument_t *arg;
  register int i;
  
  i = 0;
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    if (akIdent(arg->argKind) == akeRequestPort) {
      fprintf(file, "    rtn = (MACH_RPC(&sig, (mach_msg_size_t)sizeof(sig), %d, %s,\n", rt->rtNumber + SubsystemBase, arg->argVarName);
      fprintf(file, "                   (%s", arg->argVarName);
    }
    else if (akCheck(arg->argKind, akbServerArg)) {
      if (i && (i++ % 6 == 0))
        fprintf(file, ",\n                    ");
      else
        fprintf(file, ", ");
      fprintf(file, "%s", arg->argVarName);
    }
  }
  fprintf(file, ")));\n");
  fprintf(file, "\n");
  fprintf(file, "    if (rtn != KERN_NO_ACCESS) return rtn;\n\n");
  fprintf(file, "/* The following message rpc code is generated for the network case */\n\n");
}

static int
CheckRPCCall(register routine_t *rt)
{
  register argument_t *arg;
  register int i;
  
  i = 0;
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    if (akCheck(arg->argKind, akbUserArg) &&
        ((arg->argType->itOutName == -1) || (arg->argType->itInName == -1))) {
      return FALSE;
    }
    if (arg->argFlags & flMaybeDealloc) {
      return FALSE;
    }
  }
  return TRUE;
}

static void
WriteRPCRoutine(FILE *file, register routine_t *rt)
{
  if (CheckRPCCall(rt)) {
    WriteRPCSignature(file, rt);
    WriteRPCCall(file, rt);
  }
}

/********************** End UserRPCTrap Routines*************************/

/* Process an IN/INOUT arg before the short-circuited RPC */
static void
WriteShortCircInArgBefore(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char size[128];
  
  fprintf(file, "\n\t/* IN %s: */\n", arg->argVarName);
  
  if (akCheck(arg->argKind, akbSendKPD|akbReturnKPD)) {
    switch (arg->argKPD_Type) {

      case MACH_MSG_PORT_DESCRIPTOR:
        break;

      case MACH_MSG_OOL_DESCRIPTOR:
        /* Arg is an out-of-line array: */
        if (!(arg->argFlags & flDealloc) &&
            (!(arg->argFlags & flAuto) || !(arg->argFlags & flConst))) {
          /* Need to map a copy of the array: */
          GetArraySize(arg, size);
          fprintf(file, "\t(void)vm_read(mach_task_self(),\n");
          fprintf(file, "\t\t      (vm_address_t) %s%s, %s, (vm_address_t *) &_%sTemp_, &_MIG_Ignore_Count_);\n", (arg->argByReferenceUser ? "*" : ""), arg->argVarName, size, arg->argVarName);
          /* Point argument at the copy: */
          fprintf(file, "\t*(char **)&%s%s = _%sTemp_;\n", (arg->argByReferenceUser ? "*" : ""), arg->argVarName, arg->argVarName);
        }
        else if ((arg->argFlags & flDealloc) &&
                   ((arg->argFlags & flAuto) || it->itMigInLine)) {
          /* Point the temp var at the original argument: */
          fprintf(file, "\t_%sTemp_ = (char *) %s%s;\n", arg->argVarName, (arg->argByReferenceUser ? "*" : ""), arg->argVarName);
        }
        break;

      case MACH_MSG_OOL_PORTS_DESCRIPTOR:
        break;

      default:
        printf("MiG internal error: type of kernel processed data unknown\n");
        exit(1);
    }   /* end of switch */
  }
  else if (it->itNumber > 1) {
    if (it->itStruct) {
      /* Arg is a struct -- nothing to do. */
    }
    else {
      /* Arg is a C string or an in-line array: */
      if (!argIsOut(arg) && !(arg->argFlags & flConst)) {
        /* Have to copy it into a temp.  Use a stack var, if this would
         * not overflow the -maxonstack specification:
         * Conservatively assume ILP32 thresholds
         */
        if (it->itTypeSize <= sizeof(natural_t) ||
            rtMessOnStack(arg->argRoutine) ||
            arg->argRoutine->rtTempBytesOnStack +
            it->itTypeSize <= MaxMessSizeOnStack) {
          fprintf(file, "\t{   char _%sTemp_[%d];\n", arg->argVarName, it->itTypeSize);
          arg->argRoutine->rtTempBytesOnStack += it->itTypeSize;
          arg->argTempOnStack = TRUE;
        }
        else {
          fprintf(file, "\t{   _%sTemp_ = (char *) %s(%d);\n", arg->argVarName, MessAllocRoutine, it->itTypeSize);
          arg->argTempOnStack = FALSE;
        }
        WriteCopyArg(file, arg, "_%sTemp_", "/* %s */ (char *) %s", arg->argVarName, arg->argVarName);
        /* Point argument at temp: */
        fprintf(file, "\t    *(char **)&%s%s = _%sTemp_;\n", (arg->argByReferenceUser ? "*" : ""), arg->argVarName, arg->argVarName);
        fprintf(file, "\t}\n");
      }
    }
  }
}


/* Process an INOUT/OUT arg before the short-circuited RPC */
static void
WriteShortCircOutArgBefore(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  
  fprintf(file, "\n\t/* OUT %s: */\n", arg->argVarName);
  
  if (akCheck(arg->argKind, akbSendKPD|akbReturnKPD)) {
    switch (arg->argKPD_Type) {

      case MACH_MSG_PORT_DESCRIPTOR:
        break;

      case MACH_MSG_OOL_DESCRIPTOR:
        /* Arg is an out-of-line array: */
        if (!argIsIn(arg) && (arg->argFlags & flOverwrite)) {
          /* Point the temp var at the original argument: */
          fprintf(file, "\t    _%sTemp_ = (char *) %s%s;\n", arg->argVarName, (arg->argByReferenceUser ? "*" : ""), arg->argVarName);
        }
        break;

      case MACH_MSG_OOL_PORTS_DESCRIPTOR:
        break;

      default:
        printf("MiG internal error: type of kernel processed data unknown\n");
        exit(1);
    }   /* end of switch */
  }
  else if (it->itNumber > 1) {
    /* Arg is an in-line array: */
  }
}



/* Process an IN arg after the short-circuited RPC */
static void
WriteShortCircInArgAfter(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char size[128];
  
  fprintf(file, "\n\t/* IN %s: */\n", arg->argVarName);
  
  if (akCheck(arg->argKind, akbSendKPD|akbReturnKPD)) {
    switch (arg->argKPD_Type) {

      case MACH_MSG_PORT_DESCRIPTOR:
        break;

      case MACH_MSG_OOL_DESCRIPTOR:
        /* Arg is an out-of-line array: */
        GetArraySize(arg, size);
        if ((!(arg->argFlags & flAuto) && it->itMigInLine) ||
            ((arg->argFlags & flAuto) &&
             ((arg->argFlags & flDealloc) ||
              !(arg->argFlags & flConst))
             )) {
              /* Need to dealloc the temporary: */
              fprintf(file, "\t(void)vm_deallocate(mach_task_self(),");
          fprintf(file, " (vm_address_t *) _%sTemp_, %s);\n", arg->argVarName, size);
            }
        break;

      case MACH_MSG_OOL_PORTS_DESCRIPTOR:
        break;

      default:
        printf("MiG internal error: type of kernel processed data unknown\n");
        exit(1);
    }   /* end of switch */
  }
  else if (it->itNumber > 1) {
    if (it->itStruct) {
      /* Arg is a struct -- nothing to do. */
    }
    else {
      /* Arg is a C string or an in-line array: */
      if (!argIsOut(arg) && !(arg->argFlags & flConst)) {
        /* A temp needs to be deallocated, if not on stack: */
        if (!arg->argTempOnStack) {
          fprintf(file, "\t%s(_%sTemp_, %d);\n", MessFreeRoutine, arg->argVarName, it->itTypeSize);
        }
      }
    }
  }
}

static void
WriteShortCircOutArgAfter(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char size[128];
  
  fprintf(file, "\n\t/* OUT %s: */\n", arg->argVarName);
  
  if (akCheck(arg->argKind, akbSendKPD|akbReturnKPD)) {
    switch (arg->argKPD_Type) {

      case MACH_MSG_PORT_DESCRIPTOR:
        break;

      case MACH_MSG_OOL_DESCRIPTOR:
        /* Arg is an out-of-line array: */
        
        /* Calculate size of array: */
        GetArraySize(arg, size);
        if (!(arg->argFlags & flDealloc) || (arg->argFlags & flOverwrite)) {
          /* Copy argument to vm_allocated Temp: */
          fprintf(file, "\t(void)vm_read(mach_task_self(),\n");
          fprintf(file, "\t\t      (vm_address_t) %s%s, %s, (vm_address_t *) &_%sTemp_, &_MIG_Ignore_Count_);\n", (arg->argByReferenceUser ? "*" : ""), arg->argVarName, size, arg->argVarName);
          if (!argIsIn(arg) && (arg->argFlags & flDealloc) &&
              (arg->argFlags & flOverwrite)) {
            /* Deallocate argument returned by server */
            fprintf(file, "\t(void)vm_deallocate(mach_task_self(),");
            fprintf(file, " (vm_address_t *) %s%s, %s);\n", (arg->argByReferenceUser ? "*" : ""), arg->argVarName, size);
          }
          /* Point argument at new temporary: */
          fprintf(file, "\t*(char **)&%s%s = _%sTemp_;\n", (arg->argByReferenceUser ? "*" : ""), arg->argVarName, arg->argVarName);
        }
        break;

      case MACH_MSG_OOL_PORTS_DESCRIPTOR:
        break;

      default:
        printf("MiG internal error: type of kernel processed data unknown\n");
        exit(1);
    }   /* end of switch */
  }
  else if (it->itNumber != 1) {
    /* Arg is an in-line array: */
  }
}


static void
WriteShortCircRPC(FILE *file, register routine_t *rt)
{
  register argument_t *arg;
  register int server_argc, i;
  boolean_t ShortCircOkay = TRUE;
  boolean_t first_OOL_arg = TRUE;
  
  fprintf(file, "    if (0 /* Should be: !(%s & 0x3) XXX */) {\n", rt->rtRequestPort->argVarName);
  
  if (rt->rtOneWay) {
    /* Do not short-circuit simple routines: */
    ShortCircOkay = FALSE;
  }
  else {
    /* Scan for any types we can't yet handle.  If found, give up on short-
     * circuiting and fall back to mach_msg:
     */
    for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)  {
      if (arg->argFlags & flMaybeDealloc) {
        ShortCircOkay = FALSE;
        break;
      }
      /* Can't yet handle ports: */
      if (akCheck(arg->argKind, akbSendKPD|akbReturnKPD) &&
          (arg->argKPD_Type == MACH_MSG_PORT_DESCRIPTOR ||
           arg->argKPD_Type == MACH_MSG_OOL_PORTS_DESCRIPTOR)) {
            ShortCircOkay = FALSE;
            break;
          }
    }
  }
  
  if (ShortCircOkay) {
    
    fprintf(file,"      rpc_subsystem_t subsystem = ((rpc_port_t)%s)->rp_subsystem;\n", rt->rtRequestPort->argVarName);
    fprintf(file, "\n");
    fprintf(file, "      if (subsystem && subsystem->start == %d) {\n", SubsystemBase);
    fprintf(file, "\tkern_return_t rtn;\n");
    fprintf(file, "\n");
    
    /* Declare temp vars for out-of-line array args, and for all array
     * args, if -maxonstack has forced us to allocate in-line arrays
     * off the stack:
     */
    rt->rtTempBytesOnStack = 0;
    for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)  {
      arg->argTempOnStack = FALSE;
      if (akCheck(arg->argKind, akbSendKPD|akbReturnKPD) &&
          arg->argKPD_Type == MACH_MSG_OOL_DESCRIPTOR) {
        if (first_OOL_arg) {
          /* Need a garbage temporary to hold the datacount
           * returned by vm_read, which we always ignore:
           */
          fprintf(file, "\tmach_msg_type_number_t _MIG_Ignore_Count_;\n");
          first_OOL_arg = FALSE;
        }
      }
      else if (!rtMessOnStack(rt) &&
                 arg->argType->itNumber > 1 && !arg->argType->itStruct) {
      }
      else
        continue;
      fprintf(file, "\tchar *_%sTemp_;\n", arg->argVarName);
      /* Conservatively assume ILP32 thresholds */
      rt->rtTempBytesOnStack += sizeof(natural_t);
    }
    
    /* Process the IN arguments, in order: */
    
    fprintf(file, "\t/* Pre-Process the IN arguments: */\n");
    for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
      if (argIsIn(arg))
        WriteShortCircInArgBefore(file, arg);
      if (argIsOut(arg))
        WriteShortCircOutArgBefore(file, arg);
    }
    fprintf(file, "\n");
    
    /* Count the number of server args: */
    server_argc = 0;
    for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)
      if (akCheck(arg->argKind, akbServerArg))
        server_argc++;
    
    /* Call RPC_SIMPLE to switch to server stack and function: */
    i = 0;
    for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)  {
      if (akIdent(arg->argKind) == akeRequestPort) {
        fprintf(file, "\trtn = RPC_SIMPLE(%s, %d, %d, (", arg->argVarName, rt->rtNumber + SubsystemBase, server_argc);
        fprintf(file, "%s", arg->argVarName);
      }
      else if (akCheck(arg->argKind, akbServerArg)) {
        if (i++ % 6 == 0)
          fprintf(file, ",\n\t\t");
        else
          fprintf(file, ", ");
        fprintf(file, "%s", arg->argVarName);
      }
    }
    fprintf(file, "));\n");
    fprintf(file, "\n");
    
    /* Process the IN and OUT arguments, in order: */
    fprintf(file, "\t/* Post-Process the IN and OUT arguments: */\n");
    for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)  {
      if (argIsIn(arg))
        WriteShortCircInArgAfter(file, arg);
      if (argIsOut(arg))
        WriteShortCircOutArgAfter(file, arg);
    }
    fprintf(file, "\n");
    
    fprintf(file, "\treturn rtn;\n");
    fprintf(file, "      }\n");
  }
  
  /* In latest design, the following is not necessary, because in
   * kernel-loaded tasks, the Mach port name is the same as the handle
   * used by the RPC mechanism, namely a pointer to the ipc_port, and
   * in user-mode tasks, the Mach port name gets renamed to be a pointer
   * to the user-mode rpc_port_t struct.
   */
#if 0
  if (IsKernelUser)
    fprintf(file, "      %s = (ipc_port_t)%s->rp_receiver_name;\n", rt->rtRequestPort->argVarName, rt->rtRequestPort->argVarName);
  else
    fprintf(file, "      %s = ((rpc_port_t)%s)->rp_receiver_name;\n", rt->rtRequestPort->argVarName, rt->rtRequestPort->argVarName);
#endif
  
  fprintf(file, "    }\n");
}

static void
WriteStubDecl(FILE *file, register routine_t *rt)
{
  fprintf(file, "\n");
  fprintf(file, "/* %s %s */\n", rtRoutineKindToStr(rt->rtKind), rt->rtName);
  fprintf(file, "mig_external %s %s\n", ReturnTypeStr(rt), rt->rtUserName);
  if (BeAnsiC) {
    fprintf(file, "(\n");
    WriteList(file, rt->rtArgs, WriteUserVarDecl, akbUserArg, ",\n", "\n");
    fprintf(file, ")\n");
  }
  else {
    fprintf(file, "#if\t%s\n", NewCDecl);
    fprintf(file, "(\n");
    WriteList(file, rt->rtArgs, WriteUserVarDecl, akbUserArg, ",\n", "\n");
    fprintf(file, ")\n");
    fprintf(file, "#else\n");
    fprintf(file, "\t(");
    WriteList(file, rt->rtArgs, WriteNameDecl, akbUserArg, ", ", "");
    fprintf(file, ")\n");
    WriteList(file, rt->rtArgs, WriteUserVarDecl, akbUserArg, ";\n", ";\n");
    fprintf(file, "#endif\t/* %s */\n", NewCDecl);
  }
  fprintf(file, "{\n");
}

static void
InitKPD_Disciplines(argument_t *args)
{
  argument_t *arg;
  extern void KPD_noop();
  extern void KPD_error();
  extern void WriteTemplateKPD_port();
  extern void WriteTemplateKPD_ool();
  extern void WriteTemplateKPD_oolport();
  
  /*
   * WriteKPD_port,  WriteExtractKPD_port,
   * WriteKPD_ool,  WriteExtractKPD_ool,
   * WriteKPD_oolport,  WriteExtractKPD_oolport
   * are local to this module (which is the reason why this initialization
   * takes place here rather than in utils.c).
   * Common routines for user and server will be established SOON, and
   * all of them (including the initialization) will be transfert to
   * utils.c
   * All the KPD disciplines are defaulted to be KPD_error().
   * Note that akbSendKPD and akbReturnKPd are not exclusive,
   * because of inout type of parameters.
   */
  for (arg = args; arg != argNULL; arg = arg->argNext)
    if (akCheck(arg->argKind, akbSendKPD|akbReturnKPD))
      switch (arg->argKPD_Type) {

        case MACH_MSG_PORT_DESCRIPTOR:
          arg->argKPD_Init = KPD_noop;
          if akCheck(arg->argKind, akbSendKPD) {
            arg->argKPD_Template = WriteTemplateKPD_port;
            arg->argKPD_Pack = WriteKPD_port;
          }
          if akCheck(arg->argKind, akbReturnKPD) {
            arg->argKPD_Extract = WriteExtractKPD_port;
            arg->argKPD_TypeCheck = WriteTCheckKPD_port;
          }
          break;

        case MACH_MSG_OOL_DESCRIPTOR:
          arg->argKPD_Init = KPD_noop;
          if akCheck(arg->argKind, akbSendKPD) {
            arg->argKPD_Template = WriteTemplateKPD_ool;
            arg->argKPD_Pack = WriteKPD_ool;
          }
          if akCheck(arg->argKind, akbReturnKPD) {
            arg->argKPD_TypeCheck = WriteTCheckKPD_ool;
            arg->argKPD_Extract = WriteExtractKPD_ool;
          }
          break;

        case MACH_MSG_OOL_PORTS_DESCRIPTOR:
          arg->argKPD_Init = KPD_noop;
          if akCheck(arg->argKind, akbSendKPD) {
            arg->argKPD_Template = WriteTemplateKPD_oolport;
            arg->argKPD_Pack = WriteKPD_oolport;
          }
          if akCheck(arg->argKind, akbReturnKPD) {
            arg->argKPD_TypeCheck = WriteTCheckKPD_oolport;
            arg->argKPD_Extract = WriteExtractKPD_oolport;
          }
          break;

        default:
          printf("MiG internal error: type of kernel processed data unknown\n");
          exit(1);
      }   /* end of switch */
}

static void
WriteLimitCheck(FILE *file, routine_t *rt)
{
  if (MaxMessSizeOnStack == -1 || UserTypeLimit == -1)
    return;
  if (!rt->rtRequestUsedLimit && !rt->rtReplyUsedLimit)
    return;
  fprintf(file, "#if LimitCheck\n");
  if (rt->rtRequestUsedLimit) {
    if (rt->rtRequestFits) {
      fprintf(file, "\tif ((sizeof(Request) - %d) > %d)\n", rt->rtRequestSizeKnown, UserTypeLimit);
      fprintf(file, "\t    __RequestOnStackAbort(%d, \"%s\");\n", SubsystemBase + rt->rtNumber, rt->rtName);
    }
    else if (rt->rtReplyFits) {
      fprintf(file, "\tif (sizeof(Request) < %d)\n", MaxMessSizeOnStack);
      fprintf(file, "\t    __MessageOffStackNote(%d, \"%s\");\n", SubsystemBase + rt->rtNumber, rt->rtName);
    }
  }
  if (rt->rtReplyUsedLimit) {
    if (rt->rtReplyFits) {
      fprintf(file, "\tif ((sizeof(Reply) - %d) > %d)\n", rt->rtReplySizeKnown, UserTypeLimit);
      fprintf(file, "\t    __ReplyOnStackAbort(%d, \"%s\");\n", SubsystemBase + rt->rtNumber, rt->rtName);
    }
    else if (rt->rtRequestFits) {
      fprintf(file, "\tif (sizeof(Reply) < %d)\n", MaxMessSizeOnStack);
      fprintf(file, "\t    __MessageOffStackNote(%d, \"%s\");\n", SubsystemBase + rt->rtNumber, rt->rtName);
    }
  }
  if (rt->rtRequestUsedLimit && rt->rtReplyUsedLimit &&
      ! (rt->rtRequestFits || rt->rtReplyFits)) {
    fprintf(file, "\tif (sizeof(Request) < %d \n", MaxMessSizeOnStack);
    fprintf(file, "&& sizeof(Reply) < %d)\n", MaxMessSizeOnStack);
    fprintf(file, "\t    __MessageOffStackNote(%d, \"%s\");\n", SubsystemBase + rt->rtNumber, rt->rtName);
  }
  fprintf(file, "#endif /* LimitCheck */\n");
}

static void
WriteCheckReply(FILE *file, routine_t *rt)
{
  int i;
  
  /* initialize the disciplines for the handling of KPDs */
  InitKPD_Disciplines(rt->rtArgs);
  
  if (rt->rtOneWay)
    return;
  
  fprintf(file, "\n");
  fprintf(file, "#if ( __MigTypeCheck ");
  if (CheckNDR)
	  fprintf(file, "|| __NDR_convert__ ");
  fprintf(file, ")\n");
  fprintf(file, "#if __MIG_check__Reply__%s_subsystem__\n", SubsystemName);
  fprintf(file, "#if !defined(__MIG_check__Reply__%s_t__defined)\n", rt->rtName);
  fprintf(file, "#define __MIG_check__Reply__%s_t__defined\n", rt->rtName);
  if (CheckNDR && akCheck(rt->rtNdrCode->argKind, akbReply)) {
    WriteList(file, rt->rtArgs, WriteReplyNDRConvertIntRepArgDecl, akbReturnNdr, "\n", "\n");
    WriteList(file, rt->rtArgs, WriteReplyNDRConvertCharRepArgDecl, akbReturnNdr, "\n", "\n");
    WriteList(file, rt->rtArgs, WriteReplyNDRConvertFloatRepArgDecl, akbReturnNdr, "\n", "\n");
  }
  fprintf(file, "\n");
  fprintf(file, "mig_internal kern_return_t __MIG_check__Reply__%s_t(__Reply__%s_t *Out0P", rt->rtName, rt->rtName);
  for (i = 1; i <= rt->rtMaxReplyPos; i++)
    fprintf(file, ", __Reply__%s_t **Out%dPP", rt->rtName, i);
  fprintf(file, ")\n{\n");
  
  
  fprintf(file, "\n\ttypedef __Reply__%s_t __Reply;\n", rt->rtName);
  for (i = 1; i <= rt->rtMaxReplyPos; i++)
    fprintf(file, "\t__Reply *Out%dP;\n", i);
  if (!rt->rtSimpleReply)
    fprintf(file, "\tboolean_t msgh_simple;\n");
  if (!rt->rtNoReplyArgs) {
    fprintf(file, "#if\t__MigTypeCheck\n");
    fprintf(file, "\tunsigned int msgh_size;\n");
    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  }
  if (rt->rtMaxReplyPos > 0)
    fprintf(file, "\tunsigned int msgh_size_delta;\n");
  if (rt->rtNumReplyVar > 0 || rt->rtMaxReplyPos > 0)
    fprintf(file, "\n");
  
  /* Check the values that are returned in the reply message */
  
  WriteCheckIdentity(file, rt);
  
  /* If the reply message has no Out parameters or return values
     other than the return code, we can type-check it and
     return it directly. */
  
  if (rt->rtNoReplyArgs && !rt->rtUserImpl) {
    if (CheckNDR && akCheck(rt->rtNdrCode->argKind, akbReply) && rt->rtRetCode)
      WriteReplyNDRConvertIntRepOneArgUse(file, rt->rtRetCode);
    WriteReturn(file, rt, "\t", stRetCode, "\n");
  }
  else {
    if (UseEventLogger)
      WriteLogMsg(file, rt, LOG_USER, LOG_REPLY);
    
    WriteRetCodeCheck(file, rt);
    
    /* Type Checking for the Out parameters which are typed */
    WriteList(file, rt->rtArgs, WriteTypeCheck, akbReturnKPD, "\n", "\n");
    
    {
      register argument_t *arg, *lastVarArg;
      
      lastVarArg = argNULL;
      for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
        /*
         * Advance message pointer if the last reply argument was
         * variable-length and the reply position will change.
         */
        if (lastVarArg != argNULL &&
            lastVarArg->argReplyPos < arg->argReplyPos) {
          WriteAdjustReplyMsgPtr(file, lastVarArg);
          lastVarArg = argNULL;
        }
        
        if (akCheckAll(arg->argKind, akbReturnRcv|akbReturnBody)) {
          if (akCheck(arg->argKind, akbVariable)) {
            WriteCheckMsgSize(file, arg);
            lastVarArg = arg;
          }
        }
      }
    }
    
    if (CheckNDR && akCheck(rt->rtNdrCode->argKind, akbReply)) {
      fprintf(file, "#if\t");
      WriteList(file, rt->rtArgs, WriteReplyNDRConvertIntRepArgCond, akbReturnNdr, " || \\\n\t", "\n");
      fprintf(file, "\tif (Out0P->NDR.int_rep != NDR_record.int_rep) {\n");
      WriteList(file, rt->rtArgs, WriteReplyNDRConvertIntRepArgUse, akbReturnNdr, "", "");
      fprintf(file, "\t}\n#endif\t/* defined(__NDR_convert__int_rep...) */\n\n");
      
      fprintf(file, "#if\t");
      WriteList(file, rt->rtArgs, WriteReplyNDRConvertCharRepArgCond, akbReturnNdr, " || \\\n\t", "\n");
      fprintf(file, "\tif (Out0P->NDR.char_rep != NDR_record.char_rep) {\n");
      WriteList(file, rt->rtArgs, WriteReplyNDRConvertCharRepArgUse, akbReturnNdr, "", "");
      fprintf(file, "\t}\n#endif\t/* defined(__NDR_convert__char_rep...) */\n\n");
      
      fprintf(file, "#if\t");
      WriteList(file, rt->rtArgs, WriteReplyNDRConvertFloatRepArgCond, akbReturnNdr, " || \\\n\t", "\n");
      fprintf(file, "\tif (Out0P->NDR.float_rep != NDR_record.float_rep) {\n");
      WriteList(file, rt->rtArgs, WriteReplyNDRConvertFloatRepArgUse, akbReturnNdr, "", "");
      fprintf(file, "\t}\n#endif\t/* defined(__NDR_convert__float_rep...) */\n\n");
    }
    fprintf(file, "\treturn MACH_MSG_SUCCESS;\n");
  }
  fprintf(file, "}\n");
  fprintf(file, "#endif /* !defined(__MIG_check__Reply__%s_t__defined) */\n", rt->rtName);
  fprintf(file, "#endif /* __MIG_check__Reply__%s_subsystem__ */\n", SubsystemName);
  fprintf(file, "#endif /* ( __MigTypeCheck ");
  if (CheckNDR)
	  fprintf(file, "|| __NDR_convert__ ");
  fprintf(file, ") */\n\n");
}

static void
WriteCheckReplyCall(FILE *file, routine_t *rt)
{
  int i;
  
  fprintf(file, "\n");
  fprintf(file, "#if\tdefined(__MIG_check__Reply__%s_t__defined)\n", rt->rtName);
  fprintf(file, "\tcheck_result = __MIG_check__Reply__%s_t((__Reply__%s_t *)Out0P", rt->rtName, rt->rtName);
  for (i = 1; i <= rt->rtMaxReplyPos; i++)
    fprintf(file, ", (__Reply__%s_t **)&Out%dP", rt->rtName, i);
  fprintf(file, ");\n");
  fprintf(file, "\tif (check_result != MACH_MSG_SUCCESS)\n");
  WriteReturnMsgError(file, rt, TRUE, argNULL, "check_result");
  fprintf(file, "#endif\t/* defined(__MIG_check__Reply__%s_t__defined) */\n", rt->rtName);
  fprintf(file, "\n");
}

void
WriteCheckReplies(FILE *file, statement_t *stats)
{
  statement_t *stat;
  
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine)
      WriteCheckReply(file, stat->stRoutine);
}

static void
WriteCheckReplyTrailerArgs(FILE *file, routine_t *rt)
{
  register argument_t *arg;
  
  if (rt->rtUserImpl)
    WriteCheckTrailerHead(file, rt, TRUE);
  
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    if (akCheck(arg->argKind, akbUserImplicit))
      WriteCheckTrailerSize(file, TRUE, arg);
  }
  if (rt->rtUserImpl)
    fprintf(file, "\n");
}


/*************************************************************
 *  Writes all the code comprising a routine body. Called by
 *  WriteUser for each routine.
 *************************************************************/
static void
WriteRoutine(FILE *file, register routine_t *rt)
{
  /* write the stub's declaration */
  WriteStubDecl(file, rt);
  
  /* Use the RPC trap for user-user and user-kernel RPC */
  if (UseRPCTrap)
    WriteRPCRoutine(file, rt);
  
  /* write the code for doing a short-circuited RPC: */
  if (ShortCircuit)
    WriteShortCircRPC(file, rt);
  
  /* typedef of structure for Request and Reply messages */
  WriteStructDecl(file, rt->rtArgs, WriteFieldDecl, akbRequest, "Request", rt->rtSimpleRequest, FALSE, FALSE, FALSE);
  if (!rt->rtOneWay) {
    WriteStructDecl(file, rt->rtArgs, WriteFieldDecl, akbReply, "Reply", rt->rtSimpleReply, TRUE, rt->rtUserImpl, FALSE);
    WriteStructDecl(file, rt->rtArgs, WriteFieldDecl, akbReply, "__Reply", rt->rtSimpleReply, FALSE, FALSE, FALSE);
  }
  if (rt->rtOverwrite)
    WriteStructDecl(file, rt->rtArgs, WriteFieldDecl, akbReply|akbOverwrite, "OverwriteTemplate", FALSE, TRUE, FALSE, TRUE);
  /*
   * Define a Minimal Reply structure to be used in case of errors
   */
  fprintf(file, "\t/*\n");
  fprintf(file, "\t * typedef struct {\n");
  fprintf(file, "\t * \tmach_msg_header_t Head;\n");
  fprintf(file, "\t * \tNDR_record_t NDR;\n");
  fprintf(file, "\t * \tkern_return_t RetCode;\n");
  fprintf(file, "\t * } mig_reply_error_t;\n");
  fprintf(file, "\t */\n");
  fprintf(file, "\n");
  
  
  /* declarations for local vars: Union of Request and Reply messages,
     InP, OutP and return value */
  
  WriteVarDecls(file, rt);
  
  /* declarations and initializations of the mach_msg_type_descriptor_t variables
     for each argument that is a Kernel Processed Data */
  
  WriteList(file, rt->rtArgs, WriteTemplateDeclIn, akbRequest | akbSendKPD, "\n", "\n");
  
  WriteLimitCheck(file, rt);
  WriteRetCodeArg(file, rt);
  
  /* fill in the fields that are non related to parameters */
  
  if (!rt->rtSimpleRequest)
    fprintf(file, "\tInP->msgh_body.msgh_descriptor_count = %d;\n", rt->rtRequestKPDs);
  
  /* fill in all the request message types and then arguments */
  
  WriteRequestArgs(file, rt);
  
  /* fill in request message head */
  
  WriteRequestHead(file, rt);
  fprintf(file, "\n");
  
  /* give the application a chance to do some stuff. */
  WriteApplMacro(file, "Send", "Before", rt);
  
  /* Write the send/receive or rpc call */
  
  if (UseEventLogger)
    WriteLogMsg(file, rt, LOG_USER, LOG_REQUEST);
  
  
  if (rt->rtOneWay) {
    WriteMsgSend(file, rt);
  }
  else {
    if (UseMsgRPC
#if USE_IMMEDIATE_SEND_TIMEOUT
        && (rt->rtWaitTime == argNULL)
#endif
        ) {
      /* overwrite mode meaningful only when UseMsgRPC is enabled */
      if (rt->rtOverwrite)
        WriteOverwriteTemplate(file, rt);
      WriteMsgRPC(file, rt);
    }
    else
      WriteMsgSendReceive(file, rt);
    
    WriteCheckReplyCall(file, rt);
    WriteCheckReplyTrailerArgs(file, rt);
    
    if (UseEventLogger)
      WriteLogMsg(file, rt, LOG_USER, LOG_REPLY);
    
    WriteReplyArgs(file, rt);
  }
  /* return the return value, if any */
  if (!rt->rtOneWay)  // WriteMsgSend() already wrote the 'return'
    WriteReturnValue(file, rt);
  fprintf(file, "}\n");
}

static void
WriteRPCClientFunctions(FILE *file, statement_t *stats)
{
  register statement_t *stat;
  char *fname;
  char *argfmt = "(mach_port_t, char *, mach_msg_type_number_t)";
  
  fprintf(file, "#ifdef AUTOTEST\n");
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine) {
      fname = stat->stRoutine->rtName;
      fprintf(file, "extern void client_%s%s;\n", fname, argfmt);
    }
  fprintf(file, "function_table_entry %s_client_functions[] =\n", SubsystemName);
  fprintf(file, "{\n");
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine) {
      fname = stat->stRoutine->rtName;
      fprintf(file, "    { \"%s\", client_%s },\n", fname, fname);
    }
  fprintf(file, "    { (char *) 0, (function_ptr_t) 0 }\n");
  fprintf(file, "};\n");
  fprintf(file, "#endif /* AUTOTEST */\n");
}

/*************************************************************
 *  Writes out the xxxUser.c file. Called by mig.c
 *************************************************************/
void
WriteUser(FILE *file, statement_t *stats)
{
  register statement_t *stat;
  
  WriteProlog(file, stats);
  if (TestRPCTrap)
    WriteRPCClientFunctions(file, stats);
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    switch (stat->stKind) {

      case skRoutine:
        WriteCheckReply(file, stat->stRoutine);
        WriteRoutine(file, stat->stRoutine);
        break;

      case skImport:
      case skUImport:
      case skSImport:
      case skDImport:
      case skIImport:
        break;

      default:
      fatal("WriteUser(): bad statement_kind_t (%d)", (int) stat->stKind);
    }
  WriteEpilog(file);
}

/*************************************************************
 *  Writes out individual .c user files for each routine.  Called by mig.c
 *************************************************************/
void
WriteUserIndividual(statement_t *stats)
{
  register statement_t *stat;
  
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    switch (stat->stKind) {

      case skRoutine: {
        FILE *file;
        register char *filename;
        
        filename = strconcat(UserFilePrefix, strconcat(stat->stRoutine->rtName, ".c"));
        file = fopen(filename, "w");
        if (file == NULL)
          fatal("fopen(%s): %s", filename, strerror(errno));
        WriteProlog(file, stats);
        WriteRoutine(file, stat->stRoutine);
        WriteEpilog(file);
        fclose(file);
        strfree(filename);
      }
        break;

      case skImport:
      case skUImport:
      case skSImport:
      case skDImport:
      case skIImport:
        break;

      default:
        fatal("WriteUserIndividual(): bad statement_kind_t (%d)", (int) stat->stKind);
    }
}

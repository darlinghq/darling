/*
 * Copyright (c) 1999-2009 Apple Inc. All rights reserved.
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
 * @OSF_COPYRIGHT@
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

#include <assert.h>
#include <stdlib.h>

#include <mach/message.h>
#include "write.h"
#include "utils.h"
#include "global.h"
#include "error.h"

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif  /* max */

void WriteLogDefines();
void WriteIdentificationString();
static void WriteFieldDecl();

static void
WriteKPD_Iterator(FILE *file, boolean_t in, boolean_t varying, argument_t *arg, boolean_t bracket)
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
    sprintf(string, "In%dP", arg->argRequestPos);
  else
    sprintf(string, "OutP");

  fprintf(file, "\t    ptr = &%s->%s[0];\n", string, arg->argMsgField);

  if (varying) {
    register argument_t *count = arg->argCount;

    if (in)
      fprintf(file, "\t    for (i = 0; i < In%dP->%s; ptr++, i++) %s\n", count->argRequestPos, count->argMsgField, (bracket) ? "{" : "");
    else {
      fprintf(file, "\t    j = min(%d, ", it->itKPD_Number);
      if (akCheck(count->argKind, akbVarNeeded))
        fprintf(file, "%s);\n", count->argName);
      else
        fprintf(file, "%s->%s);\n", string, count->argMsgField);
      fprintf(file, "\t    for (i = 0; i < j; ptr++, i++) %s\n", (bracket) ? "{" : "");
    }
}
  else
    fprintf(file, "\t    for (i = 0; i < %d; ptr++, i++) %s\n", it->itKPD_Number, (bracket) ? "{" : "");
}

static void
WriteMyIncludes(FILE *file, statement_t *stats)
{
  if (ServerHeaderFileName == strNULL || UseSplitHeaders)
    WriteIncludes(file, FALSE, FALSE);
  if (ServerHeaderFileName != strNULL)
  {
    register char *cp;

    /* Strip any leading path from ServerHeaderFileName. */
    cp = strrchr(ServerHeaderFileName, '/');
    if (cp == 0)
      cp = ServerHeaderFileName;
    else
      cp++;       /* skip '/' */
    fprintf(file, "#include \"%s\"\n", cp);
  }
  if (ServerHeaderFileName == strNULL || UseSplitHeaders)
    WriteImplImports(file, stats, FALSE);
  if (UseEventLogger) {
    if (IsKernelServer) {
      fprintf(file, "#if\t__MigKernelSpecificCode\n");
      fprintf(file, "#include <mig_debug.h>\n");
      fprintf(file, "#endif\t/* __MigKernelSpecificCode */\n");
    }
    fprintf(file, "#if  MIG_DEBUG\n");
    fprintf(file, "#include <mach/mig_log.h>\n");
    fprintf(file, "#endif /* MIG_DEBUG */\n");
  }

  fprintf(file, "\n");
}

static void
WriteGlobalDecls(FILE *file)
{
  if (BeAnsiC) {
    fprintf(file, "#define novalue void\n");
  }
  else {
    fprintf(file, "#if\t%s\n", NewCDecl);
    fprintf(file, "#define novalue void\n");
    fprintf(file, "#else\n");
    fprintf(file, "#define novalue int\n");
    fprintf(file, "#endif\t/* %s */\n", NewCDecl);
  }
  fprintf(file, "\n");

  if (RCSId != strNULL)
    WriteRCSDecl(file, strconcat(SubsystemName, "_server"), RCSId);

  /* Used for locations in the request message, *not* reply message.
   Reply message locations aren't dependent on IsKernelServer. */

  if (IsKernelServer) {
    fprintf(file, "#if\t__MigKernelSpecificCode\n");
    fprintf(file, "#define msgh_request_port\tmsgh_remote_port\n");
    fprintf(file, "#define MACH_MSGH_BITS_REQUEST(bits)");
    fprintf(file, "\tMACH_MSGH_BITS_REMOTE(bits)\n");
    fprintf(file, "#define msgh_reply_port\t\tmsgh_local_port\n");
    fprintf(file, "#define MACH_MSGH_BITS_REPLY(bits)");
    fprintf(file, "\tMACH_MSGH_BITS_LOCAL(bits)\n");
    fprintf(file, "#else\n");
  }
  fprintf(file, "#define msgh_request_port\tmsgh_local_port\n");
  fprintf(file, "#define MACH_MSGH_BITS_REQUEST(bits)");
  fprintf(file, "\tMACH_MSGH_BITS_LOCAL(bits)\n");
  fprintf(file, "#define msgh_reply_port\t\tmsgh_remote_port\n");
  fprintf(file, "#define MACH_MSGH_BITS_REPLY(bits)");
  fprintf(file, "\tMACH_MSGH_BITS_REMOTE(bits)\n");
  if (IsKernelServer) {
    fprintf(file, "#endif /* __MigKernelSpecificCode */\n");
  }
  fprintf(file, "\n");
  if (UseEventLogger)
    WriteLogDefines(file, "MACH_MSG_LOG_SERVER");
  fprintf(file, "#define MIG_RETURN_ERROR(X, code)\t{\\\n");
  fprintf(file, "\t\t\t\t((mig_reply_error_t *)X)->RetCode = code;\\\n");
  fprintf(file, "\t\t\t\t((mig_reply_error_t *)X)->NDR = NDR_record;\\\n");
  fprintf(file, "\t\t\t\treturn;\\\n");
  fprintf(file, "\t\t\t\t}\n");
  fprintf(file, "\n");
}


static void
WriteForwardDeclarations(FILE *file, statement_t *stats)
{
  register statement_t *stat;

  fprintf(file, "/* Forward Declarations */\n\n");
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine) {
      fprintf(file, "\nmig_internal novalue _X%s\n", stat->stRoutine->rtName);
      fprintf(file, "\t(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP);\n");
    }
  fprintf(file, "\n");
}

static void
WriteMIGCheckDefines(FILE *file)
{
   fprintf(file, "#define\t__MIG_check__Request__%s_subsystem__ 1\n", SubsystemName);
   fprintf(file, "\n");
}

static void
WriteNDRDefines(FILE *file)
{
   fprintf(file, "#define\t__NDR_convert__Request__%s_subsystem__ 1\n", SubsystemName);
   fprintf(file, "\n");
}

static void
WriteProlog(FILE *file, statement_t *stats)
{
  WriteIdentificationString(file);
  fprintf(file, "\n");
  fprintf(file, "/* Module %s */\n", SubsystemName);
  fprintf(file, "\n");
  WriteMIGCheckDefines(file);
  if (CheckNDR)
	  WriteNDRDefines(file);
  WriteMyIncludes(file, stats);
  WriteBogusDefines(file);
  WriteApplDefaults(file, "Rcv");
  WriteGlobalDecls(file);
  if (ServerHeaderFileName == strNULL) {
    WriteRequestTypes(file, stats);
    WriteReplyTypes(file, stats);
    WriteServerReplyUnion(file, stats);
  }
}

static void
WriteSymTabEntries(FILE *file, statement_t *stats)
{
  register statement_t *stat;
  register u_int current = 0;

  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine) {
      register  int num = stat->stRoutine->rtNumber;
      char  *name = stat->stRoutine->rtName;
      while (++current <= num)
        fprintf(file,"\t\t\t{ \"\", 0, 0 },\n");
      fprintf(file, "\t{ \"%s\", %d, _X%s },\n", name, SubsystemBase + current - 1, name);
    }
  while (++current <= rtNumber)
    fprintf(file,"\t{ \"\", 0, 0 },\n");
}

static void
WriteRoutineEntries(FILE *file, statement_t *stats)
{
  register u_int current = 0;
  register statement_t *stat;
  char *sig_array, *rt_name;
  int arg_count, descr_count;
  int offset = 0;
  size_t serverSubsysNameLen = strlen(ServerSubsys);

  fprintf(file, "\t{\n");
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine) {
      register  routine_t *rt = stat->stRoutine;
      size_t       rtNameLen = strlen(rt->rtName);

      // Include length of rt->rtName in calculation of necessary buffer size, since that string
      // is actually written into the buffer along with the Server Subsystem name.
      sig_array = (char *) malloc(serverSubsysNameLen + rtNameLen + 80);
      rt_name = (char *) malloc(rtNameLen + 5);
      while (current++ < rt->rtNumber)
        fprintf(file, "\t\t{0, 0, 0, 0, 0, 0},\n");
      // NOTE: if either of the two string constants in the sprintf() function calls below get
      // much longer, be sure to increase the constant '80' (in the first malloc() call) to ensure
      // that the allocated buffer is large enough. (Currently, I count 66 characters in the first
      // string constant, 65 in the second. 80 ought to be enough for now...)
      if (UseRPCTrap) {
        sprintf(sig_array, "&%s.arg_descriptor[%d], (mach_msg_size_t)sizeof(__Reply__%s_t)", ServerSubsys, offset, rt->rtName);
      }
      else {
        sprintf(sig_array, "(routine_arg_descriptor_t)0, (mach_msg_size_t)sizeof(__Reply__%s_t)", rt->rtName);
      }
      sprintf(rt_name, "_X%s", rt->rtName);
      descr_count = rtCountArgDescriptors(rt->rtArgs, &arg_count);
      offset += descr_count;
      WriteRPCRoutineDescriptor(file, rt, arg_count, (UseRPCTrap) ? descr_count : 0, rt_name, sig_array);
      fprintf(file, ",\n");
      free(sig_array);
      free(rt_name);
    }
  while (current++ < rtNumber)
    fprintf(file, "\t\t{0, 0, 0, 0, 0, 0},\n");

  fprintf(file, "\t}");
}

static void
WriteArgDescriptorEntries(FILE *file, statement_t *stats)
{
  register statement_t *stat;

  fprintf(file, ",\n\n\t{\n");
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine) {
      register routine_t *rt = stat->stRoutine;

      /* For each arg of the routine, write an arg descriptor:
       */
      WriteRPCRoutineArgDescriptor(file, rt);
    }
  fprintf(file, "\t},\n\n");
}


/*
 * Write out the description of this subsystem, for use in direct RPC
 */
static void
WriteSubsystem(FILE *file, statement_t *stats)
{
  register statement_t *stat;
  int descr_count = 0;

  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine) {
      register routine_t *rt = stat->stRoutine;
      descr_count += rtCountArgDescriptors(rt->rtArgs, (int *) 0);
    }
  fprintf(file, "\n");
  if (ServerHeaderFileName == strNULL) {
    WriteMigExternal(file);
    fprintf(file, "boolean_t %s(", ServerDemux);
    if (BeAnsiC) {
      fprintf(file, "\n\t\tmach_msg_header_t *InHeadP,");
      fprintf(file, "\n\t\tmach_msg_header_t *OutHeadP");
    }
    fprintf(file, ");\n\n");

    WriteMigExternal(file);
    fprintf(file, "mig_routine_t %s_routine(", ServerDemux);
    if (BeAnsiC) {
      fprintf(file, "\n\t\tmach_msg_header_t *InHeadP");
    }
    fprintf(file, ");\n\n");
  }
  fprintf(file, "\n/* Description of this subsystem, for use in direct RPC */\n");
  if (ServerHeaderFileName == strNULL) {
    fprintf(file, "const struct %s {\n", ServerSubsys);
    if (UseRPCTrap) {
      fprintf(file, "\tstruct subsystem *\tsubsystem;\t/* Reserved for system use */\n");
    }
    else {
      fprintf(file, "\tmig_server_routine_t \tserver;\t/* Server routine */\n");
    }
    fprintf(file, "\tmach_msg_id_t\tstart;\t/* Min routine number */\n");
    fprintf(file, "\tmach_msg_id_t\tend;\t/* Max routine number + 1 */\n");
    fprintf(file, "\tunsigned int\tmaxsize;\t/* Max msg size */\n");
    if (UseRPCTrap) {
      fprintf(file, "\tvm_address_t\tbase_addr;\t/* Base address */\n");
      fprintf(file, "\tstruct rpc_routine_descriptor\t/*Array of routine descriptors */\n");
    }
    else {
      fprintf(file, "\tvm_address_t\treserved;\t/* Reserved */\n");
      fprintf(file, "\tstruct routine_descriptor\t/*Array of routine descriptors */\n");
    }
    fprintf(file, "\t\troutine[%d];\n", rtNumber);
    if (UseRPCTrap) {
      fprintf(file, "\tstruct rpc_routine_arg_descriptor\t/*Array of arg descriptors */\n");
      fprintf(file, "\t\targ_descriptor[%d];\n", descr_count);
    }
    fprintf(file, "} %s = {\n", ServerSubsys);
  }
  else {
    fprintf(file, "const struct %s %s = {\n", ServerSubsys, ServerSubsys);
  }
  if (UseRPCTrap) {
    fprintf(file, "\t0,\n");
  }
  else {
    fprintf(file, "\t%s_routine,\n", ServerDemux);
  }
  fprintf(file, "\t%d,\n", SubsystemBase);
  fprintf(file, "\t%d,\n", SubsystemBase + rtNumber);
  fprintf(file, "\t(mach_msg_size_t)sizeof(union __ReplyUnion__%s),\n", ServerSubsys);
  if (UseRPCTrap) {
    fprintf(file, "\t(vm_address_t)&%s,\n", ServerSubsys);
  }
  else {
    fprintf(file, "\t(vm_address_t)0,\n");
  }
  WriteRoutineEntries(file, stats);

  if (UseRPCTrap)
    WriteArgDescriptorEntries(file, stats);
  else
    fprintf(file, "\n");

  fprintf(file, "};\n\n");
}

#if NOT_CURRENTLY_USED

static void
WriteArraySizes(FILE *file, statement_t *stats)
{
  register u_int current = 0;
  register statement_t *stat;

  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine) {
      register routine_t *rt = stat->stRoutine;

      while (current++ < rt->rtNumber)
        fprintf(file, "\t\t0,\n");
      fprintf(file, "\t\t(mach_msg_size_t)sizeof(__Reply__%s_t),\n", rt->rtName);
    }
  while (current++ < rtNumber)
    fprintf(file, "\t\t\t0,\n");
}

#endif /* NOT_CURRENTLY_USED */

void
WriteServerRequestUnion(FILE *file, statement_t *stats)
{
  register statement_t *stat;

  fprintf(file, "\n");
  fprintf(file, "/* union of all requests */\n\n");
  fprintf(file, "#ifndef __RequestUnion__%s__defined\n", ServerSubsys);
  fprintf(file, "#define __RequestUnion__%s__defined\n", ServerSubsys);
  fprintf(file, "union __RequestUnion__%s {\n", ServerSubsys);
  for (stat = stats; stat != stNULL; stat = stat->stNext) {
    if (stat->stKind == skRoutine) {
      register routine_t *rt;

      rt = stat->stRoutine;
      fprintf(file, "\t__Request__%s_t Request_%s;\n", rt->rtName, rt->rtName);
    }
  }
  fprintf(file, "};\n");
  fprintf(file, "#endif /* __RequestUnion__%s__defined */\n", ServerSubsys);
}

void
WriteServerReplyUnion(FILE *file, statement_t *stats)
{
  register statement_t *stat;

  fprintf(file, "\n");
  fprintf(file, "/* union of all replies */\n\n");
  fprintf(file, "#ifndef __ReplyUnion__%s__defined\n", ServerSubsys);
  fprintf(file, "#define __ReplyUnion__%s__defined\n", ServerSubsys);
  fprintf(file, "union __ReplyUnion__%s {\n", ServerSubsys);
  for (stat = stats; stat != stNULL; stat = stat->stNext) {
    if (stat->stKind == skRoutine) {
      register routine_t *rt;

      rt = stat->stRoutine;
      fprintf(file, "\t__Reply__%s_t Reply_%s;\n", rt->rtName, rt->rtName);
    }
  }
  fprintf(file, "};\n");
  fprintf(file, "#endif /* __RequestUnion__%s__defined */\n", ServerSubsys);
}

static void
WriteDispatcher(FILE *file, statement_t *stats)
{
  /*
   * Write the subsystem stuff.
   */
  fprintf(file, "\n");
  WriteSubsystem(file, stats);

  /*
   * Then, the server routine
   */
  fprintf(file, "mig_external boolean_t %s\n", ServerDemux);
  if (BeAnsiC) {
    fprintf(file, "\t(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)\n");
  }
  else {
    fprintf(file, "#if\t%s\n", NewCDecl);
    fprintf(file, "\t(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)\n");
    fprintf(file, "#else\n");
    fprintf(file, "\t(InHeadP, OutHeadP)\n");
    fprintf(file, "\tmach_msg_header_t *InHeadP, *OutHeadP;\n");
    fprintf(file, "#endif\t/* %s */\n", NewCDecl);
  }

  fprintf(file, "{\n");
  fprintf(file, "\t/*\n");
  fprintf(file, "\t * typedef struct {\n");
  fprintf(file, "\t * \tmach_msg_header_t Head;\n");
  fprintf(file, "\t * \tNDR_record_t NDR;\n");
  fprintf(file, "\t * \tkern_return_t RetCode;\n");
  fprintf(file, "\t * } mig_reply_error_t;\n");
  fprintf(file, "\t */\n");
  fprintf(file, "\n");

  fprintf(file, "\tregister mig_routine_t routine;\n");
  fprintf(file, "\n");

  fprintf(file, "\tOutHeadP->msgh_bits = ");
  fprintf(file, "MACH_MSGH_BITS(MACH_MSGH_BITS_REPLY(InHeadP->msgh_bits), 0);\n");
  fprintf(file, "\tOutHeadP->msgh_remote_port = InHeadP->msgh_reply_port;\n");
  fprintf(file, "\t/* Minimal size: routine() will update it if different */\n");
  fprintf(file, "\tOutHeadP->msgh_size = (mach_msg_size_t)sizeof(mig_reply_error_t);\n");
  fprintf(file, "\tOutHeadP->msgh_local_port = MACH_PORT_NULL;\n");
  fprintf(file, "\tOutHeadP->msgh_id = InHeadP->msgh_id + 100;\n");
  fprintf(file, "\n");

  fprintf(file, "\tif ((InHeadP->msgh_id > %d) || (InHeadP->msgh_id < %d) ||\n", SubsystemBase + rtNumber - 1, SubsystemBase);
  fprintf(file, "\t    ((routine = %s.routine[InHeadP->msgh_id - %d].stub_routine) == 0)) {\n", ServerSubsys, SubsystemBase);
  fprintf(file, "\t\t((mig_reply_error_t *)OutHeadP)->NDR = NDR_record;\n");
  fprintf(file, "\t\t((mig_reply_error_t *)OutHeadP)->RetCode = MIG_BAD_ID;\n");
  if (UseEventLogger) {
    fprintf(file, "#if  MIG_DEBUG\n");
    fprintf(file, "\t\tLOG_ERRORS(MACH_MSG_LOG_SERVER, MACH_MSG_ERROR_UNKNOWN_ID,\n");
    fprintf(file, "\t\t\t&InHeadP->msgh_id, __FILE__, __LINE__);\n");
    fprintf(file, "#endif /* MIG_DEBUG */\n");
  }
  fprintf(file, "\t\treturn FALSE;\n");
  fprintf(file, "\t}\n");

  /* Call appropriate routine */
  fprintf(file, "\t(*routine) (InHeadP, OutHeadP);\n");
  fprintf(file, "\treturn TRUE;\n");
  fprintf(file, "}\n");
  fprintf(file, "\n");

  /*
   * Then, the <subsystem>_server_routine routine
   */
  fprintf(file, "mig_external mig_routine_t %s_routine\n", ServerDemux);
  if (BeAnsiC) {
    fprintf(file, "\t(mach_msg_header_t *InHeadP)\n");
  }
  else {
    fprintf(file, "#if\t%s\n", NewCDecl);
    fprintf(file, "\t(mach_msg_header_t *InHeadP)\n");
    fprintf(file, "#else\n");
    fprintf(file, "\t(InHeadP)\n");
    fprintf(file, "\tmach_msg_header_t *InHeadP;\n");
    fprintf(file, "#endif\t/* %s */\n", NewCDecl);
  }

  fprintf(file, "{\n");
  fprintf(file, "\tregister int msgh_id;\n");
  fprintf(file, "\n");
  fprintf(file, "\tmsgh_id = InHeadP->msgh_id - %d;\n", SubsystemBase);
  fprintf(file, "\n");
  fprintf(file, "\tif ((msgh_id > %d) || (msgh_id < 0))\n", rtNumber - 1);
  fprintf(file, "\t\treturn 0;\n");
  fprintf(file, "\n");
  fprintf(file, "\treturn %s.routine[msgh_id].stub_routine;\n", ServerSubsys);
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

#if NOT_CURRENTLY_USED
/*
 *  Returns the return type of the server-side work function.
 *  Suitable for "extern %s serverfunc()".
 */
static char *
ServerSideType(routine_t *rt)
{
  return rt->rtRetCode->argType->itTransType;
}
#endif /* NOT_CURRENTLY_USED */

static void
WriteRetCode(FILE *file, register argument_t *ret)
{
  register ipc_type_t *it = ret->argType;

  if (akCheck(ret->argKind, akbVarNeeded)) {
    fprintf(file, "\t%s %s;\n", it->itTransType, ret->argVarName);
  }
}

static void
WriteLocalVarDecl(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  register ipc_type_t *btype = it->itElement;

  if (IS_VARIABLE_SIZED_UNTYPED(it))
    fprintf(file, "\t%s %s[%d]", btype->itTransType, arg->argVarName, btype->itNumber ? it->itNumber/btype->itNumber : 0);
  else if (IS_MULTIPLE_KPD(it)) {
    if (btype->itTransType != strNULL)
      fprintf(file, "\t%s %s[%d]", btype->itTransType, arg->argVarName, it->itKPD_Number);
    else
    /* arrays of ool or oolport */
      fprintf(file, "\tvoid *%s[%d]", arg->argVarName, it->itKPD_Number);
  }
  else
    fprintf(file, "\t%s %s", it->itTransType, arg->argVarName);
}

#if NOT_CURRENTLY_USED
static void
WriteServerArgDecl(FILE *file, argument_t *arg)
{
  fprintf(file, "%s %s%s", arg->argType->itTransType, arg->argByReferenceServer ? "*" : "", arg->argVarName);
}
#endif /* NOT_CURRENTLY_USED */

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

  /* if reply is variable, we may need msgh_size_delta and msgh_size */
  if (rt->rtNumReplyVar > 1)
    fprintf(file, "\tunsigned int msgh_size;\n");
  if (rt->rtMaxReplyPos > 0)
    fprintf(file, "\tunsigned int msgh_size_delta;\n");
  if (rt->rtNumReplyVar > 1 || rt->rtMaxReplyPos > 0)
    fprintf(file, "\n");

  if (rt->rtServerImpl) {
    fprintf(file, "\tmach_msg_max_trailer_t *TrailerP;\n");
    fprintf(file, "#if\t__MigTypeCheck\n");
    fprintf(file, "\tunsigned int trailer_size;\n");
    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  }
  fprintf(file, "#ifdef\t__MIG_check__Request__%s_t__defined\n", rt->rtName);
  fprintf(file, "\tkern_return_t check_result;\n");
  fprintf(file, "#endif\t/* __MIG_check__Request__%s_t__defined */\n", rt->rtName);
  fprintf(file, "\n");
}

static void
WriteReplyInit(FILE *file, routine_t *rt)
{
  fprintf(file, "\n");
  if  (rt->rtNumReplyVar > 1 || rt->rtMaxReplyPos)
    /* WritheAdjustMsgSize() has been executed at least once! */
    fprintf(file, "\tOutP = (Reply *) OutHeadP;\n");

  if (!rt->rtSimpleReply)  /* complex reply message */
    fprintf(file, "\tOutP->Head.msgh_bits |= MACH_MSGH_BITS_COMPLEX;\n");

  if (rt->rtNumReplyVar == 0) {
    fprintf(file, "\tOutP->Head.msgh_size = ");
    rtMinReplySize(file, rt, "Reply");
    fprintf(file, ";\n");
  }
  else if (rt->rtNumReplyVar > 1)
    fprintf(file, "\tOutP->Head.msgh_size = msgh_size;\n");
  /* the case rt->rtNumReplyVar = 1 is taken care of in WriteAdjustMsgSize() */
}

static void
WriteRetCArgCheckError(FILE *file, routine_t *rt)
{
  fprintf(file, "\tif (!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) &&\n");
  fprintf(file, "\t    (In0P->Head.msgh_size == (mach_msg_size_t)sizeof(mig_reply_error_t)))\n");
  fprintf(file, "\t{\n");
}

static void
WriteRetCArgFinishError(FILE *file, routine_t *rt)
{
  argument_t *retcode = rt->rtRetCArg;

  fprintf(file, "\treturn;\n");
  fprintf(file, "\t}\n");
  retcode->argMsgField = "KERN_SUCCESS";
}

static void
WriteCheckHead(FILE *file, routine_t *rt)
{
  fprintf(file, "#if\t__MigTypeCheck\n");
  if (rt->rtNumRequestVar > 0)
    fprintf(file, "\tmsgh_size = In0P->Head.msgh_size;\n");

  if (rt->rtSimpleRequest) {
    /* Expecting a simple message. */
    fprintf(file, "\tif ((In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||\n");
    if (rt->rtNumRequestVar > 0) {
      fprintf(file, "\t    (msgh_size < ");
      rtMinRequestSize(file, rt, "__Request");
      fprintf(file, ") ||  (msgh_size > (mach_msg_size_t)sizeof(__Request)))\n");
    }
    else
      fprintf(file, "\t    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request)))\n");
  }
  else {
    /* Expecting a complex message. */

    fprintf(file, "\tif (");
    if (rt->rtRetCArg != argNULL)
      fprintf(file, "(");
    fprintf(file, "!(In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||\n");
    fprintf(file, "\t    (In0P->msgh_body.msgh_descriptor_count != %d) ||\n", rt->rtRequestKPDs);
    if (rt->rtNumRequestVar > 0) {
      fprintf(file, "\t    (msgh_size < ");
      rtMinRequestSize(file, rt, "__Request");
      fprintf(file, ") ||  (msgh_size > (mach_msg_size_t)sizeof(__Request))");
    }
    else
      fprintf(file, "\t    (In0P->Head.msgh_size != (mach_msg_size_t)sizeof(__Request))");
    if (rt->rtRetCArg == argNULL)
      fprintf(file, ")\n");
    else {
      fprintf(file, ") &&\n");
      fprintf(file, "\t    ((In0P->Head.msgh_bits & MACH_MSGH_BITS_COMPLEX) ||\n");
      fprintf(file, "\t    In0P->Head.msgh_size != (mach_msg_size_t)sizeof(mig_reply_error_t) ||\n");
      fprintf(file, "\t    ((mig_reply_error_t *)In0P)->RetCode == KERN_SUCCESS))\n");
    }
  }
  fprintf(file, "\t\treturn MIG_BAD_ARGUMENTS;\n");
  fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  fprintf(file, "\n");
}

void
WriteRequestNDRConvertIntRepArgCond(FILE *file, argument_t *arg)
{
  routine_t *rt = arg->argRoutine;

  fprintf(file, "defined(__NDR_convert__int_rep__Request__%s_t__%s__defined)", rt->rtName, arg->argMsgField);
}

void
WriteRequestNDRConvertCharRepArgCond(FILE *file, argument_t *arg)
{
  routine_t *rt = arg->argRoutine;
  
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) != akeCountInOut)
    fprintf(file, "defined(__NDR_convert__char_rep__Request__%s_t__%s__defined)", rt->rtName, arg->argMsgField);
  else
    fprintf(file, "0");
}

void
WriteRequestNDRConvertFloatRepArgCond(FILE *file, argument_t *arg)
{
  routine_t *rt = arg->argRoutine;

  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) != akeCountInOut)
    fprintf(file, "defined(__NDR_convert__float_rep__Request__%s_t__%s__defined)", rt->rtName, arg->argMsgField);
  else
    fprintf(file, "0");
}

void
WriteRequestNDRConvertIntRepArgDecl(FILE *file, argument_t *arg)
{
  WriteNDRConvertArgDecl(file, arg, "int_rep", "Request");
}

void
WriteRequestNDRConvertCharRepArgDecl(FILE *file, argument_t *arg)
{
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) != akeCountInOut)
    WriteNDRConvertArgDecl(file, arg, "char_rep", "Request");
}

void
WriteRequestNDRConvertFloatRepArgDecl(FILE *file, argument_t *arg)
{
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) != akeCountInOut)
    WriteNDRConvertArgDecl(file, arg, "float_rep", "Request");
}

void
WriteRequestNDRConvertArgUse(FILE *file, argument_t *arg, char *convert)
{
  routine_t *rt = arg->argRoutine;
  argument_t *count = arg->argCount;
  char argname[MAX_STR_LEN];

  if ((akIdent(arg->argKind) == akeCount || akIdent(arg->argKind) == akeCountInOut) &&
      (arg->argParent && akCheck(arg->argParent->argKind, akbSendNdr)))
    return;

  if (arg->argKPD_Type == MACH_MSG_OOL_DESCRIPTOR) {
    if (count && !arg->argSameCount && !strcmp(convert, "int_rep")) {
      fprintf(file, "#if defined(__NDR_convert__int_rep__Request__%s_t__%s__defined)\n", rt->rtName, count->argMsgField);
      fprintf(file, "\t\t__NDR_convert__int_rep__Request__%s_t__%s(&In%dP->%s, In%dP->NDR.int_rep);\n", rt->rtName, count->argMsgField, count->argRequestPos, count->argMsgField, count->argRequestPos);
      fprintf(file, "#endif\t/* __NDR_convert__int_rep__Request__%s_t__%s__defined */\n", rt->rtName, count->argMsgField);
    }

    sprintf(argname, "(%s)(In%dP->%s.address)", FetchServerType(arg->argType), arg->argRequestPos, arg->argMsgField);
  }
  else {
    sprintf(argname, "&In%dP->%s", arg->argRequestPos, arg->argMsgField);
  }

  fprintf(file, "#if defined(__NDR_convert__%s__Request__%s_t__%s__defined)\n", convert, rt->rtName, arg->argMsgField);
  fprintf(file, "\t\t__NDR_convert__%s__Request__%s_t__%s(%s, In0P->NDR.%s", convert, rt->rtName, arg->argMsgField, argname, convert);
  if (count)
    fprintf(file, ", In%dP->%s", count->argRequestPos, count->argMsgField);
  fprintf(file, ");\n");
  fprintf(file, "#endif\t/* __NDR_convert__%s__Request__%s_t__%s__defined */\n", convert, rt->rtName, arg->argMsgField);
}

void
WriteRequestNDRConvertIntRepOneArgUse(FILE *file, argument_t *arg)
{
  routine_t *rt = arg->argRoutine;

  fprintf(file, "#if defined(__NDR_convert__int_rep__Request__%s_t__%s__defined)\n", rt->rtName, arg->argMsgField);
  fprintf(file, "\tif (In0P->NDR.int_rep != NDR_record.int_rep)\n");
  fprintf(file, "\t\t__NDR_convert__int_rep__Request__%s_t__%s(&In%dP->%s, In%dP->NDR.int_rep);\n", rt->rtName, arg->argMsgField, arg->argRequestPos, arg->argMsgField, arg->argRequestPos);
  fprintf(file, "#endif\t/* __NDR_convert__int_rep__Request__%s_t__%s__defined */\n", rt->rtName, arg->argMsgField);
}

void
WriteRequestNDRConvertIntRepArgUse(FILE *file, argument_t *arg)
{
  WriteRequestNDRConvertArgUse(file, arg, "int_rep");
}

void
WriteRequestNDRConvertCharRepArgUse(FILE *file, argument_t *arg)
{
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) != akeCountInOut)
    WriteRequestNDRConvertArgUse(file, arg, "char_rep");
}

void
WriteRequestNDRConvertFloatRepArgUse(FILE *file, argument_t *arg)
{
  if (akIdent(arg->argKind) != akeCount && akIdent(arg->argKind) != akeCountInOut)
    WriteRequestNDRConvertArgUse(file, arg, "float_rep");
}

static void
WriteCalcArgSize(FILE *file, register argument_t *arg)
{
  register ipc_type_t *ptype = arg->argType;

  if (PackMsg == FALSE) {
    fprintf(file, "%d", ptype->itTypeSize + ptype->itPadSize);
    return;
  }

  if (IS_OPTIONAL_NATIVE(ptype))
    fprintf(file, "(In%dP->__Present__%s ? _WALIGNSZ_(%s) : 0)" , arg->argRequestPos, arg->argMsgField, ptype->itServerType);
  else {
    register ipc_type_t *btype = ptype->itElement;
    argument_t *count = arg->argCount;
    int multiplier = btype->itTypeSize;

    if (btype->itTypeSize % itWordAlign != 0)
      fprintf(file, "_WALIGN_");
    fprintf(file, "(");

    if (multiplier > 1)
      fprintf(file, "%d * ", multiplier);
    fprintf(file, "In%dP->%s", count->argRequestPos, count->argMsgField);
    fprintf(file, ")");
  }
}

static void
WriteCheckArgSize(FILE *file, routine_t *rt, argument_t *arg, const char *comparator)
{
  register ipc_type_t *ptype = arg->argType;


  fprintf(file, "\tif (((msgh_size - ");
  rtMinRequestSize(file, rt, "__Request");
  fprintf(file, ") ");
  if (PackMsg == FALSE) {
	  fprintf(file, "%s %d)", comparator, ptype->itTypeSize + ptype->itPadSize);
  } else if (IS_OPTIONAL_NATIVE(ptype)) {
	  fprintf(file, "%s (In%dP->__Present__%s ? _WALIGNSZ_(%s) : 0))" , comparator, arg->argRequestPos, arg->argMsgField, ptype->itServerType);
  } else {
    register ipc_type_t *btype = ptype->itElement;
    argument_t *count = arg->argCount;
    int multiplier = btype->itTypeSize;

    if (multiplier > 1)
      fprintf(file, "/ %d ", multiplier);
    fprintf(file, "< In%dP->%s) ||\n", count->argRequestPos, count->argMsgField);
    fprintf(file, "\t    (msgh_size %s ", comparator);
    rtMinRequestSize(file, rt, "__Request");
    fprintf(file, " + ");
    WriteCalcArgSize(file, arg);
    fprintf(file, ")");
  }
  fprintf(file, ")\n\t\treturn MIG_BAD_ARGUMENTS;\n");
}

static void
WriteCheckMsgSize(FILE *file, register argument_t *arg)
{
  register routine_t *rt = arg->argRoutine;

  if (arg->argCount && !arg->argSameCount)
    WriteRequestNDRConvertIntRepOneArgUse(file, arg->argCount);
  if (arg->argRequestPos == rt->rtMaxRequestPos)  {
    fprintf(file, "#if\t__MigTypeCheck\n");

    /* verify that the user-code-provided count does not exceed the maximum count allowed by the type. */
    fprintf(file, "\t" "if ( In%dP->%s > %d )\n", arg->argCount->argRequestPos, arg->argCount->argMsgField, arg->argType->itNumber);
    fputs("\t\t" "return MIG_BAD_ARGUMENTS;\n", file);
    /* ...end... */

    WriteCheckArgSize(file, rt, arg, "!=");

    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  }
  else {
    /* If there aren't any more variable-sized arguments after this,
       then we must check for exact msg-size and we don't need to
       update msgh_size. */

    boolean_t LastVarArg = arg->argRequestPos+1 == rt->rtNumRequestVar;

    /* calculate the actual size in bytes of the data field.  note
       that this quantity must be a multiple of four.  hence, if
       the base type size isn't a multiple of four, we have to
       round up.  note also that btype->itNumber must
       divide btype->itTypeSize (see itCalculateSizeInfo). */

    fprintf(file, "\tmsgh_size_delta = ");
    WriteCalcArgSize(file, arg);
    fprintf(file, ";\n");
    fprintf(file, "#if\t__MigTypeCheck\n");

    /* verify that the user-code-provided count does not exceed the maximum count allowed by the type. */
    fprintf(file, "\t" "if ( In%dP->%s > %d )\n", arg->argCount->argRequestPos, arg->argCount->argMsgField, arg->argType->itNumber);
    fputs("\t\t" "return MIG_BAD_ARGUMENTS;\n", file);
    /* ...end... */

    /* Don't decrement msgh_size until we've checked that
       it won't underflow. */
    WriteCheckArgSize(file, rt, arg, LastVarArg ? "!=" : "<");

    if (!LastVarArg)
      fprintf(file, "\tmsgh_size -= msgh_size_delta;\n");

    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  }
  fprintf(file, "\n");
}

static char *
InArgMsgField(register argument_t *arg,  char *str)
{
  static char buffer[MAX_STR_LEN];
  char who[20] = {0};

  /*
   * Inside the kernel, the request and reply port fields
   * really hold ipc_port_t values, not mach_port_t values.
   * Hence we must cast the values.
   */

  if (!(arg->argFlags & flRetCode)) {
    if (akCheck(arg->argKind, akbServerImplicit))
      sprintf(who, "TrailerP->");
    else
      sprintf(who, "In%dP->", arg->argRequestPos);
  }

#ifdef MIG_KERNEL_PORT_CONVERSION
  if (IsKernelServer &&
      ((akIdent(arg->argKind) == akeRequestPort) ||
       (akIdent(arg->argKind) == akeReplyPort)))
    sprintf(buffer, "(ipc_port_t) %s%s%s", who, str, (arg->argSuffix != strNULL) ? arg->argSuffix : arg->argMsgField);
  else
#endif
    sprintf(buffer, "%s%s%s", who, str, (arg->argSuffix != strNULL) ? arg->argSuffix : arg->argMsgField);

  return buffer;
}

static void
WriteExtractArgValue(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  string_t recast;

#ifdef MIG_KERNEL_PORT_CONVERSION
  if (IsKernelServer && it->itPortType && streql(it->itServerType, "ipc_port_t")
      && akIdent(arg->argKind) != akeRequestPort
      && akIdent(arg->argKind) != akeReplyPort)
    recast = "(mach_port_t)";
  else
#endif
    recast = "";
  if (it->itInTrans != strNULL)
    WriteCopyType(file, it, "%s", "/* %s */ %s(%s%s)", arg->argVarName, it->itInTrans, recast, InArgMsgField(arg, ""));
  else
    WriteCopyType(file, it, "%s", "/* %s */ %s%s", arg->argVarName, recast, InArgMsgField(arg, ""));

  fprintf(file, "\n");
}

/*
 * argKPD_Extract discipline for Port types.
 */
static void
WriteExtractKPD_port(FILE *file,  register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char *recast = "";

  WriteKPD_Iterator(file, TRUE, it->itVarArray, arg, FALSE);
  /* translation function do not apply to complex types */
#ifdef MIG_KERNEL_PORT_CONVERSION
  if (IsKernelServer)
    recast = "(mach_port_t)";
#endif
  fprintf(file, "\t\t%s[i] = %sptr->name;\n", arg->argVarName, recast);
  fprintf(file, "\t}\n");
}

/*
 * argKPD_Extract discipline for out-of-line types.
 */
static void
WriteExtractKPD_ool(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;

  WriteKPD_Iterator(file, TRUE, it->itVarArray, arg, FALSE);
  fprintf(file, "\t\t%s[i] = ptr->address;\n", arg->argVarName);
  fprintf(file, "\t}\n");
}

/*
 * argKPD_Extract discipline for out-of-line Port types.
 */
static void
WriteExtractKPD_oolport(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;

  WriteKPD_Iterator(file, TRUE, it->itVarArray, arg, FALSE);
  fprintf(file, "\t\t%s[i] = ptr->address;\n", arg->argVarName);
  fprintf(file, "\t}\n");
  if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbSendRcv)) {
    register argument_t *poly = arg->argPoly;
    register char *pref = poly->argByReferenceServer ? "*" : "";

    fprintf(file, "\t%s%s = In%dP->%s[0].disposition;\n", pref, poly->argVarName, arg->argRequestPos, arg->argMsgField);
  }
}


static void
WriteInitializeCount(FILE *file, register argument_t *arg)
{
  register ipc_type_t *ptype = arg->argParent->argType;
  register ipc_type_t *btype = ptype->itElement;
  identifier_t newstr;

  /*
   * Initialize 'count' argument for variable-length inline OUT parameter
   * with maximum allowed number of elements.
   */

  if (akCheck(arg->argKind, akbVarNeeded))
    newstr = arg->argMsgField;
  else
    newstr = (identifier_t)strconcat("OutP->", arg->argMsgField);

  fprintf(file, "\t%s = ", newstr);
  if (IS_MULTIPLE_KPD(ptype))
    fprintf(file, "%d;\n", ptype->itKPD_Number);
  else
    fprintf(file, "%d;\n", btype->itNumber? ptype->itNumber/btype->itNumber : 0);

  /*
   * If the user passed in a count, then we use the minimum.
   * We can't let the user completely override our maximum,
   * or the user might convince the server to overwrite the buffer.
   */

  if (arg->argCInOut != argNULL) {
    char *msgfield = InArgMsgField(arg->argCInOut, "");

    fprintf(file, "\tif (%s < %s)\n", msgfield, newstr);
    fprintf(file, "\t\t%s = %s;\n", newstr, msgfield);
  }

  fprintf(file, "\n");
}

static void
WriteAdjustRequestMsgPtr(FILE *file, register argument_t *arg)
{
  register ipc_type_t *ptype = arg->argType;

  if (PackMsg == FALSE) {
    fprintf(file, "\t*In%dPP = In%dP = (__Request *) ((pointer_t) In%dP);\n\n", arg->argRequestPos+1, arg->argRequestPos+1, arg->argRequestPos);
    return;
  }

  fprintf(file, "\t*In%dPP = In%dP = (__Request *) ((pointer_t) In%dP + msgh_size_delta - ", arg->argRequestPos+1, arg->argRequestPos+1, arg->argRequestPos);
  if (IS_OPTIONAL_NATIVE(ptype))
    fprintf(file, "_WALIGNSZ_(%s)", ptype->itUserType);
  else
    fprintf(file, "%d", ptype->itTypeSize + ptype->itPadSize);
  fprintf(file, ");\n\n");
}

static void
WriteCheckRequestTrailerArgs(FILE *file, routine_t *rt)
{
  register argument_t *arg;

  if (rt->rtServerImpl)
    WriteCheckTrailerHead(file, rt, FALSE);

  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
    if (akCheck(arg->argKind, akbServerImplicit))
      WriteCheckTrailerSize(file, FALSE, arg);
  }
}

static void
WriteExtractArg(FILE *file, register argument_t *arg)
{
  if (akCheckAll(arg->argKind, akbSendRcv|akbVarNeeded)) {
    if (akCheck(arg->argKind, akbSendKPD))
      (*arg->argKPD_Extract)(file, arg);
    else
      WriteExtractArgValue(file, arg);
  }

  if ((akIdent(arg->argKind) == akeCount) &&
      akCheck(arg->argKind, akbReturnSnd)) {

    register ipc_type_t *ptype = arg->argParent->argType;
    /*
     * the count will be initialized to 0 in the case of
     * unbounded arrays (MigInLine = TRUE): this is because
     * the old interface used to pass to the target procedure
     * the maximum in-line size (it was 2048 bytes)
     */
    if (IS_VARIABLE_SIZED_UNTYPED(ptype) ||
        IS_MIG_INLINE_EMUL(ptype) ||
        (IS_MULTIPLE_KPD(ptype) && ptype->itVarArray))
      WriteInitializeCount(file, arg);
  }
}

static void
WriteServerCallArg(FILE *file, register argument_t *arg)
{
  ipc_type_t *it = arg->argType;
  boolean_t NeedClose = FALSE;
  string_t  at = (arg->argByReferenceServer ||
                  it->itNativePointer) ? "&" : "";
  string_t  star = (arg->argByReferenceServer) ? " *" : "";
  string_t  msgfield =
  (arg->argSuffix != strNULL) ? arg->argSuffix : arg->argMsgField;

  if ((it->itInTrans != strNULL) &&
      akCheck(arg->argKind, akbSendRcv) &&
      !akCheck(arg->argKind, akbVarNeeded)) {
    fprintf(file, "%s%s(", at, it->itInTrans);
    NeedClose = TRUE;
  }

  if (akCheckAll(arg->argKind, akbVarNeeded|akbServerArg))
    fprintf(file, "%s%s", at, arg->argVarName);
  else if (akCheckAll(arg->argKind, akbSendRcv|akbSendKPD)) {
    if (!it->itInLine)
      /* recast the void *, although it is not necessary */
      fprintf(file, "(%s%s)%s(%s)", it->itTransType, star, at, InArgMsgField(arg, ""));
    else
#ifdef MIG_KERNEL_PORT_CONVERSION
      if (IsKernelServer && streql(it->itServerType, "ipc_port_t"))
        /* recast the port to the kernel internal form value */
        fprintf(file, "(ipc_port_t%s)%s(%s)", star, at, InArgMsgField(arg, ""));
      else
#endif
        fprintf(file, "%s%s", at, InArgMsgField(arg, ""));
  }
  else if (akCheck(arg->argKind, akbSendRcv)) {
    if (IS_OPTIONAL_NATIVE(it)) {
      fprintf(file, "(%s ? ", InArgMsgField(arg, "__Present__"));
      fprintf(file, "%s%s.__Real__%s : %s)", at, InArgMsgField(arg, ""), arg->argMsgField, it->itBadValue);
    }
    else
      fprintf(file, "%s%s", at, InArgMsgField(arg, ""));
  }
  else if (akCheckAll(arg->argKind, akbReturnSnd|akbReturnKPD)) {
    if (!it->itInLine)
      /* recast the void *, although it is not necessary */
      fprintf(file, "(%s%s)%s(OutP->%s)", it->itTransType, star, at, msgfield);
    else
#ifdef MIG_KERNEL_PORT_CONVERSION
      if (IsKernelServer && streql(it->itServerType, "ipc_port_t"))
      /* recast the port to the kernel internal form value */
        fprintf(file, "(mach_port_t%s)%s(OutP->%s)", star, at, msgfield);
      else
#endif
        fprintf(file, "%sOutP->%s", at, msgfield);

  }
  else  if (akCheck(arg->argKind, akbReturnSnd))
    fprintf(file, "%sOutP->%s", at, msgfield);

  if (NeedClose)
    fprintf(file, ")");
}

/*
 * Shrunk version of WriteServerCallArg, to implement the RetCode functionality:
 * we have received a mig_reply_error_t, therefore we want to call the target
 * routine with all 0s except for the error code (and the implicit data).
 * We know that we are a SimpleRoutine.
 */
static void
WriteConditionalCallArg(FILE *file, register argument_t *arg)
{
  ipc_type_t *it = arg->argType;
  boolean_t NeedClose = FALSE;

  if ((it->itInTrans != strNULL) &&
      akCheck(arg->argKind, akbSendRcv) &&
      !akCheck(arg->argKind, akbVarNeeded)) {
    fprintf(file, "%s(", it->itInTrans);
    NeedClose = TRUE;
  }

  if (akCheck(arg->argKind, akbSendRcv)) {
    if (akIdent(arg->argKind) == akeRequestPort ||
        akCheck(arg->argKind, akbServerImplicit))
      fprintf(file, "%s", InArgMsgField(arg, ""));
    else if (akIdent(arg->argKind) == akeRetCode)
      fprintf(file, "((mig_reply_error_t *)In0P)->RetCode");
    else
      fprintf(file, "(%s)(0)", it->itTransType);
  }

  if (NeedClose)
    fprintf(file, ")");
}

static void
WriteDestroyArg(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;

  /*
   * Deallocate IN/INOUT out-of-line args if specified by "auto" flag.
   *
   * We also have to deallocate in the cases where the target routine
   * is given a itInLine semantic whereas the underlying transmission
   * was out-of-line
   */
  if ((argIsIn(arg) && akCheck(arg->argKind, akbSendKPD|akbReturnKPD) &&
       arg->argKPD_Type == MACH_MSG_OOL_DESCRIPTOR &&
       (arg->argFlags & flAuto))
      ||
      IS_MIG_INLINE_EMUL(it)
      ) {
    /*
     * Deallocate only if out-of-line.
     */
    argument_t *count = arg->argCount;
    ipc_type_t *btype = it->itElement;
    int multiplier = btype->itNumber ? btype->itSize / (8 * btype->itNumber) : 0;

    if (IsKernelServer) {
      fprintf(file, "#if __MigKernelSpecificCode\n");
      fprintf(file, "\tvm_map_copy_discard(%s);\n", InArgMsgField(arg, ""));
      fprintf(file, "#else\n");
    }
    fprintf(file, "\tmig_deallocate((vm_offset_t) %s, ", InArgMsgField(arg, ""));
    if (it->itVarArray) {
      if (multiplier > 1)
        fprintf(file, "%d * ", multiplier);
      fprintf(file, "%s);\n", InArgMsgField(count, ""));
    }
    else
      fprintf(file, "%d);\n", (it->itNumber * it->itSize + 7) / 8);
    if (IsKernelServer) {
      fprintf(file, "#endif /* __MigKernelSpecificCode */\n");
    }
    fprintf(file, "\t%s = (void *) 0;\n", InArgMsgField(arg, ""));
    fprintf(file, "\tIn%dP->%s.%s = (mach_msg_size_t) 0;\n", arg->argRequestPos, arg->argMsgField, (RPCPortArray(arg) ? "count" : "size"));
  }
  else {
    if (akCheck(arg->argKind, akbVarNeeded))
      fprintf(file, "\t%s(%s);\n", it->itDestructor, arg->argVarName);
    else
      fprintf(file, "\t%s(%s);\n", it->itDestructor, InArgMsgField(arg, ""));
  }
}

static void
WriteDestroyPortArg(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;

  /*
   * If a translated port argument occurs in the body of a request
   * message, and the message is successfully processed, then the
   * port right should be deallocated.  However, the called function
   * didn't see the port right; it saw the translation.  So we have
   * to release the port right for it.
   *
   *  The test over it->itInTrans will exclude any complex type
   *  made out of ports
   */
  if ((it->itInTrans != strNULL) &&
      (it->itOutName == MACH_MSG_TYPE_PORT_SEND)) {
    fprintf(file, "\n");
    fprintf(file, "\tif (IP_VALID((ipc_port_t)%s))\n", InArgMsgField(arg, ""));
    fprintf(file, "\t\tipc_port_release_send((ipc_port_t)%s);\n", InArgMsgField(arg, ""));
  }
}

/*
 * Check whether WriteDestroyPortArg would generate any code for arg.
 */
boolean_t
CheckDestroyPortArg(register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;

  if ((it->itInTrans != strNULL) &&
      (it->itOutName == MACH_MSG_TYPE_PORT_SEND))  {
    return TRUE;
  }
  return FALSE;
}

static void
WriteServerCall(FILE *file, routine_t *rt,  void (*func)())
{
  argument_t *arg = rt->rtRetCode;
  ipc_type_t *it = arg->argType;
  boolean_t NeedClose = FALSE;

  fprintf(file, "\t");
  if (akCheck(arg->argKind, akbVarNeeded))
    fprintf(file, "%s = ", arg->argMsgField);
  else
    fprintf(file, "OutP->%s = ", arg->argMsgField);
  if (it->itOutTrans != strNULL) {
    fprintf(file, "%s(", it->itOutTrans);
    NeedClose = TRUE;
  }
  fprintf(file, "%s(", rt->rtServerName);
  WriteList(file, rt->rtArgs, func, akbServerArg, ", ", "");
  if (NeedClose)
    fprintf(file, ")");
  fprintf(file, ");\n");
}

static void
WriteCheckReturnValue(FILE *file, register routine_t *rt)
{
  argument_t *arg = rt->rtRetCode;
  char string[MAX_STR_LEN];

  if (akCheck(arg->argKind, akbVarNeeded))
    sprintf(string, "%s", arg->argMsgField);
  else
    sprintf(string, "OutP->%s", arg->argMsgField);
  fprintf(file, "\tif (%s != KERN_SUCCESS) {\n", string);
  fprintf(file, "\t\tMIG_RETURN_ERROR(OutP, %s);\n", string);
  fprintf(file, "\t}\n");
}

/*
 * WriteInitKPD_port, WriteInitKPD_ool, WriteInitKPD_oolport
 * initializes the OutP KPD fields (this job cannot be done once
 * the target routine has been called, otherwise informations
 * would be lost)
 */
/*
 * argKPD_Init discipline for Port types.
 */
static void
WriteInitKPD_port(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char *subindex = "";
  boolean_t close = FALSE;
  char firststring[MAX_STR_LEN];
  char string[MAX_STR_LEN];

  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, FALSE, arg, TRUE);
    (void)sprintf(firststring, "\t*ptr");
    (void)sprintf(string, "\tptr->");
    subindex = "[i]";
    close = TRUE;
  }
  else {
    (void)sprintf(firststring, "OutP->%s", arg->argMsgField);
    (void)sprintf(string, "OutP->%s.", arg->argMsgField);
  }

  fprintf(file, "#if\tUseStaticTemplates\n");
  fprintf(file, "\t%s = %s;\n", firststring, arg->argTTName);
  fprintf(file, "#else\t/* UseStaticTemplates */\n");
  if (IS_MULTIPLE_KPD(it) && it->itVarArray)
    fprintf(file, "\t%sname = MACH_PORT_NULL;\n", string);
  if (arg->argPoly == argNULL) {
    if (IsKernelServer) {
      fprintf(file, "#if __MigKernelSpecificCode\n");
      fprintf(file, "\t%sdisposition = %s;\n", string, it->itOutNameStr);
      fprintf(file, "#else\n");
    }
    fprintf(file, "\t%sdisposition = %s;\n", string, it->itInNameStr);
    if (IsKernelServer)
      fprintf(file, "#endif /* __MigKernelSpecificCode */\n");
  }
  fprintf(file, "\t%stype = MACH_MSG_PORT_DESCRIPTOR;\n", string);
  fprintf(file, "#endif\t/* UseStaticTemplates */\n");
  if (close)
    fprintf(file, "\t    }\n\t}\n");
  fprintf(file, "\n");
}

/*
 * argKPD_Init discipline for out-of-line types.
 */
static void
WriteInitKPD_ool(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char firststring[MAX_STR_LEN];
  char string[MAX_STR_LEN];
  boolean_t VarArray;
  u_int howmany, howbig;

  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, FALSE, arg, TRUE);
    (void)sprintf(firststring, "\t*ptr");
    (void)sprintf(string, "\tptr->");
    VarArray = it->itElement->itVarArray;
    howmany = it->itElement->itNumber;
    howbig = it->itElement->itSize;
  }
  else {
    (void)sprintf(firststring, "OutP->%s", arg->argMsgField);
    (void)sprintf(string, "OutP->%s.", arg->argMsgField);
    VarArray = it->itVarArray;
    howmany = it->itNumber;
    howbig = it->itSize;
  }

  fprintf(file, "#if\tUseStaticTemplates\n");
  fprintf(file, "\t%s = %s;\n", firststring, arg->argTTName);
  fprintf(file, "#else\t/* UseStaticTemplates */\n");
  if (!VarArray)
    fprintf(file, "\t%ssize = %d;\n", string, (howmany * howbig + 7)/8);
  if (arg->argDeallocate != d_MAYBE)
    fprintf(file, "\t%sdeallocate =  %s;\n", string, (arg->argDeallocate == d_YES) ? "TRUE" : "FALSE");
  fprintf(file, "\t%scopy = %s;\n", string, (arg->argFlags & flPhysicalCopy) ? "MACH_MSG_PHYSICAL_COPY" : "MACH_MSG_VIRTUAL_COPY");
#ifdef ALIGNMENT
  fprintf(file, "\t%salignment = MACH_MSG_ALIGN_%d;\n", string, arg->argMsgField, (howbig < 8) ? 1 : howbig / 8);
#endif
  fprintf(file, "\t%stype = MACH_MSG_OOL_DESCRIPTOR;\n", string);
  fprintf(file, "#endif\t/* UseStaticTemplates */\n");

  if (IS_MULTIPLE_KPD(it))
    fprintf(file, "\t    }\n\t}\n");
  fprintf(file, "\n");
}

/*
 * argKPD_Init discipline for out-of-line Port types.
 */
static void
WriteInitKPD_oolport(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  boolean_t VarArray;
  ipc_type_t *howit;
  u_int howmany;
  char firststring[MAX_STR_LEN];
  char string[MAX_STR_LEN];

  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, FALSE, arg, TRUE);
    (void)sprintf(firststring, "\t*ptr");
    (void)sprintf(string, "\tptr->");
    VarArray = it->itElement->itVarArray;
    howmany = it->itElement->itNumber;
    howit = it->itElement;
  }
  else {
    (void)sprintf(firststring, "OutP->%s", arg->argMsgField);
    (void)sprintf(string, "OutP->%s.", arg->argMsgField);
    VarArray = it->itVarArray;
    howmany = it->itNumber;
    howit = it;
  }

  fprintf(file, "#if\tUseStaticTemplates\n");
  fprintf(file, "\t%s = %s;\n", firststring, arg->argTTName);
  fprintf(file, "#else\t/* UseStaticTemplates */\n");

  if (!VarArray)
    fprintf(file, "\t%scount = %d;\n", string, howmany);
  if (arg->argPoly == argNULL) {
    if (IsKernelServer) {
      fprintf(file, "#if\t__MigKernelSpecificCode\n");
      fprintf(file, "\t%sdisposition = %s;\n", string, howit->itOutNameStr);
      fprintf(file, "#else\n");
    }
    fprintf(file, "\t%sdisposition = %s;\n", string, howit->itInNameStr);
    if (IsKernelServer)
      fprintf(file, "#endif /* __MigKernelSpecificCode */\n");
  }
  if (arg->argDeallocate != d_MAYBE)
    fprintf(file, "\t%sdeallocate =  %s;\n", string, (arg->argDeallocate == d_YES) ? "TRUE" : "FALSE");
  fprintf(file, "\t%stype = MACH_MSG_OOL_PORTS_DESCRIPTOR;\n", string);
  fprintf(file, "#endif\t/* UseStaticTemplates */\n");

  if (IS_MULTIPLE_KPD(it))
    fprintf(file, "\t    }\n\t}\n");
  fprintf(file, "\n");
}

static void
WriteInitKPDValue(FILE *file, register argument_t *arg)
{
  (*arg->argKPD_Init)(file, arg);
}

static void
WriteAdjustMsgCircular(FILE *file, register argument_t *arg)
{
  fprintf(file, "\n");

  fprintf(file,"#if\t__MigKernelSpecificCode\n");
  if (arg->argType->itOutName == MACH_MSG_TYPE_POLYMORPHIC)
    fprintf(file, "\tif (%s == MACH_MSG_TYPE_PORT_RECEIVE)\n", arg->argPoly->argVarName);

  /*
   * The carried port right can be accessed in OutP->XXXX.  Normally
   * the server function stuffs it directly there.  If it is InOut,
   * then it has already been copied into the reply message.
   * If the server function deposited it into a variable (perhaps
   * because the reply message is variable-sized) then it has already
   * been copied into the reply message.
   *
   *  The old MiG does not check for circularity in the case of
   *  array of ports. So do I ...
   */

  fprintf(file, "\t  if (IP_VALID((ipc_port_t) In0P->Head.msgh_reply_port) &&\n");
  fprintf(file, "\t    IP_VALID((ipc_port_t) OutP->%s.name) &&\n", arg->argMsgField);
  fprintf(file, "\t    ipc_port_check_circularity((ipc_port_t) OutP->%s.name, (ipc_port_t) In0P->Head.msgh_reply_port))\n", arg->argMsgField);
  fprintf(file, "\t\tOutP->Head.msgh_bits |= MACH_MSGH_BITS_CIRCULAR;\n");
  fprintf(file, "#endif /* __MigKernelSpecificCode */\n");
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
  boolean_t close = FALSE;
  char string[MAX_STR_LEN];
  ipc_type_t *real_it;

  if (akCheck(arg->argKind, akbVarNeeded)) {
    if (IS_MULTIPLE_KPD(it)) {
      WriteKPD_Iterator(file, FALSE, it->itVarArray, arg, TRUE);
      (void)sprintf(string, "\tptr->");
      subindex = "[i]";
      close = TRUE;
      real_it = it->itElement;
    }
    else {
      (void)sprintf(string, "OutP->%s.", arg->argMsgField);
      real_it = it;
    }
#ifdef MIG_KERNEL_PORT_CONVERSIONS
    if (IsKernelServer && streql(real_it->itTransType, "ipc_port_t"))
      recast = "(mach_port_t)";
#endif

    if (it->itOutTrans != strNULL && !close)
      fprintf(file, "\t%sname = (mach_port_t)%s(%s);\n", string, it->itOutTrans, arg->argVarName);
    else
      fprintf(file, "\t%sname = %s%s%s;\n", string, recast, arg->argVarName, subindex);
    if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbReturnSnd)) {
      register argument_t *poly = arg->argPoly;

      if  (akCheck(arg->argPoly->argKind, akbVarNeeded))
        fprintf(file, "\t%sdisposition = %s;\n", string, poly->argVarName);
      else if (close)
        fprintf(file, "\t%sdisposition = OutP->%s;\n", string, poly->argSuffix);
    }
    if (close)
      fprintf(file, "\t    }\n\t}\n");
    fprintf(file, "\n");
  }
  else  if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbReturnSnd|akbVarNeeded))
    fprintf(file, "\tOutP->%s.disposition = %s;\n", arg->argMsgField, arg->argPoly->argVarName);
  /*
   * If this is a KernelServer, and the reply message contains
   * a receive right, we must check for the possibility of a
   * port/message circularity.  If queueing the reply message
   * would cause a circularity, we mark the reply message
   * with the circular bit.
   */
  if (IsKernelServer && !(IS_MULTIPLE_KPD(it)) &&
      ((arg->argType->itOutName == MACH_MSG_TYPE_PORT_RECEIVE) ||
       (arg->argType->itOutName == MACH_MSG_TYPE_POLYMORPHIC)))
    WriteAdjustMsgCircular(file, arg);
}

/*
 * argKPD_Pack discipline for out-of-line types.
 */
static void
WriteKPD_ool(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;
  char string[MAX_STR_LEN];
  boolean_t VarArray;
  argument_t *count;
  u_int howbig;
  char *subindex;

  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, it->itVarArray, arg, TRUE);
    (void)sprintf(string, "\tptr->");
    VarArray = it->itElement->itVarArray;
    count = arg->argSubCount;
    howbig = it->itElement->itSize;
    subindex = "[i]";
  }
  else {
    (void)sprintf(string, "OutP->%s.", arg->argMsgField);
    VarArray = it->itVarArray;
    count = arg->argCount;
    howbig = it->itSize;
    subindex = "";
  }

  if (akCheck(arg->argKind, akbVarNeeded))
    fprintf(file, "\t%saddress = (void *)%s%s;\n", string, arg->argMsgField, subindex);
  if (arg->argDealloc != argNULL)
    if (akCheck(arg->argDealloc->argKind, akbVarNeeded) || IS_MULTIPLE_KPD(it))
      fprintf(file, "\t%sdeallocate = %s;\n", string, arg->argDealloc->argVarName);
  if (VarArray) {
    fprintf(file, "\t%ssize = ", string);
    if (akCheck(count->argKind, akbVarNeeded))
      fprintf(file, "%s%s", count->argName, subindex);
    else
      fprintf(file, "OutP->%s%s", count->argMsgField, subindex);

    if (count->argMultiplier > 1 || howbig > 8)
      fprintf(file, " * %d;\n", count->argMultiplier * howbig / 8);
    else
      fprintf(file, ";\n");
  }

  if (IS_MULTIPLE_KPD(it)) {
    fprintf(file, "\t    }\n");
    if (it->itVarArray && !it->itElement->itVarArray) {
      fprintf(file, "\t    for (i = j; i < %d; ptr++, i++)\n", it->itKPD_Number);
      /* since subordinate arrays aren't variable, they are initialized from template:
       here we must no-op 'em */
      fprintf(file, "\t\tptr->size = 0;\n");
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
  boolean_t VarArray;
  argument_t *count;
  char *subindex, string[MAX_STR_LEN];

  if (IS_MULTIPLE_KPD(it)) {
    WriteKPD_Iterator(file, FALSE, it->itVarArray, arg, TRUE);
    (void)sprintf(string, "\tptr->");
    VarArray = it->itElement->itVarArray;
    count = arg->argSubCount;
    subindex = "[i]";
  }
  else {
    (void)sprintf(string, "OutP->%s.", arg->argMsgField);
    VarArray = it->itVarArray;
    count = arg->argCount;
    subindex = "";
  }

  if (akCheck(arg->argKind, akbVarNeeded))
    fprintf(file, "\t%saddress = (void *)%s%s;\n", string, arg->argMsgField, subindex);
  if (arg->argDealloc != argNULL)
    if (akCheck(arg->argDealloc->argKind, akbVarNeeded) || IS_MULTIPLE_KPD(it))
      fprintf(file, "\t%sdeallocate = %s;\n", string, arg->argDealloc->argVarName);
  if (VarArray) {
    fprintf(file, "\t%scount = ", string);
    if (akCheck(count->argKind, akbVarNeeded))
      fprintf(file, "%s%s;\n", count->argName, subindex);
    else
      fprintf(file, "OutP->%s%s;\n", count->argMsgField, subindex);
  }
  if (arg->argPoly != argNULL && akCheckAll(arg->argPoly->argKind, akbReturnSnd))
    if (akCheck(arg->argPoly->argKind, akbVarNeeded) || IS_MULTIPLE_KPD(it))
      fprintf(file, "\t%sdisposition = %s;\n", string, arg->argPoly->argVarName);
  if (IS_MULTIPLE_KPD(it)) {
    fprintf(file, "\t    }\n");
    if (it->itVarArray && !it->itElement->itVarArray) {
      fprintf(file, "\t    for (i = j; i < %d; ptr++, i++)\n", it->itKPD_Number);
      /* since subordinate arrays aren't variable, they are initialized from template:
       here we must no-op 'em */
      fprintf(file, "\t%scount = 0;\n", string);
    }
    fprintf(file, "\t}\n");
  }
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
    WriteKPD_Iterator(file, TRUE, FALSE, arg, TRUE);
    (void)sprintf(string, "ptr->");
    tab = "\t";
    close = TRUE;
  }
  else
    (void)sprintf(string, "In%dP->%s.", arg->argRequestPos, arg->argMsgField);

  fprintf(file, "\t%sif (%stype != MACH_MSG_PORT_DESCRIPTOR", tab, string);
  /*
   * We can't check disposition on varArray
   * (because some of the entries could be empty).
   */
  if (!it->itVarArray) {
    if (arg->argPoly != argNULL) {
      switch (it->itOutName) {

        case MACH_MSG_TYPE_MOVE_RECEIVE:
          fprintf(file, " || \n\t%s    %sdisposition != MACH_MSG_TYPE_MOVE_RECEIVE", tab, string);
          break;

        case MACH_MSG_TYPE_MOVE_SEND_ONCE:
          fprintf(file, " || (\n\t%s    %sdisposition != MACH_MSG_TYPE_MOVE_SEND_ONCE", tab, string);
          fprintf(file, " &&  \n\t%s    %sdisposition != MACH_MSG_TYPE_MAKE_SEND_ONCE)", tab, string);
          break;

        case MACH_MSG_TYPE_MOVE_SEND:
          fprintf(file, " || (\n\t%s    %sdisposition != MACH_MSG_TYPE_MOVE_SEND", tab, string);
          fprintf(file, " &&  \n\t%s    %sdisposition != MACH_MSG_TYPE_MAKE_SEND", tab, string);
          fprintf(file, " &&  \n\t%s    %sdisposition != MACH_MSG_TYPE_COPY_SEND)", tab, string);
          break;
      }
    }
    else {
      fprintf(file, " ||\n\t%s    %sdisposition != %s", tab, string, it->itOutNameStr);
    }
  }
  fprintf(file, ")\n");
  fprintf(file, "\t\treturn MIG_TYPE_ERROR;\n");
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
    WriteKPD_Iterator(file, TRUE, FALSE, arg, TRUE);
    tab = "\t\t\t";
    sprintf(string, "ptr->");
    howmany = it->itElement->itNumber;
    howbig = it->itElement->itSize;
    test = !it->itVarArray && !it->itElement->itVarArray;
  }
  else {
    tab = "";
    sprintf(string, "In%dP->%s.", arg->argRequestPos, arg->argMsgField);
    howmany = it->itNumber;
    howbig = it->itSize;
    test = !it->itVarArray;
  }

  fprintf(file, "\t%sif (%stype != MACH_MSG_OOL_DESCRIPTOR", tab, string);
  if (test) {
    /* if VarArray we may use no-op; if itElement->itVarArray size might change */
    fprintf(file, " ||\n\t%s    %ssize != %d", tab, string, (howmany * howbig + 7)/8);
  }
  
  fprintf(file, ")\n");
  fprintf(file, "\t\t%s" "return MIG_TYPE_ERROR;\n", tab);
  
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
    WriteKPD_Iterator(file, TRUE, FALSE, arg, TRUE);
    tab = "\t";
    sprintf(string, "ptr->");
    howmany = it->itElement->itNumber;
    test = !it->itVarArray && !it->itElement->itVarArray;
    howstr = it->itElement->itOutNameStr;
  }
  else {
    tab = "";
    sprintf(string, "In%dP->%s.", arg->argRequestPos, arg->argMsgField);
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
  fprintf(file, ")\n");
  fprintf(file, "\t\treturn MIG_TYPE_ERROR;\n");

  if (IS_MULTIPLE_KPD(it))
    fprintf(file, "\t    }\n\t}\n");
}

/*************************************************************
 *  Writes code to check that the type of each of the arguments
 *  in the reply message is what is expected. Called by
 *  WriteRoutine for each in && typed argument in the request message.
 *************************************************************/
static void
WriteTypeCheck(FILE *file, register argument_t *arg)
{
  fprintf(file, "#if\t__MigTypeCheck\n");
  (*arg->argKPD_TypeCheck)(file, arg);
  fprintf(file, "#endif\t/* __MigTypeCheck */\n");
}

static void
WritePackArgValueNormal(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;

  if (IS_VARIABLE_SIZED_UNTYPED(it) || it->itNoOptArray) {
    if (it->itString) {
      /*
       * Copy variable-size C string with mig_strncpy.
       * Save the string length (+ 1 for trailing 0)
       * in the argument`s count field.
       */
      fprintf(file, "\tOutP->%s = mig_strncpy(OutP->%s, %s, %d);\n", arg->argCount->argMsgField, arg->argMsgField, arg->argVarName, it->itNumber);
    }
    else if (it->itNoOptArray)
      fprintf(file, "\t(void)memcpy((char *) OutP->%s, (const char *) %s, %d);\n", arg->argMsgField, arg->argVarName, it->itTypeSize);
    else {
      register argument_t *count = arg->argCount;
      register ipc_type_t *btype = it->itElement;
      identifier_t newstr;

      /* Note btype->itNumber == count->argMultiplier */

      fprintf(file, "\t(void)memcpy((char *) OutP->%s, (const char *) %s, ", arg->argMsgField, arg->argVarName);
      if (btype->itTypeSize > 1)
        fprintf(file, "%d * ", btype->itTypeSize);
      /* count is a akbVarNeeded if arg is akbVarNeeded */
      if (akCheck(count->argKind, akbVarNeeded))
        newstr = count->argVarName;
      else
        newstr = (identifier_t)strconcat("OutP->", count->argMsgField);
      fprintf(file, "%s);\n", newstr);
    }
  }
  else if (it->itOutTrans != strNULL)
    WriteCopyType(file, it, "OutP->%s", "/* %s */ %s(%s)", arg->argMsgField, it->itOutTrans, arg->argVarName);
  else
    WriteCopyType(file, it, "OutP->%s", "/* %s */ %s", arg->argMsgField, arg->argVarName);
}

static void
WritePackArgValueVariable(FILE *file, register argument_t *arg)
{
  register ipc_type_t *it = arg->argType;

  /*
   * only itString are treated here so far
   */
  if (it->itString) {
    /*
     * Emit logic to call strlen to calculate the size of the argument, and ensure that it fits within the 32-bit result field
     * in the Reply, when targeting a 64-bit architecture. If a 32-bit architecture is the target, we emit code to just call
     * strlen() directly (since it'll return a 32-bit value that is guaranteed to fit).
     */
    fputs("#ifdef __LP64__\n", file);
    fprintf(file, "\t{\n"
                  "\t\t" "size_t strLength = strlen(OutP->%s) + 1;\n", arg->argMsgField);
    fputs(        "\t\t" "if (strLength > 0xffffffff)\n"
                  "\t\t\t"  "MIG_RETURN_ERROR(OutP, MIG_BAD_ARGUMENTS);\n", file);
    fprintf(file, "\t\t" "OutP->%s = (mach_msg_type_number_t) strLength;\n"
                  "\t}\n", arg->argCount->argMsgField);
    fputs("#else\n", file);
    fprintf(file, "\tOutP->%s = (mach_msg_type_number_t) strlen(OutP->%s) + 1;\n", arg->argCount->argMsgField, arg->argMsgField);
    fputs("#endif /* __LP64__ */\n", file);

  }
}

static void
WriteCopyArgValue(FILE *file, argument_t *arg)
{
  fprintf(file, "\n");
  WriteCopyType(file, arg->argType, "/* %d */ OutP->%s", "In%dP->%s", arg->argRequestPos, (arg->argSuffix != strNULL) ? arg->argSuffix : arg->argMsgField);
}

static void
WriteInitArgValue(FILE *file, argument_t *arg)
{
  fprintf(file, "\n");
  fprintf(file, "\tOutP->%s = %s;\n\n", arg->argMsgField, arg->argVarName);
}

/*
 * Calculate the size of a variable-length message field.
 */
static void
WriteArgSize(FILE *file, register argument_t *arg)
{
  register ipc_type_t *ptype = arg->argType;
  register int bsize = ptype->itElement->itTypeSize;
  register argument_t *count = arg->argCount;

  /* If the base type size of the data field isn`t a multiple of 4,
   we have to round up. */
  if (bsize % itWordAlign != 0)
    fprintf(file, "_WALIGN_");

  /* Here, we generate ((value + %d) & ~%d). We have to put two (( at the
   * the beginning.
   */
  fprintf(file, "((");
  if (bsize > 1)
    fprintf(file, "%d * ", bsize);
  if (ptype->itString || !akCheck(count->argKind, akbVarNeeded))
    /* get count from descriptor in message */
    fprintf(file, "OutP->%s", count->argMsgField);
  else
    /* get count from argument */
    fprintf(file, "%s", count->argVarName);

  /*
   * If the base type size is not a multiple of sizeof(natural_t),
   * we have to round up.
   */
  if (bsize % sizeof(natural_t) != 0)
    fprintf(file, " + %d) & ~%d)", (int)sizeof(natural_t)-1, (int)sizeof(natural_t)-1);
  else
    fprintf(file, "))");
}

/*
 * Adjust message size and advance reply pointer.
 * Called after packing a variable-length argument that
 * has more arguments following.
 */
static void
WriteAdjustMsgSize(FILE *file, register argument_t *arg)
{
  register routine_t *rt = arg->argRoutine;
  register ipc_type_t *ptype = arg->argType;

  /* There are more Out arguments.  We need to adjust msgh_size
     and advance OutP, so we save the size of the current field
     in msgh_size_delta. */

  fprintf(file, "\tmsgh_size_delta = ");
  WriteArgSize(file, arg);
  fprintf(file, ";\n");

  if (rt->rtNumReplyVar == 1) {
    /* We can still address the message header directly.  Fill
       in the size field. */

    fprintf(file, "\tOutP->Head.msgh_size = ");
    rtMinReplySize(file, rt, "Reply");
    fprintf(file, " + msgh_size_delta;\n");
  }
  else if (arg->argReplyPos == 0) {
    /* First variable-length argument.  The previous msgh_size value
       is the minimum reply size. */

    fprintf(file, "\tmsgh_size = ");
    rtMinReplySize(file, rt, "Reply");
    fprintf(file, " + msgh_size_delta;\n");
  }
  else
    fprintf(file, "\tmsgh_size += msgh_size_delta;\n");

  fprintf(file, "\tOutP = (Reply *) ((pointer_t) OutP + msgh_size_delta - %d);\n", ptype->itTypeSize + ptype->itPadSize);
}

/*
 * Calculate the size of the message.  Called after the
 * last argument has been packed.
 */
static void
WriteFinishMsgSize(FILE *file, register argument_t *arg)
{
  /* No more Out arguments.  If this is the only variable Out
     argument, we can assign to msgh_size directly. */

  if (arg->argReplyPos == 0) {
    fprintf(file, "\tOutP->Head.msgh_size = ");
    rtMinReplySize(file, arg->argRoutine, "Reply");
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

/*
 * Handle reply arguments - fill in message types and copy arguments
 * that need to be copied.
 */
static void
WriteReplyArgs(FILE *file, register routine_t *rt)
{
  register argument_t *arg;
  register argument_t *lastVarArg;

  /*
   * 1. The Kernel Processed Data
   */
  for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext)
    if (akCheckAll(arg->argKind, akbReturnSnd|akbReturnKPD))
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
        lastVarArg->argReplyPos < arg->argReplyPos) {
      WriteAdjustMsgSize(file, lastVarArg);
      lastVarArg = argNULL;
    }

    if (akCheckAll(arg->argKind, akbReturnSnd|akbReturnBody|akbVarNeeded))
      WritePackArgValueNormal(file, arg);
    else if (akCheckAll(arg->argKind, akbReturnSnd|akbReturnBody|akbVariable))
      WritePackArgValueVariable(file, arg);

    if (akCheck(arg->argKind, akbReplyCopy))
      WriteCopyArgValue(file, arg);
    if (akCheck(arg->argKind, akbReplyInit))
      WriteInitArgValue(file, arg);
    /*
     * Remember whether this was variable-length.
     */
    if (akCheckAll(arg->argKind, akbReturnSnd|akbReturnBody|akbVariable))
      lastVarArg = arg;
  }
  /*
   * Finish the message size.
   */
  if (lastVarArg != argNULL)
    WriteFinishMsgSize(file, lastVarArg);
}

static void
WriteFieldDecl(FILE *file, argument_t *arg)
{
  if (akCheck(arg->argKind, akbSendKPD) ||
      akCheck(arg->argKind, akbReturnKPD))
    WriteFieldDeclPrim(file, arg, FetchKPDType);
  else
    WriteFieldDeclPrim(file, arg, FetchServerType);
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
   * WriteInitKPD_port, WriteKPD_port,  WriteExtractKPD_port,
   * WriteInitKPD_ool, WriteKPD_ool,  WriteExtractKPD_ool,
   * WriteInitKPD_oolport, WriteKPD_oolport,  WriteExtractKPD_oolport
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
          if akCheck(arg->argKind, akbSendKPD) {
            arg->argKPD_Extract =
            (IS_MULTIPLE_KPD(arg->argType)) ? WriteExtractKPD_port : WriteExtractArgValue;
            arg->argKPD_TypeCheck = WriteTCheckKPD_port;
          }
          if akCheck(arg->argKind, akbReturnKPD) {
            arg->argKPD_Template = WriteTemplateKPD_port;
            arg->argKPD_Init = WriteInitKPD_port;
            arg->argKPD_Pack = WriteKPD_port;
          }
          break;

        case MACH_MSG_OOL_DESCRIPTOR:
          if akCheck(arg->argKind, akbSendKPD) {
            arg->argKPD_Extract =
            (IS_MULTIPLE_KPD(arg->argType)) ? WriteExtractKPD_ool : WriteExtractArgValue;
            arg->argKPD_TypeCheck = WriteTCheckKPD_ool;
          }
          if akCheck(arg->argKind, akbReturnKPD) {
            arg->argKPD_Template = WriteTemplateKPD_ool;
            arg->argKPD_Init = WriteInitKPD_ool;
            arg->argKPD_Pack = WriteKPD_ool;
          }
          break;

        case MACH_MSG_OOL_PORTS_DESCRIPTOR:
          if akCheck(arg->argKind, akbSendKPD) {
            arg->argKPD_Extract =
            (IS_MULTIPLE_KPD(arg->argType)) ? WriteExtractKPD_oolport : WriteExtractArgValue;
            arg->argKPD_TypeCheck = WriteTCheckKPD_oolport;
          }
          if akCheck(arg->argKind, akbReturnKPD) {
            arg->argKPD_Template = WriteTemplateKPD_oolport;
            arg->argKPD_Init = WriteInitKPD_oolport;
            arg->argKPD_Pack = WriteKPD_oolport;
          }
          break;

        default:
          printf("MiG internal error: type of kernel processed data unknown\n");
          exit(1);
      }   /* end of switch */
}

static void WriteStringTerminatorCheck(FILE *file, routine_t *rt)
{
  // generate code to verify that the length of a C string is not greater than the size of the
  // buffer in which it is stored.
  argument_t  *argPtr;
  int msg_limit_calculated = FALSE;
  int found_string_argument = FALSE;
  int variable_length_args_present = (rt->rtMaxRequestPos > 0);

  // scan through arguments to see if there are any strings
  for (argPtr = rt->rtArgs; argPtr != NULL; argPtr = argPtr->argNext) {
    if ((argPtr->argKind & akbRequest) && argPtr->argType->itString) {
      found_string_argument = TRUE;
      break;
    }
  }

  if (found_string_argument) {
    // create a new scope, for local variables
    fputs("#if __MigTypeCheck\n" "\t" "{" "\n", file);

    for (argPtr = rt->rtArgs; argPtr != NULL; argPtr = argPtr->argNext) {
      if ((argPtr->argKind & akbRequest) && argPtr->argType->itString) {
        //fprintf(stderr, "### found itString: variable name = %s, max length = %d\n", argPtr->argName, argPtr->argType->itNumber);

        if (!msg_limit_calculated) {
          msg_limit_calculated = TRUE; // only need to do this once
          fputs("\t\t" "char * msg_limit = ((char *) In0P) + In0P->Head.msgh_size;\n", file);
          if (IsKernelServer) {
            fputs("#if __MigKernelSpecificCode\n", file);
            fputs("\t\t" "size_t strnlen_limit;" "\n", file);
            fputs("#else\n", file);
          }
          fputs("\t\t" "size_t memchr_limit;" "\n", file);
          if (IsKernelServer) {
            fputs("#endif /* __MigKernelSpecificCode */" "\n", file);
          }
          fputc('\n', file);
        }

        // I would really prefer to use strnlen() here, to ensure that the byte scanning logic does not extend beyond
        // the end of the buffer, but it's not necessarily guaranteed to be available. Instead, I'll use memchr(),
        // and let it look for the terminating null byte.
        // (later...)
        // It turns out that the kernel does not have memchr() available, but strnlen() IS available, so we'll just
        // have to emit some conditional code to use the appropriate runtime environment scanning function.
        //
        if (IsKernelServer) {
          fputs("#if __MigKernelSpecificCode\n", file);
          fputs("\t\t" "strnlen_limit = min((msg_limit - ", file);
          // If there are variable-length arguments within the message, the proper (adjusted)
          // pointers must be used to access those strings
          fprintf(file, "In%dP->%s),  %d);" "\n", (variable_length_args_present ? argPtr->argRequestPos : 0), argPtr->argName, argPtr->argType->itNumber);
          fputs("\t\t" "if (", file);
          fprintf(file, "( strnlen(In%dP->%s, strnlen_limit) >= %d + 1 )", (variable_length_args_present ? argPtr->argRequestPos : 0), argPtr->argName, argPtr->argType->itNumber);
          fputs(")" "\n" "\t\t\t" "return MIG_BAD_ARGUMENTS; // string length exceeds buffer length!" "\n", file);
          fputs("#else\n", file);
        }
        // If there are variable-length arguments within the message, the proper (adjusted)
        // pointers must be used to access those strings
        fprintf(file, "\t\t" "memchr_limit = min((msg_limit - In%dP->%s),  %d);" "\n", (variable_length_args_present ? argPtr->argRequestPos : 0), argPtr->argName, argPtr->argType->itNumber);
        fputs("\t\t" "if (", file);
        fprintf(file, "( memchr(In%dP->%s, '\\0', memchr_limit) == NULL )", (variable_length_args_present ? argPtr->argRequestPos : 0), argPtr->argName);
        fputs(")" "\n" "\t\t\t" "return MIG_BAD_ARGUMENTS; // string length exceeds buffer length!" "\n", file);
        if (IsKernelServer) {
          fputs("#endif /* __MigKernelSpecificCode */" "\n", file);
        }
      }
    }
    fputs("\t" "}" "\n" "#endif" "\t" "/* __MigTypeCheck */" "\n\n", file); // terminate new scope
  }

  return;
}

static void
WriteOOLSizeCheck(FILE *file, routine_t *rt)
{
  /* Emit code to validate the actual size of ool data vs. the reported size */

  argument_t  *argPtr;
  boolean_t   openedTypeCheckConditional = FALSE;

  // scan through arguments to see if there are any ool data blocks
  for (argPtr = rt->rtArgs; argPtr != NULL; argPtr = argPtr->argNext) {
    if (akCheck(argPtr->argKind, akbSendKPD) && (argPtr->argKPD_Type == MACH_MSG_OOL_DESCRIPTOR)) {
      register ipc_type_t *it = argPtr->argType;
      char *tab, string[MAX_STR_LEN];
      boolean_t test;
      argument_t  *argCountPtr;
  
      if ( !openedTypeCheckConditional ) {
        openedTypeCheckConditional = TRUE;
        fputs("#if __MigTypeCheck\n", file);
      }

      if (IS_MULTIPLE_KPD(it)) {
        WriteKPD_Iterator(file, TRUE, FALSE, argPtr, TRUE);
        tab = "\t\t\t";
        sprintf(string, "ptr->");
        test = !it->itVarArray && !it->itElement->itVarArray;
        it = it->itElement; // point to element descriptor, so size calculation is correct
        argCountPtr = argPtr->argSubCount;
      }
      else {
        tab = "";
        sprintf(string, "In%dP->%s.", argPtr->argRequestPos, argPtr->argMsgField);
        test = !it->itVarArray;
        argCountPtr = argPtr->argCount;
      }
    
      if (!test) {
        int multiplier = (argCountPtr->argMultiplier > 1 || it->itSize > 8) ? argCountPtr->argMultiplier * it->itSize / 8 : 1;
        fprintf(file, "\t%s" "if (%ssize ", tab, string);
        if (multiplier > 1)
          fprintf(file, "/ %d ", multiplier);
	fprintf(file,"!= In%dP->%s%s)\n", argCountPtr->argRequestPos, argCountPtr->argVarName, IS_MULTIPLE_KPD(it) ? "[i]" : "");

        fprintf(file, "\t\t%s" "return MIG_TYPE_ERROR;\n", tab);
      }
      
      if (IS_MULTIPLE_KPD(it))
        fprintf(file, "\t    }\n\t}\n");

    }
  }

  if ( openedTypeCheckConditional )
    fputs("#endif" "\t" "/* __MigTypeCheck */" "\n\n", file);
}


void
WriteCheckRequest(FILE *file, routine_t *rt)
{
  int i;

  /* initialize the disciplines for the handling of KPDs */
  InitKPD_Disciplines(rt->rtArgs);

  fprintf(file, "\n");
  fprintf(file, "#if ( __MigTypeCheck ");
  if (CheckNDR) 
	  fprintf(file, "|| __NDR_convert__ ");
  fprintf(file, ")\n");
  fprintf(file, "#if __MIG_check__Request__%s_subsystem__\n", SubsystemName);
  fprintf(file, "#if !defined(__MIG_check__Request__%s_t__defined)\n", rt->rtName);
  fprintf(file, "#define __MIG_check__Request__%s_t__defined\n", rt->rtName);
  if (CheckNDR && akCheck(rt->rtNdrCode->argKind, akbRequest)) {
    WriteList(file, rt->rtArgs, WriteRequestNDRConvertIntRepArgDecl, akbSendNdr, "", "");
    WriteList(file, rt->rtArgs, WriteRequestNDRConvertCharRepArgDecl, akbSendNdr, "", "");
    WriteList(file, rt->rtArgs, WriteRequestNDRConvertFloatRepArgDecl, akbSendNdr, "", "");
  }
  fprintf(file, "\n");
  fprintf(file, "mig_internal kern_return_t __MIG_check__Request__%s_t(__attribute__((__unused__)) __Request__%s_t *In0P", rt->rtName, rt->rtName);
  for (i = 1; i <= rt->rtMaxRequestPos; i++)
    fprintf(file, ", __attribute__((__unused__)) __Request__%s_t **In%dPP", rt->rtName, i);
  fprintf(file, ")\n{\n");

  fprintf(file, "\n\ttypedef __Request__%s_t __Request;\n", rt->rtName);
  for (i = 1; i <= rt->rtMaxRequestPos; i++)
    fprintf(file, "\t__Request *In%dP;\n", i);
  if (rt->rtNumRequestVar > 0) {
    fprintf(file, "#if\t__MigTypeCheck\n");
    fprintf(file, "\tunsigned int msgh_size;\n");
    fprintf(file, "#endif\t/* __MigTypeCheck */\n");
  }
  if (rt->rtMaxRequestPos > 0)
    fprintf(file, "\tunsigned int msgh_size_delta;\n");
  if (rt->rtNumRequestVar > 0 || rt->rtMaxRequestPos > 0)
    fprintf(file, "\n");

  WriteCheckHead(file, rt);

  WriteList(file, rt->rtArgs, WriteTypeCheck, akbSendKPD, "\n", "\n");

  {
    argument_t *arg, *lastVarArg;

    lastVarArg = argNULL;
    for (arg = rt->rtArgs; arg != argNULL; arg = arg->argNext) {
      if (lastVarArg != argNULL &&
          lastVarArg->argRequestPos < arg->argRequestPos) {
        WriteAdjustRequestMsgPtr(file, lastVarArg);
        lastVarArg = argNULL;
      }
      if (akCheckAll(arg->argKind, akbSendRcv|akbSendBody)) {
        if (akCheck(arg->argKind, akbVariable)) {
          WriteCheckMsgSize(file, arg);
          lastVarArg = arg;
        }
      }
    }
  }

  if (CheckNDR && akCheck(rt->rtNdrCode->argKind, akbRequest)) {
    fprintf(file, "#if\t");
    WriteList(file, rt->rtArgs, WriteRequestNDRConvertIntRepArgCond, akbSendNdr, " || \\\n\t", "\n");
    fprintf(file, "\tif (In0P->NDR.int_rep != NDR_record.int_rep) {\n");
    WriteList(file, rt->rtArgs, WriteRequestNDRConvertIntRepArgUse, akbSendNdr, "", "");
    fprintf(file, "\t}\n#endif\t/* defined(__NDR_convert__int_rep...) */\n\n");

    WriteOOLSizeCheck(file, rt);

    fprintf(file, "#if\t");
    WriteList(file, rt->rtArgs, WriteRequestNDRConvertCharRepArgCond, akbSendNdr, " || \\\n\t", "\n");
    fprintf(file, "\tif (In0P->NDR.char_rep != NDR_record.char_rep) {\n");
    WriteList(file, rt->rtArgs, WriteRequestNDRConvertCharRepArgUse, akbSendNdr, "", "");
    fprintf(file, "\t}\n#endif\t/* defined(__NDR_convert__char_rep...) */\n\n");

    fprintf(file, "#if\t");
    WriteList(file, rt->rtArgs, WriteRequestNDRConvertFloatRepArgCond, akbSendNdr, " || \\\n\t", "\n");
    fprintf(file, "\tif (In0P->NDR.float_rep != NDR_record.float_rep) {\n");
    WriteList(file, rt->rtArgs, WriteRequestNDRConvertFloatRepArgUse, akbSendNdr, "", "");
    fprintf(file, "\t}\n#endif\t/* defined(__NDR_convert__float_rep...) */\n\n");
  } else {
   WriteOOLSizeCheck(file, rt);
  }

  WriteStringTerminatorCheck(file, rt);
  
  fprintf(file, "\treturn MACH_MSG_SUCCESS;\n");
  fprintf(file, "}\n");
  fprintf(file, "#endif /* !defined(__MIG_check__Request__%s_t__defined) */\n", rt->rtName);
  fprintf(file, "#endif /* __MIG_check__Request__%s_subsystem__ */\n", SubsystemName);
  fprintf(file, "#endif /* ( __MigTypeCheck ");
  if (CheckNDR)
	  fprintf(file, "|| __NDR_convert__ ");
  fprintf(file, ") */\n");
  fprintf(file, "\n");
}

void
WriteCheckRequestCall(FILE *file, routine_t *rt)
{
  int i;

  fprintf(file, "\n");
  fprintf(file, "#if\tdefined(__MIG_check__Request__%s_t__defined)\n", rt->rtName);
  fprintf(file, "\tcheck_result = __MIG_check__Request__%s_t((__Request *)In0P", rt->rtName);
  for (i = 1; i <= rt->rtMaxRequestPos; i++)
    fprintf(file, ", (__Request **)&In%dP", i);
  fprintf(file, ");\n");
  fprintf(file, "\tif (check_result != MACH_MSG_SUCCESS)\n");
  fprintf(file, "\t\t{ MIG_RETURN_ERROR(OutP, check_result); }\n");
  fprintf(file, "#endif\t/* defined(__MIG_check__Request__%s_t__defined) */\n", rt->rtName);
  fprintf(file, "\n");
}

void
WriteCheckRequests(FILE *file, statement_t *stats)
{
  statement_t *stat;

  for (stat = stats; stat != stNULL; stat = stat->stNext)
    if (stat->stKind == skRoutine)
      WriteCheckRequest(file, stat->stRoutine);
}

static void
WriteRoutine(FILE *file, register routine_t *rt)
{
  /*  Declare the server work function: */
  if (ServerHeaderFileName == strNULL)
    WriteServerRoutine(file, rt);

  fprintf(file, "\n");

  fprintf(file, "/* %s %s */\n", rtRoutineKindToStr(rt->rtKind), rt->rtName);
  fprintf(file, "mig_internal novalue _X%s\n", rt->rtName);
  if (BeAnsiC) {
    fprintf(file, "\t(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)\n");
  }
  else {
    fprintf(file, "#if\t%s\n", NewCDecl);
    fprintf(file, "\t(mach_msg_header_t *InHeadP, mach_msg_header_t *OutHeadP)\n");
    fprintf(file, "#else\n");
    fprintf(file, "\t(InHeadP, OutHeadP)\n");
    fprintf(file, "\tmach_msg_header_t *InHeadP, *OutHeadP;\n");
    fprintf(file, "#endif\t/* %s */\n", NewCDecl);
  }

  fprintf(file, "{\n");
  WriteStructDecl(file, rt->rtArgs, WriteFieldDecl, akbRequest, "Request", rt->rtSimpleRequest, TRUE, rt->rtServerImpl, FALSE);
  fprintf(file, "\ttypedef __Request__%s_t __Request;\n", rt->rtName);
  fprintf(file, "\ttypedef __Reply__%s_t Reply;\n\n", rt->rtName);

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

  WriteVarDecls(file, rt);

  if (IsKernelServer) {
    fprintf(file, "#if\t__MigKernelSpecificCode\n");
    WriteList(file, rt->rtArgs, WriteTemplateDeclOut, akbReturnKPD, "\n", "\n");
    fprintf(file, "#else\n");
  }
  WriteList(file, rt->rtArgs, WriteTemplateDeclIn, akbReturnKPD, "\n", "\n");
  if (IsKernelServer) {
    fprintf(file, "#endif /* __MigKernelSpecificCode */\n");
  }
  WriteRetCode(file, rt->rtRetCode);
  WriteList(file, rt->rtArgs, WriteLocalVarDecl, akbVarNeeded | akbServerArg, ";\n", ";\n\n");
  WriteApplMacro(file, "Rcv", "Declare", rt);
  WriteApplMacro(file, "Rcv", "Before", rt);
  if (rt->rtRetCArg != argNULL && !rt->rtSimpleRequest) {
    WriteRetCArgCheckError(file, rt);
    if (rt->rtServerImpl)
      WriteCheckTrailerHead(file, rt, FALSE);
    WriteServerCall(file, rt, WriteConditionalCallArg);
    WriteRetCArgFinishError(file, rt);
  }

  WriteCheckRequestCall(file, rt);
  WriteCheckRequestTrailerArgs(file, rt);

  /*
   * Initialize the KPD records in the Reply structure with the
   * templates. We do this beforehand because the call to the procedure
   * will overwrite some of the values (after the call it would be impossible
   * to initialize the KPD records from the static Templates, because we
   * would lose data).
   */
  WriteList(file, rt->rtArgs, WriteInitKPDValue, akbReturnKPD, "\n", "\n");

  WriteList(file, rt->rtArgs, WriteExtractArg, akbNone, "", "");

  if (UseEventLogger)
    WriteLogMsg(file, rt, LOG_SERVER, LOG_REQUEST);

  WriteServerCall(file, rt, WriteServerCallArg);

  WriteReverseList(file, rt->rtArgs, WriteDestroyArg, akbDestroy, "", "");

  /*
   * For one-way routines, it doesn`t make sense to check the return
   * code, because we return immediately afterwards.  However,
   * kernel servers may want to deallocate port arguments - and the
   * deallocation must not be done if the return code is not KERN_SUCCESS.
   */
  if (rt->rtOneWay || rt->rtNoReplyArgs) {
    if (IsKernelServer) {
      fprintf(file,"#if\t__MigKernelSpecificCode\n");
      if (rtCheckMaskFunction(rt->rtArgs, akbSendKPD, CheckDestroyPortArg)) {
        WriteCheckReturnValue(file, rt);
      }
      WriteReverseList(file, rt->rtArgs, WriteDestroyPortArg, akbSendKPD, "", "");
      fprintf(file,"#endif /* __MigKernelSpecificCode */\n");
    }
    /* although we have an empty reply, we still have to make sure that
     some fields such as NDR get properly initialized */
    if (!rt->rtOneWay)
      WriteList(file, rt->rtArgs, WriteInitArgValue, akbReplyInit, "\n", "\n");
  }
  else {
    WriteCheckReturnValue(file, rt);

    if (IsKernelServer) {
      fprintf(file,"#if\t__MigKernelSpecificCode\n");
      WriteReverseList(file, rt->rtArgs, WriteDestroyPortArg, akbSendKPD, "", "");
      fprintf(file,"#endif /* __MigKernelSpecificCode */\n");
    }
    WriteReplyArgs(file, rt);
    WriteReplyInit(file, rt);
    if (!rt->rtSimpleReply)
      fprintf(file, "\tOutP->msgh_body.msgh_descriptor_count = %d;\n", rt->rtReplyKPDs);
  }
  if (UseEventLogger)
    WriteLogMsg(file, rt, LOG_SERVER, LOG_REPLY);

  WriteApplMacro(file, "Rcv", "After", rt);
  fprintf(file, "}\n");
}

void
WriteServer(FILE *file, statement_t *stats)
{
  register statement_t *stat;

  WriteProlog(file, stats);
  if (BeAnsiC)
    WriteForwardDeclarations(file, stats);
  for (stat = stats; stat != stNULL; stat = stat->stNext)
    switch (stat->stKind) {

      case skRoutine:
        WriteCheckRequest(file, stat->stRoutine);
        WriteRoutine(file, stat->stRoutine);
        break;

      case skIImport:
      case skImport:
      case skSImport:
      case skDImport:
      case skUImport:
        break;

      default:
        fatal("WriteServer(): bad statement_kind_t (%d)",
              (int) stat->stKind);
    }
  WriteDispatcher(file, stats);
}

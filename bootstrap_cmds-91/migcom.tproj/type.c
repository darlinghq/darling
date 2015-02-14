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
#include <sys/types.h>
#include <mach/message.h>
#include <mach/std_types.h>
#include <mach/ndr.h>
#include "mig_machine.h"
#include "routine.h"
#include "error.h"
#include "alloc.h"
#include "global.h"
#include <stdio.h>

#define PortSize  (sizeof (mach_port_t) * NBBY)

ipc_type_t *itRetCodeType;      /* used for return codes */
ipc_type_t *itNdrCodeType;      /* used for NDR format labels */
ipc_type_t *itDummyType;        /* used for camelot dummy args */
ipc_type_t *itTidType;          /* used for camelot tids */
ipc_type_t *itRequestPortType;  /* used for default Request port arg */
ipc_type_t *itZeroReplyPortType;/* used for dummy Reply port arg */
ipc_type_t *itRealReplyPortType;/* used for default Reply port arg */
ipc_type_t *itWaitTimeType;     /* used for dummy WaitTime args */
ipc_type_t *itMsgOptionType;    /* used for dummy MsgOption args */

static ipc_type_t *list = itNULL;

static char *machine_integer_name;
static u_int machine_integer_size;
static u_int machine_integer_bits;

/*
 *  Searches for a named type.  We use a simple
 *  self-organizing linked list.
 */
ipc_type_t *
itLookUp(identifier_t name)
{
  register ipc_type_t *it, **last;
  
  for (it = *(last = &list); it != itNULL; it = *(last = &it->itNext))
    if (streql(name, it->itName)) {
      /* move this type to the front of the list */
      *last = it->itNext;
      it->itNext = list;
      list = it;
      
      return it;
    }
  
  return itNULL;
}

/*
 *  Enters a new name-type association into
 *  our self-organizing linked list.
 */
void
itInsert(identifier_t name, ipc_type_t *it)
{
  it->itName = name;
  it->itNext = list;
  list = it;
}

static ipc_type_t *
itAlloc(void)
{
  static ipc_type_t prototype =
  {
    strNULL,  /* identifier_t itName */
    0,        /* ipc_type_t *itNext */
    0,        /* u_int itTypeSize */
    0,        /* u_int itPadSize */
    0,        /* u_int itMinTypeSize */
    0,        /* u_int itInName */
    0,        /* u_int itOutName */
    0,        /* u_int itSize */
    1,        /* u_int itNumber */
    0,        /* u_int itKPD_Number */
    TRUE,     /* boolean_t itInLine */
    FALSE,    /* boolean_t itMigInLine */
    FALSE,    /* boolean_t itPortType */
    strNULL,  /* string_t itInNameStr */
    strNULL,  /* string_t itOutNameStr */
    TRUE,     /* boolean_t itStruct */
    FALSE,    /* boolean_t itString */
    FALSE,    /* boolean_t itVarArray */
    FALSE,    /* boolean_t itNoOptArray */
    FALSE,    /* boolean_t itNative */
    FALSE,    /* boolean_t itNativePointer */
    itNULL,   /* ipc_type_t *itElement */
    strNULL,  /* identifier_t itUserType */
    strNULL,  /* identifier_t itServerType */
    strNULL,  /* identifier_t itTransType */
    strNULL,  /* identifier_t itUserKPDType */
    strNULL,  /* identifier_t itServerKPDType */
    strNULL,  /* identifier_t itInTrans */
    strNULL,  /* identifier_t itOutTrans */
    strNULL,  /* identifier_t itDestructor */
  };
  register ipc_type_t *new;
  
  new = (ipc_type_t *) malloc(sizeof *new);
  if (new == itNULL)
    fatal("itAlloc(): %s", strerror(errno));
  *new = prototype;
  return new;
}

/*
 * Convert an IPC type-name into a string.
 */
static char *
itNameToString(u_int name)
{
  char buffer[100];
  
  (void) sprintf(buffer, "%u", name);
  return strmake(buffer);
}

/*
 * Calculate itTypeSize, itPadSize, itMinTypeSize
 * Every type needs this info; it is recalculated
 * when itInLine, itNumber, or itSize changes.
 */
static void
itCalculateSizeInfo(register ipc_type_t *it)
{
  if (!IS_KERN_PROC_DATA(it))
  {
    u_int bytes = (it->itNumber * it->itSize + 7) / 8;
    u_int padding = machine_padding(bytes);
    
    it->itTypeSize = bytes;
    it->itPadSize = padding;
    if (IS_VARIABLE_SIZED_UNTYPED(it)) {
      /*
       * for these arrays, the argCount is not a akbRequest|akbReply,
       * therefore we need to account here for the space of the count
       * (itMinTypeSize is used only in rtFindSize)
       */
      it->itMinTypeSize = sizeof (mach_msg_type_number_t);
      /*
       * NDR encoded VarString carry the extra offset 4-bytes fields
       * for MIG, it should be always 0;
       */
      if (it->itString)
        it->itMinTypeSize += sizeof (mach_msg_type_number_t);
    }
    else
      it->itMinTypeSize = bytes + padding;
  }
  else {
    /*
     * 1) ports 2) OOL 3) ports OOL
     * all have the same size = sizeof(mach_msg_descriptor_t)
     */
    u_int bytes;
    if (IS_MULTIPLE_KPD(it))
      bytes = it->itKPD_Number * 12 /* sizeof(mach_msg_descriptor_t) */;
    else
      bytes = 12 /* sizeof(mach_msg_descriptor_t) */;
    
    it->itTypeSize = bytes;
    it->itPadSize = 0;
    it->itMinTypeSize = bytes;
  }
  
  /* Unfortunately, these warning messages can't give a type name;
   we haven't seen a name yet (it might stay anonymous.) */
  
  if ((it->itTypeSize == 0) && !it->itVarArray && !it->itNative)
    warn("sizeof(%s) == 0");
}

/*
 * Fill in default values for some fields used in code generation:
 *  itInNameStr, itOutNameStr, itUserType, itServerType, itTransType
 * Every argument's type should have these values filled in.
 */
static void
itCalculateNameInfo(register ipc_type_t *it)
{
  if (it->itInNameStr == strNULL)
    it->itInNameStr = strmake(itNameToString(it->itInName));
  if (it->itOutNameStr == strNULL)
    it->itOutNameStr = strmake(itNameToString(it->itOutName));
  
  if (it->itUserType == strNULL)
    it->itUserType = it->itName;
  if (it->itServerType == strNULL)
    it->itServerType = it->itName;
#if 0
  /*
   *  KernelServer and KernelUser interfaces get special treatment here.
   *  On the kernel side of the interface, ports are really internal
   *  port pointers (ipc_port_t), not port names (mach_port_t).
   *  At this point, we don't know if the argument is in or out,
   *  so we don't know if we should look at itInName or itOutName.
   *  Looking at both should be OK.
   *
   *  This is definitely a hack, but I think it is cleaner than
   *  mucking with type declarations throughout the kernel .def files,
   *  hand-conditionalizing on KERNEL_SERVER and KERNEL_USER.
   */
  
  if (IsKernelServer &&
      streql(it->itServerType, "mach_port_t") &&
      (((it->itInName == MACH_MSG_TYPE_POLYMORPHIC) &&
        (it->itOutName == MACH_MSG_TYPE_POLYMORPHIC)) ||
       MACH_MSG_TYPE_PORT_ANY(it->itInName) ||
       MACH_MSG_TYPE_PORT_ANY(it->itOutName)))
    it->itServerType = "ipc_port_t";
  
  if (IsKernelUser &&
      streql(it->itUserType, "mach_port_t") &&
      (((it->itInName == MACH_MSG_TYPE_POLYMORPHIC) &&
        (it->itOutName == MACH_MSG_TYPE_POLYMORPHIC)) ||
       MACH_MSG_TYPE_PORT_ANY(it->itInName) ||
       MACH_MSG_TYPE_PORT_ANY(it->itOutName)))
    it->itUserType = "ipc_port_t";
#endif /* 0 */
  
  if (it->itTransType == strNULL)
    it->itTransType = it->itServerType;
}

/******************************************************
 *  Checks for non-implemented types, conflicting type
 *  flags and whether the long or short form of msg type
 *  descriptor is appropriate. Called after each type statement
 *  is parsed.
 ******************************************************/
static void
itCheckDecl(identifier_t name, register ipc_type_t *it)
{
  it->itName = name;
  
  itCalculateNameInfo(it);
  
  /* do a bit of error checking, mostly necessary because of
   limitations in Mig */
  
  if (it->itVarArray) {
    if ((it->itInTrans != strNULL) || (it->itOutTrans != strNULL))
      error("%s: can't translate variable-sized arrays", name);
    
    if (it->itDestructor != strNULL)
      error("%s: can't destroy variable-sized array", name);
  }
}

/*
 *  Pretty-prints translation/destruction/type information.
 */
static void
itPrintTrans(register ipc_type_t *it)
{
  if (!streql(it->itName, it->itUserType))
    printf("\tCUserType:\t%s\n", it->itUserType);
  
  if (!streql(it->itName, it->itServerType))
    printf("\tCServerType:\t%s\n", it->itServerType);
  
  if (it->itInTrans != strNULL)
    printf("\tInTran:\t\t%s %s(%s)\n", it->itTransType, it->itInTrans, it->itServerType);
  
  if (it->itOutTrans != strNULL)
    printf("\tOutTran:\t%s %s(%s)\n", it->itServerType, it->itOutTrans, it->itTransType);
  
  if (it->itDestructor != strNULL)
    printf("\tDestructor:\t%s(%s)\n", it->itDestructor, it->itTransType);
}

/*
 *  Pretty-prints type declarations.
 */
static void
itPrintDecl(identifier_t name, ipc_type_t *it)
{
  printf("Type %s = ", name);
  if (!it->itInLine)
    printf("^ ");
  if (it->itVarArray)
    if (it->itNumber == 0 || it->itMigInLine)
      printf("array [] of ");
    else
      printf("array [*:%d] of ", it->itNumber);
    else if (it->itStruct && ((it->itNumber != 1) ||
                              (it->itInName == MACH_MSG_TYPE_STRING_C)))
      printf("struct [%d] of ", it->itNumber);
    else if (it->itNumber != 1)
      printf("array [%d] of ", it->itNumber);
  
  if (streql(it->itInNameStr, it->itOutNameStr))
    printf("(%s,", it->itInNameStr);
  else
    printf("(%s|%s", it->itInNameStr, it->itOutNameStr);
  
  printf(" %d)\n", it->itSize);
  
  itPrintTrans(it);
  
  printf("\n");
}

/*
 *  Handles named type-specs, which can occur in type
 *  declarations or in argument lists.  For example,
 * type foo = type-spec; // itInsert will get called later
 * routine foo(arg : bar = type-spec); // itInsert won't get called
 */
void
itTypeDecl(identifier_t name, ipc_type_t *it)
{
  itCheckDecl(name, it);
  
  if (BeVerbose)
    itPrintDecl(name, it);
}

/*
 *  Handles declarations like
 * type new = name;
 * type new = inname|outname;
 */
ipc_type_t *
itShortDecl(u_int inname,  string_t instr, u_int outname, string_t outstr, u_int defsize)
{
  register ipc_type_t *it;
  
  if (defsize == 0)
    error("must use full IPC type decl");
  
  it = itAlloc();
  it->itInName = inname;
  it->itInNameStr = instr;
  it->itOutName = outname;
  it->itOutNameStr = outstr;
  it->itSize = defsize;
  if (inname == MACH_MSG_TYPE_STRING_C)
  {
    it->itStruct = FALSE;
    it->itString = TRUE;
  }
  /*
   * I check only inname, because outname
   * has to be a port as well (polymorphic types
   * are now restricted to port rights)
   */
  if (MACH_MSG_TYPE_PORT_ANY(inname) ||
      inname == MACH_MSG_TYPE_POLYMORPHIC) {
    it->itPortType = TRUE;
    it->itKPD_Number = 1;
  }
  
  itCalculateSizeInfo(it);
  return it;
}

static ipc_type_t *
itCopyType(ipc_type_t *old)
{
  register ipc_type_t *new = itAlloc();
  
  *new = *old;
  new->itName = strNULL;
  new->itNext = itNULL;
  new->itElement = old;
  
  /* size info still valid */
  return new;
}

/*
 * A call to itCopyType is almost always followed with itResetType.
 * The exception is itPrevDecl.  Also called before adding any new
 * translation/destruction/type info (see parser.y).
 *
 * type new = old; // new doesn't get old's info
 * type new = array[*:10] of old;
 * // new doesn't get old's info, but new->itElement does
 * type new = array[*:10] of struct[3] of old;
 * // new and new->itElement don't get old's info
 */

ipc_type_t *
itResetType(ipc_type_t *old)
{
  /* reset all special translation/destruction/type info */
  
  old->itInTrans = strNULL;
  old->itOutTrans = strNULL;
  old->itDestructor = strNULL;
  old->itUserType = strNULL;
  old->itServerType = strNULL;
  old->itTransType = strNULL;
  return old;
}

/*
 *  Handles the declaration
 * type new = old;
 */
ipc_type_t *
itPrevDecl(identifier_t name)
{
  register ipc_type_t *old;
  
  old = itLookUp(name);
  if (old == itNULL) {
    error("type '%s' not defined", name);
    return itAlloc();
  }
  else
    return itCopyType(old);
}

/*
 *  Handles the declarations
 * type new = array[] of old; // number is oo
 * type new = array[*] of old; // number is oo
 * type new = array[*:number] of old;
 */
ipc_type_t *
itVarArrayDecl(u_int number, register ipc_type_t *old)
{
  register ipc_type_t *it = itResetType(itCopyType(old));
  
  if (!it->itInLine) {
    /* already an initialized KPD */
    if (it->itKPD_Number != 1 || !number)
      error("IPC type decl is too complicated for Kernel Processed Data");
    it->itKPD_Number *= number;
    it->itNumber = 1;
    it->itInLine = FALSE;
    it->itStruct = FALSE;
  }
  else if (it->itVarArray)
    error("IPC type decl is too complicated");
  else if (number) {
    it->itNumber *= number;
    /*
     * Bounded [Scalar, Port] VarArray: in-line!
     */
    it->itInLine = TRUE;
    it->itStruct = FALSE;
    if (it->itPortType)
      it->itKPD_Number *= number;
  }
  else {
    it->itNumber = 0;
    /*
     * UnBounded [Scalar, Port] VarArray: always in-line
     * interface and out-of-line mechanism!
     */
    it->itMigInLine = TRUE;
    it->itInLine = FALSE;
    it->itStruct = TRUE;
    it->itKPD_Number = 1;
  }
  
  it->itVarArray = TRUE;
  it->itString = FALSE;
  
  itCalculateSizeInfo(it);
  return it;
}

/*
 *  Handles the declaration
 * type new = array[number] of old;
 */
ipc_type_t *
itArrayDecl(u_int number, ipc_type_t *old)
{
  register ipc_type_t *it = itResetType(itCopyType(old));
  
  if (!it->itInLine) {
    /* already an initialized KPD */
    if (it->itKPD_Number != 1)
      error("IPC type decl is too complicated for Kernel Processed Data");
    it->itKPD_Number *= number;
    it->itNumber = 1;
    it->itStruct = FALSE;
    it->itString = FALSE;
    it->itVarArray = FALSE;
  }
  else if (it->itVarArray)
    error("IPC type decl is too complicated");
  else {
    it->itNumber *= number;
    it->itStruct = FALSE;
    it->itString = FALSE;
    if (it->itPortType)
      it->itKPD_Number *= number;
  }
  
  itCalculateSizeInfo(it);
  return it;
}

/*
 *  Handles the declaration
 * type new = ^ old;
 */
ipc_type_t *
itPtrDecl(ipc_type_t *it)
{
  if (!it->itInLine && !it->itMigInLine)
    error("IPC type decl is already defined to be Out-Of-Line");
  it->itInLine = FALSE;
  it->itStruct = TRUE;
  it->itString = FALSE;
  it->itMigInLine = FALSE;
  it->itKPD_Number = 1;
  
  itCalculateSizeInfo(it);
  return it;
}

/*
 *  Handles the declaration
 * type new = struct[number] of old;
 */
ipc_type_t *
itStructDecl(u_int number, ipc_type_t *old)
{
  register ipc_type_t *it = itResetType(itCopyType(old));
  
  if (!it->itInLine || it->itVarArray)
    error("IPC type decl is too complicated");
  it->itNumber *= number;
  it->itStruct = TRUE;
  it->itString = FALSE;
  
  itCalculateSizeInfo(it);
  return it;
}

/*
 * Treat 'c_string[n]' as
 * 'array[n] of (MSG_TYPE_STRING_C, 8)'
 */
ipc_type_t *
itCStringDecl(int count, boolean_t varying)
{
  register ipc_type_t *it;
  register ipc_type_t *itElement;
  
  itElement = itShortDecl(MACH_MSG_TYPE_STRING_C, "MACH_MSG_TYPE_STRING_C", MACH_MSG_TYPE_STRING_C, "MACH_MSG_TYPE_STRING_C", 8);
  itCheckDecl("char", itElement);
  
  it = itResetType(itCopyType(itElement));
  it->itNumber = count;
  it->itVarArray = varying;
  it->itStruct = FALSE;
  it->itString = TRUE;
  
  itCalculateSizeInfo(it);
  return it;
}

extern ipc_type_t *
itMakeSubCountType(int count, boolean_t varying, string_t  name)
{
  register ipc_type_t *it;
  register ipc_type_t *itElement;
  
  itElement = itShortDecl(machine_integer_size, machine_integer_name, machine_integer_size, machine_integer_name, machine_integer_bits);
  itCheckDecl("mach_msg_type_number_t", itElement);
  
  it = itResetType(itCopyType(itElement));
  it->itNumber = count;
  /*
   * I cannot consider it as a Fixed array, otherwise MiG will try
   * to follow the path for efficient copy of arrays
   */
  it->itVarArray = FALSE;
  it->itStruct = FALSE;
  it->itString = FALSE;
  it->itInLine = TRUE;
  it->itName = "mach_msg_type_number_t *";
  if (varying)
    it->itVarArray = TRUE;
  else
    /* to skip the optimized copy of fixed array: in fact we need to
     * reference each element and we also miss a user type for it */
    it->itNoOptArray = TRUE;
  
  itCalculateSizeInfo(it);
  itCalculateNameInfo(it);
  return it;
}

extern ipc_type_t *
itMakeCountType(void)
{
  ipc_type_t *it = itAlloc();
  
  it->itName = "mach_msg_type_number_t";
  it->itInName = machine_integer_size;
  it->itInNameStr = machine_integer_name;
  it->itOutName = machine_integer_size;
  it->itOutNameStr = machine_integer_name;
  it->itSize = machine_integer_bits;
  
  itCalculateSizeInfo(it);
  itCalculateNameInfo(it);
  return it;
}

extern ipc_type_t *
itMakePolyType(void)
{
  ipc_type_t *it = itAlloc();
  
  it->itName = "mach_msg_type_name_t";
  it->itInName = machine_integer_size;
  it->itInNameStr = machine_integer_name;
  it->itOutName = machine_integer_size;
  it->itOutNameStr = machine_integer_name;
  it->itSize = machine_integer_bits;
  
  itCalculateSizeInfo(it);
  itCalculateNameInfo(it);
  return it;
}

extern ipc_type_t *
itMakeDeallocType(void)
{
  ipc_type_t *it = itAlloc();
  
  it->itName = "boolean_t";
  it->itInName = MACH_MSG_TYPE_BOOLEAN;
  it->itInNameStr = "MACH_MSG_TYPE_BOOLEAN";
  it->itOutName = MACH_MSG_TYPE_BOOLEAN;
  it->itOutNameStr = "MACH_MSG_TYPE_BOOLEAN";
  it->itSize = machine_integer_bits;
  
  itCalculateSizeInfo(it);
  itCalculateNameInfo(it);
  return it;
}

extern ipc_type_t *
itNativeType(identifier_t id, boolean_t ptr, identifier_t badval)
{
  ipc_type_t *it = itAlloc();
  
  it->itInName = MACH_MSG_TYPE_BYTE;
  it->itInNameStr = "MACH_MSG_TYPE_BYTE";
  it->itOutName = MACH_MSG_TYPE_BYTE;
  it->itOutNameStr = "MACH_MSG_TYPE_BYTE";
  it->itInLine = TRUE;
  it->itNative = TRUE;
  it->itNativePointer = ptr;
  it->itServerType = id;
  it->itUserType = id;
  it->itTransType = id;
  it->itBadValue = badval;
  
  itCalculateSizeInfo(it);
  itCalculateNameInfo(it);
  return it;
}

/*
 *  Initializes the pre-defined types.
 */
void
init_type(void)
{
  u_int size;
  
  size = NBBY * sizeof (natural_t);
  if (size == 32) {
    machine_integer_name = "MACH_MSG_TYPE_INTEGER_32";
    machine_integer_size = MACH_MSG_TYPE_INTEGER_32;
  }
  else if (size == 64) {
    machine_integer_name = "MACH_MSG_TYPE_INTEGER_64";
    machine_integer_size = MACH_MSG_TYPE_INTEGER_64;
  }
  else
    error("init_type unknown size %d", size);
  
  machine_integer_bits = size;
  
  itRetCodeType = itAlloc();
  itRetCodeType->itName = "kern_return_t";
  itRetCodeType->itInName = machine_integer_size;
  itRetCodeType->itInNameStr = machine_integer_name;
  itRetCodeType->itOutName = machine_integer_size;
  itRetCodeType->itOutNameStr = machine_integer_name;
  itRetCodeType->itSize = machine_integer_bits;
  itCalculateSizeInfo(itRetCodeType);
  itCalculateNameInfo(itRetCodeType);
  
  itNdrCodeType = itAlloc();
  itNdrCodeType->itName = "NDR_record_t";
  itNdrCodeType->itInName = 0;
  itNdrCodeType->itInNameStr = "NDR_record_t";
  itNdrCodeType->itOutName = 0;
  itNdrCodeType->itOutNameStr = "NDR_record_t";
  itNdrCodeType->itSize = sizeof(NDR_record_t) * 8;
  itCalculateSizeInfo(itNdrCodeType);
  itCalculateNameInfo(itNdrCodeType);
  
  itDummyType = itAlloc();
  itDummyType->itName = "char *";
  itDummyType->itInName = MACH_MSG_TYPE_UNSTRUCTURED;
  itDummyType->itInNameStr = "MACH_MSG_TYPE_UNSTRUCTURED";
  itDummyType->itOutName = MACH_MSG_TYPE_UNSTRUCTURED;
  itDummyType->itOutNameStr = "MACH_MSG_TYPE_UNSTRUCTURED";
  itDummyType->itSize = PortSize;
  itCalculateSizeInfo(itDummyType);
  itCalculateNameInfo(itDummyType);
  
  itTidType = itAlloc();
  itTidType->itName = "tid_t";
  itTidType->itInName = machine_integer_size;
  itTidType->itInNameStr = machine_integer_name;
  itTidType->itOutName = machine_integer_size;
  itTidType->itOutNameStr = machine_integer_name;
  itTidType->itSize = machine_integer_bits;
  itTidType->itNumber = 6;
  itCalculateSizeInfo(itTidType);
  itCalculateNameInfo(itTidType);
  
  itRequestPortType = itAlloc();
  itRequestPortType->itName = "mach_port_t";
  itRequestPortType->itInName = MACH_MSG_TYPE_COPY_SEND;
  itRequestPortType->itInNameStr = "MACH_MSG_TYPE_COPY_SEND";
  itRequestPortType->itOutName = MACH_MSG_TYPE_PORT_SEND;
  itRequestPortType->itOutNameStr = "MACH_MSG_TYPE_PORT_SEND";
  itRequestPortType->itSize = PortSize;
  itCalculateSizeInfo(itRequestPortType);
  itCalculateNameInfo(itRequestPortType);
  
  itZeroReplyPortType = itAlloc();
  itZeroReplyPortType->itName = "mach_port_t";
  itZeroReplyPortType->itInName = 0;
  itZeroReplyPortType->itInNameStr = "0";
  itZeroReplyPortType->itOutName = 0;
  itZeroReplyPortType->itOutNameStr = "0";
  itZeroReplyPortType->itSize = PortSize;
  itCalculateSizeInfo(itZeroReplyPortType);
  itCalculateNameInfo(itZeroReplyPortType);
  
  itRealReplyPortType = itAlloc();
  itRealReplyPortType->itName = "mach_port_t";
  itRealReplyPortType->itInName = MACH_MSG_TYPE_MAKE_SEND_ONCE;
  itRealReplyPortType->itInNameStr = "MACH_MSG_TYPE_MAKE_SEND_ONCE";
  itRealReplyPortType->itOutName = MACH_MSG_TYPE_PORT_SEND_ONCE;
  itRealReplyPortType->itOutNameStr = "MACH_MSG_TYPE_PORT_SEND_ONCE";
  itRealReplyPortType->itSize = PortSize;
  itCalculateSizeInfo(itRealReplyPortType);
  itCalculateNameInfo(itRealReplyPortType);
  
  itWaitTimeType = itMakeCountType();
  itMsgOptionType = itMakeCountType();
}

/******************************************************
 *  Make sure return values of functions are assignable.
 ******************************************************/
void
itCheckReturnType(identifier_t name, ipc_type_t *it)
{
  if (!it->itStruct)
    error("type of %s is too complicated", name);
  if ((it->itInName == MACH_MSG_TYPE_POLYMORPHIC) ||
      (it->itOutName == MACH_MSG_TYPE_POLYMORPHIC))
    error("type of %s can't be polymorphic", name);
}


/******************************************************
 *  Called by routine.c to check that request ports are
 *  simple and correct ports with send rights.
 ******************************************************/
void
itCheckRequestPortType(identifier_t name, ipc_type_t *it)
{
  /*    error("Port size = %d %d name = %s\n", PortSize, it->itSize, it->itName);
        error("server = %s user = %x\n",it->itServerType, it->itUserType);
   */
  if (((it->itOutName != MACH_MSG_TYPE_PORT_SEND) &&
       (it->itOutName != MACH_MSG_TYPE_PORT_SEND_ONCE) &&
       (it->itOutName != MACH_MSG_TYPE_POLYMORPHIC)) ||
      (it->itNumber != 1) ||
      (it->itSize != PortSize) ||
      !it->itInLine ||
      !it->itStruct ||
      it->itVarArray)
    error("argument %s isn't a proper request port", name);
}


/******************************************************
 *  Called by routine.c to check that reply ports are
 *  simple and correct ports with send rights.
 ******************************************************/
void
itCheckReplyPortType(identifier_t name, ipc_type_t *it)
{
  if (((it->itOutName != MACH_MSG_TYPE_PORT_SEND) &&
       (it->itOutName != MACH_MSG_TYPE_PORT_SEND_ONCE) &&
       (it->itOutName != MACH_MSG_TYPE_POLYMORPHIC) &&
       (it->itOutName != 0)) ||
      (it->itNumber != 1) ||
      (it->itSize != PortSize) ||
      !it->itInLine ||
      !it->itStruct ||
      it->itVarArray)
    error("argument %s isn't a proper reply port", name);
}


/******************************************************
 *  Used by routine.c to check that WaitTime is a
 *  simple bit machine_integer_bits integer.
 ******************************************************/
void
itCheckIntType(identifier_t name, ipc_type_t *it)
{
  if ((it->itInName != machine_integer_size) ||
      (it->itOutName != machine_integer_size) ||
      (it->itNumber != 1) ||
      (it->itSize != machine_integer_bits) ||
      !it->itInLine ||
      !it->itStruct ||
      it->itVarArray)
    error("argument %s isn't a proper integer", name);
}

void
itCheckTokenType(identifier_t name, ipc_type_t *it)
{
  if (it->itMigInLine || it->itNoOptArray || it->itString ||
      it->itTypeSize != 8 || !it->itInLine || !it->itStruct ||
      it->itVarArray || it->itPortType)
    error("argument %s isn't a proper Token", name);
  
}

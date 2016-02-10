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
/*  HISTORY
 * 07-Apr-89  Richard Draves (rpd) at Carnegie-Mellon University
 * Extensive revamping.  Added polymorphic arguments.
 * Allow multiple variable-sized inline arguments in messages.
 *
 * 16-Nov-87  David Golub (dbg) at Carnegie-Mellon University
 * Changed itVarArrayDecl to take a 'max' parameter.
 * Added itDestructor.
 *
 *  18-Aug-87 Mary Thompson @ Carnegie Mellon
 * Added itPortType
 * Added itTidType
 */

#ifndef _TYPE_H
#define _TYPE_H

#include <mach/boolean.h>
#include "strdefs.h"

#ifdef linux
#include <linux/types.h>
#else /* linux */
#include <sys/types.h>
#endif /* linux */
typedef u_int ipc_flags_t;

/*
 * MIG built-in types
 */
#define MACH_MSG_TYPE_UNSTRUCTURED      0
#define MACH_MSG_TYPE_BIT               0
#define MACH_MSG_TYPE_BOOLEAN           0
#define MACH_MSG_TYPE_INTEGER_8         9
#define MACH_MSG_TYPE_INTEGER_16        1
#define MACH_MSG_TYPE_INTEGER_32        2
#define MACH_MSG_TYPE_INTEGER_64        3
#define MACH_MSG_TYPE_CHAR              8
#define MACH_MSG_TYPE_BYTE              9
#define MACH_MSG_TYPE_REAL_32           10
#define MACH_MSG_TYPE_REAL_64           11
#define MACH_MSG_TYPE_STRING            12
#define MACH_MSG_TYPE_STRING_C          12

#define flNone          (0x00)
#define flPhysicalCopy  (0x01) /* Physical Copy specified */
#define flOverwrite     (0x02) /* Overwrite mode specified */
#define flDealloc       (0x04) /* Dealloc specified */
#define flNotDealloc    (0x08) /* NotDealloc specified */
#define flMaybeDealloc  (0x10) /* Dealloc[] specified */
#define flSameCount     (0x20) /* SamCount specified, used by co-bounded arrays */
#define flCountInOut    (0x40) /* CountInOut specified */
#define flRetCode       (0x80) /* RetCode specified */
#define flAuto          (0x100) /* Will not be referenced by server after RPC */
#define flConst         (0x200) /* Will not be modified by server during RPC */

typedef enum dealloc {
 d_NO, /* do not deallocate */
 d_YES, /* always deallocate */
 d_MAYBE /* deallocate according to parameter */
} dealloc_t;

/* Convert dealloc_t to TRUE/FALSE */
#define strdealloc(d) (strbool(d == d_YES))

/*
 * itName and itNext are internal fields (not used for code generation).
 * They are only meaningful for types entered into the symbol table.
 * The symbol table is a simple self-organizing linked list.
 *
 * The function itCheckDecl checks & fills in computed information.
 * Every type actually used (pointed at by argType) is so processed.
 *
 * The itInName, itOutName, itSize, itNumber, fields correspond directly
 * to mach_msg_type_t fields.
 * For out-of-line variable sized types, itNumber is zero.  For
 * in-line variable sized types, itNumber is the maximum size of the
 * array.  itInName is the name value supplied to the kernel,
 * and itOutName is the name value received from the kernel.
 * When the type describes a MACH port, either or both may be
 * MACH_MSG_TYPE_POLYMORPHIC, indicating a "polymorphic" name.
 * For itInName, this means the user supplies the value with an argument.
 * For itOutName, this means the value is returned in an argument.
 *
 * The itInNameStr and itOutNameStr fields contain "printing" versions
 * of the itInName and itOutName values.  The mapping from number->string
 * is not into (eg, MACH_MSG_TYPE_UNSTRUCTURED/MACH_MSG_TYPE_BOOLEAN/
 * MACH_MSG_TYPE_BIT).  These fields are used for code-generation and
 * pretty-printing.
 *
 * itTypeSize is the calculated size of the C type, in bytes.
 * itPadSize is the size of any padded needed after the data field.
 * itMinTypeSize is the minimum size of the data field, including padding.
 * For variable-length inline data, it is zero.
 *
 * itUserType, itServerType, itTransType are the C types used in
 * code generation.  itUserType is the C type passed to the user-side stub
 * and used for msg declarations in the user-side stub.  itServerType
 * is the C type used for msg declarations in the server-side stub.
 * itTransType is the C type passed to the server function by the
 * server-side stub.  Normally it differs from itServerType only when
 * translation functions are defined.
 *
 * itInTrans and itOutTrans are translation functions.  itInTrans
 * takes itServerType values and returns itTransType values.  itOutTrans
 * takes itTransType vaulues and returns itServerType values.
 * itDestructor is a finalization function applied to In arguments
 * after the server-side stub calls the server function.  It takes
 * itTransType values.  Any combination of these may be defined.
 *
 * The following type specification syntax modifies these values:
 * type new = old
 * ctype: name // itUserType and itServerType
 * cusertype: itUserType
 * cservertype: itServerType
 * intran: itTransType itInTrans(itServerType)
 * outtran: itServerType itOutTrans(itTransType)
 * destructor: itDestructor(itTransType);
 *
 * At most one of itStruct and itString should be TRUE.  If both are
 * false, then this is assumed to be an array type (msg data is passed
 * by reference).  If itStruct is TRUE, then msg data is passed by value
 * and can be assigned with =.  If itString is TRUE, then the msg_data
 * is a null-terminated string, assigned with strncpy.  The itNumber
 * value is a maximum length for the string; the msg field always
 * takes up this much space.
 * NoOptArray has been introduced for the cases where the special
 * code generated for array assignments would not work (either because
 * there is  not a ctype (array of automagically generated MiG variables)
 * or because we need to reference the single elements of the array
 * (array of variable sized ool regions).
 *
 * itVarArray means this is a variable-sized array.  If it is inline,
 * then itStruct and itString are FALSE.  If it is out-of-line, then
 * itStruct is TRUE (because pointers can be assigned).
 *
 * itMigInLine means this is an indefinite-length array. Although the
 * argument was not specified as out-of-line, MIG will send it anyway
 * os an out-of-line.
 *
 * itUserKPDType (itServerKPDType) identify the type of Kernel Processed
 * Data that we must deal with: it can be either "mach_msg_port_descriptor_t"
 * or "mach_msg_ool_ports_descriptor_t" or "mach_msg_ool_descriptor_t".
 *
 * itKPD_Number is used any time a single argument require more than
 * one Kernel Processed Data entry: i.e., an in-line array of ports, an array
 * of pointers (out-of-line data)
 *
 * itElement points to any substructure that the type may have.
 * It is only used with variable-sized array types.
 */

typedef struct ipc_type
  {
    identifier_t itName;      /* Mig's name for this type */
    struct ipc_type *itNext;  /* next type in symbol table */
    
    u_int itTypeSize;         /* size of the C type */
    u_int itPadSize;          /* amount of padding after data */
    u_int itMinTypeSize;      /* minimal amount of space occupied by data */
    
    u_int itInName;           /* name supplied to kernel in sent msg */
    u_int itOutName;          /* name in received msg */
    u_int itSize;
    u_int itNumber;
    u_int itKPD_Number;       /* number of Kernel Processed Data entries */
    boolean_t itInLine;
    boolean_t itMigInLine;    /* MiG presents data as InLine, although it is sent OOL */
    boolean_t itPortType;
    
    string_t itInNameStr;     /* string form of itInName */
    string_t itOutNameStr;    /* string form of itOutName */
    
    boolean_t itStruct;
    boolean_t itString;
    boolean_t itVarArray;
    boolean_t itNoOptArray;
    boolean_t itNative;       /* User specified a native (C) type. */
    boolean_t itNativePointer;/* The user will pass a pointer to the */
    /* native C type. */
    
    struct ipc_type *itElement; /* may be NULL */
    
    identifier_t itUserType;
    identifier_t itServerType;
    identifier_t itTransType;
    
    identifier_t itKPDType;   /* descriptors for KPD type of arguments */
    
    identifier_t itInTrans;   /* may be NULL */
    identifier_t itOutTrans;  /* may be NULL */
    identifier_t itDestructor;/* may be NULL */
    identifier_t itBadValue;   /* Excluded value for PointerToIfNot.  May
     be NULL. */
  } ipc_type_t;

#define itNULL ((ipc_type_t *) 0)

#define itWordAlign     sizeof(natural_t)

extern ipc_type_t *itLookUp(/* identifier_t name */);
extern void itInsert(/* identifier_t name, ipc_type_t *it */);
extern void itTypeDecl(/* identifier_t name, ipc_type_t *it */);

extern ipc_type_t *itShortDecl(/* u_int inname, string_t instr,
                                  u_int outname, string_t outstr,
                                  u_int dfault */);
extern ipc_type_t *itPrevDecl(/* identifier_t name */);
extern ipc_type_t *itResetType(/* ipc_type_t *it */);
extern ipc_type_t *itVarArrayDecl(/* u_int number, ipc_type_t *it */);
extern ipc_type_t *itArrayDecl(/* u_int number, ipc_type_t *it */);
extern ipc_type_t *itPtrDecl(/* ipc_type_t *it */);
extern ipc_type_t *itStructDecl(/* u_int number, ipc_type_t *it */);
extern ipc_type_t *itCStringDecl(/* u_int number, boolean_t varying */);
extern ipc_type_t *itNativeType(/* identifier_t CType, boolean_t pointer
                                   identifier_t NotVal */);

extern ipc_type_t *itRetCodeType;
extern ipc_type_t *itNdrCodeType;
extern ipc_type_t *itDummyType;
extern ipc_type_t *itTidType;
extern ipc_type_t *itRequestPortType;
extern ipc_type_t *itZeroReplyPortType;
extern ipc_type_t *itRealReplyPortType;
extern ipc_type_t *itWaitTimeType;
extern ipc_type_t *itMsgOptionType;
extern ipc_type_t *itMakeCountType();
extern ipc_type_t *itMakeSubCountType();
extern ipc_type_t *itMakePolyType();
extern ipc_type_t *itMakeDeallocType();

extern void init_type();

extern void itCheckReturnType(/* identifier_t name, ipc_type_t *it */);
extern void itCheckRequestPortType(/* identifier_t name, ipc_type_t *it */);
extern void itCheckReplyPortType(/* identifier_t name, ipc_type_t *it */);
extern void itCheckIntType(/* identifier_t name, ipc_type_t *it */);
extern void itCheckTokenType(/* identifier_t name, ipc_type_t *it */);

#endif /* _TYPE_H */

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

#include <assert.h>

#ifndef _ROUTINE_H
#define _ROUTINE_H

#include "type.h"
#include <mach/message.h>
#include <mach/boolean.h>
#include <sys/types.h>

/* base kind arg */
#define akeNone         (0)
#define akeNormal       (1)   /* a normal, user-defined argument */
#define akeRequestPort  (2)   /* pointed at by rtRequestPort */
#define akeWaitTime     (3)   /* pointed at by rtWaitTime */
#define akeReplyPort    (4)   /* pointed at by rtReplyPort */
#define akeMsgOption    (5)   /* pointed at by rtMsgOption */
#define akeMsgSeqno     (6)   /* pointed at by rtMsgSeqno */
#define akeRetCode      (7)   /* pointed at by rtRetCode */
#define akeNdrCode      (8)   /* pointed at by rtNdrCode */
#define akeCount        (9)   /* a count arg for argParent */
#define akePoly         (10)  /* a poly arg for argParent */
#define akeDealloc      (11)  /* a deallocate arg for argParent */
#define akeCountInOut   (12)  /* a count-in-out arg */
#define akeSameCount    (13)  /* a samecount case: in fact, a no count! */
#define akeSubCount     (14)  /* a array of array case: subordinate arrays count */
#define akeImplicit     (15)  /* an implicit argument, from the trailer */
#define akeSecToken     (16)  /* an argument from the trailer: the security token */
#define akeAuditToken   (17)  /* an argument from the trailer: the audit token */
#define akeContextToken (18)  /* an argument from the trailer: the context token */
#define akeSendTime     (19)  /* pointed at by rtWaitTime */

#define akeBITS           (0x0000003f)
#define akbRequest        (0x00000040)  /* has a msg_type in request */
#define akbReply          (0x00000080)  /* has a msg_type in reply */
#define akbUserArg        (0x00000100)  /* an arg on user-side */
#define akbServerArg      (0x00000200)  /* an arg on server-side  */
#define akbSend           (0x00000400)  /* value carried in request */
#define akbSendBody       (0x00000800)  /* value carried in request body */
#define akbSendSnd        (0x00001000)  /* value stuffed into request */
#define akbSendRcv        (0x00002000)  /* value grabbed from request */
#define akbReturn         (0x00004000)  /* value carried in reply */
#define akbReturnBody     (0x00008000)  /* value carried in reply body */
#define akbReturnSnd      (0x00010000)  /* value stuffed into reply */
#define akbReturnRcv      (0x00020000)  /* value grabbed from reply */
#define akbReturnNdr      (0x00040000)  /* needs NDR conversion in reply */
#define akbReplyInit      (0x00080000)  /* reply value doesn't come from target routine */
#define akbReplyCopy      (0x00200000)  /* copy reply value from request */
#define akbVarNeeded      (0x00400000)  /* may need local var in server */
#define akbDestroy        (0x00800000)  /* call destructor function */
#define akbVariable       (0x01000000)  /* variable size inline data */
#define akbSendNdr        (0x04000000)  /* needs NDR conversion in request */
#define akbSendKPD        (0x08000000)  /* the arg is sent in the Kernel Processed Data
                                            section of the Request message */
#define akbReturnKPD      (0x10000000)  /* the arg is sent in the Kernel Processed Data
                                            section of the Reply message */
#define akbUserImplicit   (0x20000000)  /* the arg is Impl */
#define akbServerImplicit (0x40000000)  /* the arg is Impl */
#define akbOverwrite      (0x80000000)
/* be careful, there aren't many bits left */

typedef u_int  arg_kind_t;

/*
 * akbRequest means msg_type/data fields are allocated in the request
 * msg.  akbReply means msg_type/data fields are allocated in the
 * reply msg.  These bits * control msg structure declarations packing,
 * and checking of mach_msg_type_t fields.
 *
 * akbUserArg means this argument is an argument to the user-side stub.
 * akbServerArg means this argument is an argument to
 * the server procedure called by the server-side stub.
 *
 * The akbSend* and akbReturn* bits control packing/extracting values
 * in the request and reply messages.
 *
 * akbSend means the argument's value is carried in the request msg.
 * akbSendBody implies akbSend; the value is carried in the msg body.
 * akbSendKPD is the equivalent of akbSendBody for Kernel Processed Data.
 * akbSendSnd implies akbSend; the value is stuffed into the request.
 * akbSendRcv implies akbSend; the value is pulled out of the request.
 *
 * akbReturn, akbReturnBody, akbReturnSnd, akbReturnRcv are defined
 * similarly but apply to the reply message.
 *
 * User-side code generation (header.c, user.c) and associated code
 * should use akbSendSnd and akbReturnRcv, but not akbSendRcv and
 * akbReturnSnd.  Server-side code generation (server.c) is reversed.
 * Code generation should use the more specific akb{Send,Return}{Snd,Rcv}
 * bits when possible, instead of akb{Send,Return}.
 *
 * Note that akRetCode and akReturn lack any Return bits, although
 * there is a value in the msg.  These guys are packed/unpacked
 * with special code, unlike other arguments.
 *
 * akbReplyInit implies akbReply.  It means the server-side stub
 * should initialize the field, because its value does not come
 * from the execution of the target routine: the setting of the
 * NDR record is the sole example (at the moment) of use of this flag.
 *
 * akbVariable means the argument has variable-sized inline data.
 * It isn't currently used for code generation, but routine.c
 * does use it internally.  It is added in rtAugmentArgKind.
 *
 * akbReplyCopy and akbVarNeeded help control code generation in the
 * server-side stub.  The preferred method of handling data in the
 * server-side stub avoids copying into/out-of local variables.  In
 * arguments get passed directly to the server proc from the request msg.
 * Out arguments get stuffed directly into the reply msg by the server proc.
 * For InOut arguments, the server proc gets the address of the data in
 * the request msg, and the resulting data gets copied to the reply msg.
 * Some arguments need a local variable in the server-side stub.  The
 * code extracts the data from the request msg into the variable, and
 * stuff the reply msg from the variable.
 *
 * akbReplyCopy implies akbReply.  It means the data should get copied
 * from the request msg to the reply msg after the server proc is called.
 * It is only used by akInOut.  akTid doesn't need it because the tid
 * data in the reply msg is initialized in the server demux function.
 *
 * akbVarNeeded means the argument needs a local variable in the
 * server-side stub.  It is added in rtAugmentArgKind and
 * rtCheckVariable.  An argument shouldn't have all three of
 * akbReturnSnd, akbVarNeeded and akbReplyCopy, because this indicates
 * the reply msg should be stuffed both ways.
 *
 * akbDestroy helps control code generation in the server-side stub.
 * It means this argument has a destructor function which should be called.
 *
 * akbOverwrite is used to identify the arguments that have to put an entry in
 * the scatter list (the message-template used by the User stub to specify
 * where the out-of-line data sent by server has to land).
 *
 * akbUserImplicit (akbServerImplicit) is used to mark the arguments that
 * correspond to implicit data (data generated by the kernel and inserted in
 * the trailer).
 *
 * Header file generation (header.c) uses:
 *  akbUserArg
 *
 * User stub generation (user.c) uses:
 *  akbUserArg, akbRequest, akbReply, akbSendSnd,
 *  akbSendBody, akbSendKPD, akbReturnRcv, akbOverwrite, akbUserImplicit
 *
 * Server stub generation (server.c) uses:
 *  akbServerArg, akbRequest, akbReply, akbSendRcv, akbReturnSnd,
 *  akbReplyCopy, akbVarNeeded, akbSendBody, akbServerImplicit
 *
 *
 * During code generation, the routine, argument, and type data structures
 * are read-only.  The code generation functions' output is their only
 * side-effect.
 *
 *
 * Style note:
 * Code can use logical operators (|, &, ~) on akb values.
 * ak values should be manipulated with the ak functions.
 */

/* various useful combinations */

#define akbNone (0)
#define akbAll  (~akbNone)
#define akbAllBits  (~akeBITS)

#define akbSendBits (akbSend|akbSendBody|akbSendSnd|akbSendRcv)
#define akbReturnBits (akbReturn|akbReturnBody|akbReturnSnd|akbReturnRcv)
#define akbSendReturnBits (akbSendBits|akbReturnBits)

#define akNone  akeNone

#define akIn  akAddFeature(akeNormal, \
  akbUserArg|akbServerArg|akbRequest|akbSendBits)

#define akOut akAddFeature(akeNormal, \
  akbUserArg|akbServerArg|akbReply|akbReturnBits)

#define akServerImpl  akAddFeature(akeImplicit, \
  akbServerArg|akbServerImplicit|akbSend|akbSendRcv)
#define akUserImpl  akAddFeature(akeImplicit, \
  akbUserArg|akbUserImplicit|akbReturn|akbReturnRcv)

#define akServerSecToken akAddFeature(akeSecToken, \
  akbServerArg|akbServerImplicit|akbSend|akbSendRcv)
#define akUserSecToken akAddFeature(akeSecToken, \
  akbUserArg|akbUserImplicit|akbReturn|akbReturnRcv)
#define akSecToken akAddFeature(akeSecToken, \
        akbServerArg|akbServerImplicit|akbSend|akbSendRcv| \
        akbUserArg|akbUserImplicit|akbReturn|akbReturnRcv)

#define akServerAuditToken akAddFeature(akeAuditToken, \
  akbServerArg|akbServerImplicit|akbSend|akbSendRcv)
#define akUserAuditToken akAddFeature(akeAuditToken, \
  akbUserArg|akbUserImplicit|akbReturn|akbReturnRcv)
#define akAuditToken akAddFeature(akeAuditToken, \
        akbServerArg|akbServerImplicit|akbSend|akbSendRcv| \
        akbUserArg|akbUserImplicit|akbReturn|akbReturnRcv)

#define akServerContextToken akAddFeature(akeContextToken, \
  akbServerArg|akbServerImplicit|akbSend|akbSendRcv)

#define akMsgSeqno  akAddFeature(akeMsgSeqno, \
  akbServerArg|akbServerImplicit|akbSend|akbSendRcv)

#define akInOut akAddFeature(akeNormal, \
  akbUserArg|akbServerArg|akbRequest|akbReply|  \
  akbSendBits|akbReturnBits|akbReplyCopy)

#define akRequestPort akAddFeature(akeRequestPort,  \
  akbUserArg|akbServerArg|akbSend|akbSendSnd|akbSendRcv)

#define akWaitTime  akAddFeature(akeWaitTime, akbUserArg)

#define akSendTime  akAddFeature(akeSendTime, akbUserArg)

#define akMsgOption akAddFeature(akeMsgOption, akbUserArg)

#define akReplyPort akAddFeature(akeReplyPort,  \
  akbUserArg|akbServerArg|akbSend|akbSendSnd|akbSendRcv)

#define akUReplyPort  akAddFeature(akeReplyPort,  \
  akbUserArg|akbSend|akbSendSnd|akbSendRcv)

#define akSReplyPort  akAddFeature(akeReplyPort,  \
  akbServerArg|akbSend|akbSendSnd|akbSendRcv)

#define akRetCode akAddFeature(akeRetCode, akbReply|akbReturnBody)

#define akCount akAddFeature(akeCount,  \
  akbUserArg|akbServerArg)

#define akPoly  akePoly

#define akDealloc akAddFeature(akeDealloc, akbUserArg)

#define akCountInOut  akAddFeature(akeCountInOut, akbRequest|akbSendBits)

#define akCheck(ak, bits) ((ak) & (bits))
#define akCheckAll(ak, bits)  (akCheck(ak, bits) == (bits))
#define akAddFeature(ak, bits)  ((ak)|(bits))
#define akRemFeature(ak, bits)  ((ak)&~(bits))
#define akIdent(ak) ((ak) & akeBITS)

#define argIsIn(arg)  (akIdent(arg->argKind) == akeNormal && \
  akCheck(arg->argKind, akbRequest))
#define argIsOut(arg) (akIdent(arg->argKind) == akeNormal && \
  akCheck(arg->argKind, akbReply))

/*
 * The arguments to a routine/function are linked in left-to-right order.
 * argName is used for error messages and pretty-printing,
 * not code generation.  Code generation shouldn't make any assumptions
 * about the order of arguments, esp. count and poly arguments.
 * (Unfortunately, code generation for inline variable-sized arguments
 * does make such assumptions.)
 *
 * argVarName is the name used in generated code for function arguments
 * and local variable names.  argMsgField is the name used in generated
 * code for the field in msgs where the argument's value lives.
 * argTTName is the name used in generated code for msg-type fields and
 * static variables used to initialize those fields.  argPadName is the
 * name used in generated code for a padding field in msgs.
 *
 * argFlags can be used to override the deallocate bits
 * in the argument's type.  rtProcessArgFlags sets argDeallocate
 * from it and the type.  Code generation shouldn't use
 * argFlags.
 *
 * argCount, argPoly, and argDealloc get to the implicit count, poly,
 * and dealloc arguments associated with the argument; they should be
 * used instead of argNext.  In these implicit arguments, argParent is
 * a pointer to the "real" arg.
 *
 * In count arguments, argMultiplier is a scaling factor applied to
 * the count arg's value to get msg-type-number.  It is equal to
 *  argParent->argType->itElement->itNumber
 *
 */

typedef struct argument
{
    /* if argKind == akReturn, then argName is name of the function */
    identifier_t argName;
    struct argument *argNext;

    arg_kind_t argKind;
    ipc_type_t *argType;
    /* Kernel Processed Data */
    mach_msg_descriptor_type_t argKPD_Type;   /* KPD type: port, ool, port+ool */
    void  (* argKPD_Template)();              /* KPD discipline for static templates */
    void  (* argKPD_Init)();                  /* KPD discipline for initializing */
    void  (* argKPD_Pack)();                  /* KPD discipline for packing */
    void  (* argKPD_Extract)();               /* KPD discipline for extracting */
    void  (* argKPD_TypeCheck)();             /* KPD discipline for type checking */

    string_t argVarName;  /* local variable and argument names */
    string_t argMsgField; /* message field's name */
    string_t argTTName;   /* name for msg_type fields, static vars */
    string_t argPadName;  /* name for pad field in msg */
    string_t argSuffix;   /* name extension for KPDs */

    ipc_flags_t argFlags;
    dealloc_t argDeallocate;  /* overrides argType->itDeallocate */
    boolean_t argCountInOut;

    struct routine *argRoutine; /* routine we are part of */

    struct argument *argCount;    /* our count arg, if present */
    struct argument *argSubCount; /* our sub-count arg, if present (variable subordinate arrays) */
    struct argument *argCInOut;   /* our CountInOut arg, if present */
    struct argument *argPoly;     /* our poly arg, if present */
    struct argument *argDealloc;  /* our dealloc arg, if present */
    struct argument *argSameCount;  /* the arg to take the count from, if present */
    struct argument *argParent;   /* in a count or poly arg, the base arg */
    u_int argMultiplier;          /* for Count argument: parent is a multiple
                                      of a basic IPC type.  Argument must be
                                      multiplied by Multiplier to get IPC
                                      number-of-elements. */

    /* how variable/inline args precede this one, in request and reply */
    u_int argRequestPos;
    u_int argReplyPos;
    /* whether argument is by reference, on user and server side */
    boolean_t argByReferenceUser;
    boolean_t argByReferenceServer;

    boolean_t argTempOnStack; /* A temporary for the short-circuiting
                               * code when -maxonstack is used.
                               */
} argument_t;

/*
 * The various routine kinds' peculiarities are abstracted by rtCheckRoutine
 * into attributes like rtOneWay, etc.  These are what
 * code generation should use.  It is Bad Form for code generation to
 * test rtKind.
 */

typedef enum
{
    rkRoutine,
    rkSimpleRoutine
} routine_kind_t;

typedef struct routine
{
    identifier_t rtName;
    routine_kind_t rtKind;
    argument_t *rtArgs;
    u_int rtNumber; /* used for making msg ids */

    identifier_t rtUserName;    /* user-visible name (UserPrefix + Name) */
    identifier_t rtServerName;  /* server-side name (ServerPrefix + Name) */

    identifier_t rtErrorName;   /* error-handler name */

    boolean_t rtOneWay;         /* SimpleRoutine */

    boolean_t rtSimpleRequest;
    boolean_t rtSimpleReply;

    u_int rtNumRequestVar;      /* number of variable/inline args in request */
    u_int rtNumReplyVar;        /* number of variable/inline args in reply */

    u_int rtMaxRequestPos;      /* maximum of argRequestPos */
    u_int rtMaxReplyPos;        /* maximum of argReplyPos */

    u_int rtRequestKPDs;        /* number of Kernel Processed Data entries */
    u_int rtReplyKPDs;          /* number of Kernel Processed Data entries */
    u_int rtOverwrite;          /* number of Overwrite entries */
    u_int rtOverwriteKPDs;      /* number of entries in the Overwrite template */

    boolean_t rtNoReplyArgs;    /* if so, no reply message arguments beyond
                                   what the server dispatch routine inserts */

    boolean_t rtRequestFits;    /* Request fits within onstack limit */
    boolean_t rtReplyFits;      /* Reply fits within onstack limit */
    boolean_t rtRequestUsedLimit;/* User type limit used in deciding whether
                                    request fits within onstack limit */
    boolean_t rtReplyUsedLimit; /* User type limit used in deciding whether
                                   reply fits within onstack limit */
    u_int rtRequestSizeKnown;   /* Max size of known portion of request */
    u_int rtReplySizeKnown;     /* Max size of known portion of request */

    u_int rtServerImpl;         /* Implicit data requested */
    u_int rtUserImpl;           /* Implicit data requested */

    /* distinguished arguments */
    argument_t *rtRetCArg;      /* the Routine has this argument tagged as RetCode */
    argument_t *rtRequestPort;  /* always non-NULL, defaults to first arg */
    argument_t *rtReplyPort;    /* always non-NULL, defaults to Mig-supplied */
    argument_t *rtRetCode;      /* always non-NULL */
    argument_t *rtNdrCode;      /* always non-NULL */
    argument_t *rtWaitTime;     /* if non-NULL, will use MACH_RCV_TIMEOUT */
    argument_t *rtMsgOption;    /* always non-NULL, defaults to NONE */

    /* more info's used only when UseEventLogger is turned on */
    u_int rtCountPortsIn;    /* how many in-line Ports are sent */
    u_int rtCountOolPortsIn; /* how many out_of-line Ports are sent */
    u_int rtCountOolIn;      /* how many bytes out_of-line are sent */

    u_int rtCountPortsOut;    /* how many in-line Ports are rcv'd */
    u_int rtCountOolPortsOut; /* how many out_of-line Ports are rcv'd */
    u_int rtCountOolOut;      /* how many bytes out_of-line are rcv'd */

    u_int rtTempBytesOnStack; /* A temporary for the short-circuiting
                               * code when -maxonstack is used.
                               */

} routine_t;

#define rtNULL  ((routine_t *) 0)
#define argNULL ((argument_t *) 0)
#define argKPD_NULL ((mach_msg_descriptor_type_t) -1)

#define rtMessOnStack(rt) ((rt)->rtRequestFits && (rt)->rtReplyFits)

/*
 * These are the ways MiG organizes stub parameters
 */
#define IS_VARIABLE_SIZED_UNTYPED(x)  ((x)->itVarArray && \
                                       (x)->itInLine  && \
                                      !(x)->itPortType)
#define IS_KERN_PROC_DATA(x)         (!(x)->itInLine || (x)->itPortType)
#define IS_OPTIONAL_NATIVE(x)        ((x)->itNative && \
                                      (x)->itNativePointer && \
                                      (x)->itBadValue != NULL)

/*
 * I consider the case of fixed/variable bounded arrays of ports or ool or oolport
 */
#define IS_MULTIPLE_KPD(x)  ((x)->itKPD_Number > 1)
/*
 * I consider the case of MiG presenting data as it is inLine, even
 * if it is sent/rcvd as out-of-line
 */
#define IS_MIG_INLINE_EMUL(x)   ((x)->itMigInLine)

extern u_int rtNumber;
/* rt->rtNumber will be initialized */
extern routine_t *rtAlloc();
/* skip a number */
extern void rtSkip();

extern argument_t *argAlloc();

extern boolean_t
rtCheckMask(/* argument_t *args, u_int mask */);

extern boolean_t
rtCheckMaskFunction(/* argument_t *args, u_int mask,
                       boolean_t (*func)(argument_t *arg) */);

extern routine_t *
rtMakeRoutine(/* identifier_t name, argument_t *args */);
extern routine_t *
rtMakeSimpleRoutine(/* identifier_t name, argument_t *args */);

extern void rtPrintRoutine(/* routine_t *rt */);
extern void rtCheckRoutine(/* routine_t *rt */);

extern char *rtRoutineKindToStr(/* routine_kind_t rk */);

extern int rtCountArgDescriptors(/* argument_t *args, int *argcount */);

extern void rtMinRequestSize(/* FILE *file, routine_t *rt, char *str */);
extern void rtMinReplySize(/* FILE *file, routine_t *rt, char *str */);

#define RPCUserStruct(arg)    (arg->argType->itStruct && arg->argType->itInLine)

#define RPCString(arg)        (arg->argType->itString && arg->argType->itInLine)

#define RPCOutStruct(arg)     (arg->argType->itStruct &&\
              argIsOut(arg) && (! arg->argType->itVarArray))
#define RPCOutWord(arg)       (RPCUserStruct(arg) &&\
              (arg->argType->itSize <= 32) &&\
              (arg->argType->itNumber == 1) && argIsOut(arg))

#define RPCPort(arg)          (arg->argKPD_Type == MACH_MSG_PORT_DESCRIPTOR)

#define RPCPortArray(arg)     (arg->argKPD_Type == MACH_MSG_OOL_PORTS_DESCRIPTOR)

#define RPCVariableArray(arg) ((arg->argType->itVarArray) &&\
              !RPCPort(arg) && !RPCPortArray(arg))

#define RPCFixedArray(arg)    (((! arg->argType->itVarArray) &&\
              !RPCPort(arg) && !RPCPortArray(arg) &&\
              (arg->argType->itNumber > 1) &&\
              !RPCUserStruct(arg)) ||\
              RPCString(arg) ||\
              RPCOutWord(arg) ||\
              RPCOutStruct(arg))


#endif  /* _ROUTINE_H */




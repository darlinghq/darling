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

%token	sySkip
%token	syRoutine
%token	sySimpleRoutine

%token	sySubsystem
%token	syKernelUser
%token	syKernelServer

%token	syMsgOption
%token	syMsgSeqno
%token	syWaitTime
%token	sySendTime
%token	syNoWaitTime
%token	syNoSendTime
%token	syErrorProc
%token	syServerPrefix
%token	syUserPrefix
%token	syServerDemux
%token	syRCSId

%token	syImport
%token	syUImport
%token	sySImport
%token  syIImport
%token  syDImport

%token	syIn
%token	syOut
%token	syInOut
%token	syUserImpl
%token	syServerImpl
%token	syRequestPort
%token	syReplyPort
%token	sySReplyPort
%token	syUReplyPort

%token	syType
%token	syArray
%token	syStruct
%token	syOf

%token	syInTran
%token	syOutTran
%token	syDestructor
%token	syCType
%token	syCUserType
%token	syUserTypeLimit
%token	syOnStackLimit
%token	syCServerType
%token  syPointerTo
%token  syPointerToIfNot
%token  syValueOf

%token	syCString
%token	sySecToken
%token	syUserSecToken
%token	syServerSecToken
%token	syAuditToken
%token	syUserAuditToken
%token	syServerAuditToken
%token	syServerContextToken

%token	syColon
%token	sySemi
%token	syComma
%token	syPlus
%token	syMinus
%token	syStar
%token	syDiv
%token	syLParen
%token	syRParen
%token	syEqual
%token	syCaret
%token	syTilde
%token	syLAngle
%token	syRAngle
%token	syLBrack
%token	syRBrack
%token	syBar

%token	syError			/* lex error */

%token	<number>	syNumber
%token	<symtype>	sySymbolicType
%token	<identifier>	syIdentifier
%token	<string>	syString syQString
%token	<string>	syFileName
%token	<flag>		syIPCFlag

%left	syPlus syMinus
%left	syStar syDiv


%type	<statement_kind> ImportIndicant
%type	<number> VarArrayHead ArrayHead StructHead IntExp
%type	<type> NamedTypeSpec TransTypeSpec NativeTypeSpec TypeSpec
%type	<type> CStringSpec
%type	<type> BasicTypeSpec PrevTypeSpec ArgumentType
%type   <identifier> TypePhrase
%type	<symtype> PrimIPCType IPCType
%type	<routine> RoutineDecl Routine SimpleRoutine
%type	<direction> Direction TrImplKeyword
%type	<argument> Argument Trailer Arguments ArgumentList
%type	<flag> IPCFlags

%{

#include <stdio.h>
#include "lexxer.h"
#include "strdefs.h"
#include "type.h"
#include "routine.h"
#include "statement.h"
#include "global.h"
#include "error.h"

static char *import_name();
extern int yylex(void);

/* forward declaration */
void yyerror(char *s);

%}

%union
{
    u_int number;
    identifier_t identifier;
    string_t string;
    statement_kind_t statement_kind;
    ipc_type_t *type;
    struct
    {
      u_int innumber;		/* msgt_name value, when sending */
      string_t instr;
      u_int outnumber;	/* msgt_name value, when receiving */
      string_t outstr;
      u_int size;		/* 0 means there is no default size */
    } symtype;
    routine_t *routine;
    arg_kind_t direction;
    argument_t *argument;
    ipc_flags_t flag;
}

%%

Statements		:	/* empty */
			|	Statements Statement
			;

Statement		:	Subsystem sySemi
			|	WaitTime sySemi
			|	SendTime sySemi
			|	MsgOption sySemi
                        |       UserTypeLimit sySemi
                        |       OnStackLimit sySemi
			|	Error sySemi
			|	ServerPrefix sySemi
			|	UserPrefix sySemi
			|	ServerDemux sySemi
			|	TypeDecl sySemi
			|	RoutineDecl sySemi
{
    register statement_t *st = stAlloc();

    st->stKind = skRoutine;
    st->stRoutine = $1;
    rtCheckRoutine($1);
    if (BeVerbose)
      rtPrintRoutine($1);
}
			|	sySkip sySemi
				{ rtSkip(); }
			|	Import sySemi
			|	RCSDecl sySemi
			|	sySemi
			|	error sySemi
				{ yyerrok; }
			;

Subsystem		:	SubsystemStart SubsystemMods
				SubsystemName SubsystemBase
{
  if (BeVerbose) {
    printf("Subsystem %s: base = %u%s%s\n\n",
    SubsystemName, SubsystemBase,
    IsKernelUser ? ", KernelUser" : "",
    IsKernelServer ? ", KernelServer" : "");
  }
}
			;

SubsystemStart		:	sySubsystem
{
    if (SubsystemName != strNULL) {
      warn("previous Subsystem decl (of %s) will be ignored", SubsystemName);
      IsKernelUser = FALSE;
      IsKernelServer = FALSE;
      strfree(SubsystemName);
    }
}
			;

SubsystemMods		:	/* empty */
			|	SubsystemMods SubsystemMod
			;

SubsystemMod		:	syKernelUser
{
    if (IsKernelUser)
      warn("duplicate KernelUser keyword");
    if (!UseMsgRPC) {
      warn("with KernelUser the -R option is meaningless");
      UseMsgRPC = TRUE;
    }
    IsKernelUser = TRUE;
}
			|	syKernelServer
{
    if (IsKernelServer)
      warn("duplicate KernelServer keyword");
    IsKernelServer = TRUE;
}
			;

SubsystemName		:	syIdentifier	{ SubsystemName = $1; }
			;

SubsystemBase		:	syNumber	{ SubsystemBase = $1; }
			;

MsgOption		:	LookString syMsgOption syString
{
    if (streql($3, "MACH_MSG_OPTION_NONE")) {
      MsgOption = strNULL;
      if (BeVerbose)
          printf("MsgOption: canceled\n\n");
    }
    else {
      MsgOption = $3;
      if (BeVerbose)
          printf("MsgOption %s\n\n",$3);
    }
}
			;

UserTypeLimit           :       syUserTypeLimit syNumber
				{UserTypeLimit = $2; }
                        ;
OnStackLimit            :       syOnStackLimit syNumber
				{MaxMessSizeOnStack = $2; }
                        ;

WaitTime		:	LookString syWaitTime syString
{
    WaitTime = $3;
    if (BeVerbose)
      printf("WaitTime %s\n\n", WaitTime);
}
			|	syNoWaitTime
{
    WaitTime = strNULL;
    if (BeVerbose)
      printf("NoWaitTime\n\n");
}
			;

SendTime		:	LookString sySendTime syString
{
    SendTime = $3;
    if (BeVerbose)
      printf("SendTime %s\n\n", SendTime);
}
			|	syNoSendTime
{
    SendTime = strNULL;
    if (BeVerbose)
      printf("NoSendTime\n\n");
}
			;

Error			:	syErrorProc syIdentifier
{
    ErrorProc = $2;
    if (BeVerbose)
      printf("ErrorProc %s\n\n", ErrorProc);
}
			;

ServerPrefix		:	syServerPrefix syIdentifier
{
    ServerPrefix = $2;
    if (BeVerbose)
      printf("ServerPrefix %s\n\n", ServerPrefix);
}
			;

UserPrefix		:	syUserPrefix syIdentifier
{
    UserPrefix = $2;
    if (BeVerbose)
      printf("UserPrefix %s\n\n", UserPrefix);
}
			;

ServerDemux		:	syServerDemux syIdentifier
{
    ServerDemux = $2;
    if (BeVerbose)
      printf("ServerDemux %s\n\n", ServerDemux);
}
			;

Import			:	LookFileName ImportIndicant syFileName
{
    register statement_t *st = stAlloc();
    st->stKind = $2;
    st->stFileName = $3;

    if (BeVerbose)
      printf("%s %s\n\n", import_name($2), $3);
}
			;

ImportIndicant		:	syImport	{ $$ = skImport; }
			|	syUImport	{ $$ = skUImport; }
			|	sySImport	{ $$ = skSImport; }
			|	syIImport	{ $$ = skIImport; }
			|	syDImport	{ $$ = skDImport; }
			;

RCSDecl			:	LookQString syRCSId syQString
{
    if (RCSId != strNULL)
      warn("previous RCS decl will be ignored");
    if (BeVerbose)
      printf("RCSId %s\n\n", $3);
    RCSId = $3;
}
			;

TypeDecl		:	syType NamedTypeSpec
{
    register identifier_t name = $2->itName;

    if (itLookUp(name) != itNULL)
      warn("overriding previous definition of %s", name);
    itInsert(name, $2);
}
			;

NamedTypeSpec		:	syIdentifier syEqual TransTypeSpec
				{ itTypeDecl($1, $$ = $3); }
			;

TransTypeSpec		:	TypeSpec
				{ $$ = itResetType($1); }
			|	TransTypeSpec syInTran syColon syIdentifier
				syIdentifier syLParen syIdentifier syRParen
{
    $$ = $1;

    if (($$->itTransType != strNULL) && !streql($$->itTransType, $4))
      warn("conflicting translation types (%s, %s)", $$->itTransType, $4);
    $$->itTransType = $4;

    if (($$->itInTrans != strNULL) && !streql($$->itInTrans, $5))
      warn("conflicting in-translation functions (%s, %s)", $$->itInTrans, $5);
    $$->itInTrans = $5;

    if (($$->itServerType != strNULL) && !streql($$->itServerType, $7))
      warn("conflicting server types (%s, %s)", $$->itServerType, $7);
    $$->itServerType = $7;
}
			|	TransTypeSpec syOutTran syColon syIdentifier
				syIdentifier syLParen syIdentifier syRParen
{
    $$ = $1;

    if (($$->itServerType != strNULL) && !streql($$->itServerType, $4))
      warn("conflicting server types (%s, %s)", $$->itServerType, $4);
    $$->itServerType = $4;

    if (($$->itOutTrans != strNULL) && !streql($$->itOutTrans, $5))
      warn("conflicting out-translation functions (%s, %s)", $$->itOutTrans, $5);
    $$->itOutTrans = $5;

    if (($$->itTransType != strNULL) && !streql($$->itTransType, $7))
      warn("conflicting translation types (%s, %s)", $$->itTransType, $7);
    $$->itTransType = $7;
}
			|	TransTypeSpec syDestructor syColon syIdentifier
				syLParen syIdentifier syRParen
{
    $$ = $1;

    if (($$->itDestructor != strNULL) && !streql($$->itDestructor, $4))
      warn("conflicting destructor functions (%s, %s)", $$->itDestructor, $4);
    $$->itDestructor = $4;

    if (($$->itTransType != strNULL) && !streql($$->itTransType, $6))
      warn("conflicting translation types (%s, %s)", $$->itTransType, $6);
    $$->itTransType = $6;
}
			|	TransTypeSpec syCType syColon syIdentifier
{
    $$ = $1;

    if (($$->itUserType != strNULL) && !streql($$->itUserType, $4))
      warn("conflicting user types (%s, %s)", $$->itUserType, $4);
    $$->itUserType = $4;

    if (($$->itServerType != strNULL) && !streql($$->itServerType, $4))
      warn("conflicting server types (%s, %s)", $$->itServerType, $4);
    $$->itServerType = $4;
}
			|	TransTypeSpec syCUserType syColon syIdentifier
{
    $$ = $1;

    if (($$->itUserType != strNULL) && !streql($$->itUserType, $4))
      warn("conflicting user types (%s, %s)", $$->itUserType, $4);
    $$->itUserType = $4;
}
			|	TransTypeSpec syCServerType
				syColon syIdentifier
{
    $$ = $1;

    if (($$->itServerType != strNULL) && !streql($$->itServerType, $4))
	warn("conflicting server types (%s, %s)",
	     $$->itServerType, $4);
    $$->itServerType = $4;
}
			;

TypeSpec		:	BasicTypeSpec
				{ $$ = $1; }
			|	PrevTypeSpec
				{ $$ = $1; }
			|	VarArrayHead TypeSpec
				{ $$ = itVarArrayDecl($1, $2); }
			|	ArrayHead TypeSpec
				{ $$ = itArrayDecl($1, $2); }
			|	syCaret TypeSpec
				{ $$ = itPtrDecl($2); }
			|	StructHead TypeSpec
				{ $$ = itStructDecl($1, $2); }
			|	CStringSpec
				{ $$ = $1; }
                        |       NativeTypeSpec
				{ $$ = $1; }
			;

NativeTypeSpec          :       syPointerTo syLParen TypePhrase syRParen
                                { $$ = itNativeType($3, TRUE, 0); }
                        |       syPointerToIfNot syLParen TypePhrase syComma
                                TypePhrase syRParen
                                { $$ = itNativeType($3, TRUE, $5); }
                        |       syValueOf syLParen TypePhrase syRParen
                                { $$ = itNativeType($3, FALSE, 0); }
			;

BasicTypeSpec		:	IPCType
{
    $$ = itShortDecl($1.innumber, $1.instr,
		     $1.outnumber, $1.outstr,
		     $1.size);
}
			|	syLParen IPCType syComma IntExp
				IPCFlags syRParen
{
    error("Long form type declarations aren't allowed any longer\n");
}
			;

PrimIPCType		:	syNumber
{
    $$.innumber = $$.outnumber = $1;
    $$.instr = $$.outstr = strNULL;
    $$.size = 0;
}
			|	sySymbolicType
				{ $$ = $1; }
			;

IPCType			:	PrimIPCType
				{ $$ = $1; }
			|	PrimIPCType syBar PrimIPCType
{
  if ($1.size != $3.size) {
    if ($1.size == 0)
      $$.size = $3.size;
    else if ($3.size == 0)
      $$.size = $1.size;
    else {
      error("sizes in IPCTypes (%d, %d) aren't equal",
      $1.size, $3.size);
      $$.size = 0;
    }
  }
  else
    $$.size = $1.size;
  $$.innumber = $1.innumber;
  $$.instr = $1.instr;
  $$.outnumber = $3.outnumber;
  $$.outstr = $3.outstr;
}
			;

PrevTypeSpec		:	syIdentifier
				{ $$ = itPrevDecl($1); }
			;

VarArrayHead		:	syArray syLBrack syRBrack syOf
				{ $$ = 0; }
			|	syArray syLBrack syStar syRBrack syOf
				{ $$ = 0; }
			|	syArray syLBrack syStar syColon IntExp
				syRBrack syOf
				{ $$ = $5; }
			;

ArrayHead		:	syArray syLBrack IntExp syRBrack syOf
				{ $$ = $3; }
			;

StructHead		:	syStruct syLBrack IntExp syRBrack syOf
				{ $$ = $3; }
			;

CStringSpec		:	syCString syLBrack IntExp syRBrack
				{ $$ = itCStringDecl($3, FALSE); }
			|	syCString syLBrack syStar syColon
				IntExp syRBrack
				{ $$ = itCStringDecl($5, TRUE); }
			;

TypePhrase              :       syIdentifier
                                { $$ = $1; }
                        |       TypePhrase syIdentifier
                                { $$ = strphrase($1, $2); strfree($2); }
			;

IntExp			: 	IntExp	syPlus	IntExp
				{ $$ = $1 + $3;	}
			| 	IntExp	syMinus	IntExp
				{ $$ = $1 - $3;	}
			|	IntExp	syStar	IntExp
				{ $$ = $1 * $3;	}
			| 	IntExp	syDiv	IntExp
				{ $$ = $1 / $3;	}
			|	syNumber
				{ $$ = $1;	}
			|	syLParen IntExp syRParen
				{ $$ = $2;	}
			;


RoutineDecl		:	Routine			{ $$ = $1; }
			|	SimpleRoutine		{ $$ = $1; }
			;

Routine			:	syRoutine syIdentifier Arguments
				{ $$ = rtMakeRoutine($2, $3); }
			;

SimpleRoutine		:	sySimpleRoutine syIdentifier Arguments
				{ $$ = rtMakeSimpleRoutine($2, $3); }
			;

Arguments		:	syLParen syRParen
				{ $$ = argNULL; }
			|	syLParen ArgumentList syRParen
				{ $$ = $2; }

			;

ArgumentList		:	Argument
				{ $$ = $1; }
			|       Trailer
				{ $$ = $1; }
			|	Argument sySemi ArgumentList
{
  $$ = $1;
  $$->argNext = $3;
}
			|       Trailer sySemi ArgumentList
{
  $$ = $1;
  $$->argNext = $3;
}
			;

Argument		:	Direction syIdentifier ArgumentType IPCFlags
{
  $$ = argAlloc();
  $$->argKind = $1;
  $$->argName = $2;
  $$->argType = $3;
  $$->argFlags = $4;
  if ($3 && $3->itNative) {
      if ($1 != akIn && $1 != akOut && $1 != akInOut)
        error("Illegal direction specified");

      if (!($3->itNativePointer) && $1 != akIn)
        error("ValueOf only valid for in");

      if (($3->itBadValue) != NULL && $1 != akIn)
        error("PointerToIfNot only valid for in");
  }
}
			;

Trailer			:	TrImplKeyword syIdentifier ArgumentType
{
    $$ = argAlloc();
    $$->argKind = $1;
    $$->argName = $2;
    $$->argType = $3;
}
			;


Direction		:	/* empty */	{ $$ = akNone; }
			|	syIn		{ $$ = akIn; }
			|	syOut		{ $$ = akOut; }
			|	syInOut		{ $$ = akInOut; }
			|	syRequestPort	{ $$ = akRequestPort; }
			|	syReplyPort	{ $$ = akReplyPort; }
			|	sySReplyPort	{ $$ = akSReplyPort; }
			|	syUReplyPort	{ $$ = akUReplyPort; }
			|	syWaitTime	{ $$ = akWaitTime; }
			|	sySendTime 	{ $$ = akSendTime; }
			|	syMsgOption	{ $$ = akMsgOption; }
			|	sySecToken		{ $$ = akSecToken; }
			|	syServerSecToken	{ $$ = akServerSecToken; }
			|	syUserSecToken		{ $$ = akUserSecToken; }
			|	syAuditToken		{ $$ = akAuditToken; }
			|	syServerAuditToken	{ $$ = akServerAuditToken; }
			|	syUserAuditToken	{ $$ = akUserAuditToken; }
			|	syServerContextToken	{ $$ = akServerContextToken; }
			|	syMsgSeqno	{ $$ = akMsgSeqno; }
			;



TrImplKeyword		:	syServerImpl	{ $$ = akServerImpl; }
			|	syUserImpl	{ $$ = akUserImpl; }
			;


ArgumentType		:	syColon syIdentifier
{
  $$ = itLookUp($2);
  if ($$ == itNULL)
    error("type '%s' not defined", $2);
}
			|	syColon NamedTypeSpec
				{ $$ = $2; }
                        |       syColon NativeTypeSpec
				{ $$ = $2; }
			;

IPCFlags		:	/* empty */
				{ $$ = flNone; }
			|	IPCFlags syComma syIPCFlag
{
  if ($1 & $3)
    warn("redundant IPC flag ignored");
  else
    $$ = $1 | $3;
}
			|	IPCFlags syComma syIPCFlag syLBrack syRBrack
{
  if ($3 != flDealloc)
    warn("only Dealloc is variable");
  else
    $$ = $1 | flMaybeDealloc;
}

LookString		:	/* empty */
				{ LookString(); }
			;

LookFileName		:	/* empty */
				{ LookFileName(); }
			;

LookQString		:	/* empty */
				{ LookQString(); }
			;

%%

void
yyerror(char *s)
{
  error(s);
}

static char *
import_name(statement_kind_t sk)
{
  switch (sk) {

    case skImport:
      return "Import";

    case skSImport:
      return "SImport";

    case skUImport:
      return "UImport";

    case skIImport:
      return "IImport";

    case skDImport:
      return "DImport";

    default:
      fatal("import_name(%d): not import statement", (int) sk);
      /*NOTREACHED*/
      return strNULL;
  }
}

%k 10000
%n 5000
%a 20000
%e 10000
%p 25000

Ident		([A-Za-z_][A-Za-z_0-9]*)
Number		([0-9]+)
String		([-/._$A-Za-z0-9]+)
QString		(\"[^"\n]*\")
AString		(\<[^>\n]*\>)
FileName	({QString}|{AString})

/* new flex syntax to accomplish the same thing as #define YY_NO_UNPUT */
%option nounput

%{
/*
 * Copyright (c) 1999-2018 Apple, Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * "Portions Copyright (c) 1999, 2008 Apple Inc.  All Rights
 * Reserved.  This file contains Original Code and/or Modifications of
 * Original Code as defined in and that are subject to the Apple Public
 * Source License Version 1.0 (the 'License').  You may not use this file
 * except in compliance with the License.  Please obtain a copy of the
 * License at http://www.apple.com/publicsource and read it before using
 * this file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE OR NON-INFRINGEMENT.  Please see the
 * License for the specific language governing rights and limitations
 * under the License."
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

#include "strdefs.h"
#include "type.h"
#include <sys/types.h>
#include <mach/message.h>
#include <mach/std_types.h>
#include "statement.h"
#include "global.h"
#include "y.tab.h" // was parser.h
#include "lexxer.h"
#include "mig_machine.h"

#define	PortSize	(sizeof (mach_port_t) * NBBY)
#ifdef	__STDC__
#define stringize(x)	#x
#else	/* __STDC__ */
#define	stringize(x)	"x"
#endif	/* __STDC__ */

#ifdef	LDEBUG
#define RETURN(sym)							\
{									\
    printf("yylex: returning '%s' (%d)\n", stringize(sym), (sym));	\
    return (sym);							\
}
#else	/* LDEBUG */
#define RETURN(sym)	return (sym)
#endif	/* LDEBUG */

#define	TPRETURN(intype, outtype, tsize)				\
{									\
    yylval.symtype.innumber = (intype);					\
    yylval.symtype.instr = stringize(intype);				\
    yylval.symtype.outnumber = (outtype);				\
    yylval.symtype.outstr = stringize(outtype);				\
    yylval.symtype.size = (tsize);					\
    RETURN(sySymbolicType);						\
}

#define	TRETURN(type, tsize)	TPRETURN(type,type,tsize)

#define SAVE(s)		do {oldYYBegin = s; BEGIN s; } while (0)

#define RESTORE		BEGIN oldYYBegin

#define	FRETURN(val)							\
{									\
    yylval.flag = (val);						\
    RETURN(syIPCFlag);							\
}

#define YY_NO_UNPUT	/* suppress yyunput generation */

static int	oldYYBegin = 0;

int lineno = 0;			/* Replaces lex yylineno */

static void doSharp(char *); /* process body of # directives */
extern void yyerror(char *);
%}

%Start	Normal String FileName QString SkipToEOL

%%

<Normal>[Rr][Oo][Uu][Tt][Ii][Nn][Ee]		RETURN(syRoutine);
<Normal>[Ss][Ii][Mm][Pp][Ll][Ee][Rr][Oo][Uu][Tt][Ii][Nn][Ee] RETURN(sySimpleRoutine);
<Normal>[Ss][Uu][Bb][Ss][Yy][Ss][Tt][Ee][Mm]	RETURN(sySubsystem);
<Normal>[Mm][Ss][Gg][Oo][Pp][Tt][Ii][Oo][Nn]	RETURN(syMsgOption);
<Normal>[Uu][Ss][Ee][Ss][Pp][Ee][Cc][Ii][Aa][Ll][Rr][Ee][Pp][Ll][Yy][Pp][Oo][Rr][Tt]    RETURN(syUseSpecialReplyPort);
<Normal>[Cc][Oo][Nn][Ss][Uu][Mm][Ee][Oo][Nn][Ss][Ee][Nn][Dd][Ee][Rr][Rr][Oo][Rr]    RETURN(syConsumeOnSendError);
<Normal>[Mm][Ss][Gg][Ss][Ee][Qq][Nn][Oo]	RETURN(syMsgSeqno);
<Normal>[Ww][Aa][Ii][Tt][Tt][Ii][Mm][Ee]	RETURN(syWaitTime);
<Normal>[Ss][Ee][Nn][Dd][Tt][Ii][Mm][Ee]	RETURN(sySendTime);
<Normal>[Nn][Oo][Ww][Aa][Ii][Tt][Tt][Ii][Mm][Ee]	RETURN(syNoWaitTime);
<Normal>[Nn][Oo][Ss][Ee][Nn][Dd][Tt][Ii][Mm][Ee]	RETURN(syNoSendTime);
<Normal>[Ii][Nn]				RETURN(syIn);
<Normal>[Oo][Uu][Tt]				RETURN(syOut);
<Normal>[Uu][Ss][Ee][Rr][Ii][Mm][Pp][Ll]		RETURN(syUserImpl);
<Normal>[Ss][Ee][Rr][Vv][Ee][Rr][Ii][Mm][Pp][Ll]	RETURN(syServerImpl);
<Normal>[Ss][Ee][Cc][Tt][Oo][Kk][Ee][Nn]	RETURN(sySecToken);
<Normal>[Ss][Ee][Rr][Vv][Ee][Rr][Ss][Ee][Cc][Tt][Oo][Kk][Ee][Nn] RETURN(syServerSecToken);
<Normal>[Uu][Ss][Ee][Rr][Ss][Ee][Cc][Tt][Oo][Kk][Ee][Nn]	RETURN(syUserSecToken);
<Normal>[Aa][Uu][Dd][Ii][Tt][Tt][Oo][Kk][Ee][Nn]	RETURN(syAuditToken);
<Normal>[Ss][Ee][Rr][Vv][Ee][Rr][Aa][Uu][Dd][Ii][Tt][Tt][Oo][Kk][Ee][Nn] RETURN(syServerAuditToken);
<Normal>[Uu][Ss][Ee][Rr][Aa][Uu][Dd][Ii][Tt][Tt][Oo][Kk][Ee][Nn]	RETURN(syUserAuditToken);
<Normal>[Ss][Ee][Rr][Vv][Ee][Rr][Cc][Oo][Nn][Tt][Ee][Xx][Tt][Tt][Oo][Kk][Ee][Nn]	RETURN(syServerContextToken);
<Normal>[Ii][Nn][Oo][Uu][Tt]			RETURN(syInOut);
<Normal>[Rr][Ee][Qq][Uu][Ee][Ss][Tt][Pp][Oo][Rr][Tt]	RETURN(syRequestPort);
<Normal>[Rr][Ee][Pp][Ll][Yy][Pp][Oo][Rr][Tt]		RETURN(syReplyPort);
<Normal>[Uu][Rr][Ee][Pp][Ll][Yy][Pp][Oo][Rr][Tt]	RETURN(syUReplyPort);
<Normal>[Ss][Rr][Ee][Pp][Ll][Yy][Pp][Oo][Rr][Tt]	RETURN(sySReplyPort);
<Normal>[Aa][Rr][Rr][Aa][Yy]			RETURN(syArray);
<Normal>[Oo][Ff]				RETURN(syOf);
<Normal>[Ee][Rr][Rr][Oo][Rr]			RETURN(syErrorProc);
<Normal>[Ss][Ee][Rr][Vv][Ee][Rr][Pp][Rr][Ee][Ff][Ii][Xx] RETURN(syServerPrefix);
<Normal>[Uu][Ss][Ee][Rr][Pp][Rr][Ee][Ff][Ii][Xx]	RETURN(syUserPrefix);
<Normal>[Ss][Ee][Rr][Vv][Ee][Rr][Dd][Ee][Mm][Uu][Xx]	RETURN(syServerDemux);
<Normal>[Rr][Cc][Ss][Ii][Dd]			RETURN(syRCSId);
<Normal>[Ii][Mm][Pp][Oo][Rr][Tt]		RETURN(syImport);
<Normal>[Uu][Ii][Mm][Pp][Oo][Rr][Tt]		RETURN(syUImport);
<Normal>[Ss][Ii][Mm][Pp][Oo][Rr][Tt]		RETURN(sySImport);
<Normal>[Dd][Ii][Mm][Pp][Oo][Rr][Tt]		RETURN(syDImport);
<Normal>[Ii][Ii][Mm][Pp][Oo][Rr][Tt]		RETURN(syIImport);
<Normal>[Tt][Yy][Pp][Ee]			RETURN(syType);
<Normal>[Kk][Ee][Rr][Nn][Ee][Ll][Ss][Ee][Rr][Vv][Ee][Rr] RETURN(syKernelServer);
<Normal>[Kk][Ee][Rr][Nn][Ee][Ll][Uu][Ss][Ee][Rr]	RETURN(syKernelUser);
<Normal>[Ss][Kk][Ii][Pp]			RETURN(sySkip);
<Normal>[Ss][Tt][Rr][Uu][Cc][Tt]		RETURN(syStruct);
<Normal>[Ii][Nn][Tt][Rr][Aa][Nn]		RETURN(syInTran);
<Normal>[Oo][Uu][Tt][Tt][Rr][Aa][Nn]		RETURN(syOutTran);
<Normal>[Dd][Ee][Ss][Tt][Rr][Uu][Cc][Tt][Oo][Rr]	RETURN(syDestructor);
<Normal>[Cc][Tt][Yy][Pp][Ee]				RETURN(syCType);
<Normal>[Cc][Uu][Ss][Ee][Rr][Tt][Yy][Pp][Ee]		RETURN(syCUserType);
<Normal>[Cc][Ss][Ee][Rr][Vv][Ee][Rr][Tt][Yy][Pp][Ee]	RETURN(syCServerType);
<Normal>[Cc]_[Ss][Tt][Rr][Ii][Nn][Gg]			RETURN(syCString);

<Normal>[Ss][Aa][Mm][Ee][Cc][Oo][Uu][Nn][Tt] 		FRETURN(flSameCount);
<Normal>[Rr][Ee][Tt][Cc][Oo][Dd][Ee] 			FRETURN(flRetCode);
<Normal>[Pp][Hh][Yy][Ss][Ii][Cc][Aa][Ll][Cc][Oo][Pp][Yy] FRETURN(flPhysicalCopy);
<Normal>[Oo][Vv][Ee][Rr][Ww][Rr][Ii][Tt][Ee]	FRETURN(flOverwrite);
<Normal>[Dd][Ee][Aa][Ll][Ll][Oo][Cc]		FRETURN(flDealloc);
<Normal>[Nn][Oo][Tt][Dd][Ee][Aa][Ll][Ll][Oo][Cc] FRETURN(flNotDealloc);
<Normal>[Cc][Oo][Uu][Nn][Tt][Ii][Nn][Oo][Uu][Tt] FRETURN(flCountInOut);
<Normal>[Pp][Oo][Ll][Yy][Mm][Oo][Rr][Pp][Hh][Ii][Cc]	TPRETURN(MACH_MSG_TYPE_POLYMORPHIC, MACH_MSG_TYPE_POLYMORPHIC, PortSize);
<Normal>[Aa][Uu][Tt][Oo]			FRETURN(flAuto);
<Normal>[Cc][Oo][Nn][Ss][Tt]			FRETURN(flConst);
<Normal>"PointerTo"				RETURN(syPointerTo);
<Normal>"PointerToIfNot"		      	RETURN(syPointerToIfNot);
<Normal>"ValueOf"				RETURN(syValueOf);
<Normal>"UserTypeLimit"				RETURN(syUserTypeLimit);
<Normal>"OnStackLimit"                          RETURN(syOnStackLimit);


<Normal>"MACH_MSG_TYPE_UNSTRUCTURED"	TRETURN(MACH_MSG_TYPE_UNSTRUCTURED,0);
<Normal>"MACH_MSG_TYPE_BIT"		TRETURN(MACH_MSG_TYPE_BIT,1);
<Normal>"MACH_MSG_TYPE_BOOLEAN"		TRETURN(MACH_MSG_TYPE_BOOLEAN,32);
<Normal>"MACH_MSG_TYPE_INTEGER_8"	TRETURN(MACH_MSG_TYPE_INTEGER_8,8);
<Normal>"MACH_MSG_TYPE_INTEGER_16"	TRETURN(MACH_MSG_TYPE_INTEGER_16,16);
<Normal>"MACH_MSG_TYPE_INTEGER_32"	TRETURN(MACH_MSG_TYPE_INTEGER_32,32);
<Normal>"MACH_MSG_TYPE_INTEGER_64"	TRETURN(MACH_MSG_TYPE_INTEGER_64,64);
<Normal>"MACH_MSG_TYPE_REAL_32"		TRETURN(MACH_MSG_TYPE_REAL_32,32);
<Normal>"MACH_MSG_TYPE_REAL_64"		TRETURN(MACH_MSG_TYPE_REAL_64,64);
<Normal>"MACH_MSG_TYPE_CHAR"		TRETURN(MACH_MSG_TYPE_CHAR,8);
<Normal>"MACH_MSG_TYPE_BYTE"		TRETURN(MACH_MSG_TYPE_BYTE,8);

<Normal>"MACH_MSG_TYPE_MOVE_RECEIVE"	TPRETURN(MACH_MSG_TYPE_MOVE_RECEIVE,MACH_MSG_TYPE_PORT_RECEIVE,PortSize);
<Normal>"MACH_MSG_TYPE_COPY_SEND"	TPRETURN(MACH_MSG_TYPE_COPY_SEND,MACH_MSG_TYPE_PORT_SEND,PortSize);
<Normal>"MACH_MSG_TYPE_MAKE_SEND"	TPRETURN(MACH_MSG_TYPE_MAKE_SEND,MACH_MSG_TYPE_PORT_SEND,PortSize);
<Normal>"MACH_MSG_TYPE_MOVE_SEND"	TPRETURN(MACH_MSG_TYPE_MOVE_SEND,MACH_MSG_TYPE_PORT_SEND,PortSize);
<Normal>"MACH_MSG_TYPE_MAKE_SEND_ONCE"	TPRETURN(MACH_MSG_TYPE_MAKE_SEND_ONCE,MACH_MSG_TYPE_PORT_SEND_ONCE,PortSize);
<Normal>"MACH_MSG_TYPE_MOVE_SEND_ONCE"	TPRETURN(MACH_MSG_TYPE_MOVE_SEND_ONCE,MACH_MSG_TYPE_PORT_SEND_ONCE,PortSize);

<Normal>"MACH_MSG_TYPE_PORT_NAME"	TRETURN(MACH_MSG_TYPE_PORT_NAME,PortSize);
<Normal>"MACH_MSG_TYPE_PORT_RECEIVE"	TPRETURN(MACH_MSG_TYPE_POLYMORPHIC,MACH_MSG_TYPE_PORT_RECEIVE,PortSize);
<Normal>"MACH_MSG_TYPE_PORT_SEND"	TPRETURN(MACH_MSG_TYPE_POLYMORPHIC,MACH_MSG_TYPE_PORT_SEND,PortSize);
<Normal>"MACH_MSG_TYPE_PORT_SEND_ONCE"	TPRETURN(MACH_MSG_TYPE_POLYMORPHIC,MACH_MSG_TYPE_PORT_SEND_ONCE,PortSize);
<Normal>"MACH_MSG_TYPE_POLYMORPHIC"	TPRETURN(MACH_MSG_TYPE_POLYMORPHIC, MACH_MSG_TYPE_POLYMORPHIC, PortSize);

<Normal>":"		RETURN(syColon);
<Normal>";"		RETURN(sySemi);
<Normal>","		RETURN(syComma);
<Normal>"+"		RETURN(syPlus);
<Normal>"-"		RETURN(syMinus);
<Normal>"*"		RETURN(syStar);
<Normal>"/"		RETURN(syDiv);
<Normal>"("		RETURN(syLParen);
<Normal>")"		RETURN(syRParen);
<Normal>"="		RETURN(syEqual);
<Normal>"^"		RETURN(syCaret);
<Normal>"~"		RETURN(syTilde);
<Normal>"<"		RETURN(syLAngle);
<Normal>">"		RETURN(syRAngle);
<Normal>"["		RETURN(syLBrack);
<Normal>"]"		RETURN(syRBrack);
<Normal>"|"		RETURN(syBar);

<Normal>{Ident}		{ yylval.identifier = strmake(yytext);
			  RETURN(syIdentifier); }
<Normal>{Number}	{ yylval.number = atoi(yytext); RETURN(syNumber); }

<String>{String}	{ yylval.string = strmake(yytext);
			  SAVE(Normal); RETURN(syString); }
<FileName>{FileName}	{ yylval.string = strmake(yytext);
			  SAVE(Normal); RETURN(syFileName); }
<QString>{QString}	{ yylval.string = strmake(yytext);
			  SAVE(Normal); RETURN(syQString); }

^\#[ \t]*{Number}[ \t]*\"[^"]*\"	{ doSharp(yytext+1);
					  BEGIN SkipToEOL; }
^\#\ *{Number}				{ doSharp(yytext+1);
					  BEGIN SkipToEOL; }
^\#pragma				{ BEGIN SkipToEOL; }
^\#					{ yyerror("illegal # directive");
					  BEGIN SkipToEOL; }

<SkipToEOL>\n		{ RESTORE; lineno++; }
<SkipToEOL>.		;

[ \t]			;
\n			lineno++;
.			{ SAVE(Normal); RETURN(syError); }

%%

extern void
LookNormal()
{
  SAVE(Normal);
}

extern void
LookString()
{
  SAVE(String);
}

extern void
LookQString()
{
    SAVE(QString);
}

extern void
LookFileName()
{
  SAVE(FileName);
}

static void
doSharp(char *body)
{
  char *startName, *endName;

  lineno = atoi(body);
  startName = strchr(body, '"');
  if (startName != NULL) {
    endName = strrchr(body, '"');
    *endName = '\0';
    strfree(yyinname);
    yyinname = strmake(startName+1);
  }
}

int
yywrap()
{
  return 1;
}

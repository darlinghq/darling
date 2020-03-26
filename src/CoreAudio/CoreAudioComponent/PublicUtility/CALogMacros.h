/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#if !defined(__CALogMacros_h__)
#define __CALogMacros_h__

//=============================================================================
//	Log Macros
//=============================================================================

#if	CoreAudio_Debug

	#include "CADebugMacros.h"
	#include "CADebugPrintf.h"
	#include <stdio.h>
	#include <string.h>
	
	#define	PrintLine(msg)						DebugPrintfRtn(DebugPrintfFileComma "%s\n", (msg))
	
	#define PrintBool(msg, b)					DebugPrintfRtn(DebugPrintfFileComma "%s%s\n", (msg), (b) ? "true" : "false")
	#define PrintIndexedBool(msg, i, b)			DebugPrintfRtn(DebugPrintfFileComma "  %s %ld: %s\n", (msg), (long)(i), (b) ? "true" : "false")
	
	#define PrintToggle(msg, b)					DebugPrintfRtn(DebugPrintfFileComma "%s%s\n", (msg), (b) ? "on" : "off")
	#define PrintIndexedToggle(msg, i, b)		DebugPrintfRtn(DebugPrintfFileComma "  %s %ld: %s\n", (msg), (long)(i), (b) ? "on" : "off")
	
	#define PrintInt(msg, n)					DebugPrintfRtn(DebugPrintfFileComma "%s%ld\n", (msg), (long)(n))
	#define PrintIndexedInt(msg, i, n)			DebugPrintfRtn(DebugPrintfFileComma "  %s %ld: %ld\n", (msg), (long)(i), (long)(n))
	
	#define PrintHex(msg, n)					DebugPrintfRtn(DebugPrintfFileComma "%s0x%lX\n", (msg), (unsigned long)(n))
	#define PrintIndexedHex(msg, i, n)			DebugPrintfRtn(DebugPrintfFileComma "  %s %ld: 0x%lX\n", (msg), (long)(i), (unsigned long)(n))
	
	#define PrintFloat(msg, f)					DebugPrintfRtn(DebugPrintfFileComma "%s%.6f\n", (msg), (f))
	#define PrintIndexedFloat(msg, i, f)		DebugPrintfRtn(DebugPrintfFileComma "  %s %ld: %.6f\n", (msg), (long)(i), (f))
	#define PrintFloatIndexedFloat(msg, i, f)	DebugPrintfRtn(DebugPrintfFileComma "  %s %.6f: %.6f\n", (msg), (i), (f))
	
	#define	PrintString(msg, s)					DebugPrintfRtn(DebugPrintfFileComma "%s%s\n", (msg), (s))
	#define PrintIndexedString(msg, i, s)		DebugPrintfRtn(DebugPrintfFileComma "  %s %ld: %s\n", (msg), (long)(i), (s))
	
	#define PrintPointer(msg, p)				DebugPrintfRtn(DebugPrintfFileComma "%s%p\n", (msg), (p))
	#define PrintIndexedPointer(msg, i, p)		DebugPrintfRtn(DebugPrintfFileComma "  %s %ld: %p\n", (msg), (long)(i), (p))
	
	#define	Print4CharCode(msg, c)				{																								\
													UInt32 __4CC_number = (c);																	\
													char __4CC_string[5] = CA4CCToCString(__4CC_number);										\
													DebugPrintfRtn(DebugPrintfFileComma "%s'%s'\n", (msg), __4CC_string);							\
												}
	#define PrintIndexed4CharCode(msg, i, c)	{																								\
													UInt32 __4CC_number = (c);																	\
													char __4CC_string[5] = CA4CCToCString(__4CC_number);										\
													DebugPrintfRtn(DebugPrintfFileComma "  %s %ld: '%s'\n", (msg), (long)(i), __4CC_string);		\
												}
	
	#define	ErrorLine(s)						DebugPrintfRtn(DebugPrintfFileComma "%s\n", (s))
	#define	OSErrorLine(s, e)					{																								\
													OSStatus __err_number = (e);																\
													char __err_string[5] = CA4CCToCString(__err_number);										\
													DebugPrintfRtn(DebugPrintfFileComma "%s, OSStatus code: %s\n", (s), __err_string);				\
												}
	
	#define	MessageIfOSError(e, s)				if((e) != 0) { OSErrorLine(s, e); }
	#define	MessageIfNULL(p, s)					if((p) == 0) { ErrorLine(s); }

#else

	#define	PrintLine(msg)					
	
	#define PrintBool(msg, b)					(b)
	#define PrintIndexedBool(msg, i, b)			(b)
	
	#define PrintInt(msg, n)					(n)
	#define PrintIndexedInt(msg, i, n)			(n)
	
	#define PrintHex(msg, n)					(n)
	#define PrintIndexedHex(msg, i, n)			(n)
	
	#define PrintFloat(msg, f)					(f)
	#define PrintIndexedFloat(msg, i, f)		(f)
	#define PrintFloatIndexedFloat(msg, i, f)	(f)
	
	#define	PrintString(msg, s)					(s)
	#define PrintIndexedString(msg, i, s)		(s)
	
	#define PrintPointer(msg, p)				(p)
	#define PrintIndexedPointer(msg, i, p)		(p)
	
	#define	Print4CharCode(msg, c)				(c)
	#define PrintIndexed4CharCode(msg, i, c)	(c)
	
	#define	ErrorLine(s)						(s)
	#define	OSErrorLine(s, e)					(e)
	
	#define	MessageIfOSError(e, s)				(e)
	#define	MessageIfNULL(p, s)					(p)

#endif	//	CoreAudio_Debug

#endif

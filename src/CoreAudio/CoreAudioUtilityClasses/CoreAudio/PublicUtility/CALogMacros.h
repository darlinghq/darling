/*
     File: CALogMacros.h
 Abstract: Part of CoreAudio Utility Classes
  Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
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

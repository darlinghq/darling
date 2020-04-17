/*
     File: ACCodecDispatchTypes.h
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
#if !defined(__ACCodecDispatchTypes_h__)
#define __ACCodecDispatchTypes_h__

//=============================================================================
//	Includes
//=============================================================================

//	note that AudioCodec.h needs to be included prior to including this file
#if !defined(__AudioCodec_h__)
	#error	AudioCodec.h needs to be included prior to including this file
#endif

//=============================================================================
//	Parameter Blocks for AudioCodec Component Routines
//
//	Note that the arguments for all the AudioCodec component routines are
//	4 bytes in size (assuming pointers are 4 bytes). This means that even on
//	Windows (where all arguments are forced into SInt32s), we can get away with
//	mass casting the argument list through the appropriate paramblock structure.
//	This gets around the fact that the component glue passes the arguments in
//	a different order depending on the platform and therefore makes writing the
//	 calling glue and the dispatcher much simpler.
//=============================================================================

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=mac68k
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(push, 2)
#elif PRAGMA_STRUCT_PACK
	#pragma pack(2)
#endif

#define	AudioCodecStandardGluePBFields	UInt8 componentFlags; UInt8 componentParamSize; SInt16 componentWhat

#if	!TARGET_OS_WIN32
struct AudioCodecOpenGluePB {
	AudioCodecStandardGluePBFields;
	AudioCodec						inCodec;
	void*							unused;
};
#else
struct AudioCodecOpenGluePB {
	AudioCodecStandardGluePBFields;
	AudioCodec						inCodec;
};
#endif
typedef struct AudioCodecOpenGluePB	AudioCodecOpenGluePB;

#if	!TARGET_OS_WIN32
struct AudioCodecGetPropertyInfoGluePB
{
	AudioCodecStandardGluePBFields;
	Boolean*						outWritable;
	UInt32*							outSize;
	AudioCodecPropertyID			inPropertyID;
	AudioCodec						inCodec;
};
#else
struct AudioCodecGetPropertyInfoGluePB
{
	AudioCodecStandardGluePBFields;
	AudioCodecPropertyID			inPropertyID;
	UInt32*							outSize;
	Boolean*						outWritable;
};
#endif
typedef struct AudioCodecGetPropertyInfoGluePB	AudioCodecGetPropertyInfoGluePB;

#if	!TARGET_OS_WIN32
struct AudioCodecGetPropertyGluePB
{
	AudioCodecStandardGluePBFields;
	void*							outPropertyData;
	UInt32*							ioPropertyDataSize;
	AudioCodecPropertyID			inPropertyID;
	AudioCodec						inCodec;
};
#else
struct AudioCodecGetPropertyGluePB
{
	AudioCodecStandardGluePBFields;
	AudioCodecPropertyID			inPropertyID;
	UInt32*							ioPropertyDataSize;
	void*							outPropertyData;
};
#endif
typedef struct AudioCodecGetPropertyGluePB	AudioCodecGetPropertyGluePB;

#if	!TARGET_OS_WIN32
struct AudioCodecSetPropertyGluePB
{
	AudioCodecStandardGluePBFields;
	const void*						inPropertyData;
	UInt32							inPropertyDataSize;
	AudioCodecPropertyID			inPropertyID;
	AudioCodec						inCodec;
};
#else
struct AudioCodecSetPropertyGluePB
{
	AudioCodecStandardGluePBFields;
	AudioCodecPropertyID			inPropertyID;
	UInt32							inPropertyDataSize;
	const void*						inPropertyData;
};
#endif
typedef struct AudioCodecSetPropertyGluePB	AudioCodecSetPropertyGluePB;

#if	!TARGET_OS_WIN32
struct AudioCodecInitializeGluePB
{
	AudioCodecStandardGluePBFields;
	UInt32								inMagicCookieByteSize;
	const void*							inMagicCookie;
	const AudioStreamBasicDescription*	inOutputFormat;
	const AudioStreamBasicDescription*	inInputFormat;
	AudioCodec							inCodec;
};
#else
struct AudioCodecInitializeGluePB
{
	AudioCodecStandardGluePBFields;
	const AudioStreamBasicDescription*	inInputFormat;
	const AudioStreamBasicDescription*	inOutputFormat;
	const void*							inMagicCookie;
	UInt32								inMagicCookieByteSize;
};
#endif
typedef struct AudioCodecInitializeGluePB	AudioCodecInitializeGluePB;

#if	!TARGET_OS_WIN32
struct AudioCodecUninitializeGluePB
{
	AudioCodecStandardGluePBFields;
	AudioCodec						inCodec;
};
#else
struct AudioCodecUninitializeGluePB
{
	AudioCodecStandardGluePBFields;
};
#endif
typedef struct AudioCodecUninitializeGluePB	AudioCodecUninitializeGluePB;

#if	!TARGET_OS_WIN32
struct AudioCodecAppendInputDataGluePB
{
	AudioCodecStandardGluePBFields;
	const AudioStreamPacketDescription*	inPacketDescription;
	UInt32*								ioNumberPackets;
	UInt32*								ioInputDataByteSize;
	const void*							inInputData;
	AudioCodec							inCodec;
};
#else
struct AudioCodecAppendInputDataGluePB
{
	AudioCodecStandardGluePBFields;
	const void*							inInputData;
	UInt32*								ioInputDataByteSize;
	UInt32*								ioNumberPackets;
	const AudioStreamPacketDescription*	inPacketDescription;
};
#endif
typedef struct AudioCodecAppendInputDataGluePB	AudioCodecAppendInputDataGluePB;

#if	!TARGET_OS_WIN32
struct AudioCodecProduceOutputPacketsGluePB
{
	AudioCodecStandardGluePBFields;
	UInt32*							outStatus;
	AudioStreamPacketDescription*	outPacketDescription;
	UInt32*							ioNumberPackets;
	UInt32*							ioOutputDataByteSize;
	void*							outOutputData;
	AudioCodec						inCodec;
};
#else
struct AudioCodecProduceOutputPacketsGluePB
{
	AudioCodecStandardGluePBFields;
	void*							outOutputData;
	UInt32*							ioOutputDataByteSize;
	UInt32*							ioNumberPackets;
	AudioStreamPacketDescription*	outPacketDescription;
	UInt32*							outStatus;
};
#endif
typedef struct AudioCodecProduceOutputPacketsGluePB	AudioCodecProduceOutputPacketsGluePB;

#if	!TARGET_OS_WIN32
struct AudioCodecResetGluePB
{
	AudioCodecStandardGluePBFields;
	AudioCodec						inCodec;
};
#else
struct AudioCodecResetGluePB
{
	AudioCodecStandardGluePBFields;
};
#endif
typedef struct AudioCodecResetGluePB	AudioCodecResetGluePB;

#if PRAGMA_STRUCT_ALIGN
	#pragma options align=reset
#elif PRAGMA_STRUCT_PACKPUSH
	#pragma pack(pop)
#elif PRAGMA_STRUCT_PACK
	#pragma pack()
#endif

#endif

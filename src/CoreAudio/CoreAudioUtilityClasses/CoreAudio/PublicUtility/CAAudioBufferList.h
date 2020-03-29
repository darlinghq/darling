/*
     File: CAAudioBufferList.h
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
#if !defined(__CAAudioBufferList_h__)
#define __CAAudioBufferList_h__

//=============================================================================
//	Includes
//=============================================================================

//	System Includes
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
#else
	#include <CoreAudioTypes.h>
#endif

#include "CAException.h"
#include "CADebugMacros.h"

//=============================================================================
//	Types
//=============================================================================

typedef AudioBufferList*	AudioBufferListPtr;

//=============================================================================
//	CAAudioBufferList
//=============================================================================

struct	CAAudioBufferList
{

//	Construction/Destruction
public:
	static AudioBufferList*	Create(UInt32 inNumberBuffers);
	static void				Destroy(AudioBufferList* inBufferList);
	static UInt32			CalculateByteSize(UInt32 inNumberBuffers);

//	Operations
public:
	static UInt32			GetTotalNumberChannels(const AudioBufferList& inBufferList);
	static bool				GetBufferForChannel(const AudioBufferList& inBufferList, UInt32 inChannel, UInt32& outBufferNumber, UInt32& outBufferChannel);
	static void				Clear(AudioBufferList& outBufferList);
	static void				Copy(const AudioBufferList& inSource, UInt32 inStartingSourceChannel, AudioBufferList& outDestination, UInt32 inStartingDestinationChannel);
	static void				CopyChannel(const AudioBuffer& inSource, UInt32 inSourceChannel, AudioBuffer& outDestination, UInt32 inDestinationChannel);
	static void				Sum(const AudioBufferList& inSourceBufferList, AudioBufferList& ioSummedBufferList);
	static bool				HasData(AudioBufferList& inBufferList);
#if	CoreAudio_Debug
	static void				PrintToLog(const AudioBufferList& inBufferList);
#endif

	static const AudioBufferList*	GetEmptyBufferList();

};

// Declare a variably-sized AudioBufferList on the stack
#define STACK_ABL(name, nbufs) \
	ThrowIf(nbufs < 1 || nbufs > 64, CAException(kAudio_ParamError), "STACK_ABL: invalid number of buffers") \
	const size_t name##_ByteSize = sizeof(AudioBufferList) + (nbufs - 1) * sizeof(AudioBuffer); \
	AudioBufferList &name = *(AudioBufferList *)alloca(name##_ByteSize); \
	name.mNumberBuffers = nbufs;


#endif

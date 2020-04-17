/*
     File: CAStreamRangedDescription.h
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
#if !defined(__CAStreamRangedDescription_h__)
#define __CAStreamRangedDescription_h__

//==================================================================================================
//	Includes
//==================================================================================================

//  Super Class Includes
#include <CoreAudio/AudioHardware.h>

//  PublicUtility Includes
#include "CAAudioValueRange.h"
#include "CAStreamBasicDescription.h"

//==================================================================================================
//	CAStreamRangedDescription
//==================================================================================================

class CAStreamRangedDescription
:
	public AudioStreamRangedDescription
{

//	Constants
public:
	static const AudioStreamRangedDescription	sEmpty;

//	Construction/Destruction
public:
								CAStreamRangedDescription()																							{ memset(this, 0, sizeof(AudioStreamRangedDescription)); }
								CAStreamRangedDescription(const CAStreamRangedDescription& inFormat)												{ mFormat = inFormat.mFormat; mSampleRateRange = inFormat.mSampleRateRange; }
								CAStreamRangedDescription(const AudioStreamRangedDescription& inFormat)												{ mFormat = inFormat.mFormat; mSampleRateRange = inFormat.mSampleRateRange; }
								CAStreamRangedDescription(const AudioStreamBasicDescription& inFormat)												{ mFormat = inFormat; mSampleRateRange.mMinimum = inFormat.mSampleRate; mSampleRateRange.mMaximum = inFormat.mSampleRate; }
								CAStreamRangedDescription(const AudioStreamBasicDescription& inFormat, const AudioValueRange& inSampleRateRange)	{ mFormat = inFormat; mSampleRateRange = inSampleRateRange; }
	CAStreamRangedDescription&  operator=(const CAStreamRangedDescription& inFormat)																{ mFormat = inFormat.mFormat; mSampleRateRange = inFormat.mSampleRateRange; return *this; }
	CAStreamRangedDescription&  operator=(const AudioStreamRangedDescription& inFormat)																{ mFormat = inFormat.mFormat; mSampleRateRange = inFormat.mSampleRateRange; return *this; }

	static bool					IsMixable(const AudioStreamRangedDescription& inDescription) { return (inDescription.mFormat.mFormatID == kAudioFormatLinearPCM) && ((inDescription.mFormat.mFormatFlags & kIsNonMixableFlag) == 0); }
#if CoreAudio_Debug
	static void					PrintToLog(const AudioStreamRangedDescription& inDesc);
#endif
	static bool					Sorter(const AudioStreamRangedDescription& x, const AudioStreamRangedDescription& y);

};

inline bool operator<(const AudioStreamRangedDescription& x, const AudioStreamRangedDescription& y)													{	return	(x.mFormat.mFormatID == y.mFormat.mFormatID) ? 
																																								(x.mFormat < y.mFormat) && (x.mSampleRateRange < y.mSampleRateRange) :
																																								(x.mFormat.mFormatID == kAudioFormatLinearPCM) ? false : (x.mFormat.mFormatID < y.mFormat.mFormatID); }
inline bool operator==(const AudioStreamRangedDescription& x, const AudioStreamRangedDescription& y)												{ return (x.mFormat == y.mFormat) && (x.mSampleRateRange == y.mSampleRateRange); }
#if TARGET_OS_MAC || (TARGET_OS_WIN32 && (_MSC_VER > 600))
inline bool	operator!=(const AudioStreamRangedDescription& x, const AudioStreamRangedDescription& y)												{ return !(x == y); }
inline bool	operator<=(const AudioStreamRangedDescription& x, const AudioStreamRangedDescription& y)												{ return (x < y) || (x == y); }
inline bool	operator>=(const AudioStreamRangedDescription& x, const AudioStreamRangedDescription& y)												{ return !(x < y); }
inline bool	operator>(const AudioStreamRangedDescription& x, const AudioStreamRangedDescription& y)													{ return !((x < y) || (x == y)); }
#endif

inline bool operator<(const AudioStreamBasicDescription& x, const AudioStreamRangedDescription& y)													{	return	(x.mFormatID == y.mFormat.mFormatID) ? 
																																								(x < y.mFormat) && (x.mSampleRate < y.mSampleRateRange.mMinimum) :
																																								(x.mFormatID == kAudioFormatLinearPCM) ? false : (x.mFormatID < y.mFormat.mFormatID); }
inline bool operator<(const AudioStreamRangedDescription& x, const AudioStreamBasicDescription& y)													{	return	(x.mFormat.mFormatID == y.mFormatID) ? 
																																								(x.mFormat < y) && (x.mSampleRateRange.mMinimum < y.mSampleRate) :
																																								(x.mFormat.mFormatID == kAudioFormatLinearPCM) ? false : (x.mFormat.mFormatID < y.mFormatID); }
inline bool operator==(const AudioStreamBasicDescription& x, const AudioStreamRangedDescription& y)													{ return (x == y.mFormat) && CAAudioValueRange::ContainsValue(y.mSampleRateRange, x.mSampleRate); }
inline bool operator==(const AudioStreamRangedDescription& x, const AudioStreamBasicDescription& y)													{ return (x.mFormat == y) && CAAudioValueRange::ContainsValue(x.mSampleRateRange, y.mSampleRate); }
#if TARGET_OS_MAC || (TARGET_OS_WIN32 && (_MSC_VER > 600))
inline bool	operator!=(const AudioStreamBasicDescription& x, const AudioStreamRangedDescription& y)													{ return !(x == y); }
inline bool	operator!=(const AudioStreamRangedDescription& x, const AudioStreamBasicDescription& y)													{ return !(x == y); }
inline bool	operator<=(const AudioStreamBasicDescription& x, const AudioStreamRangedDescription& y)													{ return (x < y) || (x == y); }
inline bool	operator<=(const AudioStreamRangedDescription& x, const AudioStreamBasicDescription& y)													{ return (x < y) || (x == y); }
inline bool	operator>=(const AudioStreamBasicDescription& x, const AudioStreamRangedDescription& y)													{ return !(x < y); }
inline bool	operator>=(const AudioStreamRangedDescription& x, const AudioStreamBasicDescription& y)													{ return !(x < y); }
inline bool	operator>(const AudioStreamBasicDescription& x, const AudioStreamRangedDescription& y)													{ return !((x < y) || (x == y)); }
inline bool	operator>(const AudioStreamRangedDescription& x, const AudioStreamBasicDescription& y)													{ return !((x < y) || (x == y)); }
#endif

//	STL Functors
struct	CAStreamRangedDescription_EqualToASBD
:
	public std::unary_function<AudioStreamRangedDescription, bool>
{
			CAStreamRangedDescription_EqualToASBD(const AudioStreamBasicDescription& inFormat)	: mFormat(inFormat) {}
	bool	operator()(const AudioStreamRangedDescription& x) const								{ return mFormat == x; }
	
	AudioStreamBasicDescription	mFormat;
};

struct	CAStreamRangedDescription_ReverseSort
:
	public std::binary_function<AudioStreamRangedDescription, AudioStreamRangedDescription, bool>
{
	bool	operator()(const AudioStreamRangedDescription& x, const AudioStreamRangedDescription& y) const	{ return CAStreamRangedDescription::Sorter(x, y); }
};

#endif

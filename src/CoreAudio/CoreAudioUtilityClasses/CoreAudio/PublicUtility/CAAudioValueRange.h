/*
     File: CAAudioValueRange.h
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
#if !defined(__CAAudioValueRange_h__)
#define __CAAudioValueRange_h__

//=============================================================================
//	Includes
//=============================================================================

//	System Includes
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
#else
	#include <CoreAudioTypes.h>
#endif
//	Standard Library Includes
#include <functional>
#include <vector>

//=============================================================================
//	CAAudioValueRange
//=============================================================================

struct CAAudioValueRange
:
	public AudioValueRange
{

//	Construction/Destruction
public:
						CAAudioValueRange()																{ mMinimum = 0.0; mMaximum = 0.0; }
						CAAudioValueRange(const AudioValueRange& v)										{ mMinimum = v.mMinimum; mMaximum = v.mMaximum; }
						CAAudioValueRange(Float64 inMinimum, Float64 inMaximum)							{ mMinimum = inMinimum; mMaximum = inMaximum; }

//	Assignment
public:
	CAAudioValueRange&	operator=(const AudioValueRange& v)												{ mMinimum = v.mMinimum; mMaximum = v.mMaximum; return *this; }

//  Operations
public:
	static bool			ContainsValue(const AudioValueRange& inRange, Float64 inValue)					{ return (inValue >= inRange.mMinimum) && (inValue <= inRange.mMaximum); }
	static Float64		BoundValue(const AudioValueRange& inRange, Float64 inValue);
	static Float64		PickCommonSampleRate(const AudioValueRange& inRange);
	static bool			IsStrictlyLessThan(const AudioValueRange& x, const AudioValueRange& y)			{ return x.mMaximum < y.mMinimum; }
	static bool			IsStrictlyGreaterThan(const AudioValueRange& x, const AudioValueRange& y)		{ return x.mMinimum > y.mMaximum; }
	static bool			IsStrictlyContainedBy(const AudioValueRange& x, const AudioValueRange& y)		{ return (x.mMinimum >= y.mMinimum) && (x.mMaximum <= y.mMaximum); }
	static bool			OverlapsLow(const AudioValueRange& x, const AudioValueRange& y)					{ return (x.mMinimum < y.mMinimum) && (x.mMaximum >= y.mMinimum) && (x.mMaximum <= y.mMaximum); }
	static bool			OverlapsHigh(const AudioValueRange& x, const AudioValueRange& y)				{ return (x.mMinimum >= y.mMinimum) && (x.mMinimum <= y.mMaximum) && (x.mMaximum > y.mMaximum); }
	static bool			Intersection(const AudioValueRange& x, const AudioValueRange& y, AudioValueRange& outRange);
	static bool			Union(const AudioValueRange& x, const AudioValueRange& y, AudioValueRange& outRange);

//	STL Helpers
public:
	struct	LessThan
	:
		public std::binary_function<AudioValueRange, AudioValueRange, bool>
	{
		bool	operator()(const AudioValueRange& x, const AudioValueRange& y) const
		{
			return x.mMinimum < y.mMinimum;
		}
	};
	
};

inline bool	operator<(const AudioValueRange& x, const AudioValueRange& y)								{ return x.mMinimum < y.mMinimum; }
inline bool	operator==(const AudioValueRange& x, const AudioValueRange& y)								{ return (x.mMinimum == y.mMinimum) && (x.mMaximum == y.mMaximum); }
inline bool	operator!=(const AudioValueRange& x, const AudioValueRange& y)								{ return !(x == y); }
inline bool	operator<=(const AudioValueRange& x, const AudioValueRange& y)								{ return (x < y) || (x == y); }
inline bool	operator>=(const AudioValueRange& x, const AudioValueRange& y)								{ return !(x < y); }
inline bool	operator>(const AudioValueRange& x, const AudioValueRange& y)								{ return !((x < y) || (x == y)); }

typedef	std::vector<CAAudioValueRange>	CAAudioValueRangeList;
void	CAAudioValueRange_ComputeUnion(const AudioValueRange& inRange, const CAAudioValueRangeList& inRangeList, CAAudioValueRangeList& outUnion);
void	CAAudioValueRange_ComputeIntersection(UInt32 inNumberRangeList1Items, AudioValueRange inRangeList1[], const CAAudioValueRangeList& inRangeList2, CAAudioValueRangeList& outIntersections);

#endif

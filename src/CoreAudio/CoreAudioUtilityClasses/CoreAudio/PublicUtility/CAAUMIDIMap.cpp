/*
     File: CAAUMIDIMap.cpp
 Abstract: CAAUMIDIMap.h
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
#include "CAAUMIDIMap.h"
#include <pthread.h>

struct AllMidiTransformers
{
	MIDILinearTransformer linearTrans; 
	MIDILogTransformer logTrans;
	MIDIExpTransformer expTrans;
	MIDISqrtTransformer sqrtTrans;
	MIDISquareTransformer squareTrans;
	MIDICubeRtTransformer cubeRtTrans;
	MIDICubeTransformer cubeTrans;
};

AllMidiTransformers* gAllMidiTransformers = NULL;

#if TARGET_OS_MAC
static pthread_once_t sOnce = PTHREAD_ONCE_INIT;

static void InitAllMidiTransformers()
{
	gAllMidiTransformers = new AllMidiTransformers();
}

static void CheckInitAllMidiTransformers()
{
	pthread_once(&sOnce, InitAllMidiTransformers);
}
#endif

MIDIValueTransformer *	CAAUMIDIMap::GetTransformer (UInt32 inFlags)
{
#if TARGET_OS_MAC
	if (gAllMidiTransformers == NULL) 
		CheckInitAllMidiTransformers();
#else
	if (gAllMidiTransformers == NULL) 
		gAllMidiTransformers = new AllMidiTransformers();
#endif
	
	if (AudioUnitDisplayTypeIsLogarithmic(inFlags))
		return &gAllMidiTransformers->logTrans;
	else if (AudioUnitDisplayTypeIsExponential(inFlags))
		return &gAllMidiTransformers->expTrans;
	else if (AudioUnitDisplayTypeIsSquareRoot(inFlags))
		return &gAllMidiTransformers->sqrtTrans;
	else if (AudioUnitDisplayTypeIsSquared(inFlags))
		return &gAllMidiTransformers->squareTrans;
	else if (AudioUnitDisplayTypeIsCubed(inFlags))
		return &gAllMidiTransformers->cubeTrans;
	else if (AudioUnitDisplayTypeIsCubeRoot(inFlags))
		return &gAllMidiTransformers->cubeRtTrans;
	else
		return &gAllMidiTransformers->linearTrans;
}

// The CALLER of this method must ensure that the status byte's MIDI Command matches!!!
bool	CAAUMIDIMap::MIDI_Matches (UInt8 inChannel, UInt8 inData1, UInt8 inData2, Float32 &outLinear) const
{
	// see if the channels match first
	SInt8 chan = Channel();
	// channel matches (if chan is less than zero, "Any Channel" flag is set)
	if (chan >= 0 && chan != inChannel)
		return false;

	// match the special cases first
	if (IsKeyEvent()) {
		// we're using this key event as an on/off type switch
		if (IsBipolar()) {
			if (IsKeyPressure()){
				if (IsBipolar_OnValue()) {
					if (inData2 > 63) {
						outLinear = 1;
						return true;
					}
				} else {
					if (inData2 < 64) {
						outLinear = 0;
						return true;
					}
				}
				return false;
			}
			else {
				if (IsBipolar_OnValue()) {
					if (inData1 > 63) {
						outLinear = 1;
						return true;
					}
				} else {
					if (inData1 < 64) {
						outLinear = 0;
						return true;
					}
				}
				return false;
			}
		}
		if (IsAnyNote()) {
// not quite sure how to interpret this...
			if (IsKeyPressure())
				outLinear = inData2 / 127.0;
			else
				outLinear = inData1 / 127.0;
			return true;
		}
		if (mData1 == inData1) {
			if (IsKeyPressure())
				outLinear = inData2 / 127.0;
			else
				outLinear = 1;
			return true;
		}
		return false;
	}
	else if (IsControlChange()) {
		// controller ID matches
		if (mData1 == inData1) {
			if (IsBipolar()) {
				if (IsBipolar_OnValue()) {
					if (inData2 > 63) {
						outLinear = 1;
						return true;
					}
				} else {
					if (inData2 < 64) {
						outLinear = 0;
						return true;
					}
				}
				return false;
			}
			//printf("this in midi matches %X with ", this); 
			outLinear = inData2 / 127.; 
			return true;
		}
		return false;
	}
	
		// this just matches on the patch change value itself...
	if (IsPatchChange()) {
		if (mData1 == inData1) {
			outLinear = 1;
			return true;
		}
		return false;
	}

	// finally, for the other two, just check the bi-polar matching conditions
	// pitch bend and after touch
	if (IsBipolar()) {
		if (IsBipolar_OnValue()) {
			if (inData1 > 63) {
				outLinear = 1;
				return true;
			}
		} else {
			if (inData1 < 64) {
				outLinear = 0;
				return true;
			}
		}
		return false;
	}

	if (IsPitchBend()) {
		UInt16 value = (inData2 << 7) | inData1;
		outLinear = value / 16383.;
	}
	else if (IsChannelPressure()) {
		outLinear = inData1 / 127.0;
	}

	return true;
}


void		CAAUMIDIMap::Print () const
{
	printf ("CAAUMIDIMap:%p, (%u/%u), mParamID %d, IsValid:%c, Status:0x%X, mData1 %d, Flags:0x%X\n", this, (unsigned int)mScope, (unsigned int)mElement, (int)mParameterID, (IsValid() ? 'T' : 'F'), mStatus, mData1, (int)mFlags);
}

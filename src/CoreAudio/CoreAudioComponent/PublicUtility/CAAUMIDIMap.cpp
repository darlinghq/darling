/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
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

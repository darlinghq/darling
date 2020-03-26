/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUBase Classes
*/

#ifndef __AUDispatch_h__
#define __AUDispatch_h__

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioUnit/AudioUnit.h>
#else
	#include "AudioUnit.h"
#endif

#if !CA_USE_AUDIO_PLUGIN_ONLY
/*! @function AudioUnitBaseGetParameter */
OSStatus CMgr_AudioUnitBaseGetParameter(	AUBase *				This,
											AudioUnitParameterID	inID,
											AudioUnitScope			inScope,
											AudioUnitElement		inElement,
											float *					outValue);

/*! @function AudioUnitBaseSetParameter */
OSStatus CMgr_AudioUnitBaseSetParameter(	AUBase *		 		This,
											AudioUnitParameterID	inID,
											AudioUnitScope			inScope,
											AudioUnitElement		inElement,
											float					inValue,
											UInt32					inBufferOffset);

/*! @function AudioUnitBaseRender */
OSStatus CMgr_AudioUnitBaseRender(			AUBase *				This,
											AudioUnitRenderActionFlags *ioActionFlags,
											const AudioTimeStamp *	inTimeStamp,
											UInt32					inBusNumber,
											UInt32					inNumberFrames,
											AudioBufferList *		ioData);
#endif

#endif // __AUDispatch_h__

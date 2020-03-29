/*
     File: CAAUProcessor.h
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
#ifndef __CAAUProcessor_h__
#define __CAAUProcessor_h__

#include <AudioToolbox/AudioToolbox.h>
#include "CAStreamBasicDescription.h"
#include "CAAudioUnit.h"
#include "AUOutputBL.h"

#if TARGET_OS_IPHONE
	#include <AssertMacros.h>
#endif
/*
	This class wraps an AU (using the CAAudioUnit helper class) to use that AU for processing data
	It can be used in a RealTime context (rendering data with RT constraints) or in an OffLine context
	such as using an AU to process data that is stored in a file, and on which there are no
	RT constraints to be imposed.
	
	Order of operations:
	Create an instance
	Establish an Input Callback
	Initialize the AU to the audio formats and whether it is going to process in an offline or RT context
	Preflight
	
	while (...)
		Render  // based on your calling context
	
	PostProcess if needed (only in OL case)
	
	After any initialization, preflighting is required.
	
	Parameter Values on the AU should be set just before each call to Render. The sampleFrameOffsets
	supplied when setting those values are an offset into that next render buffer's numFrames.
	
	RT vs OT is determined by whether the inputSampleCount is set during Initialization, thus
	this class can move the AU between RT and OL contexts. If you are using an AU of type 'auol'
	(Offline), then it cannot be used in a RT context.

	The CAAUProcessor will only call your Input Callback for input when it needs valid input.
	This input callback will contain a sample time that indicates where within your input
	you should read data from, where after preflighting, the first output data produces is for 
	the output sample count of zero.
	
	MaxFrames should be set before initialisation (or is also passed in to the Reinitialize API)
		
	If RT, then PostProcessing will *never* be required, nor will Render ever return an isDone value
	If OL, then Render will at some point return isDone==true, and then also indicate if PostProcessing is required
	
	If using a Format Converter AU in offline context, then the only way for it to determine that you've finished
	is for the Input callback to return an error. This means ultimately, that you'll have potentially longer output
	than you should have. The only way to manage this is for the caller to know the relationship between 
	input to output samples that will be required, and to call Render for just that amount of output samples
	then return an error, so the tail and latency can be processed.
	
	Tail and Latency are *only* calculated at initialisation.. Some AU's may change these properties based on
	either different presets or parameter settings... Ideally, this class should listen to those properties
	and recalculate its values based on those changes.
*/	
	
class CAAUProcessor {
public:
									// by default this is set to process offline
							CAAUProcessor (const CAComponent& inComp);
							~CAAUProcessor ();
		
	CAAudioUnit&			AU() { return mUnit; }
	const CAAudioUnit&		AU() const { return mUnit; }
		
#pragma mark __Setup APIs			
	bool					IsOfflineContext () const { return mNumInputSamples != 0; }
								
								// this contains the callback and client data that the AU 
								// uses to call when it needs input
	OSStatus				EstablishInputCallback (AURenderCallbackStruct &inInputCallback);
	
	OSStatus				SetAUPreset (CFPropertyListRef 			inPreset);
	OSStatus				SetAUPresetIndex (SInt32				inPresetIndex);
	
	OSStatus				SetParameter (AudioUnitParameterID inID, AudioUnitScope scope, AudioUnitElement element,
											Float32 value, UInt32 bufferOffsetFrames = 0);
	
		// a result of zero here signifies an error
	UInt32					MaxFramesPerRender () const;
		
		// this call can return an error if the AU is initialized - see Reinitialize
	OSStatus				SetMaxFramesPerRender (UInt32 inMaxFrames);
		
	// Prepares the AU for processing at the specified format
	// only doing n-n channel processing here.. could extend this for n-m channel processing easily enough
	// if you are processing using an Offline AU, you HAVE to specify the numInputSamples to process
	// if you are processing real-time, then inNumInputSamples should be zero
	OSStatus				Initialize (const CAStreamBasicDescription &inIODesc, UInt64 inNumInputSamples = 0)
							{
								return Initialize (inIODesc, inIODesc, inNumInputSamples);
							}
	OSStatus				Initialize (const CAStreamBasicDescription 		&inInputDesc, 
										const CAStreamBasicDescription 		&inOutputDesc, 
										UInt64 								inNumInputSamples = 0);
	
		// takes the existing format state of the AU, but resets the AUProcessor so that:
		// applies an optional new number of max frames
		// the AUProcessor will have to be Preflighted again after this call
		// Use this when you want to keep the same formats, same input sample count,
		// but want to re-initialize the AU to use a new num max frames and prepare it 
		// appropriately...
	OSStatus				Reinitialize (UInt32 inNewMaxFrames);



#pragma mark __Render APIs
	// Preflighting - you HAVE to preflight
	// returns noErr when preflighting is done
	
	// Offline Context:
	// Offline AU's require preflighting, even if they will end up doing no work.
	// In the case where the AU is offline the latency samples are trimmed and not returned 
	// from the consequent Render calls
	
	// If you are NOT an offline AU then the first time Render is called, 
	// there will need to be some consumption of the input before you will get any valid output data.
	// This is determined by the value of the AU's reported latency. So, in this case preflight
	// will consume those initial samples and then return.
	// Your InputCallback MUST provide the initial data that is to be processed.
	
	// RealTime Context:
	// (1 No priming. In this case inProcessPreceedingTail is false (the default) and no
	// processing is done on the AU. However, because preflight is the first time that Rendering
	// is called on an AU we can presume that this is outside the context of the processing that
	// is occuring in the host application. To avoid taking page faults, priming cold memory, etc.
	// when the AU *is* consequently used to render real data, the AU is prerolled and reset if this is the case.

	// (2) Prime the AU's buffers with input data that preceeds the portion of audio you
	// want to process. This amount of input data is equivalent to the tail time of the AU
	// which expresses how long it takes for an sample on input to disappear from the output
	// (Think of a delay or a reverb)
	// So, by setting inProcessPreceedingTail to true, the preflight call will consume
	// the tail time samples of input (which should be valid input BEFORE the data to be processed)
	// So, your input proc here will HAVE to read samples from the preceeding input. The time
	// stamps to your input proc in this case can be interpreted as 0 is the start of the preceeding data
	// and you'll need to read up to the numSamples of tail time of the preceeding data.
	// It discards the output results of the preflighting. If you don't have any previous data
	// with which to prepare the AU, then you do NOT need to take this path and can do the step below.
	
	// In both of these cases the inital latency frames are *not* trimmed from
	// the output that will be generated by a consequent call to render.
	OSStatus				Preflight (bool inProcessPreceedingTail = false);

	// this can be used with an OfflineAU to do a gradual preflight (so you could for instance, 
	// present a progress indicator. The Preflight call will do it all at once.
	// If you break out of the preflight before you are done, then you HAVE to reinitialise the AU
	// before you can either do another preflight or do a render, otherwise the Render call will produce
	// invalid results.
	OSStatus 				OfflineAUPreflight (UInt32 inNumFrames, bool &outIsDone);

	// Here's the major stage that produces output.
	// You pass in two bool flags and an ioNumFrames....
	
	// So ioNumFrames will be *untouched* if outOLCompleted == false
	// if outOLCompleted is true, ioNumFrames contains the number of valid frames within the ioData data that are valid data
	// (ioData's pointers and sizes will also have been resized to match these number of frames)
	// outOLRequiresPostProcess should also be checked when outOLCompleted is true, 
	// to see if any post-processing is required
	// if rendering in a RT context, then neither outOLCompleted or outOLRequiresPostProcess is required
	// if rendering in an OL context, both must be specified.
	// Caller MUST provide an ABL that is set up in the previously specified output format
	
	// If you receive kAudioUnitErr_InvalidOfflineRender when dealing with an OfflineAU Type, then you have either not
	// preflighted it (and the AU requires it) or you have changed some state (its num input samples, its start offest)
	// and it needs to be re-preflighted. In that case, preflight the AU again first.
	OSStatus				Render (AudioBufferList 			*ioData, 
									UInt32 						&ioNumFrames, 
									bool						&outIsSilence,
									bool 						*outOLCompleted = NULL, 
									bool 						*outOLRequiresPostProcess = NULL);
	
	// call this method if outOLRequiresPostProcess returned true upon rendering completion
	OSStatus				PostProcess (AudioBufferList *ioData, UInt32 &ioNumFrames, bool &outIsSilence, bool &outDone);
	
	// This method returns how many input samples will need to be provided before
	// valid output data is produced.
	UInt32					LatencySampleCount () const { return mLatencySamples; }
	
	UInt32					TailSampleCount () const { return mTailSamples; }
	
	UInt64					InputSampleCount () const { return mNumInputSamples; }
		
	// the class maintains the TimeStamp from render call to render call
	// this value is the sample count that will be used in the *next* render call
	Float64					SampleTime () const { return mRenderTimeStamp.mSampleTime; }
	
		// in some settings (for instance a delay with 100% feedback) tail time is essentially infinite
		// so you should safeguard the final OL render stage (post process) which is aimed at pulling the tail through
		// default is 10 seconds
	Float64					GetMaxTailTime () const { return mMaxTailTime; }
	
	// this ONLY takes affect when you initialise the processor, it doesn't change that AU's tail time,
	// but just the max amount of time that this object will allow the AU to post-process data
	// set this to 0, to use the AU's tail time with no adjustment
	void					SetMaxTailTime (Float64 inTailTimeSecs) { mMaxTailTime = inTailTimeSecs; }

#if !TARGET_OS_IPHONE
	// if this is NULL, then there is no explicit (optional or required) preflight requirement of the AU
	// if this is valid, then the AU either requires or optionally requires preflighting. The default
	// behaviour in this case is that the processor will preflight. This name can be used to preset
	// to the user. If the AU doesn't present a name, but preflighting is optional/required, then the 
	// processor return "Preflight" as its string.
	// the String should be released by the caller.
	CFStringRef				GetOLPreflightName () const;

	// Returns true if this is processor is an offline AU, and that AU (whether optional or required) 
	// needs preflighting
	bool					OfflineAUNeedsPreflight () const;
	
	// this provides a rough approximation of the progress of an Offline Processing operation (both for the
	// preflight and the render phases)
	Float32					GetOLPercentComplete ();
#endif
	
private:
	CAAudioUnit 			mUnit;
	UInt32					mLatencySamples;
	UInt32					mTailSamples;
	UInt32					mTailSamplesToProcess;
	UInt64					mNumInputSamples;
	AudioTimeStamp			mRenderTimeStamp;
	bool					mPreflightDone;
	AUOutputBL *			mPreflightABL;
	SInt32					mTailSamplesRemaining;
	AURenderCallbackStruct	mUserCallback;
	Float64					mMaxTailTime;
	Float32					mLastPercentReported;
	
	bool			IsOfflineAU () const { return mUnit.Comp().Desc().IsOffline(); }

	void			CalculateRemainderSamples (Float64 inSampleRate);
	
	OSStatus		DoInitialisation (const CAStreamBasicDescription 	&inInputFormat,
									const CAStreamBasicDescription 		&inOutputFormat,
									UInt64								inNumInputSamples,
									UInt32 								inMaxFrames);

		// stop automatic copying of this object
	CAAUProcessor () {}
	CAAUProcessor (const CAAUProcessor &c) {}
	CAAUProcessor& operator= (const CAAUProcessor& c) { return *this; }
};

#endif //__CAAUProcessor_h__

/*
     File: CAAUProcessor.cpp
 Abstract: CAAUProcessor.h
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
#include "CAAUProcessor.h"						
#include "CAXException.h"

static OSStatus SilenceInputCallback (void 		*inRefCon, 
					AudioUnitRenderActionFlags *ioActionFlags, 
					const AudioTimeStamp 		*inTimeStamp, 
					UInt32 						inBusNumber, 
					UInt32 						inNumberFrames, 
					AudioBufferList 			*ioData)
{
	AudioBuffer *buf = ioData->mBuffers;
	for (UInt32 i = ioData->mNumberBuffers; i--; ++buf)
		memset((Byte *)buf->mData, 0, buf->mDataByteSize);
		
		//provide a hint that our input data is silent.
	*ioActionFlags &= kAudioUnitRenderAction_OutputIsSilence;
	return noErr;
}

static AURenderCallbackStruct sSilentCallback = { SilenceInputCallback, NULL };


CAAUProcessor::CAAUProcessor (const CAComponent& inComp)
	: mPreflightABL(NULL)
{
	OSStatus result = CAAudioUnit::Open (inComp, mUnit);
	if (result)
		throw result;
	memset (&mUserCallback, 0, sizeof (AURenderCallbackStruct));
	mMaxTailTime = 10.;
}

CAAUProcessor::~CAAUProcessor ()
{
	if (mPreflightABL)
		delete mPreflightABL;
}

inline OSStatus		SetInputCallback (CAAudioUnit &inUnit, AURenderCallbackStruct &inInputCallback)
{
	return inUnit.SetProperty (kAudioUnitProperty_SetRenderCallback, 
											kAudioUnitScope_Input, 
											0,
											&inInputCallback, 
											sizeof(inInputCallback));
}

static AURenderCallbackStruct sRenderCallback;
static OSStatus PrerollRenderProc (	void 						* /*inRefCon*/, 
								AudioUnitRenderActionFlags		* /*inActionFlags*/,
								const AudioTimeStamp 			* /*inTimeStamp*/, 
								UInt32 							/*inBusNumber*/,
								UInt32							/*inNumFrames*/, 
								AudioBufferList 				*ioData)
{
	AudioBuffer *buf = ioData->mBuffers;
	for (UInt32 i = ioData->mNumberBuffers; i--; ++buf)
		memset((Byte *)buf->mData, 0, buf->mDataByteSize);

	return noErr;
}

OSStatus 	Preroll (CAAudioUnit & inAU, UInt32 inFrameSize)
{
	CAStreamBasicDescription desc;
	OSStatus result = inAU.GetFormat (kAudioUnitScope_Input, 0, desc);
	bool hasInput = false;
			//we have input	
	if (result == noErr) 
	{
		sRenderCallback.inputProc = PrerollRenderProc;
		sRenderCallback.inputProcRefCon = 0;
		
		result = inAU.SetProperty (kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 
								0, &sRenderCallback, sizeof(sRenderCallback));
		if (result) return result;
		hasInput = true;
	}
	
	AudioUnitRenderActionFlags flags = 0;
	AudioTimeStamp time;
	memset (&time, 0, sizeof(time));
	time.mFlags = kAudioTimeStampSampleTimeValid;

	CAStreamBasicDescription outputFormat;
	ca_require_noerr (result = inAU.GetFormat (kAudioUnitScope_Output, 0, outputFormat), home);
	{
		AUOutputBL list (outputFormat, inFrameSize);
		list.Prepare ();
		
		result = inAU.Render (&flags, &time, 0, inFrameSize, list.ABL());
		if (result) { printf("A result %d\n", (int)result); goto home; }
	}

home:
	if (hasInput) {
            // remove our installed callback
		sRenderCallback.inputProc = 0;
		sRenderCallback.inputProcRefCon = 0;
		
		inAU.SetProperty (kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 
								0, &sRenderCallback, sizeof(sRenderCallback));
	}
	return result;
}


OSStatus		CAAUProcessor::EstablishInputCallback (AURenderCallbackStruct &inInputCallback)
{
	OSStatus result = SetInputCallback (mUnit, inInputCallback);
	if (!result)
		memcpy (&mUserCallback, &inInputCallback, sizeof(AURenderCallbackStruct));
	else
		memset (&mUserCallback, 0, sizeof (AURenderCallbackStruct));
	return result;
}

OSStatus		CAAUProcessor::SetAUPreset (CFPropertyListRef 			inPreset)
{
	return mUnit.SetProperty (kAudioUnitProperty_ClassInfo, 
									kAudioUnitScope_Global, 
									0, 
									&inPreset, 
									sizeof(inPreset));
}

OSStatus		CAAUProcessor::SetAUPresetIndex (SInt32				inPresetIndex)
{
	AUPreset aup;
	aup.presetName = NULL;
	aup.presetNumber = inPresetIndex;
	return mUnit.SetPresentPreset(aup);
}


OSStatus		CAAUProcessor::SetParameter (AudioUnitParameterID inID, AudioUnitScope scope, AudioUnitElement element,
											Float32 value, UInt32 bufferOffsetFrames)
{
	return mUnit.SetParameter(inID, scope, element, value, bufferOffsetFrames);
}


UInt32			CAAUProcessor::MaxFramesPerRender () const
{
	UInt32 maxFrames;
	UInt32 propSize = sizeof (maxFrames);
	if (mUnit.GetProperty (kAudioUnitProperty_MaximumFramesPerSlice,
							kAudioUnitScope_Global, 0, &maxFrames, &propSize))
	{
		return 0;
	}
	return maxFrames;
}

OSStatus		CAAUProcessor::SetMaxFramesPerRender (UInt32 inMaxFrames)
{
	return mUnit.SetProperty (kAudioUnitProperty_MaximumFramesPerSlice,
							kAudioUnitScope_Global, 0, &inMaxFrames, sizeof(inMaxFrames));
}

OSStatus		CAAUProcessor::Initialize (const CAStreamBasicDescription 	&inInputDesc,
											const CAStreamBasicDescription 	&inOutputDesc,
												UInt64 						inNumInputSamples)
{
	return DoInitialisation (inInputDesc, inOutputDesc, inNumInputSamples, MaxFramesPerRender());
}

OSStatus		CAAUProcessor::Reinitialize (UInt32 inNewMaxFrames)
{
	OSStatus result;
	CAStreamBasicDescription inputDesc, outputDesc;
	
	ca_require_noerr (result = mUnit.GetFormat (kAudioUnitScope_Input, 0, inputDesc), home);
	ca_require_noerr (result = mUnit.GetFormat (kAudioUnitScope_Output, 0, outputDesc), home);
	
	ca_require_noerr (result = DoInitialisation (inputDesc, outputDesc, mNumInputSamples, inNewMaxFrames), home);
	
home:
	return result;
}


OSStatus		CAAUProcessor::DoInitialisation (const CAStreamBasicDescription 	&inInputFormat,
												const CAStreamBasicDescription 		&inOutputFormat,
												UInt64								inNumInputSamples,
												UInt32 								inMaxFrames)
{
	OSStatus result;
	
	if (inNumInputSamples == 0 && IsOfflineAU())
		return kAudioUnitErr_InvalidOfflineRender;
		
	mNumInputSamples = inNumInputSamples;
	
		// first check that we can do this number of channels
	if (mUnit.CanDo (inInputFormat.NumberChannels(), inOutputFormat.NumberChannels()) == false)
		ca_require_noerr (result = kAudioUnitErr_FailedInitialization, home);
	
	// just uninitialise the AU as a matter of course
	ca_require_noerr (result = mUnit.Uninitialize(), home);

	ca_require_noerr (result = mUnit.SetFormat (kAudioUnitScope_Input, 0, inInputFormat), home); 
	ca_require_noerr (result = mUnit.SetFormat (kAudioUnitScope_Output, 0, inOutputFormat), home); 
	ca_require_noerr (result = SetMaxFramesPerRender (inMaxFrames), home);
	
#if !TARGET_OS_IPHONE
		// if we're any AU but an offline AU, we should tell it that we've processing offline
	if (!IsOfflineAU()) {
		UInt32 isOffline = (IsOfflineContext() ? 1 : 0);
			// don't care whether this succeeds of fails as many AU's don't care about this
			// but the ones that do its important that they are told their render context
		mUnit.SetProperty (kAudioUnitProperty_OfflineRender, kAudioUnitScope_Global, 0, &isOffline, sizeof(isOffline));
	} else {
			// tell the offline unit how many input samples we wish to process...
		mUnit.SetProperty (kAudioUnitOfflineProperty_InputSize,
												kAudioUnitScope_Global, 0,
												&mNumInputSamples, sizeof(mNumInputSamples));
	}
#endif
	
	ca_require_noerr (result = mUnit.Initialize(), home);

	ca_require_noerr (result = SetInputCallback (mUnit, mUserCallback), home);
	
	// finally reset our time stamp
	// the time stamp we use with the AU Render - only sample count is valid
	memset (&mRenderTimeStamp, 0, sizeof(mRenderTimeStamp));
	mRenderTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;

	// now, if we're NOT an offline AU, preflighting is not required
	// if we are an offline AU, we should preflight.. an offline AU will tell us when its preflighting is done
	mPreflightDone = false;

	if (mPreflightABL) {
		delete mPreflightABL;
		mPreflightABL = NULL;
	}
	
	mPreflightABL = new AUOutputBL (inOutputFormat);

	mLastPercentReported = 0;
	
home:
	return result;
}

void		CAAUProcessor::CalculateRemainderSamples (Float64 inSampleRate)
{
	mLatencySamples = 0;
	mTailSamplesToProcess = 0;
	mTailSamples = 0;
	mTailSamplesRemaining = 0;
	return;
	
		// nothing to do because we're not processing offline
	if (IsOfflineContext() == false) return;
		
		// because an offline unit has some indeterminancy about what it does with the input samples
		// it is *required* to deal internally with both latency and tail
	if (!IsOfflineAU()) 
	{
			// when offline we need to deal with both latency and tail
			
		// if the AU has latency - how many samples at the start will be zero?
		// we'll end up chucking these away.
		Float64 renderTimeProps;
		UInt32 propSize = sizeof (renderTimeProps);
		OSStatus result = mUnit.GetProperty (kAudioUnitProperty_Latency, kAudioUnitScope_Global, 0,
													&renderTimeProps, &propSize);
		
		Float64 latencySamples = 0;
		if (result == noErr) // we have latency to deal with - its reported in seconds
			latencySamples = renderTimeProps * inSampleRate;
			
			// AU tail
			// if the AU has a tail - we'll pull that many zeroes through at the end to flush
			// out this tail - think of a decaying digital delay or reverb...
		result = mUnit.GetProperty (kAudioUnitProperty_TailTime, kAudioUnitScope_Global, 0,
													&renderTimeProps, &propSize);
		if (renderTimeProps > mMaxTailTime)
			renderTimeProps = mMaxTailTime;
		Float64 tailSamples = 0;
		if (result == noErr)
			tailSamples = renderTimeProps * inSampleRate;
		
		// this dictates how many samples at the end we need to pull through...
		// we add latency to tail because we throw the latency samples away from the start of the rendering
		// and we have to pull that many samples after the end of course to get the last of the original data
		// then to that is added the tail of the effect...
		mTailSamplesToProcess = UInt32(tailSamples + latencySamples);
		mTailSamples = UInt32(tailSamples);
		mLatencySamples = UInt32(latencySamples);
	}
}

#if !TARGET_OS_IPHONE
CFStringRef		CAAUProcessor::GetOLPreflightName () const
{
	if (OfflineAUNeedsPreflight()) 
	{
		CFStringRef str;
		UInt32 size = sizeof(str);
		OSStatus result = mUnit.GetProperty (kAudioUnitOfflineProperty_PreflightName,
												kAudioUnitScope_Global, 0,
												&str, &size);
		return result ? NULL : str;
	}
	return NULL; // says NO to preflighting
}

bool		CAAUProcessor::OfflineAUNeedsPreflight () const
{
	if (IsOfflineAU()) {
		UInt32 preflightRequirements;
		UInt32 size = sizeof(preflightRequirements);
		OSStatus result = mUnit.GetProperty (kAudioUnitOfflineProperty_PreflightRequirements,
												kAudioUnitScope_Global, 0,
												&preflightRequirements, &size);
		if (result)
			return false;
		return preflightRequirements;
	}
	return false;
}
#endif

OSStatus	CAAUProcessor::Preflight (bool inProcessPreceedingTail)
{
	printf(">>>>CAAUProcessor::Preflight\n");
		//we're preflighting again, so reset ourselves
	if (mPreflightDone) {
		mPreflightDone = false;
		// the time stamp we use with the AU Render - only sample count is valid
		memset (&mRenderTimeStamp, 0, sizeof(mRenderTimeStamp));
		mRenderTimeStamp.mFlags = kAudioTimeStampSampleTimeValid;
		mUnit.GlobalReset();
	}

	Float64 sampleRate;
	OSStatus result = mUnit.GetSampleRate (kAudioUnitScope_Output, 0, sampleRate);
	CalculateRemainderSamples (sampleRate);

	UInt32 numFrames = MaxFramesPerRender();
	if (numFrames == 0)
		return kAudioUnitErr_InvalidProperty;
	
	if (!IsOfflineAU()) 
	{
		if ((IsOfflineContext() == false && inProcessPreceedingTail) || IsOfflineContext())
		{
			// re-establish the user's input callback
			ca_require_noerr (result = SetInputCallback (mUnit, mUserCallback), home);

			// Consume the number of input samples indicated by the AU's latency or tail
			// based on whether the AU is being used in an offline context or not.
			
			UInt32 latSamps = IsOfflineContext() ? mLatencySamples : mTailSamples;	
			printf("latSamps %d\n", (int)latSamps);
			latSamps = 0;
			while (latSamps > 0)
			{
				if (latSamps < numFrames)
					numFrames = latSamps;
					
					// process the samples (the unit's input callback will read the samples
					// from the file and convert them to float for processing
				AudioUnitRenderActionFlags renderFlags = 0;
				mPreflightABL->Prepare();
				result = mUnit.Render (&renderFlags, &mRenderTimeStamp, 0, numFrames, mPreflightABL->ABL());
				if (result) { printf("B result %d\n", (int)result); goto home; }
		
				mRenderTimeStamp.mSampleTime += numFrames;
				latSamps -= numFrames;
			}
			if (IsOfflineContext())
				mRenderTimeStamp.mSampleTime = mLatencySamples;
		}
		else
		{
			// processing real-time but not processing preceeding tail, so we should preroll the AU
			ca_require_noerr (result = Preroll(mUnit, numFrames), home);
			
			// re-establish the user's input callback
			ca_require_noerr (result = SetInputCallback (mUnit, mUserCallback), home);
			
			mRenderTimeStamp.mSampleTime = 0;
		}
	}
#if !TARGET_OS_IPHONE
	else
	{
			// re-establish the user's input callback
		ca_require_noerr (result = SetInputCallback (mUnit, mUserCallback), home);
		
		UInt32 preflightRequirements;
		UInt32 size; size = sizeof(preflightRequirements);
		ca_require_noerr (result = mUnit.GetProperty (kAudioUnitOfflineProperty_PreflightRequirements,
												kAudioUnitScope_Global, 0,
												&preflightRequirements, &size), home);
												
			// 0 indicates none, otherwise optional or required -> we do it for either
		if (preflightRequirements) 
		{
			for (;;) {
				// here we need to do the preflight loop - we don't expect any data back, but have to 
				// give the offline unit all of its input data to allow it to prepare its processing
				AudioUnitRenderActionFlags renderFlags = kAudioOfflineUnitRenderAction_Preflight;
				mPreflightABL->Prepare();
				result = mUnit.Render (&renderFlags, &mRenderTimeStamp, 0, numFrames, mPreflightABL->ABL());
				if (result) { printf("C result %d\n", (int)result); goto home; }
				mRenderTimeStamp.mSampleTime += numFrames;
		
				if (renderFlags & kAudioOfflineUnitRenderAction_Complete)
					break;
			}
		}
		// the time stamp we use with the AU Render - only sample count is valid
		mRenderTimeStamp.mSampleTime = 0;
	}
#endif

	if (result == noErr) {
		mPreflightDone = true;
	}
	
home:
	printf("<<<<CAAUProcessor::Preflight\n");
	return result;
}

#if !TARGET_OS_IPHONE
OSStatus 	CAAUProcessor::OfflineAUPreflight (UInt32 inNumFrames, bool &outIsDone)
{
	if (!IsOfflineAU())
		return -50/*paramErr*/;
	if (mNumInputSamples == 0)
		return -50/*paramErr*/;

	UInt32 preflightRequirements;
	UInt32 size = sizeof(preflightRequirements);
	OSStatus result;
	ca_require_noerr (result = mUnit.GetProperty (kAudioUnitOfflineProperty_PreflightRequirements,
												kAudioUnitScope_Global, 0,
												&preflightRequirements, &size), home);
												
		// 0 indicates none, otherwise optional or required -> we do it for either
	if (preflightRequirements) 
	{
		AudioUnitRenderActionFlags renderFlags = kAudioOfflineUnitRenderAction_Preflight;
		mPreflightABL->Prepare();
		result = mUnit.Render (&renderFlags, &mRenderTimeStamp, 0, inNumFrames, mPreflightABL->ABL());
		if (result) { printf("D result %d\n", (int)result); goto home; }
		mRenderTimeStamp.mSampleTime += inNumFrames;
		
		if (renderFlags & kAudioOfflineUnitRenderAction_Complete) {
			outIsDone = true;
			mRenderTimeStamp.mSampleTime = 0;
			mPreflightDone = true;
			mLastPercentReported = 0;
		}
	}
	else
	{
		outIsDone = true;
		mRenderTimeStamp.mSampleTime = 0;
		mPreflightDone = true;
		mLastPercentReported = 0;
	}
	
home:
	return result;
}
#endif

void SetBufferListToNumFrames (AudioBufferList &list, UInt32 inNumFrames)
{
	for (unsigned int i = 0; i < list.mNumberBuffers; ++i) {
		AudioBuffer &buf = list.mBuffers[i];
		if (buf.mDataByteSize > 0)
			buf.mDataByteSize = inNumFrames * sizeof (Float32);
	}
}

OSStatus	CAAUProcessor::Render (AudioBufferList 		*ioData, 
									UInt32 				&ioNumFrames, 
									bool				&outIsSilence,
									bool 				*outOLCompleted, 
									bool 				*outOLRequiresPostProcess)
{
	if (IsOfflineContext())
	{
		if (!mPreflightDone)
			return kAudioUnitErr_InvalidOfflineRender;
	
			// YES - this is correct!!! you have to provide both if rendering in an offline Context
		*outOLCompleted = false;
		*outOLRequiresPostProcess = false;

		if (!IsOfflineAU() && !mUnit.Comp().Desc().IsFConv()) 
		{
				// have we processed the input we expect too?
				// in an offline case, we want to create output that matches the input
				// for an OfflineAU type, it manages this internally, so we don't have to do anything
				// for a FormatConverter AU, we don't know and can't tell, so we can't do anything here
				// for any other AU type (effect, instrument) the Prime assumption is that it will 
				// ask for the same number of frames of input as it is asked to output
				// so we can ask what it is doing, and get a sample accurate output (which is input + tail time)
			if (mRenderTimeStamp.mSampleTime + ioNumFrames >= InputSampleCount()) 
			{
					// if we fall into here, we have just a partial number of input samples left 
					// (less input less than what we've been asked to produce output for.
				*outOLCompleted = true;
					// we require post processing if we've got some tail (or latency) samples to flush through
				*outOLRequiresPostProcess = mTailSamplesToProcess > 0;
				if (InputSampleCount() > mRenderTimeStamp.mSampleTime) {
					ioNumFrames = (UInt32)(InputSampleCount() - mRenderTimeStamp.mSampleTime);
				} else {
					ioNumFrames = 0;
				}
				mTailSamplesRemaining = mTailSamplesToProcess;
					// we've got no input samples to process this time.
				SetBufferListToNumFrames (*ioData, ioNumFrames);
				if (ioNumFrames == 0) {
					if (*outOLRequiresPostProcess)
						SetInputCallback (mUnit, sSilentCallback);
					else
						mUnit.GlobalReset (); //flush this out, as we're done with this phase
					return noErr;
				}
			}
		}
		AudioUnitRenderActionFlags renderFlags = IsOfflineAU() ? kAudioOfflineUnitRenderAction_Render : 0;
		OSStatus result = mUnit.Render (&renderFlags, &mRenderTimeStamp, 0, ioNumFrames, ioData);
		if (result) { printf("E result %d\n", (int)result); }
		if (result) {
			if (mUnit.Comp().Desc().IsFConv()) { 
				// this is the only way we can tell we're done with a FormatConverter AU 
				// - ie. client returns an error from input
				result = noErr;
				*outOLCompleted = true;
				*outOLRequiresPostProcess = mTailSamplesToProcess > 0;
				ioNumFrames = 0;
				SetBufferListToNumFrames (*ioData, ioNumFrames);
			} else
				return result;
		}
//	for (UInt32 i = 0; i < ioNumFrames; ++i) {
//		union {
//			float f;
//			unsigned char c[4];
//		} u;
//		u.f = ((float*)(ioData->mBuffers[0].mData))[i];
//		printf("aup out %4d  %14.10f  %02X %02X %02X %02X\n", (int)i, u.f, u.c[0], u.c[1], u.c[2], u.c[3]);
//	}
		mRenderTimeStamp.mSampleTime += ioNumFrames;
		outIsSilence = (renderFlags & kAudioUnitRenderAction_OutputIsSilence);
		
			// if we're an Offline AU type, it will set this flag on completion of its processing
		if (renderFlags & kAudioOfflineUnitRenderAction_Complete) {
			// we now need to calculate how many frames we rendered.
				// as we're dealing with PCM non-interleaved buffers, we can calculate the numFrames simply
			ioNumFrames = ioData->mBuffers[0].mDataByteSize / sizeof(Float32);
			*outOLCompleted = true;
			*outOLRequiresPostProcess = false;
			mUnit.GlobalReset (); //flush this out, as we're done with this phase
		} else {
			if (*outOLCompleted) {
				if (*outOLRequiresPostProcess)
					result = SetInputCallback (mUnit, sSilentCallback);
				else
					mUnit.GlobalReset (); //flush this out, as we're done with this phase
			}
		}
	
		return result;
	}

// rendering in a RT context:
	AudioUnitRenderActionFlags renderFlags = 0;
	OSStatus result = mUnit.Render (&renderFlags, &mRenderTimeStamp, 0, ioNumFrames, ioData);
		if (result) { printf("F result %d\n", (int)result); }
	if (!result) {
		mRenderTimeStamp.mSampleTime += ioNumFrames;
		outIsSilence = (renderFlags & kAudioUnitRenderAction_OutputIsSilence);
	}
//	for (UInt32 i = 0; i < ioNumFrames; ++i) {
//		union {
//			float f;
//			unsigned char c[4];
//		} u;
//		u.f = ((float*)(ioData->mBuffers[0].mData))[i];
//		printf("aup out %4d  %14.10f  %02X %02X %02X %02X\n", (int)i, u.f, u.c[0], u.c[1], u.c[2], u.c[3]);
//	}
	
	return result;	
}
	
OSStatus	CAAUProcessor::PostProcess (AudioBufferList 	*ioData, 
										UInt32 				&ioNumFrames, 
										bool				&outIsSilence,
										bool 				&outDone)
{
	if (IsOfflineAU() || !IsOfflineContext()) 
		return kAudioUnitErr_CannotDoInCurrentContext;
	
	outDone = false;
	
		// we've got less samples to process than we've been asked to process
	if (mTailSamplesRemaining <= SInt32(ioNumFrames)) {
		outDone = true;
		ioNumFrames = mTailSamplesRemaining > 0 ? mTailSamplesRemaining : 0;
		SetBufferListToNumFrames (*ioData, ioNumFrames);
		if (ioNumFrames == 0)
			return noErr;
	}
	
	AudioUnitRenderActionFlags renderFlags = 0;
	OSStatus result;
	result = mUnit.Render (&renderFlags, &mRenderTimeStamp, 0, ioNumFrames, ioData);
		if (result) { printf("G result %d\n", (int)result); goto home; }
	mRenderTimeStamp.mSampleTime += ioNumFrames;
	mTailSamplesRemaining -= ioNumFrames;
	outIsSilence = (renderFlags & kAudioUnitRenderAction_OutputIsSilence);
			
	if (outDone) {
		ca_require_noerr (result = SetInputCallback (mUnit, mUserCallback), home);
		mUnit.GlobalReset (); //flush this out, as we're done with this phase
	}
home:
	return result;
}		

#if !TARGET_OS_IPHONE
Float32		CAAUProcessor::GetOLPercentComplete ()
{
	if (!IsOfflineContext())
		return 0;

	Float32 percentDone = mLastPercentReported;
		
	if (IsOfflineAU()) 
	{
		// we get the output size every time, as this can change as parameters are changed
		UInt64 numOutputSamples = mNumInputSamples;
		UInt32 propSize = sizeof(numOutputSamples);
		mUnit.GetProperty (kAudioUnitOfflineProperty_OutputSize,
							kAudioUnitScope_Global, 0, &numOutputSamples, &propSize);
		
		percentDone = (mRenderTimeStamp.mSampleTime / Float64(numOutputSamples)) * 100.;
	}
	else
	{
		percentDone = (mRenderTimeStamp.mSampleTime / Float64(mNumInputSamples + mTailSamples)) * 100.;
	}
	
	if (percentDone > mLastPercentReported)
		mLastPercentReported = percentDone;

	return mLastPercentReported;
}
#endif

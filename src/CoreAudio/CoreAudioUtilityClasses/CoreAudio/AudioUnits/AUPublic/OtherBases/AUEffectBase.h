/*
     File: AUEffectBase.h
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
#ifndef __AUEffectBase_h__
#define __AUEffectBase_h__

#include "AUBase.h"
#include "AUSilentTimeout.h"
#include "CAException.h"

class AUKernelBase;

//	Base class for an effect with one input stream, one output stream,
//	any number of channels.
	/*! @class AUEffectBase */
class AUEffectBase : public AUBase {
public:
	/*! @ctor AUEffectBase */
								AUEffectBase(	AudioComponentInstance		audioUnit,
												bool						inProcessesInPlace = true );
	/*! @dtor ~AUEffectBase */
								~AUEffectBase();
	
	/*! @method Initialize */
	virtual OSStatus			Initialize();

	/*! @method Cleanup */
	virtual void				Cleanup();


	/*! @method Reset */
	virtual OSStatus			Reset(		AudioUnitScope 				inScope,
											AudioUnitElement 			inElement);

	/*! @method GetPropertyInfo */
	virtual OSStatus			GetPropertyInfo (AudioUnitPropertyID	inID,
											AudioUnitScope				inScope,
											AudioUnitElement			inElement,
											UInt32 &					outDataSize,
											Boolean &					outWritable);

	/*! @method GetProperty */
	virtual OSStatus			GetProperty (AudioUnitPropertyID 		inID,
											AudioUnitScope 				inScope,
											AudioUnitElement	 		inElement,
											void *						outData);

	/*! @method SetProperty */
	virtual OSStatus			SetProperty(AudioUnitPropertyID 		inID,
											AudioUnitScope 				inScope,
											AudioUnitElement 			inElement,
											const void *				inData,
											UInt32 						inDataSize);

	/*! @method StreamFormatWritable */
	virtual bool				StreamFormatWritable (AudioUnitScope	scope,
											AudioUnitElement			element);

	/*! @method ChangeStreamFormat */
	virtual	OSStatus			ChangeStreamFormat (
										AudioUnitScope						inScope,
										AudioUnitElement					inElement,
										const CAStreamBasicDescription & 	inPrevFormat,
										const CAStreamBasicDescription &	inNewFormat);

	/*! @method Render */
	virtual OSStatus 	Render(AudioUnitRenderActionFlags &		ioActionFlags,
										const AudioTimeStamp &			inTimeStamp,
										UInt32							inNumberFrames);

	// our virtual methods
	
	// If your unit processes N to N channels, and there are no interactions between channels,
	// it can override NewKernel to create a mono processing object per channel.  Otherwise,
	// don't override NewKernel, and instead, override ProcessBufferLists.
	/*! @method NewKernel */
	virtual AUKernelBase *		NewKernel() { return NULL; }

	/*! @method ProcessBufferLists */
	virtual OSStatus			ProcessBufferLists(
											AudioUnitRenderActionFlags &	ioActionFlags,
											const AudioBufferList &			inBuffer,
											AudioBufferList &				outBuffer,
											UInt32							inFramesToProcess );

	// convenience format accessors (use output 0's format)
	/*! @method GetSampleRate */
	Float64						GetSampleRate();
	
	/*! @method GetNumberOfChannels */
	UInt32						GetNumberOfChannels();

	// convenience wrappers for accessing parameters in the global scope
	/*! @method SetParameter */
	using AUBase::SetParameter;
	void						SetParameter(			AudioUnitParameterID			paramID,
														AudioUnitParameterValue			value)
								{
									Globals()->SetParameter(paramID, value);
								}
								
	/*! @method GetParameter */
	using AUBase::GetParameter;
	AudioUnitParameterValue		GetParameter(			AudioUnitParameterID			paramID )
								{
									return Globals()->GetParameter(paramID );
								}

	/*! @method CanScheduleParameters */
	virtual bool				CanScheduleParameters() const { return true; }
	
	/*! @method IsBypassEffect */
	// This is used for the property value - to reflect to the UI if an effect is bypassed
	bool						IsBypassEffect () { return mBypassEffect; }
	
protected:
											
	/*! @method MaintainKernels */
	void						MaintainKernels();

	/*! @method ShouldBypassEffect */
	// This is used in the render call to see if an effect is bypassed
	// It can return a different status than IsBypassEffect (though it MUST take that into account)
	virtual	bool				ShouldBypassEffect () { return IsBypassEffect(); }
					
public:
	/*! @method SetBypassEffect */
	virtual void				SetBypassEffect (bool inFlag) { mBypassEffect = inFlag; }
	
	/*! @method SetParamHasSampleRateDependency */
	void						SetParamHasSampleRateDependency (bool inFlag) 
								{ 
									mParamSRDep = inFlag; 
								}
	
	/*! @method GetParamHasSampleRateDependency */
	bool						GetParamHasSampleRateDependency () const { return mParamSRDep; }

	struct ScheduledProcessParams	// pointer passed in as void* userData param for ProcessScheduledSlice()
	{
		AudioUnitRenderActionFlags 	*actionFlags;
		AudioBufferList 			*inputBufferList;
		AudioBufferList 			*outputBufferList;
	};

	virtual OSStatus			ProcessScheduledSlice(	void				*inUserData,
														UInt32				inStartFrameInBuffer,
														UInt32				inSliceFramesToProcess,
														UInt32				inTotalBufferFrames );


	bool							ProcessesInPlace() const {return mProcessesInPlace;};
	void							SetProcessesInPlace(bool inProcessesInPlace) {mProcessesInPlace = inProcessesInPlace;};
		
	typedef std::vector<AUKernelBase *> KernelList;
	
	

protected:	
	/*! @var mKernelList */
	KernelList						mKernelList;

	AUKernelBase* GetKernel(UInt32 index) { return mKernelList[index]; }

	/*! @method IsInputSilent */
	bool 							IsInputSilent (AudioUnitRenderActionFlags 	inActionFlags, UInt32 inFramesToProcess)
									{
										bool inputSilent = (inActionFlags & kAudioUnitRenderAction_OutputIsSilence) != 0;
									
										// take latency and tail time into account when propagating the silent bit
										UInt32 silentTimeoutFrames = UInt32(GetSampleRate() * (GetLatency() + GetTailTime()));
										mSilentTimeout.Process (inFramesToProcess, silentTimeoutFrames, inputSilent);
										return inputSilent;
									}
									
#if TARGET_OS_IPHONE
	void SetOnlyOneKernel(bool inUseOnlyOneKernel) { mOnlyOneKernel = inUseOnlyOneKernel; } // set in ctor of subclass that wants it.
#endif

	template <typename T>
	void	ProcessBufferListsT(
										AudioUnitRenderActionFlags &	ioActionFlags,
										const AudioBufferList &			inBuffer,
										AudioBufferList &				outBuffer,
										UInt32							inFramesToProcess );

	CAStreamBasicDescription::CommonPCMFormat GetCommonPCMFormat() const { return mCommonPCMFormat; }
	

private:
	/*! @var mBypassEffect */
	bool							mBypassEffect;
	/*! @var mParamSRDep */
	bool							mParamSRDep;
	
	/*! @var mProcessesInplace */
	bool							mProcessesInPlace;
	
	/*! @var mSilentTimeout */
	AUSilentTimeout					mSilentTimeout;

	/*! @var mMainOutput */
	AUOutputElement *				mMainOutput;
	
	/*! @var mMainInput */
	AUInputElement *				mMainInput;
	
#if TARGET_OS_IPHONE
	/*! @var mOnlyOneKernel */
	bool							mOnlyOneKernel;
#endif

	/*! @var mCommonPCMFormat */
	CAStreamBasicDescription::CommonPCMFormat		mCommonPCMFormat;
	UInt32							mBytesPerFrame;
};


//	Base class for a "kernel", an object that performs DSP on one channel of an interleaved stream.
	/*! @class AUKernelBase */
class AUKernelBase {
public:
	/*! @ctor AUKernelBase */
								AUKernelBase(AUEffectBase *inAudioUnit ) :
									mAudioUnit(inAudioUnit) { }

	/*! @dtor ~AUKernelBase */
	virtual						~AUKernelBase() { }

	/*! @method Reset */
	virtual void				Reset() { }

	/*! @method Process */
	virtual void 				Process(	const Float32 *						inSourceP,
											Float32 *							inDestP,
											UInt32								inFramesToProcess,
											UInt32								inNumChannels,
											bool &								ioSilence) { throw CAException(kAudio_UnimplementedError ); }

	/*! @method Process */
	virtual void 				Process(	const SInt32 *						inSourceP,
											SInt32 *							inDestP,
											UInt32								inFramesToProcess,
											UInt32								inNumChannels,
											bool &								ioSilence) { throw CAException(kAudio_UnimplementedError ); }

	/*! @method Process */
	virtual void 				Process(	const SInt16 *						inSourceP,
											SInt16 *							inDestP,
											UInt32								inFramesToProcess,
											UInt32								inNumChannels,
											bool &								ioSilence) { throw CAException(kAudio_UnimplementedError ); }

	/*! @method GetSampleRate */
	Float64						GetSampleRate()
								{
									return mAudioUnit->GetSampleRate();
								}
								
	/*! @method GetParameter */
	AudioUnitParameterValue		GetParameter (AudioUnitParameterID	paramID) 
								{
									return mAudioUnit->GetParameter(paramID);
								}
	
	void						SetChannelNum (UInt32 inChan) { mChannelNum = inChan; }
	UInt32						GetChannelNum () { return mChannelNum; }
	
protected:
	/*! @var mAudioUnit */
	AUEffectBase * 		mAudioUnit;
	UInt32				mChannelNum;

};

template <typename T>
void	AUEffectBase::ProcessBufferListsT(
									AudioUnitRenderActionFlags &	ioActionFlags,
									const AudioBufferList &			inBuffer,
									AudioBufferList &				outBuffer,
									UInt32							inFramesToProcess )
{
	bool ioSilence;

	bool silentInput = IsInputSilent (ioActionFlags, inFramesToProcess);
	ioActionFlags |= kAudioUnitRenderAction_OutputIsSilence;

	// call the kernels to handle either interleaved or deinterleaved
	if (inBuffer.mNumberBuffers == 1) {
		if (inBuffer.mBuffers[0].mNumberChannels == 0)
			throw CAException(kAudio_ParamError);
			
		for (UInt32 channel = 0; channel < mKernelList.size(); ++channel) {
			AUKernelBase *kernel = mKernelList[channel];
			
			if (kernel == NULL) continue;
			ioSilence = silentInput;
			
			// process each interleaved channel individually
			kernel->Process(
				(const T *)inBuffer.mBuffers[0].mData + channel, 
				(T *)outBuffer.mBuffers[0].mData + channel,
				inFramesToProcess,
				inBuffer.mBuffers[0].mNumberChannels,
				ioSilence);
				
			if (!ioSilence)
				ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;
		}
	} else {
		for (UInt32 channel = 0; channel < mKernelList.size(); ++channel) {
			AUKernelBase *kernel = mKernelList[channel];
			
			if (kernel == NULL) continue;
			
			ioSilence = silentInput;
			const AudioBuffer *srcBuffer = &inBuffer.mBuffers[channel];
			AudioBuffer *destBuffer = &outBuffer.mBuffers[channel];
			
			kernel->Process(
				(const T *)srcBuffer->mData, 
				(T *)destBuffer->mData, 
				inFramesToProcess,
				1,
				ioSilence);
				
			if (!ioSilence)
				ioActionFlags &= ~kAudioUnitRenderAction_OutputIsSilence;
		}
	}
}


#endif // __AUEffectBase_h__

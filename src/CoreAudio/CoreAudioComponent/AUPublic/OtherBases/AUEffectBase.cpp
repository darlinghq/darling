/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUBase Classes
*/

#include "AUEffectBase.h"

/* 
	This class does not deal as well as it should with N-M effects...
	
	The problem areas are (if the channels don't match):
		ProcessInPlace if the channels don't match - there will be problems if InputChan != OutputChan
		Bypass - its just passing the buffers through when not processing them

	This will be fixed in a future update...
*/

//_____________________________________________________________________________
//
AUEffectBase::AUEffectBase(	AudioComponentInstance	audioUnit,
							bool					inProcessesInPlace ) :
	AUBase(audioUnit, 1, 1),		// 1 in bus, 1 out bus
	mBypassEffect(false),
	mParamSRDep (false),
	mProcessesInPlace(inProcessesInPlace),
	mMainOutput(NULL), mMainInput(NULL)
#if TARGET_OS_IPHONE
	, mOnlyOneKernel(false)
#endif
{
}

//_____________________________________________________________________________
//
AUEffectBase::~AUEffectBase()
{
	Cleanup();
}

//_____________________________________________________________________________
//
void AUEffectBase::Cleanup()
{
	for (KernelList::iterator it = mKernelList.begin(); it != mKernelList.end(); ++it)
		delete *it;
		
	mKernelList.clear();
	mMainOutput = NULL;
	mMainInput = NULL;
}


//_____________________________________________________________________________
//
OSStatus AUEffectBase::Initialize()
{
		// get our current numChannels for input and output
	SInt16 auNumInputs = (SInt16) GetInput(0)->GetStreamFormat().mChannelsPerFrame;
	SInt16 auNumOutputs = (SInt16) GetOutput(0)->GetStreamFormat().mChannelsPerFrame;

		// does the unit publish specific information about channel configurations?
    const AUChannelInfo *auChannelConfigs = NULL;
    UInt32 numIOconfigs = SupportedNumChannels(&auChannelConfigs);

    if ((numIOconfigs > 0) && (auChannelConfigs != NULL))
    {
        bool foundMatch = false;
        for (UInt32 i = 0; (i < numIOconfigs) && !foundMatch; ++i)
        {
            SInt16 configNumInputs = auChannelConfigs[i].inChannels;
            SInt16 configNumOutputs = auChannelConfigs[i].outChannels;
            if ((configNumInputs < 0) && (configNumOutputs < 0))
            {
					// unit accepts any number of channels on input and output
                if (((configNumInputs == -1) && (configNumOutputs == -2)) 
					|| ((configNumInputs == -2) && (configNumOutputs == -1)))
                {
				    foundMatch = true;
					// unit accepts any number of channels on input and output IFF they are the same number on both scopes
                }
				else if (((configNumInputs == -1) && (configNumOutputs == -1)) && (auNumInputs == auNumOutputs))
                {
				    foundMatch = true;
					// unit has specified a particular number of channels on both scopes
                }
				else
                    continue;
            }
            else
            {
					// the -1 case on either scope is saying that the unit doesn't care about the 
					// number of channels on that scope
                bool inputMatch = (auNumInputs == configNumInputs) || (configNumInputs == -1);
                bool outputMatch = (auNumOutputs == configNumOutputs) || (configNumOutputs == -1);
                if (inputMatch && outputMatch)
                    foundMatch = true;
            }
        }
        if (!foundMatch)
            return kAudioUnitErr_FormatNotSupported;
    }
    else
    {
			// there is no specifically published channel info
			// so for those kinds of effects, the assumption is that the channels (whatever their number)
			// should match on both scopes
        if ((auNumOutputs != auNumInputs) || (auNumOutputs == 0))
		{
		    return kAudioUnitErr_FormatNotSupported;
		}
    }

    MaintainKernels();
	
	mMainOutput = GetOutput(0);
	mMainInput = GetInput(0);
	
	const CAStreamBasicDescription& format = GetStreamFormat(kAudioUnitScope_Output, 0);
	format.IdentifyCommonPCMFormat(mCommonPCMFormat, NULL);
	mBytesPerFrame = format.mBytesPerFrame;
	
    return noErr;
}

OSStatus			AUEffectBase::Reset(		AudioUnitScope 		inScope,
								 				AudioUnitElement 	inElement)
{
	for (KernelList::iterator it = mKernelList.begin(); it != mKernelList.end(); ++it) {
		AUKernelBase *kernel = *it;
		if (kernel != NULL)
			kernel->Reset();
	}
	
	return AUBase::Reset(inScope, inElement);
}

OSStatus			AUEffectBase::GetPropertyInfo (AudioUnitPropertyID	inID,
												AudioUnitScope					inScope,
												AudioUnitElement				inElement,
												UInt32 &						outDataSize,
												Boolean &						outWritable)
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
			case kAudioUnitProperty_BypassEffect:
				outWritable = true;
				outDataSize = sizeof (UInt32);
				return noErr;
			case kAudioUnitProperty_InPlaceProcessing:
				outWritable = true;
				outDataSize = sizeof (UInt32);
				return noErr;
		}
	}
	return AUBase::GetPropertyInfo (inID, inScope, inElement, outDataSize, outWritable);
}


OSStatus			AUEffectBase::GetProperty (AudioUnitPropertyID 		inID,
									  AudioUnitScope 					inScope,
									  AudioUnitElement			 		inElement,
									  void *							outData)
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
			case kAudioUnitProperty_BypassEffect:
				*((UInt32*)outData) = (IsBypassEffect() ? 1 : 0);
				return noErr;
			case kAudioUnitProperty_InPlaceProcessing:
				*((UInt32*)outData) = (mProcessesInPlace ? 1 : 0);
				return noErr;
		}
	}
	return AUBase::GetProperty (inID, inScope, inElement, outData);
}


OSStatus			AUEffectBase::SetProperty(		AudioUnitPropertyID inID,
									   AudioUnitScope 		inScope,
									   AudioUnitElement 	inElement,
									   const void *			inData,
									   UInt32 				inDataSize)
{
	if (inScope == kAudioUnitScope_Global) {
		switch (inID) {
			case kAudioUnitProperty_BypassEffect:
			{
				if (inDataSize < sizeof(UInt32))
					return kAudioUnitErr_InvalidPropertyValue;
					
				bool tempNewSetting = *((UInt32*)inData) != 0;
					// we're changing the state of bypass
				if (tempNewSetting != IsBypassEffect()) 
				{
					if (!tempNewSetting && IsBypassEffect() && IsInitialized()) // turning bypass off and we're initialized
						Reset(0, 0);
					SetBypassEffect (tempNewSetting);
				}
				return noErr;
			}
			case kAudioUnitProperty_InPlaceProcessing:
				mProcessesInPlace = (*((UInt32*)inData) != 0);
				return noErr;
		}
	}
	return AUBase::SetProperty (inID, inScope, inElement, inData, inDataSize);
}
 

void	AUEffectBase::MaintainKernels()
{
#if TARGET_OS_IPHONE
	UInt32 nKernels = mOnlyOneKernel ? 1 : GetNumberOfChannels();
#else 
	UInt32 nKernels = GetNumberOfChannels();
#endif
	
	if (mKernelList.size() < nKernels) {
		mKernelList.reserve(nKernels);
		for (UInt32 i = (UInt32)mKernelList.size(); i < nKernels; ++i)
			mKernelList.push_back(NewKernel());
	} else {
		while (mKernelList.size() > nKernels) {
			AUKernelBase *kernel = mKernelList.back();
			delete kernel;
			mKernelList.pop_back();
		}
	}
	
	for(unsigned int i = 0; i < nKernels; i++ )
	{
		if(mKernelList[i]) {
			mKernelList[i]->SetChannelNum (i);
		}
	}
}

bool		AUEffectBase::StreamFormatWritable(	AudioUnitScope					scope,
												AudioUnitElement				element)
{
	return IsInitialized() ? false : true;
}

OSStatus			AUEffectBase::ChangeStreamFormat(	AudioUnitScope				inScope,
														AudioUnitElement			inElement,
														const CAStreamBasicDescription & inPrevFormat,
														const CAStreamBasicDescription & inNewFormat)
{
	OSStatus result = AUBase::ChangeStreamFormat(inScope, inElement, inPrevFormat, inNewFormat);
	if (result == noErr)
	{	
		// for the moment this only dependency we know about
		// where a parameter's range may change is with the sample rate
		// and effects are only publishing parameters in the global scope!
		if (GetParamHasSampleRateDependency() && fnotequal(inPrevFormat.mSampleRate, inNewFormat.mSampleRate))
			PropertyChanged(kAudioUnitProperty_ParameterList, kAudioUnitScope_Global, 0);
	}

	return result;
}


// ____________________________________________________________________________
//
//	This method is called (potentially repeatedly) by ProcessForScheduledParams()
//	in order to perform the actual DSP required for this portion of the entire buffer
//	being processed.  The entire buffer can be divided up into smaller "slices"
//	according to the timestamps on the scheduled parameters...
//
OSStatus		AUEffectBase::ProcessScheduledSlice(	void				*inUserData,
														UInt32				inStartFrameInBuffer,
														UInt32				inSliceFramesToProcess,
														UInt32				inTotalBufferFrames )
{
	ScheduledProcessParams	&sliceParams = *((ScheduledProcessParams*)inUserData);
	
	AudioUnitRenderActionFlags 	&actionFlags = *sliceParams.actionFlags;
	AudioBufferList 			&inputBufferList = *sliceParams.inputBufferList;
	AudioBufferList 			&outputBufferList = *sliceParams.outputBufferList;
	
	UInt32 channelSize = inSliceFramesToProcess * mBytesPerFrame;
		// fix the size of the buffer we're operating on before we render this slice of time
	for(unsigned int i = 0; i < inputBufferList.mNumberBuffers; i++ ) {
		inputBufferList.mBuffers[i].mDataByteSize = inputBufferList.mBuffers[i].mNumberChannels * channelSize;
	}

	for(unsigned int i = 0; i < outputBufferList.mNumberBuffers; i++ ) {
		outputBufferList.mBuffers[i].mDataByteSize = outputBufferList.mBuffers[i].mNumberChannels * channelSize;
	}
		// process the buffer
	OSStatus result = ProcessBufferLists(actionFlags, inputBufferList, outputBufferList, inSliceFramesToProcess );

		// we just partially processed the buffers, so increment the data pointers to the next part of the buffer to process
	for(unsigned int i = 0; i < inputBufferList.mNumberBuffers; i++ ) {
		inputBufferList.mBuffers[i].mData = 
			(char *)inputBufferList.mBuffers[i].mData + inputBufferList.mBuffers[i].mNumberChannels * channelSize;
	}
	
	for(unsigned int i = 0; i < outputBufferList.mNumberBuffers; i++ ) {
		outputBufferList.mBuffers[i].mData = 
			(char *)outputBufferList.mBuffers[i].mData + outputBufferList.mBuffers[i].mNumberChannels * channelSize;
	}
	
	return result;
}

// ____________________________________________________________________________
//

OSStatus 	AUEffectBase::Render(	AudioUnitRenderActionFlags &ioActionFlags,
											const AudioTimeStamp &		inTimeStamp,
											UInt32						nFrames)
{
	if (!HasInput(0))
		return kAudioUnitErr_NoConnection;

	OSStatus result = noErr;

	result = mMainInput->PullInput(ioActionFlags, inTimeStamp, 0 /* element */, nFrames);
	
	if (result == noErr)
	{
		if(ProcessesInPlace() && mMainOutput->WillAllocateBuffer())
		{
			mMainOutput->SetBufferList(mMainInput->GetBufferList() );
		}

		if (ShouldBypassEffect())
		{
			// leave silence bit alone
			
			if(!ProcessesInPlace() )
			{
				mMainInput->CopyBufferContentsTo (mMainOutput->GetBufferList());
			}
		}
		else
		{
			if(mParamList.size() == 0 )
			{
				// this will read/write silence bit
				result = ProcessBufferLists(ioActionFlags, mMainInput->GetBufferList(), mMainOutput->GetBufferList(), nFrames);
			}
			else
			{
				// deal with scheduled parameters...
				
				AudioBufferList &inputBufferList = mMainInput->GetBufferList();
				AudioBufferList &outputBufferList = mMainOutput->GetBufferList();
				
				ScheduledProcessParams processParams;
				processParams.actionFlags = &ioActionFlags;
				processParams.inputBufferList = &inputBufferList;
				processParams.outputBufferList = &outputBufferList;
	
				// divide up the buffer into slices according to scheduled params then
				// do the DSP for each slice (ProcessScheduledSlice() called for each slice)
				result = ProcessForScheduledParams(	mParamList,
													nFrames,
													&processParams );
	
				
				// fixup the buffer pointers to how they were before we started
				UInt32 channelSize = nFrames * mBytesPerFrame;
				for(unsigned int i = 0; i < inputBufferList.mNumberBuffers; i++ ) {
					UInt32 size = inputBufferList.mBuffers[i].mNumberChannels * channelSize;
					inputBufferList.mBuffers[i].mData = (char *)inputBufferList.mBuffers[i].mData - size;
					inputBufferList.mBuffers[i].mDataByteSize = size;
				}
				
				for(unsigned int i = 0; i < outputBufferList.mNumberBuffers; i++ ) {
					UInt32 size = outputBufferList.mBuffers[i].mNumberChannels * channelSize;
					outputBufferList.mBuffers[i].mData = (char *)outputBufferList.mBuffers[i].mData - size;
					outputBufferList.mBuffers[i].mDataByteSize = size;
				}
			}
		}
	
		if ( (ioActionFlags & kAudioUnitRenderAction_OutputIsSilence) && !ProcessesInPlace() )
		{
			AUBufferList::ZeroBuffer(mMainOutput->GetBufferList() );
		}
	}
	
	return result;
}


OSStatus	AUEffectBase::ProcessBufferLists(
									AudioUnitRenderActionFlags &	ioActionFlags,
									const AudioBufferList &			inBuffer,
									AudioBufferList &				outBuffer,
									UInt32							inFramesToProcess )
{
	if (ShouldBypassEffect())
		return noErr;
		
	// interleaved (or mono)
	switch (mCommonPCMFormat) {
		case CAStreamBasicDescription::kPCMFormatFloat32 :
			ProcessBufferListsT<Float32>(ioActionFlags, inBuffer, outBuffer, inFramesToProcess);
			break;
		case CAStreamBasicDescription::kPCMFormatFixed824 :
			ProcessBufferListsT<SInt32>(ioActionFlags, inBuffer, outBuffer, inFramesToProcess);
			break;
		case CAStreamBasicDescription::kPCMFormatInt16 :
			ProcessBufferListsT<SInt16>(ioActionFlags, inBuffer, outBuffer, inFramesToProcess);
			break;
		default :
			throw CAException(kAudio_UnimplementedError);
	}
	
	return noErr;
}

Float64		AUEffectBase::GetSampleRate()
{
	return GetOutput(0)->GetStreamFormat().mSampleRate;
}

UInt32		AUEffectBase::GetNumberOfChannels()
{
	return GetOutput(0)->GetStreamFormat().mChannelsPerFrame;
}


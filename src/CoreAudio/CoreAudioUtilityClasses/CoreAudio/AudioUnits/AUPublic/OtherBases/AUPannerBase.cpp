/*
     File: AUPannerBase.cpp
 Abstract: AUPannerBase.h
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
#include "AUPannerBase.h"
#include "CABundleLocker.h"
#include <AudioToolbox/AudioToolbox.h>
#include <Accelerate/Accelerate.h>

static bool sLocalized = false;

static CFStringRef kPanner_Azimuth_Name = CFSTR("azimuth"); 	
static CFStringRef kPanner_Elevation_Name = CFSTR("elevation"); 	
static CFStringRef kPanner_Distance_Name = CFSTR("distance");

static CFStringRef kPanner_CoordScale_Name = CFSTR("coordinate scale"); 	
static CFStringRef kPanner_RefDistance_Name = CFSTR("reference distance"); 	
static CFStringRef kPanner_Gain_Name = CFSTR("gain"); 	

static Float32 kPannerParamDefault_Azimuth = 0.;
static Float32 kPannerParamDefault_Elevation = 0.;
static Float32 kPannerParamDefault_Distance = 1.;

static Float32 kPannerParamDefault_CoordScale = 10.;
static Float32 kPannerParamDefault_RefDistance = 1.;
static Float32 kPannerParamDefault_Gain = 1.;

//_____________________________________________________________________________
//
AUPannerBase::AUPannerBase(AudioComponentInstance inAudioUnit)
    : AUBase(inAudioUnit, 1, 1), mBypassEffect(false)
{
	{
		CABundleLocker lock;
		if (!sLocalized)
		{		
			CFBundleRef bundle = CFBundleGetBundleWithIdentifier( CFSTR("com.apple.audio.units.Components") );
			if (bundle != NULL)
			{
				kPanner_Azimuth_Name 	= CFCopyLocalizedStringFromTableInBundle(kPanner_Azimuth_Name,    CFSTR("AudioUnits"), bundle, CFSTR(""));
				kPanner_Elevation_Name 	= CFCopyLocalizedStringFromTableInBundle(kPanner_Elevation_Name,  CFSTR("AudioUnits"), bundle, CFSTR(""));
				kPanner_Distance_Name 	= CFCopyLocalizedStringFromTableInBundle(kPanner_Distance_Name,   CFSTR("AudioUnits"), bundle, CFSTR(""));
				
				kPanner_CoordScale_Name  = CFCopyLocalizedStringFromTableInBundle(kPanner_CoordScale_Name,  CFSTR("AudioUnits"), bundle, CFSTR(""));
				kPanner_RefDistance_Name = CFCopyLocalizedStringFromTableInBundle(kPanner_RefDistance_Name, CFSTR("AudioUnits"), bundle, CFSTR(""));
				kPanner_Gain_Name 	     = CFCopyLocalizedStringFromTableInBundle(kPanner_Gain_Name,        CFSTR("AudioUnits"), bundle, CFSTR(""));

			}
			
			sLocalized = true;
		}
	}
	
	CreateElements();
	
    SetParameter(kPannerParam_Azimuth, kPannerParamDefault_Azimuth);
    SetParameter(kPannerParam_Elevation, kPannerParamDefault_Elevation);
    SetParameter(kPannerParam_Distance, kPannerParamDefault_Distance);
    
    SetParameter(kPannerParam_CoordScale, kPannerParamDefault_CoordScale);
    SetParameter(kPannerParam_RefDistance, kPannerParamDefault_RefDistance);
    SetParameter(kPannerParam_Gain, kPannerParamDefault_Gain);
}

//_____________________________________________________________________________
//
AUPannerBase::~AUPannerBase()
{
    Cleanup();
}

//_____________________________________________________________________________
//
/*! @method Initialize */
OSStatus			AUPannerBase::Initialize()
{
	OSStatus err = noErr;
	AllocBypassMatrix();
	err = UpdateBypassMatrix();
	return err;
}

//_____________________________________________________________________________
//
/*! @method AllocBypassMatrix */
void		AUPannerBase::AllocBypassMatrix()
{
	UInt32 inChannels = GetNumberOfInputChannels();
	UInt32 outChannels = GetNumberOfOutputChannels();
	mBypassMatrix.alloc(inChannels * outChannels, true);
}

static AudioChannelLayoutTag DefaultTagForNumberOfChannels(UInt32 inNumberChannels)
{
	switch (inNumberChannels) {
		case 1: return kAudioChannelLayoutTag_Mono; 
		case 2: return kAudioChannelLayoutTag_Stereo; 
		case 4: return kAudioChannelLayoutTag_Quadraphonic; 
		case 5: return kAudioChannelLayoutTag_AudioUnit_5_0; 
		case 6: return kAudioChannelLayoutTag_AudioUnit_6_0; 
		case 7: return kAudioChannelLayoutTag_AudioUnit_7_0; 
		case 8: return kAudioChannelLayoutTag_AudioUnit_8; 

		default: return 0xFFFF0000 | inNumberChannels;
	}
}

//_____________________________________________________________________________
//
/*! @method UpdateBypassMatrix */
OSStatus			AUPannerBase::SetDefaultChannelLayoutsIfNone()
{
	OSStatus err = noErr;
	
	// if layout has not been set, then guess layout from number of channels
	UInt32 inChannels = GetNumberOfInputChannels();
	AudioChannelLayout inputLayoutSubstitute;
	const AudioChannelLayout* inputLayout = &GetInputLayout();
	if (inputLayout == NULL || inputLayout->mChannelLayoutTag == 0) {
		inputLayout = &inputLayoutSubstitute;
		inputLayoutSubstitute.mNumberChannelDescriptions = 0;
		inputLayoutSubstitute.mChannelBitmap = 0;
		inputLayoutSubstitute.mChannelLayoutTag = DefaultTagForNumberOfChannels(inChannels);

		mInputLayout = &inputLayoutSubstitute;
		err = SetAudioChannelLayout(kAudioUnitScope_Input, 0, &GetInputLayout());
		if (err) return err;
	}
		
	// if layout has not been set, then guess layout from number of channels
	UInt32 outChannels = GetNumberOfOutputChannels();
	AudioChannelLayout outputLayoutSubstitute;
	const AudioChannelLayout* outputLayout = &GetOutputLayout();
	if (outputLayout == NULL || outputLayout->mChannelLayoutTag == 0) {
		outputLayout = &outputLayoutSubstitute;
		outputLayoutSubstitute.mNumberChannelDescriptions = 0;
		outputLayoutSubstitute.mChannelBitmap = 0;
		outputLayoutSubstitute.mChannelLayoutTag = DefaultTagForNumberOfChannels(outChannels);

		mOutputLayout = &outputLayoutSubstitute;
		err = SetAudioChannelLayout(kAudioUnitScope_Output, 0, &GetOutputLayout());
		if (err) return err;
	}
	
	return err;
}


	
OSStatus			AUPannerBase::UpdateBypassMatrix()
{
	OSStatus err = SetDefaultChannelLayoutsIfNone();
	if (err) return err;
	
	UInt32 inChannels = GetNumberOfInputChannels();
	UInt32 outChannels = GetNumberOfOutputChannels();	
	
	const AudioChannelLayout* inoutACL[2];
	
	inoutACL[0] = &GetInputLayout();
	inoutACL[1] = &GetOutputLayout();

	mBypassMatrix.alloc(inChannels * outChannels, true);
	
	UInt32 propSize = inChannels * outChannels * sizeof(Float32);
	
	err = AudioFormatGetProperty(kAudioFormatProperty_MatrixMixMap, sizeof(inoutACL), inoutACL, &propSize, mBypassMatrix());
	if (err) {
		// if there is an error, use a diagonal matrix.
		Float32* bypass = mBypassMatrix();
		for (UInt32 chan = 0; chan < std::min(inChannels, outChannels); ++chan)
		{
			float *amp = bypass + (chan * outChannels + chan);
			*amp = 1.;
		}
	}

    return noErr;
}

//_____________________________________________________________________________
//
/*! @method Cleanup */
void				AUPannerBase::Cleanup()
{
    
}


//_____________________________________________________________________________
//
/*! @method Reset */
OSStatus			AUPannerBase::Reset(		AudioUnitScope 				inScope,
										AudioUnitElement 			inElement)
{
    return AUBase::Reset(inScope, inElement);
}

//_____________________________________________________________________________
//
/*! @method GetParameterInfo */
OSStatus			AUPannerBase::GetParameterInfo(	AudioUnitScope			inScope,
												AudioUnitParameterID	inParameterID,
												AudioUnitParameterInfo	&outParameterInfo )
{
	OSStatus result = noErr;

	outParameterInfo.flags = 	kAudioUnitParameterFlag_IsWritable
						+		kAudioUnitParameterFlag_IsReadable;
		
	if (inScope == kAudioUnitScope_Global) {
		
		switch(inParameterID)
		{
			case kPannerParam_Azimuth:
				AUBase::FillInParameterName (outParameterInfo, kPanner_Azimuth_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Degrees;
				outParameterInfo.minValue = -180.;
				outParameterInfo.maxValue = 180;
				outParameterInfo.defaultValue = kPannerParamDefault_Azimuth;
			break;

			case kPannerParam_Elevation:
				AUBase::FillInParameterName (outParameterInfo, kPanner_Elevation_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Degrees;
				outParameterInfo.minValue = -90.;
				outParameterInfo.maxValue = 90;
				outParameterInfo.defaultValue = kPannerParamDefault_Elevation;
			break;

			case kPannerParam_Distance:
				AUBase::FillInParameterName (outParameterInfo, kPanner_Distance_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue = 0.0;
				outParameterInfo.maxValue = 1.;
				outParameterInfo.defaultValue = kPannerParamDefault_Distance;
				outParameterInfo.flags += kAudioUnitParameterFlag_IsHighResolution;
				//outParameterInfo.flags += kAudioUnitParameterFlag_DisplayLogarithmic;
			break;
 
 			case kPannerParam_CoordScale:
				AUBase::FillInParameterName (outParameterInfo, kPanner_CoordScale_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Meters;
				outParameterInfo.minValue = 0.01;
				outParameterInfo.maxValue = 1000.;
				outParameterInfo.defaultValue = kPannerParamDefault_CoordScale;
				outParameterInfo.flags += kAudioUnitParameterFlag_DisplayLogarithmic;
			break;

 			case kPannerParam_RefDistance:
				AUBase::FillInParameterName (outParameterInfo, kPanner_RefDistance_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Meters;
				outParameterInfo.minValue = 0.01;
				outParameterInfo.maxValue = 1000.;
				outParameterInfo.defaultValue = kPannerParamDefault_RefDistance;
				outParameterInfo.flags += kAudioUnitParameterFlag_DisplayLogarithmic;
			break;

			case kPannerParam_Gain:
				AUBase::FillInParameterName (outParameterInfo, kPanner_Gain_Name, false);
				outParameterInfo.unit = kAudioUnitParameterUnit_Generic;
				outParameterInfo.minValue = 0.;
				outParameterInfo.maxValue = 1.;
				outParameterInfo.defaultValue = kPannerParamDefault_Gain;
			break;

			default:
				result = kAudioUnitErr_InvalidParameter;
				break;
		}
	} else {
		result = kAudioUnitErr_InvalidParameter;
	}
	
	return result;
}												

//_____________________________________________________________________________
//
OSStatus 	AUPannerBase::GetParameter(		AudioUnitParameterID			inParamID,
													AudioUnitScope 					inScope,
													AudioUnitElement 				inElement,
													Float32 &						outValue)
{
	if (inScope != kAudioUnitScope_Global) 
		return kAudioUnitErr_InvalidScope;
		
	outValue = Globals()->GetParameter(inParamID);

	return noErr;
}

											
//_____________________________________________________________________________
//
OSStatus 	AUPannerBase::SetParameter(		AudioUnitParameterID			inParamID,
													AudioUnitScope 					inScope,
													AudioUnitElement 				inElement,
													Float32							inValue,
													UInt32							inBufferOffsetInFrames)
{
	if (inScope != kAudioUnitScope_Global) 
		return kAudioUnitErr_InvalidScope;

	Globals()->SetParameter(inParamID, inValue);
	
	return noErr;
}



//_____________________________________________________________________________
//
/*! @method GetPropertyInfo */
OSStatus			AUPannerBase::GetPropertyInfo (AudioUnitPropertyID	inID,
										AudioUnitScope				inScope,
										AudioUnitElement			inElement,
										UInt32 &					outDataSize,
										Boolean &					outWritable)
{
	OSStatus err = noErr;
	switch (inID) {
		case kAudioUnitProperty_BypassEffect:
			if (inScope != kAudioUnitScope_Global)
				return kAudioUnitErr_InvalidScope;
				
			outWritable = true;
			outDataSize = sizeof (UInt32);
			break;
		default:
			err = AUBase::GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
	}
	return err;
}

//_____________________________________________________________________________
//
/*! @method GetProperty */
OSStatus			AUPannerBase::GetProperty (AudioUnitPropertyID 		inID,
										AudioUnitScope 				inScope,
										AudioUnitElement	 		inElement,
										void *						outData)
{
	OSStatus err = noErr;
	switch (inID) 
	{
		case kAudioUnitProperty_BypassEffect:
			if (inScope != kAudioUnitScope_Global)
				return kAudioUnitErr_InvalidScope;

			*((UInt32*)outData) = (IsBypassEffect() ? 1 : 0);
			break;
		default:
			err = AUBase::GetProperty(inID, inScope, inElement, outData);
	}
	return err;
}

//_____________________________________________________________________________
//
/*! @method SetProperty */
OSStatus			AUPannerBase::SetProperty(AudioUnitPropertyID 		inID,
										AudioUnitScope 				inScope,
										AudioUnitElement 			inElement,
										const void *				inData,
										UInt32 						inDataSize)
{
	switch (inID) 
	{
		case kAudioUnitProperty_BypassEffect:
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
    return AUBase::SetProperty(inID, inScope, inElement, inData, inDataSize);
}


//_____________________________________________________________________________
//
/*! @method StreamFormatWritable */
bool				AUPannerBase::StreamFormatWritable (AudioUnitScope	scope,
										AudioUnitElement			element)
{
    return true;
}

//_____________________________________________________________________________
//
/*! @method ChangeStreamFormat */
OSStatus			AUPannerBase::ChangeStreamFormat (
									AudioUnitScope						inScope,
									AudioUnitElement					inElement,
									const CAStreamBasicDescription & 	inPrevFormat,
									const CAStreamBasicDescription &	inNewFormat)
{
	if (inScope == kAudioUnitScope_Input && !InputChannelConfigIsSupported(inNewFormat.NumberChannels())) 
		return kAudioUnitErr_FormatNotSupported;
		
	if (inScope == kAudioUnitScope_Output && !OutputChannelConfigIsSupported(inNewFormat.NumberChannels())) 
		return kAudioUnitErr_FormatNotSupported;
		
	if (inNewFormat.NumberChannels() != inPrevFormat.NumberChannels())
		RemoveAudioChannelLayout(inScope, inElement);
		
	return AUBase::ChangeStreamFormat(inScope, inElement, inPrevFormat, inNewFormat);
}

//_____________________________________________________________________________
//
/*! @method Render */
OSStatus 	AUPannerBase::Render(AudioUnitRenderActionFlags &		ioActionFlags,
									const AudioTimeStamp &			inTimeStamp,
									UInt32							inNumberFrames)
{
	if (IsBypassEffect()) 
		return BypassRender(ioActionFlags, inTimeStamp, inNumberFrames);
	else 
		return PannerRender(ioActionFlags, inTimeStamp, inNumberFrames);
}

//_____________________________________________________________________________
//
/*! @method Render */
OSStatus 	AUPannerBase::BypassRender(AudioUnitRenderActionFlags &		ioActionFlags,
									const AudioTimeStamp &			inTimeStamp,
									UInt32							inNumberFrames)
{
	AudioUnitRenderActionFlags xflags = 0;
	OSStatus result = PullInput(0, xflags, inTimeStamp, inNumberFrames);
	if (result) return false;
	bool isSilent = xflags & kAudioUnitRenderAction_OutputIsSilence;

	AudioBufferList& outputBufferList = GetOutput(0)->GetBufferList();
	AUBufferList::ZeroBuffer(outputBufferList);
	
	if (!isSilent) 
	{
		UInt32 inChannels = GetNumberOfInputChannels();
		UInt32 outChannels = GetNumberOfOutputChannels();
		Float32* bypass = mBypassMatrix();
		for (UInt32 outChan = 0; outChan < outChannels; ++outChan)
		{
			float* outData = GetOutput(0)->GetChannelData(outChan);	
			
			for (UInt32 inChan = 0; inChan < inChannels; ++inChan)
			{
				float* inData = GetInput(0)->GetChannelData(inChan);
				float *amp = bypass + (inChan * outChannels + outChan);
				vDSP_vsma(inData, 1, amp, outData, 1, outData, 1, inNumberFrames);
			}
		}
	}
    return noErr;
}

//_____________________________________________________________________________
//
UInt32		AUPannerBase::GetAudioChannelLayout(	AudioUnitScope				inScope,
													AudioUnitElement 			inElement,
													AudioChannelLayout *		outLayoutPtr,
													Boolean &					outWritable)
{	
	SetDefaultChannelLayoutsIfNone();
	
	outWritable = true;
	
	CAAudioChannelLayout* caacl = NULL;
	switch (inScope) 
	{
		case kAudioUnitScope_Input:
			caacl = &mInputLayout;
			break;
		case kAudioUnitScope_Output:
			caacl = &mOutputLayout;
			break;
		default:
			COMPONENT_THROW(kAudioUnitErr_InvalidScope);
	}
	
	if (inElement != 0)
		COMPONENT_THROW(kAudioUnitErr_InvalidElement);
		
	UInt32 size = caacl->IsValid() ? caacl->Size() : 0;
	if (size > 0 && outLayoutPtr)
		memcpy(outLayoutPtr, &caacl->Layout(), size);
		
	return size;
}

//_____________________________________________________________________________
//
OSStatus	AUPannerBase::RemoveAudioChannelLayout(		AudioUnitScope				inScope,
														AudioUnitElement			inElement)
{
	CAAudioChannelLayout* caacl = NULL;
	switch (inScope) 
	{
		case kAudioUnitScope_Input:
			caacl = &mInputLayout;
			break;
		case kAudioUnitScope_Output:
			caacl = &mOutputLayout;
			break;
		default:
			return kAudioUnitErr_InvalidScope;
	}
	
	if (inElement != 0)
		return kAudioUnitErr_InvalidElement;
	
	*caacl = NULL;
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus 	AUPannerBase::SetAudioChannelLayout(		AudioUnitScope 				inScope, 
														AudioUnitElement 			inElement,
														const AudioChannelLayout *	inLayout)
{
	if (!inLayout)
		return RemoveAudioChannelLayout(inScope, inElement);

	if (!ChannelLayoutTagIsSupported(inScope, inElement, inLayout->mChannelLayoutTag))
		return kAudioUnitErr_FormatNotSupported;
	
	CAAudioChannelLayout* caacl = NULL;
	UInt32 currentChannels;
	switch (inScope) 
	{
		case kAudioUnitScope_Input:
			caacl = &mInputLayout;
			currentChannels = GetNumberOfInputChannels();
			break;
		case kAudioUnitScope_Output:
			caacl = &mOutputLayout;
			currentChannels = GetNumberOfOutputChannels();
			break;
		default:
			return kAudioUnitErr_InvalidScope;
	}
	
	if (inElement != 0)
		return kAudioUnitErr_InvalidElement;

	UInt32 numChannelsInLayout = CAAudioChannelLayout::NumberChannels(*inLayout);
	if (currentChannels != numChannelsInLayout)
		return kAudioUnitErr_InvalidPropertyValue;
	
	*caacl = inLayout;
	if (IsInitialized())
		UpdateBypassMatrix();
		
	return noErr;
}

//_____________________________________________________________________________
//
UInt32 AUPannerBase::GetChannelLayoutTags(		AudioUnitScope				inScope,
												AudioUnitElement 			inElement, 
												AudioChannelLayoutTag*		outTags)
{
	switch (inScope) 
	{
		case kAudioUnitScope_Input:
			if (outTags) {
				outTags[0] = kAudioChannelLayoutTag_Mono;
				outTags[1] = kAudioChannelLayoutTag_Stereo;
				outTags[2] = kAudioChannelLayoutTag_Ambisonic_B_Format;
			}
			return 3;
		case kAudioUnitScope_Output:
			if (outTags) {
				outTags[0] = kAudioChannelLayoutTag_Stereo;
				outTags[1] = kAudioChannelLayoutTag_Quadraphonic;
				outTags[2] = kAudioChannelLayoutTag_AudioUnit_5_0;
				outTags[3] = kAudioChannelLayoutTag_AudioUnit_6_0;
				outTags[4] = kAudioChannelLayoutTag_AudioUnit_7_0;
				outTags[5] = kAudioChannelLayoutTag_AudioUnit_7_0_Front;
				outTags[6] = kAudioChannelLayoutTag_AudioUnit_8;
			}
			return 7;
		default: {
			OSStatus err = kAudioUnitErr_InvalidScope;
			throw err;
		}
	}
}



//_____________________________________________________________________________
//
bool 	AUPannerBase::ChannelConfigIsSupported()
{
	UInt32 inChannels = GetNumberOfInputChannels();
	UInt32 outChannels = GetNumberOfOutputChannels();
	const AUChannelInfo* cinfo = NULL;
	UInt32 numConfigs = SupportedNumChannels(&cinfo);
	for (UInt32 i = 0; i < numConfigs; ++i)
	{
		if (cinfo[i].inChannels == (SInt16)inChannels && cinfo[i].outChannels == (SInt16)outChannels)
			return true;
	}
	return false;
}


//_____________________________________________________________________________
//
bool 	AUPannerBase::InputChannelConfigIsSupported(UInt32 inNumberChannels)
{
	const AUChannelInfo* cinfo = NULL;
	UInt32 numConfigs = SupportedNumChannels(&cinfo);
	for (UInt32 i = 0; i < numConfigs; ++i)
	{
		if (cinfo[i].inChannels == (SInt16)inNumberChannels)
			return true;
	}
	return false;
}

//_____________________________________________________________________________
//
bool 	AUPannerBase::OutputChannelConfigIsSupported(UInt32 inNumberChannels)
{
	const AUChannelInfo* cinfo = NULL;
	UInt32 numConfigs = SupportedNumChannels(&cinfo);
	for (UInt32 i = 0; i < numConfigs; ++i)
	{
		if (cinfo[i].outChannels == (SInt16)inNumberChannels)
			return true;
	}
	return false;
}

//_____________________________________________________________________________
//
bool 	AUPannerBase::ChannelLayoutTagIsSupported(		AudioUnitScope			inScope, 
														AudioUnitElement		inElement, 
														AudioChannelLayoutTag	inTag)
{
	UInt32 numTags = GetChannelLayoutTags(inScope, inElement, NULL);
	CAAutoFree<AudioChannelLayoutTag> tags(numTags);
	GetChannelLayoutTags(inScope, inElement, tags());
	
	for (UInt32 i = 0; i < numTags; ++i)
	{
		if (tags[i] == inTag)
			return true;
	}
	
	return false;
}


/*
     File: AUBase.cpp
 Abstract: AUBase.h
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
#include "AUBase.h"
#include "AUDispatch.h"
#include "AUInputElement.h"
#include "AUOutputElement.h"
#include <algorithm>
#include <syslog.h>
#include "CAAudioChannelLayout.h"
#include "CAHostTimeBase.h"
#include "CAVectorUnit.h"
#include "CAXException.h"



#if TARGET_OS_MAC && (TARGET_CPU_X86 || TARGET_CPU_X86_64)
	// our compiler does ALL floating point with SSE
	inline int  GETCSR ()    { int _result; asm volatile ("stmxcsr %0" : "=m" (*&_result) ); return _result; }
	inline void SETCSR (int a)    { int _temp = a; asm volatile( "ldmxcsr %0" : : "m" (*&_temp ) ); }

	#define DISABLE_DENORMALS int _savemxcsr = GETCSR(); SETCSR(_savemxcsr | 0x8040);
	#define RESTORE_DENORMALS SETCSR(_savemxcsr);
#else
	#define DISABLE_DENORMALS
	#define RESTORE_DENORMALS
#endif

static bool sAUBaseCFStringsInitialized = false;
// this is used for the presets
static CFStringRef kUntitledString = NULL;
//these are the current keys for the class info document
static CFStringRef kVersionString = NULL;
static CFStringRef kTypeString = NULL;
static CFStringRef kSubtypeString = NULL;
static CFStringRef kManufacturerString = NULL;
static CFStringRef kDataString = NULL;
static CFStringRef kNameString = NULL;
static CFStringRef kRenderQualityString = NULL;
static CFStringRef kCPULoadString = NULL;
static CFStringRef kElementNameString = NULL;
static CFStringRef kPartString = NULL;

SInt32 AUBase::sVectorUnitType = kVecUninitialized;

//_____________________________________________________________________________
//
AUBase::AUBase(	AudioComponentInstance			inInstance, 
				UInt32							numInputElements,
				UInt32							numOutputElements,
				UInt32							numGroupElements) :
	ComponentBase(inInstance),
	mElementsCreated(false),
	mInitialized(false),
	mHasBegunInitializing(false),
	mInitNumInputEls(numInputElements), mInitNumOutputEls(numOutputElements), 
#if !CA_BASIC_AU_FEATURES
	mInitNumGroupEls(numGroupElements),
#endif
	mRenderCallbacksTouched(false),
	mRenderThreadID (NULL),
	mWantsRenderThreadID (false),
	mLastRenderError(0),
	mUsesFixedBlockSize(false),
	mBuffersAllocated(false),
	mLogString (NULL),
    mNickName (NULL),
	mAUMutex(NULL)
	#if !CA_NO_AU_UI_FEATURES
		,
		mContextName(NULL)
	#endif
{
	ResetRenderTime ();
	
	if(!sAUBaseCFStringsInitialized)
	{
		kUntitledString = CFSTR("Untitled");
		kVersionString = CFSTR(kAUPresetVersionKey);
		kTypeString = CFSTR(kAUPresetTypeKey);
		kSubtypeString = CFSTR(kAUPresetSubtypeKey);
		kManufacturerString = CFSTR(kAUPresetManufacturerKey);
		kDataString = CFSTR(kAUPresetDataKey);
		kNameString = CFSTR(kAUPresetNameKey);
		kRenderQualityString = CFSTR(kAUPresetRenderQualityKey);
		kCPULoadString = CFSTR(kAUPresetCPULoadKey);
		kElementNameString = CFSTR(kAUPresetElementNameKey);
		kPartString = CFSTR(kAUPresetPartKey);
		sAUBaseCFStringsInitialized = true;
	}

	if (sVectorUnitType == kVecUninitialized) {
		sVectorUnitType = CAVectorUnit::GetVectorUnitType() ;
	}

	mAudioUnitAPIVersion = 2;
	
	SetMaxFramesPerSlice(kAUDefaultMaxFramesPerSlice);

	GlobalScope().Initialize(this, kAudioUnitScope_Global, 1);
	
#if !CA_NO_AU_UI_FEATURES
	memset (&mHostCallbackInfo, 0, sizeof (mHostCallbackInfo));
#endif


	mCurrentPreset.presetNumber = -1;
	mCurrentPreset.presetName = kUntitledString;
	CFRetain (mCurrentPreset.presetName);
}

//_____________________________________________________________________________
//
AUBase::~AUBase()
{
	if (mCurrentPreset.presetName) CFRelease (mCurrentPreset.presetName);
#if !CA_NO_AU_UI_FEATURES
	if (mContextName) CFRelease (mContextName);
#endif
	if (mLogString) delete [] mLogString;
    if (mNickName) CFRelease(mNickName);
}

//_____________________________________________________________________________
//
void	AUBase::CreateElements()
{
	if (!mElementsCreated) {
		Inputs().Initialize(this, kAudioUnitScope_Input, mInitNumInputEls);
		Outputs().Initialize(this, kAudioUnitScope_Output, mInitNumOutputEls);
#if !CA_BASIC_AU_FEATURES
		Groups().Initialize(this, kAudioUnitScope_Group, mInitNumGroupEls);
#endif
		CreateExtendedElements();
		
		mElementsCreated = true;
	}
}

//_____________________________________________________________________________
//
void	AUBase::SetMaxFramesPerSlice(UInt32 nFrames)
{
	mMaxFramesPerSlice = nFrames;
	if (mBuffersAllocated)
		ReallocateBuffers();
	PropertyChanged(kAudioUnitProperty_MaximumFramesPerSlice, kAudioUnitScope_Global, 0);
}

//_____________________________________________________________________________
//
OSStatus			AUBase::CanSetMaxFrames() const
{
	return IsInitialized() ? kAudioUnitErr_Initialized : OSStatus(noErr);
}

//_____________________________________________________________________________
//
void				AUBase::ReallocateBuffers()
{
	CreateElements();
	
	UInt32 nOutputs = Outputs().GetNumberOfElements();
	for (UInt32 i = 0; i < nOutputs; ++i) {
		AUOutputElement *output = GetOutput(i);
		output->AllocateBuffer();	// does no work if already allocated
	}
	UInt32 nInputs = Inputs().GetNumberOfElements();
	for (UInt32 i = 0; i < nInputs; ++i) {
		AUInputElement *input = GetInput(i);
		input->AllocateBuffer();	// does no work if already allocated
	}
	mBuffersAllocated = true;
}

//_____________________________________________________________________________
//
void				AUBase::DeallocateIOBuffers()
{
	if (!mBuffersAllocated)
		return;

	UInt32 nOutputs = Outputs().GetNumberOfElements();
	for (UInt32 i = 0; i < nOutputs; ++i) {
		AUOutputElement *output = GetOutput(i);
		output->DeallocateBuffer();
	}
	UInt32 nInputs = Inputs().GetNumberOfElements();
	for (UInt32 i = 0; i < nInputs; ++i) {
		AUInputElement *input = GetInput(i);
		input->DeallocateBuffer();
	}
	mBuffersAllocated = false;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::DoInitialize()
{
	OSStatus result = noErr;
	
	if (!mInitialized) {
		result = Initialize();
		if (result == noErr) {
			if (CanScheduleParameters())
				mParamList.reserve(24);
			mHasBegunInitializing = true;
			ReallocateBuffers();	// calls CreateElements()
			mInitialized = true;	// signal that it's okay to render
			CAMemoryBarrier();
		}
	}

	return result;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::Initialize()
{
	return noErr;
}

//_____________________________________________________________________________
//
void				AUBase::PreDestructor()
{
	// this is called from the ComponentBase dispatcher, which doesn't know anything about our (optional) lock
	CAMutex::Locker lock(mAUMutex);
	DoCleanup();
}

//_____________________________________________________________________________
//
void				AUBase::DoCleanup()
{
	if (mInitialized)
		Cleanup();
	
	DeallocateIOBuffers();
	ResetRenderTime ();

	mInitialized = false;
	mHasBegunInitializing = false;
}

//_____________________________________________________________________________
//
void				AUBase::Cleanup()
{
}

//_____________________________________________________________________________
//
OSStatus			AUBase::Reset(					AudioUnitScope 					inScope,
													AudioUnitElement 				inElement)
{
	ResetRenderTime ();
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::DispatchGetPropertyInfo(AudioUnitPropertyID				inID,
													AudioUnitScope					inScope,
													AudioUnitElement				inElement,
													UInt32 &						outDataSize,
													Boolean &						outWritable)
{
	OSStatus result = noErr;
	bool validateElement = true;
	
	switch (inID) {
	case kAudioUnitProperty_MakeConnection:
		ca_require(inScope == kAudioUnitScope_Input || inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = sizeof(AudioUnitConnection);
		outWritable = true;
		break;
		
		
	case kAudioUnitProperty_SetRenderCallback:
		ca_require(AudioUnitAPIVersion() > 1, InvalidProperty);
		ca_require(inScope == kAudioUnitScope_Input || inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = sizeof(AURenderCallbackStruct);
		outWritable = true;
		break;
		
	case kAudioUnitProperty_StreamFormat:
		outDataSize = sizeof(CAStreamBasicDescription);
		outWritable = IsStreamFormatWritable(inScope, inElement);
		break;

	case kAudioUnitProperty_SampleRate:
		outDataSize = sizeof(Float64);
		outWritable = IsStreamFormatWritable(inScope, inElement);
		break;

	case kAudioUnitProperty_ClassInfo:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = sizeof(CFPropertyListRef);
		outWritable = true;
		break;

	case kAudioUnitProperty_FactoryPresets:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		result = GetPresets(NULL);
		if (!result) {
			outDataSize = sizeof(CFArrayRef);
			outWritable = false;
		}
		break;
		
	case kAudioUnitProperty_PresentPreset:
#if !CA_USE_AUDIO_PLUGIN_ONLY
#ifndef __LP64__
	case kAudioUnitProperty_CurrentPreset:
#endif
#endif
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = sizeof(AUPreset);
		outWritable = true;
		break;
	
	case kAudioUnitProperty_ElementName:
		outDataSize = sizeof (CFStringRef);
		outWritable = true;
		break;
	
	case kAudioUnitProperty_ParameterList:
		{
			UInt32 nparams = 0;
			result = GetParameterList(inScope, NULL, nparams);
			
			outDataSize = sizeof(AudioUnitParameterID) * nparams;
			outWritable = false;
			validateElement = false;
		}
		break;
		
	case kAudioUnitProperty_ParameterInfo:
		outDataSize = sizeof(AudioUnitParameterInfo);
		outWritable = false;
		validateElement = false;
		break;

	case kAudioUnitProperty_ParameterHistoryInfo:
		outDataSize = sizeof(AudioUnitParameterHistoryInfo);
		outWritable = false;
		validateElement = false;
		break;

	case kAudioUnitProperty_ElementCount:
		outDataSize = sizeof(UInt32);
		outWritable = BusCountWritable(inScope);
		validateElement = false;
		break;
	
	case kAudioUnitProperty_Latency:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = sizeof(Float64);
		outWritable = false;
		break;
	
	case kAudioUnitProperty_TailTime:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		if (SupportsTail()) {
			outDataSize = sizeof(Float64);
			outWritable = false;
		} else
			goto InvalidProperty;
		break;
	
	case kAudioUnitProperty_MaximumFramesPerSlice:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = sizeof(UInt32);
		outWritable = true;
		break;
	
	case kAudioUnitProperty_LastRenderError:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = sizeof(OSStatus);
		outWritable = false;
		break;
		
	case kAudioUnitProperty_SupportedNumChannels:
	{
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		UInt32 num = SupportedNumChannels (NULL);
		if (num) {
			outDataSize = sizeof (AUChannelInfo) * num;
			result = noErr;
		} else
			goto InvalidProperty;
		outWritable = false;
		break;
	}
	
	case kAudioUnitProperty_SupportedChannelLayoutTags:
	{
		UInt32 numLayouts = GetChannelLayoutTags(inScope, inElement, NULL);
		if (numLayouts) {
			outDataSize = numLayouts * sizeof(AudioChannelLayoutTag);
			result = noErr;
		} else
			goto InvalidProperty;
		outWritable = false;
		validateElement = false; //already done it
		break;
	}
	
	case kAudioUnitProperty_AudioChannelLayout:
	{
		outWritable = false;
		outDataSize = GetAudioChannelLayout(inScope, inElement, NULL, outWritable);
		if (outDataSize) {
			result = noErr;
		} else {
			if (GetChannelLayoutTags(inScope, inElement, NULL) == 0)
				goto InvalidProperty;
			else
				result = kAudioUnitErr_InvalidPropertyValue;
		}
		validateElement = false; //already done it
		break;
	}

#if (MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5) || TARGET_OS_IPHONE
	case kAudioUnitProperty_ShouldAllocateBuffer:
		ca_require((inScope == kAudioUnitScope_Input || inScope == kAudioUnitScope_Output), InvalidScope);
		outWritable = true;
		outDataSize = sizeof(UInt32);
		break;
#endif

#if !CA_USE_AUDIO_PLUGIN_ONLY
	case kAudioUnitProperty_FastDispatch:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		if (!IsCMgrObject()) goto InvalidProperty;
		outDataSize = sizeof(void *);
		outWritable = false;
		validateElement = false;
		break;

	case kAudioUnitProperty_GetUIComponentList:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = GetNumCustomUIComponents();
		if (outDataSize == 0)
			goto InvalidProperty;
		outDataSize *= sizeof (AudioComponentDescription);
		
		outWritable = false;
		break;
#endif
	
	case kAudioUnitProperty_ParameterValueStrings:
		result = GetParameterValueStrings(inScope, inElement, NULL);
		if (result == noErr) {
			outDataSize = sizeof(CFArrayRef);
			outWritable = false;
			validateElement = false;
		}
		break;

#if !CA_NO_AU_HOST_CALLBACKS
	case kAudioUnitProperty_HostCallbacks:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = sizeof(mHostCallbackInfo);
		outWritable = true;
		break;
#endif
#if !CA_NO_AU_UI_FEATURES
	case kAudioUnitProperty_ContextName:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		outDataSize = sizeof(CFStringRef);
		outWritable = true;
		break;
	
	case kAudioUnitProperty_IconLocation:
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		outWritable = false;
		if (!HasIcon())
			goto InvalidProperty;
		outDataSize = sizeof(CFURLRef);
		break;

	case kAudioUnitProperty_ParameterClumpName:
		outDataSize = sizeof(AudioUnitParameterNameInfo );
		outWritable = false;
		break;

#endif // !CA_NO_AU_UI_FEATURES

	case 'lrst' :  // kAudioUnitProperty_LastRenderedSampleTime
		outDataSize = sizeof(Float64);
		outWritable = false;
		break;
	
    case kAudioUnitProperty_NickName:
        ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
        outDataSize = sizeof(CFStringRef);
        outWritable = true;
        break;

	default:
		result = GetPropertyInfo(inID, inScope, inElement, outDataSize, outWritable);
		validateElement = false;
		break;
	}

	if (result == noErr && validateElement) {
		ca_require(GetElement(inScope, inElement) != NULL, InvalidElement);	
	}
	
	return result;
InvalidProperty:
	return kAudioUnitErr_InvalidProperty;
InvalidScope:
	return kAudioUnitErr_InvalidScope;
InvalidElement:
	return kAudioUnitErr_InvalidElement;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::DispatchGetProperty(	AudioUnitPropertyID 			inID,
													AudioUnitScope 					inScope,
													AudioUnitElement			 	inElement,
													void *							outData)
{
	// NOTE: We're currently only called from AUBase::ComponentEntryDispatch, which
	// calls DispatchGetPropertyInfo first, which performs validation of the scope/element,
	// and ensures that the outData buffer is non-null and large enough.
	OSStatus result = noErr;

	switch (inID) {
	case kAudioUnitProperty_StreamFormat:
		*(CAStreamBasicDescription *)outData = GetStreamFormat(inScope, inElement);
		break;
	
	case kAudioUnitProperty_SampleRate:
		*(Float64 *)outData = GetStreamFormat(inScope, inElement).mSampleRate;
		break;
	
	case kAudioUnitProperty_ParameterList:
		{
			UInt32 nparams = 0;
			result = GetParameterList(inScope, (AudioUnitParameterID *)outData, nparams);
		}
		break;

	case kAudioUnitProperty_ParameterInfo:
		result = GetParameterInfo(inScope, inElement, *(AudioUnitParameterInfo *)outData);
		break;

	case kAudioUnitProperty_ParameterHistoryInfo:
		{
			AudioUnitParameterHistoryInfo* info = (AudioUnitParameterHistoryInfo*)outData;
			result = GetParameterHistoryInfo(inScope, inElement, info->updatesPerSecond, info->historyDurationInSeconds);
		}
		break;

	case kAudioUnitProperty_ClassInfo:
		{
			*(CFPropertyListRef *)outData = NULL;
			result = SaveState((CFPropertyListRef *)outData);
		}
		break;

	case kAudioUnitProperty_FactoryPresets:
		{
			*(CFArrayRef *)outData = NULL;
			result = GetPresets ((CFArrayRef *)outData);
		}
		break;
	
	case kAudioUnitProperty_PresentPreset:
#if !CA_USE_AUDIO_PLUGIN_ONLY
#ifndef __LP64__
	case kAudioUnitProperty_CurrentPreset:
#endif
#endif
		{
			*(AUPreset *)outData = mCurrentPreset;
				
				// retain current string (as client owns a reference to it and will release it)
			if (inID == kAudioUnitProperty_PresentPreset && mCurrentPreset.presetName) 
				CFRetain (mCurrentPreset.presetName);

			result = noErr;
		}
		break;
	
	case kAudioUnitProperty_ElementName:
		{
			AUElement * element = GetElement(inScope, inElement);
			if (element->HasName()) {
				*(CFStringRef *)outData = element->GetName();
				CFRetain (element->GetName());
				result = noErr;
			} else
				result = kAudioUnitErr_InvalidPropertyValue;
		}
		break;

	case kAudioUnitProperty_ElementCount:
		*(UInt32 *)outData = GetScope(inScope).GetNumberOfElements();
		break;
	
	case kAudioUnitProperty_Latency:
		*(Float64 *)outData = GetLatency();
		break;

	case kAudioUnitProperty_TailTime:
		if (SupportsTail())
			*(Float64 *)outData = GetTailTime();
		else
			result = kAudioUnitErr_InvalidProperty;
		break;
	
	case kAudioUnitProperty_MaximumFramesPerSlice:
		*(UInt32 *)outData = mMaxFramesPerSlice;
		break;

	case kAudioUnitProperty_LastRenderError:
		*(OSStatus *)outData = mLastRenderError;
		mLastRenderError = 0;
		break;

	case kAudioUnitProperty_SupportedNumChannels:
		{
			const AUChannelInfo* infoPtr = NULL;
			UInt32 num = SupportedNumChannels (&infoPtr);
			if(num != 0 && infoPtr != NULL)
				memcpy (outData, infoPtr, num * sizeof (AUChannelInfo));
		}
		break;

	case kAudioUnitProperty_SupportedChannelLayoutTags:
		{
			AudioChannelLayoutTag* ptr = outData ? static_cast<AudioChannelLayoutTag*>(outData) : NULL;
			UInt32 numLayouts = GetChannelLayoutTags (inScope, inElement, ptr);
			if (numLayouts == 0)
				result = kAudioUnitErr_InvalidProperty;
		}
		break;
		
	case kAudioUnitProperty_AudioChannelLayout:
	{	
		AudioChannelLayout* ptr = outData ? static_cast<AudioChannelLayout*>(outData) : NULL;
		Boolean writable;
		UInt32 dataSize = GetAudioChannelLayout(inScope, inElement, ptr, writable);
		if (!dataSize) {
			result = kAudioUnitErr_InvalidProperty;
		}
		break;
	}

#if (MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5) || TARGET_OS_IPHONE
	case kAudioUnitProperty_ShouldAllocateBuffer:
	{
		AUIOElement * element = GetIOElement(inScope, inElement);
		*(UInt32*)outData = element->WillAllocateBuffer();
		break;
	}
#endif

 	case kAudioUnitProperty_ParameterValueStrings:
		result = GetParameterValueStrings(inScope, inElement, (CFArrayRef *)outData);
		break;
		
#if !CA_USE_AUDIO_PLUGIN_ONLY
	case kAudioUnitProperty_FastDispatch:
		if (!IsCMgrObject()) result = kAudioUnitErr_InvalidProperty;
		else {
			switch (inElement) {
			case kAudioUnitGetParameterSelect:
				*(AudioUnitGetParameterProc *)outData = (AudioUnitGetParameterProc)CMgr_AudioUnitBaseGetParameter;
				break;
			case kAudioUnitSetParameterSelect:
				*(AudioUnitSetParameterProc *)outData = (AudioUnitSetParameterProc)CMgr_AudioUnitBaseSetParameter;
				break;
			case kAudioUnitRenderSelect:
				if (AudioUnitAPIVersion() > 1)
					*(AudioUnitRenderProc *)outData = (AudioUnitRenderProc)CMgr_AudioUnitBaseRender;
				else result = kAudioUnitErr_InvalidElement;
				break;
			default:
				result = GetProperty(inID, inScope, inElement, outData);
				break;
			}
		}
		break;

	case kAudioUnitProperty_GetUIComponentList:
		GetUIComponentDescs ((ComponentDescription*)outData);
		break;
#endif

#if !CA_NO_AU_HOST_CALLBACKS
	case kAudioUnitProperty_HostCallbacks:
		memcpy(outData, &mHostCallbackInfo, sizeof(mHostCallbackInfo));
		break;
#endif
#if !CA_NO_AU_UI_FEATURES
	case kAudioUnitProperty_IconLocation:
		{
			CFURLRef iconLocation = CopyIconLocation();
			if (iconLocation) {
				*(CFURLRef*)outData = iconLocation;
			} else
				result = kAudioUnitErr_InvalidProperty;
		}
		break;

	case kAudioUnitProperty_ContextName:
		*(CFStringRef *)outData = mContextName;
		if (mContextName) {
			CFRetain(mContextName);
			// retain CFString (if exists) since client will be responsible for its release
			result = noErr;
		} else {
			result = kAudioUnitErr_InvalidPropertyValue;
		}
		break;
		
	case kAudioUnitProperty_ParameterClumpName:
		{
			AudioUnitParameterNameInfo * ioClumpInfo = (AudioUnitParameterNameInfo*) outData;
			if (ioClumpInfo->inID == kAudioUnitClumpID_System)	// this ID value is reserved
				result = kAudioUnitErr_InvalidPropertyValue;
			else 
			{
				result = CopyClumpName(inScope, ioClumpInfo->inID, ioClumpInfo->inDesiredLength, &ioClumpInfo->outName);
					
					// this is provided for compatbility with existing implementations that don't know
					// about this new mechanism
				if (result == kAudioUnitErr_InvalidProperty)
					result = GetProperty (inID, inScope, inElement, outData);
			}	
		}
		break;

#endif  // !CA_NO_AU_UI_FEATURES

	case 'lrst' : // kAudioUnitProperty_LastRenderedSampleTime
		*(Float64*)outData = mCurrentRenderTime.mSampleTime;
		break;

    case kAudioUnitProperty_NickName:
        // Ownership follows Core Foundation's 'Copy Rule'
        if (mNickName) CFRetain(mNickName);
        *(CFStringRef*)outData = mNickName;
        break;
            
	default:
		result = GetProperty(inID, inScope, inElement, outData);
		break;
	}
	return result;
}


//_____________________________________________________________________________
//
OSStatus			AUBase::DispatchSetProperty(	AudioUnitPropertyID 			inID,
													AudioUnitScope 					inScope,
													AudioUnitElement 				inElement,
													const void *					inData,
													UInt32 							inDataSize)
{
	OSStatus result = noErr;

	switch (inID) {
	case kAudioUnitProperty_MakeConnection:
		ca_require(inDataSize >= sizeof(AudioUnitConnection), InvalidPropertyValue);
		{
			AudioUnitConnection &connection = *(AudioUnitConnection *)inData;
			result = SetConnection(connection);
		}
		break;

		
	case kAudioUnitProperty_SetRenderCallback:
		{
			ca_require(inDataSize >= sizeof(AURenderCallbackStruct), InvalidPropertyValue);
			ca_require(AudioUnitAPIVersion() > 1, InvalidProperty);
			AURenderCallbackStruct &callback = *(AURenderCallbackStruct*)inData;
			result = SetInputCallback(kAudioUnitProperty_SetRenderCallback, inElement, callback.inputProc, callback.inputProcRefCon);
		}
		break;

	case kAudioUnitProperty_ElementCount:
		ca_require(inDataSize == sizeof(UInt32), InvalidPropertyValue);
		ca_require(BusCountWritable(inScope), NotWritable);
		result = SetBusCount(inScope, *(UInt32*)inData);
		if (result == noErr) {
			PropertyChanged(inID, inScope, inElement);
		}
		break;
	
	case kAudioUnitProperty_MaximumFramesPerSlice:
		ca_require(inDataSize == sizeof(UInt32), InvalidPropertyValue);
		result = CanSetMaxFrames();
		if (result) return result;
		SetMaxFramesPerSlice(*(UInt32 *)inData);
		break;

	case kAudioUnitProperty_StreamFormat:
		{
			if (inDataSize < 36) goto InvalidPropertyValue;
			ca_require(GetElement(inScope, inElement) != NULL, InvalidElement);

			CAStreamBasicDescription newDesc;
				// now we're going to be ultra conservative! because of discrepancies between
				// sizes of this struct based on aligment padding inconsistencies
			memset (&newDesc, 0, sizeof(newDesc));
			memcpy (&newDesc, inData, 36);

			ca_require(ValidFormat(inScope, inElement, newDesc), InvalidFormat);

			const CAStreamBasicDescription curDesc = GetStreamFormat(inScope, inElement);
			
			if ( !curDesc.IsEqual(newDesc, false) ) {
				ca_require(IsStreamFormatWritable(inScope, inElement), NotWritable);
				result = ChangeStreamFormat(inScope, inElement, curDesc, newDesc);
			}
		}
		break;
	
	case kAudioUnitProperty_SampleRate:
		{
			ca_require(inDataSize == sizeof(Float64), InvalidPropertyValue);
			ca_require(GetElement(inScope, inElement) != NULL, InvalidElement);

			const CAStreamBasicDescription curDesc = GetStreamFormat(inScope, inElement);
			CAStreamBasicDescription newDesc = curDesc;
			newDesc.mSampleRate = *(Float64 *)inData;
			
			ca_require(ValidFormat(inScope, inElement, newDesc), InvalidFormat);
			
			if ( !curDesc.IsEqual(newDesc, false) ) {
				ca_require(IsStreamFormatWritable(inScope, inElement), NotWritable);
				result = ChangeStreamFormat(inScope, inElement, curDesc, newDesc);
			}
		}
		break;

	case kAudioUnitProperty_AudioChannelLayout:
		{
			const AudioChannelLayout *layout = static_cast<const AudioChannelLayout *>(inData);
			size_t headerSize = sizeof(AudioChannelLayout) - sizeof(AudioChannelDescription);
			
			ca_require(inDataSize >= headerSize + layout->mNumberChannelDescriptions * sizeof(AudioChannelDescription), InvalidPropertyValue);
			result = SetAudioChannelLayout(inScope, inElement, layout);
			if (result == noErr)
				PropertyChanged(inID, inScope, inElement);
			break;
		}
	
	case kAudioUnitProperty_ClassInfo:
		ca_require(inDataSize == sizeof(CFPropertyListRef *), InvalidPropertyValue);
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		result = RestoreState(*(CFPropertyListRef *)inData);
		break;

	case kAudioUnitProperty_PresentPreset:
#if !CA_USE_AUDIO_PLUGIN_ONLY
#ifndef __LP64__
	case kAudioUnitProperty_CurrentPreset:
#endif
#endif
		{
			ca_require(inDataSize == sizeof(AUPreset), InvalidPropertyValue);
			ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
			AUPreset & newPreset = *(AUPreset *)inData;
			
			if (newPreset.presetNumber >= 0)
			{
				result = NewFactoryPresetSet(newPreset);
				// NewFactoryPresetSet SHOULD call SetAFactoryPreset if the preset is valid
				// from its own list of preset number->name
				if (!result)
					PropertyChanged(inID, inScope, inElement);
			}
			else if (newPreset.presetName)
			{
				result = NewCustomPresetSet(newPreset);
                if (!result)
                    PropertyChanged(inID, inScope, inElement);
			}
			else
				result = kAudioUnitErr_InvalidPropertyValue;
		}
		break;
	
	case kAudioUnitProperty_ElementName:
		{
			ca_require(GetElement(inScope, inElement) != NULL, InvalidElement);
			ca_require(inDataSize == sizeof(CFStringRef), InvalidPropertyValue);
			AUElement * element = GetScope(inScope).GetElement (inElement);
			element->SetName (*(CFStringRef *)inData);
			PropertyChanged(inID, inScope, inElement);
		}
		break;

#if (MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5) || TARGET_OS_IPHONE
	case kAudioUnitProperty_ShouldAllocateBuffer:
		{
			ca_require((inScope == kAudioUnitScope_Input || inScope == kAudioUnitScope_Output), InvalidScope);
			ca_require(GetElement(inScope, inElement) != NULL, InvalidElement);
			ca_require(inDataSize == sizeof(UInt32), InvalidPropertyValue);
			ca_require(!IsInitialized(), Initialized);

			AUIOElement * element = GetIOElement(inScope, inElement);
			element->SetWillAllocateBuffer(*(UInt32 *)inData != 0);
		}
		break;
#endif

#if !CA_NO_AU_HOST_CALLBACKS
	case kAudioUnitProperty_HostCallbacks:
	{
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		UInt32 availSize = std::min(inDataSize, (UInt32)sizeof(HostCallbackInfo));
		bool hasChanged = !memcmp (&mHostCallbackInfo, inData, availSize);
		memset (&mHostCallbackInfo, 0, sizeof (mHostCallbackInfo));
		memcpy (&mHostCallbackInfo, inData, availSize);
		if (hasChanged)
			PropertyChanged(inID, inScope, inElement);
		break;
	}
#endif
#if !CA_NO_AU_UI_FEATURES
	case kAudioUnitProperty_SetExternalBuffer:
		ca_require(inDataSize >= sizeof(AudioUnitExternalBuffer), InvalidPropertyValue);
		ca_require(IsInitialized(), Uninitialized);
		{
			AudioUnitExternalBuffer &buf = *(AudioUnitExternalBuffer*)inData;
			if (intptr_t(buf.buffer) & 0x0F) result = kAudio_ParamError;
			else if (inScope == kAudioUnitScope_Input) {
				AUInputElement *input = GetInput(inElement);
				input->UseExternalBuffer(buf);
			} else {
				AUOutputElement *output = GetOutput(inElement);
				output->UseExternalBuffer(buf);
			}
		}
		break;

	case kAudioUnitProperty_ContextName:
		{
			ca_require(inDataSize == sizeof(CFStringRef), InvalidPropertyValue);
			ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
			CFStringRef inStr = *(CFStringRef *)inData;
			if (mContextName) CFRelease(mContextName);
			if (inStr) CFRetain(inStr);
			mContextName = inStr;
			PropertyChanged(inID, inScope, inElement);
		}
		break;

#endif // !CA_NO_AU_UI_FEATURES
	
    case kAudioUnitProperty_NickName:
    {
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
        ca_require(inDataSize == sizeof(CFStringRef), InvalidPropertyValue);
        CFStringRef inStr = *(CFStringRef *)inData;
        if (mNickName) CFRelease(mNickName);
        if (inStr) CFRetain(inStr);
        mNickName = inStr;
        PropertyChanged(inID, inScope, inElement);
        break;
    }
            
	default:
		result = SetProperty(inID, inScope, inElement, inData, inDataSize);
		if (result == noErr)
			PropertyChanged(inID, inScope, inElement);
		
		break;
	}
	return result;
NotWritable:
	return kAudioUnitErr_PropertyNotWritable;
InvalidFormat:
	return kAudioUnitErr_FormatNotSupported;
#if !CA_NO_AU_UI_FEATURES
Uninitialized:
	return kAudioUnitErr_Uninitialized;
#endif
#if (MAC_OS_X_VERSION_MIN_REQUIRED > MAC_OS_X_VERSION_10_5) || CA_USE_AUDIO_PLUGIN_ONLY
Initialized:
	return kAudioUnitErr_Initialized;
#endif
InvalidScope:
	return kAudioUnitErr_InvalidScope;
InvalidProperty:
	return kAudioUnitErr_InvalidProperty;
InvalidPropertyValue:
	return kAudioUnitErr_InvalidPropertyValue;
InvalidElement:
	return kAudioUnitErr_InvalidElement;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::DispatchRemovePropertyValue (AudioUnitPropertyID		inID,
													AudioUnitScope 					inScope,
													AudioUnitElement 				inElement)
{
	OSStatus result = noErr;
	switch (inID)
	{
	case kAudioUnitProperty_AudioChannelLayout:
	{
		result = RemoveAudioChannelLayout(inScope, inElement);
		if (result == noErr)
			PropertyChanged(inID, inScope, inElement);
		break;
	}
	
#if !CA_NO_AU_HOST_CALLBACKS
	case kAudioUnitProperty_HostCallbacks:
	{
		ca_require(inScope == kAudioUnitScope_Global, InvalidScope);
		bool hasValue = false;
		void* ptr = &mHostCallbackInfo;
		for (unsigned int i = 0; i <  sizeof (HostCallbackInfo); ++i) {
			if (static_cast<char*>(ptr)[i]) {
				hasValue = true;
				break;
			}
		}
		if (hasValue) {
			memset (&mHostCallbackInfo, 0, sizeof (HostCallbackInfo));
			PropertyChanged(inID, inScope, inElement);
		}
		break;
	}
#endif
#if !CA_NO_AU_UI_FEATURES
	case kAudioUnitProperty_ContextName:
		if (mContextName) CFRelease(mContextName);
		mContextName = NULL;
		result = noErr;
		break;
	
#endif // !CA_NO_AU_UI_FEATURES

    case kAudioUnitProperty_NickName:
    {
        if(inScope == kAudioUnitScope_Global) {
            if (mNickName) CFRelease(mNickName);
            mNickName = NULL;
            PropertyChanged(inID, inScope, inElement);
        } else {
            result = kAudioUnitErr_InvalidScope;
        }
        break;
    }

	default:
		result = RemovePropertyValue (inID, inScope, inElement);		
		break;
	}
		
	return result;
#if !CA_NO_AU_UI_FEATURES || !CA_NO_AU_HOST_CALLBACKS
InvalidScope:
	return kAudioUnitErr_InvalidScope;
#endif
}

//_____________________________________________________________________________
//
OSStatus			AUBase::GetPropertyInfo(		AudioUnitPropertyID				inID,
													AudioUnitScope					inScope,
													AudioUnitElement				inElement,
													UInt32 &						outDataSize,
													Boolean &						outWritable)
{
	return kAudioUnitErr_InvalidProperty;
}


//_____________________________________________________________________________
//
OSStatus			AUBase::GetProperty(			AudioUnitPropertyID 			inID,
													AudioUnitScope 					inScope,
													AudioUnitElement			 	inElement,
													void *							outData)
{
	return kAudioUnitErr_InvalidProperty;
}


//_____________________________________________________________________________
//
OSStatus			AUBase::SetProperty(			AudioUnitPropertyID 			inID,
													AudioUnitScope 					inScope,
													AudioUnitElement 				inElement,
													const void *					inData,
													UInt32 							inDataSize)
{
	return kAudioUnitErr_InvalidProperty;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::RemovePropertyValue (	AudioUnitPropertyID		 		inID,
													AudioUnitScope 					inScope,
													AudioUnitElement 				inElement)
{
	return kAudioUnitErr_InvalidPropertyValue;
}
												
//_____________________________________________________________________________
//
OSStatus			AUBase::AddPropertyListener(	AudioUnitPropertyID				inID,
													AudioUnitPropertyListenerProc	inProc,
													void *							inProcRefCon)
{
	PropertyListener pl;
	
	pl.propertyID = inID;
	pl.listenerProc = inProc;
	pl.listenerRefCon = inProcRefCon;
	
	if (mPropertyListeners.empty())
		mPropertyListeners.reserve(32);
	mPropertyListeners.push_back(pl);

	return noErr;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::RemovePropertyListener(		AudioUnitPropertyID				inID,
														AudioUnitPropertyListenerProc	inProc,
														void *							inProcRefCon,
														bool							refConSpecified)
{
	// iterate in reverse so that it's safe to erase in the middle of the vector
	for (int i = (int)mPropertyListeners.size(); --i >=0; ) {
		PropertyListeners::iterator it = mPropertyListeners.begin() + i;
		if ((*it).propertyID == inID && (*it).listenerProc == inProc && (!refConSpecified || (*it).listenerRefCon == inProcRefCon))
			mPropertyListeners.erase(it);
	}
	return noErr;
}

//_____________________________________________________________________________
//
void				AUBase::PropertyChanged(			AudioUnitPropertyID				inID,
														AudioUnitScope					inScope, 
														AudioUnitElement				inElement)
{
	for (PropertyListeners::iterator it = mPropertyListeners.begin(); it != mPropertyListeners.end(); ++it)
		if ((*it).propertyID == inID)
			((*it).listenerProc)((*it).listenerRefCon, mComponentInstance, inID, inScope, inElement);
}

//_____________________________________________________________________________
//
OSStatus			AUBase::SetRenderNotification(	AURenderCallback		 		inProc,
													void *							inRefCon)
{
	if (inProc == NULL)
		return kAudio_ParamError;

	mRenderCallbacksTouched = true;
	mRenderCallbacks.deferred_add(RenderCallback(inProc, inRefCon));
			// this will do nothing if it's already in the list
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::RemoveRenderNotification(	AURenderCallback			inProc,
														void *						inRefCon)
{
	mRenderCallbacks.deferred_remove(RenderCallback(inProc, inRefCon));
	return noErr;	// error?
}

//_____________________________________________________________________________
//
OSStatus 	AUBase::GetParameter(			AudioUnitParameterID			inID,
													AudioUnitScope 					inScope,
													AudioUnitElement 				inElement,
													AudioUnitParameterValue &		outValue)
{
	AUElement *elem = SafeGetElement(inScope, inElement);
	outValue = elem->GetParameter(inID);
	return noErr;
}

											
//_____________________________________________________________________________
//
OSStatus 	AUBase::SetParameter(			AudioUnitParameterID			inID,
													AudioUnitScope 					inScope,
													AudioUnitElement 				inElement,
													AudioUnitParameterValue			inValue,
													UInt32							inBufferOffsetInFrames)
{
	AUElement *elem = SafeGetElement(inScope, inElement);
	elem->SetParameter(inID, inValue);
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus 	AUBase::ScheduleParameter (	const AudioUnitParameterEvent 		*inParameterEvent,
													UInt32							inNumEvents)
{
	bool canScheduleParameters = CanScheduleParameters();
		
	for (UInt32 i = 0; i < inNumEvents; ++i) 
	{
		if (inParameterEvent[i].eventType == kParameterEvent_Immediate)
		{
			SetParameter (inParameterEvent[i].parameter,
							inParameterEvent[i].scope, 
							inParameterEvent[i].element,
							inParameterEvent[i].eventValues.immediate.value, 
							inParameterEvent[i].eventValues.immediate.bufferOffset);
		}
		if (canScheduleParameters) {
			mParamList.push_back (inParameterEvent[i]);
		}
	}
	
	return noErr;
}

// ____________________________________________________________________________
//
static bool SortParameterEventList(const AudioUnitParameterEvent &ev1, const AudioUnitParameterEvent &ev2 )
{
	int offset1 = ev1.eventType == kParameterEvent_Immediate ?  ev1.eventValues.immediate.bufferOffset : ev1.eventValues.ramp.startBufferOffset;
	int offset2 = ev2.eventType == kParameterEvent_Immediate ?  ev2.eventValues.immediate.bufferOffset : ev2.eventValues.ramp.startBufferOffset;

	if(offset1 < offset2) return true;
	return false;
}


// ____________________________________________________________________________
//
OSStatus 	AUBase::ProcessForScheduledParams(	ParameterEventList		&inParamList,
														UInt32					inFramesToProcess,
														void					*inUserData )
{
	OSStatus result = noErr;
	
	int totalFramesToProcess = inFramesToProcess;
	
	int framesRemaining = totalFramesToProcess;

	unsigned int currentStartFrame = 0;	// start of the whole buffer



	// sort the ParameterEventList by startBufferOffset
	std::sort(inParamList.begin(), inParamList.end(), SortParameterEventList);

	ParameterEventList::iterator iter = inParamList.begin();
	
	
	while(framesRemaining > 0 )
	{
		// first of all, go through the ramped automation events and find out where the next
		// division of our whole buffer will be
		
		int currentEndFrame = totalFramesToProcess;	// start out assuming we'll process all the way to
													// the end of the buffer
		
		iter = inParamList.begin();
		
		// find the next break point
		while(iter != inParamList.end() )
		{
			AudioUnitParameterEvent &event = *iter;
			
			int offset = event.eventType == kParameterEvent_Immediate ?  event.eventValues.immediate.bufferOffset : event.eventValues.ramp.startBufferOffset;

			if(offset > (int)currentStartFrame && offset < currentEndFrame )
			{
				currentEndFrame = offset;
				break;
			}

			// consider ramp end to be a possible choice (there may be gaps in the supplied ramp events)
			if(event.eventType == kParameterEvent_Ramped )
			{
				offset = event.eventValues.ramp.startBufferOffset + event.eventValues.ramp.durationInFrames;
	
				if(offset > (int)currentStartFrame && offset < currentEndFrame )
				{
					currentEndFrame = offset;
				}
			}

			iter++;
		}
	
		int framesThisTime = currentEndFrame - currentStartFrame;

		// next, setup the parameter maps to be current for the ramp parameters active during 
		// this time segment...
		
		for(ParameterEventList::iterator iter2 = inParamList.begin(); iter2 != inParamList.end(); iter2++ )
		{
			AudioUnitParameterEvent &event = *iter2;
			
			bool eventFallsInSlice;
			
			
			if(event.eventType == kParameterEvent_Ramped)
				eventFallsInSlice = event.eventValues.ramp.startBufferOffset < currentEndFrame 
					&& event.eventValues.ramp.startBufferOffset + event.eventValues.ramp.durationInFrames > currentStartFrame;
			else /* kParameterEvent_Immediate */
				// actually, for the same parameter, there may be future immediate events which override this one,
				// but it's OK since the event list is sorted in time order, we're guaranteed to end up with the current one
				eventFallsInSlice = event.eventValues.immediate.bufferOffset <= currentStartFrame;
				
			if(eventFallsInSlice)
			{
				AUElement *element = GetElement(event.scope, event.element );
				
				if(element) element->SetScheduledEvent(	event.parameter,
														event,
														currentStartFrame,
														currentEndFrame - currentStartFrame );
			}
		}



		// Finally, actually do the processing for this slice.....
		
		result = ProcessScheduledSlice(	inUserData,
										currentStartFrame,
										framesThisTime,
										inFramesToProcess );
								
		if(result != noErr) break;
		
		framesRemaining -= framesThisTime;
		currentStartFrame = currentEndFrame;	// now start from where we left off last time
	}
	
	return result;
}

//_____________________________________________________________________________
//
void				AUBase::SetWantsRenderThreadID (bool inFlag)
{
	if (inFlag == mWantsRenderThreadID)	
		return;
	
	mWantsRenderThreadID = inFlag;
	if (!mWantsRenderThreadID)
		mRenderThreadID = NULL;
}

//_____________________________________________________________________________
//

//_____________________________________________________________________________
//
OSStatus			AUBase::DoRender(		AudioUnitRenderActionFlags &	ioActionFlags,
											const AudioTimeStamp &			inTimeStamp,
											UInt32							inBusNumber,
											UInt32							inFramesToProcess,
											AudioBufferList &				ioData)
{
	OSStatus theError;
	RenderCallbackList::iterator rcit;
	
	AUTRACE(kCATrace_AUBaseRenderStart, mComponentInstance, (uintptr_t)this, inBusNumber, inFramesToProcess, (uintptr_t)ioData.mBuffers[0].mData);
	DISABLE_DENORMALS
	
	try {
		ca_require(IsInitialized(), Uninitialized);
		ca_require(mAudioUnitAPIVersion >= 2, ParamErr);
		if (inFramesToProcess > mMaxFramesPerSlice) {
			static time_t lastTimeMessagePrinted = 0;
			time_t now = time(NULL);
			if (now != lastTimeMessagePrinted) {
				lastTimeMessagePrinted = now;
				syslog(LOG_ERR, "kAudioUnitErr_TooManyFramesToProcess : inFramesToProcess=%u, mMaxFramesPerSlice=%u", (unsigned)inFramesToProcess, (unsigned)mMaxFramesPerSlice);
				DebugMessageN4("%s:%d inFramesToProcess=%u, mMaxFramesPerSlice=%u; TooManyFrames", __FILE__, __LINE__, (unsigned)inFramesToProcess, (unsigned)mMaxFramesPerSlice);
			}
			goto TooManyFrames;
		}
		ca_require (!UsesFixedBlockSize() || inFramesToProcess == GetMaxFramesPerSlice(), ParamErr);

		AUOutputElement *output = GetOutput(inBusNumber);	// will throw if non-existant
		if (output->GetStreamFormat().NumberChannelStreams() != ioData.mNumberBuffers) {
			DebugMessageN4("%s:%d ioData.mNumberBuffers=%u, output->GetStreamFormat().NumberChannelStreams()=%u; kAudio_ParamError",
				__FILE__, __LINE__, (unsigned)ioData.mNumberBuffers, (unsigned)output->GetStreamFormat().NumberChannelStreams());
			goto ParamErr;
		}

		unsigned expectedBufferByteSize = inFramesToProcess * output->GetStreamFormat().mBytesPerFrame;
		for (unsigned ibuf = 0; ibuf < ioData.mNumberBuffers; ++ibuf) {
			AudioBuffer &buf = ioData.mBuffers[ibuf];
			if (buf.mData != NULL) {
				// only care about the size if the buffer is non-null
				if (buf.mDataByteSize < expectedBufferByteSize) {
					// if the buffer is too small, we cannot render safely. kAudio_ParamError.
					DebugMessageN7("%s:%d %u frames, %u bytes/frame, expected %u-byte buffer; ioData.mBuffers[%u].mDataByteSize=%u; kAudio_ParamError",
						__FILE__, __LINE__, (unsigned)inFramesToProcess, (unsigned)output->GetStreamFormat().mBytesPerFrame, expectedBufferByteSize, ibuf, (unsigned)buf.mDataByteSize);
					goto ParamErr;
				}
				// Some clients incorrectly pass bigger buffers than expectedBufferByteSize.
				// We will generally set the buffer size at the end of rendering, before we return.
				// However we should ensure that no one, DURING rendering, READS a
				// potentially incorrect size. This can lead to doing too much work, or
				// reading past the end of an input buffer into unmapped memory.
				buf.mDataByteSize = expectedBufferByteSize;
			}
		}
		
		if (WantsRenderThreadID())
		{
			#if TARGET_OS_MAC
				mRenderThreadID = pthread_self();
			#elif TARGET_OS_WIN32
				mRenderThreadID = GetCurrentThreadId();
			#endif
		}
		
		AudioUnitRenderActionFlags flags;
		if (mRenderCallbacksTouched) {
			mRenderCallbacks.update();
			flags = ioActionFlags | kAudioUnitRenderAction_PreRender;
			for (rcit = mRenderCallbacks.begin(); rcit != mRenderCallbacks.end(); ++rcit) {
				RenderCallback &rc = *rcit;
				AUTRACE(kCATrace_AUBaseRenderCallbackStart, mComponentInstance, (intptr_t)this, (intptr_t)rc.mRenderNotify, 1, 0);
				(*(AURenderCallback)rc.mRenderNotify)(rc.mRenderNotifyRefCon, 
								&flags,
								&inTimeStamp, inBusNumber, inFramesToProcess, &ioData);
				AUTRACE(kCATrace_AUBaseRenderCallbackEnd, mComponentInstance, (intptr_t)this, (intptr_t)rc.mRenderNotify, 1, 0);
			}
		}
		
		theError = DoRenderBus(ioActionFlags, inTimeStamp, inBusNumber, output, inFramesToProcess, ioData);
		
		if (mRenderCallbacksTouched) {
			flags = ioActionFlags | kAudioUnitRenderAction_PostRender;
			
			if (SetRenderError (theError)) {
				flags |= kAudioUnitRenderAction_PostRenderError;		
			}
			
			for (rcit = mRenderCallbacks.begin(); rcit != mRenderCallbacks.end(); ++rcit) {
				RenderCallback &rc = *rcit;
				AUTRACE(kCATrace_AUBaseRenderCallbackStart, mComponentInstance, (intptr_t)this, (intptr_t)rc.mRenderNotify, 2, 0);
				(*(AURenderCallback)rc.mRenderNotify)(rc.mRenderNotifyRefCon, 
								&flags,
								&inTimeStamp, inBusNumber, inFramesToProcess, &ioData);
				AUTRACE(kCATrace_AUBaseRenderCallbackEnd, mComponentInstance, (intptr_t)this, (intptr_t)rc.mRenderNotify, 2, 0);
			}
		}

		// The vector's being emptied
		// because these events should only apply to this Render cycle, so anything
		// left over is from a preceding cycle and should be dumped.  New scheduled
		// parameters must be scheduled from the next pre-render callback.
		if (!mParamList.empty())
			mParamList.clear();

	}
	catch (OSStatus err) {
		theError = err;
		goto errexit;
	}
	catch (...) {
		theError = -1;
		goto errexit;
	}
done:	
	RESTORE_DENORMALS
	AUTRACE(kCATrace_AUBaseRenderEnd, mComponentInstance, (intptr_t)this, theError, ioActionFlags, CATrace::ablData(ioData));
	
	return theError;
	
Uninitialized:	theError = kAudioUnitErr_Uninitialized;				goto errexit;
ParamErr:		theError = kAudio_ParamError;						goto errexit;
TooManyFrames:	theError = kAudioUnitErr_TooManyFramesToProcess;	goto errexit;
errexit:
	DebugMessageN2 ("  from %s, render err: %d", GetLoggingString(), (int)theError);
	SetRenderError(theError);
	goto done;
}

//_____________________________________________________________________________
//
OSStatus	AUBase::DoProcess (	AudioUnitRenderActionFlags  &		ioActionFlags,
								const AudioTimeStamp &				inTimeStamp,
								UInt32								inFramesToProcess,
								AudioBufferList &					ioData)
{
	OSStatus theError;
	AUTRACE(kCATrace_AUBaseRenderStart, mComponentInstance, (intptr_t)this, -1, inFramesToProcess, 0);
	DISABLE_DENORMALS

	try {		
	
		if (!(ioActionFlags & (1 << 9)/*kAudioUnitRenderAction_DoNotCheckRenderArgs*/)) {
			ca_require(IsInitialized(), Uninitialized);
			ca_require(inFramesToProcess <= mMaxFramesPerSlice, TooManyFrames);
			ca_require(!UsesFixedBlockSize() || inFramesToProcess == GetMaxFramesPerSlice(), ParamErr);

			AUInputElement *input = GetInput(0);	// will throw if non-existant
			if (input->GetStreamFormat().NumberChannelStreams() != ioData.mNumberBuffers) {
				DebugMessageN4("%s:%d ioData.mNumberBuffers=%u, input->GetStreamFormat().NumberChannelStreams()=%u; kAudio_ParamError",
					__FILE__, __LINE__, (unsigned)ioData.mNumberBuffers, (unsigned)input->GetStreamFormat().NumberChannelStreams());
				goto ParamErr;
			}

			unsigned expectedBufferByteSize = inFramesToProcess * input->GetStreamFormat().mBytesPerFrame;
			for (unsigned ibuf = 0; ibuf < ioData.mNumberBuffers; ++ibuf) {
				AudioBuffer &buf = ioData.mBuffers[ibuf];
				if (buf.mData != NULL) {
					// only care about the size if the buffer is non-null
					if (buf.mDataByteSize < expectedBufferByteSize) {
						// if the buffer is too small, we cannot render safely. kAudio_ParamError.
						DebugMessageN7("%s:%d %u frames, %u bytes/frame, expected %u-byte buffer; ioData.mBuffers[%u].mDataByteSize=%u; kAudio_ParamError",
							__FILE__, __LINE__, (unsigned)inFramesToProcess, (unsigned)input->GetStreamFormat().mBytesPerFrame, expectedBufferByteSize, ibuf, (unsigned)buf.mDataByteSize);
						goto ParamErr;
					}
					// Some clients incorrectly pass bigger buffers than expectedBufferByteSize.
					// We will generally set the buffer size at the end of rendering, before we return.
					// However we should ensure that no one, DURING rendering, READS a
					// potentially incorrect size. This can lead to doing too much work, or
					// reading past the end of an input buffer into unmapped memory.
					buf.mDataByteSize = expectedBufferByteSize;
				}
			}
		}
		
		if (WantsRenderThreadID())
		{
			#if TARGET_OS_MAC
				mRenderThreadID = pthread_self();
			#elif TARGET_OS_WIN32
				mRenderThreadID = GetCurrentThreadId();
			#endif
		}
		
		if (NeedsToRender (inTimeStamp)) {
			theError = ProcessBufferLists (ioActionFlags, ioData, ioData, inFramesToProcess);
		} else
			theError = noErr;
			
	}
	catch (OSStatus err) {
		theError = err;
		goto errexit;
	}
	catch (...) {
		theError = -1;
		goto errexit;
	}
done:	
	RESTORE_DENORMALS
	AUTRACE(kCATrace_AUBaseRenderEnd, mComponentInstance, (intptr_t)this, theError, ioActionFlags, CATrace::ablData(ioData));
	
	return theError;
	
Uninitialized:	theError = kAudioUnitErr_Uninitialized;				goto errexit;
ParamErr:		theError = kAudio_ParamError;						goto errexit;
TooManyFrames:	theError = kAudioUnitErr_TooManyFramesToProcess;	goto errexit;
errexit:
	DebugMessageN2 ("  from %s, process err: %d", GetLoggingString(), (int)theError);
	SetRenderError(theError);
	goto done;
}

OSStatus	AUBase::DoProcessMultiple (	AudioUnitRenderActionFlags  & ioActionFlags,
							   const AudioTimeStamp &				inTimeStamp,
							   UInt32								inFramesToProcess,
							   UInt32								inNumberInputBufferLists,
							   const AudioBufferList **				inInputBufferLists,
							   UInt32								inNumberOutputBufferLists,
							   AudioBufferList **					ioOutputBufferLists)
{
	OSStatus theError;
	DISABLE_DENORMALS
	
	try {
		
		if (!(ioActionFlags & (1 << 9)/*kAudioUnitRenderAction_DoNotCheckRenderArgs*/)) {
			ca_require(IsInitialized(), Uninitialized);
			ca_require(inFramesToProcess <= mMaxFramesPerSlice, TooManyFrames);
			ca_require (!UsesFixedBlockSize() || inFramesToProcess == GetMaxFramesPerSlice(), ParamErr);
			
			for (unsigned ibl = 0; ibl < inNumberInputBufferLists; ++ibl) {
				if (inInputBufferLists[ibl] != NULL) {
					AUInputElement *input = GetInput(ibl);	// will throw if non-existant
					unsigned expectedBufferByteSize = inFramesToProcess * input->GetStreamFormat().mBytesPerFrame;
					
					if (input->GetStreamFormat().NumberChannelStreams() != inInputBufferLists[ibl]->mNumberBuffers) {
						DebugMessageN5("%s:%d inInputBufferLists[%u]->mNumberBuffers=%u, input->GetStreamFormat().NumberChannelStreams()=%u; kAudio_ParamError",
									   __FILE__, __LINE__, ibl, (unsigned)inInputBufferLists[ibl]->mNumberBuffers, (unsigned)input->GetStreamFormat().NumberChannelStreams());
						goto ParamErr;
					}
					
					for (unsigned ibuf = 0; ibuf < inInputBufferLists[ibl]->mNumberBuffers; ++ibuf) {
						const AudioBuffer &buf = inInputBufferLists[ibl]->mBuffers[ibuf];
						if (buf.mData != NULL) {
							if (buf.mDataByteSize < expectedBufferByteSize) {
								// the buffer is too small
								DebugMessageN8("%s:%d %u frames, %u bytes/frame, expected %u-byte buffer; inInputBufferLists[%u].mBuffers[%u].mDataByteSize=%u; kAudio_ParamError",
											   __FILE__, __LINE__, (unsigned)inFramesToProcess, (unsigned)input->GetStreamFormat().mBytesPerFrame, expectedBufferByteSize, ibl, ibuf, (unsigned)buf.mDataByteSize);
								goto ParamErr;
							}
						} else {
							// the buffer must exist
							goto ParamErr;
						}
					}
				} else {
					// skip NULL input audio buffer list
				}
			}
			
			for (unsigned obl = 0; obl < inNumberOutputBufferLists; ++obl) {
				if (ioOutputBufferLists[obl] != NULL) {
					AUOutputElement *output = GetOutput(obl);	// will throw if non-existant
					unsigned expectedBufferByteSize = inFramesToProcess * output->GetStreamFormat().mBytesPerFrame;

					if (output->GetStreamFormat().NumberChannelStreams() != ioOutputBufferLists[obl]->mNumberBuffers) {
						DebugMessageN5("%s:%d ioOutputBufferLists[%u]->mNumberBuffers=%u, output->GetStreamFormat().NumberChannelStreams()=%u; kAudio_ParamError",
									   __FILE__, __LINE__, obl, (unsigned)ioOutputBufferLists[obl]->mNumberBuffers, (unsigned)output->GetStreamFormat().NumberChannelStreams());
						goto ParamErr;
					}
					
					for (unsigned obuf = 0; obuf < ioOutputBufferLists[obl]->mNumberBuffers; ++obuf) {
						AudioBuffer &buf = ioOutputBufferLists[obl]->mBuffers[obuf];
						if (buf.mData != NULL) {
							// only care about the size if the buffer is non-null
							if (buf.mDataByteSize < expectedBufferByteSize) {
								// if the buffer is too small, we cannot render safely. kAudio_ParamError.
								DebugMessageN8("%s:%d %u frames, %u bytes/frame, expected %u-byte buffer; ioOutputBufferLists[%u]->mBuffers[%u].mDataByteSize=%u; kAudio_ParamError",
											   __FILE__, __LINE__, (unsigned)inFramesToProcess, (unsigned)output->GetStreamFormat().mBytesPerFrame, expectedBufferByteSize, obl, obuf, (unsigned)buf.mDataByteSize);
								goto ParamErr;
							}
							// Some clients incorrectly pass bigger buffers than expectedBufferByteSize.
							// We will generally set the buffer size at the end of rendering, before we return.
							// However we should ensure that no one, DURING rendering, READS a
							// potentially incorrect size. This can lead to doing too much work, or
							// reading past the end of an input buffer into unmapped memory.
							buf.mDataByteSize = expectedBufferByteSize;
						}
					}
				} else {
					// skip NULL output audio buffer list
				}
			}
		}
		
		if (WantsRenderThreadID())
		{
#if TARGET_OS_MAC
			mRenderThreadID = pthread_self();
#elif TARGET_OS_WIN32
			mRenderThreadID = GetCurrentThreadId();
#endif
		}
		
		if (NeedsToRender (inTimeStamp)) {
			theError = ProcessMultipleBufferLists (ioActionFlags, inFramesToProcess, inNumberInputBufferLists, inInputBufferLists, inNumberOutputBufferLists, ioOutputBufferLists);
		} else
			theError = noErr;
	}
	catch (OSStatus err) {
		theError = err;
		goto errexit;
	}
	catch (...) {
		theError = -1;
		goto errexit;
	}
done:	
	RESTORE_DENORMALS
	
	return theError;
	
Uninitialized:	theError = kAudioUnitErr_Uninitialized;				goto errexit;
ParamErr:		theError = kAudio_ParamError;						goto errexit;
TooManyFrames:	theError = kAudioUnitErr_TooManyFramesToProcess;	goto errexit;
errexit:
	DebugMessageN2 ("  from %s, processmultiple err: %d", GetLoggingString(), (int)theError);
	SetRenderError(theError);
	goto done;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::SetInputCallback(		UInt32							inPropertyID,
													AudioUnitElement 				inElement, 
													AURenderCallback				inProc,
													void *							inRefCon)
{
	AUInputElement *input = GetInput(inElement);	// may throw
	
	input->SetInputCallback(inProc, inRefCon);
	PropertyChanged(inPropertyID, kAudioUnitScope_Input, inElement);
	
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::SetConnection(			const AudioUnitConnection &		inConnection)
{

	OSStatus err;
	AUInputElement *input = GetInput(inConnection.destInputNumber);	// may throw
		
	if (inConnection.sourceAudioUnit) {
		// connecting, not disconnecting
		CAStreamBasicDescription sourceDesc;
		UInt32 size = sizeof(CAStreamBasicDescription);
		ca_require_noerr(err = AudioUnitGetProperty(
										inConnection.sourceAudioUnit,
										kAudioUnitProperty_StreamFormat,
										kAudioUnitScope_Output,
										inConnection.sourceOutputNumber,
										&sourceDesc,
										&size), errexit);
		ca_require_noerr(err = DispatchSetProperty (kAudioUnitProperty_StreamFormat, 
								kAudioUnitScope_Input, inConnection.destInputNumber, 
								&sourceDesc, sizeof(CAStreamBasicDescription)), errexit);
	}
	input->SetConnection(inConnection);
		
	PropertyChanged(kAudioUnitProperty_MakeConnection, kAudioUnitScope_Input, inConnection.destInputNumber);
	return noErr;

errexit:
	return err;
}
 
//_____________________________________________________________________________
//
UInt32				AUBase::SupportedNumChannels (	const AUChannelInfo** 			outInfo)
{
	return 0;
}

//_____________________________________________________________________________
//
bool				AUBase::ValidFormat(			AudioUnitScope					inScope,
													AudioUnitElement				inElement,
													const CAStreamBasicDescription &		inNewFormat)
{
	return FormatIsCanonical(inNewFormat);
}

//_____________________________________________________________________________
//
bool				AUBase::IsStreamFormatWritable(	AudioUnitScope					scope,
													AudioUnitElement				element)
{
	switch (scope) {
	case kAudioUnitScope_Input:
		{
			AUInputElement *input = GetInput(element);
			if (input->HasConnection()) return false;	// can't write format when input comes from connection
		}
		// ... fall ...
	case kAudioUnitScope_Output:
		return StreamFormatWritable(scope, element);

//#warning "aliasing of global scope format should be pushed to subclasses"
	case kAudioUnitScope_Global:
		return StreamFormatWritable(kAudioUnitScope_Output, 0);
	}
	return false;
}

//_____________________________________________________________________________
//
const CAStreamBasicDescription &
					AUBase::GetStreamFormat(		AudioUnitScope					inScope,
													AudioUnitElement				inElement)
{
//#warning "aliasing of global scope format should be pushed to subclasses"
	AUIOElement *element;
	
	switch (inScope) {
	case kAudioUnitScope_Input:
		element = Inputs().GetIOElement(inElement);
		break;
	case kAudioUnitScope_Output:
		element = Outputs().GetIOElement(inElement);
		break;
	case kAudioUnitScope_Global:	// global stream description is an alias for that of output 0
		element = Outputs().GetIOElement(0);
		break;
	default:
		COMPONENT_THROW(kAudioUnitErr_InvalidScope);
	}
	return element->GetStreamFormat();
}

OSStatus			AUBase::SetBusCount(	AudioUnitScope					inScope,
											UInt32 							inCount)
{
	if (IsInitialized()) 
		return kAudioUnitErr_Initialized;
		
	GetScope(inScope).SetNumberOfElements(inCount);
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::ChangeStreamFormat(		AudioUnitScope					inScope,
													AudioUnitElement				inElement,
													const CAStreamBasicDescription & inPrevFormat,
													const CAStreamBasicDescription &	inNewFormat)
{
//#warning "aliasing of global scope format should be pushed to subclasses"
	AUIOElement *element;
	
	switch (inScope) {
	case kAudioUnitScope_Input:
		element = Inputs().GetIOElement(inElement);
		break;
	case kAudioUnitScope_Output:
		element = Outputs().GetIOElement(inElement);
		break;
	case kAudioUnitScope_Global:
		element = Outputs().GetIOElement(0);
		break;
	default:
		COMPONENT_THROW(kAudioUnitErr_InvalidScope);
	}
	element->SetStreamFormat(inNewFormat);
	PropertyChanged(kAudioUnitProperty_StreamFormat, inScope, inElement);
	return noErr;
}

UInt32		AUBase::GetChannelLayoutTags(	AudioUnitScope				inScope,
											AudioUnitElement 			inElement,
											AudioChannelLayoutTag *		outLayoutTags)
{
	return GetIOElement(inScope, inElement)->GetChannelLayoutTags(outLayoutTags);
}

UInt32		AUBase::GetAudioChannelLayout(	AudioUnitScope				scope,
											AudioUnitElement 			element,
											AudioChannelLayout *		outLayoutPtr,
											Boolean &					outWritable)
{
	AUIOElement * el = GetIOElement(scope, element);
	return el->GetAudioChannelLayout(outLayoutPtr, outWritable);
}

OSStatus	AUBase::RemoveAudioChannelLayout(			AudioUnitScope				inScope,
														AudioUnitElement			inElement)
{
	OSStatus result = noErr;
	AUIOElement * el = GetIOElement(inScope, inElement);
	Boolean writable;
	if (el->GetAudioChannelLayout(NULL, writable)) {
		result = el->RemoveAudioChannelLayout();
	}
	return result;
}

OSStatus 	AUBase::SetAudioChannelLayout(				AudioUnitScope 				inScope, 
														AudioUnitElement 			inElement,
														const AudioChannelLayout *	inLayout)
{
	AUIOElement* ioEl = GetIOElement (inScope, inElement);

	// the num channels of the layout HAS TO MATCH the current channels of the Element's stream format
	UInt32 currentChannels = ioEl->GetStreamFormat().NumberChannels();
	UInt32 numChannelsInLayout = CAAudioChannelLayout::NumberChannels(*inLayout);
	if (currentChannels != numChannelsInLayout)
		return kAudioUnitErr_InvalidPropertyValue;

	UInt32 numLayouts = GetChannelLayoutTags (inScope, inElement, NULL);
	if (numLayouts == 0)
		return kAudioUnitErr_InvalidProperty;
	AudioChannelLayoutTag *tags = (AudioChannelLayoutTag *)CA_malloc (numLayouts * sizeof (AudioChannelLayoutTag));
	GetChannelLayoutTags (inScope, inElement, tags);
	bool foundTag = false;
	for (unsigned int i = 0; i < numLayouts; ++i) {
		if (tags[i] == inLayout->mChannelLayoutTag || tags[i] == kAudioChannelLayoutTag_UseChannelDescriptions) {
			foundTag = true;
			break;
		}
	}
	free(tags);
	
	if (foundTag == false)
		return kAudioUnitErr_InvalidPropertyValue;

	return ioEl->SetAudioChannelLayout(*inLayout);
}

static void AddNumToDictionary (CFMutableDictionaryRef dict, CFStringRef key, SInt32 value)
{
	CFNumberRef num = CFNumberCreate (NULL, kCFNumberSInt32Type, &value);
	CFDictionarySetValue (dict, key, num);
	CFRelease (num);
}

#define kCurrentSavedStateVersion 0

OSStatus			AUBase::SaveState(		CFPropertyListRef * outData)
{
	AudioComponentDescription desc = GetComponentDescription();

	CFMutableDictionaryRef dict = CFDictionaryCreateMutable	(NULL, 0, 
								&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);

// first step -> save the version to the data ref
	SInt32 value = kCurrentSavedStateVersion;
	AddNumToDictionary (dict, kVersionString, value);

// second step -> save the component type, subtype, manu to the data ref
	value = desc.componentType;
	AddNumToDictionary (dict, kTypeString, value);

	value = desc.componentSubType;
	AddNumToDictionary (dict, kSubtypeString, value);
	
	value = desc.componentManufacturer;
	AddNumToDictionary (dict, kManufacturerString, value);
	
// fourth step -> save the state of all parameters on all scopes and elements
	CFMutableDataRef data = CFDataCreateMutable(NULL, 0);
	for (AudioUnitScope iscope = 0; iscope < 3; ++iscope) {
		AUScope &scope = GetScope(iscope);
		scope.SaveState (data);
	}
    
    SaveExtendedScopes(data);

// save all this in the data section of the dictionary
	CFDictionarySetValue(dict, kDataString, data);
	CFRelease (data);
	
//OK - now we're going to do some properties 	
//save the preset name...
	CFDictionarySetValue (dict, kNameString, mCurrentPreset.presetName);

// Does the unit support the RenderQuality property - if so, save it...
	value = 0;
	OSStatus result = DispatchGetProperty (kAudioUnitProperty_RenderQuality,
								kAudioUnitScope_Global,
								0,
								&value);
	
	if (result == noErr) {
		AddNumToDictionary (dict, kRenderQualityString, value);
	}
	
// Does the unit support the CPULoad Quality property - if so, save it...
	Float32 cpuLoad;
	result = DispatchGetProperty (6/*kAudioUnitProperty_CPULoad*/,
								kAudioUnitScope_Global,
								0,
								&cpuLoad);
	
	if (result == noErr) {
		CFNumberRef num = CFNumberCreate (NULL, kCFNumberFloatType, &cpuLoad);
		CFDictionarySetValue (dict, kCPULoadString, num);
		CFRelease (num);
	}

// Do we have any element names for any of our scopes?	
	// first check to see if we have any names...
	bool foundName = false;
	for (AudioUnitScope i = 0; i < kNumScopes; ++i) {
		foundName = GetScope (i).HasElementWithName();
		if (foundName) 
			break;
	}
		// OK - we found a name away we go...
	if (foundName) {
		CFMutableDictionaryRef nameDict = CFDictionaryCreateMutable	(NULL, 0, 
								&kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
		for (AudioUnitScope i = 0; i < kNumScopes; ++i) {
			GetScope (i).AddElementNamesToDict (nameDict);
		}
		
		CFDictionarySetValue (dict, kElementNameString, nameDict);
		CFRelease (nameDict);
	}
	
// we're done!!!
	*outData = dict;

	return noErr;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::RestoreState(	CFPropertyListRef	plist)
{
	if (CFGetTypeID(plist) != CFDictionaryGetTypeID()) return kAudioUnitErr_InvalidPropertyValue;
	
	AudioComponentDescription desc = GetComponentDescription();
	
	CFDictionaryRef dict = static_cast<CFDictionaryRef>(plist);

// zeroeth step - make sure the Part key is NOT present, as this method is used
// to restore the GLOBAL state of the dictionary	
	if (CFDictionaryContainsKey (dict, kPartString))
		return kAudioUnitErr_InvalidPropertyValue;
		
// first step -> check the saved version in the data ref
// at this point we're only dealing with version==0
	CFNumberRef cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue (dict, kVersionString));
	if (cfnum == NULL) return kAudioUnitErr_InvalidPropertyValue;
	SInt32 value;
	CFNumberGetValue (cfnum, kCFNumberSInt32Type, &value);
	if (value != kCurrentSavedStateVersion) return kAudioUnitErr_InvalidPropertyValue;

// second step -> check that this data belongs to this kind of audio unit
// by checking the component subtype and manuID
// We're not checking the type, since there may be different versions (effect, format-converter, offline)
// of essentially the same AU
	cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue (dict, kSubtypeString));
	if (cfnum == NULL) return kAudioUnitErr_InvalidPropertyValue;
	CFNumberGetValue (cfnum, kCFNumberSInt32Type, &value);
	if (UInt32(value) != desc.componentSubType) return kAudioUnitErr_InvalidPropertyValue;

	cfnum = reinterpret_cast<CFNumberRef>(CFDictionaryGetValue (dict, kManufacturerString));
	if (cfnum == NULL) return kAudioUnitErr_InvalidPropertyValue;
	CFNumberGetValue (cfnum, kCFNumberSInt32Type, &value);
	if (UInt32(value) != desc.componentManufacturer) return kAudioUnitErr_InvalidPropertyValue;

// fourth step -> restore the state of all of the parameters for each scope and element	
	CFDataRef data = reinterpret_cast<CFDataRef>(CFDictionaryGetValue (dict, kDataString));
	if (data != NULL) 
	{
		const UInt8 *p, *pend;
		
		p = CFDataGetBytePtr(data);
		pend = p + CFDataGetLength(data);
		
		// we have a zero length data, which may just mean there were no parameters to save!
		//	if (p >= pend) return noErr; 
	
		while (p < pend) {
            UInt32 scopeIdx = CFSwapInt32BigToHost(*(UInt32 *)p);
            p += sizeof(UInt32);
            
			AUScope &scope = GetScope(scopeIdx);
            p = scope.RestoreState(p);
        }
	}

//OK - now we're going to do some properties
//restore the preset name...
	CFStringRef name = reinterpret_cast<CFStringRef>(CFDictionaryGetValue (dict, kNameString));
	if (mCurrentPreset.presetName) CFRelease (mCurrentPreset.presetName);
	if (name)
	{
		mCurrentPreset.presetName = name;
		mCurrentPreset.presetNumber = -1;
	} 
	else { // no name entry make the default one
		mCurrentPreset.presetName = kUntitledString;
		mCurrentPreset.presetNumber = -1;
	}
	
	CFRetain (mCurrentPreset.presetName);
#if !CA_USE_AUDIO_PLUGIN_ONLY
#ifndef __LP64__
	PropertyChanged(kAudioUnitProperty_CurrentPreset, kAudioUnitScope_Global, 0);
#endif
#endif
	PropertyChanged(kAudioUnitProperty_PresentPreset, kAudioUnitScope_Global, 0);

// Does the dict contain render quality information?		
	if (CFDictionaryGetValueIfPresent (dict, kRenderQualityString, reinterpret_cast<const void**>(&cfnum))) 
	{
		CFNumberGetValue (cfnum, kCFNumberSInt32Type, &value);
		DispatchSetProperty (kAudioUnitProperty_RenderQuality,
								kAudioUnitScope_Global,
								0,
								&value,
								sizeof(value));
	}
	
// Does the unit support the CPULoad Quality property - if so, save it...
	if (CFDictionaryGetValueIfPresent (dict, kCPULoadString, reinterpret_cast<const void**>(&cfnum))) 
	{
		Float32 floatValue;
		CFNumberGetValue (cfnum, kCFNumberFloatType, &floatValue);
		DispatchSetProperty (6/*kAudioUnitProperty_CPULoad*/,
								kAudioUnitScope_Global,
								0,
								&floatValue,
								sizeof(floatValue));
	}

// Do we have any element names for any of our scopes?
	CFDictionaryRef nameDict;
	if (CFDictionaryGetValueIfPresent (dict, kElementNameString, reinterpret_cast<const void**>(&nameDict))) 
	{
		char string[64];
		for (int i = 0; i < kNumScopes; ++i) 
		{
			snprintf (string, sizeof(string), "%d", i);
			CFStringRef key = CFStringCreateWithCString (NULL, string, kCFStringEncodingASCII);
			CFDictionaryRef elementDict;
			if (CFDictionaryGetValueIfPresent (nameDict, key, reinterpret_cast<const void**>(&elementDict))) 
			{
				bool didAddElements = GetScope (i).RestoreElementNames (elementDict);
				if (didAddElements)
					PropertyChanged (kAudioUnitProperty_ElementCount, i, 0);
			}
			CFRelease (key);				
		}
	}
	
	return noErr;
}

OSStatus			AUBase::GetPresets (			CFArrayRef * 					outData) const
{
	return kAudioUnitErr_InvalidProperty;
}

OSStatus			AUBase::NewFactoryPresetSet (const AUPreset & inNewFactoryPreset)
{
	return kAudioUnitErr_InvalidProperty;
}

OSStatus            AUBase::NewCustomPresetSet (const AUPreset & inNewCustomPreset)
{
    CFRelease (mCurrentPreset.presetName);
    mCurrentPreset = inNewCustomPreset;
    CFRetain (mCurrentPreset.presetName);
    return noErr;
}

		// set the default preset for the unit -> the number of the preset MUST be >= 0
		// and the name should be valid, or the preset WON'T take
bool				AUBase::SetAFactoryPresetAsCurrent (const AUPreset & inPreset)
{
	if (inPreset.presetNumber < 0 || inPreset.presetName == NULL) return false;
	CFRelease (mCurrentPreset.presetName);
	mCurrentPreset = inPreset;
	CFRetain (mCurrentPreset.presetName);
	return true;
}

#if !CA_USE_AUDIO_PLUGIN_ONLY
int			AUBase::GetNumCustomUIComponents () 
{
	return 0;
}

void		AUBase::GetUIComponentDescs (ComponentDescription* inDescArray) {}
#endif
	
bool		AUBase::HasIcon () 
{
#if !CA_NO_AU_UI_FEATURES
	CFURLRef url = CopyIconLocation(); 
	if (url) {
		CFRelease (url);
		return true;
	}
#endif
	return false;
}

CFURLRef	AUBase::CopyIconLocation () 
{
	return NULL;
}
	
//_____________________________________________________________________________
//
OSStatus			AUBase::GetParameterList(		AudioUnitScope					inScope,
													AudioUnitParameterID *			outParameterList,
													UInt32 &						outNumParameters)
{
	AUScope &scope = GetScope(inScope);
	AUElement *elementWithMostParameters = NULL;
	UInt32 maxNumParams = 0;
	
	int nElems = scope.GetNumberOfElements();
	for (int ielem = 0; ielem < nElems; ++ielem) {
		AUElement *element = scope.GetElement(ielem);
		UInt32 nParams = element->GetNumberOfParameters();
		if (nParams > maxNumParams) {
			maxNumParams = nParams;
			elementWithMostParameters = element;
		}
	}
	
	if (outParameterList != NULL && elementWithMostParameters != NULL)
		elementWithMostParameters->GetParameterList(outParameterList);
	
	outNumParameters = maxNumParams;
	return noErr;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::GetParameterInfo(		AudioUnitScope			inScope,
													AudioUnitParameterID	inParameterID,
													AudioUnitParameterInfo	&outParameterInfo )
{
	return kAudioUnitErr_InvalidParameter;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::GetParameterValueStrings(AudioUnitScope			inScope,
													AudioUnitParameterID	inParameterID,
													CFArrayRef *			outStrings)
{
	return kAudioUnitErr_InvalidProperty;
}

//_____________________________________________________________________________
//
OSStatus			AUBase::GetParameterHistoryInfo(	AudioUnitScope					inScope,
														AudioUnitParameterID			inParameterID,
														Float32 &						outUpdatesPerSecond,
														Float32 &						outHistoryDurationInSeconds)
{
	return kAudioUnitErr_InvalidProperty;
}


//_____________________________________________________________________________
//
OSStatus			AUBase::CopyClumpName(			AudioUnitScope			inScope, 
													UInt32					inClumpID, 
													UInt32					inDesiredNameLength,
													CFStringRef *			outClumpName)
{
	return kAudioUnitErr_InvalidProperty;
}

//_____________________________________________________________________________
//
void				AUBase::SetNumberOfElements(	AudioUnitScope					inScope,
													UInt32							numElements)
{
	if (inScope == kAudioUnitScope_Global && numElements != 1)
		COMPONENT_THROW(kAudioUnitErr_InvalidScope);

	GetScope(inScope).SetNumberOfElements(numElements);
}

//_____________________________________________________________________________
//
AUElement *			AUBase::CreateElement(			AudioUnitScope					scope,
													AudioUnitElement				element)
{
	switch (scope) {
	case kAudioUnitScope_Global:
		return new AUElement(this);
	case kAudioUnitScope_Input:
		return new AUInputElement(this);
	case kAudioUnitScope_Output:
		return new AUOutputElement(this);
#if !CA_BASIC_AU_FEATURES
	case kAudioUnitScope_Group:
		return new AUElement(this);
	case kAudioUnitScope_Part:
		return new AUElement(this);
#endif
	}
	COMPONENT_THROW(kAudioUnitErr_InvalidScope);
	
	return NULL;	// get rid of compiler warning
}

//_____________________________________________________________________________
//
bool	AUBase::FormatIsCanonical(		const CAStreamBasicDescription &f)
{
	return (f.mFormatID == kAudioFormatLinearPCM
		&&	f.mFramesPerPacket == 1
		&&	f.mBytesPerPacket == f.mBytesPerFrame
//		&&	f.mChannelsPerFrame >= 0	-- this is always true since it's unsigned
		// so far, it's a valid PCM format
#if CA_PREFER_FIXED_POINT
		&&	(f.mFormatFlags & kLinearPCMFormatFlagIsFloat) == 0
		&&	(((f.mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >> kLinearPCMFormatFlagsSampleFractionShift) == kAudioUnitSampleFractionBits)
#else
		&&	(f.mFormatFlags & kLinearPCMFormatFlagIsFloat) != 0
#endif
		&&	((f.mChannelsPerFrame == 1) || ((f.mFormatFlags & kAudioFormatFlagIsNonInterleaved) == 0) == (mAudioUnitAPIVersion == 1))
#if TARGET_RT_BIG_ENDIAN
		&&	(f.mFormatFlags & kLinearPCMFormatFlagIsBigEndian) != 0
#else
		&&	(f.mFormatFlags & kLinearPCMFormatFlagIsBigEndian) == 0
#endif
		&&	f.mBitsPerChannel == 8 * sizeof(AudioUnitSampleType)
		&&	f.mBytesPerFrame == f.NumberInterleavedChannels() * sizeof(AudioUnitSampleType)
		);
}

//_____________________________________________________________________________
//
void	AUBase::MakeCanonicalFormat(	CAStreamBasicDescription &		f,
										int								nChannels)
{
	f.SetAUCanonical(nChannels, mAudioUnitAPIVersion < 2);	// interleaved for v1, non for v2
	f.mSampleRate = 0.0;
}

const Float64 AUBase::kNoLastRenderedSampleTime = -1.;

#include "AUBaseHelper.h"

char*	AUBase::GetLoggingString () const
{
	if (mLogString) return mLogString;
	
	AudioComponentDescription desc = GetComponentDescription();
	
	const size_t logStringSize = 256;
	const_cast<AUBase*>(this)->mLogString = new char[logStringSize];
	char str[24];
	char str1[24];
	char str2[24];
	snprintf (const_cast<AUBase*>(this)->mLogString, logStringSize, "AU (%p): %s %s %s",
		GetComponentInstance(),
		CAStringForOSType(desc.componentType, str, sizeof(str)),
		CAStringForOSType(desc.componentSubType, str1, sizeof(str1)),
		CAStringForOSType(desc.componentManufacturer, str2, sizeof(str2)));

	return mLogString;
}


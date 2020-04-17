/*
     File: AUDispatch.cpp
 Abstract: AUDispatch.h
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
#include "CAXException.h"
#include "AUDispatch.h"



#if TARGET_OS_MAC
	#if __LP64__
		// comp instance, parameters in forward order
		#define PARAM(_typ, _name, _index, _nparams) \
			_typ _name = *(_typ *)&params->params[_index + 1];
	#else
		// parameters in reverse order, then comp instance
		#define PARAM(_typ, _name, _index, _nparams) \
			_typ _name = *(_typ *)&params->params[_nparams - 1 - _index];
	#endif
#elif TARGET_OS_WIN32
		// (no comp instance), parameters in forward order
		#define PARAM(_typ, _name, _index, _nparams) \
			_typ _name = *(_typ *)&params->params[_index];
#endif


OSStatus		AUBase::ComponentEntryDispatch(ComponentParameters *params, AUBase *This)
{
	if (This == NULL) return kAudio_ParamError;

	OSStatus result = noErr;

	switch (params->what) {
	case kComponentCanDoSelect:
		switch (GetSelectorForCanDo(params)) {
	// any selectors
			case kAudioUnitInitializeSelect:
			case kAudioUnitUninitializeSelect:
			case kAudioUnitGetPropertyInfoSelect:
			case kAudioUnitGetPropertySelect:
			case kAudioUnitSetPropertySelect:
			case kAudioUnitAddPropertyListenerSelect:
#if (!__LP64__)
			case kAudioUnitRemovePropertyListenerSelect:
#endif
			case kAudioUnitGetParameterSelect:
			case kAudioUnitSetParameterSelect:
			case kAudioUnitResetSelect:
				result = 1;
				break;
	// v1 selectors

	// v2 selectors
			case kAudioUnitRemovePropertyListenerWithUserDataSelect:
			case kAudioUnitAddRenderNotifySelect:
			case kAudioUnitRemoveRenderNotifySelect:
			case kAudioUnitScheduleParametersSelect:
			case kAudioUnitRenderSelect:
				result = (This->AudioUnitAPIVersion() > 1);
				break;
				
			default:
				return ComponentBase::ComponentEntryDispatch(params, This);
		}
		break;
		
	case kAudioUnitInitializeSelect:
	{
		CAMutex::Locker lock2(This->GetMutex());
		result = This->DoInitialize();
	}
		break;
		
	case kAudioUnitUninitializeSelect:
	{
		CAMutex::Locker lock2(This->GetMutex());
		This->DoCleanup();
		result = noErr;
	}
		break;

	case kAudioUnitGetPropertyInfoSelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AudioUnitPropertyID, pinID, 0, 5);
			PARAM(AudioUnitScope, pinScope, 1, 5);
			PARAM(AudioUnitElement, pinElement, 2, 5);
			PARAM(UInt32 *, poutDataSize, 3, 5);
			PARAM(Boolean *, poutWritable, 4, 5);

			// pass our own copies so that we assume responsibility for testing
			// the caller's pointers against null and our C++ classes can
			// always assume they're non-null
			UInt32 dataSize;
			Boolean writable;
			
			result = This->DispatchGetPropertyInfo(pinID, pinScope, pinElement, dataSize, writable);
			if (poutDataSize != NULL)
				*poutDataSize = dataSize;
			if (poutWritable != NULL)
				*poutWritable = writable;
		}
		break;

	case kAudioUnitGetPropertySelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AudioUnitPropertyID, pinID, 0, 5);
			PARAM(AudioUnitScope, pinScope, 1, 5);
			PARAM(AudioUnitElement, pinElement, 2, 5);
			PARAM(void *, poutData, 3, 5);
			PARAM(UInt32 *, pioDataSize, 4, 5);

			UInt32 actualPropertySize, clientBufferSize;
			Boolean writable;
			char *tempBuffer;
			void *destBuffer;
			
			if (pioDataSize == NULL) {
				ca_debug_string("AudioUnitGetProperty: null size pointer");
				result = kAudio_ParamError;
				goto finishGetProperty;
			}
			if (poutData == NULL) {
				UInt32 dataSize;
				
				result = This->DispatchGetPropertyInfo(pinID, pinScope, pinElement, dataSize, writable);
				*pioDataSize = dataSize;
				goto finishGetProperty;
			}
			
			clientBufferSize = *pioDataSize;
			if (clientBufferSize == 0)
			{
				ca_debug_string("AudioUnitGetProperty: *ioDataSize == 0 on entry");
				// $$$ or should we allow this as a shortcut for finding the size?
				result = kAudio_ParamError;
				goto finishGetProperty;
			}
			
			result = This->DispatchGetPropertyInfo(pinID, pinScope, pinElement, 
													actualPropertySize, writable);
			if (result) 
				goto finishGetProperty;
			
			if (clientBufferSize < actualPropertySize) 
			{
				tempBuffer = new char[actualPropertySize];
				destBuffer = tempBuffer;
			} else {
				tempBuffer = NULL;
				destBuffer = poutData;
			}
			
			result = This->DispatchGetProperty(pinID, pinScope, pinElement, destBuffer);
			
			if (result == noErr) {
				if (clientBufferSize < actualPropertySize && tempBuffer != NULL)
				{
					memcpy(poutData, tempBuffer, clientBufferSize);
					delete[] tempBuffer;
					// pioDataSize remains correct, the number of bytes we wrote
				} else
					*pioDataSize = actualPropertySize;
			} else
				*pioDataSize = 0;

			finishGetProperty:
				;

		}
		break;
		
	case kAudioUnitSetPropertySelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AudioUnitPropertyID, pinID, 0, 5);
			PARAM(AudioUnitScope, pinScope, 1, 5);
			PARAM(AudioUnitElement, pinElement, 2, 5);
			PARAM(const void *, pinData, 3, 5);
			PARAM(UInt32, pinDataSize, 4, 5);
			
			if (pinData && pinDataSize)
				result = This->DispatchSetProperty(pinID, pinScope, pinElement, pinData, pinDataSize);
			else {
				if (pinData == NULL && pinDataSize == 0) {
					result = This->DispatchRemovePropertyValue (pinID, pinScope, pinElement);
				} else {
					if (pinData == NULL) {
						ca_debug_string("AudioUnitSetProperty: inData == NULL");
						result = kAudio_ParamError;
						goto finishSetProperty;
					}

					if (pinDataSize == 0) {
						ca_debug_string("AudioUnitSetProperty: inDataSize == 0");
						result = kAudio_ParamError;
						goto finishSetProperty;
					}
				}
			}
			finishSetProperty:
					;

		}
		break;
		
	case kAudioUnitAddPropertyListenerSelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AudioUnitPropertyID, pinID, 0, 3);
			PARAM(AudioUnitPropertyListenerProc, pinProc, 1, 3);
			PARAM(void *, pinProcRefCon, 2, 3);
			result = This->AddPropertyListener(pinID, pinProc, pinProcRefCon);
		}
		break;

#if (!__LP64__)
	case kAudioUnitRemovePropertyListenerSelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AudioUnitPropertyID, pinID, 0, 2);
			PARAM(AudioUnitPropertyListenerProc, pinProc, 1, 2);
			result = This->RemovePropertyListener(pinID, pinProc, NULL, false);
		}
		break;
#endif

	case kAudioUnitRemovePropertyListenerWithUserDataSelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AudioUnitPropertyID, pinID, 0, 3);
			PARAM(AudioUnitPropertyListenerProc, pinProc, 1, 3);
			PARAM(void *, pinProcRefCon, 2, 3);
			result = This->RemovePropertyListener(pinID, pinProc, pinProcRefCon, true);
		}
		break;
		
	case kAudioUnitAddRenderNotifySelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AURenderCallback, pinProc, 0, 2);
			PARAM(void *, pinProcRefCon, 1, 2);
			result = This->SetRenderNotification (pinProc, pinProcRefCon);
		}
		break;

	case kAudioUnitRemoveRenderNotifySelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AURenderCallback, pinProc, 0, 2);
			PARAM(void *, pinProcRefCon, 1, 2);
			result = This->RemoveRenderNotification (pinProc, pinProcRefCon);
		}
		break;

	case kAudioUnitGetParameterSelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AudioUnitParameterID, pinID, 0, 4);
			PARAM(AudioUnitScope, pinScope, 1, 4);
			PARAM(AudioUnitElement, pinElement, 2, 4);
			PARAM(AudioUnitParameterValue *, poutValue, 3, 4);
			result = (poutValue == NULL ? kAudio_ParamError : This->GetParameter(pinID, pinScope, pinElement, *poutValue));
		}
		break;

	case kAudioUnitSetParameterSelect:
		{
			CAMutex::Locker lock(This->GetMutex()); // is this realtime or no???
			PARAM(AudioUnitParameterID, pinID, 0, 5);
			PARAM(AudioUnitScope, pinScope, 1, 5);
			PARAM(AudioUnitElement, pinElement, 2, 5);
			PARAM(AudioUnitParameterValue, pinValue, 3, 5);
			PARAM(UInt32, pinBufferOffsetInFrames, 4, 5);
			result = This->SetParameter(pinID, pinScope, pinElement, pinValue, pinBufferOffsetInFrames);
		}
		break;

	case kAudioUnitScheduleParametersSelect:
		{
			CAMutex::Locker lock(This->GetMutex()); // is this realtime or no???
			if (This->AudioUnitAPIVersion() > 1)
			{
				PARAM(AudioUnitParameterEvent *, pinParameterEvent, 0, 2);
				PARAM(UInt32, pinNumParamEvents, 1, 2);
				result = This->ScheduleParameter (pinParameterEvent, pinNumParamEvents);
			} else
				result = badComponentSelector;
		}
		break;


	case kAudioUnitRenderSelect:
		{
			// realtime; no lock
			{
				PARAM(AudioUnitRenderActionFlags *, pinActionFlags, 0, 5);
				PARAM(const AudioTimeStamp *, pinTimeStamp, 1, 5);
				PARAM(UInt32, pinOutputBusNumber, 2, 5);
				PARAM(UInt32, pinNumberFrames, 3, 5);
				PARAM(AudioBufferList *, pioData, 4, 5);
				AudioUnitRenderActionFlags tempFlags;
				
				if (pinTimeStamp == NULL || pioData == NULL)
					result = kAudio_ParamError;
				else {
					if (pinActionFlags == NULL) {
						tempFlags = 0;
						pinActionFlags = &tempFlags;
					}
					result = This->DoRender(*pinActionFlags, *pinTimeStamp, pinOutputBusNumber, pinNumberFrames, *pioData);
				}
			}			
		}
		break;

	case kAudioUnitResetSelect:
		{
			CAMutex::Locker lock(This->GetMutex());
			PARAM(AudioUnitScope, pinScope, 0, 2);
			PARAM(AudioUnitElement, pinElement, 1, 2);
			This->ResetRenderTime();
			result = This->Reset(pinScope, pinElement);
		}
		break;

	default:
		result = ComponentBase::ComponentEntryDispatch(params, This);
		break;
	}

	return result;
}

// Fast dispatch entry points -- these need to replicate all error-checking logic from above

OSStatus CMgr_AudioUnitBaseGetParameter(	AUBase *				This,
											AudioUnitParameterID	inID,
											AudioUnitScope			inScope,
											AudioUnitElement		inElement,
											float					*outValue)
{
	OSStatus result = AUBase::noErr;
	
	try {
		if (This == NULL || outValue == NULL) return kAudio_ParamError;
		result = This->GetParameter(inID, inScope, inElement, *outValue);
	}
	COMPONENT_CATCH
	
	return result;
}

OSStatus CMgr_AudioUnitBaseSetParameter(	AUBase * 				This,
											AudioUnitParameterID	inID,
											AudioUnitScope			inScope,
											AudioUnitElement		inElement,
											float					inValue,
											UInt32					inBufferOffset)
{
	OSStatus result = AUBase::noErr;
	
	try {
		if (This == NULL) return kAudio_ParamError;
		result = This->SetParameter(inID, inScope, inElement, inValue, inBufferOffset);
	}
	COMPONENT_CATCH
	
	return result;
}

OSStatus CMgr_AudioUnitBaseRender(			AUBase *				This,
											AudioUnitRenderActionFlags *ioActionFlags,
											const AudioTimeStamp *	inTimeStamp,
											UInt32					inBusNumber,
											UInt32					inNumberFrames,
											AudioBufferList *		ioData)
{
	if (inTimeStamp == NULL || ioData == NULL) return kAudio_ParamError;
	
	OSStatus result = AUBase::noErr;
	AudioUnitRenderActionFlags tempFlags;
	
	try {
		if (ioActionFlags == NULL) {
			tempFlags = 0;
			ioActionFlags = &tempFlags;
		}
		result = This->DoRender(*ioActionFlags, *inTimeStamp, inBusNumber, inNumberFrames, *ioData);
	}
	COMPONENT_CATCH
	
	return result;
}

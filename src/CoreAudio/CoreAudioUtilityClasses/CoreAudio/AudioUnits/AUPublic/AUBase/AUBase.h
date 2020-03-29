/*
     File: AUBase.h
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
#ifndef __AUBase_h__
#define __AUBase_h__

#include <TargetConditionals.h>

#if TARGET_OS_MAC
	#include <pthread.h>
#elif TARGET_OS_WIN32
	#include <windows.h>
#else
	#error Unsupported Operating System
#endif

#include <vector>

#include "AUScopeElement.h"
#include "AUInputElement.h"
#include "AUOutputElement.h"
#include "AUBuffer.h"
#include "CAMath.h"
#include "CAThreadSafeList.h"
#include "CAVectorUnit.h"
#include "CAMutex.h"
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioUnit/AudioUnit.h>
	#if !CA_BASIC_AU_FEATURES
		#include <AudioUnit/MusicDevice.h>
	#endif
#else
	#include "AudioUnit.h"
	#if !CA_BASIC_AU_FEATURES
		#include "MusicDevice.h"
	#endif
#endif

#ifndef AUTRACE
	#define AUTRACE(code, obj, a, b, c, d)
#endif	

#include "AUPlugInDispatch.h"



// ________________________________________________________________________
// These are to be moved to the public AudioUnit headers

#define kAUDefaultSampleRate		44100.0
#if !TARGET_OS_WIN32
#define kAUDefaultMaxFramesPerSlice	1156 
//this allows enough default frames for a 512 dest 44K and SRC from 96K
// add a padding of 4 frames for any altivec rounding
#else
#define kAUDefaultMaxFramesPerSlice	2048 
#endif

// ________________________________________________________________________

/*! @class AUBase */
class AUBase : public ComponentBase {
public:

	/*! @ctor AUBase */
								AUBase(					AudioComponentInstance			inInstance, 
														UInt32							numInputElements,
														UInt32							numOutputElements,
														UInt32							numGroupElements = 0);
	/*! @dtor AUBase */
	virtual						~AUBase();
	
	/*! @method PostConstructor */
	virtual void				PostConstructor() { CreateElements(); }
								
	/*! @method PreDestructor */
	virtual void				PreDestructor();

	/*! @method CreateElements */
	void						CreateElements();
									// Called immediately after construction, when virtual methods work.
									// Or, a subclass may call this in order to have access to elements
									// in its constructor.

	/*! @method CreateExtendedElements */
	virtual void CreateExtendedElements() {}

#pragma mark -
#pragma mark AU dispatch
	// ________________________________________________________________________
	// Virtual methods (mostly) directly corresponding to the entry points.  Many of these
	// have useful implementations here and will not need overriding.
	
	/*! @method DoInitialize */
			OSStatus			DoInitialize();
				// this implements the entry point and makes sure that initialization
				// is only attempted exactly once...

	/*! @method Initialize */
	virtual OSStatus			Initialize();
				// ... so that overrides to this method can assume that they will only
				// be called exactly once.
	
	/*! @method IsInitialized */
			bool				IsInitialized() const { return mInitialized; }
	/*! @method HasBegunInitializing */
			bool				HasBegunInitializing() const { return mHasBegunInitializing; }
			
	/*! @method DoCleanup */
			void				DoCleanup();
				// same pattern as with Initialize
	
	/*! @method Cleanup */
	virtual void				Cleanup();

	/*! @method Reset */
	virtual OSStatus			Reset(					AudioUnitScope 					inScope,
														AudioUnitElement 				inElement);

	// Note about GetPropertyInfo, GetProperty, SetProperty:
	// Certain properties are trapped out in these dispatch functions and handled with different virtual 
	// methods.  (To discourage hacks and keep vtable size down, these are non-virtual)

	/*! @method DispatchGetPropertyInfo */
			OSStatus			DispatchGetPropertyInfo(AudioUnitPropertyID				inID,
														AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														UInt32 &						outDataSize,
														Boolean &						outWritable);

	/*! @method DispatchGetProperty */
			OSStatus			DispatchGetProperty(	AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement,
														void *							outData);

	/*! @method DispatchSetProperty */
			OSStatus			DispatchSetProperty(	AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														const void *					inData,
														UInt32 							inDataSize);
													
			OSStatus			DispatchRemovePropertyValue(	AudioUnitPropertyID 	inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement);

	/*! @method GetPropertyInfo */
	virtual OSStatus			GetPropertyInfo(		AudioUnitPropertyID				inID,
														AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														UInt32 &						outDataSize,
														Boolean &						outWritable);

	/*! @method GetProperty */
	virtual OSStatus			GetProperty(			AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement,
														void *							outData);

	/*! @method SetProperty */
	virtual OSStatus			SetProperty(			AudioUnitPropertyID 			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														const void *					inData,
														UInt32 							inDataSize);
													
	/*! @method ClearPropertyUsage */
	virtual OSStatus			RemovePropertyValue (	AudioUnitPropertyID		 		inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement);

	/*! @method AddPropertyListener */
	virtual OSStatus			AddPropertyListener(	AudioUnitPropertyID				inID,
														AudioUnitPropertyListenerProc	inProc,
														void *							inProcRefCon);
														
	/*! @method RemovePropertyListener */
	virtual OSStatus			RemovePropertyListener(	AudioUnitPropertyID				inID,
														AudioUnitPropertyListenerProc	inProc,
														void *							inProcRefCon,
														bool							refConSpecified);
	
	/*! @method SetRenderNotification */
	virtual OSStatus			SetRenderNotification(	AURenderCallback		 		inProc,
														void *							inRefCon);
	
	/*! @method RemoveRenderNotification */
	virtual OSStatus			RemoveRenderNotification(
														AURenderCallback		 		inProc,
														void *							inRefCon);
	
	/*! @method GetParameter */
	virtual OSStatus 	GetParameter(			AudioUnitParameterID			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														AudioUnitParameterValue &		outValue);
												
	/*! @method SetParameter */
	virtual OSStatus 	SetParameter(			AudioUnitParameterID			inID,
														AudioUnitScope 					inScope,
														AudioUnitElement 				inElement,
														AudioUnitParameterValue			inValue,
														UInt32							inBufferOffsetInFrames);

	/*! @method CanScheduleParams */
	virtual bool CanScheduleParameters() const = 0;

	/*! @method ScheduleParameter */
	virtual OSStatus 	ScheduleParameter (		const AudioUnitParameterEvent 	*inParameterEvent,
														UInt32							inNumEvents);
	

	/*! @method DoRender */
	OSStatus 	DoRender(								AudioUnitRenderActionFlags &	ioActionFlags,
														const AudioTimeStamp &			inTimeStamp,
														UInt32							inBusNumber,
														UInt32							inNumberFrames,
														AudioBufferList &				ioData);
	

	/*! @method Process */
	OSStatus	DoProcess (							AudioUnitRenderActionFlags  &		ioActionFlags,
													const AudioTimeStamp &				inTimeStamp,
													UInt32								inFramesToProcess,
													AudioBufferList &					ioData);
	
	/*! @method ProcessMultiple */
	OSStatus	DoProcessMultiple (					AudioUnitRenderActionFlags  &		ioActionFlags,
													const AudioTimeStamp &				inTimeStamp,
													UInt32								inFramesToProcess,
													UInt32								inNumberInputBufferLists,
													const AudioBufferList **			inInputBufferLists,
													UInt32 								inNumberOutputBufferLists,
													AudioBufferList **					ioOutputBufferLists);
		
	/*! @method ProcessBufferLists */
	virtual OSStatus	ProcessBufferLists(			AudioUnitRenderActionFlags &		ioActionFlags,
													const AudioBufferList &				inBuffer,
													AudioBufferList &					outBuffer,
													UInt32								inFramesToProcess )
						{
							return kAudio_UnimplementedError;
						}

	/*! @method ProcessMultipleBufferLists */
	virtual OSStatus	ProcessMultipleBufferLists(	AudioUnitRenderActionFlags &		ioActionFlags,
													UInt32								inFramesToProcess, 
												    UInt32								inNumberInputBufferLists,
												    const AudioBufferList **			inInputBufferLists,
												    UInt32							 	inNumberOutputBufferLists,
												    AudioBufferList **					ioOutputBufferLists)	
						{
							return kAudio_UnimplementedError;
						}
	
	/*! @method ComplexRender */
	virtual OSStatus	ComplexRender(				AudioUnitRenderActionFlags &		ioActionFlags, 
													const AudioTimeStamp &				inTimeStamp, 
													UInt32								inOutputBusNumber, 
													UInt32								inNumberOfPackets, 
													UInt32 *							outNumberOfPackets, 
													AudioStreamPacketDescription *		outPacketDescriptions, 
													AudioBufferList &					ioData, 
													void *								outMetadata, 
													UInt32 *							outMetadataByteSize) 
						{	
							return kAudio_UnimplementedError;
						}

	// Override this method if your AU processes multiple output busses completely independently --
	// you'll want to just call Render without the NeedsToRender check.
	// Otherwise, override Render().
	//
	// N.B. Implementations of this method can assume that the output's buffer list has already been
	// prepared and access it with GetOutput(inBusNumber)->GetBufferList() instead of 
	// GetOutput(inBusNumber)->PrepareBuffer(nFrames) -- if PrepareBuffer is called, a
	// copy may occur after rendering.
	/*! @method RenderBus */
	virtual OSStatus			RenderBus(				AudioUnitRenderActionFlags &	ioActionFlags,
														const AudioTimeStamp &			inTimeStamp,
														UInt32							inBusNumber,
														UInt32							inNumberFrames)
								{
									if (NeedsToRender(inTimeStamp))
										return Render(ioActionFlags, inTimeStamp, inNumberFrames);
									return noErr;	// was presumably already rendered via another bus
								}

	// N.B. For a unit with only one output bus, it can assume in its implementation of this
	// method that the output's buffer list has already been prepared and access it with 
	// GetOutput(0)->GetBufferList() instead of GetOutput(0)->PrepareBuffer(nFrames)
	//  -- if PrepareBuffer is called, a copy may occur after rendering.
	/*! @method Render */
	virtual OSStatus			Render(					AudioUnitRenderActionFlags &	ioActionFlags,
														const AudioTimeStamp &			inTimeStamp,
														UInt32							inNumberFrames)
								{
									return noErr;
								}

								
#pragma mark -
#pragma mark Property Dispatch

	static const Float64 kNoLastRenderedSampleTime;

	// ________________________________________________________________________
	// These are generated from DispatchGetProperty/DispatchGetPropertyInfo/DispatchSetProperty

	/*! @method BusCountWritable */
	virtual bool				BusCountWritable(		AudioUnitScope					inScope)
								{
									return false;
								}
	virtual OSStatus			SetBusCount(		AudioUnitScope					inScope,
													UInt32							inCount);

	/*! @method SetConnection */
	virtual OSStatus			SetConnection(			const AudioUnitConnection &		inConnection);
	
	/*! @method SetInputCallback */
	virtual OSStatus			SetInputCallback(		UInt32							inPropertyID,
														AudioUnitElement 				inElement, 
														AURenderCallback				inProc,
														void *							inRefCon);

	/*! @method GetParameterList */
	virtual OSStatus			GetParameterList(		AudioUnitScope					inScope,
														AudioUnitParameterID *			outParameterList,
														UInt32 &						outNumParameters);
															// outParameterList may be a null pointer

	/*! @method GetParameterInfo */
	virtual OSStatus			GetParameterInfo(		AudioUnitScope					inScope,
														AudioUnitParameterID			inParameterID,
														AudioUnitParameterInfo &		outParameterInfo);

	virtual OSStatus			GetParameterHistoryInfo(AudioUnitScope					inScope,
														AudioUnitParameterID			inParameterID,
														Float32 &						outUpdatesPerSecond,
														Float32 &						outHistoryDurationInSeconds);

	/*! @method SaveState */
	virtual OSStatus			SaveState(				CFPropertyListRef *				outData);
    
    /*! @method SaveExtendedScopes */
	virtual void                SaveExtendedScopes(		CFMutableDataRef				outData) {};

	/*! @method RestoreState */
	virtual OSStatus			RestoreState(			CFPropertyListRef				inData);

	/*! @method GetParameterValueStrings */
	virtual OSStatus			GetParameterValueStrings(AudioUnitScope					inScope,
														AudioUnitParameterID			inParameterID,
														CFArrayRef *					outStrings);

	/*! @method CopyClumpName */
	virtual OSStatus			CopyClumpName(			AudioUnitScope					inScope, 
														UInt32							inClumpID, 
														UInt32							inDesiredNameLength,
														CFStringRef *					outClumpName);

	/*! @method GetPresets */
	virtual OSStatus			GetPresets (			CFArrayRef * 					outData) const;

		// set the default preset for the unit -> the number of the preset MUST be >= 0
		// and the name should be valid, or the preset WON'T take
	/*! @method SetAFactoryPresetAsCurrent */
	bool						SetAFactoryPresetAsCurrent (const AUPreset & inPreset);
		
		// Called when someone sets a new, valid preset
		// If this is a valid preset, then the subclass sets its state to that preset
		// and returns noErr.
		// If not a valid preset, return an error, and the pre-existing preset is restored
	/*! @method NewFactoryPresetSet */
	virtual OSStatus			NewFactoryPresetSet (const AUPreset & inNewFactoryPreset);
	
	/*! @method NewCustomPresetSet */
	virtual OSStatus            NewCustomPresetSet (const AUPreset & inNewCustomPreset);

#if !CA_USE_AUDIO_PLUGIN_ONLY
	/*! @method GetNumCustomUIComponents */
	virtual int					GetNumCustomUIComponents ();

	/*! @method GetUIComponentDescs */
	virtual void				GetUIComponentDescs (ComponentDescription* inDescArray);
#endif
	
	/*! @method CopyIconLocation */
	virtual CFURLRef			CopyIconLocation ();
	
	// default is no latency, and unimplemented tail time
	/*! @method GetLatency */
    virtual Float64				GetLatency() {return 0.0;}
	/*! @method GetTailTime */
    virtual Float64				GetTailTime() {return 0;}
	/*! @method SupportsRampAndTail */
	virtual	bool				SupportsTail () { return false; }

	/*! @method IsStreamFormatWritable */
			bool				IsStreamFormatWritable(	AudioUnitScope					scope,
														AudioUnitElement				element);
	
	/*! @method StreamFormatWritable */
	virtual bool				StreamFormatWritable(	AudioUnitScope					scope,
														AudioUnitElement				element) = 0;
															// scope will always be input or output
			
			// pass in a pointer to get the struct, and num channel infos
			// you can pass in NULL to just get the number
			// a return value of 0 (the default in AUBase) means the property is not supported...
	/*! @method SupportedNumChannels */
	virtual UInt32				SupportedNumChannels (	const AUChannelInfo**			outInfo);
																												
	/*! @method ValidFormat */
	virtual bool				ValidFormat(			AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														const CAStreamBasicDescription & inNewFormat);
															// Will only be called after StreamFormatWritable
															// has succeeded.
															// Default implementation requires canonical format:
															// native-endian 32-bit float, any sample rate,
															// any number of channels; override when other
															// formats are supported.  A subclass's override can
															// choose to always return true and trap invalid 
															// formats in ChangeStreamFormat.


	/*! @method FormatIsCanonical */
			bool				FormatIsCanonical(		const CAStreamBasicDescription &format);

	/*! @method MakeCanonicalFormat */
			void				MakeCanonicalFormat(	CAStreamBasicDescription &	outDesc,
														int							numChannels = 2);

	/*! @method GetStreamFormat */
	virtual const CAStreamBasicDescription &
								GetStreamFormat(		AudioUnitScope					inScope,
														AudioUnitElement				inElement);

	/*! @method ChangeStreamFormat */
	virtual	OSStatus			ChangeStreamFormat(		AudioUnitScope					inScope,
														AudioUnitElement				inElement,
														const CAStreamBasicDescription & inPrevFormat,
														const CAStreamBasicDescription & inNewFormat);
															// Will only be called after StreamFormatWritable
															// and ValidFormat have succeeded.
															
	// ________________________________________________________________________

#if !CA_USE_AUDIO_PLUGIN_ONLY
	/*! @method ComponentEntryDispatch */
	static OSStatus			ComponentEntryDispatch(	ComponentParameters *			params,
														AUBase *						This);
#endif

	// ________________________________________________________________________
	// Methods useful for subclasses
	
	/*! @method GetScope */
	AUScope &					GetScope(				AudioUnitScope					inScope)
	{
		if (inScope >= kNumScopes) {
			AUScope * scope = GetScopeExtended(inScope);
			if (!scope) COMPONENT_THROW(kAudioUnitErr_InvalidScope);
			return *scope;
		}
		return mScopes[inScope];
	}
	
	/*! @method GetScopeExtended */
	virtual AUScope *			GetScopeExtended (AudioUnitScope inScope) { return NULL; }
	
	/*! @method GlobalScope */
	AUScope &					GlobalScope() { return mScopes[kAudioUnitScope_Global]; }
	/*! @method Inputs */
	AUScope &					Inputs()	{ return mScopes[kAudioUnitScope_Input]; }
	/*! @method Outputs */
	AUScope &					Outputs()	{ return mScopes[kAudioUnitScope_Output]; }
#if !CA_BASIC_AU_FEATURES
	/*! @method Groups */
	AUScope &					Groups()	{ return mScopes[kAudioUnitScope_Group]; }
#endif
	/*! @method Globals */
	AUElement *					Globals()	{ return mScopes[kAudioUnitScope_Global].GetElement(0); }
	
	/*! @method SetNumberOfElements */
	void						SetNumberOfElements(	AudioUnitScope					inScope,
														UInt32							numElements);

	/*! @method GetElement */
	AUElement *					GetElement(				AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement)
	{
		return GetScope(inScope).GetElement(inElement);
	}
	
	/*! @method GetIOElement */
	AUIOElement *				GetIOElement(			AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement)
	{
		return GetScope(inScope).GetIOElement(inElement);
	}
	
	/*! @method SafeGetElement */
	AUElement *					SafeGetElement(			AudioUnitScope 					inScope,
														AudioUnitElement			 	inElement)
	{
		return GetScope(inScope).SafeGetElement(inElement);
	}

	/*! @method GetInput */
	AUInputElement *			GetInput(				AudioUnitElement				inElement)
	{
		return static_cast<AUInputElement *>(Inputs().SafeGetElement(inElement));
	}
	
	/*! @method GetOutput */
	AUOutputElement *			GetOutput(				AudioUnitElement				inElement)
	{
		return static_cast<AUOutputElement *>(Outputs().SafeGetElement(inElement));
	}
	
#if !CA_BASIC_AU_FEATURES
	/*! @method GetGroup */
	AUElement *					GetGroup(				AudioUnitElement				inElement)
	{
		return Groups().SafeGetElement(inElement);
	}
#endif
	
	/*! @method PullInput */
	OSStatus					PullInput(				UInt32	 					inBusNumber,
														AudioUnitRenderActionFlags &ioActionFlags,
														const AudioTimeStamp &		inTimeStamp,
														UInt32						inNumberFrames)
	{
		AUInputElement *input = GetInput(inBusNumber);	// throws if error
		return input->PullInput(ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames);
	}

	/*! @method GetMaxFramesPerSlice */
	UInt32						GetMaxFramesPerSlice() const { return mMaxFramesPerSlice; }
	/*! @method UsesFixedBlockSize */
	bool						UsesFixedBlockSize() const { return mUsesFixedBlockSize; }
	/*! @method SetUsesFixedBlockSize */
	void						SetUsesFixedBlockSize(bool inUsesFixedBlockSize) { mUsesFixedBlockSize = inUsesFixedBlockSize; }
	
	/*! @method GetVectorUnitType */
	static SInt32				GetVectorUnitType() { return sVectorUnitType; }
	/*! @method HasVectorUnit */
	static bool					HasVectorUnit() { return sVectorUnitType > 0; }
	/*! @method HasAltivec */
	static bool					HasAltivec() { return sVectorUnitType == kVecAltivec; }
	/*! @method HasSSE2 */
	static bool					HasSSE2() { return sVectorUnitType >= kVecSSE2; }
	/*! @method HasSSE3 */
	static bool					HasSSE3() { return sVectorUnitType >= kVecSSE3; }
	
	/*! @method AudioUnitAPIVersion */
	UInt8						AudioUnitAPIVersion() const { return mAudioUnitAPIVersion; }
	
	/*! @method IsRenderThread */
	bool						InRenderThread () const 
								{
#if TARGET_OS_MAC
									return (mRenderThreadID ? pthread_equal (mRenderThreadID, pthread_self()) : false);
#elif TARGET_OS_WIN32
									return (mRenderThreadID ? mRenderThreadID == GetCurrentThreadId() : false);
#endif
								}

	/*! @method HasInput */
	bool						HasInput(				AudioUnitElement				inElement) {
									AUInputElement *in = static_cast<AUInputElement *>(Inputs().GetElement(inElement));
									return in != NULL && in->IsActive();
								}
									// says whether an input is connected or has a callback

	/*! @method PropertyChanged */
	virtual void				PropertyChanged(		AudioUnitPropertyID				inID,
														AudioUnitScope					inScope, 
														AudioUnitElement				inElement);

#if !CA_NO_AU_UI_FEATURES
	// These calls can be used to call a Host's Callbacks. The method returns -1 if the host
	// hasn't supplied the callback. Any other result is returned by the host.
	// As in the API contract, for a parameter's value, you specify a pointer
	// to that data type. Specify NULL for a parameter that you are not interested
	// as this can save work in the host.

	/*! @method CallHostBeatAndTempo */
	OSStatus	CallHostBeatAndTempo (Float64				*outCurrentBeat,  
										Float64				*outCurrentTempo)
	{
		return (mHostCallbackInfo.beatAndTempoProc 
						? (*mHostCallbackInfo.beatAndTempoProc) (mHostCallbackInfo.hostUserData, 
																	outCurrentBeat, 
																	outCurrentTempo)
						: -1);
	}

	/*! @method CallHostMusicalTimeLocation */
	OSStatus	CallHostMusicalTimeLocation (UInt32  		*outDeltaSampleOffsetToNextBeat,
										Float32           	*outTimeSig_Numerator,
										UInt32            	*outTimeSig_Denominator,
										Float64           	*outCurrentMeasureDownBeat)
	{
		return (mHostCallbackInfo.musicalTimeLocationProc 
						? (*mHostCallbackInfo.musicalTimeLocationProc) (mHostCallbackInfo.hostUserData,
																			outDeltaSampleOffsetToNextBeat,
																			outTimeSig_Numerator,
																			outTimeSig_Denominator,
																			outCurrentMeasureDownBeat)
						: -1);
	}											

	/*! @method CallHostTransportState */
	OSStatus	CallHostTransportState (Boolean 			*outIsPlaying,
										Boolean 			*outTransportStateChanged,
										Float64 			*outCurrentSampleInTimeLine,
										Boolean 			*outIsCycling,
										Float64 			*outCycleStartBeat,
										Float64 			*outCycleEndBeat)
	{
		return (mHostCallbackInfo.transportStateProc 
						? (*mHostCallbackInfo.transportStateProc) (mHostCallbackInfo.hostUserData,
																		outIsPlaying,
																		outTransportStateChanged,
																		outCurrentSampleInTimeLine,
																		outIsCycling,
																		outCycleStartBeat,
																		outCycleEndBeat)
						: -1);
	}
#endif

	char*						GetLoggingString () const;
	
	CAMutex*					GetMutex() { return mAUMutex; }

	// ________________________________________________________________________
	/*! @method CreateElement */
	virtual AUElement *			CreateElement(			AudioUnitScope					scope,
														AudioUnitElement				element);

#pragma mark -
#pragma mark AU Output Base Dispatch
	// ________________________________________________________________________
	// ________________________________________________________________________
	// ________________________________________________________________________
	// output unit methods
	/*! @method Start */
	virtual OSStatus	Start() { return kAudio_UnimplementedError; }
	/*! @method Stop */
	virtual OSStatus	Stop() { return kAudio_UnimplementedError; }
	
#if !CA_BASIC_AU_FEATURES
#pragma mark -
#pragma mark AU Music Base Dispatch

#if !TARGET_OS_IPHONE
// these methods are deprecated, so we don't include them except for compatability
	/*! @method PrepareInstrument */
	virtual OSStatus			PrepareInstrument(MusicDeviceInstrumentID inInstrument) { return kAudio_UnimplementedError; }

	/*! @method PrepareInstrument */
	virtual OSStatus			ReleaseInstrument(MusicDeviceInstrumentID inInstrument) { return kAudio_UnimplementedError; }
#endif
	
	// ________________________________________________________________________
	// ________________________________________________________________________
	// ________________________________________________________________________
	// music device/music effect methods -- incomplete
	/*! @method MIDIEvent */
	virtual OSStatus	MIDIEvent(		UInt32 						inStatus, 
										UInt32 						inData1, 
										UInt32 						inData2, 
										UInt32 						inOffsetSampleFrame) { return kAudio_UnimplementedError; }

	/*! @method SysEx */
	virtual OSStatus	SysEx(			const UInt8 *				inData, 
										UInt32 						inLength) { return kAudio_UnimplementedError;}
										
	/*! @method StartNote */
	virtual OSStatus	StartNote(		MusicDeviceInstrumentID 	inInstrument, 
										MusicDeviceGroupID 			inGroupID, 
										NoteInstanceID *			outNoteInstanceID, 
										UInt32 						inOffsetSampleFrame, 
										const MusicDeviceNoteParams &inParams) { return kAudio_UnimplementedError; }

	/*! @method StopNote */
	virtual OSStatus	StopNote(		MusicDeviceGroupID 			inGroupID, 
										NoteInstanceID 				inNoteInstanceID, 
										UInt32 						inOffsetSampleFrame) { return kAudio_UnimplementedError; }
#endif

	// ________________________________________________________________________
	// ________________________________________________________________________
	// ________________________________________________________________________
	
protected:
#pragma mark -
#pragma mark Implementation methods

	/*! @method ReallocateBuffers */
	virtual void				ReallocateBuffers();
									// needs to be called when mMaxFramesPerSlice changes
	virtual void				DeallocateIOBuffers();
		
	/*! @method FillInParameterName */
	static void					FillInParameterName (AudioUnitParameterInfo& ioInfo, CFStringRef inName, bool inShouldRelease)
	{
		ioInfo.cfNameString = inName;
		ioInfo.flags |= kAudioUnitParameterFlag_HasCFNameString;
		if (inShouldRelease)
			ioInfo.flags |= kAudioUnitParameterFlag_CFNameRelease;
		CFStringGetCString (inName, ioInfo.name, offsetof (AudioUnitParameterInfo, clumpID), kCFStringEncodingUTF8);
	}
	
	static void					HasClump (AudioUnitParameterInfo& ioInfo, UInt32 inClumpID)				
	{
		ioInfo.clumpID = inClumpID;
		ioInfo.flags |= kAudioUnitParameterFlag_HasClump;
	}
	
	/*! @method SetMaxFramesPerSlice */
	virtual void				SetMaxFramesPerSlice(UInt32 nFrames);

	/*! @method CanSetMaxFrames */
	virtual OSStatus			CanSetMaxFrames() const;
	
	/*! @method WantsRenderThreadID */
	bool						WantsRenderThreadID () const { return mWantsRenderThreadID; }
	
	/*! @method SetWantsRenderThreadID */
	void						SetWantsRenderThreadID (bool inFlag);
	
	/*! @method SetRenderError */
	OSStatus					SetRenderError (OSStatus inErr)
	{
		if (inErr && mLastRenderError == 0) {
			mLastRenderError = inErr;
			PropertyChanged(kAudioUnitProperty_LastRenderError, kAudioUnitScope_Global, 0);
		}
		return inErr;
	}
	
private:
	/*! @method DoRenderBus */
	// shared between Render and RenderSlice, inlined to minimize function call overhead
	OSStatus					DoRenderBus(			AudioUnitRenderActionFlags &	ioActionFlags,
														const AudioTimeStamp &			inTimeStamp,
														UInt32							inBusNumber,
														AUOutputElement *				theOutput,
														UInt32							inNumberFrames,
														AudioBufferList &				ioData)
	{
		if (ioData.mBuffers[0].mData == NULL || (theOutput->WillAllocateBuffer() && Outputs().GetNumberOfElements() > 1))
			// will render into cache buffer
			theOutput->PrepareBuffer(inNumberFrames);
		else
			// will render into caller's buffer
			theOutput->SetBufferList(ioData);
		OSStatus result = RenderBus(ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames);
		if (result == noErr) {
			if (ioData.mBuffers[0].mData == NULL) {
				theOutput->CopyBufferListTo(ioData);
				AUTRACE(kCATrace_AUBaseDoRenderBus, mComponentInstance, inNumberFrames, (intptr_t)theOutput->GetBufferList().mBuffers[0].mData, 0, *(UInt32 *)ioData.mBuffers[0].mData);
			} else {
				theOutput->CopyBufferContentsTo(ioData);
				AUTRACE(kCATrace_AUBaseDoRenderBus, mComponentInstance, inNumberFrames, (intptr_t)theOutput->GetBufferList().mBuffers[0].mData, (intptr_t)ioData.mBuffers[0].mData, *(UInt32 *)ioData.mBuffers[0].mData);
				theOutput->InvalidateBufferList();
			}
		}
		return result;
	}

	/*! @method HasIcon */
	bool						HasIcon ();

	/*! @method ResetRenderTime */
	void						ResetRenderTime ()
								{
									memset (&mCurrentRenderTime, 0, sizeof(mCurrentRenderTime));
									mCurrentRenderTime.mSampleTime = kNoLastRenderedSampleTime;
								}
				
protected:
	/*! @method GetAudioChannelLayout */
	virtual UInt32				GetChannelLayoutTags(	AudioUnitScope				scope,
														AudioUnitElement 			element,
														AudioChannelLayoutTag *		outLayoutTags);
														
	/*! @method GetAudioChannelLayout */
	virtual UInt32				GetAudioChannelLayout(	AudioUnitScope				scope,
														AudioUnitElement 			element,
														AudioChannelLayout *		outLayoutPtr,
														Boolean &					outWritable);

	/*! @method SetAudioChannelLayout */
	virtual OSStatus			SetAudioChannelLayout(	AudioUnitScope 				scope, 
														AudioUnitElement 			element,
														const AudioChannelLayout *	inLayout);

	/*! @method RemoveAudioChannelLayout */
	virtual OSStatus			RemoveAudioChannelLayout(AudioUnitScope scope, AudioUnitElement element);
	
	/*! @method NeedsToRender */
	bool						NeedsToRender(			const AudioTimeStamp &		inTimeStamp)
								{
									bool needsToRender = fnotequal(inTimeStamp.mSampleTime, mCurrentRenderTime.mSampleTime);
									if (needsToRender)	// only copy this if we need to render
										mCurrentRenderTime = inTimeStamp;
									return needsToRender;
								}
	
	// Scheduled parameter implementation:

	typedef std::vector<AudioUnitParameterEvent> ParameterEventList;

	// Usually, you won't override this method.  You only need to call this if your DSP code
	// is prepared to handle scheduled immediate and ramped parameter changes.
	// Before calling this method, it is assumed you have already called PullInput() on the input busses
	// for which the DSP code depends.  ProcessForScheduledParams() will call (potentially repeatedly)
	// virtual method ProcessScheduledSlice() to perform the actual DSP for a given sub-division of
	// the buffer.  The job of ProcessForScheduledParams() is to sub-divide the buffer into smaller
	// pieces according to the scheduled times found in the ParameterEventList (usually coming 
	// directly from a previous call to ScheduleParameter() ), setting the appropriate immediate or
	// ramped parameter values for the corresponding scopes and elements, then calling ProcessScheduledSlice()
	// to do the actual DSP for each of these divisions.
	virtual OSStatus 	ProcessForScheduledParams(	ParameterEventList		&inParamList,
															UInt32					inFramesToProcess,
															void					*inUserData );
	
	//	This method is called (potentially repeatedly) by ProcessForScheduledParams()
	//	in order to perform the actual DSP required for this portion of the entire buffer
	//	being processed.  The entire buffer can be divided up into smaller "slices"
	//	according to the timestamps on the scheduled parameters...
	//
	//	sub-classes wishing to handle scheduled parameter changes should override this method
	//  in order to do the appropriate DSP.  AUEffectBase already overrides this for standard
	//	effect AudioUnits.
	virtual OSStatus			ProcessScheduledSlice(	void				*inUserData,
														UInt32				inStartFrameInBuffer,
														UInt32				inSliceFramesToProcess,
														UInt32				inTotalBufferFrames ) {return noErr;};	// default impl does nothing...
	
	
	/*! @method CurrentRenderTime */
	const AudioTimeStamp &		CurrentRenderTime () const { return mCurrentRenderTime; }
	
	// ________________________________________________________________________
	//	Private data members to discourage hacking in subclasses
private:
	struct RenderCallback {
		RenderCallback(AURenderCallback proc, void *ref) :
			mRenderNotify(proc),
			mRenderNotifyRefCon(ref)
		{ }
		
		AURenderCallback			mRenderNotify;
		void *						mRenderNotifyRefCon;
		
		bool operator == (const RenderCallback &other) {
			return this->mRenderNotify == other.mRenderNotify &&
					this->mRenderNotifyRefCon == other.mRenderNotifyRefCon;
		}
	};
	typedef TThreadSafeList<RenderCallback>	RenderCallbackList;
	
#if !CA_BASIC_AU_FEATURES
	enum { kNumScopes = 4 };
#else
	enum { kNumScopes = 3 };
#endif
	
	/*! @var mElementsCreated */
	bool						mElementsCreated;
protected:
	/*! @var mInitialized */
	bool						mInitialized;
	/*! @var mHasBegunInitializing */
	bool						mHasBegunInitializing;
private:
	/*! @var mAudioUnitAPIVersion */
	UInt8						mAudioUnitAPIVersion;
	
	/*! @var mInitNumInputEls */
	const UInt32				mInitNumInputEls;
	/*! @var mInitNumOutputEls */
	const UInt32				mInitNumOutputEls;
#if !CA_BASIC_AU_FEATURES
	/*! @var mInitNumGroupEls */
	const UInt32				mInitNumGroupEls;
#endif
	/*! @var mScopes */
	AUScope						mScopes[kNumScopes];
	
	/*! @var mRenderCallbacks */
	RenderCallbackList			mRenderCallbacks;
	bool						mRenderCallbacksTouched;
	
	/*! @var mRenderThreadID */
#if TARGET_OS_MAC
	pthread_t					mRenderThreadID;
#elif TARGET_OS_WIN32
	UInt32						mRenderThreadID;
#endif
	
	/*! @var mWantsRenderThreadID */
	bool						mWantsRenderThreadID;
		
	/*! @var mCurrentRenderTime */
	AudioTimeStamp				mCurrentRenderTime;
	
	/*! @var mMaxFramesPerSlice */
	UInt32						mMaxFramesPerSlice;
	
	/*! @var mLastRenderError */
	OSStatus					mLastRenderError;
	/*! @var mCurrentPreset */
	AUPreset					mCurrentPreset;
	
protected:
	/*! @var mUsesFixedBlockSize */
	bool						mUsesFixedBlockSize;
	
	struct PropertyListener {
		AudioUnitPropertyID				propertyID;
		AudioUnitPropertyListenerProc	listenerProc;
		void *							listenerRefCon;
	};
	typedef std::vector<PropertyListener>	PropertyListeners;

	/*! @var mParamList */
	ParameterEventList			mParamList;
	/*! @var mPropertyListeners */
	PropertyListeners			mPropertyListeners;
	
	/*! @var mBuffersAllocated */
	bool						mBuffersAllocated;
	
	/*! @var mLogString */
	// if this is NOT null, it will contain identifying info about this AU.
	char*						mLogString;

	/*! @var mNickName */
	CFStringRef					mNickName;

	/*! @var mAUMutex */
	CAMutex *					mAUMutex;

private:
	/*! @var sVectorUnitType */
	static SInt32	sVectorUnitType;

#if !CA_NO_AU_HOST_CALLBACKS
protected:
	/*! @var mHostCallbackInfo */
	HostCallbackInfo 			mHostCallbackInfo;

#endif
#if !CA_NO_AU_UI_FEATURES
protected:
	/*! @var mContextInfo */
	CFStringRef					mContextName;
#endif
};

inline 	OSStatus	AUInputElement::PullInputWithBufferList(	
												AudioUnitRenderActionFlags &  	ioActionFlags,
												const AudioTimeStamp &			inTimeStamp,
												AudioUnitElement				inElement,
												UInt32							nFrames,
												AudioBufferList *				inBufferList)
{
	OSStatus theResult;
	
	if (HasConnection()) {
			// only support connections for V2 audio units
#if !CA_USE_AUDIO_PLUGIN_ONLY
		if (mConnRenderProc != NULL)
			theResult = reinterpret_cast<AudioUnitRenderProc>(mConnRenderProc)(
							mConnInstanceStorage, &ioActionFlags, &inTimeStamp, mConnection.sourceOutputNumber, nFrames, inBufferList);
		else
#endif
			theResult = AudioUnitRender(
							mConnection.sourceAudioUnit, &ioActionFlags, &inTimeStamp, mConnection.sourceOutputNumber, nFrames, inBufferList);
	} else {
		// kFromCallback:
			theResult = (mInputProc)(
							mInputProcRefCon, &ioActionFlags, &inTimeStamp, inElement, nFrames, inBufferList);
	}
	
	if (mInputType == kNoInput)	// defense: the guy upstream could have disconnected
								// it's a horrible thing to do, but may happen!
		return kAudioUnitErr_NoConnection;


	return theResult;
}

#endif // __AUBase_h__

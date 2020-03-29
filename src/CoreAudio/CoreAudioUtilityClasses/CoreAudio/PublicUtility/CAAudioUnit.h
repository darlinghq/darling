/*
     File: CAAudioUnit.h
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
#ifndef __CAAudioUnit_h__
#define __CAAudioUnit_h__

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
	#include <AudioUnit/AudioUnit.h>
	#include <AudioToolbox/AUGraph.h>
	#include <AudioUnit/MusicDevice.h>
#else
	#include <ConditionalMacros.h>
	#include <CoreAudioTypes.h>
	#include <AudioUnit.h>
	#include <MusicDevice.h>
	#include <AUGraph.h>
	#include <MusicDevice.h>
#endif

#include <vector>
#include "CAStreamBasicDescription.h"
#include "CAComponent.h"
#include "CAAudioChannelLayout.h"

// defined below
class CAAUChanHelper;

// These constructors will NOT throw exceptions - so "check" after creation if AU IsValid()
// The destructor will NOT automatically close the AU down
// This state should be managed by the Caller
// once closed, the unit represented by this object is no longer valid
// it is up to the user of this object to ensure its validity is in sync 
// if it is removed from a graph

// methods that can significantly change the state of the AU (like its format) are
// NOT const whereas those that don't change the externally related state of the AU are not const

class CAAudioUnit {
	enum {
		paramErr = -50,
		badComponentSelector = (long)0x80008002
	};
public:
	typedef std::vector<AudioChannelLayoutTag> 	ChannelTagVector;
	typedef ChannelTagVector::iterator 			ChannelTagVectorIter;

public:
							CAAudioUnit () 
								: mDataPtr(0) {}

							CAAudioUnit (const AudioUnit& inUnit);

							CAAudioUnit (const AUNode &inNode, const AudioUnit& inUnit);

							CAAudioUnit (const CAAudioUnit& y)
								: mDataPtr(0) { *this = y; }

	static OSStatus			Open (const CAComponent& inComp, CAAudioUnit &outUnit);

							~CAAudioUnit ();

	void					Close ();

	
	CAAudioUnit&			operator= (const CAAudioUnit& y);

	bool					operator== (const CAAudioUnit& y) const;

	bool					operator== (const AudioUnit& y) const;

#pragma mark __State Management	
	bool					IsValid () const;
	
	AudioUnit				AU() const;
	operator AudioUnit () const { return AU(); }

	const CAComponent&		Comp() const { return mComp; }
	
	const CAComponentDescription& Desc() const { return mComp.Desc(); }
	
	bool					FromAUGraph () const { return GetAUNode() != 0 && GetAUNode() != kCAAU_DoNotKnowIfAUNode; }
	
	AUNode					GetAUNode () const;
	operator AUNode () const { return GetAUNode(); }
	
#pragma mark __API Wrapper
	OSStatus				Initialize() const {
								return AudioUnitInitialize(AU());
							}
	OSStatus				Uninitialize() const {
								return AudioUnitUninitialize(AU());
							}
	OSStatus				GetPropertyInfo(AudioUnitPropertyID propID, AudioUnitScope scope, AudioUnitElement element,
											UInt32 *outDataSize, Boolean *outWritable) const
							{
								return AudioUnitGetPropertyInfo(AU(), propID, scope, element, outDataSize, outWritable);
							}
	OSStatus				GetProperty(AudioUnitPropertyID propID, AudioUnitScope scope, AudioUnitElement element,
											void *outData, UInt32 *ioDataSize) const
							{
								return AudioUnitGetProperty(AU(), propID, scope, element, outData, ioDataSize);
							}
	OSStatus				SetProperty(AudioUnitPropertyID propID, AudioUnitScope scope, AudioUnitElement element,
											const void *inData, UInt32 inDataSize)
							{
								return AudioUnitSetProperty(AU(), propID, scope, element, inData, inDataSize);
							}
	OSStatus				SetParameter(AudioUnitParameterID inID, AudioUnitScope scope, AudioUnitElement element,
											Float32 value, UInt32 bufferOffsetFrames=0);
							
	OSStatus				GetParameter(AudioUnitParameterID inID, AudioUnitScope scope, AudioUnitElement element,
											Float32 &outValue) const;

	OSStatus				Render (AudioUnitRenderActionFlags 				* ioActionFlags,
												const AudioTimeStamp 		* inTimeStamp,
												UInt32						inOutputBusNumber,
												UInt32						inNumberFrames,
												AudioBufferList				* ioData);

	OSStatus				Process (AudioUnitRenderActionFlags 			& ioActionFlags,
												const AudioTimeStamp 		& inTimeStamp,
												UInt32						inNumberFrames,
												AudioBufferList				& ioData);
	
	OSStatus				ProcessMultiple (AudioUnitRenderActionFlags 	& ioActionFlags,
												const AudioTimeStamp		& inTimeStamp,
												UInt32						inNumberFrames,
												UInt32						inNumberInputBufferLists,
												const AudioBufferList **	inInputBufferLists,
												UInt32						inNumberOutputBufferLists,
												AudioBufferList **			ioOutputBufferLists);

	
	OSStatus				Reset (AudioUnitScope scope, AudioUnitElement element)
							{
								return AudioUnitReset (AU(), scope, element);
							}
	OSStatus				GlobalReset ()
							{
								return AudioUnitReset (AU(), kAudioUnitScope_Global, 0);
							}

	OSStatus				AddRenderNotify (AURenderCallback   inProc, void *inProcRefCon)
							{
								return AudioUnitAddRenderNotify (AU(), inProc, inProcRefCon);
							}
	
	OSStatus				RemoveRenderNotify (AURenderCallback   inProc, void *inProcRefCon)
							{
								return AudioUnitRemoveRenderNotify (AU(), inProc, inProcRefCon);
							}

	OSStatus				AddPropertyListener (AudioUnitPropertyID				inID,
													AudioUnitPropertyListenerProc	inProc,
													void *							inProcRefCon)
							{
								return AudioUnitAddPropertyListener (AU(), inID, inProc, inProcRefCon);
							}
	
	OSStatus				RemovePropertyListener (AudioUnitPropertyID				inID,
													AudioUnitPropertyListenerProc	inProc,
													void *							inProcUserData);
	
	OSStatus				MIDIEvent (UInt32					inStatus,
										UInt32					inData1,
										UInt32					inData2,
										UInt32					inOffsetSampleFrame);
								
								// uses the default VoiceForGroup value - this is the normal case
	OSStatus				StartNote (MusicDeviceGroupID		inGroupID,
									NoteInstanceID *			outNoteInstanceID,
									UInt32						inOffsetSampleFrame,
									const MusicDeviceNoteParams * inParams)
							{
								return StartNote (kMusicNoteEvent_UseGroupInstrument, 
													inGroupID, outNoteInstanceID, 
													inOffsetSampleFrame, inParams);
							}

	OSStatus				StartNote (MusicDeviceInstrumentID	inInstrument,
									MusicDeviceGroupID			inGroupID,
									NoteInstanceID *			outNoteInstanceID,
									UInt32						inOffsetSampleFrame,
									const MusicDeviceNoteParams * inParams);

	OSStatus				StopNote (MusicDeviceGroupID		inGroupID,
									NoteInstanceID				inNoteInstanceID,
									UInt32						inOffsetSampleFrame);

#pragma mark __Format Utilities
		// typically you ask this about an AU
		// These Questions are asking about Input and Output...

		// These ones just say whether an AU can do a single combination of channels
		// and is fine if the AU has a single output (and if an input, a single input)
	bool					CanDo (int inChannelsInOut) const
							{
								return CanDo (inChannelsInOut, inChannelsInOut);
							}
							
	bool					CanDo (		int 				inChannelsIn, 
										int 				inChannelsOut) const;
		
		// This version does a more thorough test for ANY AU with ANY ins/outs
		// you pass in the channel helper (for the current element count on that scope)
		
	bool					CanDo (		const CAAUChanHelper		&input,
										const CAAUChanHelper		&output) const;
	
	bool					SupportsNumChannels () const;

	bool					HasChannelLayouts (AudioUnitScope 		inScope, 
											AudioUnitElement 		inEl) const;
		
	OSStatus				GetChannelLayoutTags (AudioUnitScope 	inScope,
									AudioUnitElement 				inEl,
									ChannelTagVector				&outChannelVector) const;
	
	bool					HasChannelLayout (AudioUnitScope 		inScope, 
											AudioUnitElement 		inEl) const;
	
	OSStatus				GetChannelLayout (AudioUnitScope 		inScope,
											AudioUnitElement 		inEl,
											CAAudioChannelLayout	&outLayout) const;	

	OSStatus				SetChannelLayout (AudioUnitScope 		inScope, 
											AudioUnitElement 		inEl,
											const CAAudioChannelLayout	&inLayout);

	OSStatus				SetChannelLayout (AudioUnitScope 		inScope, 
											AudioUnitElement 		inEl,
											const AudioChannelLayout		&inLayout,
											UInt32					inSize);
											
	OSStatus				ClearChannelLayout (AudioUnitScope		inScope,
											AudioUnitElement		inEl);
												
	OSStatus				GetFormat (AudioUnitScope					inScope,
											AudioUnitElement			inEl,
											AudioStreamBasicDescription	&outFormat) const;
	// if an AudioChannelLayout is either required or set, this call can fail
	// and the SetChannelLayout call should be used to set the format
	OSStatus				SetFormat (AudioUnitScope							inScope,
											AudioUnitElement					inEl,
											const AudioStreamBasicDescription	&inFormat);

	OSStatus				GetSampleRate (AudioUnitScope		inScope,
											AudioUnitElement	inEl,
											Float64				&outRate) const;
	OSStatus				SetSampleRate (AudioUnitScope		inScope,
											AudioUnitElement	inEl,
											Float64				inRate);

	// this sets the sample rate on all in/out buses of the AU
	OSStatus				SetSampleRate (Float64				inSampleRate);
	
	OSStatus				NumberChannels (AudioUnitScope		inScope,
											AudioUnitElement	inEl,
											UInt32				&outChans) const;

	OSStatus				GetNumberChannels (AudioUnitScope	inScope,
											AudioUnitElement	inEl,
											UInt32				&outChans) const 
							{ 
								return NumberChannels (inScope, inEl, outChans); 
							}

	OSStatus				SetNumberChannels (AudioUnitScope	inScope,
											AudioUnitElement	inEl,
											UInt32				inChans);

	OSStatus				IsElementCountWritable (AudioUnitScope inScope, bool &outWritable) const;

	OSStatus				GetElementCount (AudioUnitScope 	inScope, UInt32 &outCount) const;

	OSStatus				SetElementCount (AudioUnitScope		inScope, UInt32 inCount);
		
		// value of -1 for outTotalNumChannels indicates no restriction on num channels
		// for ex. the Matrix Mixer satisfies this (its in/out element count is writable, and can be set to 
		// any number of channels.
		// outTotalNumChannels is only valid if method returns true...
	bool					HasDynamicInputs (SInt32 &outTotalNumChannels) const
							{
								return HasDynamicScope (kAudioUnitScope_Input, outTotalNumChannels);
							}
							
	bool					HasDynamicOutputs (SInt32 &outTotalNumChannels) const
							{
								return HasDynamicScope (kAudioUnitScope_Output, outTotalNumChannels);
							}
	
		// here, if the in (or out) elements are dynamic, then you supply the number of elements
		// you want on in (or out) scope, and the number of channels on each consecutive element
	OSStatus				ConfigureDynamicInput (UInt32 inNumElements, UInt32 *inChannelsPerElement, Float64 inSampleRate)
							{
								return ConfigureDynamicScope (kAudioUnitScope_Input, inNumElements, inChannelsPerElement, inSampleRate);
							}
							
	OSStatus				ConfigureDynamicOutput (UInt32 inNumElements, UInt32 *inChannelsPerElement, Float64 inSampleRate)
							{
								return ConfigureDynamicScope (kAudioUnitScope_Output, inNumElements, inChannelsPerElement, inSampleRate);
							}

	bool					CanBypass 		() const;

	bool					GetBypass 		() const;

	OSStatus				SetBypass 		(bool				inBypass) const;
	
	OSStatus				GetMaxFramesPerSlice (UInt32& outMaxFrames) const;
	
	OSStatus				SetMaxFramesPerSlice (UInt32 inMaxFrames);
	
	Float64					Latency () const;
	
		// these calls just deal with the global preset state
		// you could rescope them to deal with presets on the part scope
	OSStatus				GetAUPreset (CFPropertyListRef &outData) const;

	OSStatus				SetAUPreset (CFPropertyListRef &inData);
	
	OSStatus				SetAUPresetFromDocument (CFPropertyListRef &inData);

	OSStatus				GetPresentPreset (AUPreset &outData) const;
	
	OSStatus				SetPresentPreset (AUPreset &inData);
	
	bool					HasCustomView () const;
	
#pragma mark __Print	
	void					Print () const { Print (stdout); }
	void					Print (FILE* file) const;
	
private:
	CAComponent				mComp;
	
	class AUState;
	AUState*		mDataPtr;
		
		// this can throw - so wrap this up in a static that returns a result code...
	CAAudioUnit (const CAComponent& inComp);

	bool				HasDynamicScope (AudioUnitScope inScope, SInt32 &outTotalNumChannels) const;
	OSStatus			ConfigureDynamicScope (AudioUnitScope   inScope, 
											UInt32				inNumElements, 
											UInt32				*inChannelsPerElement, 
											Float64				inSampleRate);
	bool				ValidateChannelPair (int 				inChannelsIn, 
											int 				inChannelsOut,
											const AUChannelInfo * info,
											UInt32				numChanInfo) const;
											
	bool				ValidateDynamicScope (AudioUnitScope	inScope, 
											SInt32				&outTotalNumChannels, 
											const AUChannelInfo * info, 
											UInt32 numInfo) const;
	bool				CheckOneSide (const CAAUChanHelper		&inHelper, 
											bool				checkOutput, 
											const AUChannelInfo *info, 
											UInt32				numInfo) const;
	enum {
			kCAAU_DoNotKnowIfAUNode = -1
	};
};

class CAAUChanHelper {
public:
				CAAUChanHelper() 
					: mChans(mStaticChans), mNumEls(0), mDidAllocate(false) 
				{
					memset (mChans, 0, sizeof(UInt32) * kStaticElCount);
				}
				CAAUChanHelper(UInt32 inMaxElems);
				CAAUChanHelper(const CAAudioUnit &inAU, AudioUnitScope inScope);
				CAAUChanHelper (const CAAUChanHelper &c) 
					: mChans(mStaticChans), mNumEls(0), mDidAllocate(false) 
					{ *this = c; }
				
				~CAAUChanHelper();

	CAAUChanHelper& operator= (const CAAUChanHelper &c);

	UInt32		* mChans;
	UInt32		mNumEls;
	
private:
	enum {
		kStaticElCount = 8
	};
	UInt32 mStaticChans[kStaticElCount];
	bool mDidAllocate;
};

#endif

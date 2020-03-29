/*
     File: CAAudioUnit.cpp
 Abstract: CAAudioUnit.h
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
#include "CAAudioUnit.h"

#if !TARGET_OS_IPHONE
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreServices/../Frameworks/CarbonCore.framework/Headers/Components.h>
	#include <dlfcn.h>
#else
	#include <Components.h>
#endif
#endif

#include "CAXException.h"
#include "CAReferenceCounted.h"
#include "AUOutputBL.h" //this is for the Preroll only

struct StackAUChannelInfo {
		StackAUChannelInfo (UInt32 inSize) : mChanInfo ((AUChannelInfo*)malloc (inSize)) {}
		~StackAUChannelInfo() { free (mChanInfo); }
		
	AUChannelInfo* mChanInfo;
};

// is this symbol is not defined then we use the default setting which is that fast dispatch
// is supported on a desktop environment
#ifndef CA_AU_USE_FAST_DISPATCH
	#define CA_AU_USE_FAST_DISPATCH !TARGET_OS_IPHONE
#endif

#if CA_AU_USE_FAST_DISPATCH
static void *LoadGetComponentInstanceStorage (void *inst);
#endif


class CAAudioUnit::AUState : public CAReferenceCounted  {
public:
	AUState (AudioComponent inComp)
						: mUnit(0), mNode (0)
						{ 
							OSStatus result = ::AudioComponentInstanceNew (inComp, &mUnit); 
							if (result)
								throw result;
							Init();
						}

	AUState (const AUNode &inNode, const AudioUnit& inUnit)
						: mUnit (inUnit), mNode (inNode) 
						{
							Init();
						}
						
	~AUState();
											
	AudioUnit			mUnit;
	AUNode				mNode;

	OSStatus			GetParameter(AudioUnitParameterID inID, AudioUnitScope scope, AudioUnitElement element,
											Float32 &outValue) const
	{
#if CA_AU_USE_FAST_DISPATCH
			if (mGetParamProc != NULL) {
				return (mGetParamProc) (mConnInstanceStorage, inID, scope, element, &outValue);
			}							
#endif
		return AudioUnitGetParameter(mUnit, inID, scope, element, &outValue);
	}

	OSStatus			SetParameter(AudioUnitParameterID inID, AudioUnitScope scope, AudioUnitElement element,
											Float32 value, UInt32 bufferOffsetFrames)
	{
#if CA_AU_USE_FAST_DISPATCH
			if (mSetParamProc != NULL) {
				return (mSetParamProc) (mConnInstanceStorage, inID, scope, element, value, bufferOffsetFrames);
			}							
#endif
			return AudioUnitSetParameter(mUnit, inID, scope, element, value, bufferOffsetFrames);
	}
	
	OSStatus			Render (AudioUnitRenderActionFlags *  ioActionFlags,
								const AudioTimeStamp *        inTimeStamp,
								UInt32                        inOutputBusNumber,
								UInt32                        inNumberFrames,
								AudioBufferList *             ioData)
	{
#if CA_AU_USE_FAST_DISPATCH
		if (mRenderProc != NULL) {
			return (mRenderProc) (mConnInstanceStorage, ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData);
		}							
#endif
		return AudioUnitRender(mUnit, ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData);
	}
	
	OSStatus		MIDIEvent (UInt32					inStatus,
								UInt32					inData1,
								UInt32					inData2,
								UInt32					inOffsetSampleFrame)
	{
#if !TARGET_OS_WIN32
#if CA_AU_USE_FAST_DISPATCH
		if (mMIDIEventProc != NULL) {
			return (mMIDIEventProc) (mConnInstanceStorage, inStatus, inData1, inData2, inOffsetSampleFrame);
		}
#endif
		return MusicDeviceMIDIEvent (mUnit, inStatus, inData1, inData2, inOffsetSampleFrame);
#else	// ON WINDOWS _ NO MIDI EVENT dispatch
		return paramErr;
#endif
	}

	OSStatus				StartNote (MusicDeviceInstrumentID	inInstrument,
									MusicDeviceGroupID			inGroupID,
									NoteInstanceID *			outNoteInstanceID,
									UInt32						inOffsetSampleFrame,
									const MusicDeviceNoteParams * inParams)
	{
#if !TARGET_OS_WIN32
#if CA_AU_USE_FAST_DISPATCH
		if (mStartNoteProc != NULL) {
			return (mStartNoteProc) (mConnInstanceStorage, inInstrument, inGroupID, outNoteInstanceID, inOffsetSampleFrame, inParams);
		}
#endif
		return MusicDeviceStartNote (mUnit, inInstrument, inGroupID, outNoteInstanceID, inOffsetSampleFrame, inParams);
#else
		return paramErr;
#endif
	}
	OSStatus				StopNote (MusicDeviceGroupID		inGroupID,
									NoteInstanceID				inNoteInstanceID,
									UInt32						inOffsetSampleFrame)
	{
#if !TARGET_OS_WIN32
#if CA_AU_USE_FAST_DISPATCH
		if (mStopNoteProc != NULL) {
			return (mStopNoteProc) (mConnInstanceStorage, inGroupID, inNoteInstanceID, inOffsetSampleFrame);
		}
#endif
		return MusicDeviceStopNote (mUnit, inGroupID, inNoteInstanceID, inOffsetSampleFrame);
#else
		return paramErr;
#endif
	}

private:
	// get the fast dispatch pointers
	void Init() 
	{
#if CA_AU_USE_FAST_DISPATCH
		UInt32 size = sizeof(AudioUnitRenderProc);
		if (AudioUnitGetProperty(mUnit, kAudioUnitProperty_FastDispatch,
								kAudioUnitScope_Global, kAudioUnitRenderSelect,
								&mRenderProc, &size) != noErr)
			mRenderProc = NULL;
		
		size = sizeof(AudioUnitGetParameterProc);
		if (AudioUnitGetProperty(mUnit, kAudioUnitProperty_FastDispatch,
								kAudioUnitScope_Global, kAudioUnitGetParameterSelect,
								&mGetParamProc, &size) != noErr)
			mGetParamProc = NULL;
		
		size = sizeof(AudioUnitSetParameterProc);
		if (AudioUnitGetProperty(mUnit, kAudioUnitProperty_FastDispatch,
								kAudioUnitScope_Global, kAudioUnitSetParameterSelect,
								&mSetParamProc, &size) != noErr)
			mSetParamProc = NULL;

		size = sizeof(MusicDeviceMIDIEventProc);
		if (AudioUnitGetProperty(mUnit, kAudioUnitProperty_FastDispatch,
								kAudioUnitScope_Global, kMusicDeviceMIDIEventSelect,
								&mMIDIEventProc, &size) != noErr)
			mMIDIEventProc = NULL;

		size = sizeof(MusicDeviceStartNoteProc);
		if (AudioUnitGetProperty(mUnit, kAudioUnitProperty_FastDispatch,
								kAudioUnitScope_Global, kMusicDeviceStartNoteSelect,
								&mStartNoteProc, &size) != noErr)
			mStartNoteProc = NULL;

		size = sizeof(MusicDeviceStopNoteProc);
		if (AudioUnitGetProperty(mUnit, kAudioUnitProperty_FastDispatch,
								kAudioUnitScope_Global, kMusicDeviceStopNoteSelect,
								&mStopNoteProc, &size) != noErr)
			mStopNoteProc = NULL;
		
		if (mRenderProc || mGetParamProc || mSetParamProc || mMIDIEventProc || mStartNoteProc || mStopNoteProc) {
			mConnInstanceStorage = LoadGetComponentInstanceStorage ( mUnit );
		} else
			mConnInstanceStorage = NULL;
#else
		mConnInstanceStorage = NULL;
#endif
	}
	
#if CA_AU_USE_FAST_DISPATCH
	AudioUnitRenderProc			mRenderProc;
	AudioUnitGetParameterProc	mGetParamProc; 
	AudioUnitSetParameterProc	mSetParamProc;
	MusicDeviceMIDIEventProc	mMIDIEventProc;
	MusicDeviceStartNoteProc	mStartNoteProc;
	MusicDeviceStopNoteProc		mStopNoteProc;
#endif

	void *						mConnInstanceStorage;

private:
		// get the compiler to tell us when we do a bad thing!!!
	AUState () {}
	AUState (const AUState&);
	AUState& operator= (const AUState&);
};						
						
						
CAAudioUnit::AUState::~AUState ()
{
	if (mUnit && (mNode == 0)) {
		::AudioComponentInstanceDispose (mUnit);
	}
	mNode = 0;
	mUnit = 0;
}

OSStatus		CAAudioUnit::Open (const CAComponent& inComp, CAAudioUnit &outUnit)
{
	try {
		outUnit = inComp; 
		return noErr;
	} catch (OSStatus res) {
		return res;
	} catch (...) {
		return -1;
	}
}

CAAudioUnit::CAAudioUnit (const AudioUnit& inUnit)
	: mComp (inUnit), mDataPtr (new AUState (kCAAU_DoNotKnowIfAUNode, inUnit))
{
}

CAAudioUnit::CAAudioUnit (const CAComponent& inComp)
	: mComp (inComp), mDataPtr (new AUState (mComp.Comp()))
{
}

CAAudioUnit::CAAudioUnit (const AUNode &inNode, const AudioUnit& inUnit)
	: mComp (inUnit), mDataPtr(new AUState (inNode, inUnit)) 
{
}

CAAudioUnit::~CAAudioUnit ()
{
	Close();
}

void		CAAudioUnit::Close()
{
	if (mDataPtr) {
		mDataPtr->release();
		mDataPtr = NULL;
	}
}

CAAudioUnit&	CAAudioUnit::operator= (const CAAudioUnit &a)
{
	if (mDataPtr != a.mDataPtr) {
		if (mDataPtr)
			mDataPtr->release();
	
		if ((mDataPtr = a.mDataPtr) != NULL)
			mDataPtr->retain();
		
		mComp = a.mComp;
	}
	
	return *this;
}

bool			CAAudioUnit::operator== (const CAAudioUnit& y) const
{
	if (mDataPtr == y.mDataPtr) return true;
	AudioUnit au1 = mDataPtr ? mDataPtr->mUnit : 0;
	AudioUnit au2 = y.mDataPtr ? y.mDataPtr->mUnit : 0;
	return au1 == au2;
}

bool			CAAudioUnit::operator== (const AudioUnit& y) const
{
	if (!mDataPtr) return false;
	return mDataPtr->mUnit == y;
}

OSStatus		CAAudioUnit::RemovePropertyListener (AudioUnitPropertyID		inID,
												AudioUnitPropertyListenerProc	inProc,
												void *							inProcUserData)
{
		// we call this first. If it fails we call the old API as the failure can
		// mean that the AU doesn't implement that selector.
	OSStatus result = AudioUnitRemovePropertyListenerWithUserData(AU(), inID, 
									inProc, inProcUserData);
	#if !__LP64__ && !TARGET_OS_IPHONE
		if (result) result = AudioUnitRemovePropertyListener (AU(), inID, inProc);
	#endif									
	return result;
}	

#pragma mark __State Management	

bool			CAAudioUnit::IsValid () const 
{ 
	return mDataPtr ? mDataPtr->mUnit != 0 : false; 
}
	
AudioUnit		CAAudioUnit::AU() const 
{ 
	return mDataPtr ? mDataPtr->mUnit : 0; 
}

AUNode			CAAudioUnit::GetAUNode () const
{
	return mDataPtr ? mDataPtr->mNode : 0; 
}

#pragma mark __Format Handling
	
bool		CAAudioUnit::CanDo (	int 				inChannelsIn, 
									int 				inChannelsOut) const
{		
	// this is the default assumption of an audio effect unit
	Boolean* isWritable = 0;
	UInt32	dataSize = 0;
		// lets see if the unit has any channel restrictions
	OSStatus result = AudioUnitGetPropertyInfo (AU(),
									kAudioUnitProperty_SupportedNumChannels,
									kAudioUnitScope_Global, 0,
									&dataSize, isWritable); //don't care if this is writable
		
		// if this property is NOT implemented an FX unit
		// is expected to deal with same channel valance in and out
	if (result) 
	{
		if ((Comp().Desc().IsEffect() && inChannelsIn == inChannelsOut)
			|| (Comp().Desc().IsOffline() && inChannelsIn == inChannelsOut))
		{
			return true;
		}
		else 
		{
			// the au should either really tell us about this
			// or we will assume the worst
			return false;
		}
	}
	
	StackAUChannelInfo info (dataSize);
	
	result = GetProperty (kAudioUnitProperty_SupportedNumChannels,
							kAudioUnitScope_Global, 0,
							info.mChanInfo, &dataSize);
	if (result) { return false; }
	
	return ValidateChannelPair (inChannelsIn, inChannelsOut, info.mChanInfo, (dataSize / sizeof (AUChannelInfo)));
}

bool	CAAudioUnit::ValidateChannelPair (int 				inChannelsIn, 
										int 				inChannelsOut,
										const AUChannelInfo * info,
										UInt32				numChanInfo) const
{
// we've the following cases (some combinations) to test here:
/*
>0		An explicit number of channels on either side
0		that side (generally input!) has no elements
-1		wild card:
-1,-1	any num channels as long as same channels on in and out
-1,-2	any num channels channels on in and out - special meaning
-2+ 	indicates total num channs AU can handle 
			- elements configurable to any num channels, 
			- element count in scope must be writable
*/

	//now chan layout can contain -1 for either scope (ie. doesn't care)
	for (unsigned int i = 0; i < numChanInfo; ++i)
	{
			//less than zero on both sides - check for special attributes
		if ((info[i].inChannels < 0) && (info[i].outChannels < 0))
		{
				// these are our wild card matches
			if (info[i].inChannels == -1 && info[i].outChannels == -1) {
				if (inChannelsIn && inChannelsOut) {
					if (inChannelsOut == inChannelsIn)
						return true;
				} else
					return true; // if one of these is zero, then a -1 means any
			}
			else if ((info[i].inChannels == -1 && info[i].outChannels == -2)
					|| (info[i].inChannels == -2 && info[i].outChannels == -1)) 
			{
				return true;
			}
				// these are our total num channels matches
				// element count MUST be writable
			else {
				bool outWrite = false; bool inWrite = false;
				IsElementCountWritable (kAudioUnitScope_Output, outWrite);
				IsElementCountWritable (kAudioUnitScope_Input, inWrite);
				if (inWrite && outWrite) {
					if ((inChannelsOut <= abs(info[i].outChannels))
						&& (inChannelsIn <= abs(info[i].inChannels))) 
					{
						return true;
					}
				}
			}
		}
			
			// special meaning on input, specific num on output
		else if (info[i].inChannels < 0) {
			if (info[i].outChannels == inChannelsOut) 
			{
					// can do any in channels
				if (info[i].inChannels == -1) {
					return true;
				} 
					// total chans on input
				else {
					bool inWrite = false;
					IsElementCountWritable (kAudioUnitScope_Input, inWrite);
					if (inWrite && (inChannelsIn <= abs(info[i].inChannels))) {
						return true;
					}
				}
			}
		}
		
			// special meaning on output, specific num on input
		else if (info[i].outChannels < 0) {
			if (info[i].inChannels == inChannelsIn) 
			{
					// can do any out channels
				if (info[i].outChannels == -1) {
					return true;
				} 
					// total chans on output
				else {
					bool outWrite = false;
					IsElementCountWritable (kAudioUnitScope_Output, outWrite);
					if (outWrite && (inChannelsOut <= abs(info[i].outChannels))) {
						return true;
					}
				}
			}
		}

			// both chans in struct >= 0 - thus has to explicitly match
		else if ((info[i].inChannels == inChannelsIn) && (info[i].outChannels == inChannelsOut)) {
			return true;
		} 
		
			// now check to see if a wild card on the args (inChannelsIn or inChannelsOut chans is zero) is found 
			// tells us to match just one side of the scopes
		else if (inChannelsIn == 0) {
			if (info[i].outChannels == inChannelsOut) {
				return true;
			}
		}
		else if (inChannelsOut == 0) {
			if (info[i].inChannels == inChannelsIn) {
				return true;
			}
		}
	}
	
	return false;
}

static
bool CheckDynCount (SInt32 inTotalChans, const CAAUChanHelper &inHelper)
{
	int totalChans = 0;
	for (unsigned int i = 0; i < inHelper.mNumEls; ++i)
		totalChans += inHelper.mChans[i];
	return (totalChans <= inTotalChans);
}

bool	CAAudioUnit::CheckOneSide (const CAAUChanHelper		&inHelper, 
									bool					checkOutput, 
									const AUChannelInfo		*info, 
									UInt32					numInfo) const
{
		// now we can use the wildcard option (see above impl) to see if this matches
	for (unsigned int el = 0; el < inHelper.mNumEls; ++el) {
		bool testAlready = false;
		for (unsigned int i = 0; i < el; ++i) {
			if (inHelper.mChans[i] == inHelper.mChans[el]) {
				testAlready = true;
				break;
			}
		}
		if (!testAlready) {
			if (checkOutput) {
				if (!ValidateChannelPair (0, inHelper.mChans[el], info, numInfo)) return false;
			} else {
				if (!ValidateChannelPair (inHelper.mChans[el], 0, info, numInfo)) return false;
			}
		}
	}
	return true;
}

bool		CAAudioUnit::CanDo (const CAAUChanHelper		&inputs,  
								const CAAUChanHelper		&outputs) const

{
// first check our state
		// huh!
	if (inputs.mNumEls == 0 && outputs.mNumEls == 0) return false;
	
	UInt32 elCount;
	if (GetElementCount (kAudioUnitScope_Input, elCount)) { return false; }
	if (elCount != inputs.mNumEls) return false;

	if (GetElementCount (kAudioUnitScope_Output, elCount)) { return false; }
	if (elCount != outputs.mNumEls) return false;
		
// (1) special cases (effects and sources (generators and instruments) only)
	UInt32	dataSize = 0;
	if (GetPropertyInfo (kAudioUnitProperty_SupportedNumChannels,
									kAudioUnitScope_Global, 0, &dataSize, NULL) != noErr) 
	{
		if (Comp().Desc().IsEffect() || Comp().Desc().IsOffline()) {
			UInt32 numChan = outputs.mNumEls > 0 ? outputs.mChans[0] : inputs.mChans[0];
			for (unsigned int in = 0; in < inputs.mNumEls; ++in)
				if (numChan != inputs.mChans[in]) return false;
			for (unsigned int out = 0; out < outputs.mNumEls; ++out)
				if (numChan != outputs.mChans[out]) return false;
			return true;
		}
		
			// in this case, all the channels have to match the current config
		if (Comp().Desc().IsGenerator() || Comp().Desc().IsMusicDevice()) {
			for (unsigned int in = 0; in < inputs.mNumEls; ++in) {
				UInt32 chan;
				if (NumberChannels (kAudioUnitScope_Input, in, chan)) return false;
				if (chan != UInt32(inputs.mChans[in])) return false;
			}
			for (unsigned int out = 0; out < outputs.mNumEls; ++out) {
				UInt32 chan;
				if (NumberChannels (kAudioUnitScope_Output, out, chan)) return false;
				if (chan != UInt32(outputs.mChans[out])) return false;
			}
			return true;
		}
		
			// if we get here we can't determine anything about channel capabilities
		return false;
	}

	StackAUChannelInfo info (dataSize);
	
	if (GetProperty (kAudioUnitProperty_SupportedNumChannels,
							kAudioUnitScope_Global, 0,
							info.mChanInfo, &dataSize) != noErr)
	{ 
		return false; 
	}
	
	int numInfo = dataSize / sizeof(AUChannelInfo);
	
// (2) Test for dynamic capability (or no elements on that scope)
	SInt32 dynInChans = 0;
	if (ValidateDynamicScope (kAudioUnitScope_Input, dynInChans, info.mChanInfo, numInfo)) {
		if (CheckDynCount (dynInChans, inputs) == false) return false;
	}

	SInt32 dynOutChans = 0;
	if (ValidateDynamicScope (kAudioUnitScope_Output, dynOutChans, info.mChanInfo, numInfo)) {
		if (CheckDynCount (dynOutChans, outputs) == false) return false;
	}

	if (dynOutChans && dynInChans) { return true; }

// (3)	Just need to test one side
	if (dynInChans || (inputs.mNumEls == 0)) {
		return CheckOneSide (outputs, true, info.mChanInfo, numInfo);
	}

	if (dynOutChans || (outputs.mNumEls == 0)) {
		return CheckOneSide (inputs, false, info.mChanInfo, numInfo);
	}

// (4) - not a dynamic AU, has ins and outs, and has channel constraints so we test every possible pairing
	for (unsigned int in = 0; in < inputs.mNumEls; ++in) 
	{
		bool testInAlready = false;
		for (unsigned int i = 0; i < in; ++i) {
			if (inputs.mChans[i] == inputs.mChans[in]) {
				testInAlready = true;
				break;
			}
		}
		if (!testInAlready) {
			for (unsigned int out = 0; out < outputs.mNumEls; ++out) {
					// try to save a little bit and not test the same pairing multiple times...
				bool testOutAlready = false;
				for (unsigned int i = 0; i < out; ++i) {
					if (outputs.mChans[i] == outputs.mChans[out]) {
						testOutAlready = true;
						break;
					}
				}
				if (!testOutAlready) {
					if (!ValidateChannelPair (inputs.mChans[in], outputs.mChans[out],info.mChanInfo, numInfo)) {
						return false;
					}
				}
			}
		}
	}
	
	return true;
}

bool		CAAudioUnit::SupportsNumChannels () const
{
	// this is the default assumption of an audio effect unit
	Boolean* isWritable = 0;
	UInt32	dataSize = 0;
		// lets see if the unit has any channel restrictions
	OSStatus result = AudioUnitGetPropertyInfo (AU(),
									kAudioUnitProperty_SupportedNumChannels,
									kAudioUnitScope_Global, 0,
									&dataSize, isWritable); //don't care if this is writable
		
		// if this property is NOT implemented an FX unit
		// is expected to deal with same channel valance in and out
	if (result) {
		if (Comp().Desc().IsEffect() || Comp().Desc().IsOffline())
			return true;
	}
	return result == noErr;
}

OSStatus		CAAudioUnit::GetChannelLayoutTags (AudioUnitScope 	inScope,
										AudioUnitElement 			inEl,
										ChannelTagVector			&outChannelVector) const
{
	if (HasChannelLayouts (inScope, inEl) == false) return kAudioUnitErr_InvalidProperty; 

	UInt32 dataSize;
	OSStatus result = AudioUnitGetPropertyInfo (AU(),
								kAudioUnitProperty_SupportedChannelLayoutTags,
								inScope, inEl,
								&dataSize, NULL);

	if (result) return result;
	
		// OK lets get our channel layouts and see if the one we want is present
	AudioChannelLayoutTag* info = (AudioChannelLayoutTag*)malloc (dataSize);
	result = AudioUnitGetProperty (AU(),
							kAudioUnitProperty_SupportedChannelLayoutTags,
							inScope, inEl,
							info, &dataSize);
	if (result) goto home;
	
	outChannelVector.erase (outChannelVector.begin(), outChannelVector.end());
	for (unsigned int i = 0; i < (dataSize / sizeof (AudioChannelLayoutTag)); ++i)
		outChannelVector.push_back (info[i]);

home:
	free (info);
	return result;
}

bool		CAAudioUnit::HasChannelLayouts (AudioUnitScope 		inScope, 
										AudioUnitElement 		inEl) const
{
	OSStatus result = AudioUnitGetPropertyInfo (AU(),
									kAudioUnitProperty_SupportedChannelLayoutTags,
									inScope, inEl,
									NULL, NULL);
	return !result;
}

bool		CAAudioUnit::HasChannelLayout (AudioUnitScope 		inScope, 
											AudioUnitElement 		inEl) const
{
	Boolean writable;
	UInt32 size;
	
	return AudioUnitGetPropertyInfo (AU(),
									kAudioUnitProperty_AudioChannelLayout,
									inScope, inEl,
									&size, &writable) == noErr;
}	

OSStatus	CAAudioUnit::GetChannelLayout (AudioUnitScope 		inScope,
										AudioUnitElement 		inEl,
										CAAudioChannelLayout	&outLayout) const
{
	UInt32 size;
	OSStatus result = AudioUnitGetPropertyInfo (AU(), kAudioUnitProperty_AudioChannelLayout,
									inScope, inEl, &size, NULL);
	if (result) return result;
	
	AudioChannelLayout *layout = (AudioChannelLayout*)malloc (size);

	ca_require_noerr (result = AudioUnitGetProperty (AU(), kAudioUnitProperty_AudioChannelLayout,
									inScope, inEl, layout, &size), home);

	outLayout = CAAudioChannelLayout (layout);
	
home:
	free (layout);
	return result;
}

OSStatus	CAAudioUnit::SetChannelLayout (AudioUnitScope 		inScope,
									AudioUnitElement 			inEl,
									const CAAudioChannelLayout 		&inLayout)
{
	OSStatus result = AudioUnitSetProperty (AU(),
									kAudioUnitProperty_AudioChannelLayout,
									inScope, inEl,
									inLayout, inLayout.Size());
	return result;
}

OSStatus	CAAudioUnit::SetChannelLayout (AudioUnitScope 			inScope, 
											AudioUnitElement 		inEl,
											const AudioChannelLayout		&inLayout,
											UInt32					inSize)
{
	OSStatus result = AudioUnitSetProperty (AU(),
									kAudioUnitProperty_AudioChannelLayout,
									inScope, inEl,
									&inLayout, inSize);
	return result;
}

OSStatus		CAAudioUnit::ClearChannelLayout (AudioUnitScope	inScope,
											AudioUnitElement	inEl)
{
	return AudioUnitSetProperty (AU(),
							kAudioUnitProperty_AudioChannelLayout,
							inScope, inEl, NULL, 0);
}

OSStatus	CAAudioUnit::GetFormat (AudioUnitScope				inScope,
									AudioUnitElement			inEl,
									AudioStreamBasicDescription	&outFormat) const
{
	UInt32 dataSize = sizeof (AudioStreamBasicDescription);
	return AudioUnitGetProperty (AU(), kAudioUnitProperty_StreamFormat,
								inScope, inEl, 
								&outFormat, &dataSize);
}

OSStatus	CAAudioUnit::SetFormat (AudioUnitScope						inScope,
									AudioUnitElement					inEl,
									const AudioStreamBasicDescription	&inFormat)
{
	return AudioUnitSetProperty (AU(), kAudioUnitProperty_StreamFormat,
								inScope, inEl,
								const_cast<AudioStreamBasicDescription*>(&inFormat), 
								sizeof (AudioStreamBasicDescription));
}

OSStatus	CAAudioUnit::GetSampleRate (AudioUnitScope		inScope,
										AudioUnitElement	inEl,
										Float64				&outRate) const
{
	UInt32 dataSize = sizeof (Float64);
	return AudioUnitGetProperty (AU(), kAudioUnitProperty_SampleRate,
								inScope, inEl, 
								&outRate, &dataSize);
}

OSStatus	CAAudioUnit::SetSampleRate (AudioUnitScope		inScope,
										AudioUnitElement	inEl,
										Float64				inRate)
{
	AudioStreamBasicDescription desc;
	OSStatus result = GetFormat (inScope, inEl, desc);
	if (result) return result;
	desc.mSampleRate = inRate;
	return SetFormat (inScope, inEl, desc);
}

OSStatus	CAAudioUnit::SetSampleRate (Float64			inSampleRate)
{
	OSStatus result;
	
	UInt32 elCount;
	ca_require_noerr (result = GetElementCount(kAudioUnitScope_Input, elCount), home);
	if (elCount) {
		for (unsigned int i = 0; i < elCount; ++i) {
			ca_require_noerr (result = SetSampleRate (kAudioUnitScope_Input, i, inSampleRate), home);
		}
	}

	ca_require_noerr (result = GetElementCount(kAudioUnitScope_Output, elCount), home);
	if (elCount) {
		for (unsigned int i = 0; i < elCount; ++i) {
			ca_require_noerr (result = SetSampleRate (kAudioUnitScope_Output, i, inSampleRate), home);
		}
	}
	
home:
	return result;
}

OSStatus	CAAudioUnit::NumberChannels (AudioUnitScope		inScope,
										AudioUnitElement	inEl,
										UInt32				&outChans) const
{
	AudioStreamBasicDescription desc;
	OSStatus result = GetFormat (inScope, inEl, desc);
	if (!result)
		outChans = desc.mChannelsPerFrame;
	return result;
}

OSStatus	CAAudioUnit::SetNumberChannels (AudioUnitScope	inScope,
										AudioUnitElement	inEl,
										UInt32				inChans)
{
			// set this as the output of the AU
	CAStreamBasicDescription desc;
	OSStatus result = GetFormat (inScope, inEl, desc);
		if (result) return result;
	desc.ChangeNumberChannels (inChans, desc.IsInterleaved());
	result = SetFormat (inScope, inEl, desc);
	return result;
}

OSStatus		CAAudioUnit::IsElementCountWritable (AudioUnitScope inScope, bool &outWritable) const
{
	Boolean isWritable;
	UInt32 outDataSize;
	OSStatus result = GetPropertyInfo (kAudioUnitProperty_ElementCount, inScope, 0, &outDataSize, &isWritable);
	if (result)
		return result;
	outWritable = isWritable ? true : false;
	return noErr;	
}

OSStatus		CAAudioUnit::GetElementCount (AudioUnitScope inScope, UInt32 &outCount) const
{
	UInt32 propSize = sizeof(outCount);
	return GetProperty (kAudioUnitProperty_ElementCount, inScope, 0, &outCount, &propSize);
}

OSStatus		CAAudioUnit::SetElementCount (AudioUnitScope inScope, UInt32 inCount)
{
	return SetProperty (kAudioUnitProperty_ElementCount, inScope, 0, &inCount, sizeof(inCount));
}

bool			CAAudioUnit::HasDynamicScope (AudioUnitScope inScope, SInt32 &outTotalNumChannels) const
{
	// ok - now we need to check the AU's capability here.
	// this is the default assumption of an audio effect unit
	Boolean* isWritable = 0;
	UInt32	dataSize = 0;
	OSStatus result = GetPropertyInfo (kAudioUnitProperty_SupportedNumChannels,
								kAudioUnitScope_Global, 0,
								&dataSize, isWritable); //don't care if this is writable
		
		// AU has to explicitly tell us about this.
	if (result) return false;

	StackAUChannelInfo info (dataSize);
	
	result = GetProperty (kAudioUnitProperty_SupportedNumChannels,
							kAudioUnitScope_Global, 0,
							info.mChanInfo, &dataSize);
	if (result) return false;

	return ValidateDynamicScope (inScope, outTotalNumChannels, info.mChanInfo, (dataSize / sizeof(AUChannelInfo)));
}

// as we've already checked that the element count is writable
// the following conditions will match this..
/*
-1, -2 ->	signifies no restrictions
-2, -1 ->	signifies no restrictions -> in this case outTotalNumChannels == -1 (any num channels)

-N 	(where N is less than -2), signifies the total channel count on the scope side (in or out)
*/
bool	CAAudioUnit::ValidateDynamicScope (AudioUnitScope		inScope, 
											SInt32				&outTotalNumChannels, 
											const AUChannelInfo *info, 
											UInt32				numInfo) const
{
	bool writable = false;
	OSStatus result = IsElementCountWritable (inScope, writable);
	if (result || (writable == false))
		return false;

	//now chan layout can contain -1 for either scope (ie. doesn't care)
	for (unsigned int i = 0; i < numInfo; ++i)
	{
		// lets test the special wild card case first...
		// this says the AU can do any num channels on input or output - for eg. Matrix Mixer
		if (((info[i].inChannels == -1) && (info[i].outChannels == -2))
			|| ((info[i].inChannels == -2) && (info[i].outChannels == -1)))
		{
			outTotalNumChannels = -1;
			return true;
		}
		
		// ok lets now test our special case....
		if (inScope == kAudioUnitScope_Input) {
				// isn't dynamic on this side at least
			if (info[i].inChannels >= 0)
				continue;
				
			if (info[i].inChannels < -2) {
				outTotalNumChannels = abs (info[i].inChannels);
				return true;
			}
		} 
		
		else if (inScope == kAudioUnitScope_Output) {
				// isn't dynamic on this side at least
			if (info[i].outChannels >= 0)
				continue;
				
			if (info[i].outChannels < -2) {
				outTotalNumChannels = abs (info[i].outChannels);
				return true;
			}
		} 
		
		else {
			break; // wrong scope was specified
		}
	}
	
	return false;	
}

OSStatus	CAAudioUnit::ConfigureDynamicScope (AudioUnitScope 		inScope, 
											UInt32 					inNumElements, 
											UInt32 					*inChannelsPerElement, 
											Float64 				inSampleRate)
{
	SInt32 numChannels = 0;
	bool isDyamic = HasDynamicScope (inScope, numChannels);
	if (isDyamic == false)
		return kAudioUnitErr_InvalidProperty;
	
	//lets to a sanity check...
	// if numChannels == -1, then it can do "any"...
	if (numChannels > 0) {
		SInt32 count = 0;
		for (unsigned int i = 0; i < inNumElements; ++i)
			count += inChannelsPerElement[i];
		if (count > numChannels)
			return kAudioUnitErr_InvalidPropertyValue;
	}
	
	OSStatus result = SetElementCount (inScope, inNumElements);
	if (result)
		return result;
		
	for (unsigned int i = 0; i < inNumElements; ++i) {
		CAStreamBasicDescription desc;
		result = GetFormat (inScope, i, desc);
			if (result) return result;
		desc.ChangeNumberChannels (inChannelsPerElement[i], desc.IsInterleaved());
		desc.mSampleRate = inSampleRate;
		result = SetFormat (inScope, i, desc);
		if (result)
			return result;
	}
	return noErr;
}

#pragma mark __Properties

bool		CAAudioUnit::CanBypass () const
{
	Boolean outWritable;
	OSStatus result = AudioUnitGetPropertyInfo (AU(), kAudioUnitProperty_BypassEffect,
									kAudioUnitScope_Global, 0,
									NULL, &outWritable);
	return (!result && outWritable);
}

bool		CAAudioUnit::GetBypass 		() const
{
	UInt32 dataSize = sizeof (UInt32);
	UInt32 outBypass;
	OSStatus result = AudioUnitGetProperty (AU(), kAudioUnitProperty_BypassEffect,
								kAudioUnitScope_Global, 0,
								&outBypass, &dataSize);
	return (result ? false : outBypass);
}

OSStatus	CAAudioUnit::SetBypass 		(bool	inBypass) const
{	
	UInt32 bypass = inBypass ? 1 : 0;
	return AudioUnitSetProperty (AU(), kAudioUnitProperty_BypassEffect,
								kAudioUnitScope_Global, 0,
								&bypass, sizeof (UInt32));
}

OSStatus	CAAudioUnit::GetMaxFramesPerSlice (UInt32& outMaxFrames) const
{
	UInt32 dataSize = sizeof(outMaxFrames);
	return AudioUnitGetProperty (AU(), kAudioUnitProperty_MaximumFramesPerSlice,
								kAudioUnitScope_Global, 0,
								&outMaxFrames, &dataSize);
}

OSStatus	CAAudioUnit::SetMaxFramesPerSlice (UInt32 inMaxFrames)
{
	return AudioUnitSetProperty (AU(), kAudioUnitProperty_MaximumFramesPerSlice,
								 kAudioUnitScope_Global, 0,
								 &inMaxFrames, sizeof (UInt32));	
}

Float64		CAAudioUnit::Latency () const
{
	Float64 secs;
	UInt32 size = sizeof(secs);
	if (GetProperty (kAudioUnitProperty_Latency, kAudioUnitScope_Global, 0, &secs, &size))
		return 0;
	return secs;
}

OSStatus	CAAudioUnit::GetAUPreset (CFPropertyListRef &outData) const
{
	UInt32 dataSize = sizeof(outData);
	return AudioUnitGetProperty (AU(), kAudioUnitProperty_ClassInfo,
								kAudioUnitScope_Global, 0,
								&outData, &dataSize);
}

OSStatus	CAAudioUnit::SetAUPreset (CFPropertyListRef &inData)
{
	return AudioUnitSetProperty (AU(), kAudioUnitProperty_ClassInfo,
								kAudioUnitScope_Global, 0,
								&inData, sizeof (CFPropertyListRef));
}

#if !TARGET_OS_IPHONE
OSStatus	CAAudioUnit::SetAUPresetFromDocument (CFPropertyListRef &inData)
{
	return AudioUnitSetProperty (AU(), kAudioUnitProperty_ClassInfoFromDocument,
								kAudioUnitScope_Global, 0,
								&inData, sizeof (CFPropertyListRef));
}
#endif

OSStatus	CAAudioUnit::GetPresentPreset (AUPreset &outData) const
{
	UInt32 dataSize = sizeof(outData);
	OSStatus result = AudioUnitGetProperty (AU(), kAudioUnitProperty_PresentPreset,
								kAudioUnitScope_Global, 0,
								&outData, &dataSize);
#if !TARGET_OS_IPHONE
#ifndef __LP64__ 
	if (result == kAudioUnitErr_InvalidProperty) {
		dataSize = sizeof(outData);
		result = AudioUnitGetProperty (AU(), kAudioUnitProperty_CurrentPreset,
									kAudioUnitScope_Global, 0,
									&outData, &dataSize);
		if (result == noErr) {
			// we now retain the CFString in the preset so for the client of this API
			// it is consistent (ie. the string should be released when done)
			if (outData.presetName)
				CFRetain (outData.presetName);
		}
	}
#endif
#endif
	return result;
}
	
OSStatus	CAAudioUnit::SetPresentPreset (AUPreset &inData)
{
	OSStatus result = AudioUnitSetProperty (AU(), kAudioUnitProperty_PresentPreset,
								kAudioUnitScope_Global, 0,
								&inData, sizeof (AUPreset));
#if !TARGET_OS_IPHONE
#ifndef __LP64__
	if (result == kAudioUnitErr_InvalidProperty) {
		result = AudioUnitSetProperty (AU(), kAudioUnitProperty_CurrentPreset,
								kAudioUnitScope_Global, 0,
								&inData, sizeof (AUPreset));
	}
#endif
#endif
	return result;
}

bool		CAAudioUnit::HasCustomView () const
{
#if !TARGET_OS_IPHONE
	UInt32 dataSize = 0;
	OSStatus result = -4/*unimpErr*/;
#ifndef __LP64__
	result = GetPropertyInfo(kAudioUnitProperty_GetUIComponentList,
                                        kAudioUnitScope_Global, 0,
                                        &dataSize, NULL);	
#endif
	if (result || !dataSize) {
		dataSize = 0;
		result = GetPropertyInfo(kAudioUnitProperty_CocoaUI,
                                        kAudioUnitScope_Global, 0,
                                        &dataSize, NULL);
		if (result || !dataSize)
			return false;
	}
	return true;
#else
	return false;
#endif

}

OSStatus		CAAudioUnit::GetParameter(AudioUnitParameterID inID, AudioUnitScope scope, AudioUnitElement element,
											Float32 &outValue) const
{
	return mDataPtr ? mDataPtr->GetParameter (inID, scope, element, outValue) : static_cast<OSStatus>(paramErr);
}

OSStatus		CAAudioUnit::SetParameter(AudioUnitParameterID inID, AudioUnitScope scope, AudioUnitElement element,
											Float32 value, UInt32 bufferOffsetFrames)
{
	return mDataPtr ? mDataPtr->SetParameter (inID, scope, element, value, bufferOffsetFrames) : static_cast<OSStatus>(paramErr);
}

OSStatus		CAAudioUnit::MIDIEvent (UInt32			inStatus,
								UInt32					inData1,
								UInt32					inData2,
								UInt32					inOffsetSampleFrame)
{
	return mDataPtr ? mDataPtr->MIDIEvent (inStatus, inData1, inData2, inOffsetSampleFrame) : paramErr;
}

OSStatus	CAAudioUnit::StartNote (MusicDeviceInstrumentID		inInstrument,
									MusicDeviceGroupID			inGroupID,
									NoteInstanceID *			outNoteInstanceID,
									UInt32						inOffsetSampleFrame,
									const MusicDeviceNoteParams * inParams)
{
	return mDataPtr ? mDataPtr->StartNote (inInstrument, inGroupID, outNoteInstanceID, inOffsetSampleFrame, inParams) 
					: paramErr;
}

OSStatus	CAAudioUnit::StopNote (MusicDeviceGroupID		inGroupID,
									NoteInstanceID				inNoteInstanceID,
									UInt32						inOffsetSampleFrame)
{
	return mDataPtr ? mDataPtr->StopNote (inGroupID, inNoteInstanceID, inOffsetSampleFrame) : paramErr;
}


#pragma mark __Render

OSStatus		CAAudioUnit::Render (AudioUnitRenderActionFlags 			* ioActionFlags,
												const AudioTimeStamp 		* inTimeStamp,
												UInt32						inOutputBusNumber,
												UInt32						inNumberFrames,
												AudioBufferList				* ioData)
{
	return mDataPtr ? mDataPtr->Render (ioActionFlags, inTimeStamp, inOutputBusNumber, inNumberFrames, ioData) : static_cast<OSStatus>(paramErr);
}

extern "C" OSStatus
AudioUnitProcess (					AudioUnit						inUnit, 
									AudioUnitRenderActionFlags *	ioActionFlags, 
									const AudioTimeStamp *			inTimeStamp, 
									UInt32							inNumberFrames, 
									AudioBufferList *				ioData);

OSStatus		CAAudioUnit::Process (AudioUnitRenderActionFlags 			& ioActionFlags,
												const AudioTimeStamp 		& inTimeStamp,
												UInt32						inNumberFrames,
												AudioBufferList				& ioData)
{
#if defined(__MAC_10_7) || defined(__IPHONE_4_0)
	return AudioUnitProcess (AU(), &ioActionFlags, &inTimeStamp, inNumberFrames, &ioData);
#else
	return -4/*unimpErr*/;
#endif
}

extern "C" OSStatus
AudioUnitProcessMultiple (			AudioUnit						inUnit, 
									AudioUnitRenderActionFlags *	ioActionFlags, 
									const AudioTimeStamp *			inTimeStamp, 
									UInt32							inNumberFrames,
									UInt32							inNumberInputBufferLists,
									const AudioBufferList **		inInputBufferLists,
									UInt32							inNumberOutputBufferLists,
									AudioBufferList **				ioOutputBufferLists);

OSStatus		CAAudioUnit::ProcessMultiple (AudioUnitRenderActionFlags 	& ioActionFlags,
									const AudioTimeStamp					& inTimeStamp,
									UInt32									inNumberFrames,
									UInt32									inNumberInputBufferLists,
									const AudioBufferList **				inInputBufferLists,
									UInt32									inNumberOutputBufferLists,
									AudioBufferList **						ioOutputBufferLists)
{
#if defined(__MAC_10_7) || defined(__IPHONE_4_0)
	return AudioUnitProcessMultiple (AU(), &ioActionFlags, &inTimeStamp, inNumberFrames, 
				inNumberInputBufferLists, inInputBufferLists, inNumberOutputBufferLists, ioOutputBufferLists);
#else
	return -4/*unimpErr*/;
#endif
}

#pragma mark __CAAUChanHelper

CAAUChanHelper::CAAUChanHelper(const CAAudioUnit &inAU, AudioUnitScope inScope)
	:mChans(NULL), mNumEls(0), mDidAllocate(false)
{
	UInt32 elCount;
	if (inAU.GetElementCount (inScope, elCount)) return;
	if (elCount > kStaticElCount) {
		mChans = new UInt32[elCount];
		mDidAllocate = true;
		memset (mChans, 0, sizeof(int) * elCount);
	} else {
		mChans = mStaticChans;
		memset (mChans, 0, sizeof(int) * kStaticElCount);
	}
	for (unsigned int i = 0; i < elCount; ++i) {
		UInt32 numChans;
		if (inAU.NumberChannels (inScope, i, numChans)) return;
		mChans[i] = numChans;
	}
	mNumEls = elCount;
}

CAAUChanHelper::CAAUChanHelper(UInt32 inMaxElems) 
	: mNumEls(inMaxElems), mDidAllocate(false) 
{
	if (inMaxElems > kStaticElCount) {
		mChans = new UInt32[inMaxElems];
		mDidAllocate = true;
		memset (mChans, 0, sizeof(int) * inMaxElems);
	} else {
		mChans = mStaticChans;
		memset (mChans, 0, sizeof(int) * kStaticElCount);
	}
}

CAAUChanHelper::~CAAUChanHelper()
{
	if (mDidAllocate) delete [] mChans;
}

CAAUChanHelper&		CAAUChanHelper::operator= (const CAAUChanHelper &c) 
{ 
	if (mDidAllocate) delete [] mChans;
	if (c.mDidAllocate) {
		mChans = new UInt32[c.mNumEls];
		mDidAllocate = true;
	} else {
		mDidAllocate = false;
		mChans = mStaticChans;
	}
	memcpy (mChans, c.mChans, c.mNumEls * sizeof(int));
	
	return *this; 
}


#pragma mark __Print Utilities

void		CAAudioUnit::Print (FILE* file) const
{
	fprintf (file, "AudioUnit:%p\n", AU());
	if (IsValid()) { 
		fprintf (file, "\tnode=%ld\t", (long)GetAUNode()); Comp().Print (file);
	}
}

#if CA_AU_USE_FAST_DISPATCH
// Handle  GetComponentInstanceStorage(ComponentInstance aComponentInstance)
static void *LoadGetComponentInstanceStorage (void *inst)
{
	typedef void* (*GetComponentInstanceStorageProc)(void* aComponentInstance);
	static GetComponentInstanceStorageProc sGetComponentInstanceStorageProc = NULL;
	
	static int sDoCSLoad = 1;
	if (sDoCSLoad) {
		sDoCSLoad = 0;
		void *theImage = dlopen("/System/Library/Frameworks/CoreServices.framework/CoreServices", RTLD_LAZY);
		if (!theImage) return NULL;
	
		sGetComponentInstanceStorageProc = (GetComponentInstanceStorageProc) dlsym(theImage, "GetComponentInstanceStorage");
	}
	if (sGetComponentInstanceStorageProc)
		return (*sGetComponentInstanceStorageProc)(inst);
	return NULL;
}
#endif


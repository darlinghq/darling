/*
     File: AUScopeElement.h
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
#ifndef __AUScopeElement_h__
#define __AUScopeElement_h__

#include <map>
#include <vector>

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioUnit/AudioUnit.h>
#else
	#include <AudioUnit.h>
#endif
#include "ComponentBase.h"
#include "AUBuffer.h"


class AUBase;

// ____________________________________________________________________________
//
// represents a parameter's value (either constant or ramped)
/*! @class ParameterMapEvent */
class ParameterMapEvent
{
public:
/*! @ctor ParameterMapEvent */
	ParameterMapEvent() 
		: mEventType(kParameterEvent_Immediate), mBufferOffset(0), mDurationInFrames(0), mValue1(0.0f), mValue2(0.0f), mSliceDurationFrames(0) 
		{}

/*! @ctor ParameterMapEvent */
	ParameterMapEvent(AudioUnitParameterValue inValue)
		: mEventType(kParameterEvent_Immediate), mBufferOffset(0), mDurationInFrames(0), mValue1(inValue), mValue2(inValue), mSliceDurationFrames(0) 
		{}
		
	// constructor for scheduled event
/*! @ctor ParameterMapEvent */
	ParameterMapEvent(	const AudioUnitParameterEvent 	&inEvent,
						UInt32 							inSliceOffsetInBuffer,
						UInt32							inSliceDurationFrames )
	{
		SetScheduledEvent(inEvent, inSliceOffsetInBuffer, inSliceDurationFrames );
	};
	
/*! @method SetScheduledEvent */
	void SetScheduledEvent(	const AudioUnitParameterEvent 	&inEvent,
							UInt32 							inSliceOffsetInBuffer,
							UInt32							inSliceDurationFrames )
	{
		mEventType = inEvent.eventType;
		mSliceDurationFrames = inSliceDurationFrames;
		
		if(mEventType == kParameterEvent_Immediate )
		{
			// constant immediate value for the whole slice
			mValue1 = inEvent.eventValues.immediate.value;
			mValue2 = mValue1;
			mDurationInFrames = inSliceDurationFrames;
			mBufferOffset = 0;
		}
		else
		{
			mDurationInFrames 	= 	inEvent.eventValues.ramp.durationInFrames;
			mBufferOffset 		= 	inEvent.eventValues.ramp.startBufferOffset - inSliceOffsetInBuffer;	// shift over for this slice
			mValue1 			= 	inEvent.eventValues.ramp.startValue;
			mValue2 			= 	inEvent.eventValues.ramp.endValue;
		}
	};
	
	
	
/*! @method GetEventType */
	AUParameterEventType		GetEventType() const {return mEventType;};

/*! @method GetValue */
	AudioUnitParameterValue		GetValue() const {return mValue1;};	// only valid if immediate event type
/*! @method GetEndValue */
	AudioUnitParameterValue		GetEndValue() const {return mValue2;};	// only valid if immediate event type
/*! @method SetValue */
	void						SetValue(AudioUnitParameterValue inValue) 
								{
									mEventType = kParameterEvent_Immediate; 
									mValue1 = inValue; 
									mValue2 = inValue;
								}
	
	// interpolates the start and end values corresponding to the current processing slice
	// most ramp parameter implementations will want to use this method
	// the start value will correspond to the start of the slice
	// the end value will correspond to the end of the slice
/*! @method GetRampSliceStartEnd */
	void					GetRampSliceStartEnd(	AudioUnitParameterValue &	outStartValue,
													AudioUnitParameterValue &	outEndValue,
													AudioUnitParameterValue &	outValuePerFrameDelta )
	{
		if (mEventType == kParameterEvent_Ramped) {
			outValuePerFrameDelta = (mValue2 - mValue1) / mDurationInFrames;
		
			outStartValue = mValue1 + outValuePerFrameDelta * (-mBufferOffset);	// corresponds to frame 0 of this slice
			outEndValue = outStartValue +  outValuePerFrameDelta * mSliceDurationFrames;
		} else {
			outValuePerFrameDelta = 0;
			outStartValue = outEndValue = mValue1;
		}
	};

	// Some ramp parameter implementations will want to interpret the ramp using their
	// own interpolation method (perhaps non-linear)
	// This method gives the raw ramp information, relative to this processing slice
	// for the client to interpret as desired
/*! @method GetRampInfo */
	void					GetRampInfo(	SInt32 	&					outBufferOffset,
											UInt32 	&					outDurationInFrames,
											AudioUnitParameterValue &	outStartValue,
											AudioUnitParameterValue &	outEndValue )
	{
		outBufferOffset = mBufferOffset;
		outDurationInFrames = mDurationInFrames;
		outStartValue = mValue1;
		outEndValue = mValue2;
	};

#if DEBUG
	void					Print()
	{
		printf("ParameterEvent @ %p\n", this);
		printf("	mEventType = %d\n", (int)mEventType);
		printf("	mBufferOffset = %d\n", (int)mBufferOffset);
		printf("	mDurationInFrames = %d\n", (int)mDurationInFrames);
		printf("	mSliceDurationFrames = %d\n", (int)mSliceDurationFrames);
		printf("	mValue1 = %.5f\n", mValue1);
		printf("	mValue2 = %.5f\n", mValue2);
	}
#endif

private:	
	AUParameterEventType		mEventType;
	
	SInt32						mBufferOffset;		// ramp start offset relative to start of this slice (may be negative)
	UInt32						mDurationInFrames;	// total duration of ramp parameter
	AudioUnitParameterValue     mValue1;				// value if immediate : startValue if ramp
	AudioUnitParameterValue		mValue2;				// endValue (only used for ramp)
	
	UInt32					mSliceDurationFrames;	// duration of this processing slice 
};



// ____________________________________________________________________________
//
class AUIOElement;

/*! @class AUElement */
class AUElement {
public:
/*! @ctor AUElement */
								AUElement(AUBase *audioUnit) : mAudioUnit(audioUnit),
									mUseIndexedParameters(false), mElementName(0) { }
	
/*! @dtor ~AUElement */
	virtual						~AUElement() { if (mElementName) CFRelease (mElementName); }
	
/*! @method GetNumberOfParameters */
	virtual UInt32				GetNumberOfParameters()
	{
		if(mUseIndexedParameters) return static_cast<UInt32>(mIndexedParameters.size()); else return static_cast<UInt32>(mParameters.size());
	}
/*! @method GetParameterList */
	virtual void				GetParameterList(AudioUnitParameterID *outList);
/*! @method HasParameterID */
	bool						HasParameterID (AudioUnitParameterID paramID) const;
	
/*! @method GetParameter */
	AudioUnitParameterValue		GetParameter(AudioUnitParameterID paramID);
/*! @method SetParameter */
	void						SetParameter(AudioUnitParameterID paramID, AudioUnitParameterValue value, bool okWhenInitialized = false);
	// Only set okWhenInitialized to true when you know the outside world cannot access this element. Otherwise the parameter map could get corrupted. 

	// interpolates the start and end values corresponding to the current processing slice
	// most ramp parameter implementations will want to use this method
/*! @method GetRampSliceStartEnd */
	void						GetRampSliceStartEnd(	AudioUnitParameterID	paramID,
													AudioUnitParameterValue &	outStartValue,
													AudioUnitParameterValue &	outEndValue,
													AudioUnitParameterValue &	outValuePerFrameDelta );
													
/*! @method GetEndValue */
	AudioUnitParameterValue		GetEndValue(	AudioUnitParameterID		paramID);

/*! @method SetRampParameter */
	void						SetScheduledEvent(	AudioUnitParameterID 			paramID,
													const AudioUnitParameterEvent 	&inEvent,
													UInt32 							inSliceOffsetInBuffer,
													UInt32							inSliceDurationFrames,
													bool							okWhenInitialized = false );
	// Only set okWhenInitialized to true when you know the outside world cannot access this element. Otherwise the parameter map could get corrupted. 


/*! @method GetAudioUnit */
	AUBase *					GetAudioUnit() const { return mAudioUnit; };

/*! @method SaveState */
	void						SaveState(CFMutableDataRef data);
/*! @method RestoreState */
	const UInt8 *				RestoreState(const UInt8 *state);
/*! @method GetName */
	CFStringRef					GetName () const { return mElementName; }
/*! @method SetName */
	void						SetName (CFStringRef inName);
/*! @method HasName */
	bool						HasName () const { return mElementName != 0; }
/*! @method UseIndexedParameters */
	virtual void				UseIndexedParameters(int inNumberOfParameters);

/*! @method AsIOElement*/
	virtual AUIOElement*		AsIOElement () { return NULL; }
	
protected:
	inline ParameterMapEvent&	GetParamEvent(AudioUnitParameterID paramID);
	
private:
	typedef std::map<AudioUnitParameterID, ParameterMapEvent, std::less<AudioUnitParameterID> > ParameterMap;
	
/*! @var mAudioUnit */
	AUBase *						mAudioUnit;
/*! @var mParameters */
	ParameterMap					mParameters;

/*! @var mUseIndexedParameters */
	bool							mUseIndexedParameters;
/*! @var mIndexedParameters */
	std::vector<ParameterMapEvent>	mIndexedParameters;
	
/*! @var mElementName */
	CFStringRef						mElementName;
};



// ____________________________________________________________________________
//
/*! @class AUIOElement */
class AUIOElement : public AUElement {
public:
/*! @ctor AUIOElement */
								AUIOElement(AUBase *audioUnit);

/*! @method GetStreamFormat */
	const CAStreamBasicDescription &GetStreamFormat() const { return mStreamFormat; }
	
/*! @method SetStreamFormat */
	virtual OSStatus			SetStreamFormat(const CAStreamBasicDescription &desc);

/*! @method AllocateBuffer */
	virtual void				AllocateBuffer(UInt32 inFramesToAllocate = 0);
/*! @method DeallocateBuffer */
	void						DeallocateBuffer();
/*! @method NeedsBufferSpace */
	virtual bool				NeedsBufferSpace() const = 0;

/*! @method SetWillAllocateBuffer */
	void						SetWillAllocateBuffer(bool inFlag) { 
									mWillAllocate = inFlag; 
								}
/*! @method WillAllocateBuffer */
	bool						WillAllocateBuffer() const { 
									return mWillAllocate; 
								}
	
/*! @method UseExternalBuffer */
	void						UseExternalBuffer(const AudioUnitExternalBuffer &buf) {
									mIOBuffer.UseExternalBuffer(mStreamFormat, buf);
								}
/*! @method PrepareBuffer */
	AudioBufferList &			PrepareBuffer(UInt32 nFrames) {
									if (mWillAllocate)
										return mIOBuffer.PrepareBuffer(mStreamFormat, nFrames);
									throw OSStatus(kAudioUnitErr_InvalidPropertyValue);
								}
/*! @method PrepareNullBuffer */
	AudioBufferList &			PrepareNullBuffer(UInt32 nFrames) {
									return mIOBuffer.PrepareNullBuffer(mStreamFormat, nFrames);
								}
/*! @method SetBufferList */
	AudioBufferList &			SetBufferList(AudioBufferList &abl) { return mIOBuffer.SetBufferList(abl); }
/*! @method SetBuffer */
	void						SetBuffer(UInt32 index, AudioBuffer &ab) { mIOBuffer.SetBuffer(index, ab); }
/*! @method InvalidateBufferList */
	void						InvalidateBufferList() { mIOBuffer.InvalidateBufferList(); }

/*! @method GetBufferList */
	AudioBufferList &			GetBufferList() const { return mIOBuffer.GetBufferList(); }

/*! @method GetChannelData */
	AudioUnitSampleType *		GetChannelData(int ch) const {
									if (mStreamFormat.IsInterleaved())
										return static_cast<AudioUnitSampleType *>(mIOBuffer.GetBufferList().mBuffers[0].mData) + ch;
									else
										return static_cast<AudioUnitSampleType *>(mIOBuffer.GetBufferList().mBuffers[ch].mData);
								}
	Float32 *					GetFloat32ChannelData(int ch) const {
									if (mStreamFormat.IsInterleaved())
										return static_cast<Float32 *>(mIOBuffer.GetBufferList().mBuffers[0].mData) + ch;
									else
										return static_cast<Float32 *>(mIOBuffer.GetBufferList().mBuffers[ch].mData);
								}
	SInt32 *					GetSInt32ChannelData(int ch) const {
									if (mStreamFormat.IsInterleaved())
										return static_cast<SInt32 *>(mIOBuffer.GetBufferList().mBuffers[0].mData) + ch;
									else
										return static_cast<SInt32 *>(mIOBuffer.GetBufferList().mBuffers[ch].mData);
								}
	SInt16 *					GetInt16ChannelData(int ch) const {
									if (mStreamFormat.IsInterleaved())
										return static_cast<SInt16 *>(mIOBuffer.GetBufferList().mBuffers[0].mData) + ch;
									else
										return static_cast<SInt16 *>(mIOBuffer.GetBufferList().mBuffers[ch].mData);
								}

/*! @method CopyBufferListTo */
	void						CopyBufferListTo(AudioBufferList &abl) const {
									mIOBuffer.CopyBufferListTo(abl);
								}
/*! @method CopyBufferContentsTo */
	void						CopyBufferContentsTo(AudioBufferList &abl) const {
									mIOBuffer.CopyBufferContentsTo(abl);
								}

/*	UInt32						BytesToFrames(UInt32 nBytes) { return nBytes / mStreamFormat.mBytesPerFrame; }
	UInt32						BytesToFrames(AudioBufferList &abl) {
									return BytesToFrames(abl.mBuffers[0].mDataByteSize);
								}
	UInt32						FramesToBytes(UInt32 nFrames) { return nFrames * mStreamFormat.mBytesPerFrame; }*/

/*! @method IsInterleaved */
	bool						IsInterleaved() const { return mStreamFormat.IsInterleaved(); }
/*! @method NumberChannels */
	UInt32						NumberChannels() const { return mStreamFormat.NumberChannels(); }
/*! @method NumberInterleavedChannels */
	UInt32						NumberInterleavedChannels() const { return mStreamFormat.NumberInterleavedChannels(); }

/*! @method GetChannelMapTags */
	virtual UInt32				GetChannelLayoutTags (AudioChannelLayoutTag	*outLayoutTagsPtr);

/*! @method GetAudioChannelLayout */
	virtual UInt32				GetAudioChannelLayout (AudioChannelLayout	*outMapPtr, Boolean	&outWritable);

/*! @method SetAudioChannelLayout */
	virtual OSStatus			SetAudioChannelLayout (const AudioChannelLayout &inData);
		
/*! @method RemoveAudioChannelLayout */
	virtual OSStatus			RemoveAudioChannelLayout ();

/*! @method AsIOElement*/
	virtual AUIOElement*		AsIOElement () { return this; }

protected:
/*! @var mStreamFormat */
	CAStreamBasicDescription	mStreamFormat;
/*! @var mIOBuffer */
	AUBufferList				mIOBuffer;	// for input: input proc buffer, only allocated when needed
											// for output: output cache, usually allocated early on
/*! @var mWillAllocate */
	bool						mWillAllocate;
};

// ____________________________________________________________________________
//
// AUScopeDelegates are a way to get virtual scopes.
/*! @class AUScopeDelegate */
class AUScopeDelegate {
public:
/*! @ctor AUScopeDelegate */
					AUScopeDelegate() : mCreator(NULL), mScope(0) { }	
/*! @dtor ~AUScopeDelegate */
					virtual ~AUScopeDelegate() {}
	
/*! @method Initialize */
	void					Initialize(	AUBase *creator, 
										AudioUnitScope scope, 
										UInt32 numElements)
	{
		mCreator = creator;
		mScope = scope;
		SetNumberOfElements(numElements);
	}
	
/*! @method SetNumberOfElements */
	virtual void			SetNumberOfElements(UInt32 numElements) = 0;
	
/*! @method GetNumberOfElements */
	virtual UInt32			GetNumberOfElements()	 = 0;
	
/*! @method GetElement */
	virtual AUElement *		GetElement(UInt32 elementIndex) = 0;
	
	AUBase *			GetCreator() const { return mCreator; }
	AudioUnitScope		GetScope() const { return mScope; }
	

private:
/*! @var mCreator */
	AUBase *					mCreator;
/*! @var mScope */
	AudioUnitScope				mScope;
};



// ____________________________________________________________________________
//
/*! @class AUScope */
class AUScope {
public:
/*! @ctor AUScope */
					AUScope() : mCreator(NULL), mScope(0), mDelegate(0) { }	
/*! @dtor ~AUScope */
					~AUScope();
	
/*! @method Initialize */
	void			Initialize(AUBase *creator, 
								AudioUnitScope scope, 
								UInt32 numElements)
	{
		mCreator = creator;
		mScope = scope;

		if (mDelegate)
			return mDelegate->Initialize(creator, scope, numElements);
			
		SetNumberOfElements(numElements);
	}
	
/*! @method SetNumberOfElements */
	void			SetNumberOfElements(UInt32 numElements);
	
/*! @method GetNumberOfElements */
	UInt32			GetNumberOfElements()	const	
	{
		if (mDelegate)
			return mDelegate->GetNumberOfElements();
			
		return static_cast<UInt32>(mElements.size());
	}
	
/*! @method GetElement */
	AUElement *		GetElement(UInt32 elementIndex) const
	{
		if (mDelegate)
			return mDelegate->GetElement(elementIndex);

		ElementVector::const_iterator i = mElements.begin() + elementIndex;
			// catch passing -1 in as the elementIndex - causes a wrap around
		return (i >= mElements.end() || i < mElements.begin()) ? NULL : *i;
	}
	
/*! @method SafeGetElement */
	AUElement *		SafeGetElement(UInt32 elementIndex)
	{
		AUElement *element = GetElement(elementIndex);
		if (element == NULL)
			COMPONENT_THROW(kAudioUnitErr_InvalidElement);
		return element;
	}
	
/*! @method GetIOElement */
	AUIOElement *	GetIOElement(UInt32 elementIndex) const
	{
		AUElement *element = GetElement(elementIndex);
		AUIOElement *ioel = element ? element->AsIOElement () : NULL;
		if (!ioel)
			COMPONENT_THROW (kAudioUnitErr_InvalidElement);
		return ioel;
	}
	
/*! @method HasElementWithName */
	bool			HasElementWithName () const;
	
/*! @method AddElementNamesToDict */
	void			AddElementNamesToDict (CFMutableDictionaryRef & inNameDict);
	
	bool			RestoreElementNames (CFDictionaryRef& inNameDict);
	
	AudioUnitScope		GetScope() const { return mScope; }

	void SetDelegate(AUScopeDelegate* inDelegate) { mDelegate = inDelegate; }

/*! @method SaveState */
    void            SaveState(CFMutableDataRef data);

/*! @method RestoreState */
    const UInt8 *	RestoreState(const UInt8 *state);
	
private:
	typedef std::vector<AUElement *> ElementVector;
/*! @var mCreator */
	AUBase *					mCreator;
/*! @var mScope */
	AudioUnitScope				mScope;
/*! @var mElements */
	ElementVector				mElements;
/*! @var mDelegate */
	AUScopeDelegate *			mDelegate;
};



#endif // __AUScopeElement_h__

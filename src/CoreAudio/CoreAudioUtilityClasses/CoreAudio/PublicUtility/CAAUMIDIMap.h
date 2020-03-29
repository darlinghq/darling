/*
     File: CAAUMIDIMap.h
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
#ifndef __CAAUMIDIMap_h_
#define __CAAUMIDIMap_h_

#include <AudioUnit/AudioUnitProperties.h>
#include <algorithm>

/*
enum {
	kAUParameterMIDIMapping_AnyChannelFlag		= (1L << 0),
		// If this flag is set and mStatus is a MIDI channel message, then the MIDI channel number 
		// in the status byte is ignored; the mapping is from the specified MIDI message on ANY channel.

	kAUParameterMIDIMapping_AnyNoteFlag			= (1L << 1),
		// If this flag is set and mStatus is a Note On, Note Off, or Polyphonic Pressure message,
		// the message's note number is ignored; the mapping is from ANY note number.

	kAUParameterMIDIMapping_SubRange			= (1L << 2),
		// set this flag if the midi control should map only to a sub-range of the parameter's value
		// then specify that range in the mSubRangeMin and mSubRangeMax members

	kAUParameterMIDIMapping_Toggle				= (1L << 3),
		// this is only useful for boolean typed parameters. When set, it means that the parameter's
		// value should be toggled (if true, become false and vice versa) when the represented MIDI message
		// is received
	
	kAUParameterMIDIMapping_Bipolar				= (1L << 4),
		// this can be set to when mapping a MIDI Controller to indicate that the parameter (typically a boolean
		// style parameter) will only have its value changed to either the on or off state of a MIDI controller message
		// (0 < 64 is off, 64 < 127 is on) such as the sustain pedal. The seeting of the next flag
		// (kAUParameterMIDIMapping_Bipolar_On) determine whether the parameter is mapped to the on or off
		// state of the controller
	kAUParameterMIDIMapping_Bipolar_On			= (1L << 5)
		// only a valid flag if kAUParameterMIDIMapping_Bipolar is set
};

// The reserved fields here are being used to reserve space (as well as align to 64 bit size) for future use
// When/If these fields are used, the names of the fields will be changed to reflect their functionality
// so, apps should NOT refer to these reserved fields directly by name
typedef struct AUParameterMIDIMapping
{
	AudioUnitScope			mScope;
	AudioUnitElement		mElement;
	AudioUnitParameterID	mParameterID;
	UInt32					mFlags;
	Float32					mSubRangeMin;
	Float32					mSubRangeMax;
	UInt8					mStatus;
	UInt8					mData1;
	UInt8					reserved1; // MUST be set to zero
	UInt8					reserved2; // MUST be set to zero
	UInt32					reserved3; // MUST be set to zero
} AUParameterMIDIMapping;
*/

/*
Parameter To MIDI Mapping Properties
These properties are used to:
Describe a current set of mappings between MIDI messages and Parameter value setting
Create a mapping between a parameter and a MIDI message through either:
- explicitly adding (or removing) the mapping
- telling the AU to hot-map the next MIDI message to a specified Parameter
	The same MIDI Message can map to one or more parameters
	One Parameter can be mapped from multiple MIDI messages

	In general usage, these properties only apply to AU's that implement the MIDI API
	AU Instruments (type=='aumu') and Music Effects (type == 'aumf')

	These properties are used in the Global scope. The scope and element members of the structure describe
	the scope and element of the parameter. In all usages, mScope, mElement and mParameterID must be
	correctly specified.


	* The AUParameterMIDIMapping Structure

	Command				mStatus			mData1			
	Note Off			0x8n			Note Num		
	Note On				0x9n			Note Num		
	Key Pressure		0xAn			Note Num		
	Control Change		0xBn			ControllerID	
	Patch Change		0xCn			Patch Num		
	Channel Pressure	DxDn			0 (Unused)		
	Pitch Bend			0xEn			0 (Unused)		

	(where n is 0-0xF to correspond to MIDI channels 1-16)

		Details:

	In general MIDI Commands can be mapped to either a specific channel as specified in the mStatus bit.
	If the kAUParameterMIDIMapping_AnyChannelFlag bit is set mStatus is a MIDI channel message, then the 
	MIDI channel number in the status byte is ignored; the mapping is from the specified MIDI message on ANY channel.

	For note commands (note on, note off, key pressure), the MIDI message can trigger either with just a specific
	note number, or any note number if the kAUParameterMIDIMapping_AnyNoteFlag bit is set. In these instances, the
	note number is used as the trigger value (for instance, a note message could be used to set the 
											  cut off frequency of a filter).

 The Properties:								

	kAudioUnitProperty_AllParameterMIDIMappings							array of AUParameterMIDIMapping (read/write)
	This property is used to both retreive and set the current mapping state between (some/many/all of) its parameters
	and MIDI messages. When set, it should replace any previous mapped settings the AU had.

	If this property is implemented by a non-MIDI capable AU (such as an 'aufx' type), then the property is
	read only, and recommends a suggested set of mappings for the host to perform. In this case, it is the 
	host's responsibility to map MIDI message to the AU parameters. As described previously, there are a set
	of default mappings (see AudioToolbox/AUMIDIController.h) that the host can recommend to the user 
	in this circumstance.

	This property's size will be very dynamic, depending on the number of mappings currently in affect, so the 
	caller should always get the size of the property first before retrieving it. The AU should return an error
	if the caller doesn't provide enough space to return all of the current mappings.

	kAudioUnitProperty_AddParameterMIDIMapping							array of AUParameterMIDIMapping (write only)
	This property is used to Add mappings to the existing set of mappings the AU possesses. It does NOT replace
	any existing mappings.

	kAudioUnitProperty_RemoveParameterMIDIMapping						array of AUParameterMIDIMapping (write only)
	This property is used to remove the specified mappings from the AU. If a mapping is specified that does not
	currently exist in the AU, then it should just be ignored.

	kAudioUnitProperty_HotMapParameterMIDIMapping								AUParameterMIDIMapping (read/write)
	This property is used in two ways, determined by the value supplied by the caller.
	(1) If a mapping struct is provided, then that struct provides *all* of the information that the AU should
	use to map the parameter, *except* for the MIDI message. The AU should then listen for the next MIDI message
	and associate that MIDI message with the supplied AUParameter mapping. When this MIDI message is received and
	the mapping made, the AU should also issue a notification on this property 
	(kAudioUnitProperty_HotMapParameterMIDIMapping) to indicate to the host that the mapping has been made. The host
	can then retrieve the mapping that was made by getting the value of this property.

	To avoid possible confusion, it is recommended that once the host has retrieved this mapping (if it is 
	presenting a UI to describe the mappings for example), that it then clears the mapping state as described next.

	Thus, the only time this property will return a valid value is when the AU has made a mapping. If the AU's mapping
	state has been cleared (or it has not been asked to make a mapping), then the AU should return 
	kAudioUnitErr_InvalidPropertyValue if the host tries to read this value.

	(2) If the value passed in is NULL, then if the AU had a parameter that it was in the process of mapping, it
	should disregard that (stop listening to the MIDI messages to create a mapping) and discard the partially 
	mapped struct. If the value is NULL and the AU is not in the process of mapping, the AU can ignore the request.

	At all times, the _AllMappings property will completely describe the current known state of the AU's mappings
	of MIDI messages to parameters.
*/


/*
	When mapping, it is recommended that LSB controllers are in general not mapped (ie. the controller range of 32 < 64)
	as many host parsers will map 14 bit control values. If you know (or can present an option) that the host deals with
	7 bit controllers only, then these controller ID's can be mapped of course.
*/


struct MIDIValueTransformer {
	virtual double  tolinear(double) = 0;
	virtual double  fromlinear(double) = 0;
#if DEBUG
	// suppress warning
	virtual ~MIDIValueTransformer() { }
#endif
};

struct MIDILinearTransformer : public MIDIValueTransformer {
	virtual double  tolinear(double x) { return x; }
	virtual double  fromlinear(double x) { return x; }
};

struct MIDILogTransformer : public MIDIValueTransformer {
	virtual double  tolinear(double x) { return log(std::max(x, .00001)); }
	virtual double  fromlinear(double x) { return exp(x); }
};

struct MIDIExpTransformer : public MIDIValueTransformer {
	virtual double  tolinear(double x) { return exp(x); }
	virtual double  fromlinear(double x) { return log(std::max(x, .00001)); }
};

struct MIDISqrtTransformer : public MIDIValueTransformer {
	virtual double  tolinear(double x) { return x < 0. ? -(sqrt(-x)) : sqrt(x); }
	virtual double  fromlinear(double x) { return x < 0. ? -(x * x) : x * x; }
};

struct MIDISquareTransformer : public MIDIValueTransformer {
	virtual double  tolinear(double x) { return x < 0. ? -(x * x) : x * x; }
	virtual double  fromlinear(double x) { return x < 0. ? -(sqrt(-x)) : sqrt(x); }
};

struct MIDICubeRtTransformer : public MIDIValueTransformer {
	virtual double  tolinear(double x) { return x < 0. ? -(pow(-x, 1./3.)) : pow(x, 1./3.); }
	virtual double  fromlinear(double x) { return x * x * x; }
};

struct MIDICubeTransformer : public MIDIValueTransformer {
	virtual double  tolinear(double x) { return x * x * x; }
	virtual double  fromlinear(double x) { return x < 0. ? -(pow(-x, 1./3.)) : pow(x, 1./3.); }
};


class CAAUMIDIMap : public AUParameterMIDIMapping {
	
public:
// variables for more efficient parsing of MIDI to Param value	
	Float32						mMinValue;
	Float32						mMaxValue;
	MIDIValueTransformer		*mTransType;

// methods	
	static MIDIValueTransformer *GetTransformer (UInt32 inFlags);
	
								CAAUMIDIMap() { memset(this, 0, sizeof(CAAUMIDIMap)); }
								CAAUMIDIMap (const AUParameterMIDIMapping& inMap) 
								{
									memset(this, 0, sizeof(CAAUMIDIMap));
									memcpy (this, &inMap, sizeof(inMap));
								}
								CAAUMIDIMap (AudioUnitScope inScope, AudioUnitElement inElement, AudioUnitParameterID inParam) 
								{ 
									memset(this, 0, sizeof(CAAUMIDIMap)); 
									mScope = inScope;
									mElement = inElement;
									mParameterID = inParam;
								}


	bool						IsValid () const { return mStatus != 0; }

	// returns -1 if any channel bit is set
	SInt32						Channel () const { return IsAnyChannel() ? -1 : (mStatus & 0xF); }
	bool						IsAnyChannel () const { 
									return mFlags & kAUParameterMIDIMapping_AnyChannelFlag; 
								}
									// preserves the existing channel info in the status byte
									// preserves any previously set mFlags value
	void						SetAnyChannel (bool inFlag) 
								{ 
									if (inFlag) 
										mFlags |= kAUParameterMIDIMapping_AnyChannelFlag; 
									else
										mFlags &= ~kAUParameterMIDIMapping_AnyChannelFlag;
								}

	bool						IsAnyNote () const { 		
									return (mFlags & kAUParameterMIDIMapping_AnyNoteFlag) != 0;
								}
									// preserves the existing key num in the mData1 byte
									// preserves any previously set mFlags value
	void						SetAnyNote (bool inFlag)
								{ 
									if (inFlag) 
										mFlags |= kAUParameterMIDIMapping_AnyNoteFlag; 
									else
										mFlags &= ~kAUParameterMIDIMapping_AnyNoteFlag;
								}
									
	bool						IsToggle() const { return (mFlags & kAUParameterMIDIMapping_Toggle) != 0; }
	void						SetToggle (bool inFlag)
								{
									if (inFlag) 
										mFlags |= kAUParameterMIDIMapping_Toggle; 
									else
										mFlags &= ~kAUParameterMIDIMapping_Toggle;
								}
	
	bool						IsBipolar() const { return (mFlags & kAUParameterMIDIMapping_Bipolar) != 0; }
									// inUseOnValue is valid ONLY if inFlag is true
	void						SetBipolar (bool inFlag, bool inUseOnValue = false)
								{
									if (inFlag) {
										mFlags |= kAUParameterMIDIMapping_Bipolar;
										if (inUseOnValue)
											mFlags |= kAUParameterMIDIMapping_Bipolar_On;
										else
											mFlags &= ~kAUParameterMIDIMapping_Bipolar_On;
									} else {
										mFlags &= ~kAUParameterMIDIMapping_Bipolar;
										mFlags &= ~kAUParameterMIDIMapping_Bipolar_On;
									}
								}
	bool						IsBipolar_OnValue () const { return (mFlags & kAUParameterMIDIMapping_Bipolar_On) != 0; }

	bool						IsSubRange () const { return (mFlags & kAUParameterMIDIMapping_SubRange) != 0; }
	void						SetSubRange (Float32 inStartValue, Float32 inStopValue)
								{
									mFlags |= kAUParameterMIDIMapping_SubRange; 
									
									mSubRangeMin = inStartValue;
									mSubRangeMax = inStopValue;
								}
	
	void						SetParamRange(Float32 minValue, Float32 maxValue)
								{
									mMinValue = minValue;
									mMaxValue = maxValue;		
								}
									
								// this will retain the subrange values previously set.
	void						SetSubRange (bool inFlag) 
								{ 
									if (inFlag)
										mFlags |= kAUParameterMIDIMapping_SubRange; 
									else
										mFlags &= ~kAUParameterMIDIMapping_SubRange; 
								}
	
	bool						IsAnyValue() const{return !IsBipolar();}
	bool						IsOnValue() const{return IsBipolar_OnValue();}
	bool						IsOffValue() const{return IsBipolar();}
								
	bool						IsNoteOff () const { return ((mStatus & 0xF0) == 0x80); }
	bool						IsNoteOn () const { return ((mStatus & 0xF0) == 0x90); }
	
	bool						IsKeyPressure () const { return ((mStatus & 0xF0) == 0xA0); }
	
	bool						IsKeyEvent () const { return (mStatus > 0x7F) && (mStatus < 0xB0); }
	
	bool						IsPatchChange () const { return ((mStatus & 0xF0) == 0xC0); }
	bool						IsChannelPressure () const { return ((mStatus & 0xF0) == 0xD0); }
	bool						IsPitchBend () const { return ((mStatus & 0xF0) == 0xE0); }
	bool						IsControlChange () const { return ((mStatus & 0xF0) == 0xB0); }
	
	
	void						SetControllerOnValue(){SetBipolar(true,true);}
	void						SetControllerOffValue(){SetBipolar(true,false);}
	void						SetControllerAnyValue(){SetBipolar(false,false);}
								
	// All of these Set calls will reset the mFlags field based on the 
	// anyChannel param value
	void						SetNoteOff (UInt8 key, SInt8 channel, bool anyChannel = false)
								{
									mStatus = 0x80 | (channel & 0xF);
									mData1 = key;
									mFlags = (anyChannel ? kAUParameterMIDIMapping_AnyChannelFlag : 0);
									
								}

	void						SetNoteOn (UInt8 key, SInt8 channel, bool anyChannel = false)
								{
									mStatus = 0x90 | (channel & 0xF);
									mData1 = key;
									mFlags = (anyChannel ? kAUParameterMIDIMapping_AnyChannelFlag : 0);
								}

	void						SetPolyKey (UInt8 key, SInt8 channel, bool anyChannel = false)
								{
									mStatus = 0xA0 | (channel & 0xF);
									mData1 = key;
									mFlags = (anyChannel ? kAUParameterMIDIMapping_AnyChannelFlag : 0);
								}

	void						SetControlChange (UInt8 controllerID, SInt8 channel, bool anyChannel = false)
								{
									mStatus = 0xB0 | (channel & 0xF);
									mData1 = controllerID;
									mFlags = (anyChannel ? kAUParameterMIDIMapping_AnyChannelFlag : 0);
								}
	
	void						SetPatchChange (UInt8 patchChange, SInt8 channel, bool anyChannel = false)
								{
									mStatus = 0xC0 | (channel & 0xF);
									mData1 = patchChange;
									mFlags = (anyChannel ? kAUParameterMIDIMapping_AnyChannelFlag : 0);
								}

	void						SetChannelPressure (SInt8 channel, bool anyChannel = false)
								{
									mStatus = 0xD0 | (channel & 0xF);
									mData1 = 0;
									mFlags = (anyChannel ? kAUParameterMIDIMapping_AnyChannelFlag : 0);
								}

	void						SetPitchBend (SInt8 channel, bool anyChannel = false)
								{
									mStatus = 0xE0 | (channel & 0xF);
									mData1 = 0;
									mFlags = (anyChannel ? kAUParameterMIDIMapping_AnyChannelFlag : 0);
								}
	
	
	Float32						ParamValueFromMIDILinear (Float32		inLinearValue) const
	{
								Float32 low, high;
								if (IsSubRange()){
									low = mSubRangeMin;
									high = mSubRangeMax;
								}
								else {
									low = mMinValue;
									high = mMaxValue;
								}
								
								
								// WE ARE ASSUMING YOU HAVE SET THIS UP PROPERLY!!!!! (or this will crash cause it will be NULL)
								return (Float32)mTransType->fromlinear((inLinearValue * (high - low)) + low);
	}
		

		// The CALLER of this method must ensure that the status byte's MIDI Command (ignoring the channel) matches!!!
	bool						MIDI_Matches (UInt8 inChannel, UInt8 inData1, UInt8 inData2, Float32 &outLinear) const;
	
	void						Print () const;
	
	void						Save (CFPropertyListRef &outData) const;
	void						Restore (CFDictionaryRef inData);
	
	static void					SaveAsMapPList (AudioUnit						inUnit, 
											const AUParameterMIDIMapping		* inMappings, 
											UInt32								inNumMappings, 
											CFPropertyListRef					&outData,
											CFStringRef							inName = NULL);

									// inNumMappings describes how much memory is allocated in outMappings
	static void					RestoreFromMapPList (const CFDictionaryRef			inData, 
														AUParameterMIDIMapping		* outMappings, 
														UInt32						inNumMappings);
														
	static UInt32				NumberOfMaps (const CFDictionaryRef inData);
};


	// these sorting operations sort for run-time efficiency based on the MIDI messages
inline bool operator== (const CAAUMIDIMap &a, const CAAUMIDIMap &b)
{
		// ignore channel first
	return (((a.mStatus & 0xF0) == (b.mStatus & 0xF0))
			&& (a.mData1 == b.mData1)
			&& ((a.mStatus & 0xF) == (b.mStatus & 0xf))  // now compare the channel
			&&  (a.mParameterID == b.mParameterID)
			&& (a.mElement == b.mElement)
			&& (a.mScope == b.mScope));
	
	// reserved field comparisons - ignored until/if they are used
}

inline bool operator< (const CAAUMIDIMap	&a, const CAAUMIDIMap &b)
{
	if ((a.mStatus & 0xF0) != (b.mStatus & 0xF0)) 
		return ((a.mStatus & 0xF0) < (b.mStatus & 0xF0));
	
	if (a.mData1 != b.mData1)
		return (a.mData1 < b.mData1);

	if ((a.mStatus & 0xF) != (b.mStatus & 0xf))  // now compare the channel
		return ((a.mStatus & 0xF) < (b.mStatus & 0xf));

// reserved field comparisons - ignored until/if they are used
		
//		we're sorting this by MIDI, so we don't really care how the rest is sorted
	return	((a.mParameterID < b.mParameterID)
				&& (a.mElement < b.mElement)
				&& (a.mScope < b.mScope));
}



class CompareMIDIMap {
	int compare (const CAAUMIDIMap &a, const CAAUMIDIMap &b) 
	{
		if ((a.mStatus & 0xF0) < (b.mStatus & 0xF0))
			return -1;
		if ((a.mStatus & 0xF0) > (b.mStatus & 0xF0))
			return 1;

			// note event
		if (a.mStatus < 0xB0 || a.mStatus >= 0xD0)
			return 0;
		if (a.mData1 > b.mData1) return 1;
		if (a.mData1 < b.mData1) return -1;
		return 0;
	}
					 
public:
	bool operator() (const CAAUMIDIMap &a, const CAAUMIDIMap &b) {
		return compare (a, b) < 0;
	}
	bool Finish (const CAAUMIDIMap &a, const CAAUMIDIMap &b) {
		return compare (a, b) != 0;
	}
};


/*
	usage: To find potential mapped events for a given status byte, where mMMapEvents is a sorted vec
	CompareMIDIMap comparObj;
	sortVecIter lower_iter = std::lower_bound(mMMapEvents.begin(), mMMapEvents.end(), inStatusByte, compareObj);
	for (;lower_iter < mMMapEvents.end(); ++lower_iter) {
		// then, see if we go out of the status byte range, using the Finish method
		if (compareObj.Finish(map, tempMap)) // tempMap is a CAAUMIDIMap object with the status/dataByte 1 set
			break;
	// ...
	}
	
	in the for loop you call the MIDI_Matches call, to see if the MIDI event matches a given AUMIDIParam mapping
	special note: you HAVE to transform note on (with vel zero) events to the note off status byte
*/

#endif

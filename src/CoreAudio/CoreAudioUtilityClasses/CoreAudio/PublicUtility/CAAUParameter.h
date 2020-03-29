/*
     File: CAAUParameter.h
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
#ifndef __CAAUParameter_h__
#define __CAAUParameter_h__

#include <AudioToolbox/AudioUnitUtilities.h>

// ____________________________________________________________________________
//	CAAUParameter
//	complete parameter specification
	/*! @class CAAUParameter */
class CAAUParameter : public AudioUnitParameter {
public:
								/*! @ctor CAAUParameter.0 */
								CAAUParameter();
								/*! @ctor CAAUParameter.1 */
								CAAUParameter(AudioUnit au, AudioUnitParameterID param, AudioUnitScope scope, AudioUnitElement element);
								/*! @ctor CAAUParameter.2 */
								CAAUParameter(AudioUnitParameter &inParam);
								/*! @ctor CAAUParameter.3 */
								CAAUParameter(const CAAUParameter &a);
								/*! @dtor ~CAAUParameter */
								~CAAUParameter();
		
	/*! @method operator <@ */
	bool						operator < (const CAAUParameter &a) const 
								{ 
									return memcmp(this, &a, sizeof(AudioUnitParameter)) < 0; 
								}

	/*! @method operator ==@ */
	bool						operator == (const CAAUParameter &a) const 
								{ 
									return !memcmp(this, &a, sizeof(AudioUnitParameter)); 
								}
	
	/*! @method operator =@ */
	CAAUParameter &				operator = (const CAAUParameter &a);
	
	/*! @method GetValue */
	Float32						GetValue() const;
	/*! @method SetValue */
	void						SetValue(	AUParameterListenerRef			inListener, 
											void *							inObject,
											Float32							inValue) const;
	
	/*! @method GetName */
	CFStringRef					GetName() const { return mParamName; }  
										// borrowed reference!

	/*! @method GetStringFromValueCopy */
	CFStringRef					GetStringFromValueCopy(const Float32 *value = NULL) const;	
										// returns a copy of the name of the current parameter value
										// or null if there is no name associated
										// caller must release
	/*! @method ValuesHaveStrings */
	bool						ValuesHaveStrings () const 
								{ 
									return (mParamInfo.flags & kAudioUnitParameterFlag_ValuesHaveStrings) != 0; 
								}
	
	/*! @method GetValueFromString */
	Float32						GetValueFromString (CFStringRef str) const;					
									// caller must release

	/*! @method ParamInfo */
	const AudioUnitParameterInfo &		
								ParamInfo()	const { return mParamInfo; }

	/*! @method GetParamTag */
	CFStringRef					GetParamTag() const	{ return mParamTag; }
									// this may return null! - 
									// in which case there is no descriptive tag for the parameter

	/*! @method GetParamName */
	CFStringRef					GetParamName (int inIndex) const
									// this can return null if there is no name for the parameter
								{ 
									return (mNamedParams && inIndex < mNumIndexedParams) 
												? (CFStringRef) CFArrayGetValueAtIndex(mNamedParams, inIndex)
												: 0; 
								}
	
	/*! @method GetNumIndexedParams */
	int							GetNumIndexedParams () const { return mNumIndexedParams; }
	
	/*! @method IsIndexedParam */
	bool						IsIndexedParam () const { return mNumIndexedParams != 0; }
	
	/*! @method HasNamedParams */
	bool						HasNamedParams () const { return IsIndexedParam() && mNamedParams; }
	
	/*! @method GetClumpID */
	bool						GetClumpID (UInt32 &outClumpID) const 
								{ 
									if (mParamInfo.flags & kAudioUnitParameterFlag_HasClump) {
										outClumpID = mParamInfo.clumpID;
										return true;
									}
									return false;
								}
								
	/*! @method HasDisplayTransformation */
	bool						HasDisplayTransformation () const 
								{ 
									return GetAudioUnitParameterDisplayType (mParamInfo.flags); 
								}

	/*! @method IsExpert */
	bool						IsExpert () const 
								{ 
									return mParamInfo.flags & kAudioUnitParameterFlag_ExpertMode; 
								}
#if DEBUG
	void						Print () const;
#endif
	
		// these methods are defined in CAPersistence.cpp
		// they will persist and restore only the scope, element and param ID's of the AudioUnitParameter
		// however, this is sufficient to be able to save/restore a CAAUParameter object
	void						Save (CFPropertyListRef &outData) const;
	
	static void					Save (const AudioUnitParameter &inParam, CFPropertyListRef &outData);
	
	static OSStatus				Restore	(const CFPropertyListRef inData, AudioUnitParameter &outParam);

protected:
	// cached parameter info
	/*! @var mParamInfo */
	AudioUnitParameterInfo		mParamInfo;
	/*! @var mParamName */
	CFStringRef					mParamName;
	/*! @var mParamTag */
	CFStringRef					mParamTag;
	/*! @var mNumIndexedParams */
	short						mNumIndexedParams;
	/*! @var mNamedParams */
	CFArrayRef					mNamedParams;
	
private:
	void						Init (AudioUnit au, AudioUnitParameterID param, AudioUnitScope scope, AudioUnitElement element);

};



#endif // __CAAUParameter_h__

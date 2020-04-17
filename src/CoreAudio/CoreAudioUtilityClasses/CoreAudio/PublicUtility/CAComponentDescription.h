/*
     File: CAComponentDescription.h
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
#ifndef __CAComponentDescription_h__
#define __CAComponentDescription_h__

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioUnit/AudioUnit.h>
#else
	#include <ConditionalMacros.h>
	#include <AudioUnit.h>
#endif

#include "CACFDictionary.h"
#include <stdio.h>
#include <string.h>

void CAShowComponentDescription(const AudioComponentDescription *desc);

// ____________________________________________________________________________
//
//	CAComponentDescription
class CAComponentDescription : public AudioComponentDescription {
public:
	CAComponentDescription() { memset (this, 0, sizeof (AudioComponentDescription)); }
	
	CAComponentDescription (OSType inType, OSType inSubtype = 0, OSType inManu = 0);

	CAComponentDescription(const AudioComponentDescription& desc) { memcpy (this, &desc, sizeof (AudioComponentDescription)); }
		
	// _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
	//
	// interrogation
	
	bool	IsAU () const;
	
	bool	IsAUFX() const { return componentType == kAudioUnitType_Effect; }
	bool	IsAUFM() const { return componentType == kAudioUnitType_MusicEffect; }
    
    bool    IsAUMI() const { return componentType == 'aumi' /*kAudioUnitType_MIDIProcessor*/; }
	
	bool	IsAUAX () const { return componentType == 'auax' /*kAudioUnitType_Auxiliary*/; }
	
	bool 	IsEffect () const { return IsAUFX() || IsAUFM() || IsPanner(); }
	
	bool	IsOffline () const { return componentType == 'auol' /*kAudioUnitType_Offline*/; }
	
	bool 	IsFConv () const { return componentType == kAudioUnitType_FormatConverter; }
	
	bool	IsPanner () const { return componentType == kAudioUnitType_Panner; }
	
	bool	IsMusicDevice () const { return componentType == kAudioUnitType_MusicDevice; }
	
#ifndef MAC_OS_X_VERSION_10_4
	bool	IsGenerator () const { return componentType =='augn'; }
#else
	bool	IsGenerator () const { return componentType ==kAudioUnitType_Generator; }
#endif
	
	bool	IsOutput () const { return componentType == kAudioUnitType_Output; }
	
	bool	IsSource () const { return IsMusicDevice() || IsGenerator(); }
	
	OSType	Type () const { return componentType; }
	OSType	SubType () const { return componentSubType; }
	OSType 	Manu () const { return componentManufacturer; }

	int		Count() const { return AudioComponentCount(const_cast<CAComponentDescription*>(this)); }
	
		// does a semantic match where "wild card" values for type, subtype, manu will match
	bool	Matches (const AudioComponentDescription &desc) const;
	
	// _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
	//
	//	other
	
	void	Print(FILE* file = stdout) const 	{ _CAShowComponentDescription (this, file); }

	OSStatus			Save (CFPropertyListRef *outData) const;
	OSStatus			Restore (CFPropertyListRef &inData);

private:
	static void _CAShowComponentDescription (const AudioComponentDescription *desc, FILE* file);
	friend void CAShowComponentDescription (const AudioComponentDescription *desc);
};

inline bool	operator< (const AudioComponentDescription& x, const AudioComponentDescription& y)
{
	return memcmp (&x, &y, offsetof (AudioComponentDescription, componentFlags)) < 0;
}

inline bool	operator== (const AudioComponentDescription& x, const AudioComponentDescription& y)
{
	return !memcmp (&x, &y, offsetof (AudioComponentDescription, componentFlags));
}

inline bool	operator!= (const AudioComponentDescription& x, const AudioComponentDescription& y)
{
	return !(x == y);
}

#endif

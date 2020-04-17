/*
     File: CAComponentDescription.cpp
 Abstract: CAComponentDescription.h
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
#include "CAComponentDescription.h"
#include "CAStreamBasicDescription.h"
#include <ctype.h>

void CAShowComponentDescription(const AudioComponentDescription *desc)
{
	CAComponentDescription::_CAShowComponentDescription (desc, stdout);
}

void 	CAComponentDescription::_CAShowComponentDescription(const AudioComponentDescription *desc, FILE* file)
{
	if (desc)
	{
		char str[24];
		fprintf (file, "AudioComponentDescription: %s - ", CAStringForOSType(desc->componentType, str, sizeof(str)));
		fprintf (file, "%s - ", CAStringForOSType(desc->componentSubType, str, sizeof(str)));
		fprintf (file, "%s", CAStringForOSType(desc->componentManufacturer, str, sizeof(str)));		
		fprintf (file, ", 0x%X, 0x%X\n", (int)desc->componentFlags, (int)desc->componentFlagsMask);
	}
}

CAComponentDescription::CAComponentDescription (OSType inType, OSType inSubtype, OSType inManu)
{
	componentType = inType;
	componentSubType = inSubtype;
	componentManufacturer = inManu;
	componentFlags = 0;
	componentFlagsMask = 0;
}

bool	CAComponentDescription::IsAU () const 
{ 
	bool flag = IsEffect() || IsMusicDevice() || IsOffline();
	if (flag) return true;
	
	switch (componentType) {
		case kAudioUnitType_Output:
		case kAudioUnitType_FormatConverter:
		case kAudioUnitType_Mixer:
			return true;
	}
	return false;
}

inline bool _MatchTest (const OSType &inTypeA, const OSType &inTypeB)
{
	return ((inTypeA == inTypeB) || (!inTypeA && !inTypeB) || (inTypeA && !inTypeB) || (!inTypeA && inTypeB)); 
}

bool	CAComponentDescription::Matches (const AudioComponentDescription &desc) const
{
	bool matches = false;
		
		// see if the type matches
	matches = _MatchTest (componentType, desc.componentType);
	
	if (matches)
		matches = _MatchTest (componentSubType, desc.componentSubType);
	
	if (matches)
		matches = _MatchTest (componentManufacturer, desc.componentManufacturer);
	
	return matches;
}

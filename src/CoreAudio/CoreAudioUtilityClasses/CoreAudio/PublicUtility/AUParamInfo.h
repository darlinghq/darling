/*
     File: AUParamInfo.h
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
#include <map>
#include <vector>
#include <AudioUnit/AudioUnit.h>
#include "CAAUParameter.h"

/*
	The ParameterMap returned by the Map() method is a map where 
		- the key is the clumpID
		- the value is a ParameterList (vector<CAAUParameter>)
		
	If you have parameters on multiple scopes (or elements within a scope), then you should create one of these 
	for each scope-element pair
*/

class AUParamInfo {

public:
	typedef std::vector <CAAUParameter> ParameterList;
	typedef std::map <UInt32, ParameterList, std::less<UInt32> > ParameterMap;



							AUParamInfo (AudioUnit		inAU, 
									bool				inIncludeExpert, 
									bool				inIncludeReadOnly, 
									AudioUnitScope		inScope = kAudioUnitScope_Global,
									AudioUnitElement	inElement = 0);
									
							~AUParamInfo();
							
	const ParameterMap&		Map () const { return mParams; }
	
	// some convenience methods
	UInt32					NumParams () const { return mNumParams; }
	
	AudioUnitParameterID	ParamID (UInt32 inIndex) const 
							{ 
								if (inIndex < mNumParams) return mParamListID[inIndex]; 
								return 0xFFFFFFFF; 
							}
							 
	UInt32					NumClumps () const { return static_cast<UInt32>(mParams.size()); }

	UInt32					NumParamsForClump (UInt32 inClump) const;
	
			// returns NULL if there's no info for the parameter
	const CAAUParameter*	GetParamInfo (AudioUnitParameterID inParamID) const;
	
	AudioUnitScope			GetScope () const { return mScope; }
	AudioUnitElement		GetElement () const { return mElement; }
	
private:

	AudioUnit				mAU;
	UInt32					mNumParams;
	AudioUnitParameterID *	mParamListID;
	
	ParameterMap			mParams;
	AudioUnitScope			mScope;
	AudioUnitElement		mElement;
		
		// disallow
	AUParamInfo () {}
	AUParamInfo (const AUParamInfo &c) {}
	AUParamInfo& operator= (const AUParamInfo& c) { return *this; }
};

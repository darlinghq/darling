/*
     File: AUParamInfo.cpp
 Abstract: AUParamInfo.h
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
#include "AUParamInfo.h"
#include "CAXException.h"

AUParamInfo::AUParamInfo (AudioUnit				inAU, 
							bool				inIncludeExpert, 
							bool				inIncludeReadOnly,
							AudioUnitScope		inScope,
							AudioUnitElement	inElement)
	: mAU (inAU),
	  mNumParams (0),
	  mParamListID(NULL),
	  mScope (inScope),
	  mElement (inElement)
{
	UInt32 size;
	OSStatus result = AudioUnitGetPropertyInfo(mAU, kAudioUnitProperty_ParameterList, inScope, mElement, &size, NULL);
		if (size == 0 || result) return;
	
	int nparams = size / sizeof(AudioUnitPropertyID);
	mParamListID = new AudioUnitParameterID[nparams];

	memset (mParamListID, 0xFF, size);

	AudioUnitParameterID *paramList = new AudioUnitParameterID[nparams];
	
	result = AudioUnitGetProperty(mAU, kAudioUnitProperty_ParameterList, mScope, mElement, paramList, &size);
	if (result) {
		delete [] mParamListID;
		delete [] paramList;
		mParamListID = NULL;
		return;
	}
	
	ParameterMap params;
	for (int i = 0; i < nparams; ++i) 
	{
		CAAUParameter auvp (mAU, paramList[i], mScope, mElement); // took out only using global scope in CAAUParameter creation
		const AudioUnitParameterInfo &paramInfo = auvp.ParamInfo();
			
		//	don't include if parameter can't be read or written
		if (!(paramInfo.flags & kAudioUnitParameterFlag_IsWritable) 
			&& !(paramInfo.flags & kAudioUnitParameterFlag_IsReadable))
			continue;

		// only include if expert params wanted
		if (!inIncludeExpert && auvp.IsExpert())
			continue;
		
		// only include if read only params are wanted
		if (!(paramInfo.flags & kAudioUnitParameterFlag_IsWritable) 
			&& (paramInfo.flags & kAudioUnitParameterFlag_IsReadable))
		{	
			if (!inIncludeReadOnly)
				continue;
		}
		
		mParamListID[mNumParams] = paramList[i];
		mNumParams++;
		
		// ok - if we're here, then we have a parameter we are going to display.
		UInt32 clump = 0;
		auvp.GetClumpID (clump);
		mParams[clump].push_back (auvp);
	}

	delete [] paramList;
}

AUParamInfo::~AUParamInfo()
{
	delete [] mParamListID;
}

UInt32			AUParamInfo::NumParamsForClump (UInt32 inClump) const
{ 
	ParameterMap::const_iterator it = mParams.find(inClump);
	if (it != mParams.end()) 
		return static_cast<UInt32>((*it).second.size());
	return 0;
}

const CAAUParameter*	AUParamInfo::GetParamInfo (AudioUnitParameterID inParamID) const
{
	for (ParameterMap::const_iterator it = mParams.begin(); it != mParams.end(); ++it) {
		const ParameterList &list = (*it).second;
		for (ParameterList::const_iterator iter = list.begin(); iter != list.end(); ++iter) {
			if (inParamID == (*iter).mParameterID) {
				return &(*iter);
			}
		}
	}
	return NULL;
}

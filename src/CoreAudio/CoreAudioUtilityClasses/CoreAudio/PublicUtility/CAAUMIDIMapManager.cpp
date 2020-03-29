/*
     File: CAAUMIDIMapManager.cpp
 Abstract: CAAUMIDIMapManager.h
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
#include "CAAUMIDIMapManager.h"
#include <AudioToolbox/AudioUnitUtilities.h>

CAAUMIDIMapManager::CAAUMIDIMapManager()
{	
	hotMapping = false;	
}

static void FillInMap (CAAUMIDIMap &map, AUBase &That)
{
	AudioUnitParameterInfo info;
	That.GetParameterInfo (map.mScope, map.mParameterID, info);
	
	if (map.IsSubRange()) {
		map.mMinValue = map.mSubRangeMin;
		map.mMaxValue = map.mSubRangeMax;
	} else {
		map.mMinValue = info.minValue;			
		map.mMaxValue = info.maxValue;		
	}
	
	map.mTransType = CAAUMIDIMap::GetTransformer(info.flags);
}

OSStatus	CAAUMIDIMapManager::SortedInsertToParamaterMaps	(AUParameterMIDIMapping *maps, UInt32 inNumMaps, AUBase &That)
{	
	for (unsigned int i = 0; i < inNumMaps; ++i) 
	{
		CAAUMIDIMap map(maps[i]);

		FillInMap (map, That);
		
		int idx = FindParameterIndex (maps[i]);
		if (idx > -1)
			mParameterMaps.erase(mParameterMaps.begin() + idx);

			// least disruptive place to put this is at the end
		mParameterMaps.push_back(map);
	}
	
	std::sort(mParameterMaps.begin(), mParameterMaps.end(), CompareMIDIMap());	
	
	return noErr;
}

void CAAUMIDIMapManager::GetHotParameterMap(AUParameterMIDIMapping &outMap )
{
	outMap = mHotMap;
}

void CAAUMIDIMapManager::SortedRemoveFromParameterMaps(AUParameterMIDIMapping *maps, UInt32 inNumMaps, bool &outMapDidChange)
{	
	if (hotMapping) {
		hotMapping = false;
	}

	outMapDidChange = false;
	for (unsigned int i = 0; i < inNumMaps; ++i) {
		int idx = FindParameterIndex (maps[i]);
		if (idx > -1) {
			//mParameterMaps[idx].Print();
			mParameterMaps.erase(mParameterMaps.begin() + idx);
			outMapDidChange = true;
		}
	}
}

void	CAAUMIDIMapManager::ReplaceAllMaps (AUParameterMIDIMapping* inMappings, UInt32 inNumMaps, AUBase &That)
{
	mParameterMaps.clear();

	for (unsigned int i = 0; i < inNumMaps; ++i) {
		CAAUMIDIMap mapping(inMappings[i]);

		FillInMap (mapping, That);
		mParameterMaps.push_back (mapping);
	}

	std::sort(mParameterMaps.begin(),mParameterMaps.end(), CompareMIDIMap());	
}

bool CAAUMIDIMapManager::HandleHotMapping(UInt8 	inStatus,
										  UInt8 	inChannel,
										  UInt8 	inData1,
										  AUBase	&That)
{ //used to set the hot map info

	if (inStatus == 0xf0) return false;
	
	if (!hotMapping) return false;
	hotMapping = false;

	mHotMap.mStatus = inStatus | inChannel;  
	mHotMap.mData1 = inData1; 
		
	SortedInsertToParamaterMaps (&mHotMap, 1, That);
	return true;
}

#if DEBUG

void CAAUMIDIMapManager::Print()
{
	for ( ParameterMaps::iterator i = mParameterMaps.begin(); i < mParameterMaps.end(); ++i) { 
		CAAUMIDIMap* listmap =  &(*i);		
		listmap->Print();		
	}		
}

#endif // DEBUG

void CAAUMIDIMapManager::GetMaps(AUParameterMIDIMapping* maps)
{
	int i = 0;
	for ( ParameterMaps::iterator iter = mParameterMaps.begin(); iter < mParameterMaps.end(); ++iter, ++i) { 
		AUParameterMIDIMapping &listmap =  (*iter);	
		maps[i] = listmap;	
	}
}

int CAAUMIDIMapManager::FindParameterIndex (AUParameterMIDIMapping &inMap)
{ 
	//used to get back hot mapping and one at a time maps, for ui
	
	int idx = 0;
	for ( ParameterMaps::iterator i = mParameterMaps.begin(); i < mParameterMaps.end(); ++i) { 
		CAAUMIDIMap & listmap =  (*i);
		if ( (listmap.mParameterID == inMap.mParameterID) && 
			 (listmap.mScope == inMap.mScope) && 
			 (listmap.mElement == inMap.mElement) )
		{ 
				return idx; 
		}
		idx++;
	}
	return -1;
}

bool CAAUMIDIMapManager::FindParameterMapEventMatch(	UInt8			inStatus,
														UInt8			inChannel,
														UInt8			inData1,
														UInt8			inData2,
														UInt32			inBufferOffset,
														AUBase&			inAUBase)
{
	bool ret_value = false;

	if (inStatus == 0x90 && !inData2)
		inStatus = 0x80 | inChannel;
	
	//used to test for midi matches once map is made
	CAAUMIDIMap tempMap;
	tempMap.mStatus = inStatus | inChannel;
	tempMap.mData1 = inData1;
	
	CompareMIDIMap compareObj;

	AudioUnitEvent event;
	event.mEventType = kAudioUnitEvent_ParameterValueChange;
	event.mArgument.mParameter.mAudioUnit = inAUBase.GetComponentInstance();
	
	ParameterMaps::iterator lower_iter = 
	std::lower_bound(mParameterMaps.begin(), mParameterMaps.end(), tempMap, compareObj);
	
	while (lower_iter < mParameterMaps.end()) 
	{
		CAAUMIDIMap & map = (*lower_iter);
		if (compareObj.Finish(map, tempMap)) 
			break;
		
		Float32 value;
		if (map.MIDI_Matches(inChannel, inData1, inData2, value))
		{	
			inAUBase.SetParameter ( map.mParameterID, map.mScope, map.mElement, 
									map.ParamValueFromMIDILinear(value), inBufferOffset);

			event.mArgument.mParameter.mParameterID = map.mParameterID;
			event.mArgument.mParameter.mScope = map.mScope;
			event.mArgument.mParameter.mElement = map.mElement;
			
			AUEventListenerNotify(NULL, NULL, &event);
			ret_value = true;
		}
		++lower_iter;
	}
	return ret_value;
}

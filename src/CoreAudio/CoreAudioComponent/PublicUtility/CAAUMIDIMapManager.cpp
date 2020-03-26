/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
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

/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#ifndef __CAAUMIDIMapManager_h_
#define __CAAUMIDIMapManager_h_

#include "AUBase.h"
#include "CAAUMIDIMap.h"
#include <vector>
#include <AudioToolbox/AudioUnitUtilities.h>

class CAAUMIDIMapManager {
		
protected:
	
	typedef std::vector<CAAUMIDIMap>	ParameterMaps;
	ParameterMaps						mParameterMaps;
	
	bool								hotMapping;
	AUParameterMIDIMapping				mHotMap;
	
public:
					
							CAAUMIDIMapManager();
	
	UInt32					NumMaps(){return static_cast<UInt32>(mParameterMaps.size());}
	void					GetMaps(AUParameterMIDIMapping* maps);
	
	int						FindParameterIndex(AUParameterMIDIMapping &map);
	
	void					GetHotParameterMap(AUParameterMIDIMapping &outMap);
		
	void					SortedRemoveFromParameterMaps	(AUParameterMIDIMapping *maps, UInt32 inNumMaps, bool &outMapDidChange);
	OSStatus				SortedInsertToParamaterMaps	(AUParameterMIDIMapping *maps, UInt32 inNumMaps, AUBase &That);
	
	void					ReplaceAllMaps (AUParameterMIDIMapping* inMappings, UInt32 inNumMaps, AUBase &That);
	
	bool					IsHotMapping(){return hotMapping;}
	void					SetHotMapping (AUParameterMIDIMapping &inMap){hotMapping = true; mHotMap = inMap; }
	
	bool					HandleHotMapping(	UInt8 	inStatus,
												UInt8 	inChannel,
												UInt8 	inData1,
												AUBase	&That);
	
		
	bool					FindParameterMapEventMatch(UInt8 	inStatus,
													   UInt8 	inChannel,
													   UInt8 	inData1,
													   UInt8 	inData2,
													   UInt32	inBufferOffset,
													   AUBase&	inAUBase);	
#if DEBUG
	void					Print();
#endif
};


#endif

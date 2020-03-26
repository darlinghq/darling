/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUBase Classes
*/

#ifndef __AUBaseHelper_h__
#define __AUBaseHelper_h__

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreFoundation/CoreFoundation.h>
	#include <AudioUnit/AUComponent.h>
#else
	#include <CoreFoundation.h>
	#include <AUComponent.h>
#endif

#include "AUBase.h"

// helpers for dealing with the file-references dictionary in an AUPreset
OSStatus GetFileRefPath (CFDictionaryRef parent, CFStringRef frKey, CFStringRef * fPath);

// if fileRefDict is NULL, this call creates one
// if not NULL, then the key value is added to it
CFMutableDictionaryRef CreateFileRefDict (CFStringRef fKey, CFStringRef fPath, CFMutableDictionaryRef fileRefDict);

int AccessURLAsset(const CFURLRef inURL, int mode);

#if DEBUG
	void PrintAUParamEvent (AudioUnitParameterEvent& event, FILE* f);
#endif



#endif // __AUBaseHelper_h__
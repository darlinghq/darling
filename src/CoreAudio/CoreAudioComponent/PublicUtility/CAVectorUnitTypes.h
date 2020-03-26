/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#ifndef __CAVectorUnitTypes_h__
#define __CAVectorUnitTypes_h__

enum {
	kVecUninitialized = -1,
	kVecNone = 0,
	kVecAltivec = 1,
	kVecSSE2 = 100,
	kVecSSE3 = 101,
	kVecAVX1 = 110,
	kVecNeon = 200
};

#endif

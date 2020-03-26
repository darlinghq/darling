/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUBase Classes
*/

#include "AUOutputElement.h"
#include "AUBase.h"

AUOutputElement::AUOutputElement(AUBase *audioUnit) : 
	AUIOElement(audioUnit)
{
	AllocateBuffer();
}

OSStatus	AUOutputElement::SetStreamFormat(const CAStreamBasicDescription &desc)
{
	OSStatus result = AUIOElement::SetStreamFormat(desc);	// inherited
	if (result == AUBase::noErr)
		AllocateBuffer();
	return result;
}
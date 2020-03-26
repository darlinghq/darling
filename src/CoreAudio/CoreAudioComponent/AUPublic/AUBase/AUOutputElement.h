/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio AUBase Classes
*/

#ifndef __AUOutput_h__
#define __AUOutput_h__

#include "AUScopeElement.h"
#include "AUBuffer.h"

	/*! @class AUOutputElement */
class AUOutputElement : public AUIOElement {
public:
	/*! @ctor AUOutputElement */
						AUOutputElement(AUBase *audioUnit);

	// AUElement override
	/*! @method SetStreamFormat */
	virtual OSStatus	SetStreamFormat(const CAStreamBasicDescription &desc);
	/*! @method NeedsBufferSpace */
	virtual bool		NeedsBufferSpace() const { return true; }
};

#endif // __AUOutput_h__

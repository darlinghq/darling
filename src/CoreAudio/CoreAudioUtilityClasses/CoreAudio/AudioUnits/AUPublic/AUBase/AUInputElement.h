/*
     File: AUInputElement.h
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
#ifndef __AUInput_h__
#define __AUInput_h__

#include "AUScopeElement.h"
#include "AUBuffer.h"

/*! @class AUInputElement */
class AUInputElement : public AUIOElement {
public:
	
	/*! @ctor AUInputElement */
						AUInputElement(AUBase *audioUnit);
	/*! @dtor ~AUInputElement */
	virtual				~AUInputElement() { }

	// AUElement override
	/*! @method SetStreamFormat */
	virtual OSStatus	SetStreamFormat(const CAStreamBasicDescription &desc);
	/*! @method NeedsBufferSpace */
	virtual bool		NeedsBufferSpace() const { return IsCallback(); }

	/*! @method SetConnection */
	void				SetConnection(const AudioUnitConnection &conn);
	/*! @method SetInputCallback */
	void				SetInputCallback(AURenderCallback proc, void *refCon);

	/*! @method IsActive */
	bool				IsActive() const { return mInputType != kNoInput; }
	/*! @method IsCallback */
	bool				IsCallback() const { return mInputType == kFromCallback; }
	/*! @method HasConnection */
	bool				HasConnection() const { return mInputType == kFromConnection; }

	/*! @method PullInput */
	OSStatus			PullInput(	AudioUnitRenderActionFlags &  	ioActionFlags,
									const AudioTimeStamp &			inTimeStamp,
									AudioUnitElement				inElement,
									UInt32							inNumberFrames);

	/*! @method PullInputWithBufferList */
	OSStatus			PullInputWithBufferList(	AudioUnitRenderActionFlags &  	ioActionFlags,
													const AudioTimeStamp &			inTimeStamp,
													AudioUnitElement				inElement,
													UInt32							nFrames,
													AudioBufferList *				inBufferList);
protected:
	/*! @method Disconnect */
	void				Disconnect();

	enum EInputType { kNoInput, kFromConnection, kFromCallback };

	/*! @var mInputType */
	EInputType					mInputType;

	// if from callback:
	/*! @var mInputProc */
	AURenderCallback			mInputProc;
	/*! @var mInputProcRefCon */
	void *						mInputProcRefCon;
	
	// if from connection:
	/*! @var mConnection */
	AudioUnitConnection			mConnection;
#if !CA_USE_AUDIO_PLUGIN_ONLY
	/*! @var mConnRenderProc */
	AudioUnitRenderProc			mConnRenderProc;
#endif
	/*! @var mConnInstanceStorage */
	void *						mConnInstanceStorage;		// for the input component
};


#endif // __AUInput_h__

/*
     File: AUOutputBL.h
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
#ifndef __AUOutputBL_h__
#define __AUOutputBL_h__

#include "CAStreamBasicDescription.h"
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
#else
#endif

// ____________________________________________________________________________
//
//	AUOutputBL - Simple Buffer List wrapper targetted to use with retrieving AU output
// Works in one of two ways (both adjustable)... Can use it with NULL pointers, or allocate
// memory to receive the data in.

// Before using this with any call to AudioUnitRender, it needs to be Prepared
// as some calls to AudioUnitRender can reset the ABL

class AUOutputBL {
public:
											
											// you CANNOT use one of these - it will crash!
//										AUOutputBL ();
										
											// this is the constructor that you use
											// it can't be reset once you've constructed it
										AUOutputBL (const CAStreamBasicDescription &inDesc, UInt32 inDefaultNumFrames = 512);
										~AUOutputBL();

	void 								Prepare ()
										{
											Prepare (mFrames);
										}
									
								// this version can throw if this is an allocted ABL and inNumFrames is > AllocatedFrames()
								// you can set the bool to true if you want a NULL buffer list even if allocated
								// inNumFrames must be a valid number (will throw if inNumFrames is 0)
	void 								Prepare (UInt32 inNumFrames, bool inWantNullBufferIfAllocated = false);
	
	AudioBufferList*					ABL() { return mBufferList; }
								
								// You only need to call this if you want to allocate a buffer list
								// if you want an empty buffer list, just call Prepare()
								// if you want to dispose previously allocted memory, pass in 0
								// then you either have an empty buffer list, or you can re-allocate
								// Memory is kept around if an Allocation request is less than what is currently allocated
	void								Allocate (UInt32 inNumberFrames);
	
	UInt32								AllocatedFrames() const { return mFrames; }
	
	const CAStreamBasicDescription&		GetFormat() const { return mFormat; }

#if DEBUG
	void								Print();
#endif
	
private:
	UInt32						AllocatedBytes () const { return (mBufferSize * mNumberBuffers); }

	CAStreamBasicDescription	mFormat;
	Byte*						mBufferMemory;
	AudioBufferList* 			mBufferList;
	UInt32						mNumberBuffers;
	UInt32						mBufferSize;
	UInt32						mFrames;

// don't want to copy these.. can if you want, but more code to write!
	AUOutputBL () {}
	AUOutputBL (const AUOutputBL &c);
	AUOutputBL& operator= (const AUOutputBL& c);
};

#endif // __AUOutputBL_h__

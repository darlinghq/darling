/*
     File: AUBuffer.h
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
#ifndef __AUBuffer_h__
#define __AUBuffer_h__

#include <TargetConditionals.h>
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioUnit/AudioUnit.h>
#else
	#include <AudioUnit.h>
#endif

#include <string.h>
#include "CAStreamBasicDescription.h"
#include "CAAutoDisposer.h"
#include "CADebugMacros.h"

// make this usable outside the stricter context of AudiUnits
#ifndef COMPONENT_THROW
	#define COMPONENT_THROW(err) \
		do { DebugMessage(#err); throw static_cast<OSStatus>(err); } while (0)
#endif


	/*! @class AUBufferList */
class AUBufferList {
	enum EPtrState {
		kPtrsInvalid,
		kPtrsToMyMemory,
		kPtrsToExternalMemory
	};
public:
	/*! @ctor AUBufferList */
	AUBufferList() : mPtrState(kPtrsInvalid), mExternalMemory(false), mPtrs(NULL), mMemory(NULL), 
		mAllocatedStreams(0), mAllocatedFrames(0), mAllocatedBytes(0) { }
	/*! @dtor ~AUBufferList */
	~AUBufferList();

	/*! @method PrepareBuffer */
	AudioBufferList &	PrepareBuffer(const CAStreamBasicDescription &format, UInt32 nFrames);
	/*! @method PrepareNullBuffer */
	AudioBufferList &	PrepareNullBuffer(const CAStreamBasicDescription &format, UInt32 nFrames);

	/*! @method SetBufferList */
	AudioBufferList &	SetBufferList(const AudioBufferList &abl) {
							if (mAllocatedStreams < abl.mNumberBuffers)
								COMPONENT_THROW(-1);
							mPtrState = kPtrsToExternalMemory;
							memcpy(mPtrs, &abl, (char *)&abl.mBuffers[abl.mNumberBuffers] - (char *)&abl);
							return *mPtrs;
						}
	
	/*! @method SetBuffer */
	void				SetBuffer(UInt32 index, const AudioBuffer &ab) {
							if (mPtrState == kPtrsInvalid || index >= mPtrs->mNumberBuffers)
								COMPONENT_THROW(-1);
							mPtrState = kPtrsToExternalMemory;
							mPtrs->mBuffers[index] = ab;
						}

	/*! @method InvalidateBufferList */
	void				InvalidateBufferList() { mPtrState = kPtrsInvalid; }

	/*! @method GetBufferList */
	AudioBufferList &	GetBufferList() const {
							if (mPtrState == kPtrsInvalid)
								COMPONENT_THROW(-1);
							return *mPtrs;
						}

	/*! @method CopyBufferListTo */
	void				CopyBufferListTo(AudioBufferList &abl) const {
							if (mPtrState == kPtrsInvalid)
								COMPONENT_THROW(-1);
							memcpy(&abl, mPtrs, (char *)&abl.mBuffers[abl.mNumberBuffers] - (char *)&abl);
						}
	
	/*! @method CopyBufferContentsTo */
	void				CopyBufferContentsTo(AudioBufferList &abl) const {
							if (mPtrState == kPtrsInvalid)
								COMPONENT_THROW(-1);
							const AudioBuffer *srcbuf = mPtrs->mBuffers;
							AudioBuffer *destbuf = abl.mBuffers;

							for (UInt32 i = 0; i < abl.mNumberBuffers; ++i, ++srcbuf, ++destbuf) {
								if (i >= mPtrs->mNumberBuffers) // duplicate last source to additional outputs [4341137]
									--srcbuf;
								if (destbuf->mData != srcbuf->mData)
									memmove(destbuf->mData, srcbuf->mData, srcbuf->mDataByteSize);
								destbuf->mDataByteSize = srcbuf->mDataByteSize;
							}
						}
	
	/*! @method Allocate */
	void				Allocate(const CAStreamBasicDescription &format, UInt32 nFrames);
	/*! @method Deallocate */
	void				Deallocate();
	
	/*! @method UseExternalBuffer */
	void				UseExternalBuffer(const CAStreamBasicDescription &format, const AudioUnitExternalBuffer &buf);

	// AudioBufferList utilities
	/*! @method ZeroBuffer */
	static void			ZeroBuffer(AudioBufferList &abl) {
							AudioBuffer *buf = abl.mBuffers;
							for (UInt32 i = abl.mNumberBuffers ; i--; ++buf)
								memset(buf->mData, 0, buf->mDataByteSize);
						}
#if DEBUG
	/*! @method PrintBuffer */
	static void			PrintBuffer(const char *label, int subscript, const AudioBufferList &abl, UInt32 nFrames = 8, bool asFloats = true);
#endif

	/*! @method GetAllocatedFrames */
	UInt32				GetAllocatedFrames() const { return mAllocatedFrames; }
	
private:
	/*! @ctor AUBufferList */
	AUBufferList(AUBufferList &) { }	// prohibit copy constructor

	/*! @var mPtrState */
	EPtrState					mPtrState;
	/*! @var mExternalMemory */
	bool						mExternalMemory;
	/*! @var mPtrs */
	AudioBufferList *			mPtrs;
	/*! @var mMemory */
	Byte *						mMemory;
	/*! @var mAllocatedStreams */
	UInt32						mAllocatedStreams;
	/*! @var mAllocatedFrames */
	UInt32						mAllocatedFrames;
	/*! @var mAllocatedBytes */
	UInt32						mAllocatedBytes;
};


// Allocates an array of samples (type T), to be optimally aligned for the processor
	/*! @class TAUBuffer */
template <class T>
class TAUBuffer {
public:
	enum { 
		kAlignInterval = 0x10,
		kAlignMask = kAlignInterval - 1
	};
	
	/*! @ctor TAUBuffer.0 */
	TAUBuffer() :	mMemObject(NULL), mAlignedBuffer(NULL), mBufferSizeBytes(0)
	{
	}
	
	/*! @ctor TAUBuffer.1 */
	TAUBuffer(UInt32 numElems, UInt32 numChannels) :	mMemObject(NULL), mAlignedBuffer(NULL),
														mBufferSizeBytes(0)
	{
		Allocate(numElems, numChannels);
	}
	
	/*! @dtor ~TAUBuffer */
	~TAUBuffer()
	{
		Deallocate();
	}
		
	/*! @method Allocate */
	void	Allocate(UInt32 numElems)			// can also re-allocate
	{
		UInt32 reqSize = numElems * sizeof(T);
		
		if (mMemObject != NULL && reqSize == mBufferSizeBytes)
			return;	// already allocated

		mBufferSizeBytes = reqSize;
		mMemObject = CA_realloc(mMemObject, reqSize);
		UInt32 misalign = (uintptr_t)mMemObject & kAlignMask;
		if (misalign) {
			mMemObject = CA_realloc(mMemObject, reqSize + kAlignMask);
			mAlignedBuffer = (T *)((char *)mMemObject + kAlignInterval - misalign);
		} else
			mAlignedBuffer = (T *)mMemObject;
	}

	/*! @method Deallocate */
	void	Deallocate()
	{
		if (mMemObject == NULL) return;			// so this method has no effect if we're using
												// an external buffer
		
		free(mMemObject);
		mMemObject = NULL;
		mAlignedBuffer = NULL;
		mBufferSizeBytes = 0;
	}
	
	/*! @method AllocateClear */
	void	AllocateClear(UInt32 numElems)		// can also re-allocate
	{
		Allocate(numElems);
		Clear();
	}
	
	/*! @method Clear */
	void	Clear()
	{
		memset(mAlignedBuffer, 0, mBufferSizeBytes);
	}
	
	// accessors
	
	/*! @method operator T *()@ */
	operator T *()				{ return mAlignedBuffer; }

private:
	/*! @var mMemObject */
	void *		mMemObject;			// null when using an external buffer
	/*! @var mAlignedBuffer */
	T *			mAlignedBuffer;		// always valid once allocated
	/*! @var mBufferSizeBytes */
	UInt32		mBufferSizeBytes;
};

#endif // __AUBuffer_h__

/*
     File: CABufferList.h
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
#ifndef __CABufferList_h__
#define __CABufferList_h__

#include <stddef.h>
#include "CAStreamBasicDescription.h"
#include "CAXException.h"

void CAShowAudioBufferList(const AudioBufferList &abl, int framesToPrint, const AudioStreamBasicDescription &fmt, const char *label=NULL);
void CAShowAudioBufferList(const AudioBufferList &abl, int framesToPrint, int wordSize, const char *label=NULL);
extern "C" void CAShowAudioBufferList(const AudioBufferList *abl, int framesToPrint, int wordSize);
extern "C" int CrashIfClientProvidedBogusAudioBufferList(const AudioBufferList *abl, bool nullOK=false);

/* ____________________________________________________________________________
//	CABufferList - variable length buffer list

	This class is designed for use in non-simplistic cases. For AudioUnits, AUBufferList
	is preferred.
	
	CABufferList can be used in one of two ways:
		- as mutable pointers into non-owned memory
		- as an immutable array of buffers (owns its own memory).

 	All buffers are assumed to have the same format (number of channels, word size), so that
		we can assume their mDataByteSizes are all the same.
____________________________________________________________________________ */
class CABufferList {
public:
	void *	operator new(size_t /*size*/, int nBuffers) {
				return ::operator new(sizeof(CABufferList) + (nBuffers-1) * sizeof(AudioBuffer));
			}
	static CABufferList *	New(const char *name, const CAStreamBasicDescription &format)
	{
		UInt32 numBuffers = format.NumberChannelStreams(), channelsPerBuffer = format.NumberInterleavedChannels();
		return new(numBuffers) CABufferList(name, numBuffers, channelsPerBuffer);
	}
	static CABufferList *	New(const CAStreamBasicDescription &format) { return New("", format); }

	static CABufferList *	New(UInt32 numBuffers, UInt32 channelsPerBuffer, const char *name="") {
		return new(numBuffers) CABufferList(name, numBuffers, channelsPerBuffer);
	}

protected:
	CABufferList(const char *name, UInt32 numBuffers, UInt32 channelsPerBuffer) :
		mName(name),
		mBufferMemory(NULL),
		mBufferCapacity(0)
	{
		//XAssert(numBuffers > 0 /*&& channelsPerBuffer > 0*/);
		mABL.mNumberBuffers = numBuffers;
		AudioBuffer *buf = mABL.mBuffers;
		for (UInt32 i = mABL.mNumberBuffers; i--; ++buf) {
			buf->mNumberChannels = channelsPerBuffer;
			buf->mDataByteSize = 0;
			buf->mData = NULL;
		}
	}

public:
	~CABufferList()
	{
		if (mBufferMemory)
			delete[] mBufferMemory;
	}
	
	const char *				Name() { return mName; }
	
	const AudioBufferList &		GetBufferList() const { return mABL; }
	
	AudioBufferList &			GetModifiableBufferList() { return _GetBufferList(); }
	
	UInt32		GetNumberBuffers() const { return mABL.mNumberBuffers; }
	
	UInt32		GetNumBytes() const
	{
		return mABL.mBuffers[0].mDataByteSize;
	}
	
	void		SetBytes(UInt32 nBytes, void *data)
	{
		VerifyNotTrashingOwnedBuffer();
		XAssert(mABL.mNumberBuffers == 1);
		mABL.mBuffers[0].mDataByteSize = nBytes;
		mABL.mBuffers[0].mData = data;
	}
	
	void		CopyAllFrom(CABufferList *srcbl, CABufferList *ptrbl)
					// copies bytes from srcbl
					// make ptrbl reflect the length copied
					// note that srcbl may be same as ptrbl!
	{
		// Note that this buffer *can* own memory and its pointers/lengths are not
		// altered; only its buffer contents, which are copied from srcbl.
		// The pointers/lengths in ptrbl are updated to reflect the addresses/lengths
		// of the copied data, and srcbl's contents are consumed.
		ptrbl->VerifyNotTrashingOwnedBuffer();
		UInt32 nBytes = srcbl->GetNumBytes();
		AudioBuffer *mybuf = mABL.mBuffers, *srcbuf = srcbl->mABL.mBuffers,
					*ptrbuf = ptrbl->mABL.mBuffers;
		for (UInt32 i = mABL.mNumberBuffers; i--; ++mybuf, ++srcbuf, ++ptrbuf) {
			memmove(mybuf->mData, srcbuf->mData, srcbuf->mDataByteSize);
			ptrbuf->mData = mybuf->mData;
			ptrbuf->mDataByteSize = srcbuf->mDataByteSize;
		}
		if (srcbl != ptrbl)
			srcbl->BytesConsumed(nBytes);
	}
	
	// copies data from another buffer list.
	void		CopyDataFrom(const AudioBufferList &other)
	{
		for (unsigned i = 0; i < other.mNumberBuffers; ++i) {
			XAssert(mBufferCapacity == 0 || other.mBuffers[i].mDataByteSize <= mBufferCapacity);
			memcpy(mABL.mBuffers[i].mData, other.mBuffers[i].mData, 
				mABL.mBuffers[i].mDataByteSize = other.mBuffers[i].mDataByteSize);
		}
	}
	
	void		AppendFrom(CABufferList *blp, UInt32 nBytes)
	{
		// this may mutate a buffer that owns memory.
		AudioBuffer *mybuf = mABL.mBuffers, *srcbuf = blp->mABL.mBuffers;
		for (UInt32 i = mABL.mNumberBuffers; i--; ++mybuf, ++srcbuf) {
			XAssert(nBytes <= srcbuf->mDataByteSize);
			XAssert(mBufferCapacity == 0 || mybuf->mDataByteSize + nBytes <= mBufferCapacity);
			memcpy((Byte *)mybuf->mData + mybuf->mDataByteSize, srcbuf->mData, nBytes);
			mybuf->mDataByteSize += nBytes;
		}
		blp->BytesConsumed(nBytes);
	}
	
	void		PadWithZeroes(UInt32 desiredBufferSize)
					// for cases where an algorithm (e.g. SRC) requires some
					// padding to create silence following end-of-file
	{
		XAssert(mBufferCapacity == 0 || desiredBufferSize <= mBufferCapacity);
		if (GetNumBytes() > desiredBufferSize) return;
		AudioBuffer *buf = mABL.mBuffers;
		for (UInt32 i = mABL.mNumberBuffers; i--; ++buf) {
			memset((Byte *)buf->mData + buf->mDataByteSize, 0, desiredBufferSize - buf->mDataByteSize);
			buf->mDataByteSize = desiredBufferSize;
		}
	}
	
	void		SetToZeroes(UInt32 nBytes)
	{
		XAssert(mBufferCapacity == 0 || nBytes <= mBufferCapacity);
		AudioBuffer *buf = mABL.mBuffers;
		for (UInt32 i = mABL.mNumberBuffers; i--; ++buf) {
			memset((Byte *)buf->mData, 0, nBytes);
			buf->mDataByteSize = nBytes;
		}
	}
	
	void		Reset()
	{
		DeallocateBuffers();
	}
	
	Boolean		SameDataAs(const CABufferList* anotherBufferList)
	{
		// check to see if two buffer lists point to the same memory.
		if (mABL.mNumberBuffers != anotherBufferList->mABL.mNumberBuffers) return false;
		
		for (UInt32 i = 0; i < mABL.mNumberBuffers; ++i) {
			if (mABL.mBuffers[i].mData != anotherBufferList->mABL.mBuffers[i].mData) return false;
		}
		return true;
	}
	
	void		BytesConsumed(UInt32 nBytes)
					// advance buffer pointers, decrease buffer sizes
	{
		VerifyNotTrashingOwnedBuffer();
		AudioBuffer *buf = mABL.mBuffers;
		for (UInt32 i = mABL.mNumberBuffers; i--; ++buf) {
			XAssert(nBytes <= buf->mDataByteSize);
			buf->mData = (Byte *)buf->mData + nBytes;
			buf->mDataByteSize -= nBytes;
		}
	}
	
	void		SetFrom(const AudioBufferList *abl)
	{
		VerifyNotTrashingOwnedBuffer();
		memcpy(&_GetBufferList(), abl, (char *)&abl->mBuffers[abl->mNumberBuffers] - (char *)abl);
	}
	
	void		SetFrom(const CABufferList *blp)
	{
		SetFrom(&blp->GetBufferList());
	}
	
	void		SetFrom(const AudioBufferList *abl, UInt32 nBytes)
	{
		VerifyNotTrashingOwnedBuffer();
		AudioBuffer *mybuf = mABL.mBuffers;
		const AudioBuffer *srcbuf = abl->mBuffers;
		for (UInt32 i = mABL.mNumberBuffers; i--; ++mybuf, ++srcbuf) {
			mybuf->mNumberChannels = srcbuf->mNumberChannels;
			mybuf->mDataByteSize = nBytes;
			mybuf->mData = srcbuf->mData;
		}
	}
	
	void		SetFrom(const CABufferList *blp, UInt32 nBytes)
	{
		SetFrom(&blp->GetBufferList(), nBytes);
	}
	
	AudioBufferList *	ToAudioBufferList(AudioBufferList *abl) const
	{
		memcpy(abl, &GetBufferList(), (char *)&abl->mBuffers[mABL.mNumberBuffers] - (char *)abl);
		return abl;
	}
	
	void		AllocateBuffers(UInt32 nBytes);
	void		AllocateBuffersAndCopyFrom(UInt32 nBytes, CABufferList *inCopyFromList, CABufferList *inSetPtrList);
	
	void		DeallocateBuffers();
	
	void		UseExternalBuffer(Byte *ptr, UInt32 nBytes);
    
	void		AdvanceBufferPointers(UInt32 nBytes) // $$$ ReducingSize
					// this is for bufferlists that function simply as
					// an array of pointers into another bufferlist, being advanced,
					// as in RenderOutput implementations
	{
		VerifyNotTrashingOwnedBuffer();
		AudioBuffer *buf = mABL.mBuffers;
		for (UInt32 i = mABL.mNumberBuffers; i--; ++buf) {
			buf->mData = (Byte *)buf->mData + nBytes;
			buf->mDataByteSize -= nBytes;
		}
	}
	
	void		SetNumBytes(UInt32 nBytes)
	{
		XAssert(mBufferCapacity == 0 || nBytes <= mBufferCapacity);
		AudioBuffer *buf = mABL.mBuffers;
		for (UInt32 i = mABL.mNumberBuffers; i--; ++buf)
			buf->mDataByteSize = nBytes;
	}

	void		Print(const char *label=NULL, int nframes=0, int wordSize=0) const
	{
		if (label == NULL)
			label = mName;
		printf("%s - ", label);
		CAShowAudioBufferList(&GetBufferList(), nframes, wordSize);
		if (mBufferMemory)
			printf("  owned memory @ 0x%p:\n", mBufferMemory);
	}
	
	UInt32		GetCapacityBytes() const { return mBufferCapacity; }
	
	template <typename T>
	T*			GetData(UInt32 inBuffer) {
		return static_cast<T*>(mABL.mBuffers[inBuffer].mData);
	}

protected:
	AudioBufferList &	_GetBufferList() { return mABL; }	// use with care
							// if we make this public, then we lose ability to call VerifyNotTrashingOwnedBuffer
	void				VerifyNotTrashingOwnedBuffer()
	{
		// This needs to be called from places where we are modifying the buffer pointers.
		// It's an error to modify the buffer pointers or lengths if we own the buffer memory.
		XAssert(mBufferMemory == NULL);
	}

	const char *						mName;	// for debugging
	Byte *								mBufferMemory;
	UInt32								mBufferCapacity;	// max mDataByteSize of each buffer
	AudioBufferList						mABL;
	// don't add anything here
};

#endif // __CABufferList_h__

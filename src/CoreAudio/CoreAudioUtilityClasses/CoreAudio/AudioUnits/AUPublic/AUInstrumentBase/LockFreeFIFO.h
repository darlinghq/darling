/*
     File: LockFreeFIFO.h
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
#include <libkern/OSAtomic.h>

template <class ITEM>
class LockFreeFIFOWithFree
{
	LockFreeFIFOWithFree(); // private, unimplemented.
public:
	LockFreeFIFOWithFree(UInt32 inMaxSize)
		: mReadIndex(0), mWriteIndex(0), mFreeIndex(0)
	{
		//assert(IsPowerOfTwo(inMaxSize));
		mItems = new ITEM[inMaxSize];
		mMask = inMaxSize - 1;
	}
	
	~LockFreeFIFOWithFree()
	{
		delete [] mItems;
	}

	
	void Reset() 
	{
		FreeItems();
		mReadIndex = 0;
		mWriteIndex = 0;
		mFreeIndex = 0;
	}
	
	ITEM* WriteItem() 
	{
		//printf("WriteItem %d %d\n", mReadIndex, mWriteIndex);
		FreeItems(); // free items on the write thread.
		int32_t nextWriteIndex = (mWriteIndex + 1) & mMask;
		if (nextWriteIndex == mFreeIndex) return NULL;
		return &mItems[mWriteIndex];
	}
	
	ITEM* ReadItem() 
	{
		//printf("ReadItem %d %d\n", mReadIndex, mWriteIndex);
		if (mReadIndex == mWriteIndex) return NULL;
		return &mItems[mReadIndex];
	}
	void AdvanceWritePtr() { OSAtomicCompareAndSwap32(mWriteIndex, (mWriteIndex + 1) & mMask, &mWriteIndex); }
	void AdvanceReadPtr()  { OSAtomicCompareAndSwap32(mReadIndex,  (mReadIndex  + 1) & mMask, &mReadIndex); }
private:
	ITEM* FreeItem() 
	{
		if (mFreeIndex == mReadIndex) return NULL;
		return &mItems[mFreeIndex];
	}
	void AdvanceFreePtr() { OSAtomicCompareAndSwap32(mFreeIndex, (mFreeIndex + 1) & mMask, &mFreeIndex); }
	
	void FreeItems() 
	{
		ITEM* item;
		while ((item = FreeItem()) != NULL)
		{
			item->Free();
			AdvanceFreePtr();
		}
	}
	
	volatile int32_t mReadIndex, mWriteIndex, mFreeIndex;
	int32_t mMask;
	ITEM *mItems;
};



// Same as above but no free.

template <class ITEM>
class LockFreeFIFO
{
	LockFreeFIFO(); // private, unimplemented.
public:
	LockFreeFIFO(UInt32 inMaxSize)
		: mReadIndex(0), mWriteIndex(0)
	{
		//assert(IsPowerOfTwo(inMaxSize));
		mItems = new ITEM[inMaxSize];
		mMask = inMaxSize - 1;
	}
	
	~LockFreeFIFO()
	{
		delete [] mItems;
	}
	
	void Reset() 
	{
		mReadIndex = 0;
		mWriteIndex = 0;
	}
	
	ITEM* WriteItem() 
	{
		int32_t nextWriteIndex = (mWriteIndex + 1) & mMask;
		if (nextWriteIndex == mReadIndex) return NULL;
		return &mItems[mWriteIndex];
	}
	
	ITEM* ReadItem() 
	{
		if (mReadIndex == mWriteIndex) return NULL;
		return &mItems[mReadIndex];
	}
	
		// the CompareAndSwap will always succeed. We use CompareAndSwap because it calls the PowerPC sync instruction,
		// plus any processor bug workarounds for various CPUs.
	void AdvanceWritePtr() { OSAtomicCompareAndSwap32(mWriteIndex, (mWriteIndex + 1) & mMask, &mWriteIndex); }
	void AdvanceReadPtr()  { OSAtomicCompareAndSwap32(mReadIndex,  (mReadIndex  + 1) & mMask, &mReadIndex); }
	
private:
	
	volatile int32_t mReadIndex, mWriteIndex;
	int32_t mMask;
	ITEM *mItems;
};


/*
     File: CompressedPacketTable.h
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
#include <iterator>
#include <vector>
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreAudio/CoreAudioTypes.h>
#else
	#include "CoreAudioTypes.h"
#endif

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

struct  AudioStreamPacketDescriptionExtended : AudioStreamPacketDescription
{
    SInt64  mFrameOffset; // this is the sum of the mVariableFramesInPacket up to this point so we can binary search.
};
typedef struct AudioStreamPacketDescriptionExtended AudioStreamPacketDescriptionExtended;

inline bool operator < (const AudioStreamPacketDescriptionExtended& a, const AudioStreamPacketDescriptionExtended& b)
{
	return a.mFrameOffset < b.mFrameOffset;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

class CompressedPacketTable
{
public:
	CompressedPacketTable(UInt32 inFramesPerPacket) : mSize(0), mFramesPerPacket(inFramesPerPacket) {}
	~CompressedPacketTable();
	
	SInt64 size() const { return mSize; }
	void push_back(const AudioStreamPacketDescriptionExtended& inDesc);
	
	const AudioStreamPacketDescriptionExtended operator[](SInt64 inPacketIndex) const;
	const AudioStreamPacketDescriptionExtended front() const { return (*this)[0]; }
	const AudioStreamPacketDescriptionExtended back() const { return (*this)[mSize-1]; }
	
	//SInt64 PacketForByte(SInt64 inByteOffset) const;
	SInt64 ByteForPacket(SInt64 inPacketIndex) const { return (*this)[inPacketIndex].mStartOffset; }
		
	class iterator {
		public:
			typedef std::input_iterator_tag iterator_category;
			typedef iterator pointer;
			typedef SInt64 difference_type;
			typedef AudioStreamPacketDescriptionExtended value_type;
			typedef value_type& reference;
			
			iterator() : mTable(NULL), mIndex(0) {}
			iterator(const CompressedPacketTable* table, SInt64 index) : mTable(table), mIndex(index) {}
			iterator(const iterator& that) : mTable(that.mTable), mIndex(that.mIndex) {}
			
			iterator& operator=(const iterator& that) { mTable = that.mTable; mIndex = that.mIndex; return *this; }
			
			const AudioStreamPacketDescriptionExtended operator*() const { return (*mTable)[mIndex]; }
			const AudioStreamPacketDescriptionExtended* const operator->() { mValue = (*mTable)[mIndex]; return &mValue; }
			iterator& operator++() { ++mIndex; return *this; }
			iterator& operator--() { --mIndex; return *this; }
			
			SInt64 operator-(const iterator& that) { return mIndex - that.mIndex; }
			const iterator operator-(SInt64 index) { return iterator(mTable, mIndex - index); }
			const iterator operator+(SInt64 index) { return iterator(mTable, mIndex + index); }
			bool operator==(const iterator& that) { return mIndex == that.mIndex; }
			bool operator!=(const iterator& that) { return mIndex != that.mIndex; }
			bool operator>(const iterator& that) { return mIndex > that.mIndex; }
			bool operator<(const iterator& that) { return mIndex < that.mIndex; }
		private:
			const CompressedPacketTable* mTable;
			SInt64 mIndex;
			AudioStreamPacketDescriptionExtended mValue; // in order to support operator-> .
	};
	
	iterator begin() const { return iterator(this, 0); }
	iterator end() const { return iterator(this, mSize); }
	
private:
	struct TinyContiguousPacketDescription
	{
		UInt16 mNextOffset;
	};
	
	struct TinyDiscontiguousPacketDescription : TinyContiguousPacketDescription
	{
		UInt16 mDataByteSize;
	};
	
	// There could be a 24 bit packet description. But ALAC is who usually needs SmallContiguousPacketDescription and 
	// it already uses 8x fewer packet descriptions than AAC due to the mFramesPerPacket being 8x greater. 
	// So there isn't as great a need for saving space. 4 bytes per packet is OK for ALAC.
	
	struct SmallContiguousPacketDescription
	{
		UInt32 mNextOffset;
	};
	
	struct SmallDiscontiguousPacketDescription : SmallContiguousPacketDescription
	{
		UInt16 mDataByteSize;
	};
	
	struct BigContiguousPacketDescription
	{
		UInt64 mNextOffset;
	};
	
	struct BigDiscontiguousPacketDescription : BigContiguousPacketDescription
	{
		UInt32 mDataByteSize;
	};
	
	struct PacketBase
	{
		SInt64 mBaseOffset;
		UInt8 mDescType;
		void* mDescs;
	};
	
	enum {
		kTinyContiguousPacketDescription,
		kTinyDiscontiguousPacketDescription,
		kSmallContiguousPacketDescription,
		kSmallDiscontiguousPacketDescription,
		kBigContiguousPacketDescription,
		kBigDiscontiguousPacketDescription,
		kExtendedPacketDescription
	};
	static const UInt32 kShift;
	static const UInt32 kMask;
	
	bool isContiguous(PacketBase& base);
	bool hasVariableFrames(PacketBase& base);
	UInt32 largestPacket(PacketBase& base);
	
	void Compress(PacketBase& base);

private:	
	std::vector<PacketBase> mBases;
	UInt64 mSize;
	UInt32 mFramesPerPacket;
};


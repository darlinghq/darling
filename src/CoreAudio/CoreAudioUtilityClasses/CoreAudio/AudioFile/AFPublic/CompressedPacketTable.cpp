/*
     File: CompressedPacketTable.cpp
 Abstract: CompressedPacketTable.h
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
#include "CompressedPacketTable.h"
#include "CAAutoDisposer.h"

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

const UInt32 CompressedPacketTable::kShift = 5;
const UInt32 CompressedPacketTable::kMask = 31;

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

CompressedPacketTable::~CompressedPacketTable()
{
	size_t size = mBases.size();
	for (size_t i = 0; i < size; ++i) {
		free(mBases[i].mDescs);
	}
}

void CompressedPacketTable::push_back(const AudioStreamPacketDescriptionExtended& inDesc)
{
	SInt64 baseIndex = mSize >> kShift;
	UInt32 packetIndex = (UInt32)(mSize & kMask);
	
	if (packetIndex == 0) {
		// first packet in a new sequence. create a new PacketBase.
		PacketBase newBase;
		newBase.mBaseOffset = 0;
		newBase.mDescs = CA_malloc((kMask+1) * sizeof(AudioStreamPacketDescriptionExtended));
		newBase.mDescType = kExtendedPacketDescription;
		mBases.push_back(newBase);
	}
	
	PacketBase& base = mBases[(size_t)baseIndex];
	AudioStreamPacketDescriptionExtended* descs = (AudioStreamPacketDescriptionExtended*)base.mDescs;
	descs[packetIndex] = inDesc;
	
	if (packetIndex == kMask) {
		// last packet in a sequence. compress the sequence.
		Compress(base);
	}
	
	mSize++;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define ACCESS_TYPE(TYPE) \
		case k##TYPE##ContiguousPacketDescription : { \
			TYPE##ContiguousPacketDescription* descs = (TYPE##ContiguousPacketDescription*)base.mDescs; \
			packetOffset = packetIndex ? descs[packetIndex-1].mNextOffset : 0; \
			packetSize = (UInt32)(descs[packetIndex].mNextOffset - packetOffset); \
		} break; \
		case k##TYPE##DiscontiguousPacketDescription : { \
			TYPE##DiscontiguousPacketDescription* descs = (TYPE##DiscontiguousPacketDescription*)base.mDescs; \
			packetOffset = packetIndex ? descs[packetIndex-1].mNextOffset : 0; \
			packetSize = descs[packetIndex].mDataByteSize; \
		} break;

const AudioStreamPacketDescriptionExtended CompressedPacketTable::operator[](SInt64 inPacketIndex) const
{
	SInt64 baseIndex = inPacketIndex >> kShift;
	UInt32 packetIndex = (UInt32)(inPacketIndex & kMask);

	if ((size_t)baseIndex >= mBases.size())
		throw -1;
		
	const PacketBase& base = mBases[(size_t)baseIndex];
	
	SInt64 packetOffset = 0;
	UInt32 packetSize = 0;
		
	switch (base.mDescType) 
	{
		ACCESS_TYPE(Tiny)
		ACCESS_TYPE(Small)
		ACCESS_TYPE(Big)
		case kExtendedPacketDescription :
			return ((AudioStreamPacketDescriptionExtended*)base.mDescs)[packetIndex];
	}
	
	AudioStreamPacketDescriptionExtended outDesc;
	outDesc.mStartOffset = base.mBaseOffset + packetOffset;
	outDesc.mDataByteSize = packetSize;
	outDesc.mVariableFramesInPacket = 0;
	outDesc.mFrameOffset = mFramesPerPacket * inPacketIndex;

	//printf("get %d %10qd   %10qd %2d   %10qd %6d %10qd\n", base.mDescType, inPacketIndex, baseIndex, packetIndex, outDesc.mStartOffset, outDesc.mDataByteSize, outDesc.mFrameOffset);
	
	return outDesc;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool CompressedPacketTable::isContiguous(PacketBase& base)
{	
	AudioStreamPacketDescriptionExtended* descs = (AudioStreamPacketDescriptionExtended*)base.mDescs;
	SInt64 expectedOffset = descs[0].mStartOffset + descs[0].mDataByteSize;
	for (UInt32 i = 1; i <= kMask; ++i) {
		if (expectedOffset != descs[i].mStartOffset) return false;
		expectedOffset += descs[i].mDataByteSize;
	}
	return true;
}

bool CompressedPacketTable::hasVariableFrames(PacketBase& base)
{	
	AudioStreamPacketDescriptionExtended* descs = (AudioStreamPacketDescriptionExtended*)base.mDescs;
	for (UInt32 i = 0; i <= kMask; ++i) {
		if (descs[i].mVariableFramesInPacket) return true;
	}
	return false;
}

UInt32 CompressedPacketTable::largestPacket(PacketBase& base)
{	
	UInt32 maxPacketSize = 0;
	AudioStreamPacketDescriptionExtended* descs = (AudioStreamPacketDescriptionExtended*)base.mDescs;
	for (UInt32 i = 0; i <= kMask; ++i) {
		UInt32 packetSize = descs[i].mDataByteSize;
		if (packetSize > maxPacketSize) maxPacketSize = packetSize;
	}
	return maxPacketSize;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#define COMPRESS_TYPE(TYPE, BITS) \
	if (contiguous) { \
		TYPE##ContiguousPacketDescription* newDescs = (TYPE##ContiguousPacketDescription*)CA_malloc((kMask+1) * sizeof(TYPE##ContiguousPacketDescription)); \
		base.mDescs = newDescs; \
		base.mDescType = k##TYPE##ContiguousPacketDescription; \
		for (UInt32 i = 0; i <= kMask; ++i) { \
			newDescs[i].mNextOffset = (BITS)(descs[i].mStartOffset + descs[i].mDataByteSize - baseOffset); \
		} \
		free(descs); \
	} else { \
		TYPE##DiscontiguousPacketDescription* newDescs = (TYPE##DiscontiguousPacketDescription*)CA_malloc((kMask+1) * sizeof(TYPE##DiscontiguousPacketDescription)); \
		base.mDescs = newDescs; \
		base.mDescType = k##TYPE##DiscontiguousPacketDescription; \
		for (UInt32 i = 0; i <= kMask; ++i) { \
			newDescs[i].mNextOffset = i == kMask ? 0 : (BITS)(descs[i+1].mStartOffset - baseOffset); \
			newDescs[i].mDataByteSize = descs[i].mDataByteSize; \
		} \
		free(descs); \
	} \
	return;

void CompressedPacketTable::Compress(PacketBase& base)
{
	if (hasVariableFrames(base)) 
		return;
	
	bool contiguous = isContiguous(base);
	
	AudioStreamPacketDescriptionExtended* descs = (AudioStreamPacketDescriptionExtended*)base.mDescs;
	SInt64 delta = descs[kMask].mStartOffset + descs[kMask].mDataByteSize - descs[0].mStartOffset;
		
	SInt64 baseOffset = descs[0].mStartOffset;
	base.mBaseOffset = baseOffset;
	
	if (delta <= 65535LL) {
		COMPRESS_TYPE(Tiny, UInt16)
	} else if (delta <= 4294967295LL && largestPacket(base) <= 65535) {
		COMPRESS_TYPE(Small, UInt32)
	} else {
		COMPRESS_TYPE(Big, SInt64)
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

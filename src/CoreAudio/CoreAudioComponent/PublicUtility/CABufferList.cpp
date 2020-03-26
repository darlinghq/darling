/*
Copyright (C) 2016 Apple Inc. All Rights Reserved.
See LICENSE.txt for this sample’s licensing information

Abstract:
Part of Core Audio Public Utility Classes
*/

#include "CABufferList.h"
#include "CAByteOrder.h"

void		CABufferList::AllocateBuffers(UInt32 nBytes)
{
	if (nBytes <= GetNumBytes()) return;

	if (mABL.mNumberBuffers > 1)
		// align successive buffers for Altivec and to take alternating
		// cache line hits by spacing them by odd multiples of 16
		nBytes = ((nBytes + 15) & ~15) | 16;
	UInt32 memorySize = nBytes * mABL.mNumberBuffers;
	Byte *newMemory = new Byte[memorySize], *p = newMemory;
	memset(newMemory, 0, memorySize);	// get page faults now, not later
	
	AudioBuffer *buf = mABL.mBuffers;
	for (UInt32 i = mABL.mNumberBuffers; i--; ++buf) {
		if (buf->mData != NULL && buf->mDataByteSize > 0)
			// preserve existing buffer contents
			memcpy(p, buf->mData, buf->mDataByteSize);
		buf->mDataByteSize = nBytes;
		buf->mData = p;
		p += nBytes;
	}
	Byte *oldMemory = mBufferMemory;
	mBufferMemory = newMemory;
	mBufferCapacity = nBytes;
	delete[] oldMemory;
}

void		CABufferList::AllocateBuffersAndCopyFrom(UInt32 nBytes, CABufferList *inSrcList, CABufferList *inSetPtrList)
{
	if (mABL.mNumberBuffers != inSrcList->mABL.mNumberBuffers) return;
	if (mABL.mNumberBuffers != inSetPtrList->mABL.mNumberBuffers) return;
	if (nBytes <= GetNumBytes()) {
		CopyAllFrom(inSrcList, inSetPtrList);
		return;
	}
	inSetPtrList->VerifyNotTrashingOwnedBuffer();
	UInt32 fromByteSize = inSrcList->GetNumBytes();
	
	if (mABL.mNumberBuffers > 1)
		// align successive buffers for Altivec and to take alternating
		// cache line hits by spacing them by odd multiples of 16
		nBytes = ((nBytes + 15) & ~15) | 16;
	UInt32 memorySize = nBytes * mABL.mNumberBuffers;
	Byte *newMemory = new Byte[memorySize], *p = newMemory;
	memset(newMemory, 0, memorySize);	// make buffer "hot"
	
	AudioBuffer *buf = mABL.mBuffers;
	AudioBuffer *ptrBuf = inSetPtrList->mABL.mBuffers;
	AudioBuffer *srcBuf = inSrcList->mABL.mBuffers;
	for (UInt32 i = mABL.mNumberBuffers; i--; ++buf, ++ptrBuf, ++srcBuf) {
		if (srcBuf->mData != NULL && srcBuf->mDataByteSize > 0)
			// preserve existing buffer contents
			memmove(p, srcBuf->mData, srcBuf->mDataByteSize);
		buf->mDataByteSize = nBytes;
		buf->mData = p;
		ptrBuf->mDataByteSize = srcBuf->mDataByteSize;
		ptrBuf->mData = p;
		p += nBytes;
	}
	Byte *oldMemory = mBufferMemory;
	mBufferMemory = newMemory;
	mBufferCapacity = nBytes;
	if (inSrcList != inSetPtrList)
		inSrcList->BytesConsumed(fromByteSize);
	delete[] oldMemory;
}

void		CABufferList::DeallocateBuffers()
{
	AudioBuffer *buf = mABL.mBuffers;
	for (UInt32 i = mABL.mNumberBuffers; i--; ++buf) {
		buf->mData = NULL;
		buf->mDataByteSize = 0;
	}
	if (mBufferMemory != NULL) {
		delete[] mBufferMemory;
		mBufferMemory = NULL;
		mBufferCapacity = 0;
	}
    
}

static void show(const AudioBufferList &abl, int framesToPrint, int wordSize, const char *label, const char *fmtstr=NULL)
{
	printf("%s %p (%d fr%s):\n", label ? label : "AudioBufferList", &abl, framesToPrint, fmtstr ? fmtstr : "");
	const AudioBuffer *buf = abl.mBuffers;
	for (UInt32 i = 0; i < abl.mNumberBuffers; ++i, ++buf) {
		printf("    [%2d] %5dbytes %dch @ %p", (int)i, (int)buf->mDataByteSize, (int)buf->mNumberChannels, buf->mData);
		if (framesToPrint && buf->mData != NULL) {
			printf(":");
			Byte *p = (Byte *)buf->mData;
			for (int j = framesToPrint * buf->mNumberChannels; --j >= 0; )
				switch (wordSize) {
				case 0:	// native float
					printf(" %6.3f", *(Float32 *)p);
					p += sizeof(Float32);
					break;
				// positive: big endian
				case 1:
				case -1:
					printf(" %02X", *p);
					p += 1;
					break;
				case 2:
					printf(" %04X", CFSwapInt16BigToHost(*(UInt16 *)p));
					p += 2;
					break;
				case 3:
					printf(" %06X", (p[0] << 16) | (p[1] << 8) | p[2]);
					p += 3;
					break;
				case 4:
					printf(" %08X", (unsigned int)CFSwapInt32BigToHost(*(UInt32 *)p));
					p += 4;
					break;
				case 10:
					printf(" %6.3f", CASwapFloat32BigToHost(*(Float32 *)p));
					p += sizeof(Float32);
					break;
				case -2:
					printf(" %04X", CFSwapInt16LittleToHost(*(UInt16 *)p));
					p += 2;
					break;
				case -3:
					printf(" %06X", (p[2] << 16) | (p[1] << 8) | p[0]);
					p += 3;
					break;
				case -4:
					printf(" %08X", (unsigned int)CFSwapInt32LittleToHost(*(UInt32 *)p));
					p += 4;
					break;
				case -10:
					printf(" %6.3f", CASwapFloat32LittleToHost(*(Float32 *)p));
					p += sizeof(Float32);
					break;
				}
		}
		printf("\n");
	}
}

void CAShowAudioBufferList(const AudioBufferList &abl, int framesToPrint, const AudioStreamBasicDescription &asbd, const char *label)
{
	CAStreamBasicDescription fmt(asbd);
	int wordSize = 1;
	char fmtstr[80] = { 0 };
	
	if (fmt.mFormatID == kAudioFormatLinearPCM) {
		if (fmt.mFormatFlags & kLinearPCMFormatFlagIsFloat) {
			if (fmt.mBitsPerChannel == 32) {
				if (fmt.mFormatFlags & kLinearPCMFormatFlagIsBigEndian) {
					wordSize = 10;
					strlcpy(fmtstr, ", BEF", sizeof(fmtstr));
				} else {
					wordSize = -10;
					strlcpy(fmtstr, ", LEF", sizeof(fmtstr));
				}
			}
		} else {
			wordSize = fmt.SampleWordSize();
			if (wordSize > 0) {
				int fracbits = (asbd.mFormatFlags & kLinearPCMFormatFlagsSampleFractionMask) >> kLinearPCMFormatFlagsSampleFractionShift;
				if (fracbits > 0)
					snprintf(fmtstr, sizeof(fmtstr), ", %d.%d-bit", (int)asbd.mBitsPerChannel - fracbits, fracbits);
				else
					snprintf(fmtstr, sizeof(fmtstr), ", %d-bit", (int)asbd.mBitsPerChannel);

				if (!(fmt.mFormatFlags & kLinearPCMFormatFlagIsBigEndian)) {
					wordSize = -wordSize;
					strlcat(fmtstr, " LEI", sizeof(fmtstr));
				} else {
					strlcat(fmtstr, " BEI", sizeof(fmtstr));
				}
			}
		}
	}
	show(abl, framesToPrint, wordSize, label, fmtstr);
}

void CAShowAudioBufferList(const AudioBufferList &abl, int framesToPrint, int wordSize, const char *label)
{
	show(abl, framesToPrint, wordSize, label);
}

extern "C" void CAShowAudioBufferList(const AudioBufferList *abl, int framesToPrint, int wordSize)
{
	show(*abl, framesToPrint, wordSize, NULL);
}

#pragma mark- CrashIfClientProvidedBogusAudioBufferList Diagostic Function
// if the return result is odd, there was a null buffer.
/*
extern "C" int CrashIfClientProvidedBogusAudioBufferList(const AudioBufferList *abl, bool nullok)
{
	const AudioBuffer *buf = abl->mBuffers, *bufend = buf + abl->mNumberBuffers;
	int sum = 0;	// defeat attempts by the compiler to optimize away the code that touches the buffers
	int anyNull = 0;
	for ( ; buf < bufend; ++buf) {
		const int *p = (const int *)buf->mData;
		if (p == NULL) {
			anyNull = 1;
			if (nullok) continue;
		}
		unsigned datasize = buf->mDataByteSize;
		if (datasize >= sizeof(int)) {
			sum += p[0];
			sum += p[datasize / sizeof(int) - 1];
		}
	}
	return anyNull | (sum & ~1);
}
*/

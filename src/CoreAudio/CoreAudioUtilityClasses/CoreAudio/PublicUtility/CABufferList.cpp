/*
     File: CABufferList.cpp
 Abstract: CABufferList.h
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

// if the return result is odd, there was a null buffer.
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
		if (datasize >= sizeof(int) && p != NULL) {
			sum += p[0];
			sum += p[datasize / sizeof(int) - 1];
		}
	}
	return anyNull | (sum & ~1);
}


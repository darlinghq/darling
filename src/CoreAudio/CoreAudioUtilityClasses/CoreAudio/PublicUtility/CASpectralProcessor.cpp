/*
     File: CASpectralProcessor.cpp
 Abstract: CASpectralProcessor.h
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
 
//#include "AudioFormulas.h"
#include "CASpectralProcessor.h"
#include "CABitOperations.h"


#include <Accelerate/Accelerate.h>


#define OFFSETOF(class, field)((size_t)&((class*)0)->field)

CASpectralProcessor::CASpectralProcessor(UInt32 inFFTSize, UInt32 inHopSize, UInt32 inNumChannels, UInt32 inMaxFrames)
	: mFFTSize(inFFTSize), mHopSize(inHopSize), mNumChannels(inNumChannels), mMaxFrames(inMaxFrames),
	mLog2FFTSize(Log2Ceil(mFFTSize)), 
	mFFTMask(mFFTSize - 1),
	mFFTByteSize(mFFTSize * sizeof(Float32)),
	mIOBufSize(NextPowerOfTwo(mFFTSize + mMaxFrames)),
	mIOMask(mIOBufSize - 1),
	mInputSize(0),
	mInputPos(0), mOutputPos(-mFFTSize & mIOMask), 
	mInFFTPos(0), mOutFFTPos(0),
	mSpectralFunction(0), mUserData(0)
{
	mWindow.alloc(mFFTSize, false);
	SineWindow(); // set default window.
	
	mChannels.alloc(mNumChannels);
	mSpectralBufferList.allocBytes(OFFSETOF(SpectralBufferList, mDSPSplitComplex[mNumChannels]), true);
	mSpectralBufferList->mNumberSpectra = mNumChannels;
	for (UInt32 i = 0; i < mNumChannels; ++i) 
	{
		mChannels[i].mInputBuf.alloc(mIOBufSize, true);
		mChannels[i].mOutputBuf.alloc(mIOBufSize, true);
		mChannels[i].mFFTBuf.alloc(mFFTSize, true);
		mChannels[i].mSplitFFTBuf.alloc(mFFTSize, true);
		mSpectralBufferList->mDSPSplitComplex[i].realp = mChannels[i].mSplitFFTBuf();
		mSpectralBufferList->mDSPSplitComplex[i].imagp = mChannels[i].mSplitFFTBuf() + (mFFTSize >> 1);
	}

	mFFTSetup = vDSP_create_fftsetup (mLog2FFTSize, FFT_RADIX2);
	
}

CASpectralProcessor::~CASpectralProcessor()
{
	mWindow.free();
	mChannels.free();
	mSpectralBufferList.free();
	vDSP_destroy_fftsetup(mFFTSetup);
}

void CASpectralProcessor::Reset()
{
	mInputPos = 0;
	mOutputPos = -mFFTSize & mIOMask;
	mInFFTPos = 0;
	mOutFFTPos = 0;
	
	for (UInt32 i = 0; i < mNumChannels; ++i) 
	{
		memset(mChannels[i].mInputBuf(), 0, mIOBufSize * sizeof(Float32));
		memset(mChannels[i].mOutputBuf(), 0, mIOBufSize * sizeof(Float32));
		memset(mChannels[i].mFFTBuf(), 0, mFFTSize * sizeof(Float32));
	}
}

const double two_pi = 2. * M_PI;

void CASpectralProcessor::HanningWindow()
{ 
	// this is also vector optimized

	double w = two_pi / (double)(mFFTSize - 1);
	for (UInt32 i = 0; i < mFFTSize; ++i)
	{
		mWindow[i] = (0.5 - 0.5 * cos(w * (double)i));	
	}
}

void CASpectralProcessor::SineWindow()
{
	double w = M_PI / (double)(mFFTSize - 1);
	for (UInt32 i = 0; i < mFFTSize; ++i)
	{
		mWindow[i] = sin(w * (double)i);
	}
}

void CASpectralProcessor::Process(UInt32 inNumFrames, AudioBufferList* inInput, AudioBufferList* outOutput)
{
	// copy from buffer list to input buffer
	CopyInput(inNumFrames, inInput);
	
	// if enough input to process, then process.
	while (mInputSize >= mFFTSize) 
	{
		CopyInputToFFT(); // copy from input buffer to fft buffer
		DoWindowing();
		DoFwdFFT();
		ProcessSpectrum(mFFTSize, mSpectralBufferList());
		DoInvFFT();
		DoWindowing();
		OverlapAddOutput();
	}

	// copy from output buffer to buffer list
	CopyOutput(inNumFrames, outOutput);
}

void CASpectralProcessor::DoWindowing()
{
	Float32 *win = mWindow();
	if (!win) return;
	for (UInt32 i=0; i<mNumChannels; ++i) {
		Float32 *x = mChannels[i].mFFTBuf();
		vDSP_vmul(x, 1, win, 1, x, 1, mFFTSize);
	}
	//printf("DoWindowing %g %g\n", mChannels[0].mFFTBuf()[0], mChannels[0].mFFTBuf()[200]);
}



void CASpectralProcessor::CopyInput(UInt32 inNumFrames, AudioBufferList* inInput)
{
	UInt32 numBytes = inNumFrames * sizeof(Float32);
	UInt32 firstPart = mIOBufSize - mInputPos;
	

	if (firstPart < inNumFrames) {
		UInt32 firstPartBytes = firstPart * sizeof(Float32);
		UInt32 secondPartBytes = numBytes - firstPartBytes;
		for (UInt32 i=0; i<mNumChannels; ++i) {		
			memcpy(mChannels[i].mInputBuf + mInputPos, inInput->mBuffers[i].mData, firstPartBytes);
			memcpy(mChannels[i].mInputBuf, (UInt8*)inInput->mBuffers[i].mData + firstPartBytes, secondPartBytes);
		}
	} else {
		UInt32 numBytes = inNumFrames * sizeof(Float32);
		for (UInt32 i=0; i<mNumChannels; ++i) {		
			memcpy(mChannels[i].mInputBuf + mInputPos, inInput->mBuffers[i].mData, numBytes);
		}
	}
	//printf("CopyInput %g %g\n", mChannels[0].mInputBuf[mInputPos], mChannels[0].mInputBuf[(mInputPos + 200) & mIOMask]);
	//printf("CopyInput mInputPos %u   mIOBufSize %u\n", (unsigned)mInputPos, (unsigned)mIOBufSize);
	mInputSize += inNumFrames;
	mInputPos = (mInputPos + inNumFrames) & mIOMask;
}

void CASpectralProcessor::CopyOutput(UInt32 inNumFrames, AudioBufferList* outOutput)
{
	//printf("->CopyOutput %g %g\n", mChannels[0].mOutputBuf[mOutputPos], mChannels[0].mOutputBuf[(mOutputPos + 200) & mIOMask]);
	//printf("CopyOutput mOutputPos %u\n", (unsigned)mOutputPos);
	UInt32 numBytes = inNumFrames * sizeof(Float32);
	UInt32 firstPart = mIOBufSize - mOutputPos;
	if (firstPart < inNumFrames) {
		UInt32 firstPartBytes = firstPart * sizeof(Float32);
		UInt32 secondPartBytes = numBytes - firstPartBytes;
		for (UInt32 i=0; i<mNumChannels; ++i) {
			memcpy(outOutput->mBuffers[i].mData, mChannels[i].mOutputBuf + mOutputPos, firstPartBytes);
			memcpy((UInt8*)outOutput->mBuffers[i].mData + firstPartBytes, mChannels[i].mOutputBuf, secondPartBytes);
			memset(mChannels[i].mOutputBuf + mOutputPos, 0, firstPartBytes);
			memset(mChannels[i].mOutputBuf, 0, secondPartBytes);
		}
	} else {
		for (UInt32 i=0; i<mNumChannels; ++i) {
			memcpy(outOutput->mBuffers[i].mData, mChannels[i].mOutputBuf + mOutputPos, numBytes);
			memset(mChannels[i].mOutputBuf + mOutputPos, 0, numBytes);
		}
	}
	//printf("<-CopyOutput %g %g\n", ((Float32*)outOutput->mBuffers[0].mData)[0], ((Float32*)outOutput->mBuffers[0].mData)[200]);
	mOutputPos = (mOutputPos + inNumFrames) & mIOMask;
}

void CASpectralProcessor::PrintSpectralBufferList()
{
	UInt32 half = mFFTSize >> 1;
	for (UInt32 i=0; i<mNumChannels; ++i) {
		DSPSplitComplex	&freqData = mSpectralBufferList->mDSPSplitComplex[i];
	
		for (UInt32 j=0; j<half; j++){
			printf(" bin[%d]: %lf + %lfi\n", (int) j, freqData.realp[j], freqData.imagp[j]);
		}
	}
}


void CASpectralProcessor::CopyInputToFFT()
{
	//printf("CopyInputToFFT mInFFTPos %u\n", (unsigned)mInFFTPos);
	UInt32 firstPart = mIOBufSize - mInFFTPos;
	UInt32 firstPartBytes = firstPart * sizeof(Float32);
	if (firstPartBytes < mFFTByteSize) {
		UInt32 secondPartBytes = mFFTByteSize - firstPartBytes;
		for (UInt32 i=0; i<mNumChannels; ++i) {
			memcpy(mChannels[i].mFFTBuf(), mChannels[i].mInputBuf() + mInFFTPos, firstPartBytes);
			memcpy((UInt8*)mChannels[i].mFFTBuf() + firstPartBytes, mChannels[i].mInputBuf(), secondPartBytes);
		}
	} else {
		for (UInt32 i=0; i<mNumChannels; ++i) {
			memcpy(mChannels[i].mFFTBuf(), mChannels[i].mInputBuf() + mInFFTPos, mFFTByteSize);
		}
	}
	mInputSize -= mHopSize;
	mInFFTPos = (mInFFTPos + mHopSize) & mIOMask;
	//printf("CopyInputToFFT %g %g\n", mChannels[0].mFFTBuf()[0], mChannels[0].mFFTBuf()[200]);
}

void CASpectralProcessor::OverlapAddOutput()
{
	//printf("OverlapAddOutput mOutFFTPos %u\n", (unsigned)mOutFFTPos);
	UInt32 firstPart = mIOBufSize - mOutFFTPos;
	if (firstPart < mFFTSize) {
		UInt32 secondPart = mFFTSize - firstPart;
		for (UInt32 i=0; i<mNumChannels; ++i) {
			float* out1 = mChannels[i].mOutputBuf() + mOutFFTPos;
			vDSP_vadd(out1, 1, mChannels[i].mFFTBuf(), 1, out1, 1, firstPart);
			float* out2 = mChannels[i].mOutputBuf();
			vDSP_vadd(out2, 1, mChannels[i].mFFTBuf() + firstPart, 1, out2, 1, secondPart);
		}
	} else {
		for (UInt32 i=0; i<mNumChannels; ++i) {
			float* out1 = mChannels[i].mOutputBuf() + mOutFFTPos;
			vDSP_vadd(out1, 1, mChannels[i].mFFTBuf(), 1, out1, 1, mFFTSize);
		}
	}
	//printf("OverlapAddOutput %g %g\n", mChannels[0].mOutputBuf[mOutFFTPos], mChannels[0].mOutputBuf[(mOutFFTPos + 200) & mIOMask]);
	mOutFFTPos = (mOutFFTPos + mHopSize) & mIOMask;
}


void CASpectralProcessor::DoFwdFFT()
{
	//printf("->DoFwdFFT %g %g\n", mChannels[0].mFFTBuf()[0], mChannels[0].mFFTBuf()[200]);
	UInt32 half = mFFTSize >> 1;
	for (UInt32 i=0; i<mNumChannels; ++i) 
	{
		vDSP_ctoz((DSPComplex*)mChannels[i].mFFTBuf(), 2, &mSpectralBufferList->mDSPSplitComplex[i], 1, half);
		vDSP_fft_zrip(mFFTSetup, &mSpectralBufferList->mDSPSplitComplex[i], 1, mLog2FFTSize, FFT_FORWARD);
	}
	//printf("<-DoFwdFFT %g %g\n", direction, mChannels[0].mFFTBuf()[0], mChannels[0].mFFTBuf()[200]);
}

void CASpectralProcessor::DoInvFFT()
{
	//printf("->DoInvFFT %g %g\n", mChannels[0].mFFTBuf()[0], mChannels[0].mFFTBuf()[200]);
	UInt32 half = mFFTSize >> 1;
	for (UInt32 i=0; i<mNumChannels; ++i) 
	{
		vDSP_fft_zrip(mFFTSetup, &mSpectralBufferList->mDSPSplitComplex[i], 1, mLog2FFTSize, FFT_INVERSE);
		vDSP_ztoc(&mSpectralBufferList->mDSPSplitComplex[i], 1, (DSPComplex*)mChannels[i].mFFTBuf(), 2, half);		
		float scale = 0.5 / mFFTSize;
		vDSP_vsmul(mChannels[i].mFFTBuf(), 1, &scale, mChannels[i].mFFTBuf(), 1, mFFTSize );
	}
	//printf("<-DoInvFFT %g %g\n", direction, mChannels[0].mFFTBuf()[0], mChannels[0].mFFTBuf()[200]);
}

void CASpectralProcessor::SetSpectralFunction(SpectralFunction inFunction, void* inUserData)
{
	mSpectralFunction = inFunction; 
	mUserData = inUserData;
}

void CASpectralProcessor::ProcessSpectrum(UInt32 inFFTSize, SpectralBufferList* inSpectra)
{
	if (mSpectralFunction)
		(mSpectralFunction)(inSpectra, mUserData);
}

#pragma mark ___Utility___

void CASpectralProcessor::GetMagnitude(AudioBufferList* list, Float32* min, Float32* max) 
{	
	UInt32 half = mFFTSize >> 1;	
	for (UInt32 i=0; i<mNumChannels; ++i) {
		DSPSplitComplex	&freqData = mSpectralBufferList->mDSPSplitComplex[i];		
		
		Float32* b = (Float32*) list->mBuffers[i].mData;
		
		vDSP_zvabs(&freqData,1,b,1,half); 		
   
		vDSP_maxmgv(b, 1, &max[i], half); 
 		vDSP_minmgv(b, 1, &min[i], half); 
		
   } 
}


void CASpectralProcessor::GetFrequencies(Float32* freqs, Float32 sampleRate)
{
	UInt32 half = mFFTSize >> 1;	

	for (UInt32 i=0; i< half; i++){
		freqs[i] = ((Float32)(i))*sampleRate/((Float32)mFFTSize);	
	}
}


bool CASpectralProcessor::ProcessForwards(UInt32 inNumFrames, AudioBufferList* inInput)
{
	// copy from buffer list to input buffer
	CopyInput(inNumFrames, inInput);
		
	bool processed = false;
	// if enough input to process, then process.
	while (mInputSize >= mFFTSize) 
	{
		CopyInputToFFT(); // copy from input buffer to fft buffer
		DoWindowing();
		DoFwdFFT();
		ProcessSpectrum(mFFTSize, mSpectralBufferList()); // here you would copy the fft results out to a buffer indicated in mUserData, say for sonogram drawing
		processed = true;
	}
	
	return processed;
}

bool CASpectralProcessor::ProcessBackwards(UInt32 inNumFrames, AudioBufferList* outOutput)
{		
	
	ProcessSpectrum(mFFTSize, mSpectralBufferList());
	DoInvFFT();
	DoWindowing();
	OverlapAddOutput();		
	
	// copy from output buffer to buffer list
	CopyOutput(inNumFrames, outOutput);
	
	return true;
}



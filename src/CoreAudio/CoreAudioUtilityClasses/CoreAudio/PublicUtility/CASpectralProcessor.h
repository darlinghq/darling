/*
     File: CASpectralProcessor.h
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
#ifndef _SpectralProcesor_H_
#define _SpectralProcesor_H_
 
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreFoundation/CoreFoundation.h>
#else
#include <CoreAudioTypes.h>
#include <CoreFoundation.h>
#endif

#include <Accelerate/Accelerate.h>

#include "CAAutoDisposer.h"

struct SpectralBufferList
{
	UInt32 mNumberSpectra;
	DSPSplitComplex mDSPSplitComplex[1];
};

class CASpectralProcessor 
{
public:
	CASpectralProcessor(UInt32 inFFTSize, UInt32 inHopSize, UInt32 inNumChannels, UInt32 inMaxFrames);
	virtual ~CASpectralProcessor();
	
	void Reset();
	
	void Process(UInt32 inNumFrames, AudioBufferList* inInput, AudioBufferList* outOutput);
	
	typedef void (*SpectralFunction)(SpectralBufferList* inSpectra, void* inUserData);
	
	void SetSpectralFunction(SpectralFunction inFunction, void* inUserData);
	
	UInt32 FFTSize() const { return mFFTSize; }
	UInt32 MaxFrames() const { return mMaxFrames; }
	UInt32 NumChannels() const { return mNumChannels; }
	UInt32 HopSize() const { return mHopSize; }
	Float32* Window() const { return mWindow; }
	
	
	void HanningWindow(); // set up a hanning window
	void SineWindow();
	
	void GetFrequencies(Float32* freqs, Float32 sampleRate);				// only for processed forward
	void GetMagnitude(AudioBufferList* inCopy, Float32* min, Float32* max); // only for processed forward
	
	virtual bool ProcessForwards(UInt32 inNumFrames, AudioBufferList* inInput);
	bool ProcessBackwards(UInt32 inNumFrames, AudioBufferList* outOutput);


	void PrintSpectralBufferList();
	
protected:
	void CopyInput(UInt32 inNumFrames, AudioBufferList* inInput);
	void CopyInputToFFT();
	void DoWindowing();
	void DoFwdFFT();
	void DoInvFFT();
	void OverlapAddOutput();
	void CopyOutput(UInt32 inNumFrames, AudioBufferList* inOutput);
	void ProcessSpectrum(UInt32 inFFTSize, SpectralBufferList* inSpectra);
	
	UInt32 mFFTSize;
	UInt32 mHopSize;
	UInt32 mNumChannels;
	UInt32 mMaxFrames;

	UInt32 mLog2FFTSize;
	UInt32 mFFTMask; 
	UInt32 mFFTByteSize;
	UInt32 mIOBufSize;
	UInt32 mIOMask;
	UInt32 mInputSize;
	UInt32 mInputPos;
	UInt32 mOutputPos;
	UInt32 mInFFTPos;
	UInt32 mOutFFTPos;
	FFTSetup mFFTSetup;

	CAAutoFree<Float32> mWindow;
	struct SpectralChannel 
	{
		CAAutoFree<Float32> mInputBuf;		// log2ceil(FFT size + max frames)
		CAAutoFree<Float32> mOutputBuf;		// log2ceil(FFT size + max frames)
		CAAutoFree<Float32> mFFTBuf;		// FFT size
		CAAutoFree<Float32> mSplitFFTBuf;	// FFT size
	};
	CAAutoArrayDelete<SpectralChannel> mChannels;

	CAAutoFree<SpectralBufferList> mSpectralBufferList;
	
	SpectralFunction mSpectralFunction;
	void *mUserData;
	
};


#endif

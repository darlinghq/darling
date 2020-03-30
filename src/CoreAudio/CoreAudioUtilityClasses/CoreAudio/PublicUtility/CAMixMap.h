/*
     File: CAMixMap.h
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
#ifndef __CAMixMap_h__
#define __CAMixMap_h__

	// manages the setting of mix map volumes
	
class CAMixMap {
public:
				CAMixMap ()
					: mIns(0), mOuts (0), mMixMap(NULL)
					{}
					
				CAMixMap (const CAMixMap &mm) 
					: mIns(0), mOuts (0), mMixMap(NULL)
					{ *this = mm; }
				
				CAMixMap (UInt32 numIns, UInt32 numOuts) 
					: mIns(numIns), mOuts (numOuts), mMixMap(NULL)
					{ 
						mMixMap = new Float32[numIns * numOuts]; 
						memset (mMixMap, 0, ByteSize());
					}
								
				~CAMixMap () { delete [] mMixMap; }
	
	CAMixMap&	operator=(const CAMixMap& mm) 
				{ 
					if (mMixMap) { delete [] mMixMap; mMixMap = NULL; }
					mIns = mm.mIns; mOuts = mm.mOuts;
					if (NumIns() && NumOuts()) { 
						mMixMap = new Float32 [ NumIns() * NumOuts() ];
						memcpy (mMixMap, mm.mMixMap, ByteSize());
					}
					return *this;
				}
				
	UInt32		NumIns () const { return mIns; }
	UInt32		NumOuts () const { return mOuts; }

	void		SetCrossPoint (UInt32 inputChan, UInt32 outputChan, Float32 val)
				{
					if (inputChan < NumIns() && outputChan < NumOuts())
						mMixMap[inputChan * NumOuts() + outputChan] = val;
				}
	Float32		GetCrossPoint (UInt32 inputChan, UInt32 outputChan) const
				{
					return (inputChan < NumIns() && outputChan < NumOuts())
							? mMixMap[inputChan * NumOuts() + outputChan]
							: 0;
				}

	void		SetDiagonal (Float32 val)
				{
					for (UInt32 i = 0; i < NumIns() && i < NumOuts(); ++i) {
						mMixMap[i * NumOuts() + i] = val;
					}
				}
	
	void		Clear () { memset (mMixMap, 0, ByteSize()); }
	
	
	Float32*	MM() { return mMixMap; }
	const Float32*	MM() const { return mMixMap; }
	UInt32		ByteSize () const { return NumIns() * NumOuts() * sizeof(Float32); }

	UInt32		CountActiveInputs(UInt32 inOutputChannel)
				{
					UInt32 sum = 0;
					for (UInt32 i = 0, k = inOutputChannel; i < mIns; ++i, k+=mOuts) {
						if (mMixMap[k] != 0.f) sum++;
					}
					return sum;
				}
				
	void		Normalize()
				{
					// ensure that no output channel will sum over unity.
					Float32* mixmap = mMixMap;
					Float32 maxsum = 0.f;
					for (UInt32 j = 0; j < mOuts; ++j) {
						Float32 sum = 0.f;
						for (UInt32 i = 0, k = j; i < mIns; ++i, k+=mOuts) {
							sum += mixmap[k];
						}
						if (sum > maxsum) maxsum = sum;
					}
					
					if (maxsum == 0.f) return;
					Float32 scale = 1.f / maxsum;
					for (UInt32 i = 0; i < mIns * mOuts; ++i) {
						mixmap[i] *= scale;
					}
				}
	
	void		Print ()
				{
					printf ("Num Ins: %d, Num Outs: %d\n", (int)mIns, (int)mOuts);
					for (unsigned int ins = 0; ins < mIns; ++ins) {
						printf ("\t%d: ", ins);
						for (unsigned int outs = 0; outs < mOuts; ++outs)
							printf ("(%.3f) ", mMixMap[ins * NumOuts() + outs]); 
						printf("\n");
					}
				}
				
private:
	UInt32	mIns;
	UInt32	mOuts;
	Float32 *mMixMap;
};

#endif

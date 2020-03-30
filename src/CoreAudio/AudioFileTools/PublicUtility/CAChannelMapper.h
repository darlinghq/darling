/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple’s
			copyrights in this original Apple software (the "Apple Software"), to use,
			reproduce, modify and redistribute the Apple Software, with or without
			modifications, in source and/or binary forms; provided that if you redistribute
			the Apple Software in its entirety and without modifications, you must retain
			this notice and the following text and disclaimers in all such redistributions of
			the Apple Software.  Neither the name, trademarks, service marks or logos of
			Apple Computer, Inc. may be used to endorse or promote products derived from the
			Apple Software without specific prior written permission from Apple.  Except as
			expressly stated in this notice, no other rights or licenses, express or implied,
			are granted by Apple herein, including but not limited to any patent rights that
			may be infringed by your derivative works or by other works in which the Apple
			Software may be incorporated.

			The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
			WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
			WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A PARTICULAR
			PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION ALONE OR IN
			COMBINATION WITH YOUR PRODUCTS.

			IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
			CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
			GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
			ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION AND/OR DISTRIBUTION
			OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER UNDER THEORY OF CONTRACT, TORT
			(INCLUDING NEGLIGENCE), STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN
			ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/*=============================================================================
	CAChannelMapper.h
	
=============================================================================*/

#ifndef __CAChannelMapper_h__
#define __CAChannelMapper_h__

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioUnit/AudioUnit.h>
#else
	#include <AudioUnit.h>
#endif
#include "CAAudioChannelLayout.h"
#include "CAStreamBasicDescription.h"
#include "CAAudioUnit.h"
#include "MatrixMixerVolumes.h"

class CAChannelMapper {
public:
	CAChannelMapper(const CAStreamBasicDescription &srcFormat, 
					const CAStreamBasicDescription &destFormat,
					const CAAudioChannelLayout *	srcLayout = NULL,
					const CAAudioChannelLayout *	destLayout = NULL);
	~CAChannelMapper();
	
	UInt32			NumSourceChannels() { return mSrcNChannels; }
	UInt32			NumDestChannels() { return mDestNChannels; }
	const CAAudioChannelLayout	&SourceLayout() { return mSrcLayout; }
	const CAAudioChannelLayout	&DestLayout() { return mDestLayout; }
	AudioUnit		GetMixer() { return mMatrixMixer.AU(); }
	
	bool			CanDownmix() { return mSrcLayout.IsValid() && mDestLayout.IsValid(); }
	OSStatus		OpenMixer(double sampleRate);
	OSStatus		ResetMixer();		// enables all ins/outs, zeroes all crosspoints
	OSStatus		ConfigureDownmix();
	OSStatus		ConnectChannelToChannel(UInt32 inChannel, UInt32 outChannel);
	OSStatus		Mix(const AudioBufferList *src, AudioBufferList *dest, UInt32 nFrames);
	void			PrintMatrixMixerVolumes(FILE *f) { ::PrintMatrixMixerVolumes(f, mMatrixMixer.AU()); }
	
private:
	static OSStatus	MixerInputProc(
							void *						inRefCon,
							AudioUnitRenderActionFlags *ioActionFlags,
							const AudioTimeStamp *		inTimeStamp,
							UInt32 						inBusNumber,
							UInt32 						inNumberFrames,
							AudioBufferList *			ioData);

	UInt32					mSrcNChannels, mDestNChannels;
	CAAudioChannelLayout	mSrcLayout, mDestLayout;
	CAAudioUnit				mMatrixMixer;
	const AudioBufferList *	mMixInputBufferList;
};

#endif // __CAChannelMapper_h__

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
	audioformats.cpp
	
=============================================================================*/

#include "CAAudioFileFormats.h"
#include <stdlib.h>
#include "AFToolsCommon.h"

#if TARGET_OS_WIN32
	#include <QTML.h>
#endif

// This program interrogates the AudioFile and AudioFormat API's for information
// useful in constructing test scripts. It might also be useful as an example
// of how to use these API's.

void	PrintFileFormats()
{
	CAAudioFileFormats *fileFormats = CAAudioFileFormats::Instance();
	
	for (int i = 0; i < fileFormats->mNumFileFormats; ++i) {
		CAAudioFileFormats::FileFormatInfo *ffi = &fileFormats->mFileFormats[i];
		char buf[20], extbuf[32];
		printf("('%s', '%s'", OSTypeToStr(buf, ffi->mFileTypeID), ffi->GetExtension(0, extbuf, sizeof(extbuf)));
		
		for (int j = 0; j < ffi->mNumDataFormats; ++j) {
			CAAudioFileFormats::DataFormatInfo *dfi = &ffi->mDataFormats[j];
			if (dfi->mFormatID == kAudioFormatLinearPCM) {
				for (int k = 0; k < dfi->mNumVariants; ++k) {
					AudioStreamBasicDescription *asbd = &dfi->mVariants[k];
					if (asbd->mFormatFlags & ~(kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsFloat))
						printf("(%08lx/%ld) ", asbd->mFormatFlags, asbd->mBitsPerChannel);
					else {
						printf(", '%s",
							(asbd->mFormatFlags & kAudioFormatFlagIsBigEndian) ? "BE" : "LE");
						if (asbd->mFormatFlags & kAudioFormatFlagIsFloat)
							printf("F");
						else
							printf("%sI",
								(asbd->mFormatFlags & kAudioFormatFlagIsSignedInteger) ? "" : "U");
						printf("%ld'", asbd->mBitsPerChannel);
					}
				}
			} else {
				printf(", '%s'", OSTypeToStr(buf, dfi->mFormatID));
			}
		}
		printf("),\n");
	}
}

struct LayoutTag {
	UInt32		constant;
	const char *name;
};

#define TAG(x)	{ x, #x },

static LayoutTag gLayoutTags[] = {
	TAG(kAudioChannelLayoutTag_Stereo)
	TAG(kAudioChannelLayoutTag_AAC_Quadraphonic)
	TAG(kAudioChannelLayoutTag_AAC_4_0)
	TAG(kAudioChannelLayoutTag_AAC_5_0)
	TAG(kAudioChannelLayoutTag_AAC_5_1)
	TAG(kAudioChannelLayoutTag_AAC_6_0)	
	TAG(kAudioChannelLayoutTag_AAC_6_1)
	TAG(kAudioChannelLayoutTag_AAC_7_0)	
	TAG(kAudioChannelLayoutTag_AAC_7_1)
	TAG(kAudioChannelLayoutTag_AAC_Octagonal)
	TAG(kAudioChannelLayoutTag_MPEG_5_0_A)
	TAG(kAudioChannelLayoutTag_MPEG_5_0_B)
	TAG(kAudioChannelLayoutTag_MPEG_5_1_A)
	TAG(kAudioChannelLayoutTag_MPEG_5_1_B)
	{ 0, NULL }	// sentinel
};

static const char *prefix = "kAudioChannelLayoutTag_";

void	PrintCompatibleChannelLayouts()
{
	int plen = strlen(prefix);
	for (LayoutTag *tag1 = gLayoutTags; tag1->name != NULL; ++tag1) {
		AudioChannelLayout layout1 = { tag1->constant, 0, 0 };
		printf("\t'%s' : (", tag1->name + plen);
		int printed = 0;
		for (LayoutTag *tag2 = gLayoutTags; tag2->name != NULL; ++tag2) {
			AudioChannelLayout layout2 = { tag2->constant, 0, 0 };
			AudioChannelLayout *layouts[] = { &layout1, &layout2 };

			UInt32 propertySize;
			OSStatus err = AudioFormatGetPropertyInfo(kAudioFormatProperty_ChannelMap, sizeof(layouts), layouts, &propertySize);
			if (err == noErr) {
				SInt32 *map = (SInt32 *)malloc(propertySize);
				err = AudioFormatGetProperty(kAudioFormatProperty_ChannelMap, sizeof(layouts), layouts, &propertySize, map);
				if (err == noErr) {
					if (printed++) printf(", ");
					printf("'%s'", tag2->name + plen);
				}
			}
		}
		printf("),\n");
	}
}

int main(int argc, const char *argv[])
{
	#if TARGET_OS_WIN32
		InitializeQTML(0L);
	#endif

	PrintFileFormats();
	printf("#######################\n");
	PrintCompatibleChannelLayouts();
	
	return 0;
}

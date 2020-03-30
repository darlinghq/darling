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
	CAChannelLayouts.cpp
	
=============================================================================*/

#include "CAChannelLayouts.h"
#include <string.h>
#include <stdio.h>

struct LayoutTag {
	UInt32		constant;
	const char *name;
};

#define TAG(x)	{ x, #x },

static LayoutTag gLayoutTags[] = {
	TAG(kAudioChannelLayoutTag_Mono)
	TAG(kAudioChannelLayoutTag_Stereo)
	TAG(kAudioChannelLayoutTag_StereoHeadphones)
	TAG(kAudioChannelLayoutTag_MatrixStereo)
	TAG(kAudioChannelLayoutTag_MidSide)
	TAG(kAudioChannelLayoutTag_XY)
	TAG(kAudioChannelLayoutTag_Binaural)
	TAG(kAudioChannelLayoutTag_Ambisonic_B_Format)
	TAG(kAudioChannelLayoutTag_Quadraphonic)
	TAG(kAudioChannelLayoutTag_Pentagonal)
	TAG(kAudioChannelLayoutTag_Hexagonal)
	TAG(kAudioChannelLayoutTag_Octagonal)
	TAG(kAudioChannelLayoutTag_Cube)
	TAG(kAudioChannelLayoutTag_MPEG_1_0)
	TAG(kAudioChannelLayoutTag_MPEG_2_0)
	TAG(kAudioChannelLayoutTag_MPEG_3_0_A)
	TAG(kAudioChannelLayoutTag_MPEG_3_0_B)
	TAG(kAudioChannelLayoutTag_MPEG_4_0_A)
	TAG(kAudioChannelLayoutTag_MPEG_4_0_B)
	TAG(kAudioChannelLayoutTag_MPEG_5_0_A)
	TAG(kAudioChannelLayoutTag_MPEG_5_0_B)
	TAG(kAudioChannelLayoutTag_MPEG_5_0_C)
	TAG(kAudioChannelLayoutTag_MPEG_5_0_D)
	TAG(kAudioChannelLayoutTag_MPEG_5_1_A)
	TAG(kAudioChannelLayoutTag_MPEG_5_1_B)
	TAG(kAudioChannelLayoutTag_MPEG_5_1_C)
	TAG(kAudioChannelLayoutTag_MPEG_5_1_D)
	TAG(kAudioChannelLayoutTag_MPEG_6_1_A)
	TAG(kAudioChannelLayoutTag_MPEG_7_1_A)
	TAG(kAudioChannelLayoutTag_MPEG_7_1_B)
	TAG(kAudioChannelLayoutTag_MPEG_7_1_C)
	TAG(kAudioChannelLayoutTag_Emagic_Default_7_1)
	TAG(kAudioChannelLayoutTag_SMPTE_DTV)
	TAG(kAudioChannelLayoutTag_ITU_1_0)
	TAG(kAudioChannelLayoutTag_ITU_2_0)
	TAG(kAudioChannelLayoutTag_ITU_2_1)
	TAG(kAudioChannelLayoutTag_ITU_2_2)
	TAG(kAudioChannelLayoutTag_ITU_3_0)
	TAG(kAudioChannelLayoutTag_ITU_3_1)
	TAG(kAudioChannelLayoutTag_ITU_3_2)
	TAG(kAudioChannelLayoutTag_ITU_3_2_1)
	TAG(kAudioChannelLayoutTag_ITU_3_4_1)
	TAG(kAudioChannelLayoutTag_DVD_0)
	TAG(kAudioChannelLayoutTag_DVD_1)
	TAG(kAudioChannelLayoutTag_DVD_2)
	TAG(kAudioChannelLayoutTag_DVD_3)
	TAG(kAudioChannelLayoutTag_DVD_4)
	TAG(kAudioChannelLayoutTag_DVD_5)
	TAG(kAudioChannelLayoutTag_DVD_6)
	TAG(kAudioChannelLayoutTag_DVD_7)
	TAG(kAudioChannelLayoutTag_DVD_8)
	TAG(kAudioChannelLayoutTag_DVD_9)
	TAG(kAudioChannelLayoutTag_DVD_10)
	TAG(kAudioChannelLayoutTag_DVD_11)
	TAG(kAudioChannelLayoutTag_DVD_12)
	TAG(kAudioChannelLayoutTag_DVD_13)
	TAG(kAudioChannelLayoutTag_DVD_14)
	TAG(kAudioChannelLayoutTag_DVD_15)
	TAG(kAudioChannelLayoutTag_DVD_16)
	TAG(kAudioChannelLayoutTag_DVD_17)
	TAG(kAudioChannelLayoutTag_DVD_18)
	TAG(kAudioChannelLayoutTag_DVD_19)
	TAG(kAudioChannelLayoutTag_DVD_20)
	TAG(kAudioChannelLayoutTag_AudioUnit_4)
	TAG(kAudioChannelLayoutTag_AudioUnit_5)
	TAG(kAudioChannelLayoutTag_AudioUnit_6)
	TAG(kAudioChannelLayoutTag_AudioUnit_8)
	TAG(kAudioChannelLayoutTag_AudioUnit_5_0)
	TAG(kAudioChannelLayoutTag_AudioUnit_6_0)
	TAG(kAudioChannelLayoutTag_AudioUnit_7_0)
	TAG(kAudioChannelLayoutTag_AudioUnit_5_1)
	TAG(kAudioChannelLayoutTag_AudioUnit_6_1)
	TAG(kAudioChannelLayoutTag_AudioUnit_7_1)
	TAG(kAudioChannelLayoutTag_AAC_Quadraphonic)
	TAG(kAudioChannelLayoutTag_AAC_4_0)
	TAG(kAudioChannelLayoutTag_AAC_5_0)
	TAG(kAudioChannelLayoutTag_AAC_5_1)
	TAG(kAudioChannelLayoutTag_AAC_6_0)	
	TAG(kAudioChannelLayoutTag_AAC_6_1)
	TAG(kAudioChannelLayoutTag_AAC_7_0)	
	TAG(kAudioChannelLayoutTag_AAC_7_1)
	TAG(kAudioChannelLayoutTag_AAC_Octagonal)

	TAG(kAudioChannelLayoutTag_TMH_10_2_std)  
	TAG(kAudioChannelLayoutTag_TMH_10_2_full)
	{ 0, NULL }	// sentinel
};

static const char *prefix = "kAudioChannelLayoutTag_";

UInt32		CAChannelLayouts::StringToConstant(const char *s)
{
	int prefixlen = strlen(prefix);
	for (LayoutTag *t = gLayoutTags; t->name != NULL; ++t) {
		if (!strcmp(t->name, s) || !strcmp(t->name + prefixlen, s))
			return t->constant;
	}
	return kInvalidTag;
}

const char *	CAChannelLayouts::ConstantToString(UInt32 layoutTag)
{
	static char buffer[256];
	for (LayoutTag *t = gLayoutTags; t->name != NULL; ++t) {
		if (layoutTag == t->constant)
			return t->name + strlen(prefix);
	}
	sprintf(buffer, "unknown (0x%lx)", layoutTag);
	return buffer;
}


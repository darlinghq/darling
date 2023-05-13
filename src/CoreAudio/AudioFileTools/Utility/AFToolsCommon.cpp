/*	Copyright: 	� Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under Apple�s
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
	AFToolsCommon.cpp
	
=============================================================================*/

#include "AFToolsCommon.h"
#include "CAAudioFileFormats.h"
#include <ctype.h>
#include <CarbonCore/Endian.h>


/*
struct AudioStreamBasicDescription
{
	Float64	mSampleRate;		//	the native sample rate of the audio stream
	UInt32	mFormatID;			//	the specific encoding type of audio stream
	UInt32	mFormatFlags;		//	flags specific to each format
	UInt32	mBytesPerPacket;	//	the number of bytes in a packet
	UInt32	mFramesPerPacket;	//	the number of frames in each packet
	UInt32	mBytesPerFrame;		//	the number of bytes in a frame
	UInt32	mChannelsPerFrame;	//	the number of channels in each frame
	UInt32	mBitsPerChannel;	//	the number of bits in each channel
	UInt32	mReserved;			//	reserved, pads the structure out to force 8 byte alignment
};
*/
bool ParseStreamDescription(const char *inTextDesc, CAStreamBasicDescription &fmt)
{
	const char *p = inTextDesc;
	int bitdepth = 0;
	CAAudioFileFormats *theFileFormats = CAAudioFileFormats::Instance();
	
	memset(&fmt, 0, sizeof(fmt));
	OSType formatID;
	int x;
	if (strchr("-@/#", p[3])) {
		// special-case for 3-char format ID's ending with a space
		char fmtstr[4];
		memcpy(fmtstr, p, 3);
		fmtstr[3] = ' ';
		formatID = EndianU32_BtoN(*(UInt32 *)fmtstr);
		x = 3;
	} else {
		x = StrToOSType(p, formatID);
	}
	if (theFileFormats->IsKnownDataFormat(formatID)) {
		p += x;
		fmt.mFormatID = formatID;
		if (*p == '-') {
			++p;
			bitdepth = 0;
			while (isdigit(*p))
				bitdepth = 10 * bitdepth + *p++ - '0';
			fmt.mBitsPerChannel = bitdepth;
		}
	}
	
	if (fmt.mFormatID == 0) {
		// unknown format, assume LPCM
		if (p[0] == '-')	// previously we required a leading dash on PCM formats
			// pcm
			++p;
		fmt.mFormatID = kAudioFormatLinearPCM;
		fmt.mFormatFlags = kAudioFormatFlagIsPacked;
		fmt.mFramesPerPacket = 1;
		fmt.mChannelsPerFrame = 1;
		bool isUnsigned = false;
	
		if (p[0] == 'B' && p[1] == 'E') {
			fmt.mFormatFlags |= kLinearPCMFormatFlagIsBigEndian;
			p += 2;
		} else if (p[0] == 'L' && p[1] == 'E') {
			p += 2;
		} else {
			// default is native-endian
#if TARGET_RT_BIG_ENDIAN
			fmt.mFormatFlags |= kLinearPCMFormatFlagIsBigEndian;
#endif
		}
		if (p[0] == 'F') {
			fmt.mFormatFlags |= kAudioFormatFlagIsFloat;
			++p;
		} else {
			if (p[0] == 'U') {
				isUnsigned = true;
				++p;
			}
			if (p[0] == 'I')
				++p;
			else {
				fprintf(stderr, "The format '%s' is unknown or an unparseable PCM format specifier\n", inTextDesc);
				goto Bail;
			}
		}
		
		while (isdigit(*p))
			bitdepth = 10 * bitdepth + *p++ - '0';
		if (fmt.mFormatFlags & kAudioFormatFlagIsFloat) {
			if (bitdepth != 32 && bitdepth != 64) {
				fprintf(stderr, "Valid float bitdepths are 32 and 64\n");
				goto Bail;
			}
		} else {
			if (bitdepth != 8 && bitdepth != 16 && bitdepth != 24 && bitdepth != 32) {
				fprintf(stderr, "Valid integer bitdepths are 8, 16, 24, and 32\n");
				goto Bail;
			}
			if (!isUnsigned)
				fmt.mFormatFlags |= kAudioFormatFlagIsSignedInteger;
		}
		fmt.mBitsPerChannel = bitdepth;
		fmt.mBytesPerPacket = fmt.mBytesPerFrame = bitdepth / 8;
	}
	if (*p == '@') {
		++p;
		while (isdigit(*p))
			fmt.mSampleRate = 10 * fmt.mSampleRate + (*p++ - '0');
	}
	if (*p == '/') {
		UInt32 flags = 0;
		while (true) {
			char c = *++p;
			if (c >= '0' && c <= '9')
				flags = (flags << 4) | (c - '0');
			else if (c >= 'A' && c <= 'F')
				flags = (flags << 4) | (c - 'A' + 10);
			else if (c >= 'a' && c <= 'f')
				flags = (flags << 4) | (c - 'a' + 10);
			else break;
		}
		fmt.mFormatFlags = flags;
	}
	if (*p == '#') {
		++p;
		while (isdigit(*p))
			fmt.mFramesPerPacket = 10 * fmt.mFramesPerPacket + (*p++ - '0');
	}
	if (*p != '\0')
		goto Bail;
	return true;

Bail:
	fprintf(stderr, "Invalid format string: %s\n", inTextDesc);
	fprintf(stderr, "Syntax of format strings is: [-][BE|LE]{F|I|UI}{8|16|24|32|64}[@sample_rate_hz][/format_flags]\n");
	return false;
}

void	PrintAudioFileTypesAndFormats(FILE *outfile)
{
	CAAudioFileFormats *theFileFormats = CAAudioFileFormats::Instance();

	for (int i = 0; i < theFileFormats->mNumFileFormats; ++i) {
		CAAudioFileFormats::FileFormatInfo *ffi = &theFileFormats->mFileFormats[i];
		
		int j;
		char buf[20];
		char fmtName[256] = { 0 };
		if (ffi->mFileTypeName)
			CFStringGetCString(ffi->mFileTypeName, fmtName, sizeof(fmtName), kCFStringEncodingUTF8);
		fprintf(outfile, "        '%s' = %s", OSTypeToStr(buf, sizeof(buf), ffi->mFileTypeID), fmtName);
		
		int next = ffi->NumberOfExtensions();
		if (next > 0) {
			fprintf(outfile, " (");
			for (j = 0; j < next; ++j) {
				if (j > 0) fprintf(outfile, ", ");
				fprintf(outfile, ".%s", ffi->GetExtension(j, buf, sizeof(buf)));
			}
			fprintf(outfile, ")");
		}
		
		fprintf(outfile, "\n                   data_formats: ");
		int count = 0;
		for (j = 0; j < ffi->mNumDataFormats; ++j) {
			CAAudioFileFormats::DataFormatInfo *dfi = &ffi->mDataFormats[j];
			if (dfi->mFormatID == kAudioFormatLinearPCM) {
				for (int k = 0; k < dfi->mNumVariants; ++k) {
					if (++count == 6) {
						fprintf(outfile, "\n                                 ");
						count = 0;
					}
					AudioStreamBasicDescription *asbd = &dfi->mVariants[k];
					if (asbd->mFormatFlags & ~(kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsFloat))
						fprintf(outfile, "(%08lx/%ld) ", asbd->mFormatFlags, asbd->mBitsPerChannel);
					else {
						fprintf(outfile, "%s",
							(asbd->mFormatFlags & kAudioFormatFlagIsBigEndian) ? "BE" : "LE");
						if (asbd->mFormatFlags & kAudioFormatFlagIsFloat)
							fprintf(outfile, "F");
						else
							fprintf(outfile, "%sI",
								(asbd->mFormatFlags & kAudioFormatFlagIsSignedInteger) ? "" : "U");
						fprintf(outfile, "%ld ", asbd->mBitsPerChannel);
					}
				}
			} else {
				if (++count == 6) {
					fprintf(outfile, "\n                                 ");
					count = 0;
				}
				fprintf(outfile, "'%s' ", OSTypeToStr(buf, sizeof(buf), dfi->mFormatID));
			}
		}
		fprintf(outfile, "\n");
	}
}

#if 0
char *OSTypeToStr(char *buf, OSType t)
{
	char *p = buf;
	char str[4], *q = str;
	*(UInt32 *)str = EndianU32_NtoB(t);
	for (int i = 0; i < 4; ++i) {
		if (isprint(*q) && *q != '\\')
			*p++ = *q++;
		else {
			sprintf(p, "\\x%02x", *q++);
			p += 4;
		}
	}
	*p = '\0';
	return buf;
}

int		StrToOSType(const char *str, OSType &t)
{
	char buf[4];
	const char *p = str;
	int x;
	for (int i = 0; i < 4; ++i) {
		if (*p != '\\') {
			if ((buf[i] = *p++) == '\0')
				goto fail;
		} else {
			if (*++p != 'x') goto fail;
			if (sscanf(++p, "%02X", &x) != 1) goto fail;
			buf[i] = x;
			p += 2;
		}
	}
	t = EndianU32_BtoN(*(UInt32 *)buf);
	return p - str;
fail:
	return 0;
}
#endif

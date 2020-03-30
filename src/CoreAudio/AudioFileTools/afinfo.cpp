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
	afinfo.cpp
	
=============================================================================*/

#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <AudioToolbox/AudioToolbox.h>
#else
	#include <AudioToolbox.h>
#endif

#if TARGET_OS_WIN32
	#include <QTML.h>
#endif

#include <stdio.h>
#include "CAStreamBasicDescription.h"
#include <stdlib.h>
#include <CoreServices/Endian.h>

//using namespace std;

int main (int argc, const char * argv[]) 
{
#if TARGET_OS_WIN32
	InitializeQTML(0L);
#endif
	int enc = kCFStringEncodingUTF8;
	
	for (int i=1; i<argc; ++i)
	{
		OSStatus err;
		CAStreamBasicDescription asbd;
		AudioChannelLayout *acl = 0;
		UInt32 propertySize;
		UInt32 specifierSize;
		AudioFileID afid;
		Boolean res;
		FSRef fsRef;
		AudioFileTypeID filetype;
		CFStringRef filename = CFStringCreateWithCString(NULL, argv[i], enc);
		if (!filename) continue;
		
		CFURLRef url = CFURLCreateWithFileSystemPath(NULL, filename, kCFURLPOSIXPathStyle, true);
		if (!url) {
			fprintf(stderr, "Can't create CFURL for '%s'\n", argv[i]);
			goto Bail3;
		}
		
		res = CFURLGetFSRef(url, &fsRef);
		if (!res) {
			fprintf(stderr, "Can't get FSRef for '%s'\n", argv[i]);
			goto Bail2;
		}

		err = AudioFileOpen(&fsRef, fsRdPerm, 0, &afid);
		if (err) {
			fprintf(stderr, "AudioFileOpen failed for '%s'\n", argv[i]);
			goto Bail2;
		}
		
		propertySize = sizeof(AudioFileTypeID);
		err = AudioFileGetProperty(afid, kAudioFilePropertyFileFormat, &propertySize, &filetype);
		if (err) {
			fprintf(stderr, "AudioFileGetProperty kAudioFilePropertyFileFormat failed for '%s'\n", argv[i]);
			goto Bail1;
		}
		filetype = EndianU32_NtoB(filetype);	// for display purposes
		
		propertySize = sizeof(asbd);
		err = AudioFileGetProperty(afid, kAudioFilePropertyDataFormat, &propertySize, &asbd);
		if (err) {
			fprintf(stderr, "AudioFileGetProperty kAudioFilePropertyDataFormat failed for '%s'\n", argv[i]);
			goto Bail1;
		}

		printf("File:           %s\n", argv[i]);
		printf("File type ID:   %-4.4s\n", (char *)&filetype);
		asbd.PrintFormat(stdout, "", "Data format:   ");

		UInt32 writable;
		err = AudioFileGetPropertyInfo(afid, kAudioFilePropertyChannelLayout, &propertySize, &writable);
		if (err) {
			//fprintf(stderr, "AudioFileGetPropertyInfo kAudioFilePropertyChannelLayout failed for '%s'\n", argv[i]);
			printf("   no channel layout.\n");
		} else {
			
			acl = (AudioChannelLayout*)calloc(1, propertySize);
			err = AudioFileGetProperty(afid, kAudioFilePropertyChannelLayout, &propertySize, acl);
			if (err) {
				fprintf(stderr, "AudioFileGetProperty kAudioFilePropertyChannelLayout failed for '%s'\n", argv[i]);
				goto Bail1;
			}
			
			CFStringRef aclname;
			char aclstr[512];
			specifierSize = propertySize;
			propertySize = sizeof(aclname);
			AudioFormatGetProperty(kAudioFormatProperty_ChannelLayoutName, specifierSize, acl, &propertySize, &aclname);
			if (err) {
				fprintf(stderr, "AudioFileGetProperty kAudioFilePropertyChannelLayout failed for '%s'\n", argv[i]);
				goto Bail1;
			}
			CFStringGetCString(aclname, aclstr, 512, kCFStringEncodingUTF8);
			
			printf("Channel layout: %s\n", aclstr);
		}
		
		UInt64 dataByteCount;
		propertySize = sizeof(dataByteCount);
		err = AudioFileGetProperty(afid, kAudioFilePropertyAudioDataByteCount, &propertySize, &dataByteCount);
		if (err) {
			fprintf(stderr, "AudioFileGetProperty kAudioFilePropertyAudioDataByteCount failed for '%s'\n", argv[i]);
		} else {
			printf("audio bytes: %llu\n", dataByteCount);
		}
		
		UInt64 dataPacketCount;
		UInt64 totalFrames;
		totalFrames = 0;
		propertySize = sizeof(dataPacketCount);
		err = AudioFileGetProperty(afid, kAudioFilePropertyAudioDataPacketCount, &propertySize, &dataPacketCount);
		if (err) {
			fprintf(stderr, "AudioFileGetProperty kAudioFilePropertyAudioDataPacketCount failed for '%s'\n", argv[i]);
		} else {
			printf("audio packets: %llu\n", dataPacketCount);
			if (asbd.mFramesPerPacket)
				totalFrames = asbd.mFramesPerPacket * dataPacketCount;
		}
		
		AudioFilePacketTableInfo pti;
		propertySize = sizeof(pti);
		err = AudioFileGetProperty(afid, kAudioFilePropertyPacketTableInfo, &propertySize, &pti);
		if (err == noErr) {
			totalFrames = pti.mNumberValidFrames;
			printf("audio %qd valid frames + %ld priming + %ld remainder = %qd\n", pti.mNumberValidFrames, pti.mPrimingFrames, pti.mRemainderFrames, pti.mNumberValidFrames + pti.mPrimingFrames + pti.mRemainderFrames);
		}
		
		if (totalFrames != 0)
			printf("duration: %.4f seconds\n", (double)totalFrames / (double)asbd.mSampleRate );

		UInt32 maxPacketSize;
		propertySize = sizeof(maxPacketSize);
		err = AudioFileGetProperty(afid, kAudioFilePropertyMaximumPacketSize, &propertySize, &maxPacketSize);
		if (err) {
			fprintf(stderr, "AudioFileGetProperty kAudioFilePropertyMaximumPacketSize failed for '%s'\n", argv[i]);
		} else {
			printf("maximum packet size: %lu\n", maxPacketSize);
		}
		
		UInt64 dataOffset;
		propertySize = sizeof(dataOffset);
		err = AudioFileGetProperty(afid, kAudioFilePropertyDataOffset, &propertySize, &dataOffset);
		if (err) {
			fprintf(stderr, "AudioFileGetProperty kAudioFilePropertyDataOffset failed for '%s'\n", argv[i]);
		} else {
			printf("audio data file offset: %llu\n", dataOffset);
		}
		
		UInt32 isOptimized;
		propertySize = sizeof(isOptimized);
		err = AudioFileGetProperty(afid, kAudioFilePropertyIsOptimized, &propertySize, &isOptimized);
		if (err) {
			fprintf(stderr, "AudioFileGetProperty kAudioFilePropertyIsOptimized failed for '%s'\n", argv[i]);
		} else {
			printf(isOptimized ? "optimized\n" : "not optimized\n");
		}
								
		Bail1:
		AudioFileClose(afid);
		Bail2:
		CFRelease(url);
		Bail3:
		CFRelease(filename);
		free(acl);
		
		printf("----\n");
	}
    return 0;
}

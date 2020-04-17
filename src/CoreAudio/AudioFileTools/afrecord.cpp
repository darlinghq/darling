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
	afrecord.cpp
	
=============================================================================*/

#include "CAAudioFileRecorder.h"
#include "CAXException.h"
#include <unistd.h>
#include "AFToolsCommon.h"
#include "CAFilePathUtils.h"
#include "CAAudioFileFormats.h"

static void usage()
{
	fprintf(stderr,
			"Usage:\n"
			"%s [option...] audio_file\n\n"
			"Options: (may appear before or after arguments)\n"
			"    { -f | --file } file_format:\n"
			, getprogname());
	PrintAudioFileTypesAndFormats(stderr);
	fprintf(stderr,
			"    { -d | --data } data_format[@sample_rate_hz]:\n"
			"        [-][BE|LE]{F|[U]I}{8|16|24|32|64}          (PCM)\n"
			"            e.g. -BEI16 -F32@44100\n"
			"        or a data format appropriate to file format, as above\n"
			"    { -c | --channels } number_of_channels\n"
			"        add/remove channels without regard to order\n"
			"    { -l | --channellayout } layout_tag\n"
			"        layout_tag: name of a constant from CoreAudioTypes.h\n"
			"          (prefix \"kAudioChannelLayoutTag_\" may be omitted)\n"
			"        if specified once, applies to output file; if twice, the first\n"
			"          applies to the input file, the second to the output file\n"
			"    { -b | --bitrate } bit_rate_bps\n"
			"         e.g. 128000\n"
			"    { -q | --quality } quality\n"
			"        quality: 0-127\n"
			"    { -v | --verbose }\n"
			"        print progress verbosely\n"
			"    { -p | --profile }\n"
			"        collect and print performance profile\n"
			);
	exit(1);
}

static void	MissingArgument()
{
	fprintf(stderr, "missing argument\n\n");
	usage();
}

static OSType Parse4CharCode(const char *arg, const char *name)
{
	OSType t;
	StrToOSType(arg, t);
	if (t == 0) {
		fprintf(stderr, "invalid 4-char-code argument for %s: '%s'\n\n", name, arg);
		usage();
	}
	return t;
}

static int	ParseInt(const char *arg, const char *name)
{
	int x;
	if (sscanf(arg, "%d", &x) != 1) {
		fprintf(stderr, "invalid integer argument for %s: '%s'\n\n", name, arg);
		usage();
	}
	return x;
}

static void Record(CAAudioFileRecorder &recorder)
{
	recorder.Start();
	printf("Recording, press any key to stop:");
	fflush(stdout);
	getchar();
	//sleep(10);
	recorder.Stop();
}

int main(int argc, const char *argv[])
{
	const char *recordFileName = NULL;

	// set up defaults
	AudioFileTypeID filetype = kAudioFileAIFFType;
	
	bool gotOutDataFormat = false;
	CAStreamBasicDescription dataFormat;
	dataFormat.mSampleRate = 44100.;	// later get this from the hardware
	dataFormat.mFormatID = kAudioFormatLinearPCM;
	dataFormat.mFormatFlags = kAudioFormatFlagIsBigEndian | kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
	dataFormat.mFramesPerPacket = 1;
	dataFormat.mChannelsPerFrame = 2;
	dataFormat.mBitsPerChannel = 16;
	dataFormat.mBytesPerPacket = dataFormat.mBytesPerFrame = 4;
	
	SInt32 bitrate = -1, quality = -1;
	
	// parse arguments
	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (arg[0] != '-') {
			if (recordFileName != NULL) {
				fprintf(stderr, "may only specify one record file\n");
				usage();
			}
			recordFileName = arg;
		} else {
			arg += 1;
			if (arg[0] == 'f' || !strcmp(arg, "-file")) {
				if (++i == argc) MissingArgument();
				filetype = Parse4CharCode(argv[i], "-f | --file");
			} else if (arg[0] == 'd' || !strcmp(arg, "-data")) {
				if (++i == argc) MissingArgument();
				if (!ParseStreamDescription(argv[i], dataFormat))
					usage();
				gotOutDataFormat = true;
			} else if (arg[0] == 'b' || !strcmp(arg, "-bitrate")) {
				if (++i == argc) MissingArgument();
				bitrate = ParseInt(argv[i], "-b | --bitrate");
			} else if (arg[0] == 'q' || !strcmp(arg, "-quality")) {
				if (++i == argc) MissingArgument();
				quality = ParseInt(argv[i], "-q | --quality");
			} else {
				fprintf(stderr, "unknown argument: %s\n\n", arg - 1);
				usage();
			}
		}
	}
	
	if (recordFileName == NULL)
		usage();
	
	if (!gotOutDataFormat) {
		if (filetype == 0) {
			fprintf(stderr, "no output file or data format specified\n\n");
			usage();
		}
		if (!CAAudioFileFormats::Instance()->InferDataFormatFromFileFormat(filetype, dataFormat)) {
			fprintf(stderr, "Couldn't infer data format from file format\n\n");
			usage();
		}
	} else if (filetype == 0) {
		if (!CAAudioFileFormats::Instance()->InferFileFormatFromDataFormat(dataFormat, filetype)) {
			dataFormat.PrintFormat(stderr, "", "Couldn't infer file format from data format");
			usage();
		}
	}

	unlink(recordFileName);
	
	if (dataFormat.IsPCM())
		dataFormat.ChangeNumberChannels(2, true);
	else
		dataFormat.mChannelsPerFrame = 2;
	
	try {
		const int kNumberBuffers = 3;
		const unsigned kBufferSize = 0x8000;
		CAAudioFileRecorder recorder(kNumberBuffers, kBufferSize);
		FSRef parentDir;
		CFStringRef filename;
		XThrowIfError(PosixPathToParentFSRefAndName(recordFileName, parentDir, filename), "couldn't find output directory");
		recorder.SetFile(parentDir, filename, filetype, dataFormat, NULL);
		
		CAAudioFile &recfile = recorder.GetFile();
		if (bitrate >= 0)
			recfile.SetConverterProperty(kAudioConverterEncodeBitRate, sizeof(UInt32), &bitrate);
		if (quality >= 0)
			recfile.SetConverterProperty(kAudioConverterCodecQuality, sizeof(UInt32), &quality);

		Record(recorder);
	}
	catch (CAXException &e) {
		char buf[256];
		fprintf(stderr, "Error: %s (%s)\n", e.mOperation, CAXException::FormatError(buf, sizeof(buf), e.mError));
		return 1;
	}
	catch (...) {
		fprintf(stderr, "An unknown error occurred\n");
		return 1;
	}
	return 0;
}


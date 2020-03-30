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
	afconvert.cpp
	
=============================================================================*/

/*
	afconvert
	
	command-line tool to convert an audio file to another format
*/

#include "afconvert.h"
#include <stdio.h>
#include <vector>
#if !defined(__COREAUDIO_USE_FLAT_INCLUDES__)
	#include <CoreServices/CoreServices.h>
	#include <AudioToolbox/AudioToolbox.h>
#else
	#include <CoreServices.h>
	#include <AudioToolbox.h>
#endif
#include "CAChannelLayouts.h"

#include "CAAudioFileConverter.h"
#include "CAXException.h"
#include "CAAudioFileFormats.h"
#include "AFToolsCommon.h"
#if !TARGET_OS_MAC
	#include <QTML.h>
	#include "CAWindows.h"
#endif


void AFConvertTool::usage()
{
	fprintf(stderr,
			"Usage:\n"
			"%s [option...] input_file [output_file]\n\n"
			"Options: (may appear before or after arguments)\n"
			"    { -f | --file } file_format:\n",
#if !TARGET_OS_WIN32
			getprogname());
#else
			"afconvert");
#endif
	PrintAudioFileTypesAndFormats(stderr);
	fprintf(stderr,
			"    { -d | --data } data_format[@sample_rate_hz][/format_flags][#frames_per_packet] :\n"
			"        [-][BE|LE]{F|[U]I}{8|16|24|32|64}          (PCM)\n"
			"            e.g.   BEI16   F32@44100\n"
			"        or a data format appropriate to file format, as above\n"
			"        format_flags: hex digits, e.g. '80'\n"
			"        bitdepth on non-PCM formats can be specified, e.g.: alac-24\n"
			"        Frames per packet can be specified for some encoders, e.g.: samr#12\n"
			"    { -c | --channels } number_of_channels\n"
			"        add/remove channels without regard to order\n"
			"    { -l | --channellayout } layout_tag\n"
			"        layout_tag: name of a constant from CoreAudioTypes.h\n"
			"          (prefix \"kAudioChannelLayoutTag_\" may be omitted)\n"
			"        if specified once, applies to output file; if twice, the first\n"
			"          applies to the input file, the second to the output file\n"
			"    { -b | --bitrate } bit_rate_bps\n"
			"         e.g. 128000\n"
			"    { -q | --quality } codec_quality\n"
			"        codec_quality: 0-127\n"
			"    { -r | --src-quality } src_quality\n"
			"        src_quality (sample rate converter quality): 0-127\n"
			"    { -v | --verbose }\n"
			"        print progress verbosely\n"
			"    { -s | --strategy } strategy\n"
			"        bitrate strategy for encoded file\n"
			"        0 for CBR, 1 for ABR, 2 for VBR\n"
			"    { -t | --tag }\n"
			"        If encoding to CAF, store the source file's format and name in a user chunk.\n"
			"        If decoding from CAF, use the destination format and filename found in a user chunk.\n"
			"    --prime-method method\n"
			"        decode priming method (see AudioConverter.h)\n"
			);
	ExtraUsageOptions();
	exit(1);
}

void	Warning(const char *s, OSStatus error)
{
	char buf[256];
	fprintf(stderr, "*** warning: %s (%s)\n", s, CAXException::FormatError(buf, sizeof(buf), error));
}

OSType AFConvertTool::Parse4CharCode(const char *arg, const char *name)
{
	OSType t;
	StrToOSType(arg, t);
	if (t == 0) {
		fprintf(stderr, "invalid 4-char-code argument for %s: '%s'\n\n", name, arg);
		usage();
	}
	return t;
}


int		AFConvertTool::main(int argc, const char * argv[])
{
	Init();
	
	CAAudioFileConverter::ConversionParameters &params = *mParams;
	bool gotOutDataFormat = false;
	
	CAXException::SetWarningHandler(Warning);
	
	if (argc < 2)
		usage();
	
	params.flags = CAAudioFileConverter::kOpt_OverwriteOutputFile;
	
	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (arg[0] != '-') {
			if (params.input.filePath == NULL) params.input.filePath = arg;
			else if (params.output.filePath == NULL) params.output.filePath = arg;
			else usage();
		} else {
			arg += 1;
			if (arg[0] == 'f' || !strcmp(arg, "-file")) {
				if (++i == argc) MissingArgument();
				params.output.fileType = Parse4CharCode(argv[i], "-f | --file");
			} else if (arg[0] == 'd' || !strcmp(arg, "-data")) {
				if (++i == argc) MissingArgument();
				if (!ParseStreamDescription(argv[i], params.output.dataFormat))
					usage();
				gotOutDataFormat = true;
			} else if (arg[0] == 'b' || !strcmp(arg, "-bitrate")) {
				if (++i == argc) MissingArgument();
				params.output.bitRate = ParseInt(argv[i], "-b | --bitrate");
			} else if (arg[0] == 'q' || !strcmp(arg, "-quality")) {
				if (++i == argc) MissingArgument();
				params.output.codecQuality = ParseInt(argv[i], "-q | --quality");
			} else if (arg[0] == 'r' || !strcmp(arg, "-src-quality")) {
				if (++i == argc) MissingArgument();
				params.output.srcQuality = ParseInt(argv[i], "-r | --src-quality");
			} else if (arg[0] == 'l' || !strcmp(arg, "-channellayout")) {
				if (++i == argc) MissingArgument();
				UInt32 tag = CAChannelLayouts::StringToConstant(argv[i]);
				if (tag == CAChannelLayouts::kInvalidTag) {
					fprintf(stderr, "unknown channel layout tag: %s\n\n", argv[i]);
					usage();
				}
				if (params.output.channelLayoutTag == 0)
					params.output.channelLayoutTag = tag;
				else if (params.input.channelLayoutTag == 0) {
					params.input.channelLayoutTag = params.output.channelLayoutTag;
					params.output.channelLayoutTag = tag;
				} else {
					fprintf(stderr, "too many channel layout tags!\n\n");
					usage();
				}
			} else if (arg[0] == 'c' || !strcmp(arg, "-channels")) {
				if (++i == argc) MissingArgument();
				params.output.channels = ParseInt(argv[i], "-c | --channels");
			} else if (arg[0] == 'v' || !strcmp(arg, "-verbose")) {
				params.flags |= CAAudioFileConverter::kOpt_Verbose;
			} else if (arg[0] == 's' || !strcmp(arg, "-strategy")) {
				if (++i == argc) MissingArgument();
				params.output.strategy = ParseInt(argv[i], "-s | --strategy");
			} else if (arg[0] == 't' || !strcmp(arg, "-tag")) {
				params.flags |= CAAudioFileConverter::kOpt_CAFTag;
			} else if (!strcmp(arg, "-prime-method")) {
				if (++i == argc) MissingArgument();
				params.output.primeMethod = ParseInt(argv[i], "-p | --prime-method");
			} else if (!ParseOtherOption(argv, i)) {
				fprintf(stderr, "unknown argument: %s\n\n", arg - 1);
				usage();
			}
		}
	}
	if (params.input.filePath == NULL) {
		fprintf(stderr, "no input file specified\n\n");
		usage();
	}
	
	if (!(params.flags & CAAudioFileConverter::kOpt_CAFTag)) {
		if (!gotOutDataFormat) {
			if (params.output.fileType == 0) {
				fprintf(stderr, "no output file or data format specified\n\n");
				usage();
			}
			if (!CAAudioFileFormats::Instance()->InferDataFormatFromFileFormat(params.output.fileType, params.output.dataFormat)) {
				fprintf(stderr, "Couldn't infer data format from file format\n\n");
				usage();
			}
		} else if (params.output.fileType == 0) {
			CAAudioFileFormats *formats = CAAudioFileFormats::Instance();
			if (!formats->InferFileFormatFromFilename(params.output.filePath, params.output.fileType) && !formats->InferFileFormatFromDataFormat(params.output.dataFormat, params.output.fileType)) {
				params.output.dataFormat.PrintFormat(stderr, "", "Couldn't infer file format from data format");
				usage();
			}
		}
	}

	try {
		mAFConverter->ConvertFile(params);
		Success();
	}
	catch (CAXException &e) {
		char buf[256];
		fprintf(stderr, "Error: %s (%s)\n", e.mOperation, e.FormatError(buf, sizeof(buf)));
		return 1;
	}
	catch (...) {
		fprintf(stderr, "An unknown error occurred\n");
		return 1;
	}

	return 0;
}

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
	Interleaver.cpp
	
=============================================================================*/

#include "CAAudioFile.h"
#include "CAXException.h"
#include <unistd.h>
#include "CAChannelLayouts.h"
#include "CAFilePathUtils.h"

static void usage()
{
	fprintf(stderr,
			"Usage:\n"
			"%s [option...] input_file... -o output_file\n\n"
			"Options: (may appear before or after arguments)\n"
			"    { -l | --channellayout } layout_tag\n"
			"        layout_tag: name of a constant from CoreAudioTypes.h\n"
			"          (prefix \"kAudioChannelLayoutTag_\" may be omitted)\n",
			getprogname()
			);
	exit(1);
}


void	Interleave(int nInputs, const char *infilenames[], const char *outfilename, const CAAudioChannelLayout *layout)
{
	const UInt32 kBufferSizeFrames = 0x8000;
	const UInt32 kBufferSizeBytes = kBufferSizeFrames * sizeof(Float32);
	class FileAndBuffer : public CAAudioFile {
	public:
		FileAndBuffer() : mBuf(NULL), mPtrs(NULL) { }
		~FileAndBuffer() { delete mBuf; delete mPtrs; }
		
		CABufferList *	mBuf;
		CABufferList *	mPtrs;
	};
	FileAndBuffer *infiles = new FileAndBuffer[nInputs], *file;
	FileAndBuffer outfile;
	int i;
	UInt32 outputChannels = 0;
	double sampleRate = 0.;
	UInt32 maxBitDepth = 0;
	CAStreamBasicDescription clientFormat;
	bool outFileCreated = false;
	
	try {
		// set up input files
		for (i = 0; i < nInputs; ++i) {
			file = &infiles[i];
			file->Open(infilenames[i]);
			const CAStreamBasicDescription &fmt = file->GetFileDataFormat();
			//fmt.PrintFormat(stdout, "", "input file");
			XThrowIf(fmt.mFormatID != kAudioFormatLinearPCM, -1, "input files must be PCM");
			outputChannels += fmt.mChannelsPerFrame;
			if (sampleRate == 0.)
				sampleRate = fmt.mSampleRate;
			else
				XThrowIf(fmt.mSampleRate != sampleRate, -1, "input files must have the same sample rate");
			if (fmt.mBitsPerChannel > maxBitDepth)
				maxBitDepth = fmt.mBitsPerChannel;
			clientFormat.mSampleRate = sampleRate;
			clientFormat.SetCanonical(fmt.mChannelsPerFrame, false);	// deinterleaved
			file->SetClientFormat(clientFormat, NULL);
			file->mBuf = CABufferList::New("readbuf", clientFormat);
			file->mBuf->AllocateBuffers(kBufferSizeBytes);
			file->mPtrs = CABufferList::New("readptrs", clientFormat);
			//clientFormat.PrintFormat(stdout, "", "input client");
		}
		
		if (layout != NULL) {
			if (AudioChannelLayoutTag_GetNumberOfChannels(layout->Tag()) != outputChannels) {
				fprintf(stderr, "Channel layout tag '%s' is inappropriate for %lu channels of audio -- aborting\n", 
					CAChannelLayouts::ConstantToString(layout->Tag()), outputChannels);
				exit(2);
			}
		}

		// prepare output file format
		CAStreamBasicDescription outfmt;
		outfmt.mSampleRate = sampleRate;
		outfmt.mFormatID = kAudioFormatLinearPCM;
		outfmt.mFormatFlags = kLinearPCMFormatFlagIsBigEndian | kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
		outfmt.mBitsPerChannel = maxBitDepth;
		outfmt.mChannelsPerFrame = outputChannels;
		outfmt.mBytesPerPacket = outfmt.mBytesPerFrame = outputChannels * (maxBitDepth >> 3);
		outfmt.mFramesPerPacket = 1;
		//outfmt.PrintFormat(stdout, "", "output file");
		
		unlink(outfilename);
		FSRef parentDir;
		CFStringRef outName;
		XThrowIfError(PosixPathToParentFSRefAndName(outfilename, parentDir, outName), "Couldn't locate output directory");
		outfile.CreateNew(parentDir, outName, kAudioFileAIFFType, outfmt, layout ? &layout->Layout() : NULL);
		outFileCreated = true;
		
		// create the output file and buffers
		clientFormat.mSampleRate = sampleRate;
		clientFormat.SetCanonical(outputChannels, false);
		outfile.SetClientFormat(clientFormat, NULL);
		//clientFormat.PrintFormat(stdout, "", "output client");

		outfile.mPtrs = CABufferList::New("writeptrs", clientFormat);

		AudioBufferList &writebufs = outfile.mPtrs->GetModifiableBufferList();
		int outbuf = 0;
		for (i = 0; i < nInputs; ++i) {
			file = &infiles[i];
			const CABufferList *bl = file->mBuf;
			const AudioBufferList &readbufs = bl->GetBufferList();
			memcpy(&writebufs.mBuffers[outbuf], &readbufs.mBuffers[0], 
				readbufs.mNumberBuffers * sizeof(AudioBuffer));
			outbuf += readbufs.mNumberBuffers;
		}
		
		while (true) {
			UInt32 maxFramesRead = 0;
			UInt32 nframes;
			for (i = 0; i < nInputs; ++i) {
				file = &infiles[i];
				file->mPtrs->SetFrom(file->mBuf);
				nframes = kBufferSizeFrames;
				AudioBufferList &readbufs = file->mPtrs->GetModifiableBufferList();
				file->Read(nframes, &readbufs);
				//CAShowAudioBufferList(&readbufs, 8, 0);
				if (nframes > maxFramesRead)
					maxFramesRead = nframes;
				if (nframes < kBufferSizeFrames)
					file->mPtrs->PadWithZeroes(kBufferSizeBytes);
			}
			if (maxFramesRead == 0)
				break;

			if (maxFramesRead < kBufferSizeFrames)
				outfile.mPtrs->SetNumBytes(maxFramesRead * sizeof(Float32));
			//CAShowAudioBufferList(&writebufs, 8, 0);
			outfile.Write(maxFramesRead, &writebufs);
			if (maxFramesRead < kBufferSizeFrames)
				break;
		}
	}
	catch (...) {
		if (outFileCreated)
			unlink(outfilename);
		delete[] infiles;
		throw;
	}
	outfile.Close();
	// input files are closed from destructors
	delete[] infiles;
}

static void	MissingArgument()
{
	fprintf(stderr, "missing argument\n\n");
	usage();
}

int	main(int argc, const char *argv[])
{
	int nins = 0;
	static const int MAX_INPUT_FILES = 32;
	const char *infiles[MAX_INPUT_FILES];
	const char *outfile = NULL;
	UInt32 layoutTag = 0;

	for (int i = 1; i < argc; ++i) {
		const char *arg = argv[i];
		if (arg[0] != '-') {
			if (nins >= MAX_INPUT_FILES) {
				fprintf(stderr, "too many input files\n\n");
				usage();
			}
			infiles[nins++] = arg;
		} else {
			arg += 1;
			if (arg[0] == 'o') {
				if (++i == argc) MissingArgument();
				outfile = argv[i];
			} else if (arg[0] == 'l' || !strcmp(arg, "-channellayout")) {
				if (++i == argc) MissingArgument();
				layoutTag = CAChannelLayouts::StringToConstant(argv[i]);
				if (layoutTag == CAChannelLayouts::kInvalidTag) {
					fprintf(stderr, "unknown channel layout tag: %s\n\n", argv[i]);
					usage();
				}
			} else {
				fprintf(stderr, "unknown argument: %s\n\n", arg - 1);
				usage();
			}
		}
	}
	
	if (nins < 2 || outfile == NULL)
		usage();
	
	try {
		if (layoutTag != 0) {
			CAAudioChannelLayout layout = CAAudioChannelLayout(layoutTag);
			Interleave(nins, infiles, outfile, &layout);
		} else
			Interleave(nins, infiles, outfile, NULL);
	}
	catch (CAXException &e) {
		char buf[256];
		fprintf(stderr, "Error: %s (%s)\n", e.mOperation, CAXException::FormatError(buf, e.mError));
		return 1;
	}
	catch (...) {
		fprintf(stderr, "An unknown error occurred\n");
		return 1;
	}
	return 0;
}

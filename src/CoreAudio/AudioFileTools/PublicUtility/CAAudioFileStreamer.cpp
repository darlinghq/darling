/*	Copyright: 	© Copyright 2005 Apple Computer, Inc. All rights reserved.

	Disclaimer:	IMPORTANT:  This Apple software is supplied to you by Apple Computer, Inc.
			("Apple") in consideration of your agreement to the following terms, and your
			use, installation, modification or redistribution of this Apple software
			constitutes acceptance of these terms.  If you do not agree with these terms,
			please do not use, install, modify or redistribute this Apple software.

			In consideration of your agreement to abide by the following terms, and subject
			to these terms, Apple grants you a personal, non-exclusive license, under AppleÕs
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
	CAAudioFileStreamer.cpp
	
=============================================================================*/

#include "CAAudioFileStreamer.h"

// ____________________________________________________________________________

void	CAAudioFileReader::FileReadBuffer::UpdateAfterRead(SInt64 curFrame, UInt32 nFrames)
{
	//printf("read %ld PCM packets, file packets %qd-%qd\n", nPackets, b->mStartPacket, b->mEndPacket);
	mEndFrame = nFrames;
	mEndOfStream = (nFrames == 0);
	mBufferStartFileFrame = curFrame;
}

// ____________________________________________________________________________

void	CAAudioFileReader::SetFile(const FSRef &inFile)
{
	Stop();
	CancelAndDisposeBuffers();
	
	delete mFile;   mFile = NULL;
	mFile = new CAAudioFile;
	mFile->Open(inFile);
	
	const CAStreamBasicDescription &fileFmt = mFile->GetFileDataFormat();
	CAStreamBasicDescription iofmt;
	iofmt.SetCanonical(fileFmt.mChannelsPerFrame, false);	// deinterleaved
	iofmt.mSampleRate = fileFmt.mSampleRate;
	mFile->SetClientFormat(iofmt, NULL);
	
	SetFormat(iofmt);
}

void	CAAudioFileReader::Start()
{
	mErrorCount = 0;
	mEndOfStream = false;
	Prime();
	mRunning = true;
}

void	CAAudioFileReader::Stop()
{
	mRunning = false;
}

void	CAAudioFileReader::ReadBuffer(FileReadBuffer *b)
{
	b->SetEmpty();
	CABufferList *ioMemory = b->GetBufferList();
	CABufferList *fileBuffers = GetBufferList();
	fileBuffers->SetFrom(ioMemory);
	UInt32 nFrames = GetBufferSizeFrames();
	SInt64 curFrame = mFile->Tell();
	mFile->Read(nFrames, &fileBuffers->GetModifiableBufferList());
	b->UpdateAfterRead(curFrame, nFrames);
}

double		CAAudioFileReader::GetCurrentPosition() const
{
	return double(GetCurrentFrame()) / double(GetNumberFrames());
#if 0
	double nFrames = double(GetNumberFrames());	// +1 to account for leftovers from decoder
	if (!mRunning)
		return double(GetCurrentFrame()) / nFrames;

	if (mEndOfStream) {
		//printf("EOF\n");
		return 1.0;
	}

	const FileReadBuffer *b = static_cast<const FileReadBuffer *>(GetCurrentBuffer());
		// the buffer from which we're reading
	UInt32 startFrame, endFrame;
	b->GetLocation(startFrame, endFrame);
	//printf("%qd + %ld / %.f\n", b->mBufferStartFileFrame, startFrame, nFrames);
	return double(b->mBufferStartFileFrame + startFrame) / nFrames;
	//if (endFrame > 0) {
		//double frac = (double(startFrame) / double(endFrame)) * double(endPacket - startPacket);
		//packetIndex += frac;
		//printf("frames %ld-%ld, packets %qd-%qd, frac %.3f\n",
		//	startFrame, endFrame, startPacket, endPacket, frac);
	//}
	//double pos = packetIndex / nPacketsPlus1;
	//printf("%.3f / %.0f = %.3f\n", packetIndex, nPacketsPlus1, pos);
	//return pos;

	//return double(GetCurrentFrame()) / nFrames;
#endif
}

SInt64	CAAudioFileReader::GetCurrentFrame() const
{
	if (!mRunning)
		return mFile->Tell();
	if (mEndOfStream)
		return GetNumberFrames();
	const FileReadBuffer *b = static_cast<const FileReadBuffer *>(GetCurrentBuffer());
		// the buffer from which we're reading
	UInt32 startFrame, endFrame;
	b->GetLocation(startFrame, endFrame);
	//printf("%qd + %ld / %.f\n", b->mBufferStartFileFrame, startFrame, nFrames);
	return b->mBufferStartFileFrame + startFrame;
}

void	CAAudioFileReader::SetCurrentPosition(double loc)
{
	bool wasRunning = IsRunning();
	if (wasRunning)
		Stop();
	SInt64 frameNumber = SInt64(loc * GetFile().GetNumberFrames() + 0.5);
	try {
		GetFile().Seek(frameNumber);
	}
	catch (...) {
	
	}
	if (wasRunning)
		Start();
}

// ____________________________________________________________________________

void	CAAudioFileWriter::SetFile(AudioFileID fileID)
{
	Stop();
	CancelAndDisposeBuffers();
	
	delete mFile;   mFile = NULL;
	mFile = new CAAudioFile;
	mFile->Wrap(fileID, true);
	
	const CAStreamBasicDescription &fileFmt = mFile->GetFileDataFormat();
	CAStreamBasicDescription iofmt;
	iofmt.SetCanonical(fileFmt.mChannelsPerFrame, false);	// deinterleaved
	iofmt.mSampleRate = fileFmt.mSampleRate;
	mFile->SetClientFormat(iofmt, NULL);
	
	SetFormat(iofmt);
}

void	CAAudioFileWriter::SetFile(const FSRef &parentDir, CFStringRef filename, AudioFileTypeID filetype, const CAStreamBasicDescription &dataFormat, const CAAudioChannelLayout *layout)
{
	Stop();
	CancelAndDisposeBuffers();
	
	delete mFile;   mFile = NULL;
	mFile = new CAAudioFile;
	mFile->CreateNew(parentDir, filename, filetype, dataFormat, layout ? &layout->Layout() : NULL);
	
	const CAStreamBasicDescription &fileFmt = mFile->GetFileDataFormat();
	CAStreamBasicDescription iofmt;
	iofmt.SetCanonical(fileFmt.mChannelsPerFrame, false);	// deinterleaved
	iofmt.mSampleRate = fileFmt.mSampleRate;
	mFile->SetClientFormat(iofmt, NULL);
	
	SetFormat(iofmt);
}

void	CAAudioFileWriter::Start()
{
	mErrorCount = 0;
	mRunning = true;
}

void	CAAudioFileWriter::Stop()
{
	Flush();
	mRunning = false;
}

void	CAAudioFileWriter::WriteBuffer(CABufferQueue::Buffer *b)
{
	CABufferList *ioMemory = b->GetBufferList();
	CABufferList *fileBuffers = GetBufferList();
	UInt32 nFrames = b->FrameCount();
	fileBuffers->SetFrom(ioMemory, GetBytesPerFrame() * nFrames);
	mFile->Write(nFrames, &fileBuffers->GetModifiableBufferList());
	b->SetEmpty();
}

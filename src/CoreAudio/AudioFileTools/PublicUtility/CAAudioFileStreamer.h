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
	CAAudioFileStreamer.h
	
=============================================================================*/

#ifndef __CAAudioFileStreamer_h__
#define __CAAudioFileStreamer_h__

#include "CABufferQueue.h"
#include "CAAudioFile.h"

// ____________________________________________________________________________
// Base class for CAAudioFileReader and CAAudioFileWriter
class CAAudioFileStreamer {
public:
	CAAudioFileStreamer() : mRunning(false), mFile(NULL) { }
	virtual ~CAAudioFileStreamer() { delete mFile; }

	CAAudioFile &		GetFile() { return *mFile; }
	const CAAudioFile & GetFile() const { return *mFile; }

	bool				IsRunning() { return mRunning; }

protected:
	bool				mRunning;
	CAAudioFile *		mFile;
};

// ____________________________________________________________________________

class CAAudioFileReader : public CAPullBufferQueue, public CAAudioFileStreamer {
public:
	CAAudioFileReader(int nBuffers, UInt32 bufferSizeFrames) :
		CAPullBufferQueue(nBuffers, bufferSizeFrames) { }
	
	void				SetFile(const FSRef &inFile);
	virtual void		Start();
	virtual void		Stop();
	double				GetCurrentPosition() const;	// 0-1
	SInt64				GetCurrentFrame() const;// { return mFile->Tell(); }
	SInt64				GetNumberFrames() const { return mFile->GetNumberFrames(); }

	void				SetCurrentPosition(double loc);	// 0-1

private:
	class FileReadBuffer : public CABufferQueue::Buffer {
	public:
		FileReadBuffer(CABufferQueue *queue, const CAStreamBasicDescription &fmt, UInt32 nBytes) :
			CABufferQueue::Buffer(queue, fmt, nBytes)
		{ }
		
		void			UpdateAfterRead(SInt64 curFrame, UInt32 nFramesRead);
		void			GetLocation(UInt32 &frm0, UInt32 &frame1) const {
							frm0 = mStartFrame; frame1 = mEndFrame;
						}
		
		SInt64			mBufferStartFileFrame;
	};
	
	virtual CABufferQueue::Buffer *	CreateBuffer(const CAStreamBasicDescription &fmt, UInt32 nBytes) {
							return new FileReadBuffer(this, fmt, nBytes);
						}
	virtual void		ProcessBuffer(CABufferQueue::Buffer *b) {
							ReadBuffer(static_cast<FileReadBuffer *>(b));
						}
	void				ReadBuffer(FileReadBuffer *b);
};

// ____________________________________________________________________________

class CAAudioFileWriter : public CAPushBufferQueue, public CAAudioFileStreamer {
public:
	CAAudioFileWriter(int nBuffers, UInt32 bufferSizeFrames) :
		CAPushBufferQueue(nBuffers, bufferSizeFrames) { }

	void				SetFile(AudioFileID file);
	void				SetFile(const FSRef &parentDir, CFStringRef filename, AudioFileTypeID filetype, const CAStreamBasicDescription &dataFormat, const CAAudioChannelLayout *layout);
	virtual void		Start();
	virtual void		Stop();

private:
	virtual CABufferQueue::Buffer *	CreateBuffer(const CAStreamBasicDescription &fmt, UInt32 nBytes) {
							return new CABufferQueue::Buffer(this, fmt, nBytes);
						}
	virtual void		ProcessBuffer(CABufferQueue::Buffer *b) {
							WriteBuffer(b);
						}
	void				WriteBuffer(CABufferQueue::Buffer *b);
};

#endif // __CAAudioFileStreamer_h__

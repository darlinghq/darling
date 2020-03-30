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
	CABufferQueue.h
	
=============================================================================*/

#ifndef __CABufferQueue_h__
#define __CABufferQueue_h__

#include "CAPThread.h"
#include "CAGuard.h"
#include "CAStreamBasicDescription.h"
#include "CABufferList.h"
#include <list>
#include "CAAtomicStack.h"

// ____________________________________________________________________________

// Abstraction for moving audio buffers between threads.
// Has abstract subclasses for push and pull.
class CABufferQueue {
	friend class CAPushBufferQueue;
	friend class CAPullBufferQueue;
public:
						CABufferQueue(int nBuffers, UInt32 bufferSizeFrames);
	virtual				~CABufferQueue();

	void				SetFormat(const CAStreamBasicDescription &fmt);
	UInt32				GetBufferSizeFrames() const { return mBufferSizeFrames; }
	int					ErrorCount() const { return mErrorCount; }
	
	// -----
	class Buffer {
	public:
		Buffer(CABufferQueue *owner, const CAStreamBasicDescription &fmt, UInt32 nBytes);
		
		~Buffer() { delete mMemory; }
		
		CABufferQueue * Queue() { return mQueue; }
		CABufferList *  GetBufferList() { return mMemory; }
		UInt32			FrameCount() { return mEndFrame - mStartFrame; }
		void			SetEmpty() { mStartFrame = mEndFrame = 0; }
		
		void			SetInProgress(bool b) { mInProgress = b; }
		bool			InProgress() const { return mInProgress; }
		bool			ReachedEndOfStream() const { return mEndOfStream; }

		bool			CopyInto(AudioBufferList *destBufferList, int bytesPerFrame, UInt32 &framesProduced, UInt32 &framesRequired);	// return true if buffer emptied

		bool			CopyFrom(const AudioBufferList *srcBufferList, int bytesPerFrame, UInt32 &framesProduced, UInt32 &framesRequired); // return true if buffer filled and not end-of-stream
		
		Buffer *		get_next() { return mNext; }
		void			set_next(Buffer *b) { mNext = b; }
		Buffer *& 		next() { return mNext; }
		
#if DEBUG
		void			print() {
							printf("Buffer %p:\n  inProgress %d, endOfStream %d, frames %ld-%ld\n", this, mInProgress, mEndOfStream, mStartFrame, mEndFrame);
						}
#endif
		
	protected:
		Buffer *		mNext;
		CABufferQueue * mQueue;
		CABufferList *	mMemory;
		UInt32			mByteSize;
		
		bool			mInProgress;				// true if in the work queue
		bool			mEndOfStream;				// true if the operation resulted in end-of-stream
		UInt32			mStartFrame, mEndFrame;		// produce/consume pointers within the buffer
	};
	
#if DEBUG
	void	print() {
		printf("BufferQueue %p\n  mCurrentBuffer=%d\n", this, mCurrentBuffer);
		if (mBuffers)
		for (int i = 0; i < mNumberBuffers; ++i) {
			Buffer *b = mBuffers[i];
			printf("  buffer[%d]: ", i);
			if (b)
				b->print();
			else printf("NULL\n");
		}
	}
#endif
	
protected:	
	virtual Buffer *	CreateBuffer(const CAStreamBasicDescription &fmt, UInt32 nBytes) = 0;
	virtual void		ProcessBuffer(Buffer *b) = 0;
	void				CancelBuffers();
	void				CancelAndDisposeBuffers();
	
	CABufferList *		GetBufferList() { return mBufferList; }
	const Buffer *		GetCurrentBuffer() const { return mBuffers[mCurrentBuffer]; }
	UInt32				GetBytesPerFrame() const { return mBytesPerFrame; }

private:
	
	// -----
	class WorkThread : public CAPThread {
	public:
		WorkThread();
		
		static void * ThreadEntry(void *param)
		{
			static_cast<WorkThread *>(param)->Run();
			return NULL;
		}
		void	Run();
		void	Stop();
		
		void	AddBuffer(Buffer *buffer);
		void	RemoveBuffers(CABufferQueue *owner);
	
	private:
		typedef std::list<Buffer *> WorkQueue;

		bool			mStopped;
		WorkQueue		mWorkQueue;
		CAGuard			mRunGuard;
		TAtomicStack<Buffer>  mBuffersToAdd;
	};
	
	static WorkThread *	sWorkThread;
	
	// -----
private:
	WorkThread *		mWorkThread;

	int					mCurrentBuffer;
	int					mNumberBuffers;
	Buffer **			mBuffers;					// array of pointers
	UInt32				mBufferSizeFrames;
	UInt32				mBytesPerFrame;				// function of client format
	CABufferList *		mBufferList;				// maintained in SetFormat
protected:
	int					mErrorCount;
};

// ____________________________________________________________________________

// Abstract class.
// The client pushes buffers in; they are consumed (via ProcessBuffer) on the work thread.
// (ex: file recorder)
class CAPushBufferQueue : public CABufferQueue {
public:
	CAPushBufferQueue(int nBuffers, UInt32 bufferSizeFrames) :
		CABufferQueue(nBuffers, bufferSizeFrames) { }

	void			PushBuffer(UInt32 inNumberFrames, const AudioBufferList *inBufferList);
						// push a buffer in
	void			Flush();
						// emit a possibly incomplete final buffer
};

// ____________________________________________________________________________

// Abstract class.
// The client pulls buffers out; they are produced (via ProcessBuffer) on the work thread.
// (ex: file player)
class CAPullBufferQueue : public CABufferQueue {
public:
	CAPullBufferQueue(int nBuffers, UInt32 bufferSizeFrames) :
		CABufferQueue(nBuffers, bufferSizeFrames),
		mEndOfStream(false) { }

	void			Prime();
						// produce initial buffers
	void			PullBuffer(UInt32 &ioFrames, AudioBufferList *outBufferList);
						// pull a buffer out
	bool			ReachedEndOfStream() const { return mEndOfStream; }

protected:
	bool			mEndOfStream;
};

#endif // __CABufferQueue_h__

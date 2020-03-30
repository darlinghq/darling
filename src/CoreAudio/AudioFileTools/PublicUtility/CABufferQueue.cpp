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
	CABufferQueue.cpp
	
=============================================================================*/

#include "CABufferQueue.h"

#if TARGET_OS_WIN32
	#include "CAWindows.h"
#endif

// ____________________________________________________________________________

CABufferQueue::WorkThread *CABufferQueue::sWorkThread = NULL;

CABufferQueue::WorkThread::WorkThread() :
	CAPThread(ThreadEntry, this, CAPThread::kMaxThreadPriority, true),
	mStopped(false),
	mRunGuard("CABufferQueue::mRunGuard")
{
	// prime the container to have some elements so we're not calling malloc dynamically
	Buffer *b = NULL;
	for (int i = 0; i < 64; ++i)
		mWorkQueue.push_back(b);
	mWorkQueue.clear();
	Start();
}

void	CABufferQueue::WorkThread::Run()
{
	while (!mStopped) {
		CAGuard::Locker lock(mRunGuard);
		mRunGuard.Wait();
		
		while (!mStopped) {
			Buffer *b, *next;
			
			// add buffers from the other thread
			TAtomicStack<Buffer> reversed;
			
			b = mBuffersToAdd.pop_all();	// these are in reverse order
			while (b != NULL) {
				next = b->get_next();
				reversed.push_NA(b);
				b = next;
			}
			while ((b = reversed.pop_NA()) != NULL)
				mWorkQueue.push_back(b);
			
			if (mWorkQueue.empty())
				break;
			b = mWorkQueue.front();
			mWorkQueue.pop_front();
			
			b->Queue()->ProcessBuffer(b);
			b->SetInProgress(false);
		}
	}
}

void	CABufferQueue::WorkThread::Stop()
{
	mStopped = true;
	mRunGuard.Notify();
}

void	CABufferQueue::WorkThread::AddBuffer(Buffer *b)
{
	b->SetInProgress(true);
	mBuffersToAdd.push_atomic(b);
	mRunGuard.Notify();
}

void	CABufferQueue::WorkThread::RemoveBuffers(CABufferQueue *owner)
{
	CAGuard::Locker lock(mRunGuard);
	for (WorkQueue::iterator it = mWorkQueue.begin(); it != mWorkQueue.end(); ) {
		if ((*it)->Queue() == owner) {
			WorkQueue::iterator next = it;	++next;
			mWorkQueue.erase(it);
			it = next;
		} else
			++it;
	}
}

// ____________________________________________________________________________

CABufferQueue::Buffer::Buffer(CABufferQueue *queue, const CAStreamBasicDescription &fmt, UInt32 nBytes) :
	mQueue(queue)
{
	mMemory = CABufferList::New("", fmt);
	mMemory->AllocateBuffers(nBytes);
	mByteSize = nBytes;
	mInProgress = false;
	mStartFrame = mEndFrame = 0;
	mEndOfStream = false;
}

// return true if buffer emptied AND we're not at end-of-stream
bool CABufferQueue::Buffer::CopyInto(AudioBufferList *destBufferList, int bytesPerFrame, UInt32 &framesProduced, UInt32 &framesRequired)
{
	UInt32 framesInBuffer = mEndFrame - mStartFrame;
	UInt32 framesToCopy = std::min(framesInBuffer, framesRequired);
	if (framesToCopy > 0) {
		const CABufferList *bufMemory = mMemory;
		const AudioBufferList &srcBufferList = bufMemory->GetBufferList();
		const AudioBuffer *srcbuf = srcBufferList.mBuffers;
		AudioBuffer *dstbuf = destBufferList->mBuffers;
		for (int i = destBufferList->mNumberBuffers; --i >= 0; ++srcbuf, ++dstbuf) {
			memcpy(
				(Byte *)dstbuf->mData + framesProduced * bytesPerFrame,
				(Byte *)srcbuf->mData + mStartFrame * bytesPerFrame,
				framesToCopy * bytesPerFrame);
		}
		framesProduced += framesToCopy;
		framesRequired -= framesToCopy;
		mStartFrame += framesToCopy;
	}
	return (framesToCopy == framesInBuffer) && !mEndOfStream;
}

// return true if buffer filled
bool	CABufferQueue::Buffer::CopyFrom(const AudioBufferList *srcBufferList, int bytesPerFrame, UInt32 &framesProduced, UInt32 &framesRequired)
{
	UInt32 framesInBuffer = mEndFrame - mStartFrame;
	UInt32 freeFramesInBuffer = (mByteSize / bytesPerFrame) - framesInBuffer;
	UInt32 framesToCopy = std::min(freeFramesInBuffer, framesRequired);
	if (framesToCopy > 0) {
		const AudioBuffer *srcbuf = srcBufferList->mBuffers;
		const CABufferList *bufMemory = mMemory;
		const AudioBufferList &destBufferList = bufMemory->GetBufferList();
		const AudioBuffer *dstbuf = destBufferList.mBuffers;
		for (int i = srcBufferList->mNumberBuffers; --i >= 0; ++srcbuf, ++dstbuf) {
			memcpy(
				(Byte *)dstbuf->mData + framesInBuffer * bytesPerFrame,
				(Byte *)srcbuf->mData + framesProduced * bytesPerFrame,
				framesToCopy * bytesPerFrame);
		}
		framesProduced += framesToCopy;
		framesRequired -= framesToCopy;
		mEndFrame += framesToCopy;
	}
	return (framesToCopy == freeFramesInBuffer);
}

// ____________________________________________________________________________

CABufferQueue::CABufferQueue(int nBuffers, UInt32 bufferSizeFrames) :
	mNumberBuffers(nBuffers),
	mBuffers(NULL),
	mBufferSizeFrames(bufferSizeFrames),
	mBufferList(NULL)
{
	mCurrentBuffer = 0;
	mErrorCount = 0;
	
	if (sWorkThread == NULL)
		sWorkThread = new WorkThread();
	mWorkThread = sWorkThread;  // for now
}

CABufferQueue::~CABufferQueue()
{
	CancelAndDisposeBuffers();
}

void	CABufferQueue::CancelBuffers()
{
	mWorkThread->RemoveBuffers(this);
}

void	CABufferQueue::CancelAndDisposeBuffers()
{
	CancelBuffers();
	if (mBuffers) {
		for (int i = 0; i < mNumberBuffers; ++i)
			delete mBuffers[i];
		delete[] mBuffers;		mBuffers = NULL;
	}
	delete mBufferList;		mBufferList = NULL;
}

void	CABufferQueue::SetFormat(const CAStreamBasicDescription &fmt)
{
	CancelAndDisposeBuffers();
	
	mBytesPerFrame = fmt.mBytesPerFrame;
	mBuffers = new Buffer*[mNumberBuffers];
	for (int i = 0; i < mNumberBuffers; ++i)
		mBuffers[i] = CreateBuffer(fmt, mBufferSizeFrames * mBytesPerFrame);
	mBufferList = CABufferList::New("", fmt);
}

// ____________________________________________________________________________

void	CAPushBufferQueue::PushBuffer(UInt32 inNumberFrames, const AudioBufferList *inBufferList)
{
	UInt32 framesRequired = inNumberFrames;
	UInt32 framesProduced = 0;
	
	do {
		Buffer *b = mBuffers[mCurrentBuffer];
		
		if (b->InProgress()) {
			++mErrorCount;
			break;
		}
		
		if (b->CopyFrom(inBufferList, mBytesPerFrame, framesProduced, framesRequired)) {
			// buffer was filled, we're done with it
			sWorkThread->AddBuffer(b);
			if (++mCurrentBuffer == mNumberBuffers)
				mCurrentBuffer = 0;
		}
	} while (framesRequired > 0);
}

void	CAPushBufferQueue::Flush()
{
	if (mBuffers != NULL) {
		Buffer *b = mBuffers[mCurrentBuffer];
		if (b->FrameCount() > 0 && !b->InProgress())
			ProcessBuffer(b);
	}
}

// ____________________________________________________________________________

void	CAPullBufferQueue::PullBuffer(UInt32 &ioFrames, AudioBufferList *outBufferList)
{
	if (mEndOfStream) {
		ioFrames = 0;
		return;
	}
	UInt32 framesRequired = ioFrames;
	UInt32 framesProduced = 0;
	
	do {
		Buffer *b = mBuffers[mCurrentBuffer];
		
		if (b->InProgress()) {
			++mErrorCount;
			break;
		}
		
		if (b->CopyInto(outBufferList, mBytesPerFrame, framesProduced, framesRequired)) {
			// buffer emptied
			sWorkThread->AddBuffer(b);
	
			if (++mCurrentBuffer == mNumberBuffers)
				mCurrentBuffer = 0;
		}
		else if (b->ReachedEndOfStream()) {
			mEndOfStream = true;
			break;
		}
	} while (framesRequired > 0);
	ioFrames = framesProduced;
}

void	CAPullBufferQueue::Prime()
{
	mEndOfStream = false;
	for (int i = 0; i < mNumberBuffers; ++i) {
		Buffer *b = mBuffers[i];
		ProcessBuffer(b);
		b->SetInProgress(false);
	}
	mCurrentBuffer = 0;
}


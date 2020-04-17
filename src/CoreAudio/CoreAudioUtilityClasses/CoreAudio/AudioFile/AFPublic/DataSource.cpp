/*
     File: DataSource.cpp
 Abstract: DataSource.h
  Version: 1.1
 
 Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple
 Inc. ("Apple") in consideration of your agreement to the following
 terms, and your use, installation, modification or redistribution of
 this Apple software constitutes acceptance of these terms.  If you do
 not agree with these terms, please do not use, install, modify or
 redistribute this Apple software.
 
 In consideration of your agreement to abide by the following terms, and
 subject to these terms, Apple grants you a personal, non-exclusive
 license, under Apple's copyrights in this original Apple software (the
 "Apple Software"), to use, reproduce, modify and redistribute the Apple
 Software, with or without modifications, in source and/or binary forms;
 provided that if you redistribute the Apple Software in its entirety and
 without modifications, you must retain this notice and the following
 text and disclaimers in all such redistributions of the Apple Software.
 Neither the name, trademarks, service marks or logos of Apple Inc. may
 be used to endorse or promote products derived from the Apple Software
 without specific prior written permission from Apple.  Except as
 expressly stated in this notice, no other rights or licenses, express or
 implied, are granted by Apple herein, including but not limited to any
 patent rights that may be infringed by your derivative works or by other
 works in which the Apple Software may be incorporated.
 
 The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
 MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
 THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
 FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
 OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
 
 IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
 OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
 MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
 AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
 STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
 
 Copyright (C) 2014 Apple Inc. All Rights Reserved.
 
*/
#include "DataSource.h"
#if TARGET_OS_WIN32
	#include <io.h>
#else
	#include <unistd.h>
	#include <fcntl.h>
#endif
#include <sys/stat.h>
#include <algorithm>
#include <climits>

#define VERBOSE 0

//////////////////////////////////////////////////////////////////////////////////////////


const UInt16 kPositionModeMask = 3;

DataSource::DataSource(Boolean inCloseOnDelete)
	: mCloseOnDelete(inCloseOnDelete)
{
}

DataSource::~DataSource()
{
}

SInt64 DataSource::CalcOffset(	UInt16 positionMode, 
								SInt64 positionOffset,
								SInt64 currentOffset,
								SInt64 size)
{
	SInt64 newOffset = 0;
	switch (positionMode & kPositionModeMask) {
		//case fsAtMark : newOffset = currentOffset; break;
		case SEEK_SET : newOffset = positionOffset; break;
		case SEEK_END : newOffset = size + positionOffset; break;
		case SEEK_CUR : newOffset = positionOffset + currentOffset; break;
	}
	return newOffset;
}

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

#if 0

MacFile_DataSource::MacFile_DataSource( FSIORefNum inForkRefNum, SInt8 inPermissions, Boolean inCloseOnDelete)
	: DataSource(inCloseOnDelete), mFileNum(inForkRefNum), mPermissions(inPermissions)
{
}

MacFile_DataSource::~MacFile_DataSource()
{
	if (mCloseOnDelete) FSCloseFork(mFileNum);
}


OSStatus MacFile_DataSource::GetSize(SInt64& outSize)
{
	outSize = -1; // in case of error
	OSStatus err = FSGetForkSize(mFileNum, &outSize);
	return err;
}

OSStatus MacFile_DataSource::GetPos(SInt64& outPos) const
{
	return FSGetForkPosition(mFileNum, &outPos);
}

OSStatus MacFile_DataSource::SetSize(SInt64 inSize)
{
	return FSSetForkSize(mFileNum, SEEK_SET, inSize);
}


OSStatus MacFile_DataSource::ReadBytes(
					UInt16 positionMode, 
					SInt64 positionOffset, 
					UInt32 requestCount, 
					void *buffer, 
					UInt32* actualCount)
{
	if (actualCount) *actualCount = 0;
	if (!buffer) return kAudio_ParamError;
	ByteCount bc_actualCount = 0;
	OSStatus err = FSReadFork(mFileNum, positionMode, positionOffset, requestCount, buffer, &bc_actualCount);
	if (actualCount) *actualCount = (UInt32)bc_actualCount;
	return err;
}

OSStatus MacFile_DataSource::WriteBytes(
					UInt16 positionMode, 
					SInt64 positionOffset, 
					UInt32 requestCount, 
					const void *buffer, 
					UInt32* actualCount)
{
	if (!buffer) return kAudio_ParamError;
	ByteCount bc_actualCount = 0;
	OSStatus err =  FSWriteFork(mFileNum, positionMode, positionOffset, requestCount, buffer, &bc_actualCount);
	if (actualCount) *actualCount = (UInt32)bc_actualCount;
	return err;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////

#define kAudioFileNoCacheMask		0x20

UnixFile_DataSource::UnixFile_DataSource( int inFD, SInt8 inPermissions, Boolean inCloseOnDelete)
	: DataSource(inCloseOnDelete), mFileD(inFD), mPermissions(inPermissions), mNoCache(0), mCachedSize(-1), mFilePointer(0)
{
}

UnixFile_DataSource::~UnixFile_DataSource()
{
	if (mCloseOnDelete) close(mFileD);
}
	
OSStatus	UnixFile_DataSource::GetSize(SInt64& outSize)
{
#if 0 // 6764274 using the cached file size causes a regression for apps that play a file while writing to it.
	if (mCachedSize >= 0) {
		outSize = mCachedSize;
		return noErr;
	}
#endif
	outSize = -1; // in case of error
	struct stat stbuf;
	if (fstat (mFileD, &stbuf) == -1) return kAudio_FileNotFoundError;
	outSize = mCachedSize = stbuf.st_size;
	return noErr;
}

OSStatus	UnixFile_DataSource::SetSize(SInt64 inSize)
{
	mCachedSize = -1;
#if TARGET_OS_WIN32
	if (chsize (mFileD, inSize)) return kAudioFilePermissionsError;
#else
	if (ftruncate (mFileD, inSize) == -1) return kAudioFilePermissionsError;
#endif
	mCachedSize = inSize;
	return noErr;
}


OSStatus	UnixFile_DataSource::GetPos(SInt64& outPos) const
{
	outPos = mFilePointer;
	return noErr;
}

SInt64		UnixFile_DataSource::UFCurrentOffset (UInt16	positionMode, 
												SInt64		positionOffset)
{
	SInt64 offset = -1;
	switch (positionMode & kPositionModeMask) 
	{
		/*case fsAtMark : 
		{
			SInt64 pos;
			OSStatus result = GetPos (pos);
				if (result) return result;
			offset = pos; 
			break;
		}*/
		case SEEK_SET :
		{
			offset = positionOffset; 
			break;
		}
		case SEEK_END : 
		{
			SInt64 size;
			OSStatus result = GetSize (size);
				if (result) return result;
			offset = size + positionOffset; 
			break;
		}
		case SEEK_CUR :
		{
			SInt64 pos;
			OSStatus result = GetPos (pos);
				if (result) return result;
			offset = positionOffset + pos; 
			break;
		}
	}
	return offset;
}
	
OSStatus	UnixFile_DataSource::ReadBytes(	UInt16 positionMode,
								SInt64 positionOffset, 
								UInt32 requestCount, 
								void *buffer, 
								UInt32* actualCount)
{
	if (actualCount) *actualCount = 0;
	if (!buffer) return kAudio_ParamError;

		// can't use current offset as we need to go to the disk too much		
	SInt64 offset = UFCurrentOffset (positionMode, positionOffset);
		if (offset < 0) return kAudioFilePositionError;

#if 0 // 6571050 fstat-ing the file every read causes a performance regression
	// 5931571 check that read may exceed eof and curtail it.
	do {
		SInt64 size;
		OSStatus serr = GetSize(size);
		if (serr) break;
		SInt64 remain = size - offset;
		if (remain < 0) requestCount = 0;
		else if (requestCount > remain) requestCount = remain;
	} while (false);
#endif
	
	if (requestCount <= 0) {
		if (actualCount) *actualCount = 0;
		return noErr;
	}

#if !TARGET_OS_WIN32
	UInt32 noCache = positionMode & kAudioFileNoCacheMask ? 1 : 0;
	if (noCache != mNoCache) {
		mNoCache = noCache;
		fcntl(mFileD, F_NOCACHE, mNoCache);
	}
#endif

	size_t readBytes = requestCount;
#if TARGET_OS_WIN32
	lseek(mFileD, offset, SEEK_SET);
	int numBytes = read (mFileD, buffer, readBytes);
#else
	ssize_t numBytes = pread (mFileD, buffer, readBytes, offset);
#endif
	if (numBytes == -1) return kAudioFilePositionError;
	mFilePointer = offset + numBytes;
	
	if (actualCount) *actualCount = (UInt32)numBytes;
	return noErr;
}
						
OSStatus	UnixFile_DataSource::WriteBytes(UInt16 positionMode, 
								SInt64 positionOffset, 
								UInt32 requestCount, 
								const void *buffer, 
								UInt32* actualCount)
{
	if (!buffer) return kAudio_ParamError;

		// can't use current offset as we need to go to the disk too much		
	SInt64 offset = UFCurrentOffset (positionMode, positionOffset);
		if (offset < 0) return kAudioFilePositionError;

	mCachedSize = -1;

	size_t writeBytes = requestCount;

#if !TARGET_OS_WIN32
	UInt32 noCache = positionMode & kAudioFileNoCacheMask ? 1 : 0;
	if (noCache != mNoCache) {
		mNoCache = noCache;
		fcntl(mFileD, F_NOCACHE, mNoCache);
	}
#endif

#if TARGET_OS_WIN32
	lseek(mFileD, offset, SEEK_SET);
	int numBytes = write (mFileD, buffer, writeBytes);
#else
	ssize_t numBytes = pwrite (mFileD, buffer, writeBytes, offset);
#endif
	if (numBytes == -1) return kAudioFilePositionError;
	mFilePointer = offset + numBytes;
	
	if (actualCount) *actualCount = (UInt32)numBytes;
	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////////

#define NO_CACHE 0

OSStatus Cached_DataSource::ReadFromHeaderCache(
					SInt64 offset, 
					UInt32 requestCount,
					void *buffer, 
					UInt32* actualCount)
{
	if (actualCount) *actualCount = 0;
	OSStatus err = noErr;
	ByteCount theActualCount = 0;

#if VERBOSE	
	printf("read from header %lld %lu   %lld %lu\n", offset, requestCount, 0LL, mHeaderCacheSize);
#endif

	if (!mHeaderCache()) 
	{
		mHeaderCache.allocBytes(mHeaderCacheSize, true);
		err = mDataSource->ReadBytes(SEEK_SET, 0, mHeaderCacheSize, mHeaderCache(), &mHeaderCacheSize);
		if (err == kAudioFileEndOfFileError) err = noErr;
		if (err) return err;
	}
	
	ByteCount firstPart = std::min((ByteCount)requestCount, (ByteCount)(mHeaderCacheSize - offset));
	ByteCount secondPart = requestCount - firstPart;
	
	memcpy(buffer, mHeaderCache + (ByteCount)offset, firstPart);
	theActualCount = firstPart;
	
	if (secondPart) {
		UInt32 secondPartActualCount = 0;
		err = mDataSource->ReadBytes(SEEK_SET, mHeaderCacheSize, static_cast<UInt32>(secondPart), (char*)buffer + firstPart, &secondPartActualCount);
		theActualCount += secondPartActualCount;
	}
	
	if (actualCount) *actualCount = (UInt32)theActualCount;
	mOffset = offset + theActualCount;
	
	return err;
}

OSStatus Cached_DataSource::ReadBytes(
					UInt16 positionMode, 
					SInt64 positionOffset, 
					UInt32 requestCount, 
					void *buffer, 
					UInt32* actualCount)
{
	if (actualCount) *actualCount = 0;
	OSStatus err = noErr;
	SInt64 size;
	UInt32 theActualCount = 0;

	if (!buffer) return kAudio_ParamError;

	if ((positionMode & kPositionModeMask) != SEEK_END) size = 0; // not used in this case
	else 
	{
		err = GetSize(size);
		if (err) return err;
	}

	SInt64 offset = CalcOffset(positionMode, positionOffset, mOffset, size);
	if (offset < 0) return kAudioFilePositionError;
	
	if (offset < mHeaderCacheSize) {
		return ReadFromHeaderCache(offset, requestCount, buffer, actualCount);
	}

#if NO_CACHE
	err = mDataSource->ReadBytes(positionMode, positionOffset, requestCount, buffer, &theActualCount);
	mOffset = offset + theActualCount;
#else

	SInt64 cacheEnd = mBodyCacheOffset + mBodyCacheCurSize;
	if (mBodyCache() && requestCount < mBodyCacheSize && offset >= mBodyCacheOffset && offset < cacheEnd)
	{
		if (offset + requestCount <= cacheEnd) 
		{
			// request is entirely within cache
#if VERBOSE	
			printf("request is entirely within cache %lld %lu   %lld %lu\n", offset, requestCount, mBodyCacheOffset, mBodyCacheCurSize);
#endif
			memcpy(buffer, mBodyCache + (size_t)(offset - mBodyCacheOffset), requestCount);
			theActualCount = requestCount;
		}
		else
		{
			// part of request is within cache. copy, read next cache block, copy.
#if VERBOSE	
			printf("part of request is within cache %lld %lu   %lld %lu\n", offset, requestCount, mBodyCacheOffset, mBodyCacheCurSize);
#endif
			
			// copy first part.
			ByteCount firstPart = (ByteCount)(cacheEnd - offset);
			ByteCount secondPart = requestCount - firstPart;
#if VERBOSE	
			printf("memcpy   offset %lld  mBodyCacheOffset %lld  offset - mBodyCacheOffset %lld  firstPart %lu   requestCount %lu\n", 
						offset, mBodyCacheOffset, offset - mBodyCacheOffset, firstPart, requestCount);
#endif
			memcpy(buffer, mBodyCache + (size_t)(offset - mBodyCacheOffset), firstPart);
			
			theActualCount = static_cast<UInt32>(firstPart);
			
			// read new block
			SInt64 nextOffset = mBodyCacheOffset + mBodyCacheCurSize;
			err = mDataSource->ReadBytes(SEEK_SET, nextOffset, mBodyCacheSize, mBodyCache(), &mBodyCacheCurSize);
			
			if (err == kAudioFileEndOfFileError) err = noErr;
			if (err) goto leave;

			mBodyCacheOffset = nextOffset;

			// copy second part
			secondPart = std::min(secondPart, (ByteCount)mBodyCacheCurSize);
			if (secondPart) memcpy((char*)buffer + firstPart, mBodyCache(), secondPart);
			theActualCount = static_cast<UInt32>(firstPart + secondPart);
		}
	}
	else 
	{
		if (requestCount > mBodyCacheSize)
		{
#if VERBOSE	
			printf("large request %lld %lu   %lld %lu\n", offset, requestCount, mBodyCacheOffset, mBodyCacheCurSize);
#endif
			// the request is larger than we normally cache, just do a read and don't cache.
			err = mDataSource->ReadBytes(positionMode, positionOffset, requestCount, buffer, &theActualCount);
			mOffset = offset + theActualCount;
		}
		else
		{
			// request is outside cache. read new block.
#if VERBOSE	
			printf("request is outside cache %lld %lu   %lld %lu\n", offset, requestCount, mBodyCacheOffset, mBodyCacheCurSize);
#endif
			if (!mBodyCache()) 
			{
				mBodyCache.allocBytes(mBodyCacheSize, true);
#if VERBOSE	
				printf("alloc mBodyCache %08X\n", mBodyCache());
#endif
			}
			mBodyCacheOffset = offset;
			err = mDataSource->ReadBytes(SEEK_SET, mBodyCacheOffset, mBodyCacheSize, mBodyCache(), &mBodyCacheCurSize);
#if VERBOSE	
			printf("read %08X %d    mBodyCacheOffset %lld   %lu %lu\n", err, err, mBodyCacheOffset, mBodyCacheSize, mBodyCacheCurSize);
#endif
			if (err == kAudioFileEndOfFileError) err = noErr;
			if (err) return err;

			theActualCount = std::min(requestCount, mBodyCacheCurSize);
			memcpy(buffer, mBodyCache(), theActualCount);
		}
		
	}

leave:
#endif
	if (actualCount) *actualCount = (UInt32)theActualCount;
#if VERBOSE	
	printf("<<read err %d  actualCount %lu\n", err, *actualCount);
#endif
	return err;
}

OSStatus Cached_DataSource::WriteBytes(
					UInt16 positionMode, 
					SInt64 positionOffset, 
					UInt32 requestCount, 
					const void *buffer, 
					UInt32* actualCount)
{
	OSStatus err = noErr;
	SInt64 size;

	if (!buffer) return kAudio_ParamError;
	
	if ((positionMode & kPositionModeMask) != SEEK_END) size = 0; // not used in this case
	else 
	{
		err = GetSize(size);
		if (err) return err;
	}

	SInt64 offset = CalcOffset(positionMode, positionOffset, mOffset, size);
	if (offset < 0) return kAudioFilePositionError;
	
	if (mHeaderCache() && offset < mHeaderCacheSize) 
	{
		// header cache write through
		ByteCount firstPart = std::min((ByteCount)requestCount, (ByteCount)(mHeaderCacheSize - offset));
#if VERBOSE	
		printf("header cache write through %lu %lu\n", mHeaderCacheSize, firstPart);
#endif
		memcpy(mHeaderCache + (size_t)offset, buffer, firstPart);
	}
		
#if VERBOSE	
	printf("write %lld %lu    %lld %d %lld\n", offset, requestCount, mOffset, positionMode, positionOffset);
#endif

	SInt64 cacheEnd = mBodyCacheOffset + mBodyCacheCurSize;
	if (mBodyCache() && offset >= mBodyCacheOffset && offset < cacheEnd)
	{
		// body cache write through
		ByteCount firstPart = std::min((SInt64)requestCount, cacheEnd - offset);
#if VERBOSE	
		printf("body cache write through %lld %lu  %lld %lu\n", mBodyCacheOffset, mBodyCacheCurSize, offset, firstPart);
#endif
		memcpy(mBodyCache + (offset - mBodyCacheOffset), buffer, firstPart);
	}
	
	UInt32 theActualCount;
	err = mDataSource->WriteBytes(positionMode, positionOffset, requestCount, buffer, &theActualCount);
	
	mOffset = offset + theActualCount;
	if (actualCount) *actualCount = (UInt32)theActualCount;
	
	return err;
}


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

Seekable_DataSource::Seekable_DataSource(	void * inClientData,
							AudioFile_ReadProc					inReadFunc, 
							AudioFile_WriteProc					inWriteFunc, 
							AudioFile_GetSizeProc				inGetSizeFunc,
							AudioFile_SetSizeProc				inSetSizeFunc)
	: DataSource(false), mClientData(inClientData), mReadFunc(inReadFunc), mWriteFunc(inWriteFunc), 
		mSizeFunc(inGetSizeFunc), mSetSizeFunc(inSetSizeFunc)
{
}

Seekable_DataSource::~Seekable_DataSource()
{
}


OSStatus Seekable_DataSource::GetSize(SInt64& outSize)
{
	if (!mSizeFunc) {
		outSize = LLONG_MAX;
	} else {
		outSize = (*mSizeFunc)(mClientData);
	}
	return noErr;
}

OSStatus Seekable_DataSource::SetSize(SInt64 inSize)
{
	if (!mSetSizeFunc) return kAudioFileOperationNotSupportedError;
	return (*mSetSizeFunc)(mClientData, inSize);
}

OSStatus Seekable_DataSource::ReadBytes(	
							UInt16 positionMode, 
							SInt64 positionOffset, 
							UInt32 requestCount, 
							void *buffer, 
							UInt32* actualCount)
{
	OSStatus err;
	
	if (actualCount) *actualCount = 0;
	if (!mReadFunc) return kAudioFileOperationNotSupportedError;
	if (!buffer) return kAudio_ParamError;

	positionMode &= kPositionModeMask;

	SInt64 size;
	err = GetSize(size);
	if (err) return err;

	SInt64 offset = CalcOffset(positionMode, positionOffset, mOffset, size);
	
	// request is outside bounds of file
	if (offset < 0) 
		return kAudioFilePositionError;
	if (offset >= size) 
		return kAudioFileEndOfFileError;
	
	// reduce request if it exceeds the amount available
	requestCount = static_cast<UInt32>(std::min((SInt64)requestCount, size - offset));
	
	UInt32 theActualCount = 0;
	err = (*mReadFunc)(mClientData, offset, requestCount, buffer, &theActualCount);
	if (actualCount) *actualCount = theActualCount;
	mOffset = offset + theActualCount;
	return err;
}

					
OSStatus Seekable_DataSource::WriteBytes(
							UInt16 positionMode, 
							SInt64 positionOffset, 
							UInt32 requestCount, 
							const void *buffer, 
							UInt32* actualCount)
{
	OSStatus err;
	
	if (!mWriteFunc) return kAudioFileOperationNotSupportedError;
	if (!buffer) return kAudio_ParamError;

	SInt64 size;
	positionMode &= kPositionModeMask;
	if (positionMode != SEEK_END) size = 0; // not used in this case
	else 
	{
		err = GetSize(size);
		if (err) return err;
	}

	SInt64 offset = CalcOffset(positionMode, positionOffset, mOffset, size);
	if (offset < 0) return kAudioFilePositionError;
	
	UInt32 theActualCount;
	err = (*mWriteFunc)(mClientData, offset, requestCount, buffer, &theActualCount);
	if (err) return err;
	if (actualCount) *actualCount = theActualCount;
	mOffset = offset + theActualCount;
	return noErr;
}

//////////////////////////////////////////////////////////////////////////////////////////

OSStatus Buffer_DataSource::ReadBytes(
								UInt16 positionMode, 
								SInt64 positionOffset, 
								UInt32 requestCount, 
								void *buffer, 
								UInt32* actualCount)
{
	if (actualCount) *actualCount = 0;
	SInt64 offsetWithinBuffer = CalcOffset(positionMode, positionOffset, mOffset, mDataByteSize + mStartOffset) - mStartOffset;		
	if (offsetWithinBuffer < 0 || offsetWithinBuffer >= mDataByteSize) return kAudioFilePositionError;
	
	SInt64 bytesAfterOffset = mDataByteSize - offsetWithinBuffer;
	SInt64 theActualCount = std::min(bytesAfterOffset, (SInt64)requestCount);

	if (theActualCount <= 0) {
		if (actualCount) *actualCount = 0;
		return kAudio_ParamError;
	}

	memcpy(buffer, mData + offsetWithinBuffer, theActualCount);
	
	if (actualCount) *actualCount = static_cast<UInt32>(theActualCount);
	mOffset = offsetWithinBuffer + theActualCount;

	return noErr;
}



//////////////////////////////////////////////////////////////////////////////////////////

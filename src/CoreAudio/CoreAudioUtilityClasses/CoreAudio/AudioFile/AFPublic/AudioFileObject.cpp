/*
     File: AudioFileObject.cpp
 Abstract: AudioFileObject.h
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
#include "AudioFileObject.h"
#include "CADebugMacros.h"
#include <algorithm>
#include <sys/stat.h>

#define kAudioFileNoCacheMask		0x20

const SInt64 kScanToEnd = 0x7fffFFFFffffFFFFLL;

//////////////////////////////////////////////////////////////////////////////////////////////////////////

AudioFileObject::~AudioFileObject()
{
	delete mDataSource;
	DeletePacketTable();
	SetURL(NULL);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoCreate(		
									CFURLRef							inFileRef,
									const AudioStreamBasicDescription	*inFormat,
									UInt32								inFlags)
{
	// common prologue
	if (!IsDataFormatValid(inFormat))
		return kAudioFileUnsupportedDataFormatError;

	if (!IsDataFormatSupported(inFormat)) 
		return kAudioFileUnsupportedDataFormatError;

	SetPermissions(kAudioFileReadWritePermission);
	
	SetAlignDataWithFillerChunks(!(inFlags & 2 /* kAudioFileFlags_DontPageAlignAudioData */ ));
	
	// call virtual method for particular format.
	return Create(inFileRef, inFormat);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                
OSStatus AudioFileObject::Create(		
									CFURLRef							inFileRef,
									const AudioStreamBasicDescription	*inFormat)
{
	int fileD;
	OSStatus err = CreateDataFile (inFileRef, fileD);
    FailIf (err != noErr, Bail, "CreateDataFile failed");
	
	SetURL (inFileRef);

	err = OpenFile(kAudioFileReadWritePermission, fileD);
    FailIf (err != noErr, Bail, "OpenFile failed");

	err = SetDataFormat(inFormat);
    FailIf (err != noErr, Bail, "SetDataFormat failed");
	
    mIsInitialized = false;
	
Bail:
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoOpen(	
									CFURLRef		inFileRef, 
									SInt8  			inPermissions,
									int				inFD)
{		
	OSStatus err = noErr;
	SetPermissions(inPermissions);
	
	err = Open(inFileRef, inPermissions, inFD);
    FailIf (err != noErr, Bail, "Open failed");

	err = ValidateFormatAndData();
    FailIf (err != noErr, Bail, "ValidateFormatAndData failed");

Bail:
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::Open(			
									CFURLRef		inFileRef, 
									SInt8  			inPermissions,
									int				inFD)
{		
	if (!(inPermissions & kAudioFileReadPermission))
		return kAudioFilePermissionsError; // file must have read permissions
 
	SetURL(inFileRef);
	
	OSStatus err = OpenFile(inPermissions, inFD);
    FailIf (err != noErr, Bail, "OpenFile failed");
		
	err = OpenFromDataSource();
    FailIf (err != noErr, Bail, "OpenFromDataSource failed");
	
Bail:
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::ValidateFormatAndData()
{
	

	AudioStreamBasicDescription asbd = GetDataFormat();

	if (!IsDataFormatValid(&asbd))
		return kAudioFileInvalidFileError;

	if (asbd.mFormatID == kAudioFormatLinearPCM) 
	{
		SInt64 maxPackets = GetNumBytes() / asbd.mBytesPerPacket;
		if (GetNumPackets() > maxPackets) 
			return kAudioFileInvalidFileError;
	}
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoOpenWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc)
{
	SInt8	perms = (inSetSizeFunc || inWriteFunc) ? kAudioFileReadWritePermission : kAudioFileReadPermission;
	SetPermissions(perms);
	
	DataSource* dataSource = new Seekable_DataSource(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc);
	SetDataSource(dataSource);
	return OpenFromDataSource();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
							
OSStatus AudioFileObject::OpenFromDataSource(void)
{		
	return noErr;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoInitializeWithCallbacks(
				void *								inRefCon, 
				AudioFile_ReadProc					inReadFunc, 
				AudioFile_WriteProc					inWriteFunc, 
				AudioFile_GetSizeProc				inGetSizeFunc,
				AudioFile_SetSizeProc				inSetSizeFunc,
                UInt32								inFileType,
				const AudioStreamBasicDescription	*inFormat,
				UInt32								inFlags)
{		
	DataSource* dataSource = new Seekable_DataSource(inRefCon, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc);
	if (!dataSource->CanWrite()) return -54/*permErr*/;
	dataSource->SetSize(0);
	SetDataSource(dataSource);
	SetPermissions(kAudioFileReadWritePermission);

	SetAlignDataWithFillerChunks(!(inFlags & 2 /* kAudioFileFlags_DontPageAlignAudioData */ ));

	OSStatus err = SetDataFormat(inFormat);
	if (err) return err;
	
	return InitializeDataSource(inFormat, inFlags);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					
OSStatus AudioFileObject::DoInitialize(	
									CFURLRef							inFileRef,
									const AudioStreamBasicDescription	*inFormat,
									UInt32			inFlags)
{
	SetURL (inFileRef);
	SetPermissions(kAudioFileReadWritePermission);

	SetAlignDataWithFillerChunks(!(inFlags & 2 /* kAudioFileFlags_DontPageAlignAudioData */ ));

	return Initialize(inFileRef, inFormat, inFlags);
}
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
					
OSStatus AudioFileObject::Initialize(	
									CFURLRef							inFileRef,
									const AudioStreamBasicDescription	*inFormat,
									UInt32								inFlags)
{
	OSStatus err = noErr;
		
	UInt8 fPath[FILENAME_MAX];	
	if (!CFURLGetFileSystemRepresentation (inFileRef, true, fPath, FILENAME_MAX))
		return kAudio_FileNotFoundError;

#if TARGET_OS_WIN32
	int filePerms = 0;
	int flags = O_TRUNC | O_RDWR | O_BINARY;
#else
	mode_t filePerms = 0;
	int flags = O_TRUNC | O_RDWR;
#endif

	int fileD = open((const char*)fPath, flags, filePerms);
	if (fileD < 0)
		return AudioFileTranslateErrno(errno);
	
	err = OpenFile(kAudioFileReadWritePermission, fileD);
    FailIf (err != noErr, Bail, "OpenFile failed");
	
		// don't need to do this as open has an option to truncate the file
//	GetDataSource()->SetSize(0);

	err = SetDataFormat(inFormat);
    FailIf (err != noErr, Bail, "SetDataFormat failed");

	InitializeDataSource(inFormat, inFlags);
	
Bail:	
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
							
OSStatus AudioFileObject::InitializeDataSource(const AudioStreamBasicDescription	*inFormat, UInt32 /*inFlags*/)
{		
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::DoClose()
{
	OSStatus err = UpdateSizeIfNeeded();
	if (err) return err;
	
	return Close();
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::Close()
{
    try {
		delete mDataSource;
		mDataSource = 0;
	} catch (OSStatus err) {
		return err;
	} catch (...) {
		return kAudioFileUnspecifiedError;
	}
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


OSStatus AudioFileObject::Optimize()
{
	// default is that nothing needs to be done. This happens to be true for Raw, SD2 and NeXT/Sun types.
	SetIsOptimized(true); 
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


OSStatus AudioFileObject::DoOptimize()
{
	if (!CanWrite()) return kAudioFilePermissionsError;

	OSStatus err = UpdateSizeIfNeeded();
	if (err) return err;
	
	if (IsOptimized()) return noErr;

	err = Optimize();
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::UpdateNumBytes(SInt64 inNumBytes)	
{
    OSStatus err = noErr;
	if (inNumBytes != GetNumBytes()) {
		SetNumBytes(inNumBytes);
        
        // #warning " this will not work for vbr formats"
		SetNumPackets(GetNumBytes() / mDataFormat.mBytesPerPacket);
		SizeChanged();
	}
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::UpdateNumPackets(SInt64 inNumPackets)	
{
    OSStatus err = noErr;
	if (inNumPackets != GetNumPackets()) {
		// sync current state.
		SetNeedsSizeUpdate(true);
		UpdateSizeIfNeeded();
		SetNumPackets(inNumPackets);
        
        // #warning " this will not work for vbr formats"
		SetNumBytes(GetNumPackets() * mDataFormat.mBytesPerFrame);
		SizeChanged();
	}
	return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::PacketToFrame(SInt64 inPacket, SInt64& outFirstFrameInPacket)
{
	if (mDataFormat.mFramesPerPacket == 0)
	{
		OSStatus err = ScanForPackets(inPacket+1); // the packet count must be one greater than the packet index
		if (err) return err;
		
		SInt64 packetTableSize = GetPacketTableSize();
		
		if (mPacketTable && inPacket >= packetTableSize)
			return kAudioFileEndOfFileError;
		
		CompressedPacketTable* packetTable = GetPacketTable();
		if (!packetTable)
			return kAudioFileInvalidPacketOffsetError;
			
		if (inPacket < 0 || inPacket >= packetTableSize)
			return kAudioFileInvalidPacketOffsetError;
			
		outFirstFrameInPacket = (*packetTable)[(size_t)inPacket].mFrameOffset;
	}
	else
	{
		outFirstFrameInPacket = inPacket * mDataFormat.mFramesPerPacket;
	}
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::FrameToPacket(SInt64 inFrame, SInt64& outPacket, UInt32& outFrameOffsetInPacket)
{
	if (mDataFormat.mFramesPerPacket == 0)
	{
		CompressedPacketTable* packetTable = GetPacketTable();
		if (!packetTable)
			return kAudioFileInvalidPacketOffsetError;
			
		// search packet table
		AudioStreamPacketDescriptionExtended pext;
		memset(&pext, 0, sizeof(pext));
		pext.mFrameOffset = inFrame;
		CompressedPacketTable::iterator iter = std::lower_bound(packetTable->begin(), packetTable->end(), pext);
		
		if (iter == packetTable->end())
			return kAudioFileInvalidPacketOffsetError;
		
		if (iter > packetTable->begin()) --iter;
		
		outPacket = iter - packetTable->begin();
		outFrameOffsetInPacket = (UInt32)(inFrame - iter->mFrameOffset);
	}
	else
	{
		outPacket = inFrame / mDataFormat.mFramesPerPacket;
		outFrameOffsetInPacket = (UInt32)(inFrame % mDataFormat.mFramesPerPacket);
	}
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::PacketToByte(AudioBytePacketTranslation* abpt)
{
    if (abpt->mPacket < 0)
        return kAudioFileInvalidPacketOffsetError;
    
	if (mDataFormat.mBytesPerPacket == 0)
	{
		CompressedPacketTable* packetTable = GetPacketTable();
		if (!packetTable)
			return kAudioFileInvalidPacketOffsetError;
			
		if (abpt->mPacket < GetPacketTableSize()) {
			abpt->mByte = (*packetTable)[(int)abpt->mPacket].mStartOffset;
			abpt->mFlags = 0;
		} else {
			SInt64 numPackets = packetTable->size();
			if (numPackets < 8) 
				return 'more' /*kAudioFileStreamError_DataUnavailable*/ ;
				
			const AudioStreamPacketDescriptionExtended lastPacket = (*packetTable)[numPackets - 1];
			SInt64 bytesReadSoFar = lastPacket.mStartOffset + lastPacket.mDataByteSize;
			double averageBytesPerPacket = (double)(bytesReadSoFar - GetDataOffset()) / (double)numPackets;
			abpt->mByte = (SInt64)floor((double)abpt->mPacket * averageBytesPerPacket);
			abpt->mFlags = kBytePacketTranslationFlag_IsEstimate;
		}
	}
	else
	{
		abpt->mByte = abpt->mPacket * mDataFormat.mBytesPerPacket;
		abpt->mFlags = 0;
	}
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline bool byte_less_than (const AudioStreamPacketDescriptionExtended& a, const AudioStreamPacketDescriptionExtended& b)
{
	return a.mStartOffset < b.mStartOffset;
}

OSStatus AudioFileObject::ByteToPacket(AudioBytePacketTranslation* abpt)
{
    if (abpt->mByte < 0)
        return kAudioFileInvalidPacketOffsetError;

	if (mDataFormat.mBytesPerPacket == 0)
	{
		CompressedPacketTable* packetTable = GetPacketTable();
		if (!packetTable)
			return kAudioFileInvalidPacketOffsetError;
			// search packet table
		AudioStreamPacketDescriptionExtended pext;
		memset(&pext, 0, sizeof(pext));
		pext.mStartOffset = abpt->mByte;
		CompressedPacketTable::iterator iter = std::lower_bound(packetTable->begin(), packetTable->end(), pext, byte_less_than);
		
		if (iter == packetTable->end()) {
			SInt64 numPackets = packetTable->size();
			if (numPackets < 8) 
				return 'more' /*kAudioFileStreamError_DataUnavailable*/ ;
				
			const AudioStreamPacketDescriptionExtended lastPacket = (*packetTable)[numPackets - 1];
			SInt64 bytesReadSoFar = lastPacket.mStartOffset + lastPacket.mDataByteSize;
			double averageBytesPerPacket = (double)(bytesReadSoFar - GetDataOffset()) / (double)numPackets;
			
			double fpacket = (double)abpt->mByte / averageBytesPerPacket;
			abpt->mPacket = (SInt64)floor(fpacket);
			abpt->mByteOffsetInPacket = (UInt32)floor((fpacket - (double)abpt->mPacket) * averageBytesPerPacket);
			abpt->mFlags = kBytePacketTranslationFlag_IsEstimate;
			
		} else {
			if (iter > packetTable->begin()) --iter;
			abpt->mPacket = iter - packetTable->begin();
			abpt->mByteOffsetInPacket = (UInt32)(abpt->mByte - iter->mStartOffset);
			abpt->mFlags = 0;
		}
	}
	else
	{
		abpt->mPacket = abpt->mByte / mDataFormat.mBytesPerPacket;
		abpt->mByteOffsetInPacket = (UInt32)(abpt->mByte % mDataFormat.mBytesPerPacket);
		abpt->mFlags = 0;
	}
	return noErr;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::ReadBytes(		
								Boolean			inUseCache,
								SInt64			inStartingByte, 
								UInt32			*ioNumBytes, 
								void			*outBuffer)
{
    OSStatus		err = noErr;
    UInt16			mode = SEEK_SET;
	SInt64 			fileOffset = mDataOffset + inStartingByte;
    bool			readingPastEnd = false;
	
    FailWithAction((ioNumBytes == NULL) || (outBuffer == NULL), err = kAudio_ParamError, 
		Bail, "invalid num bytes parameter");

	//printf("inStartingByte %lld  GetNumBytes %lld\n", inStartingByte, GetNumBytes());

	if (inStartingByte >= GetNumBytes()) 
	{
		*ioNumBytes = 0;
		return kAudioFileEndOfFileError;
	}

	if ((fileOffset + *ioNumBytes) > (GetNumBytes() + mDataOffset)) 
	{
		*ioNumBytes = (UInt32)(GetNumBytes() + mDataOffset - fileOffset);
		readingPastEnd = true;
	}
	//printf("fileOffset %lld  mDataOffset %lld  readingPastEnd %d\n", fileOffset, mDataOffset, readingPastEnd);

    if (!inUseCache)
        mode |= kAudioFileNoCacheMask;
	
    err = GetDataSource()->ReadBytes(mode, fileOffset, *ioNumBytes, outBuffer, ioNumBytes);
	
	if (readingPastEnd && err == noErr)
		err = kAudioFileEndOfFileError;

Bail:
    return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::WriteBytes(	
								Boolean			inUseCache,
								SInt64			inStartingByte, 
								UInt32			*ioNumBytes, 
								const void		*inBuffer)
{
    OSStatus		err = noErr;
    UInt16			mode = SEEK_SET;
	bool			extendingTheAudioData;

	if (!CanWrite()) return kAudioFilePermissionsError;

    FailWithAction((ioNumBytes == NULL) || (inBuffer == NULL), err = kAudioFileUnspecifiedError, Bail, "invalid parameters");

    // Do not try to write to a postion greater than 32 bits for some file types
    // see if starting byte + ioNumBytes is greater than 32 bits
    // if so, see if file type supports this and bail if not
    err = IsValidFilePosition(inStartingByte + *ioNumBytes);
    FailIf(err != noErr, Bail, "invalid file position");
    
    extendingTheAudioData = inStartingByte + *ioNumBytes > GetNumBytes();
    
    // if file is not optimized, then do not write data that would overwrite chunks following the sound data chunk
    FailWithAction(	extendingTheAudioData && !IsOptimized(), 
                    err = kAudioFileNotOptimizedError, Bail, "Can't write more data until the file is optimized");

    if (!inUseCache)
        mode |= kAudioFileNoCacheMask;
    
    err = GetDataSource()->WriteBytes(mode, mDataOffset + inStartingByte, *ioNumBytes, 
                        inBuffer, ioNumBytes);
	
    FailIf(err != noErr, Bail, "couldn't write new data");
    
    if (extendingTheAudioData) {
        SInt64		nuEOF;						// Get the total bytes of audio data
        SInt64		nuByteTotal;

		err = GetDataSource()->GetSize(nuEOF);
		FailIf(err != noErr, Bail, "GetSize failed");
            
		nuByteTotal = nuEOF - mDataOffset;
		err = UpdateNumBytes(nuByteTotal);
    }
    
Bail:
    return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::ReadPackets(	
								Boolean							inUseCache,
								UInt32							*outNumBytes,
								AudioStreamPacketDescription	*outPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32  						*ioNumPackets, 
								void							*outBuffer)
{
	// This only works with CBR. To suppport VBR you must override.
    OSStatus		err = noErr;
    
    FailWithAction(outBuffer == NULL, err = kAudio_ParamError, Bail, "NULL buffer");
	
    FailWithAction((ioNumPackets == NULL) || (*ioNumPackets < 1), err = kAudio_ParamError, Bail, "invalid num packets parameter");
    
	{
		UInt32			byteCount = *ioNumPackets * mDataFormat.mBytesPerPacket;
		SInt64			startingByte = inStartingPacket * mDataFormat.mBytesPerPacket;
			
		err = ReadBytes (inUseCache, startingByte, &byteCount, outBuffer);
		if ((err == noErr) || (err == kAudioFileEndOfFileError))
		{
			if (byteCount != (*ioNumPackets * mDataFormat.mBytesPerPacket))
			{
				*ioNumPackets = byteCount / mDataFormat.mBytesPerPacket;
				byteCount = *ioNumPackets * mDataFormat.mBytesPerPacket;
			}

			if (outNumBytes)
				*outNumBytes = byteCount;
			
			if (err == kAudioFileEndOfFileError)
				err = noErr;
		}
	}
Bail:
    return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::ReadPacketData(	
								Boolean							inUseCache,
								UInt32							*ioNumBytes,
								AudioStreamPacketDescription	*outPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32  						*ioNumPackets, 
								void							*outBuffer)
{
	OSStatus		err = noErr;
	FailWithAction(ioNumPackets == NULL || *ioNumPackets < 1, err = kAudio_ParamError, Bail, "invalid ioNumPackets parameter");
	FailWithAction(ioNumBytes   == NULL || *ioNumBytes   < 1, err = kAudio_ParamError, Bail, "invalid ioNumBytes parameter");
	if (mDataFormat.mBytesPerPacket) {
	 	// CBR
		FailWithAction(outBuffer    == NULL, err = kAudio_ParamError, Bail, "NULL buffer");
		UInt32 maxPackets = *ioNumBytes / mDataFormat.mBytesPerPacket;
		if (*ioNumPackets > maxPackets) *ioNumPackets = maxPackets;

		if (outBuffer) {
			UInt32 byteCount = *ioNumPackets * mDataFormat.mBytesPerPacket;
			SInt64 startingByte = inStartingPacket * mDataFormat.mBytesPerPacket;
			err = ReadBytes (inUseCache, startingByte, &byteCount, outBuffer);
			if (err == noErr || err == kAudioFileEndOfFileError) {
				if (byteCount != (*ioNumPackets * mDataFormat.mBytesPerPacket)) {
					*ioNumPackets = byteCount / mDataFormat.mBytesPerPacket;
					byteCount = *ioNumPackets * mDataFormat.mBytesPerPacket;
				}
				*ioNumBytes = byteCount;
			}
		}
	} else {
		FailWithAction(outPacketDescriptions   == NULL, err = kAudio_ParamError, Bail, "invalid outPacketDescriptions parameter");
		err = ReadPacketDataVBR(inUseCache, ioNumBytes, outPacketDescriptions, inStartingPacket, ioNumPackets, outBuffer);
	}
Bail:
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::ReadPacketDataVBR(	
								Boolean							inUseCache,
								UInt32							*ioNumBytes,
								AudioStreamPacketDescription	*outPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32  						*ioNumPackets, 
								void							*outBuffer)
{
	OSStatus err = ScanForPackets(inStartingPacket+1); // need to scan packets up to start
	if (err && err != kAudioFileEndOfFileError)
		return err;

	SInt64 dataOffset = GetDataOffset();

	CompressedPacketTable* packetTable = GetPacketTable();
    if (!packetTable) 
		return kAudioFileInvalidFileError;

	SInt64 packetTableSize = GetPacketTableSize();

	if (inStartingPacket >= packetTableSize) {
		*ioNumBytes = 0;
		*ioNumPackets = 0;
		return kAudioFileEndOfFileError;
	}
	
	if (inStartingPacket + *ioNumPackets <= packetTableSize) {
		err = ReadPacketDataVBR_InTable(inUseCache, ioNumBytes, outPacketDescriptions, inStartingPacket, ioNumPackets, outBuffer);
	} else {

		AudioStreamPacketDescription firstPacket = (*packetTable)[inStartingPacket];
		SInt64 firstPacketOffset = firstPacket.mStartOffset;
		UInt32 bytesRead = *ioNumBytes;
		SInt64 fileSize = 0;
		GetDataSource()->GetSize(fileSize);
		SInt64 remainingBytesInFile = fileSize - firstPacketOffset - dataOffset;
		if (bytesRead > remainingBytesInFile)
			bytesRead = (UInt32)remainingBytesInFile;
		
		err = ReadBytes (inUseCache, firstPacketOffset, &bytesRead, outBuffer);
		if (err && err != kAudioFileEndOfFileError) {
			*ioNumBytes = 0;
			*ioNumPackets = 0;
			return err;
		}
		
		Buffer_DataSource bufSrc(bytesRead, outBuffer, dataOffset + firstPacketOffset);
		
		OSStatus scanErr = ScanForPackets(kScanToEnd, &bufSrc, false);
		if (scanErr && scanErr != kAudioFileEndOfFileError)
			return scanErr;
		packetTableSize = packetTable->size();
		
		UInt32 numPacketsRead = 0;
		UInt32 endOfData = 0;
		SInt64 packetNumber = inStartingPacket;
		for (; numPacketsRead < *ioNumPackets && packetNumber < packetTableSize; ++numPacketsRead, ++packetNumber) {
			AudioStreamPacketDescription curPacket = (*packetTable)[numPacketsRead + inStartingPacket];
			SInt64 curPacketOffset = curPacket.mStartOffset - firstPacketOffset;
			SInt64 endOfPacket = curPacketOffset + curPacket.mDataByteSize;
			if (endOfPacket > bytesRead) break;
			endOfData = (UInt32)endOfPacket;
			outPacketDescriptions[numPacketsRead] = curPacket;				
			outPacketDescriptions[numPacketsRead].mStartOffset = curPacketOffset;
		}
		
		*ioNumBytes = endOfData;
		*ioNumPackets = numPacketsRead;
	}
    return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AudioFileObject::HowManyPacketsCanBeReadIntoBuffer(UInt32* ioNumBytes, SInt64 inStartingPacket, UInt32 *ioNumPackets)
{		
	CompressedPacketTable*	packetTable = GetPacketTable();
	SInt64 packetTableSize = GetPacketTableSize();

	if (inStartingPacket + *ioNumPackets > (SInt64)packetTableSize) {
		*ioNumPackets = (UInt32)(packetTableSize - inStartingPacket);
	}
	
	AudioStreamPacketDescription firstPacket = (*packetTable)[inStartingPacket];

	if (*ioNumBytes < firstPacket.mDataByteSize) {
		*ioNumBytes = 0;
		*ioNumPackets = 0;
		return kAudio_ParamError;
	}

	SInt64 lastPacketIndex = inStartingPacket + *ioNumPackets - 1;
	if (lastPacketIndex >= packetTableSize) 
		lastPacketIndex = packetTableSize - 1;
		
	AudioStreamPacketDescription lastPacket = (*packetTable)[lastPacketIndex];
	
	SInt64 readBytes = lastPacket.mStartOffset + lastPacket.mDataByteSize - firstPacket.mStartOffset;
	if (readBytes <= *ioNumBytes) {
		*ioNumBytes = (UInt32)readBytes;
		return noErr;
	}
	
	SInt64 lowBound = inStartingPacket;
	SInt64 highBound = lastPacketIndex + 1;
	SInt64 okIndex = lowBound;
	while (highBound >= lowBound) {		
		SInt64 tryBound = (lowBound + highBound) >> 1;
		if (tryBound > lastPacketIndex) 
			break;
		AudioStreamPacketDescription tryPacket = (*packetTable)[tryBound];
		
		SInt64 readBytes = tryPacket.mStartOffset + tryPacket.mDataByteSize - firstPacket.mStartOffset;
		
		if (readBytes > (SInt64)*ioNumBytes) {
			highBound = tryBound - 1;
		} else if (readBytes < (SInt64)*ioNumBytes) {
			okIndex = tryBound;
			lowBound = tryBound + 1;
		} else {
			okIndex = tryBound;
			break;
		}
	}	
	
	SInt64 numPackets = okIndex - inStartingPacket + 1;
	if (numPackets > *ioNumPackets) {
		numPackets = *ioNumPackets;
		okIndex = inStartingPacket + numPackets - 1;
	}
	
	AudioStreamPacketDescription packet = (*packetTable)[okIndex];
	*ioNumBytes = (UInt32)(packet.mStartOffset + packet.mDataByteSize - firstPacket.mStartOffset);
	*ioNumPackets = (UInt32)numPackets;
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::ReadPacketDataVBR_InTable(	
								Boolean							inUseCache,
								UInt32							*ioNumBytes,
								AudioStreamPacketDescription	*outPacketDescriptions,
								SInt64							inStartingPacket, 
								UInt32  						*ioNumPackets, 
								void							*outBuffer)
{
	CompressedPacketTable* packetTable = GetPacketTable();
    if (!packetTable) 
		return kAudioFileInvalidFileError;
	
	OSStatus err = HowManyPacketsCanBeReadIntoBuffer(ioNumBytes, inStartingPacket, ioNumPackets);
	if (err) return err;
			
	AudioStreamPacketDescription firstPacket = (*packetTable)[inStartingPacket];
	SInt64 firstPacketOffset = firstPacket.mStartOffset;
	UInt32 bytesRead = *ioNumBytes;
	
	if (outBuffer) {
		err = ReadBytes (inUseCache, firstPacketOffset, &bytesRead, outBuffer);
		if (err && err != kAudioFileEndOfFileError) {
			*ioNumBytes = 0;
			*ioNumPackets = 0;
			return err;
		}
		*ioNumBytes = bytesRead;
	}
	
	// fill out packet descriptions
	if (outPacketDescriptions) {						
		for (UInt32 i = 0; i < *ioNumPackets; i++) {
			AudioStreamPacketDescription curPacket = (*packetTable)[i + inStartingPacket];
			outPacketDescriptions[i] = curPacket;				
			outPacketDescriptions[i].mStartOffset = curPacket.mStartOffset - firstPacketOffset;
		}
	}

    return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::WritePackets(	
									Boolean								inUseCache,
                                    UInt32								inNumBytes,
                                    const AudioStreamPacketDescription	*inPacketDescriptions,
                                    SInt64								inStartingPacket, 
                                    UInt32								*ioNumPackets, 
                                    const void							*inBuffer)
{
	// This only works with CBR. To suppport VBR you must override.
    OSStatus		err = noErr;
    
    FailWithAction(inStartingPacket > GetNumPackets(), err = kAudioFileInvalidPacketOffsetError, Bail, "write past end");
    FailWithAction((ioNumPackets == NULL) || (inBuffer == NULL), err = kAudioFileUnspecifiedError, Bail, "invalid parameter");
	
	{
		UInt32 byteCount = *ioNumPackets * mDataFormat.mBytesPerPacket;
		SInt64 startingByte = inStartingPacket * mDataFormat.mBytesPerPacket;

		err = WriteBytes(inUseCache, startingByte, &byteCount, inBuffer);
		FailIf (err != noErr, Bail, "Write Bytes Failed");

		if (byteCount != (*ioNumPackets * mDataFormat.mBytesPerPacket))
			*ioNumPackets = byteCount / mDataFormat.mBytesPerPacket;
	}
Bail:
    return (err);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetBitRate(			UInt32					*outBitRate)
{
	
	if (!outBitRate) return kAudioFileUnspecifiedError;
    
	UInt32 bytesPerPacket = GetDataFormat().mBytesPerPacket;
	UInt32 framesPerPacket = GetDataFormat().mFramesPerPacket;
	Float64 sampleRate = GetDataFormat().mSampleRate;
	const Float64 bitsPerByte = 8.;
	
	if (bytesPerPacket && framesPerPacket) {
		*outBitRate = (UInt32)(bitsPerByte * (Float64)bytesPerPacket * sampleRate / (Float64)framesPerPacket);
	} else {		
		SInt64 numPackets = GetNumPackets();
		SInt64 numBytes = GetNumBytes();
		SInt64 numFrames = 0;
		
		if (framesPerPacket) {
			numFrames = numPackets * framesPerPacket;
		} else {
			// count frames
			CompressedPacketTable* packetTable = GetPacketTable();
			if (packetTable) {
                if (packetTable->size() != numPackets) {
                    return kAudioFileInvalidFileError;
                }
#if !TARGET_OS_WIN32
				for (ssize_t i = 0; i < numPackets; i++)
#else
				for (int i = 0; i < numPackets; i++)
#endif
				{
					numFrames += (*packetTable)[i].mVariableFramesInPacket;
				}		
			} else {
				return kAudioFileUnsupportedPropertyError;
			}
		}
        
        if (numFrames == 0 || (sampleRate == 0.)) {
            *outBitRate = 0;
            return noErr;
        }
		Float64 duration = (Float64)numFrames / sampleRate;
		*outBitRate = (UInt32)(bitsPerByte * (Float64)numBytes / duration);
	}

	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetMagicCookieDataSize(
												UInt32					*outDataSize,
												UInt32					*isWritable)
{
	if (outDataSize)  *outDataSize = 0;
	if (isWritable)  *isWritable = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetMagicCookieData(
												UInt32					*ioDataSize,
												void					*ioPropertyData)
{
	*ioDataSize = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetMagicCookieData(	UInt32					/*inDataSize*/,
												const void				*inPropertyData)
{
	return kAudioFileInvalidChunkError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetMarkerListSize(
												UInt32					*outDataSize,
												UInt32					*isWritable)
{
	if (outDataSize) *outDataSize = 0;
	if (isWritable) *isWritable = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetMarkerList(
												UInt32					*ioDataSize,
												AudioFileMarkerList*	/*ioPropertyData*/)
{
	*ioDataSize = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetMarkerList(	UInt32						/*inDataSize*/,
											const AudioFileMarkerList*  /*inPropertyData*/)
{
	return kAudioFileUnsupportedPropertyError;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetRegionListSize(
												UInt32					*outDataSize,
												UInt32					*isWritable)
{
	if (outDataSize) *outDataSize = 0;
	if (isWritable) *isWritable = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetRegionList(
												UInt32					*ioDataSize,
												AudioFileRegionList		*ioPropertyData)
{
	*ioDataSize = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetRegionList(	UInt32						/*inDataSize*/,
											const AudioFileRegionList*   /*inPropertyData*/)
{
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetChannelLayoutSize(
												UInt32					*outDataSize,
												UInt32					*isWritable)
{
	if (outDataSize) *outDataSize = 0;
	if (isWritable) *isWritable = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetChannelLayout(
												UInt32						*ioDataSize,
												AudioChannelLayout*			/*ioPropertyData*/)
{
	*ioDataSize = 0;
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetChannelLayout(	UInt32						/*inDataSize*/,
											const AudioChannelLayout*   /*inPropertyData*/)
{
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetInfoDictionarySize(		UInt32						*outDataSize,
									UInt32						*isWritable)
{
	if (outDataSize) *outDataSize = sizeof(CFDictionaryRef);
	if (isWritable) *isWritable = 0;
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
												
OSStatus AudioFileObject::GetInfoDictionary(CACFDictionary  *infoDict)
{	
	Float64 fl;
	if (GetEstimatedDuration(&fl) == noErr)
		return AddDurationToInfoDictionary(infoDict, fl);
		
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
												
OSStatus AudioFileObject::SetInfoDictionary(CACFDictionary *infoDict)
{
	return kAudioFileUnsupportedPropertyError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OSStatus AudioFileObject::GetEstimatedDuration(Float64*		duration)
{
	// calculate duration
	AudioStreamBasicDescription		ASBD = GetDataFormat();
		
	*duration  = (ASBD.mFramesPerPacket != 0) ? (GetNumPackets() * ASBD.mFramesPerPacket) / ASBD.mSampleRate : 0.0;
	
	/*
		For now, assume that any ASBD that has zero in the frames per packet field has been subclassed for this
		method. i.e. A CAF file has a frame count in one of it's chunks.
		
		MP3 has been subclassed because it guesstimates a duration so the entire file does not 
		need to be parsed in order to calculate the total frames.
	*/
	
	return noErr;

}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetPropertyInfo	(	
                                        AudioFilePropertyID		inPropertyID,
                                        UInt32					*outDataSize,
                                        UInt32					*isWritable)
{
    OSStatus		err = noErr;
    UInt32			writable = 0;
        
    switch (inPropertyID)
    {
		case kAudioFilePropertyDeferSizeUpdates :
            if (outDataSize) *outDataSize = sizeof(UInt32);
            writable = 1;
            break;

        case kAudioFilePropertyFileFormat:
            if (outDataSize) *outDataSize = sizeof(UInt32);
            writable = 0;
            break;

        case kAudioFilePropertyDataFormat:
            if (outDataSize) *outDataSize = sizeof(AudioStreamBasicDescription);
            writable = 1;
            break;
                        
		case kAudioFilePropertyFormatList:
			err = GetFormatListInfo(*outDataSize, writable);
            break;
		
		case kAudioFilePropertyPacketSizeUpperBound:
        case kAudioFilePropertyIsOptimized:
        case kAudioFilePropertyMaximumPacketSize:
            if (outDataSize) *outDataSize = sizeof(UInt32);
            writable = 0;
            break;

        case kAudioFilePropertyAudioDataByteCount:
        case kAudioFilePropertyAudioDataPacketCount:
            writable = 1;
            if (outDataSize) *outDataSize = sizeof(SInt64);
            break;

		case kAudioFilePropertyDataOffset:
            writable = 0;
            if (outDataSize) *outDataSize = sizeof(SInt64);
            break;

		case kAudioFilePropertyBitRate:            
            writable = 0;
            if (outDataSize) *outDataSize = sizeof(UInt32);
			break;

		case kAudioFilePropertyMagicCookieData:            
            err = GetMagicCookieDataSize(outDataSize, &writable);
			break;

		case kAudioFilePropertyMarkerList :
            err = GetMarkerListSize(outDataSize, &writable);			
			break;
			
		case kAudioFilePropertyRegionList :
            err = GetRegionListSize(outDataSize, &writable);			
			break;
			
		case kAudioFilePropertyChannelLayout :
            err = GetChannelLayoutSize(outDataSize, &writable);			
			break;

		case kAudioFilePropertyPacketToFrame :
		case kAudioFilePropertyFrameToPacket :
            if (outDataSize) *outDataSize = sizeof(AudioFramePacketTranslation);
            writable = 0;
			break;

		case kAudioFilePropertyPacketToByte :
		case kAudioFilePropertyByteToPacket :
            if (outDataSize) *outDataSize = sizeof(AudioBytePacketTranslation);
            writable = 0;
			break;

        case kAudioFilePropertyInfoDictionary :
            err = GetInfoDictionarySize(outDataSize, &writable);			
            break;

        case kTEMPAudioFilePropertySoundCheckDictionary :
            err = GetSoundCheckDictionarySize(outDataSize, &writable);			
            break;

		case kTEMPAudioFilePropertyGenerateLoudnessInfo :
            err = GetLoudnessInfoSize(outDataSize, &writable);
            writable = 0;
            break;

		case kTEMPAudioFilePropertyLoudnessInfo :
            err = GetLoudnessInfoSize(outDataSize, &writable);
            break;

		case kAudioFilePropertyEstimatedDuration :
            if (outDataSize) *outDataSize = sizeof(Float64);
            writable = 0;
			break;

		case 'LYRC':
			if (outDataSize) *outDataSize = sizeof(CFStringRef);
			if (isWritable) *isWritable = 0;
			break;

		case 'eof?':
			if (outDataSize) *outDataSize = sizeof(UInt32);
			if (isWritable) *isWritable = 0;
			break;

		case 'sbtd' /*kAudioFilePropertySourceBitDepth*/ :
			if (outDataSize) *outDataSize = sizeof(SInt32);
			if (isWritable) *isWritable = CanWrite();
			break;
		
        default:
            writable = 0;
            err = kAudioFileUnsupportedPropertyError;
            break;
    }

    if (isWritable)
        *isWritable = writable;
    return (err);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus	AudioFileObject::GetProperty(
                                    AudioFilePropertyID		inPropertyID,
                                    UInt32					*ioDataSize,
                                    void					*ioPropertyData)
{
    OSStatus		err = noErr;
	UInt32			neededSize;
    UInt32			writable;
	
    switch (inPropertyID)
    {
        case kAudioFilePropertyFileFormat:
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
				
            *(UInt32 *) ioPropertyData = GetFileType();
            break;

		case kAudioFilePropertyFormatList:
			err = GetFormatList(*ioDataSize, (AudioFormatListItem*)ioPropertyData);
            break;
		
        case kAudioFilePropertyDataFormat:
            FailWithAction(*ioDataSize != sizeof(AudioStreamBasicDescription), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
				
            memcpy(ioPropertyData, &mDataFormat, sizeof(AudioStreamBasicDescription));
            break;
		case kAudioFilePropertyDataOffset:
            FailWithAction(*ioDataSize != sizeof(SInt64), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(SInt64 *) ioPropertyData = mDataOffset;
			break;
        case kAudioFilePropertyIsOptimized:
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(UInt32 *) ioPropertyData = mIsOptimized;
            break;

        case kAudioFilePropertyAudioDataByteCount:
            FailWithAction(*ioDataSize != sizeof(SInt64), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(SInt64 *)ioPropertyData = GetNumBytes();
            break;

        case kAudioFilePropertyAudioDataPacketCount:
            FailWithAction(*ioDataSize != sizeof(SInt64), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(SInt64 *)ioPropertyData = GetNumPackets();
            break;

		case kAudioFilePropertyPacketSizeUpperBound:
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(UInt32 *)ioPropertyData = GetPacketSizeUpperBound();
            break;
			
        case kAudioFilePropertyMaximumPacketSize:
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            *(UInt32 *)ioPropertyData = FindMaximumPacketSize();
            break;


         case kAudioFilePropertyBitRate:            
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
			err = GetBitRate((UInt32*)ioPropertyData);
            break;

         case kAudioFilePropertyMagicCookieData:            
            
			err = GetMagicCookieData(ioDataSize, ioPropertyData);
            break;

		case kAudioFilePropertyMarkerList :
            err = GetMarkerList(ioDataSize, static_cast<AudioFileMarkerList*>(ioPropertyData));			
			break;
			
		case kAudioFilePropertyRegionList :
			memset(ioPropertyData, 0, *ioDataSize);
            err = GetRegionList(ioDataSize, static_cast<AudioFileRegionList*>(ioPropertyData));			
			break;
			
		case kAudioFilePropertyChannelLayout :
			err = GetChannelLayoutSize(&neededSize, &writable);
            FailIf(err, Bail, "GetChannelLayoutSize failed");
            FailWithAction(*ioDataSize != neededSize, err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
			
            err = GetChannelLayout(ioDataSize, static_cast<AudioChannelLayout*>(ioPropertyData));			
			break;
						
		case kAudioFilePropertyDeferSizeUpdates :
            FailWithAction(*ioDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
				
            *(UInt32 *) ioPropertyData = DeferSizeUpdates();
            break;

		case kAudioFilePropertyPacketToFrame : 
		{
            FailWithAction(*ioDataSize != sizeof(AudioFramePacketTranslation), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
			
			AudioFramePacketTranslation* afpt = (AudioFramePacketTranslation*)ioPropertyData;
			err = PacketToFrame(afpt->mPacket, afpt->mFrame);
			break;
		}	
		case kAudioFilePropertyFrameToPacket :
		{
            FailWithAction(*ioDataSize != sizeof(AudioFramePacketTranslation), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			AudioFramePacketTranslation* afpt = (AudioFramePacketTranslation*)ioPropertyData;
			err = FrameToPacket(afpt->mFrame, afpt->mPacket, afpt->mFrameOffsetInPacket);
			break;
		}

		case kAudioFilePropertyPacketToByte : 
		{
            FailWithAction(*ioDataSize != sizeof(AudioBytePacketTranslation), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
			
			AudioBytePacketTranslation* abpt = (AudioBytePacketTranslation*)ioPropertyData;
			err = PacketToByte(abpt);
			break;
		}	
		case kAudioFilePropertyByteToPacket :
		{
            FailWithAction(*ioDataSize != sizeof(AudioBytePacketTranslation), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			AudioBytePacketTranslation* abpt = (AudioBytePacketTranslation*)ioPropertyData;
			err = ByteToPacket(abpt);
			break;
		}

        case kAudioFilePropertyInfoDictionary :
		{
            FailWithAction(*ioDataSize != sizeof(CFDictionaryRef), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			CACFDictionary		afInfoDictionary(true);

            err = GetInfoDictionary(&afInfoDictionary);			
            
			if (!err)
			{
				*(CFMutableDictionaryRef *)ioPropertyData = afInfoDictionary.CopyCFMutableDictionary();
			}
            break;
		}

        case kTEMPAudioFilePropertySoundCheckDictionary :
		{
            FailWithAction(*ioDataSize != sizeof(CFDictionaryRef), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			CACFDictionary		afInfoDictionary(true);

            err = GetSoundCheckDictionary(&afInfoDictionary);
            if (err) {
				OSStatus err2 = GetSoundCheckDictionaryFromLoudnessInfo(&afInfoDictionary);
				if (err2 == noErr) err = noErr; // else report original error from GetSoundCheckDictionary.
			}
            
			if (!err)
			{
				*(CFMutableDictionaryRef *)ioPropertyData = afInfoDictionary.CopyCFMutableDictionary();
			}
            break;
		}

        case kTEMPAudioFilePropertyLoudnessInfo :
		{
            FailWithAction(*ioDataSize != sizeof(CFDictionaryRef), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			CACFDictionary		afInfoDictionary(true);

            err = GetLoudnessInfo(&afInfoDictionary);
            if (err) {
				OSStatus err2 = GetLoudnessInfoFromSoundCheckDictionary(&afInfoDictionary);
				if (err2 == noErr) err = noErr; // else report original error from GetLoudnessInfo.
			}
			
			if (!err) 
			{
				*(CFMutableDictionaryRef *)ioPropertyData = afInfoDictionary.CopyCFMutableDictionary();
			}
            break;
		}

		case kTEMPAudioFilePropertyGenerateLoudnessInfo :
		{
            FailWithAction(*ioDataSize != sizeof(CFDictionaryRef),
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			CACFDictionary		afInfoDictionary(true);

            err = GenerateLoudnessInfo(&afInfoDictionary);			
            
			if (!err)
			{
				*(CFMutableDictionaryRef *)ioPropertyData = afInfoDictionary.CopyCFMutableDictionary();
			}
            break;
		}

		case kAudioFilePropertyEstimatedDuration :
		{
            FailWithAction(*ioDataSize != sizeof(Float64), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
		
			err = GetEstimatedDuration((Float64*)ioPropertyData); 
			break;
		}

        case 'LYRC' :
            if (*ioDataSize < sizeof(CFStringRef))
                return kAudioFileBadPropertySizeError;

			*ioDataSize = sizeof(CFStringRef);
			err = GetLyrics((CFStringRef*) ioPropertyData);
			break;
	
		case 'eof?' : 
		{
            if (*ioDataSize != sizeof(UInt32))
                return kAudioFileBadPropertySizeError;
			
			SInt64 pos;
			err = GetDataSource()->GetPos(pos);
			if (err) break;
			
			SInt64 endOfData = GetDataOffset() + GetNumBytes();
			*(UInt32*)ioPropertyData = pos >= endOfData;
			
			break;
		}	
		case 'sbtd' /*kAudioFilePropertySourceBitDepth*/ :
		{
            if (*ioDataSize != sizeof(SInt32))
                return kAudioFileBadPropertySizeError;

			SInt32 outValue = 0;
			err = GetSourceBitDepth(outValue);
			if ((err || outValue == 0) && GetDataFormat().mFormatID == kAudioFormatLinearPCM) {
				// if there was no stored source bit depth, and this file is LPCM, then report this file's bit depth.
				err = noErr;
				outValue = GetDataFormat().mBitsPerChannel;
				if (GetDataFormat().mFormatFlags & kAudioFormatFlagIsFloat) 
					outValue = -outValue;
			} else if (err) 
				break;
			
			*(SInt32 *) ioPropertyData = outValue;
			
			break;
		}	
		default:
            err = kAudioFileUnsupportedPropertyError;			
            break;
    }

Bail:
    return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus	AudioFileObject::SetProperty(
                                    AudioFilePropertyID		inPropertyID,
                                    UInt32					inDataSize,
                                    const void				*inPropertyData)
{
    OSStatus		err = noErr;

    switch (inPropertyID)
    {
        case kAudioFilePropertyDataFormat:
            FailWithAction(inDataSize != sizeof(AudioStreamBasicDescription), 
				err = kAudioFileBadPropertySizeError, Bail, "Incorrect data size");
            err = UpdateDataFormat((AudioStreamBasicDescription *) inPropertyData);
			break;
		case kAudioFilePropertyFormatList:
			err = SetFormatList(inDataSize, (AudioFormatListItem*)inPropertyData);
            break;
		
        case kAudioFilePropertyAudioDataByteCount: {
            FailWithAction(inDataSize != sizeof(SInt64), err = kAudioFileBadPropertySizeError, Bail, "Incorrect data size");
            SInt64 numBytes = *(SInt64 *) inPropertyData;
			if (numBytes > GetNumBytes()) {
				// can't use this to increase data size.
				return kAudioFileOperationNotSupportedError;
			}
			UInt32 saveDefer = DeferSizeUpdates();
			SetDeferSizeUpdates(0); // force an update.
			err = UpdateNumBytes(numBytes);
			SetDeferSizeUpdates(saveDefer);
		} break;

		case kAudioFilePropertyAudioDataPacketCount: {
			SInt64 numPackets = *(SInt64 *) inPropertyData;
			if (numPackets > GetNumPackets()) {
				// can't use this to increase data size.
				return kAudioFileOperationNotSupportedError;
			}
			err = UpdateNumPackets(numPackets);
		} break;
		
		case kAudioFilePropertyMagicCookieData:            
			err = SetMagicCookieData(inDataSize, inPropertyData);
			break;


		case kAudioFilePropertyMarkerList :
            err = SetMarkerList(inDataSize, static_cast<const AudioFileMarkerList*>(inPropertyData));			
			break;
			
		case kAudioFilePropertyRegionList :
            err = SetRegionList(inDataSize, static_cast<const AudioFileRegionList*>(inPropertyData));			
			break;
			
		case kAudioFilePropertyChannelLayout :
            err = SetChannelLayout(inDataSize, static_cast<const AudioChannelLayout*>(inPropertyData));			
			break;

		case kAudioFilePropertyDeferSizeUpdates :
            FailWithAction(inDataSize != sizeof(UInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");
            SetDeferSizeUpdates(*(UInt32 *) inPropertyData);
            break;

        case kAudioFilePropertyInfoDictionary :
		{
            FailWithAction(inDataSize != sizeof(CFDictionaryRef), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			// pass the SetInfoDictionary a CACFDictionary object made with the provided CFDictionaryRef
			// Let the caller release their own CFObject so pass false for th erelease parameter
			CACFDictionary		afInfoDictionary(*(CFDictionaryRef *)inPropertyData, false);
            err = SetInfoDictionary(&afInfoDictionary);			
            
            break;
		}
			
        case kTEMPAudioFilePropertySoundCheckDictionary :
		{
            FailWithAction(inDataSize != sizeof(CFDictionaryRef), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			// pass the SetInfoDictionary a CACFDictionary object made with the provided CFDictionaryRef
			// Let the caller release their own CFObject so pass false for the release parameter
			CACFDictionary		afInfoDictionary(*(CFDictionaryRef *)inPropertyData, false);
            err = SetSoundCheckDictionary(&afInfoDictionary);			
            
            break;
		}

        case kTEMPAudioFilePropertyLoudnessInfo :
		{
            FailWithAction(inDataSize != sizeof(CFDictionaryRef), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			// pass the SetInfoDictionary a CACFDictionary object made with the provided CFDictionaryRef
			// Let the caller release their own CFObject so pass false for the release parameter
			CACFDictionary		afInfoDictionary(*(CFDictionaryRef *)inPropertyData, false);
            err = SetLoudnessInfo(&afInfoDictionary);			
            
            break;
		}
			
  		case 'sbtd' /*kAudioFilePropertySourceBitDepth*/ :
		{
            FailWithAction(inDataSize != sizeof(SInt32), 
				err = kAudioFileBadPropertySizeError, Bail, "inDataSize is wrong");

			SInt32 inValue = *(SInt32 *)inPropertyData;
			err = SetSourceBitDepth(inValue);
			
			break;
		}	

      default:
            err = kAudioFileUnsupportedPropertyError;			
		break;
    }

Bail:
    return err;
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetDataFormat(const AudioStreamBasicDescription* inStreamFormat)
{
	OSStatus err = noErr;
	
	if (!IsDataFormatValid(inStreamFormat))
		return kAudioFileUnsupportedDataFormatError;

	if (!IsDataFormatSupported(inStreamFormat)) 
		return kAudioFileUnsupportedDataFormatError;
	
	UInt32 prevBytesPerPacket = mDataFormat.mBytesPerPacket;
	
	mDataFormat = *inStreamFormat;
	
	// if CBR and bytes per packet changes, we need to change the number of packets we think we have.
	if (mDataFormat.mBytesPerPacket && mDataFormat.mBytesPerPacket != prevBytesPerPacket)
	{
		SInt64 numPackets = GetNumBytes() / mDataFormat.mBytesPerPacket;
		SetNumPackets(numPackets);
		SetMaximumPacketSize(mDataFormat.mBytesPerPacket);

		if (!mFirstSetFormat)
			SizeChanged();
	}
	
	mFirstSetFormat = false;
	
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetFormatListInfo(	UInt32				&outDataSize,
													UInt32				&outWritable)
{
	// default implementation is to just return the data format
	outDataSize = sizeof(AudioFormatListItem);
	outWritable = false;
	return noErr;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::GetFormatList(	UInt32									&ioDataSize,
												AudioFormatListItem						*ioPropertyData)
{
	// default implementation is to just return the data format
	if (ioDataSize < sizeof(AudioFormatListItem))
		return kAudioFileBadPropertySizeError;
	
	AudioFormatListItem afli;
	afli.mASBD = mDataFormat;
	AudioChannelLayoutTag layoutTag = /*kAudioChannelLayoutTag_Unknown*/ 0xFFFF0000 | mDataFormat.mChannelsPerFrame;
	UInt32 layoutSize, isWritable;
	OSStatus err = GetChannelLayoutSize(&layoutSize, &isWritable);
	if (err == noErr)
	{
		CAAutoFree<AudioChannelLayout> layout;
		layout.allocBytes(layoutSize);
		err = GetChannelLayout(&layoutSize, layout());
		if (err == noErr) {
			layoutTag = layout->mChannelLayoutTag;
		}
	}
	afli.mChannelLayoutTag = layoutTag;	
	
	memcpy(ioPropertyData, &afli, sizeof(AudioFormatListItem));
	
	ioDataSize = sizeof(AudioFormatListItem);
	return noErr;
}
										
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::SetFormatList(	UInt32									inDataSize,
											const AudioFormatListItem				*inPropertyData)
{
	return kAudioFileOperationNotSupportedError;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

OSStatus AudioFileObject::UpdateDataFormat(const AudioStreamBasicDescription* inStreamFormat)
{
	return SetDataFormat(inStreamFormat);
}


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Boolean AudioFileObject::IsDataFormatValid(AudioStreamBasicDescription const* inDesc)
{
	if (inDesc->mSampleRate < 0.)
		return false;

	if (inDesc->mSampleRate > 3e6)
		return false;

	if (inDesc->mChannelsPerFrame < 1 || inDesc->mChannelsPerFrame > 0x000FFFFF)
		return false;

	if (inDesc->mFormatID == kAudioFormatLinearPCM)
	{			
		if (inDesc->mBitsPerChannel < 1 || inDesc->mBitsPerChannel > 64)
			return false;
			
		if (inDesc->mFramesPerPacket != 1)
			return false;
			
		if (inDesc->mBytesPerPacket == 0) 
			return false;

		if (inDesc->mBytesPerFrame != inDesc->mBytesPerPacket)
			return false;
			
		// [3605260] we assume here that a packet is an integer number of frames.
		UInt32 minimumBytesPerPacket = (inDesc->mBitsPerChannel * inDesc->mChannelsPerFrame + 7) / 8;
		if (inDesc->mBytesPerPacket < minimumBytesPerPacket) 
			return false;
	}
	return true;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void AudioFileObject::SetDataSource(DataSource* inDataSource)	
{
	if (mDataSource != inDataSource) {
		delete mDataSource;
		mDataSource = inDataSource;
	}
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

void	AudioFileObject::SetURL (CFURLRef inURL)
{
	if (mFileRef == inURL) return;
	if (inURL) CFRetain (inURL);
	if (mFileRef) CFRelease(mFileRef);
	mFileRef = inURL;
}

OSStatus AudioFileObject::OpenFile(SInt8 inPermissions, int inFD)
{
	OSStatus err = noErr;

	SetDataSource(new Cached_DataSource(new UnixFile_DataSource(inFD, inPermissions, true)));
	
	mFileD = inFD;
	SetPermissions (inPermissions);

	return err;
}
								
OSStatus AudioFileObject::CreateDataFile (CFURLRef	inFileRef, int	&outFileD)
{	 
	UInt8 fPath[FILENAME_MAX];	
	if (!CFURLGetFileSystemRepresentation (inFileRef, true, fPath, FILENAME_MAX))
		return kAudio_FileNotFoundError;
	
	struct stat stbuf;
	if (stat ((const char*)fPath, &stbuf) == 0) 
		return kAudioFilePermissionsError;

#if TARGET_OS_WIN32
	int filePerms = S_IREAD | S_IWRITE;
	int flags = O_CREAT | O_EXCL | O_RDWR | O_BINARY;
#else
	mode_t filePerms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	int flags = O_CREAT | O_EXCL | O_RDWR;
#endif
	outFileD = open((const char*)fPath, flags, filePerms);
	if (outFileD < 0)
		return AudioFileTranslateErrno(errno);

	return noErr;
}

OSStatus AudioFileObject::AddDurationToInfoDictionary(CACFDictionary *infoDict, Float64 &inDuration)
{
#if !TARGET_OS_WIN32
	CFLocaleRef currentLocale = CFLocaleGetSystem();
	CFNumberFormatterRef numberFormatter = NULL;
	numberFormatter = CFNumberFormatterCreate(kCFAllocatorDefault, currentLocale, kCFNumberFormatterDecimalStyle);
	CFStringRef cfStr = CFNumberFormatterCreateStringWithValue( kCFAllocatorDefault, numberFormatter, kCFNumberFloat64Type, &inDuration);
	if (cfStr)
	{
		if (CFStringGetLength(cfStr) != 0)
			infoDict->AddString(CFSTR(kAFInfoDictionary_ApproximateDurationInSeconds), cfStr);
		CFRelease(cfStr);
	}
	CFRelease(numberFormatter);
#endif
	return noErr;
}

OSStatus AudioFileObject::SizeChanged()
{
	OSStatus err = noErr;
	if (mPermissions & kAudioFileWritePermission) 
	{
		if (DeferSizeUpdates())
			SetNeedsSizeUpdate(true);
		else
			err = UpdateSize();
	}
	return err;
}

OSStatus AudioFileObject::UpdateSizeIfNeeded()
{		
	if (GetNeedsSizeUpdate()) 
	{
		OSStatus err = UpdateSize();
		if (err) return err;
		SetNeedsSizeUpdate(false);
	}
	return noErr;
}

OSStatus AudioFileObject::CountUserData(	UInt32					/*inUserDataID*/,
											UInt32*					/*outNumberItems*/)
{
	return kAudioFileOperationNotSupportedError;
}

OSStatus AudioFileObject::GetUserDataSize(  UInt32					/*inUserDataID*/,
											UInt32					/*inIndex*/,
											UInt32*					/*outDataSize*/)
{
	return kAudioFileOperationNotSupportedError;
}
											
OSStatus AudioFileObject::GetUserData(		UInt32					/*inUserDataID*/,
											UInt32					/*inIndex*/,
											UInt32*					/*ioDataSize*/,
											void*					/*ioUserData*/)
{
	return kAudioFileOperationNotSupportedError;
}
											
OSStatus AudioFileObject::SetUserData(		UInt32					/*inUserDataID*/,
											UInt32					/*inIndex*/,
											UInt32					/*inDataSize*/,
											const void*				/*inUserData*/)
{
	return kAudioFileOperationNotSupportedError;
}
											
OSStatus AudioFileObject::RemoveUserData(	UInt32					/*inUserDataID*/,
											UInt32					/*inIndex*/)
{
	return kAudioFileOperationNotSupportedError;
}

OSStatus AudioFileObject::MoveData(SInt64 fromPos, SInt64 toPos, SInt64 size)
{
	if (fromPos == toPos) 
		return noErr;

	OSStatus err = noErr;
	CAAutoFree<char> audioData(kCopySoundDataBufferSize, true);

	SInt64 bytesRemaining = size;
	if (fromPos < toPos) {
		while (bytesRemaining > 0)
		{
			// read from old file
			UInt32 byteCount;
			SInt64 count = (bytesRemaining < kCopySoundDataBufferSize) ? bytesRemaining : kCopySoundDataBufferSize; 
			err = GetDataSource()->ReadBytes(SEEK_SET, fromPos+(bytesRemaining-count), (UInt32)count, audioData(), &byteCount);
			FailIf (err != noErr, Bail, "MoveData ReadBytes failed");

			err = GetDataSource()->WriteBytes(SEEK_SET, toPos+(bytesRemaining-count), (UInt32)count, audioData(), &byteCount);
			FailIf (err != noErr, Bail, "WriteBytes failed");
			
			bytesRemaining -= count;
		}
	} else {
		while (bytesRemaining > 0)
		{
			// read from old file
			UInt32 byteCount;
			SInt64 count = (bytesRemaining < kCopySoundDataBufferSize) ? bytesRemaining : kCopySoundDataBufferSize; 
			err = GetDataSource()->ReadBytes(SEEK_SET, fromPos+(size - bytesRemaining), (UInt32)count, audioData(), &byteCount);
			FailIf (err != noErr, Bail, "MoveData ReadBytes failed");
			
			err = GetDataSource()->WriteBytes(SEEK_SET, toPos+(size - bytesRemaining), (UInt32)count, audioData(), &byteCount);
			FailIf (err != noErr, Bail, "WriteBytes failed");
			
			bytesRemaining -= count;
		}
	}
	
Bail:
	return err;
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


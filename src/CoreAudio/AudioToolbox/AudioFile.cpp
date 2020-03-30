/*
This file is part of Darling.

Copyright (C) 2020 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <AudioToolbox/AudioFile.h>
#include <CoreServices/MacErrors.h>

OSStatus AudioFileCreateWithURL (CFURLRef inFileRef,
	AudioFileTypeID inFileType,
	const AudioStreamBasicDescription *inFormat,
	AudioFileFlags inFlags,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	return unimpErr;
}

OSStatus AudioFileOpenURL ( CFURLRef inFileRef,
	AudioFilePermissions inPermissions,
	AudioFileTypeID inFileTypeHint,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	return unimpErr;
}

OSStatus AudioFileInitializeWithCallbacks (
	void * inClientData,
	AudioFile_ReadProc inReadFunc,
	AudioFile_WriteProc inWriteFunc,
	AudioFile_GetSizeProc inGetSizeFunc,
	AudioFile_SetSizeProc inSetSizeFunc,
	AudioFileTypeID inFileType,
	const AudioStreamBasicDescription *inFormat,
	AudioFileFlags inFlags,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	return unimpErr;
}

OSStatus AudioFileOpenWithCallbacks (
	void * inClientData,
	AudioFile_ReadProc inReadFunc,
	AudioFile_WriteProc _Nullable inWriteFunc,
	AudioFile_GetSizeProc inGetSizeFunc,
	AudioFile_SetSizeProc _Nullable inSetSizeFunc,
	AudioFileTypeID inFileTypeHint,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	return unimpErr;
}

OSStatus AudioFileClose (AudioFileID inAudioFile)
{
	return unimpErr;
}

OSStatus AudioFileOptimize (AudioFileID inAudioFile)
{
	return unimpErr;
}

OSStatus AudioFileReadBytes ( AudioFileID inAudioFile,
	Boolean inUseCache,
	SInt64 inStartingByte,
	UInt32 *ioNumBytes,
	void *outBuffer)
{
	return unimpErr;
}

OSStatus AudioFileWriteBytes ( AudioFileID inAudioFile,
	Boolean inUseCache,
	SInt64 inStartingByte,
	UInt32 *ioNumBytes,
	const void *inBuffer)
{
	return unimpErr;
}

OSStatus AudioFileReadPacketData ( AudioFileID inAudioFile,
	Boolean inUseCache,
	UInt32 * ioNumBytes,
	AudioStreamPacketDescription * _Nullable outPacketDescriptions,
	SInt64 inStartingPacket,
	UInt32 * ioNumPackets,
	void * _Nullable outBuffer)
{
	return unimpErr;
}

OSStatus AudioFileReadPackets ( AudioFileID inAudioFile,
	Boolean inUseCache,
	UInt32 * outNumBytes,
	AudioStreamPacketDescription * _Nullable outPacketDescriptions,
	SInt64 inStartingPacket,
	UInt32 * ioNumPackets,
	void * _Nullable outBuffer)
{
	return unimpErr;
}

OSStatus AudioFileWritePackets ( AudioFileID inAudioFile,
	Boolean inUseCache,
	UInt32 inNumBytes,
	const AudioStreamPacketDescription * _Nullable inPacketDescriptions,
	SInt64 inStartingPacket,
	UInt32 *ioNumPackets,
	const void *inBuffer)
{
	return unimpErr;
}

OSStatus AudioFileCountUserData ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 *outNumberItems)
{
	return unimpErr;
}

OSStatus AudioFileGetUserDataSize ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 inIndex,
	UInt32 *outUserDataSize)
{
	return unimpErr;
}

OSStatus AudioFileGetUserData ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 inIndex,
	UInt32 *ioUserDataSize,
	void *outUserData)
{
	return unimpErr;
}

OSStatus AudioFileSetUserData ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 inIndex,
	UInt32 inUserDataSize,
	const void *inUserData)
{
	return unimpErr;
}

OSStatus AudioFileRemoveUserData ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 inIndex)
{
	return unimpErr;
}

OSStatus AudioFileGetPropertyInfo( AudioFileID inAudioFile,
	AudioFilePropertyID inPropertyID,
	UInt32 * _Nullable outDataSize,
	UInt32 * _Nullable isWritable)
{
	return unimpErr;
}

OSStatus AudioFileGetProperty( AudioFileID inAudioFile,
	AudioFilePropertyID inPropertyID,
	UInt32 *ioDataSize,
	void *outPropertyData)
{
	return unimpErr;
}

OSStatus AudioFileSetProperty( AudioFileID inAudioFile,
	AudioFilePropertyID inPropertyID,
	UInt32 inDataSize,
	const void *inPropertyData)
{
	return unimpErr;
}

OSStatus AudioFileGetGlobalInfoSize( AudioFilePropertyID inPropertyID,
	UInt32 inSpecifierSize,
	void * _Nullable inSpecifier,
	UInt32 *outDataSize)
{
	return unimpErr;
}

OSStatus AudioFileGetGlobalInfo( AudioFilePropertyID inPropertyID,
	UInt32 inSpecifierSize,
	void * _Nullable inSpecifier,
	UInt32 *ioDataSize,
	void *outPropertyData)
{
	return unimpErr;
}

OSStatus AudioFileCreate ( const struct FSRef *inParentRef,
	CFStringRef inFileName,
	AudioFileTypeID inFileType,
	const AudioStreamBasicDescription *inFormat,
	AudioFileFlags inFlags,
	struct FSRef *outNewFileRef,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	return unimpErr;
}

OSStatus AudioFileInitialize ( const struct FSRef *inFileRef,
	AudioFileTypeID inFileType,
	const AudioStreamBasicDescription *inFormat,
	AudioFileFlags inFlags,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	return unimpErr;
}

OSStatus AudioFileOpen ( const struct FSRef *inFileRef,
	AudioFilePermissions inPermissions,
	AudioFileTypeID inFileTypeHint,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	return unimpErr;
}




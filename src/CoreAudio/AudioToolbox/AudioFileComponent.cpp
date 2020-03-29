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

#include <AudioToolbix/AudioFileComponent.h>
#include <CoreServices/MacErrors.h>

OSStatus
AudioFileComponentCreateURL (
       AudioFileComponent inComponent,
       CFURLRef inFileRef,
       const AudioStreamBasicDescription *inFormat,
       UInt32 inFlags)
{
	return unimpErr;
}

OSStatus
AudioFileComponentOpenURL (
     AudioFileComponent inComponent,
     CFURLRef inFileRef,
     SInt8 inPermissions,
     int inFileDescriptor)
{
	return unimpErr;
}

OSStatus
AudioFileComponentOpenWithCallbacks(
        AudioFileComponent inComponent,
        void *inClientData,
        AudioFile_ReadProc inReadFunc,
        AudioFile_WriteProc inWriteFunc,
        AudioFile_GetSizeProc inGetSizeFunc,
        AudioFile_SetSizeProc inSetSizeFunc)
{
	return unimpErr;
}

OSStatus
AudioFileComponentInitializeWithCallbacks(
        AudioFileComponent inComponent,
        void *inClientData,
        AudioFile_ReadProc inReadFunc,
        AudioFile_WriteProc inWriteFunc,
        AudioFile_GetSizeProc inGetSizeFunc,
        AudioFile_SetSizeProc inSetSizeFunc,
        UInt32 inFileType,
        const AudioStreamBasicDescription *inFormat,
        UInt32 inFlags)
{
	return unimpErr;
}

OSStatus
AudioFileComponentCloseFile(
        AudioFileComponent inComponent)
{
	return unimpErr;
}



OSStatus
AudioFileComponentOptimize(
        AudioFileComponent inComponent)
{
	return unimpErr;
}

OSStatus
AudioFileComponentReadBytes(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        SInt64 inStartingByte,
        UInt32 *ioNumBytes,
        void *outBuffer)
{
	return unimpErr;
}

OSStatus
AudioFileComponentWriteBytes(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        SInt64 inStartingByte,
        UInt32 *ioNumBytes,
        const void *inBuffer)
{
	return unimpErr;
}

OSStatus
AudioFileComponentReadPackets(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        UInt32 *outNumBytes,
        AudioStreamPacketDescription * _Nullable outPacketDescriptions,
        SInt64 inStartingPacket,
        UInt32 *ioNumPackets,
        void *outBuffer)
{
	return unimpErr;
}

OSStatus
AudioFileComponentReadPacketData(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        UInt32 *ioNumBytes,
        AudioStreamPacketDescription * _Nullable outPacketDescriptions,
        SInt64 inStartingPacket,
        UInt32 *ioNumPackets,
        void *outBuffer)
{
	return unimpErr;
}

OSStatus
AudioFileComponentWritePackets(
        AudioFileComponent inComponent,
        Boolean inUseCache,
        UInt32 inNumBytes,
        const AudioStreamPacketDescription * _Nullable inPacketDescriptions,
        SInt64 inStartingPacket,
        UInt32 *ioNumPackets,
        const void *inBuffer)
{
	return unimpErr;
}

OSStatus
AudioFileComponentGetPropertyInfo(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 * _Nullable outPropertySize,
        UInt32 * _Nullable outWritable)
{
	return unimpErr;
}

OSStatus
AudioFileComponentGetProperty(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 *ioPropertyDataSize,
        void *outPropertyData)
{
	return unimpErr;
}

OSStatus
AudioFileComponentSetProperty(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 inPropertyDataSize,
        const void *inPropertyData)
{
	return unimpErr;
}

OSStatus
AudioFileComponentCountUserData(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 *outNumberItems)
{
	return unimpErr;
}

OSStatus
AudioFileComponentGetUserDataSize(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 inIndex,
        UInt32 *outUserDataSize)
{
	return unimpErr;
}

OSStatus
AudioFileComponentGetUserData(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 inIndex,
        UInt32 *ioUserDataSize,
        void *outUserData)
{
	return unimpErr;
}

OSStatus
AudioFileComponentSetUserData(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 inIndex,
        UInt32 inUserDataSize,
        const void *inUserData)
{
	return unimpErr;
}

OSStatus
AudioFileComponentRemoveUserData(
        AudioFileComponent inComponent,
        UInt32 inUserDataID,
        UInt32 inIndex)
{
	return unimpErr;
}

OSStatus
AudioFileComponentExtensionIsThisFormat(
        AudioFileComponent inComponent,
        CFStringRef inExtension,
        UInt32 *outResult)
{
	return unimpErr;
}

OSStatus
AudioFileComponentFileDataIsThisFormat(
        AudioFileComponent inComponent,
        UInt32 inDataByteSize,
        const void* inData,
        UInt32 *outResult)
{
	return unimpErr;
}

OSStatus
AudioFileComponentFileIsThisFormat(
        AudioFileComponent inComponent,
        SInt16 inFileRefNum,
        UInt32 *outResult)
{
	return unimpErr;
}

OSStatus
AudioFileComponentDataIsThisFormat(
        AudioFileComponent inComponent,
        void * _Nullable inClientData,
        AudioFile_ReadProc _Nullable inReadFunc,
        AudioFile_WriteProc _Nullable inWriteFunc,
        AudioFile_GetSizeProc _Nullable inGetSizeFunc,
        AudioFile_SetSizeProc _Nullable inSetSizeFunc,
        UInt32 *outResult)
{
	return unimpErr;
}

OSStatus
AudioFileComponentGetGlobalInfoSize(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 inSpecifierSize,
        const void * _Nullable inSpecifier,
        UInt32 *outPropertySize)
{
	return unimpErr;
}

OSStatus
AudioFileComponentGetGlobalInfo(
        AudioFileComponent inComponent,
        AudioFileComponentPropertyID inPropertyID,
        UInt32 inSpecifierSize,
        const void * _Nullable inSpecifier,
        UInt32 *ioPropertyDataSize,
        void *outPropertyData)
{
	return unimpErr;
}


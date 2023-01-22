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
#include <CarbonCore/MacErrors.h>
#include "AudioFileFormatManager.h"

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
	return AudioFileGetGlobalInfo(inPropertyID, inSpecifierSize, inSpecifier, outDataSize, nullptr);
}

static void writeUInt32Set(const std::set<UInt32>& set, void *outPropertyData)
{
	if (outPropertyData)
	{
		UInt32* out = static_cast<UInt32*>(outPropertyData);
		int i = 0;
		for (UInt32 v : set)
			out[i++] = v;
	}
}

template <typename ContainerType>
CFArrayRef stringContainerToArray(const ContainerType& t)
{
	std::unique_ptr<CFStringRef[]> ptrs(new CFStringRef[t.size()]);
	int i = 0;

	for (const std::string& str : t)
		ptrs[i++] = CFStringCreateWithCString(nullptr, str.c_str(), kCFStringEncodingUTF8);
	
	CFArrayRef rv = CFArrayCreate(nullptr, (const void**) ptrs.get(), t.size(), &kCFTypeArrayCallBacks);

	for (int i = 0; i < t.size(); i++)
		CFRelease(ptrs[i]);
	return rv;
}

OSStatus AudioFileGetGlobalInfo( AudioFilePropertyID inPropertyID,
	UInt32 inSpecifierSize,
	void * _Nullable inSpecifier,
	UInt32 *ioDataSize,
	void *outPropertyData)
{
	switch (inPropertyID)
	{
		case kAudioFileGlobalInfo_ReadableTypes:
		case kAudioFileGlobalInfo_WritableTypes:
		{
			std::set<UInt32> types = AudioFileFormatManager::instance()->types(inPropertyID == kAudioFileGlobalInfo_WritableTypes);

			*ioDataSize = types.size() * sizeof(UInt32);
			writeUInt32Set(types, outPropertyData);
			
			return noErr;
		}
		case kAudioFileGlobalInfo_FileTypeName:
		{
			const UInt32* fileType = static_cast<const UInt32*>(inSpecifier);
			if (inSpecifierSize != sizeof(*fileType))
				return kAudioFileBadPropertySizeError;

			const AudioFileFormatManager::ComponentInfo* ci;

			ci = AudioFileFormatManager::instance()->fileType(*fileType);
			if (!ci)
				return kAudioFileUnsupportedFileTypeError;

			CFStringRef* out = static_cast<CFStringRef*>(outPropertyData);
			*ioDataSize = sizeof(*out);

			if (out)
				*out = CFStringCreateWithCString(nullptr, ci->name.c_str(), kCFStringEncodingUTF8);

			return noErr;
		}
		case kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat:
		{
			const AudioFileTypeAndFormatID* spec = static_cast<const AudioFileTypeAndFormatID*>(inSpecifier);
			if (inSpecifierSize != sizeof(*spec))
				return kAudioFileBadPropertySizeError;
			break;
		}
		case kAudioFileGlobalInfo_AvailableFormatIDs:
		{
			const UInt32* fileType = static_cast<const UInt32*>(inSpecifier);
			if (inSpecifierSize != sizeof(*fileType))
				return kAudioFileBadPropertySizeError;

			auto set = AudioFileFormatManager::instance()->availableFormatIDs(*fileType);
			*ioDataSize = set.size() * sizeof(UInt32);

			writeUInt32Set(set, outPropertyData);

			return noErr;
		}
		case kAudioFileGlobalInfo_AllHFSTypeCodes:
		case kAudioFileGlobalInfo_HFSTypeCodesForType:
		case kAudioFileGlobalInfo_TypesForHFSTypeCode:
			*ioDataSize = 0;
			return noErr;
		case kAudioFileGlobalInfo_AllExtensions:
		case kAudioFileGlobalInfo_AllUTIs:
		case kAudioFileGlobalInfo_AllMIMETypes:
		{
			*ioDataSize = sizeof(CFArrayRef);

			if (outPropertyData)
			{
				std::set<std::string> set;
				AudioFileFormatManager* mgr = AudioFileFormatManager::instance();

				if (inPropertyID == kAudioFileGlobalInfo_AllExtensions)
					set = mgr->allExtensions();
				else if (inPropertyID == kAudioFileGlobalInfo_AllUTIs)
					set = mgr->allUTIs();
				else if (inPropertyID == kAudioFileGlobalInfo_AllMIMETypes)
					set = mgr->allMIMEs();

				*((CFArrayRef*)outPropertyData) = stringContainerToArray(set);
			}

			return noErr;
		}
		case kAudioFileGlobalInfo_ExtensionsForType:
		case kAudioFileGlobalInfo_UTIsForType:
		case kAudioFileGlobalInfo_MIMETypesForType:
		{
			const UInt32* fileType = static_cast<const UInt32*>(inSpecifier);
			if (inSpecifierSize != sizeof(*fileType))
				return kAudioFileBadPropertySizeError;

			*ioDataSize = sizeof(CFArrayRef);

			if (outPropertyData)
			{
				const std::vector<std::string>* vector;
				const AudioFileFormatManager::ComponentInfo* ci;

				ci = AudioFileFormatManager::instance()->fileType(*fileType);
				if (!ci)
					return kAudioFileUnsupportedFileTypeError;
				
				if (inPropertyID == kAudioFileGlobalInfo_ExtensionsForType)
					vector = &ci->extensions;
				else if (inPropertyID == kAudioFileGlobalInfo_UTIsForType)
					vector = &ci->utis;
				else if (inPropertyID == kAudioFileGlobalInfo_MIMETypesForType)
					vector = &ci->mimeTypes;

				*((CFArrayRef*)outPropertyData) = stringContainerToArray(*vector);
			}

			return noErr;
		}
		case kAudioFileGlobalInfo_TypesForMIMEType:
		case kAudioFileGlobalInfo_TypesForUTI:
		case kAudioFileGlobalInfo_TypesForExtension:
		{
			CFStringRef str = static_cast<CFStringRef>(inSpecifier);
			if (inSpecifierSize != sizeof(str))
				return kAudioFileBadPropertySizeError;
			
			AudioFileFormatManager* mgr = AudioFileFormatManager::instance();
			const char* cstr = CFStringGetCStringPtr(str, kCFStringEncodingUTF8);
			std::set<UInt32> set;

			if (inPropertyID == kAudioFileGlobalInfo_TypesForMIMEType)
				set = mgr->typesForMIME(cstr);
			else if (inPropertyID == kAudioFileGlobalInfo_TypesForUTI)
				set = mgr->typesForUTI(cstr);
			else if (inPropertyID == kAudioFileGlobalInfo_TypesForExtension)
				set = mgr->typesForExtension(cstr);

			*ioDataSize = set.size() * sizeof(UInt32);
			writeUInt32Set(set, outPropertyData);

			return noErr;
		}
	}
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




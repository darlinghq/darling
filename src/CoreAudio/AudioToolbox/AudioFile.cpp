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
#include <AudioToolbox/AudioFileComponent.h>
#include <CarbonCore/Files.h>
#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <limits.h>
#include <string>
#include <unistd.h>
#include <vector>

// How much of a file is enough to tell what format it is
static const UInt32 PROBE_SIZE = 64 * 1024;

static OSStatus newFileComponent(AudioFileTypeID fileType, AudioFileComponent* outComponent)
{
	AudioComponentDescription description = {};
	description.componentType = 'afil';
	description.componentSubType = fileType;

	AudioComponent component = AudioComponentFindNext(nullptr, &description);
	if (!component)
		return kAudioFileUnsupportedFileTypeError;

	return AudioComponentInstanceNew(component, outComponent);
}

static OSStatus errnoToStatus(int err)
{
	switch (err)
	{
		case ENOENT:
			return fnfErr;
		case EACCES:
		case EPERM:
		case EROFS:
			return kAudioFilePermissionsError;
		default:
			return kAudioFileUnspecifiedError;
	}
}

static bool urlToPath(CFURLRef url, char* path)
{
	return url && CFURLGetFileSystemRepresentation(url, true, reinterpret_cast<UInt8*>(path), PATH_MAX);
}

// What a file that cannot be opened at all reports: no reason is given, except for permissions.
static OSStatus openErrnoToStatus(int err)
{
	return (err == EACCES || err == EPERM) ? kAudioFilePermissionsError : kAudioFileUnspecifiedError;
}

// Types to try for a file: what the caller says it is, then what its extension says,
// and then whatever recognizes its contents. The types that recognized the contents are also returned in
// `recognized`: if there are none, nothing says what the file is.
static std::vector<AudioFileTypeID> candidateTypes(AudioFileTypeID hint, CFURLRef url, const void* data, UInt32 dataSize,
	std::vector<AudioFileTypeID>& recognized)
{
	AudioFileFormatManager* manager = AudioFileFormatManager::instance();
	std::vector<AudioFileTypeID> types;

	auto add = [&](AudioFileTypeID type) {
		if (std::find(types.begin(), types.end(), type) == types.end())
			types.push_back(type);
	};

	if (hint)
		add(hint);

	if (url)
	{
		CFStringRef extension = CFURLCopyPathExtension(url);
		if (extension)
		{
			char ext[32];
			if (CFStringGetCString(extension, ext, sizeof(ext), kCFStringEncodingUTF8))
			{
				std::transform(ext, ext + strlen(ext), ext, [](unsigned char c) { return tolower(c); });
				for (AudioFileTypeID type : manager->typesForExtension(ext))
					add(type);
			}
			CFRelease(extension);
		}
	}

	if (dataSize > 0)
	{
		for (AudioFileTypeID type : manager->types(false))
		{
			AudioFileComponent component;
			if (newFileComponent(type, &component) != noErr)
				continue;

			UInt32 result = 0;
			if (AudioFileComponentFileDataIsThisFormat(component, dataSize, data, &result) == noErr && result == 1)
			{
				add(type);
				recognized.push_back(type);
			}

			AudioComponentInstanceDispose(component);
		}
	}

	return types;
}

OSStatus AudioFileCreateWithURL (CFURLRef inFileRef,
	AudioFileTypeID inFileType,
	const AudioStreamBasicDescription *inFormat,
	AudioFileFlags inFlags,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	char path[PATH_MAX];

	if (!outAudioFile || !inFormat || !urlToPath(inFileRef, path))
		return kAudio_ParamError;

	*outAudioFile = nullptr;

	AudioFileComponent component;
	OSStatus status = newFileComponent(inFileType, &component);
	if (status != noErr)
		return status;

	if ((inFlags & kAudioFileFlags_EraseFile) && ::unlink(path) != 0 && errno != ENOENT)
	{
		AudioComponentInstanceDispose(component);
		return errnoToStatus(errno);
	}

	status = AudioFileComponentCreateURL(component, inFileRef, inFormat, inFlags);
	if (status != noErr)
	{
		AudioComponentInstanceDispose(component);
		return status;
	}

	*outAudioFile = reinterpret_cast<AudioFileID>(component);
	return noErr;
}

OSStatus AudioFileOpenURL ( CFURLRef inFileRef,
	AudioFilePermissions inPermissions,
	AudioFileTypeID inFileTypeHint,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	char path[PATH_MAX];

	if (!outAudioFile || !urlToPath(inFileRef, path))
		return kAudio_ParamError;

	*outAudioFile = nullptr;

	const int flags = (inPermissions == kAudioFileReadPermission) ? O_RDONLY
		: (inPermissions == kAudioFileWritePermission) ? O_WRONLY : O_RDWR;

	int fd = ::open(path, O_RDONLY);
	if (fd < 0)
		return openErrnoToStatus(errno);

	std::vector<uint8_t> probe(PROBE_SIZE);
	const ssize_t probed = ::read(fd, probe.data(), probe.size());
	::close(fd);

	if (probed < 0)
		return openErrnoToStatus(errno);
	if (probed == 0)
		return kAudioFileUnspecifiedError; // an empty file

	OSStatus status = kAudioFileUnsupportedFileTypeError;
	OSStatus recognizedStatus = kAudioFileUnsupportedFileTypeError;
	bool triedRecognized = false;
	std::vector<AudioFileTypeID> recognized;

	for (AudioFileTypeID type : candidateTypes(inFileTypeHint, inFileRef, probe.data(), UInt32(probed), recognized))
	{
		AudioFileComponent component;
		if (newFileComponent(type, &component) != noErr)
			continue;

		// The component owns the descriptor, and closes it even if it can't read the file
		fd = ::open(path, flags);
		if (fd < 0)
		{
			AudioComponentInstanceDispose(component);
			return openErrnoToStatus(errno);
		}

		status = AudioFileComponentOpenURL(component, inFileRef, inPermissions, fd);
		if (status == noErr)
		{
			*outAudioFile = reinterpret_cast<AudioFileID>(component);
			return noErr;
		}

		if (!triedRecognized && std::find(recognized.begin(), recognized.end(), type) != recognized.end())
		{
			triedRecognized = true;
			recognizedStatus = status;
		}

		AudioComponentInstanceDispose(component);
	}

	// A file that looks like a kind of file but fails to open as one is invalid; one that looks like nothing is of an unsupported type
	return triedRecognized ? recognizedStatus : kAudioFileUnsupportedFileTypeError;
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
	if (!outAudioFile || !inFormat)
		return kAudio_ParamError;

	*outAudioFile = nullptr;

	AudioFileComponent component;
	OSStatus status = newFileComponent(inFileType, &component);
	if (status != noErr)
		return status;

	status = AudioFileComponentInitializeWithCallbacks(component, inClientData, inReadFunc, inWriteFunc,
		inGetSizeFunc, inSetSizeFunc, inFileType, inFormat, inFlags);

	if (status != noErr)
	{
		AudioComponentInstanceDispose(component);
		return status;
	}

	*outAudioFile = reinterpret_cast<AudioFileID>(component);
	return noErr;
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
	if (!outAudioFile || !inReadFunc || !inGetSizeFunc)
		return kAudio_ParamError;

	*outAudioFile = nullptr;

	std::vector<uint8_t> probe(PROBE_SIZE);
	UInt32 probed = 0;

	// An error of the caller's read function is the result, whatever it is
	const OSStatus readStatus = inReadFunc(inClientData, 0, UInt32(probe.size()), probe.data(), &probed);
	if (readStatus != noErr)
		return readStatus;

	OSStatus status = kAudioFileUnsupportedFileTypeError;
	OSStatus recognizedStatus = kAudioFileUnsupportedFileTypeError;
	bool triedRecognized = false;
	std::vector<AudioFileTypeID> recognized;

	for (AudioFileTypeID type : candidateTypes(inFileTypeHint, nullptr, probe.data(), probed, recognized))
	{
		AudioFileComponent component;
		if (newFileComponent(type, &component) != noErr)
			continue;

		status = AudioFileComponentOpenWithCallbacks(component, inClientData, inReadFunc, inWriteFunc, inGetSizeFunc, inSetSizeFunc);
		if (status == noErr)
		{
			*outAudioFile = reinterpret_cast<AudioFileID>(component);
			return noErr;
		}

		if (!triedRecognized && std::find(recognized.begin(), recognized.end(), type) != recognized.end())
		{
			triedRecognized = true;
			recognizedStatus = status;
		}

		AudioComponentInstanceDispose(component);
	}

	return triedRecognized ? recognizedStatus : kAudioFileUnsupportedFileTypeError;
}

static AudioFileComponent toComponent(AudioFileID file)
{
	return reinterpret_cast<AudioFileComponent>(file);
}

OSStatus AudioFileClose (AudioFileID inAudioFile)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	OSStatus status = AudioFileComponentCloseFile(toComponent(inAudioFile));
	OSStatus disposeStatus = AudioComponentInstanceDispose(toComponent(inAudioFile));

	return (status != noErr) ? status : disposeStatus;
}

OSStatus AudioFileOptimize (AudioFileID inAudioFile)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentOptimize(toComponent(inAudioFile));
}

OSStatus AudioFileReadBytes ( AudioFileID inAudioFile,
	Boolean inUseCache,
	SInt64 inStartingByte,
	UInt32 *ioNumBytes,
	void *outBuffer)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentReadBytes(toComponent(inAudioFile), inUseCache, inStartingByte, ioNumBytes, outBuffer);
}

OSStatus AudioFileWriteBytes ( AudioFileID inAudioFile,
	Boolean inUseCache,
	SInt64 inStartingByte,
	UInt32 *ioNumBytes,
	const void *inBuffer)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentWriteBytes(toComponent(inAudioFile), inUseCache, inStartingByte, ioNumBytes, inBuffer);
}

OSStatus AudioFileReadPacketData ( AudioFileID inAudioFile,
	Boolean inUseCache,
	UInt32 * ioNumBytes,
	AudioStreamPacketDescription * _Nullable outPacketDescriptions,
	SInt64 inStartingPacket,
	UInt32 * ioNumPackets,
	void * _Nullable outBuffer)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentReadPacketData(toComponent(inAudioFile), inUseCache, ioNumBytes, outPacketDescriptions,
		inStartingPacket, ioNumPackets, outBuffer);
}

OSStatus AudioFileReadPackets ( AudioFileID inAudioFile,
	Boolean inUseCache,
	UInt32 * outNumBytes,
	AudioStreamPacketDescription * _Nullable outPacketDescriptions,
	SInt64 inStartingPacket,
	UInt32 * ioNumPackets,
	void * _Nullable outBuffer)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentReadPackets(toComponent(inAudioFile), inUseCache, outNumBytes, outPacketDescriptions,
		inStartingPacket, ioNumPackets, outBuffer);
}

OSStatus AudioFileWritePackets ( AudioFileID inAudioFile,
	Boolean inUseCache,
	UInt32 inNumBytes,
	const AudioStreamPacketDescription * _Nullable inPacketDescriptions,
	SInt64 inStartingPacket,
	UInt32 * ioNumPackets,
	const void * inBuffer)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentWritePackets(toComponent(inAudioFile), inUseCache, inNumBytes, inPacketDescriptions,
		inStartingPacket, ioNumPackets, inBuffer);
}

OSStatus AudioFileCountUserData ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 *outNumberItems)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentCountUserData(toComponent(inAudioFile), inUserDataID, outNumberItems);
}

OSStatus AudioFileGetUserDataSize ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 inIndex,
	UInt32 *outUserDataSize)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentGetUserDataSize(toComponent(inAudioFile), inUserDataID, inIndex, outUserDataSize);
}

OSStatus AudioFileGetUserData ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 inIndex,
	UInt32 *ioUserDataSize,
	void *outUserData)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentGetUserData(toComponent(inAudioFile), inUserDataID, inIndex, ioUserDataSize, outUserData);
}

OSStatus AudioFileSetUserData ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 inIndex,
	UInt32 inUserDataSize,
	const void *inUserData)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentSetUserData(toComponent(inAudioFile), inUserDataID, inIndex, inUserDataSize, inUserData);
}

OSStatus AudioFileRemoveUserData ( AudioFileID inAudioFile,
	UInt32 inUserDataID,
	UInt32 inIndex)
{
	if (!inAudioFile)
		return kAudioFileNotOpenError;

	return AudioFileComponentRemoveUserData(toComponent(inAudioFile), inUserDataID, inIndex);
}

OSStatus AudioFileGetPropertyInfo( AudioFileID inAudioFile,
	AudioFilePropertyID inPropertyID,
	UInt32 * _Nullable outDataSize,
	UInt32 * _Nullable isWritable)
{
	if (!inAudioFile)
		return kAudio_ParamError;

	return AudioFileComponentGetPropertyInfo(toComponent(inAudioFile), inPropertyID, outDataSize, isWritable);
}

OSStatus AudioFileGetProperty( AudioFileID inAudioFile,
	AudioFilePropertyID inPropertyID,
	UInt32 *ioDataSize,
	void *outPropertyData)
{
	if (!inAudioFile)
		return kAudio_ParamError;

	return AudioFileComponentGetProperty(toComponent(inAudioFile), inPropertyID, ioDataSize, outPropertyData);
}

OSStatus AudioFileSetProperty( AudioFileID inAudioFile,
	AudioFilePropertyID inPropertyID,
	UInt32 inDataSize,
	const void *inPropertyData)
{
	if (!inAudioFile)
		return kAudio_ParamError;

	return AudioFileComponentSetProperty(toComponent(inAudioFile), inPropertyID, inDataSize, inPropertyData);
}

static OSStatus globalInfo(AudioFilePropertyID inPropertyID, UInt32 inSpecifierSize, void* inSpecifier,
	UInt32* ioDataSize, void* outPropertyData, bool sizeOnly);

OSStatus AudioFileGetGlobalInfoSize( AudioFilePropertyID inPropertyID,
	UInt32 inSpecifierSize,
	void * _Nullable inSpecifier,
	UInt32 *outDataSize)
{
	if (!outDataSize)
		return kAudio_ParamError;

	return globalInfo(inPropertyID, inSpecifierSize, inSpecifier, outDataSize, nullptr, true);
}

// Copies what fits in `capacity` bytes
static void writeUInt32Set(const std::set<UInt32>& set, void *outPropertyData, UInt32 capacity)
{
	if (outPropertyData)
	{
		UInt32* out = static_cast<UInt32*>(outPropertyData);
		UInt32 room = capacity / sizeof(UInt32);
		for (UInt32 v : set)
		{
			if (room-- == 0)
				break;
			*out++ = v;
		}
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
	if (!ioDataSize || !outPropertyData)
		return kAudio_ParamError;

	return globalInfo(inPropertyID, inSpecifierSize, inSpecifier, ioDataSize, outPropertyData, false);
}

// The size query reports the size of the answer and ignores the specifier where the size does not depend on it.
// The query for the answer fails with kAudioFileBadPropertySizeError when it does not fit in *ioDataSize bytes, except for the
// lists of types, which are cut at the end of the buffer while *ioDataSize reports the size of the whole list.
static OSStatus globalInfo(AudioFilePropertyID inPropertyID, UInt32 inSpecifierSize, void* inSpecifier,
	UInt32* ioDataSize, void* outPropertyData, bool sizeOnly)
{
	const UInt32 capacity = *ioDataSize;

	switch (inPropertyID)
	{
		case kAudioFileGlobalInfo_ReadableTypes:
		case kAudioFileGlobalInfo_WritableTypes:
		{
			std::set<UInt32> types = AudioFileFormatManager::instance()->types(inPropertyID == kAudioFileGlobalInfo_WritableTypes);

			*ioDataSize = types.size() * sizeof(UInt32);
			writeUInt32Set(types, outPropertyData, capacity);
			
			return noErr;
		}
		case kAudioFileGlobalInfo_FileTypeName:
		{
			if (sizeOnly)
			{
				*ioDataSize = sizeof(CFStringRef);
				return noErr;
			}

			const UInt32* fileType = static_cast<const UInt32*>(inSpecifier);
			if (inSpecifierSize != sizeof(*fileType))
				return kAudioFileBadPropertySizeError;
			if (!fileType)
				return kAudio_ParamError;
			if (capacity < sizeof(CFStringRef))
				return kAudioFileBadPropertySizeError;

			const AudioFileFormatManager::ComponentInfo* ci;

			ci = AudioFileFormatManager::instance()->fileType(*fileType);
			if (!ci)
				return kAudioFileUnsupportedFileTypeError;

			CFStringRef* out = static_cast<CFStringRef*>(outPropertyData);
			*ioDataSize = sizeof(*out);
			*out = CFStringCreateWithCString(nullptr, ci->name.c_str(), kCFStringEncodingUTF8);

			return noErr;
		}
		case kAudioFileGlobalInfo_AvailableStreamDescriptionsForFormat:
		{
			const AudioFileTypeAndFormatID* spec = static_cast<const AudioFileTypeAndFormatID*>(inSpecifier);
			if (inSpecifierSize != sizeof(*spec))
				return kAudioFileBadPropertySizeError;
			if (!spec)
				return kAudio_ParamError;
			break;
		}
		case kAudioFileGlobalInfo_AvailableFormatIDs:
		{
			const UInt32* fileType = static_cast<const UInt32*>(inSpecifier);
			if (inSpecifierSize != sizeof(*fileType))
				return kAudioFileBadPropertySizeError;
			if (!fileType)
				return kAudio_ParamError;

			auto set = AudioFileFormatManager::instance()->availableFormatIDs(*fileType);
			*ioDataSize = set.size() * sizeof(UInt32);

			writeUInt32Set(set, outPropertyData, capacity);

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
				if (capacity < sizeof(CFArrayRef))
					return kAudioFileBadPropertySizeError;

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
			*ioDataSize = sizeof(CFArrayRef);
			if (sizeOnly)
				return noErr;

			const UInt32* fileType = static_cast<const UInt32*>(inSpecifier);
			if (inSpecifierSize != sizeof(*fileType))
				return kAudioFileBadPropertySizeError;
			if (!fileType)
				return kAudio_ParamError;
			if (capacity < sizeof(CFArrayRef))
				return kAudioFileBadPropertySizeError;

			const std::vector<std::string>* vector = nullptr;
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

			return noErr;
		}
		case kAudioFileGlobalInfo_TypesForMIMEType:
		case kAudioFileGlobalInfo_TypesForUTI:
		case kAudioFileGlobalInfo_TypesForExtension:
		{
			// The specifier is a buffer holding a CFStringRef, like every other specifier
			CFStringRef str;
			if (!inSpecifier || inSpecifierSize != sizeof(str))
				return kAudioFileBadPropertySizeError;
			str = *static_cast<const CFStringRef*>(inSpecifier);
			if (!str)
				return kAudio_ParamError;

			// CFStringGetCStringPtr() may return NULL for any string, so copy it when it does
			std::string key;
			if (const char* cstr = CFStringGetCStringPtr(str, kCFStringEncodingUTF8))
				key = cstr;
			else
			{
				CFIndex size = CFStringGetMaximumSizeForEncoding(CFStringGetLength(str), kCFStringEncodingUTF8) + 1;

				key.resize(size);
				if (!CFStringGetCString(str, &key[0], size, kCFStringEncodingUTF8))
					return kAudio_ParamError;
				key.resize(key.find('\0'));
			}

			AudioFileFormatManager* mgr = AudioFileFormatManager::instance();
			std::set<UInt32> set;

			if (inPropertyID == kAudioFileGlobalInfo_TypesForMIMEType)
				set = mgr->typesForMIME(key.c_str());
			else if (inPropertyID == kAudioFileGlobalInfo_TypesForUTI)
				set = mgr->typesForUTI(key.c_str());
			else if (inPropertyID == kAudioFileGlobalInfo_TypesForExtension)
				set = mgr->typesForExtension(key.c_str());

			*ioDataSize = set.size() * sizeof(UInt32);
			writeUInt32Set(set, outPropertyData, capacity);

			return noErr;
		}
	}
	return sizeOnly ? kAudioFileUnspecifiedError : unimpErr;
}

OSStatus AudioFileCreate ( const struct FSRef *inParentRef,
	CFStringRef inFileName,
	AudioFileTypeID inFileType,
	const AudioStreamBasicDescription *inFormat,
	AudioFileFlags inFlags,
	struct FSRef *outNewFileRef,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	if (!inParentRef || !inFileName)
		return kAudio_ParamError;

	CFURLRef parent = CFURLCreateFromFSRef(nullptr, const_cast<FSRef*>(inParentRef));
	if (!parent)
		return fnfErr;

	CFURLRef url = CFURLCreateCopyAppendingPathComponent(nullptr, parent, inFileName, false);
	CFRelease(parent);

	OSStatus status = AudioFileCreateWithURL(url, inFileType, inFormat, inFlags, outAudioFile);

	if (status == noErr && outNewFileRef)
		CFURLGetFSRef(url, outNewFileRef);

	CFRelease(url);
	return status;
}

OSStatus AudioFileInitialize ( const struct FSRef *inFileRef,
	AudioFileTypeID inFileType,
	const AudioStreamBasicDescription *inFormat,
	AudioFileFlags inFlags,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	if (!inFileRef)
		return kAudio_ParamError;

	CFURLRef url = CFURLCreateFromFSRef(nullptr, const_cast<FSRef*>(inFileRef));
	if (!url)
		return fnfErr;

	// Initializing replaces all of the file's contents
	OSStatus status = AudioFileCreateWithURL(url, inFileType, inFormat, inFlags | kAudioFileFlags_EraseFile, outAudioFile);

	CFRelease(url);
	return status;
}

OSStatus AudioFileOpen ( const struct FSRef *inFileRef,
	AudioFilePermissions inPermissions,
	AudioFileTypeID inFileTypeHint,
	AudioFileID _Nullable * _Nonnull outAudioFile)
{
	if (!inFileRef)
		return kAudio_ParamError;

	CFURLRef url = CFURLCreateFromFSRef(nullptr, const_cast<FSRef*>(inFileRef));
	if (!url)
		return fnfErr;

	OSStatus status = AudioFileOpenURL(url, inPermissions, inFileTypeHint, outAudioFile);

	CFRelease(url);
	return status;
}

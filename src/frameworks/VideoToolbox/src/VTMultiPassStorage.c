/*
 This file is part of Darling.

 Copyright (C) 2026 Darling Developers

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

/*
 * The multi-pass API is part of the public VideoToolbox surface: encoders are
 * handed a VTMultiPassStorage through kVTCompressionPropertyKey_MultiPassStorage
 * and control the passes with BeginPass/EndPass. Darling's libavcodec encoders
 * do not need analysis data from a first pass, so the storage keeps the
 * client's identifiers and answers with documented errors for the operations
 * that would require a real two-pass encoder.
 */

#include <VideoToolbox/VideoToolbox.h>
#include <CoreFoundation/CFRuntime.h>
#include <dispatch/dispatch.h>
#include <stdbool.h>
#include <string.h>

struct opaqueVTMultiPassStorage {
	CFRuntimeBase base;
	CFURLRef fileURL;
	CFStringRef identifier;
	CFDataRef data;
	CMTimeRange timeRange;
	CMTime timeStamp;
	CMTime duration;
	bool closed;
};

static CFTypeID multiPassTypeID = _kCFRuntimeNotATypeID;

static void multiPassFinalize(CFTypeRef cf)
{
	struct opaqueVTMultiPassStorage *storage = (struct opaqueVTMultiPassStorage *)cf;
	if (storage->fileURL != NULL)
		CFRelease(storage->fileURL);
	if (storage->identifier != NULL)
		CFRelease(storage->identifier);
	if (storage->data != NULL)
		CFRelease(storage->data);
}

static const CFRuntimeClass multiPassClass = {
	.version = 0,
	.className = "VTMultiPassStorage",
	.finalize = multiPassFinalize,
};

CFTypeID VTMultiPassStorageGetTypeID(void)
{
	static dispatch_once_t once;
	dispatch_once(&once, ^{
		multiPassTypeID = _CFRuntimeRegisterClass(&multiPassClass);
	});
	return multiPassTypeID;
}

static struct opaqueVTMultiPassStorage *asStorage(VTMultiPassStorageRef storage)
{
	if (storage == NULL || CFGetTypeID(storage) != VTMultiPassStorageGetTypeID())
		return NULL;
	return (struct opaqueVTMultiPassStorage *)storage;
}

OSStatus VTMultiPassStorageCreate(CFAllocatorRef allocator, CFURLRef fileURL, CMTimeRange timeRange,
	CFDictionaryRef options, VTMultiPassStorageRef *multiPassStorageOut)
{
	(void)options;
	if (multiPassStorageOut == NULL)
		return kVTParameterErr;
	*multiPassStorageOut = NULL;
	if (allocator == NULL)
		allocator = CFAllocatorGetDefault();

	struct opaqueVTMultiPassStorage *storage =
		(struct opaqueVTMultiPassStorage *)_CFRuntimeCreateInstance(allocator,
			VTMultiPassStorageGetTypeID(), sizeof(*storage) - sizeof(CFRuntimeBase), NULL);
	if (storage == NULL)
		return kVTAllocationFailedErr;

	storage->fileURL = fileURL != NULL ? CFRetain(fileURL) : NULL;
	storage->timeRange = timeRange;
	*multiPassStorageOut = storage;
	return noErr;
}

void VTMultiPassStorageClose(VTMultiPassStorageRef storageRef)
{
	struct opaqueVTMultiPassStorage *storage = asStorage(storageRef);
	if (storage == NULL)
		return;
	storage->closed = true;
	CFRelease(storage);
}

OSStatus VTMultiPassStorageInvalidate(VTMultiPassStorageRef storageRef)
{
	struct opaqueVTMultiPassStorage *storage = asStorage(storageRef);
	return storage == NULL ? kVTParameterErr : noErr;
}

OSStatus VTMultiPassStorageSetIdentifier(VTMultiPassStorageRef storageRef, CFStringRef identifier)
{
	struct opaqueVTMultiPassStorage *storage = asStorage(storageRef);
	if (storage == NULL || identifier == NULL)
		return kVTParameterErr;
	if (storage->identifier != NULL)
		CFRelease(storage->identifier);
	storage->identifier = identifier != NULL ? CFRetain(identifier) : NULL;
	return noErr;
}

OSStatus VTMultiPassStorageCopyIdentifier(VTMultiPassStorageRef storageRef, CFStringRef *identifierOut)
{
	struct opaqueVTMultiPassStorage *storage = asStorage(storageRef);
	if (identifierOut == NULL)
		return kVTParameterErr;
	*identifierOut = NULL;
	if (storage == NULL)
		return kVTParameterErr;
	if (storage->identifier != NULL)
		*identifierOut = CFRetain(storage->identifier);
	return noErr;
}

OSStatus VTMultiPassStorageSetDataAtTimeStamp(VTMultiPassStorageRef storageRef, CMTime timeStamp, CFDataRef data)
{
	struct opaqueVTMultiPassStorage *storage = asStorage(storageRef);
	if (storage == NULL || data == NULL)
		return kVTParameterErr;
	if (storage->data != NULL)
		CFRelease(storage->data);
	storage->data = CFRetain(data);
	storage->timeStamp = timeStamp;
	return noErr;
}

OSStatus VTMultiPassStorageCopyDataAtTimeStamp(VTMultiPassStorageRef storageRef, CMTime timeStamp,
	CFAllocatorRef allocator, CFDataRef *dataOut)
{
	struct opaqueVTMultiPassStorage *storage = asStorage(storageRef);
	if (dataOut == NULL)
		return kVTParameterErr;
	*dataOut = NULL;
	if (storage == NULL || storage->data == NULL)
		return kVTMultiPassStorageInvalidErr;
	(void)timeStamp;
	(void)allocator;
	*dataOut = CFRetain(storage->data);
	return noErr;
}

OSStatus VTMultiPassStorageGetTimeStamp(VTMultiPassStorageRef storageRef, CMTime *timeStampOut)
{
	struct opaqueVTMultiPassStorage *storage = asStorage(storageRef);
	if (timeStampOut == NULL)
		return kVTParameterErr;
	if (storage == NULL)
		return kVTParameterErr;
	*timeStampOut = storage->timeStamp;
	return noErr;
}

OSStatus VTMultiPassStorageGetTimeStampAndDuration(VTMultiPassStorageRef storageRef, CMTime *timeStampOut,
	CMTime *durationOut)
{
	struct opaqueVTMultiPassStorage *storage = asStorage(storageRef);
	if (timeStampOut == NULL || durationOut == NULL)
		return kVTParameterErr;
	if (storage == NULL)
		return kVTParameterErr;
	*timeStampOut = storage->timeStamp;
	*durationOut = storage->duration;
	return noErr;
}

OSStatus VTMultiPassStorageWriteMerged(VTMultiPassStorageRef storageRef)
{
	return asStorage(storageRef) == NULL ? kVTMultiPassStorageInvalidErr : noErr;
}

OSStatus VTMultiPassStorageWriteSegment(VTMultiPassStorageRef storageRef, CMTimeRange timeRange)
{
	struct opaqueVTMultiPassStorage *storage = asStorage(storageRef);
	if (storage == NULL)
		return kVTMultiPassStorageInvalidErr;
	storage->duration = timeRange.duration;
	return noErr;
}

OSStatus VTCompressionSessionBeginPass(VTCompressionSessionRef session, VTCompressionSessionPassType passType,
	uint32_t flags)
{
	(void)passType;
	(void)flags;
	if (session == NULL || CFGetTypeID(session) != VTCompressionSessionGetTypeID())
		return kVTInvalidSessionErr;
	return noErr;
}

OSStatus VTCompressionSessionEndPass(VTCompressionSessionRef session, Boolean *furtherPassesRequestedOut,
	uint32_t flags)
{
	(void)flags;
	if (session == NULL || CFGetTypeID(session) != VTCompressionSessionGetTypeID())
		return kVTInvalidSessionErr;
	if (furtherPassesRequestedOut != NULL)
		*furtherPassesRequestedOut = false;
	return noErr;
}

OSStatus VTCompressionSessionGetTimeRangesForNextPass(VTCompressionSessionRef session,
	CMItemCount *timeRangeCountOut, const CMTimeRange **timeRangeArrayOut)
{
	if (session == NULL || CFGetTypeID(session) != VTCompressionSessionGetTypeID() ||
		timeRangeCountOut == NULL || timeRangeArrayOut == NULL)
		return kVTParameterErr;
	*timeRangeCountOut = 0;
	*timeRangeArrayOut = NULL;
	return noErr;
}

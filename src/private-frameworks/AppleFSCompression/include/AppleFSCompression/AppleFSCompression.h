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


#ifndef _AppleFSCompression_H_
#define _AppleFSCompression_H_

#include <stdint.h>
#include <stddef.h>
#include <CoreFoundation/CoreFoundation.h>

// generated stubs
void* AFSCLockFileFd(void);
void* AFSCLockFilePath(void);
void* AFSCUnlockFile(void);
void* CloseStreamCompressor(void);
void* CreateStreamCompressor(void);
void* CreateStreamCompressorQueue(void);
void* CreateStreamCompressorQueueWithOptions(void);
void* DrainStreamCompressorQueue(void);
void* FinishStreamCompressorQueue(void);
void* VolumeSupportsCompression(void);
void* WriteToStreamCompressor(void);
void* catFile(void);
void* decompressFile(void);
void* decompressWithCallback(void);
void* type5_handler_function(void);

// manually fixed up declarations and definitions

// someone please check this value
#define kAFSCCompressionTypes CFSTR("AFSCCompressionTypes")

typedef struct {
	int compressionType;
	size_t compressedSize;
} cmpInfo;

typedef void* CompressionQueueContext;

int queryCompressionInfo(const char* path, cmpInfo* info);
CompressionQueueContext CreateCompressionQueue(void* arg1, void* arg2, void* arg3, void* arg4, CFDictionaryRef options);
Boolean CompressFile(CompressionQueueContext queue, const char* path, void* arg3);
void FinishCompressionAndCleanUp(CompressionQueueContext queue);

#endif

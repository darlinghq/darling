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


#include <AppleFSCompression/AppleFSCompression.h>
#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;

__attribute__((constructor))
static void initme(void) {
	verbose = getenv("STUB_VERBOSE") != NULL;
}

void* AFSCLockFileFd(void)
{
	if (verbose) puts("STUB: AFSCLockFileFd called");
	return NULL;
}

void* AFSCLockFilePath(void)
{
	if (verbose) puts("STUB: AFSCLockFilePath called");
	return NULL;
}

void* AFSCUnlockFile(void)
{
	if (verbose) puts("STUB: AFSCUnlockFile called");
	return NULL;
}

void* CloseStreamCompressor(void)
{
	if (verbose) puts("STUB: CloseStreamCompressor called");
	return NULL;
}

void* CreateStreamCompressor(void)
{
	if (verbose) puts("STUB: CreateStreamCompressor called");
	return NULL;
}

void* CreateStreamCompressorQueue(void)
{
	if (verbose) puts("STUB: CreateStreamCompressorQueue called");
	return NULL;
}

void* CreateStreamCompressorQueueWithOptions(void)
{
	if (verbose) puts("STUB: CreateStreamCompressorQueueWithOptions called");
	return NULL;
}

void* DrainStreamCompressorQueue(void)
{
	if (verbose) puts("STUB: DrainStreamCompressorQueue called");
	return NULL;
}

void* FinishStreamCompressorQueue(void)
{
	if (verbose) puts("STUB: FinishStreamCompressorQueue called");
	return NULL;
}

void* VolumeSupportsCompression(void)
{
	if (verbose) puts("STUB: VolumeSupportsCompression called");
	return NULL;
}

void* WriteToStreamCompressor(void)
{
	if (verbose) puts("STUB: WriteToStreamCompressor called");
	return NULL;
}

void* catFile(void)
{
	if (verbose) puts("STUB: catFile called");
	return NULL;
}

void* decompressFile(void)
{
	if (verbose) puts("STUB: decompressFile called");
	return NULL;
}

void* decompressWithCallback(void)
{
	if (verbose) puts("STUB: decompressWithCallback called");
	return NULL;
}

void* type5_handler_function(void)
{
	if (verbose) puts("STUB: type5_handler_function called");
	return NULL;
}

int queryCompressionInfo(const char* path, cmpInfo* info) {
	if (verbose) puts("STUB: queryCompressionInfo called");
	*info = (cmpInfo){0};
	return -1;
};

CompressionQueueContext CreateCompressionQueue(void* arg1, void* arg2, void* arg3, void* arg4, CFDictionaryRef options) {
	if (verbose) puts("STUB: CreateCompressionQueue called");
	return NULL;
};

Boolean CompressFile(CompressionQueueContext queue, const char* path, void* arg3) {
	if (verbose) puts("STUB: CompressFile called");
	return 0;
};

void FinishCompressionAndCleanUp(CompressionQueueContext queue) {
	if (verbose) puts("STUB: FinishCompressionAndCleanUp called");
};

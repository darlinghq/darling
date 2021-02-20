/*
 * Copyright (c) 2008 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 * 
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 * 
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 * 
 * @APPLE_LICENSE_HEADER_END@
 */
//
//	wipefs.cpp
//

#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <sys/stat.h>
#include <sys/syslimits.h>
#include <string.h>
#include <spawn.h>
#include <os/log.h>

#include "ExtentManager.h"
#include "wipefs.h"

#define	wipefs_roundup(x, y)	((((x)+((y)-1))/(y))*(y))

struct __wipefs_ctx {
	int fd;
	class ExtentManager extMan;
    
	// xartutil information
	char *diskname;
};

static void
AddExtentsForFutureFS(class ExtentManager *extMan)
{
	// we don't know what blocks future FS will use to recognize itself.  But we'd better be safe than sorry and write
	// the first and last 2MB of the volume
	off_t size = 2 * 1024 * 1024;
	extMan->AddByteRangeExtent(0, size);
	extMan->AddByteRangeExtent(extMan->totalBytes - size, size);
}

static void
AddExtentsForHFS(class ExtentManager *extMan)
{
	// first 1KB is boot block, last 512B is reserved
	// the Volume Header (512B) is after 1KB and before the last 512B
	extMan->AddByteRangeExtent(0, 1024 + 512);
	extMan->AddByteRangeExtent(extMan->totalBytes - 1024, 1024);
}

static void
AddExtentsForMSDOS(class ExtentManager *extMan)
{
	// MSDOS needs the first block (in theory, up to 32KB)
	extMan->AddByteRangeExtent(0, 32 * 1024);
}

static void
AddExtentsForNTFS(class ExtentManager *extMan)
{
	// NTFS supports block size from 256B to 32768B.  The first, middle and last block are needed
	extMan->AddByteRangeExtent(0, 32 * 1024);
	extMan->AddByteRangeExtent(extMan->totalBytes - 32 * 1024, 32 * 1024);
	// to be safe, add the rage from (mid_point - 32KB) to (mid_point + 32KB)
	extMan->AddByteRangeExtent(extMan->totalBytes / 2 - 32 * 1024, 64 * 1024);
}

static void
AddExtentsForUDF(class ExtentManager *extMan)
{
	off_t lastBlockAddr = extMan->totalBlocks - 1;

	// Volume Recognization Sequence (VRS) starts at 32KB, usually less than 7 Volume Structure Descriptors (2KB each)
	extMan->AddByteRangeExtent(32 * 1024, 14 * 1024);

	// AVDP is on 256, 512, last block, last block - 256
	extMan->AddBlockRangeExtent(256, 1);
	extMan->AddBlockRangeExtent(512, 1);
	extMan->AddBlockRangeExtent(lastBlockAddr, 1);
	extMan->AddBlockRangeExtent(lastBlockAddr - 256, 1);

	// to be safe, assume the device has 2KB block size and do it again
	if (extMan->blockSize != 2048) {
		off_t blockSize = 2048;
		// AVDP is on 256, 512, last block, last block - 256
		extMan->AddByteRangeExtent(256 * blockSize, blockSize);
		extMan->AddByteRangeExtent(512 * blockSize, blockSize);
		extMan->AddByteRangeExtent(extMan->totalBytes - blockSize, blockSize);
		extMan->AddByteRangeExtent(extMan->totalBytes - 256 * blockSize, blockSize);
	}
}

static void
AddExtentsForUFS(class ExtentManager *extMan)
{
	// UFS super block is 8KB at offset 8KB
	extMan->AddByteRangeExtent(8192, 8192);
}

static void
AddExtentsForZFS(class ExtentManager *extMan)
{
	// ZFS needs the first 512KB and last 512KB for all the 4 disk labels
	extMan->AddByteRangeExtent(0, 512 * 1024);
	extMan->AddByteRangeExtent(extMan->totalBytes - 512 * 1024, 512 * 1024);
}

static void
AddExtentsForPartitions(class ExtentManager *extMan)
{
	// MBR (Master Boot Record) needs the first sector
	// APM (Apple Partition Map) needs the second sector
	// GPT (GUID Partition Table) needs the first 34 and last 33 sectors
	extMan->AddByteRangeExtent(0, 512 * 34);
	extMan->AddByteRangeExtent(extMan->totalBytes - 512 * 33, 512 * 33);
}

static void
AddExtentsForCoreStorage(class ExtentManager *extMan)
{
	// the CoreStorage VolumeHeader structures reside in the first/last 512 bytes of each PV
	extMan->AddByteRangeExtent(0, 512);
	extMan->AddByteRangeExtent(extMan->totalBytes - 512, 512);
}

static char *
query_disk_info(int fd) {
	char disk_path[PATH_MAX];
	char *disk_name;

	// Fetch the path
	if (fcntl(fd, F_GETPATH, disk_path) == -1) {
		return (NULL);
	}

	// Find the last pathname component.
	disk_name = strrchr(disk_path, '/');
	if (disk_name == NULL) {
		// Not that we expect this to happen...
		disk_name = disk_path;
	} else {
		// Skip over the '/'.
		disk_name++;
	}

	if (*disk_name == 'r') {
		// Raw device; skip over leading 'r'.
		disk_name++;
	}

	// ...and make sure it's really a disk.
	if (strncmp(disk_name, "disk", strlen("disk")) != 0) {
		return (NULL);
	}

	return (strdup(disk_name));
}

static
int run_xartutil(char *const diskname)
{
	pid_t child_pid, wait_pid;
	posix_spawn_file_actions_t fileActions;
	bool haveFileActions = false;
	int child_status = 0;
	int result = 0;

	char arg1[] = "xartutil";
	char arg2[] = "--erase-disk";

	char *const xartutil_argv[] = {arg1, arg2, diskname, NULL};

	result = posix_spawn_file_actions_init(&fileActions);
	if (result) {
		os_log_error(OS_LOG_DEFAULT, "Warning, init xartutil file actions error: %d", result);
		result = -1;
		goto out;
	}

	haveFileActions = true;
    
	// Redirect stdout & stderr (results not critical, so we ignore return values).
	posix_spawn_file_actions_addopen(&fileActions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
	posix_spawn_file_actions_addopen(&fileActions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);

	result = posix_spawn(&child_pid, "/usr/sbin/xartutil", &fileActions, NULL, xartutil_argv, NULL);

	if (result) {
		os_log_error(OS_LOG_DEFAULT, "Warning, unable to start xartutil, spawn error: %d", result);
		result = -1;
		goto out;
	}
    
	do {
		wait_pid = waitpid(child_pid, &child_status, 0);
	} while (wait_pid == -1 && errno == EINTR);

	if (wait_pid == -1) {
		os_log_error(OS_LOG_DEFAULT, "Warning, unable to start xartutil, waitpid error: %d", errno);
		result = -1;
		goto out;
	}

	if (WIFEXITED(child_status)) {
		// xartutil terminated normally, get exit status
		result = WEXITSTATUS(child_status);

		if (result) {
			os_log_error(OS_LOG_DEFAULT, "Warning, xartutil returned status %d", result);
		}
	} else {
		result = -1;

		if (WIFSIGNALED(child_status)) {
			os_log_error(OS_LOG_DEFAULT, "Warning, xartutil terminated by signal: %u", WTERMSIG(child_status));
		} else if (WIFSTOPPED(child_status)) {
			os_log_error(OS_LOG_DEFAULT, "Warning, xartutil stopped by signal: %u", WSTOPSIG(child_status));
		} else {
			os_log_error(OS_LOG_DEFAULT, "Warning, xartutil terminated abnormally, status 0x%x", child_status);
		}
	}

out:

	if (haveFileActions) {
		posix_spawn_file_actions_destroy(&fileActions);
	}

	return (result);
}

extern "C" int
wipefs_alloc(int fd, size_t block_size, wipefs_ctx *handle)
{
	int err = 0;
	uint64_t numBlocks = 0;
	uint32_t nativeBlockSize = 0;
	off_t totalSizeInBytes = 0;
	class ExtentManager *extMan = NULL;
	struct stat sbuf = { 0 };
	char *diskname = NULL;

	*handle = NULL;
	(void)fstat(fd, &sbuf);
	switch (sbuf.st_mode & S_IFMT) {
	case S_IFCHR:
	case S_IFBLK:
		if (ioctl(fd, DKIOCGETBLOCKSIZE, (char *)&nativeBlockSize) < 0) {
			err = errno;
			goto labelExit;
		}
		if (ioctl(fd, DKIOCGETBLOCKCOUNT, (char *)&numBlocks) < 0) {
			err = errno;
			goto labelExit;
		}
		totalSizeInBytes = numBlocks * nativeBlockSize;

		diskname = query_disk_info(fd);
		break;
	case S_IFREG:
		nativeBlockSize = sbuf.st_blksize;
		numBlocks = sbuf.st_size / sbuf.st_blksize;
		totalSizeInBytes = sbuf.st_size;
		break;
	default:
		errno = EINVAL;
		goto labelExit;
	}
	if (block_size == 0) {
		block_size = nativeBlockSize;
	}
	if (block_size == 0 || totalSizeInBytes == 0) {
		err = EINVAL;
		goto labelExit;
	}

	try {
		*handle = new __wipefs_ctx;
		if (*handle == NULL) {
			bad_alloc e;
			throw e;
		}

		(*handle)->fd = fd;
		(*handle)->diskname = NULL;
		extMan = &(*handle)->extMan;

		extMan->Init(block_size, nativeBlockSize, totalSizeInBytes);
		AddExtentsForFutureFS(extMan);
		AddExtentsForHFS(extMan);
		AddExtentsForMSDOS(extMan);
		AddExtentsForNTFS(extMan);
		AddExtentsForUDF(extMan);
		AddExtentsForUFS(extMan);
		AddExtentsForZFS(extMan);
		AddExtentsForPartitions(extMan);
		AddExtentsForCoreStorage(extMan);

		(*handle)->diskname = diskname;
	}
	catch (bad_alloc &e) {
		err = ENOMEM;
	}
	catch (...) { // currently only ENOMEM is possible
		err = ENOMEM;
	}

  labelExit:
	if (err != 0) {
		if (diskname != NULL)
			free(diskname);
		wipefs_free(handle);
	}
	return err;
} // wipefs_alloc

extern "C" int
wipefs_include_blocks(wipefs_ctx handle, off_t block_offset, off_t nblocks)
{
	int err = 0;
	try {
		handle->extMan.AddBlockRangeExtent(block_offset, nblocks);
	}
	catch (bad_alloc &e) {
		err = ENOMEM;
	}
	catch (...) { // currently only ENOMEM is possible
		err = ENOMEM;
	}
	return err;
}

extern "C" int
wipefs_except_blocks(wipefs_ctx handle, off_t block_offset, off_t nblocks)
{
	int err = 0;
	try {
		handle->extMan.RemoveBlockRangeExtent(block_offset, nblocks);
	}
	catch (bad_alloc &e) {
		err = ENOMEM;
	}
	catch (...) { // currently only ENOMEM is possible
		err = ENOMEM;
	}
	return err;
}

extern "C" int
wipefs_wipe(wipefs_ctx handle)
{
	int err = 0;
	uint8_t *bufZero = NULL;
	ListExtIt curExt;
	size_t bufSize;
	dk_extent_t extent;
	dk_unmap_t unmap;

	if (handle->diskname != NULL) {
		// Remove this disk's entry from the xART.
		run_xartutil(handle->diskname);
	}

	memset(&extent, 0, sizeof(dk_extent_t));
	extent.length = handle->extMan.totalBytes;

	memset(&unmap, 0, sizeof(dk_unmap_t));
	unmap.extents = &extent;
	unmap.extentsCount = 1;

	//
	// Don't bother to check the return value since this is mostly
	// informational for the lower-level drivers.
	//
	ioctl(handle->fd, DKIOCUNMAP, (caddr_t)&unmap);

	bufSize = 128 * 1024; // issue large I/O to get better performance
	if (handle->extMan.nativeBlockSize > bufSize) {
	    bufSize = handle->extMan.nativeBlockSize;
	}
	bufZero = new uint8_t[bufSize];
	bzero(bufZero, bufSize);

	off_t byteOffset, totalBytes;
	size_t numBytes, numBytesToWrite, blockSize;

	blockSize = handle->extMan.blockSize;
	totalBytes = handle->extMan.totalBytes;
	// write zero to all extents
	for (curExt = handle->extMan.extentList.begin(); curExt != handle->extMan.extentList.end(); curExt++) {
		byteOffset = curExt->blockAddr * blockSize;
		numBytes = curExt->numBlocks * blockSize;
		// make both offset and numBytes on native block boundary
		if (byteOffset % handle->extMan.nativeBlockSize != 0 ||
			numBytes % handle->extMan.nativeBlockSize != 0) {
			size_t nativeBlockSize = handle->extMan.nativeBlockSize;
			off_t newOffset, newEndOffset;
			newOffset = byteOffset / nativeBlockSize * nativeBlockSize;
			newEndOffset = wipefs_roundup(byteOffset + numBytes, nativeBlockSize);
			byteOffset = newOffset;
			numBytes = newEndOffset - newOffset;
		}
		if (byteOffset + (off_t)numBytes > totalBytes) {
			numBytes = totalBytes - byteOffset;
		}
		while (numBytes > 0) {
			numBytesToWrite = min(numBytes, bufSize);
			if (pwrite(handle->fd, bufZero, numBytesToWrite, byteOffset) != (ssize_t)numBytesToWrite) {
				err = errno;
				goto labelExit;
			}
			numBytes -= numBytesToWrite;
			byteOffset += numBytesToWrite;
		}
	}

  labelExit:

	(void)ioctl(handle->fd, DKIOCSYNCHRONIZECACHE);
	if (bufZero != NULL)
		delete[] bufZero;

	return err;
} // wipefs_wipe

extern "C" void
wipefs_free(wipefs_ctx *handle)
{
	if (*handle != NULL) {
		char *diskname;

		if ((diskname = (*handle)->diskname) != NULL)
			free(diskname);
		delete *handle;
		*handle = NULL;
	}
}

/*
 * Copyright (c) 2017 Apple Inc. All rights reserved.
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

#include "ClosureFileSystemPhysical.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#if BUILDING_UPDATE_DYLD_CACHE_BUILDER
  #include <rootless.h>
#endif
#include <sys/errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <mach/mach.h>
#if !TARGET_OS_SIMULATOR && !TARGET_OS_DRIVERKIT
  #include <sandbox.h>
  #include <sandbox/private.h>
#endif

using dyld3::closure::FileSystemPhysical;

bool FileSystemPhysical::getRealPath(const char possiblePath[MAXPATHLEN], char realPath[MAXPATHLEN]) const {
    __block bool success = false;
    // first pass: open file and ask kernel for canonical path
    forEachPath(possiblePath, ^(const char* aPath, unsigned prefixLen, bool& stop) {
        int fd = ::open(aPath, O_RDONLY, 0);
        if ( fd != -1 ) {
            char tempPath[MAXPATHLEN];
            success = (fcntl(fd, F_GETPATH, tempPath) == 0);
            ::close(fd);
            if ( success ) {
                // if prefix was used, remove it
                strcpy(realPath, &tempPath[prefixLen]);
            }
            stop = true;
        }
    });
    if (success)
        return success;

    // second pass: file does not exist but may be a symlink to a non-existent file
    // This is only for use on-device on platforms where dylibs are removed
    if ( _overlayPath == nullptr && _rootPath == nullptr ) {
        realpath(possiblePath, realPath);
        int realpathErrno = errno;
        // If realpath() resolves to a path which does not exist on disk, errno is set to ENOENT
        success = (realpathErrno == ENOENT) || (realpathErrno == 0);
    }
    return success;
}

static bool sandboxBlocked(const char* path, const char* kind)
{
#if TARGET_OS_SIMULATOR || TARGET_OS_DRIVERKIT
    // sandbox calls not yet supported in dyld_sim
    return false;
#else
    sandbox_filter_type filter = (sandbox_filter_type)(SANDBOX_FILTER_PATH | SANDBOX_CHECK_NO_REPORT);
    return ( sandbox_check(getpid(), kind, filter, path) > 0 );
#endif
}

static bool sandboxBlockedMmap(const char* path)
{
    return sandboxBlocked(path, "file-map-executable");
}

static bool sandboxBlockedOpen(const char* path)
{
    return sandboxBlocked(path, "file-read-data");
}

static bool sandboxBlockedStat(const char* path)
{
    return sandboxBlocked(path, "file-read-metadata");
}

void FileSystemPhysical::forEachPath(const char* path, void (^handler)(const char* fullPath, unsigned prefixLen, bool& stop)) const
{
    bool stop = false;
    char altPath[PATH_MAX];
    if ( _overlayPath != nullptr ) {
        strlcpy(altPath, _overlayPath, PATH_MAX);
        strlcat(altPath, path, PATH_MAX);
        handler(altPath, (unsigned)strlen(_overlayPath), stop);
        if ( stop )
            return;
    }
    if ( _rootPath != nullptr ) {
        strlcpy(altPath, _rootPath, PATH_MAX);
        strlcat(altPath, path, PATH_MAX);
        handler(altPath, (unsigned)strlen(_rootPath), stop);
        if ( stop )
            return;
    }
    else {
        handler(path, 0, stop);
    }
}

static bool isFileRelativePath(const char* path)
{
    if ( path[0] == '/' )
        return false;
    if ( path[0] != '.' )
        return true;
    if ( path[1] == '/' )
        return true;
    if ( (path[1] == '.') && (path[2] == '/') )
        return true;
    return false;
}

// Returns true on success.  If an error occurs the given callback will be called with the reason.
// On success, info is filled with info about the loaded file.  If the path supplied includes a symlink,
// the supplier realerPath is filled in with the real path of the file, otherwise it is set to the empty string.
bool FileSystemPhysical::loadFile(const char* path, LoadedFileInfo& info, char realerPath[MAXPATHLEN], void (^error)(const char* format, ...)) const {
    if ( !_allowRelativePaths && isFileRelativePath(path) ) {
        error("relative file paths not allowed '%s'", path);
        return false;
    }
    // open file
    __block int fd;
    __block struct stat statBuf;
    __block bool sipProtected = false;
    forEachPath(path, ^(const char* aPath, unsigned prefixLen, bool& stop) {
        fd = ::open(aPath, O_RDONLY, 0);
        if ( fd == -1 ) {
            int openErrno = errno;
            if ( (openErrno == EPERM) && sandboxBlockedOpen(path) )
                error("file system sandbox blocked open(\"%s\", O_RDONLY)", path);
            else if ( (openErrno != ENOENT) && (openErrno != ENOTDIR) )
                error("open(\"%s\", O_RDONLY) failed with errno=%d", path, openErrno);
        }
        else {
             // get file info
        #if TARGET_OS_SIMULATOR
            if ( ::stat(aPath, &statBuf) != 0 ) {
        #else
            if ( ::fstat(fd, &statBuf) != 0 ) {
        #endif
                int statErr = errno;
                if ( (statErr == EPERM) && sandboxBlockedStat(path) )
                    error("file system sandbox blocked stat(\"%s\")", path);
                else
                    error("stat(\"%s\") failed with errno=%d", path, errno);
                ::close(fd);
                fd = -1;
            }
            else {
                // Get the realpath of the file if it is a symlink
                char tempPath[MAXPATHLEN];
                if ( fcntl(fd, F_GETPATH, tempPath) == 0 ) {
                    const char* realPathWithin = &tempPath[prefixLen];
                    // Don't set the realpath if it is just the same as the regular path
                    if ( strcmp(path, realPathWithin) == 0 ) {
                        // zero out realerPath if path is fine as-is
                        // <rdar://45018392> don't trash input 'path' if realerPath is same buffer as path
                        if ( realerPath != path )
                            realerPath[0] = '\0';
                    }
                    else
                        strcpy(realerPath, realPathWithin);
    #if BUILDING_UPDATE_DYLD_CACHE_BUILDER
                    sipProtected = (rootless_check_trusted_fd(fd) == 0);
    #endif
                    stop = true;
                }
                else {
                    error("Could not get real path for \"%s\"\n", path);
                    ::close(fd);
                    fd = -1;
                }
            }
        }
    });
    if ( fd == -1 )
        return false;

    // only regular files can be loaded
    if ( !S_ISREG(statBuf.st_mode) ) {
        error("not a file for %s", path);
        ::close(fd);
        return false;
    }

    // mach-o files must be at list one page in size
    if ( statBuf.st_size < 4096  ) {
        error("file too short %s", path);
        ::close(fd);
        return false;
    }

    info.fileContent = nullptr;
    info.fileContentLen = statBuf.st_size;
    info.sliceOffset = 0;
    info.sliceLen = statBuf.st_size;
    info.isSipProtected = sipProtected;
    info.inode = statBuf.st_ino;
    info.mtime = statBuf.st_mtime;
    info.path  = path;

    // mmap() whole file
    void* wholeFile = ::mmap(nullptr, (size_t)statBuf.st_size, PROT_READ, MAP_PRIVATE|MAP_RESILIENT_CODESIGN, fd, 0);
    if ( wholeFile == MAP_FAILED ) {
        int mmapErr = errno;
        if ( mmapErr == EPERM ) {
            if ( sandboxBlockedMmap(path) )
                error("file system sandbox blocked mmap() of '%s'", path);
            else
                error("code signing blocked mmap() of '%s'", path);
        }
        else {
            error("mmap() failed with errno=%d for %s", errno, path);
        }
        ::close(fd);
        return false;
    }
    info.fileContent = wholeFile;

    // Set unmap as the unload method.
    info.unload = [](const LoadedFileInfo& info) {
        ::munmap((void*)info.fileContent, (size_t)info.fileContentLen);
    };

    ::close(fd);
    return true;
}

void FileSystemPhysical::unloadFile(const LoadedFileInfo& info) const {
    if (info.unload)
        info.unload(info);
}

void FileSystemPhysical::unloadPartialFile(LoadedFileInfo& info, uint64_t keepStartOffset, uint64_t keepLength) const {
    // Unmap from 0..keepStartOffset and (keepStartOffset+keepLength)..info.fileContentLen
    if (keepStartOffset)
        ::munmap((void*)info.fileContent, (size_t)keepStartOffset);
    if ((keepStartOffset + keepLength) != info.fileContentLen) {
        // Round up to page alignment
        keepLength = (keepLength + PAGE_SIZE - 1) & (-PAGE_SIZE);
        ::munmap((void*)((char*)info.fileContent + keepStartOffset + keepLength), (size_t)(info.fileContentLen - (keepStartOffset + keepLength)));
    }
    info.fileContent = (const void*)((char*)info.fileContent + keepStartOffset);
    info.fileContentLen = keepLength;
}

bool FileSystemPhysical::fileExists(const char* path, uint64_t* inode, uint64_t* mtime,
                                    bool* issetuid, bool* inodesMatchRuntime) const {
    __block bool result = false;
    forEachPath(path, ^(const char* aPath, unsigned prefixLen, bool& stop) {
        struct stat statBuf;
        if ( ::stat(aPath, &statBuf) == 0 ) {
            if (inode)
                *inode = statBuf.st_ino;
            if (mtime)
                *mtime = statBuf.st_mtime;
            if (issetuid)
                *issetuid = (statBuf.st_mode & (S_ISUID|S_ISGID));
            if (inodesMatchRuntime)
                *inodesMatchRuntime = true;
            stop   = true;
            result = true;
        }
    });
    return result;
}

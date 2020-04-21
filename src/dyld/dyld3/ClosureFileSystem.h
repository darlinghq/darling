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

#ifndef ClosureFileSystem_h
#define ClosureFileSystem_h

// For MAXPATHLEN
#include <sys/param.h>
// For va_list
#include <stdarg.h>
// For uint64_t
#include <stdint.h>

namespace dyld3 {
namespace closure {

struct LoadedFileInfo {
    const void*  fileContent                = nullptr;
    uint64_t     fileContentLen             = 0;
    uint64_t     sliceOffset                = 0;
    uint64_t     sliceLen            : 63,
                 isSipProtected      : 1;
    uint64_t     inode                      = 0;
    uint64_t     mtime                      = 0;
    void (*unload)(const LoadedFileInfo&)   = nullptr;
    const char*  path                       = nullptr;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
class FileSystem {
protected:
    FileSystem() { }

public:

    // Get the real path for a given path, if it exists.
    // Returns true if the real path was found and updates the given buffer iff that is the case
    virtual bool getRealPath(const char possiblePath[MAXPATHLEN], char realPath[MAXPATHLEN]) const = 0;

    // Returns true on success.  If an error occurs the given callback will be called with the reason.
    // On success, info is filled with info about the loaded file.  If the path supplied includes a symlink,
    // the supplier realerPath is filled in with the real path of the file, otherwise it is set to the empty string.
    virtual bool loadFile(const char* path, LoadedFileInfo& info, char realerPath[MAXPATHLEN], void (^error)(const char* format, ...)) const = 0;

    // Frees the buffer allocated by loadFile()
    virtual void unloadFile(const LoadedFileInfo& info) const = 0;

    // Frees all but the requested range and adjusts info to new buffer location
    // Remaining buffer can be freed later with unloadFile()
    virtual void unloadPartialFile(LoadedFileInfo& info, uint64_t keepStartOffset, uint64_t keepLength) const = 0;

    // If a file exists at path, returns true and sets inode and mtime
    virtual bool fileExists(const char* path, uint64_t* inode=nullptr, uint64_t* mtime=nullptr,
                            bool* issetuid=nullptr, bool* inodesMatchRuntime = nullptr) const = 0;
};
#pragma clang diagnostic pop

} //  namespace closure
} //  namespace dyld3

#endif /* ClosureFileSystem_h */

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


#ifndef FileUtils_h
#define FileUtils_h

#include <stdint.h>

#include <map>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <dispatch/dispatch.h>

#include "DyldSharedCache.h"

class Diagnostics;

#if BUILDING_CACHE_BUILDER
struct FileCache {
    FileCache(void);
    std::pair<uint8_t*, struct stat> cacheLoad(Diagnostics& diags, const std::string path);

private:
    std::pair<uint8_t*, struct stat> fill(Diagnostics& diags, const std::string& path);

    std::unordered_map<std::string, std::pair<uint8_t*, struct stat>> entries;
    dispatch_queue_t cache_queue;
};

extern FileCache fileCache;
#endif

//
// recursively walk all files in a directory tree
// symlinks are ignored
// dirFilter should return true on directories which should not be recursed into
// callback is called on each regular file found with stat() info about the file
//
void iterateDirectoryTree(const std::string& pathPrefix, const std::string& path, bool (^dirFilter)(const std::string& dirPath),
                          void (^callback)(const std::string& path, const struct stat& statBuf), bool processFiles=true, bool recurse=true);


//
// writes the buffer to a temp file, then renames the file to the final path
// returns true on success
//
bool safeSave(const void* buffer, size_t bufferLen, const std::string& path);


const void* mapFileReadOnly(const char* path, size_t& mappedSize);

bool isProtectedBySIP(const std::string& path);
bool isProtectedBySIPExceptDyld(const std::string& path);
bool isProtectedBySIP(int fd);

bool fileExists(const std::string& path);

std::unordered_map<std::string, uint32_t> parseOrderFile(const std::string& orderFileData);
std::string loadOrderFile(const std::string& orderFilePath);

std::string normalize_absolute_file_path(std::string path);
std::string basePath(const std::string& path);
std::string dirPath(const std::string& path);
std::string realPath(const std::string& path);
std::string realFilePath(const std::string& path);

std::string toolDir();

#if BUILDING_CACHE_BUILDER
class SymlinkResolver {
public:
    SymlinkResolver() { }

    void addFile(Diagnostics& diags, std::string path);

    void addSymlink(Diagnostics& diags, std::string fromPath, std::string toPath);

    std::string realPath(Diagnostics& diags, const std::string& path) const;

    std::vector<DyldSharedCache::FileAlias> getResolvedSymlinks(Diagnostics& diags);

private:
    std::set<std::string> filePaths;
    std::map<std::string, std::string> symlinks;
};
#endif  // BUILDING_CACHE_BUILDER


#endif // FileUtils_h

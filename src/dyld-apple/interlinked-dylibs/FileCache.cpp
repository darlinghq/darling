/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2014 Apple Inc. All rights reserved.
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


#include "mega-dylib-utils.h"
#include "MachOFileAbstraction.hpp"
#include "Trie.hpp"

#include <dirent.h>
#include <sys/errno.h>
#include <sys/fcntl.h>
#include <sys/param.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/dyld.h>
#include <assert.h>
#include <Availability.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include <System/sys/csr.h>

#include "dyld_cache_config.h"

#include "OptimizerBranches.h"

#include "CacheFileAbstraction.hpp"

#include "mega-dylib-utils.h"
#include "Logging.h"


//#include <rootless.h>
extern "C" int rootless_check_trusted(const char *path);
extern "C" int rootless_check_trusted_fd(int fd) __attribute__((weak_import));

static bool            rootlessEnabled;
static dispatch_once_t onceToken;

bool isProtectedBySIP(const std::string& path, int fd)
{
    bool isProtected = false;
    // Check to make sure file system protections are on at all
    dispatch_once(&onceToken, ^{
        rootlessEnabled = csr_check(CSR_ALLOW_UNRESTRICTED_FS);
    });
    if (!rootlessEnabled)
        return false;
#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
    if ( (fd != -1) && (&rootless_check_trusted_fd != NULL) )
        isProtected = (rootless_check_trusted_fd(fd) == 0);
    else
#endif
    if ( &rootless_check_trusted != NULL )
        isProtected = (rootless_check_trusted(path.c_str()) == 0);
    return isProtected;
}


std::string toolDir()
{
    char buffer[PATH_MAX];
    uint32_t bufsize = PATH_MAX;
    int result = _NSGetExecutablePath(buffer, &bufsize);
    if ( result == 0 ) {
        std::string path = buffer;
        size_t pos = path.rfind('/');
        if ( pos != std::string::npos )
            return path.substr(0,pos+1);
    }
    warning("tool directory not found");
    return "/tmp/";
}

std::string baspath(const std::string& path)
{
    std::string::size_type slash_pos = path.rfind("/");
    if (slash_pos != std::string::npos) {
        slash_pos++;
        return path.substr(slash_pos);
    } else {
        return path;
    }
}

std::string dirpath(const std::string& path)
{
    std::string::size_type slash_pos = path.rfind("/");
    if (slash_pos != std::string::npos) {
        slash_pos++;
        return path.substr(0, slash_pos);
    } else {
        char cwd[MAXPATHLEN];
        (void)getcwd(cwd, MAXPATHLEN);
        return cwd;
    }
}

std::string normalize_absolute_file_path(const std::string &path) {
    std::vector<std::string> components;
    std::vector<std::string> processed_components;
    std::stringstream ss(path);
    std::string retval;
    std::string item;

    while (std::getline(ss, item, '/')) {
        components.push_back(item);
    }

    if (components[0] == ".") {
        retval = ".";
    }

    for (auto& component : components) {
        if (component.empty() || component == ".")
            continue;
        else if (component == ".." && processed_components.size())
            processed_components.pop_back();
        else
            processed_components.push_back(component);
    }

    for (auto & component : processed_components) {
        retval = retval + "/" + component;
    }

    return retval;
}

FileCache fileCache;

FileCache::FileCache(void) {
    cache_queue = dispatch_queue_create("com.apple.dyld.cache.cache", DISPATCH_QUEUE_SERIAL);
}


void FileCache::preflightCache(const std::unordered_set<std::string>& paths) {
    for (auto &path : paths) {
        preflightCache(path);
    }
}

void FileCache::preflightCache(const std::string& path)
{
    cacheBuilderDispatchAsync(cache_queue, [=] {
        std::string normalizedPath = normalize_absolute_file_path(path);
        if (entries.count(normalizedPath) == 0) {
            fill(normalizedPath);
        }
    });
}

std::tuple<uint8_t *, struct stat, bool> FileCache::cacheLoad(const std::string path) {
    std::string normalizedPath = normalize_absolute_file_path(path);
    cacheBuilderDispatchSync(cache_queue, [=] {
        if ( entries.count(normalizedPath) == 0 )
            fill(normalizedPath);
    });

    return entries[normalizedPath];
}


//FIXME error handling
void FileCache::fill(const std::string& path) {
    struct stat stat_buf;

    int fd = ::open(path.c_str(), O_RDONLY, 0);
    if ( fd == -1 ) {
        verboseLog("can't open file '%s', errno=%d", path.c_str(), errno);
        entries[path] = std::make_tuple((uint8_t*)(-1), stat_buf, false);
        return;
    }

    if ( fstat(fd, &stat_buf) == -1) {
        verboseLog("can't stat open file '%s', errno=%d", path.c_str(), errno);
        entries[path] = std::make_tuple((uint8_t*)(-1), stat_buf, false);
        ::close(fd);
        return;
    }

    if ( stat_buf.st_size < 4096 ) {
        verboseLog("file too small '%s'", path.c_str());
        entries[path] = std::make_tuple((uint8_t*)(-1), stat_buf, false);
        ::close(fd);
        return;
    }

    bool rootlessProtected = isProtectedBySIP(path, fd);

    void* buffer_ptr = mmap(NULL, stat_buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (buffer_ptr == MAP_FAILED) {
        verboseLog("mmap() for shared cache at %s failed, errno=%d", path.c_str(), errno);
        entries[path] = std::make_tuple((uint8_t*)(-1), stat_buf, false);
        ::close(fd);
        return;
    }

    entries[path] = std::make_tuple((uint8_t*)buffer_ptr, stat_buf, rootlessProtected);
    ::close(fd);
}







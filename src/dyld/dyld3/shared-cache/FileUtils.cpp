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


#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/mman.h>
#include <dispatch/dispatch.h>
#include <mach-o/dyld.h>
#include <System/sys/csr.h>
#ifndef DARLING
#include <rootless.h>
#endif

#include <string>
#include <fstream>
#include <sstream>

#include "FileUtils.h"
#include "StringUtils.h"
#include "Diagnostics.h"

#if __MAC_OS_X_VERSION_MIN_REQUIRED < 101200
extern "C" int rootless_check_trusted_fd(int fd) __attribute__((weak_import));
#endif

#ifdef DARLING
static int rootless_check_trusted(const char* path) { return -1; }
static int rootless_check_trusted_class(const char* path, const char* cls) { return -1; }
#endif

void iterateDirectoryTree(const std::string& pathPrefix, const std::string& path, bool (^dirFilter)(const std::string& path), void (^fileCallback)(const std::string& path, const struct stat&), bool processFiles, bool recurse)
{
    std::string fullDirPath = pathPrefix + path;
    DIR* dir = ::opendir(fullDirPath.c_str());
    if ( dir == nullptr ) {
        //fprintf(stderr, "can't read 'dir '%s', errno=%d\n", inputPath.c_str(), errno);
        return;
    }
    while (dirent* entry = readdir(dir)) {
        struct stat statBuf;
        std::string dirAndFile = path + "/" + entry->d_name;
        std::string fullDirAndFile = pathPrefix + dirAndFile;
         switch ( entry->d_type ) {
            case DT_REG:
                if ( processFiles ) {
                    if ( ::lstat(fullDirAndFile.c_str(), &statBuf) == -1 )
                        break;
                    if ( ! S_ISREG(statBuf.st_mode)  )
                        break;
                    fileCallback(dirAndFile, statBuf);
                }
                break;
            case DT_DIR:
                if ( strcmp(entry->d_name, ".") == 0 )
                    break;
                if ( strcmp(entry->d_name, "..") == 0 )
                    break;
                if ( dirFilter(dirAndFile) )
                    break;
                if (recurse)
                    iterateDirectoryTree(pathPrefix, dirAndFile, dirFilter, fileCallback, processFiles, true);
                break;
            case DT_LNK:
                // don't follow symlinks, dylib will be found through absolute path
                break;
        }
    }
    ::closedir(dir);
}


bool safeSave(const void* buffer, size_t bufferLen, const std::string& path)
{
    std::string pathTemplate = path + "-XXXXXX";
    size_t templateLen = strlen(pathTemplate.c_str())+2;
    char pathTemplateSpace[templateLen];
    strlcpy(pathTemplateSpace, pathTemplate.c_str(), templateLen);
    int fd = mkstemp(pathTemplateSpace);
    if ( fd != -1 ) {
        ssize_t writtenSize = pwrite(fd, buffer, bufferLen, 0);
        if ( (size_t)writtenSize == bufferLen ) {
            ::fchmod(fd, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH); // mkstemp() makes file "rw-------", switch it to "rw-r--r--"
            if ( ::rename(pathTemplateSpace, path.c_str()) == 0) {
                ::close(fd);
                return true; // success
            }
        }
        ::close(fd);
        ::unlink(pathTemplateSpace);
    }
    return false; // failure
}

const void* mapFileReadOnly(const char* path, size_t& mappedSize)
{
    struct stat statBuf;
    if ( ::stat(path, &statBuf) != 0 )
        return nullptr;

    int fd = ::open(path, O_RDONLY);
    if ( fd < 0 )
        return nullptr;

    const void *p = ::mmap(NULL, (size_t)statBuf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    ::close(fd);
    if ( p != MAP_FAILED ) {
        mappedSize = (size_t)statBuf.st_size;
        return p;
    }

    return nullptr;
}

static bool sipIsEnabled()
{
    static bool             rootlessEnabled;
    static dispatch_once_t  onceToken;
    // Check to make sure file system protections are on at all
    dispatch_once(&onceToken, ^{
        rootlessEnabled = (csr_check(CSR_ALLOW_UNRESTRICTED_FS) != 0);
    });
    return rootlessEnabled;
}

bool isProtectedBySIP(const std::string& path)
{
    if ( !sipIsEnabled() )
        return false;

    return (rootless_check_trusted(path.c_str()) == 0);
}

bool isProtectedBySIPExceptDyld(const std::string& path)
{
    if ( !sipIsEnabled() )
        return false;

    return (rootless_check_trusted_class(path.c_str(), "dyld") == 0);
}

bool isProtectedBySIP(int fd)
{
    if ( !sipIsEnabled() )
        return false;

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
    return (rootless_check_trusted_fd(fd) == 0);
#else
    // fallback to using rootless_check_trusted
    char realPath[MAXPATHLEN];
    if ( fcntl(fd, F_GETPATH, realPath) == 0 )
        return (rootless_check_trusted(realPath) == 0);
    return false;
#endif
}

bool fileExists(const std::string& path)
{
    struct stat statBuf;
    return ( ::stat(path.c_str(), &statBuf) == 0 );
}

// There is an order file specifying the order in which dylibs are laid out in
// general, as well as an order file specifying the order in which __DATA_DIRTY
// segments are laid out in particular.
//
// The syntax is one dylib (install name) per line.  Blank lines are ignored.
// Comments start with the # character.
std::unordered_map<std::string, uint32_t> parseOrderFile(const std::string& orderFileData) {
    std::unordered_map<std::string, uint32_t> order;

    if (orderFileData.empty())
        return order;

    std::stringstream myData(orderFileData);

    uint32_t count = 0;
    std::string line;
    while ( std::getline(myData, line) ) {
        size_t pos = line.find('#');
        if ( pos != std::string::npos )
            line.resize(pos);
        while ( !line.empty() && isspace(line.back()) ) {
            line.pop_back();
        }
        if ( !line.empty() )
            order[line] = count++;
    }
    return order;
}

std::string loadOrderFile(const std::string& orderFilePath) {
    std::string order;

    size_t size = 0;
    char* data = (char*)mapFileReadOnly(orderFilePath.c_str(), size);
    if (data) {
        order = std::string(data, size);
        ::munmap((void*)data, size);
    }

    return order;
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
    //warning("tool directory not found");
    return "/tmp/";
}

std::string basePath(const std::string& path)
{
    std::string::size_type slash_pos = path.rfind("/");
    if (slash_pos != std::string::npos) {
        slash_pos++;
        return path.substr(slash_pos);
    } else {
        return path;
    }
}

std::string dirPath(const std::string& path)
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

std::string realPath(const std::string& path)
{
    char resolvedPath[PATH_MAX];
    if (realpath(dirPath(path).c_str(), &resolvedPath[0]) != nullptr) {
        return std::string(resolvedPath) + "/" + basePath(path);
    } else {
        return "";
    }
}

std::string realFilePath(const std::string& path)
{
    char resolvedPath[PATH_MAX];
    if ( realpath(path.c_str(), resolvedPath) != nullptr )
        return std::string(resolvedPath);
    else
        return "";
}


std::string normalize_absolute_file_path(std::string path) {
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


#if BUILDING_CACHE_BUILDER

FileCache fileCache;

FileCache::FileCache(void)
{
    cache_queue = dispatch_queue_create("com.apple.dyld.cache.cache", dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL, QOS_CLASS_USER_INITIATED, 0));
}

std::pair<uint8_t*, struct stat> FileCache::cacheLoad(Diagnostics& diags, const std::string path)
{
    __block bool found = false;
    __block std::pair<uint8_t*, struct stat> retval;
    std::string normalizedPath = normalize_absolute_file_path(path);
    dispatch_sync(cache_queue, ^{
        auto entry = entries.find(normalizedPath);
        if (entry != entries.end()) {
            retval = entry->second;
            found = true;
        }
    });

    if (!found) {
        auto info = fill(diags, normalizedPath);
        dispatch_sync(cache_queue, ^{
            auto entry = entries.find(normalizedPath);
            if (entry != entries.end()) {
                retval = entry->second;
            } else {
                retval = entries[normalizedPath] = info;
                retval = info;
            }
        });
    }

    return retval;
}

//FIXME error handling
std::pair<uint8_t*, struct stat> FileCache::fill(Diagnostics& diags, const std::string& path)
{
    void* buffer_ptr = nullptr;
    struct stat stat_buf;
    struct statfs statfs_buf;
    bool localcopy = true;

    int fd = ::open(path.c_str(), O_RDONLY, 0);
    if (fd == -1) {
        diags.verbose("can't open file '%s', errno=%d\n", path.c_str(), errno);
        return std::make_pair((uint8_t*)(-1), stat_buf);
    }

    if (fstat(fd, &stat_buf) == -1) {
        diags.verbose("can't stat open file '%s', errno=%d\n", path.c_str(), errno);
        ::close(fd);
        return std::make_pair((uint8_t*)(-1), stat_buf);
    }

    if (stat_buf.st_size < 4096) {
        diags.verbose("file too small '%s'\n", path.c_str());
        ::close(fd);
        return std::make_pair((uint8_t*)(-1), stat_buf);
    }

    if(fstatfs(fd, &statfs_buf) == 0) {
        std::string fsName = statfs_buf.f_fstypename;
        if (fsName == "hfs" || fsName == "apfs") {
            localcopy = false;
        }
    }
    
    if (!localcopy) {
        buffer_ptr = mmap(NULL, (size_t)stat_buf.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (buffer_ptr == MAP_FAILED) {
            diags.verbose("mmap() for file at %s failed, errno=%d\n", path.c_str(), errno);
            ::close(fd);
            return std::make_pair((uint8_t*)(-1), stat_buf);
        }
    } else {
        buffer_ptr = malloc((size_t)stat_buf.st_size);
        ssize_t readBytes = pread(fd, buffer_ptr, (size_t)stat_buf.st_size, 0);
        if (readBytes == -1) {
            diags.verbose("Network read for file at %s failed, errno=%d\n", path.c_str(), errno);
            ::close(fd);
            return std::make_pair((uint8_t*)(-1), stat_buf);
        } else if (readBytes != stat_buf.st_size) {
            diags.verbose("Network read udnerrun for file at %s, expected %lld bytes, got  %zd bytes\n", path.c_str(), stat_buf.st_size, readBytes);
            ::close(fd);
            return std::make_pair((uint8_t*)(-1), stat_buf);
        }
    }

    ::close(fd);

    return std::make_pair((uint8_t*)buffer_ptr, stat_buf);
}

static void normalizePath(std::string& path) {
    // Remove a bunch of stuff we don't need, like trailing slashes.
    while ( !path.empty() && (path.back() == '/'))
        path.pop_back();
}

void SymlinkResolver::addFile(Diagnostics& diags, std::string path) {
    if (path.front() != '/') {
        diags.error("Path must start with '/'");
        return;
    }
    if (symlinks.find(path) != symlinks.end()) {
        diags.error("Cannot add regular file as it is already a symlink");
        return;
    }
    filePaths.insert(path);
}

void SymlinkResolver::addSymlink(Diagnostics& diags, std::string fromPath, std::string toPath) {
    normalizePath(fromPath);
    normalizePath(toPath);
    if (fromPath.front() != '/') {
        diags.error("Path must start with '/'");
        return;
    }
    if (filePaths.find(fromPath) != filePaths.end()) {
        diags.error("Cannot add symlink from '%s' as it is already a regular path", fromPath.c_str());
        return;
    }
    auto itAndInserted = symlinks.insert({ fromPath, toPath });
    if (!itAndInserted.second) {
        // The path is already a symlink.  Make sure its a dupe.
        if (toPath != itAndInserted.first->second) {
            diags.error("Duplicate symlink for path '%s'", fromPath.c_str());
            return;
        }
    }
}

std::string SymlinkResolver::realPath(Diagnostics& diags, const std::string& originalPath) const {
    // First make sure the path doesn't have any magic in it.
    std::string path = originalPath;
    normalizePath(path);

    std::set<std::string> seenSymlinks;

    // Now see if any prefix is a symlink
    if (path.front() != '/')
        return path;

    std::string::size_type prev_pos = 0;
    while (prev_pos != std::string::npos) {
        std::string::size_type pos = path.find("/", prev_pos + 1);

        // First look to see if this path component is special, eg, ., .., etc.
        std::string component = path.substr(prev_pos, pos - prev_pos);
        if (component == "/..") {
            // Fold with the previous path component.
            if (prev_pos == 0) {
                // This is the root path, and .. applied to / is just /
                path = path.substr(3);
                prev_pos = 0;
            } else {
                std::string::size_type lastSlashPos = path.rfind("/", prev_pos - 1);
                path = path.substr(0, lastSlashPos) + path.substr(pos);
                prev_pos = lastSlashPos;
            }
            continue;
        } else if (component == "/.") {
            if (prev_pos == 0) {
                // Path starts with /./ so just remove the first one.
                path = path.substr(2);
            } else {
                if (pos == std::string::npos) {
                    // Trailing . on the path
                    path = path.substr(0, prev_pos );
                } else {
                    path = path.substr(0, prev_pos) + path.substr(pos);
                }
            }
            continue;
        } else if (component == "/") {
            // Path must contain // somewhere so strip out the duplicates.
            if (prev_pos == 0) {
                // Path starts with // so just remove the first one.
                path = path.substr(1);
            } else {
                if (pos == std::string::npos) {
                    // Trailing / on the path
                    path = path.substr(0, prev_pos);
                    prev_pos = pos;
                } else {
                    path = path.substr(0, pos) + path.substr(pos + 1);
                }
            }
            continue;
        }

        // Path is not special, so see if it is a symlink to something.
        std::string prefix = path.substr(0, pos);
        //printf("%s\n", prefix.c_str());
        auto it = symlinks.find(prefix);
        if (it == symlinks.end()) {
            // This is not a symlink so move to the next prefix.
            prev_pos = pos;
            continue;
        }

        // If we've already done this prefix then error out.
        if (seenSymlinks.count(prefix)) {
            diags.error("Loop in symlink processing for '%s'", originalPath.c_str());
            return std::string();
        }

        seenSymlinks.insert(prefix);

        // This is a symlink, so resolve the new path.
        std::string toPath = it->second;
        if (toPath.front() == '/') {
            // Symlink points to an absolute address so substitute the whole prefix for the new path
            // If we didn't substitute the last component of the path then there is also a path suffix.
            std::string pathSuffix = "";
            if (pos != std::string::npos) {
                std::string::size_type nextSlashPos = path.find("/", pos + 1);
                if (nextSlashPos != std::string::npos)
                    pathSuffix = path.substr(nextSlashPos);
            }
            path = toPath + pathSuffix;
            prev_pos = 0;
            continue;
        }

        // Symlink points to a relative path so we need to do more processing to get the real path.

        // First calculate which part of the previous prefix we'll keep.  Eg, in /a/b/c where "b -> blah", we want to keep /a here.
        std::string prevPrefix = path.substr(0, prev_pos);
        //printf("prevPrefix %s\n", prevPrefix.c_str());

        // If we didn't substitute the last component of the path then there is also a path suffix.
        std::string pathSuffix = "";
        if (prefix.size() != path.size())
            pathSuffix = path.substr(pos);

        // The new path is the remaining prefix, plus the symlink target, plus any remaining suffix from the original path.
        path = prevPrefix + "/" + toPath + pathSuffix;
        prev_pos = 0;
    }
    return path;
}

std::vector<DyldSharedCache::FileAlias> SymlinkResolver::getResolvedSymlinks(Diagnostics& diags) {
    diags.assertNoError();
    std::vector<DyldSharedCache::FileAlias> aliases;
    for (auto& fromPathAndToPath : symlinks) {
        std::string newPath = realPath(diags, fromPathAndToPath.first);
        if (diags.hasError()) {
            aliases.clear();
            return aliases;
        }

        if (filePaths.count(newPath)) {
            aliases.push_back({ newPath, fromPathAndToPath.first });
            // printf("symlink ('%s' -> '%s') resolved to '%s'\n", fromPathAndToPath.first.c_str(), fromPathAndToPath.second.c_str(), newPath.c_str());
        }
    }
    return aliases;
}

#endif // BUILDING_CACHE_BUILDER


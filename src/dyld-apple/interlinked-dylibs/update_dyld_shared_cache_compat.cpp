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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <signal.h>
#include <errno.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <dirent.h>

#include <vector>
#include <set>
#include <map>

#include "mega-dylib-utils.h"

/*
 This is a compatibility driver to allow us to support migrating to the new cache codebase and format without forcing B&I to alter gencaches.
 This tool only supports flags used by B&I
 This intention is for this tool to be removed in the near future and replaced with a tool that is not commandline compatible,
   but allows shared caches to be built directly out of BuildRecords.
 */

/* 
   Example commandline:
   [60042] INFO - Executing Command: /SWE/Teams/DT-SWB/iOS/Binaries/Binaries5/update_dyld_shared_cache/update_dyld_shared_cache-357.0.91~2/Root/Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/usr/local/bin/update_dyld_shared_cache
	-debug
	-root /BuildRoot//Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS9.0.Internal.sdk
    -dylib_list /tmp/dylibs.K93aInternalOS.60042
	-cache_dir /tmp/K93aInternalOS.60042
    -arch armv7
    -iPhone
	-dont_map_local_symbols
	-overlay /SWE/Teams/DT-SWB/iOS/Binaries/Binaries5/XPCService_caches/XPCService_caches-119~94/Root/K93aInternalOS/
	-overlay /SWE/Teams/DT-SWB/iOS/Binaries/Binaries5/libxpc_caches/libxpc_caches-649~18/Root/K93aInternalOS/
 */

// record warnings and add to .map file
static std::vector<std::unique_ptr<const char>> sWarnings;


static void write_cache(const char* cachePath, SharedCache& cache) {
	// create temp file for cache
	int fd = ::open(cachePath, O_CREAT | O_RDWR | O_TRUNC, 0644);
	if ( fd == -1 )
		terminate("can't create temp file %s, errnor=%d", cachePath, errno);

	// try to allocate whole cache file contiguously
	fstore_t fcntlSpec = { F_ALLOCATECONTIG|F_ALLOCATEALL, F_PEOFPOSMODE, 0, static_cast<off_t>(cache.fileSize()), 0 };
	::fcntl(fd, F_PREALLOCATE, &fcntlSpec);

	// write out cache file
	if ( ::pwrite(fd, cache.buffer().get(), cache.fileSize(), 0) != cache.fileSize() )
	terminate("write() failure creating cache file, errno=%d", errno);

	// flush to disk and close
	int result = ::fcntl(fd, F_FULLFSYNC, NULL);
	if ( result == -1 )
	warning("fcntl(F_FULLFSYNC) failed with errno=%d for %s\n", errno, cachePath);
	::close(fd);

	// write .map file
	char mapPath[MAXPATHLEN];
	strlcpy(mapPath, cachePath, MAXPATHLEN);
	strlcat(mapPath, ".map", MAXPATHLEN);
	cache.writeCacheMapFile(mapPath);

	// append any warnings encountered
	if ( !sWarnings.empty() ) {
		FILE* fmap = ::fopen(mapPath, "a");
		if ( fmap != NULL ) {
			fprintf(fmap, "\n\n");
			for ( std::unique_ptr<const char>& msg : sWarnings ) {
				fprintf(fmap, "# %s", &*msg);
			}
			::fclose(fmap);
		}
	}
}

int main(int argc, const char* argv[])
{
	std::set<ArchPair> onlyArchs;
	const char *rootPath = NULL;
	std::vector<const char*> searchPaths;
	std::vector<std::unique_ptr<DylibProxy>> dylibs;
	const char* dylibListFile = NULL;
//	bool force = false;
//	bool keepSignatures = false;
	bool explicitCacheDir = false;
	bool dontMapLocalSymbols = false;
	bool verbose = false;
	bool iPhoneOS = false;
	const char* cacheDir = NULL;
	const char* archStr = NULL;
	ArchPair archPair(CPU_TYPE_ARM64, CPU_SUBTYPE_ARM64_ALL);
	
	// parse command line options
	for(int i=1; i < argc; ++i) {
		const char* arg = argv[i];
		if ( arg[0] == '-' ) {
			if ( strcmp(arg, "-debug") == 0 ) {
				verbose = true;
			}
			else if ( strcmp(arg, "-dont_map_local_symbols") == 0 ) {
				dontMapLocalSymbols = true;
			}
			else if ( strcmp(arg, "-iPhone") == 0 ) {
				iPhoneOS = true;
			}
			else if ( strcmp(arg, "-dylib_list") == 0 ) {
				dylibListFile = argv[++i];
				if ( dylibListFile == NULL )
					terminate("-dylib_list missing path argument\n");
			}
			else if ( (strcmp(arg, "-root") == 0) || (strcmp(arg, "--root") == 0) ) {
				rootPath = argv[++i];
			}
			else if ( strcmp(arg, "-overlay") == 0 ) {
				const char* path = argv[++i];
				if ( path == NULL )
					terminate("-overlay missing path argument\n");
				searchPaths.push_back(path);
			}
			else if ( strcmp(arg, "-cache_dir") == 0 ) {
				cacheDir = argv[++i];
				if ( cacheDir == NULL )
					terminate("-cache_dir missing path argument\n");
				explicitCacheDir = true;
			}
			else if ( strcmp(arg, "-arch") == 0 ) {
				archStr = argv[++i];
				archPair = archForString(archStr);  // terminates if unknown
			}
			else if ( strcmp(arg, "-force") == 0 ) {
				// ignore.  always forced for iOS
			}
			else {
				//usage();
				terminate("unknown option: %s\n", arg);
			}
		}
		else {
			//usage();
			terminate("unknown option: %s\n", arg);
		}
	}
	
	if (!rootPath) {
		terminate("-root is a required option\n");
	}
	if (!iPhoneOS) {
		terminate("-iPhone is a required option\n");
	}
	if (!cacheDir) {
		terminate("-cache_dir is a required option\n");
	}
	if (!dylibListFile) {
		terminate("-dylib_list is a required option\n");
	}
	if (!archStr) {
		terminate("-arch is a required option\n");
	}
	
	char prodCachePath[MAXPATHLEN];
	char devCachePath[MAXPATHLEN];
	strcpy(prodCachePath, cacheDir);
	if ( prodCachePath[strlen(prodCachePath)-1] != '/' )
		strcat(prodCachePath, "/");
	strcat(prodCachePath, "dyld_shared_cache_");
	strcat(prodCachePath, archStr);
	strcpy(devCachePath, prodCachePath);
	strcat(devCachePath, ".development");
	
	verboseLog("developement cache path = %s\n", devCachePath);
	verboseLog("cache path = %s\n", prodCachePath);

	// create cache dirs if needed
	char neededDirs[1024];
	strcpy(neededDirs, prodCachePath);
	char* lastSlash = strrchr(neededDirs, '/');
	if ( lastSlash != NULL )
		lastSlash[1] = '\0';
	struct stat stat_buf;
	if ( stat(neededDirs, &stat_buf) != 0 ) {
		const char* afterSlash = &neededDirs[1];
		char* slash;
		while ( (slash = strchr(afterSlash, '/')) != NULL ) {
			*slash = '\0';
			::mkdir(neededDirs, S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH);
			*slash = '/';
			afterSlash = slash+1;
		}
	}

	std::string dylibOrderFile = toolDir() + "/dylib-order.txt";
	std::string dirtyDataOrderFile = toolDir() + "/dirty-data-segments-order.txt";

	std::unordered_map<std::string, std::unordered_set<std::string>> dependents;
	SharedCache devCache(rootPath, searchPaths, dylibListFile, archPair, dylibOrderFile, dirtyDataOrderFile);

	if ( devCache.fileSize() == 0 )
		terminate("Could not find all necessary dylibs\n");
	
	devCache.buildUnoptimizedCache();

	SharedCache prodCache = devCache;

	prodCache.optimizeForProduction();
	devCache.optimizeForDevelopment();

	verboseLog("developement cache size = %llu", devCache.fileSize());
	verboseLog("developement cache vm size = %llu", devCache.vmSize());

	// optimize cache
	write_cache(devCachePath, devCache);
	if ( devCache.vmSize()+align(devCache.vmSize()/200, sharedRegionRegionAlignment(archPair)) > sharedRegionRegionSize(archPair)) {
		terminate("update_dyld_shared_cache[%u] for arch=%s, shared cache will not fit in shared regions address space.  Overflow amount: %llu\n",
					getpid(), archStr, devCache.vmSize()+align(devCache.vmSize()/200, sharedRegionRegionAlignment(archPair)) - sharedRegionRegionSize(archPair));
	}

	write_cache(prodCachePath, prodCache);

	return 0;
}


void uniqueWarning(const char* format, ...)
{
	va_list	list;
	va_start(list, format);
	vfprintf(stderr, format, list);
	va_end(list);
	fprintf(stderr, "\n");
}

void log(const char * __restrict format, ...)
{
	va_list	list;
	va_start(list, format);
	vfprintf(stderr, format, list);
	va_end(list);
	fprintf(stderr, "\n");
}
void verboseLog(const char* format, ...)
{
	va_list	list;
	va_start(list, format);
	vfprintf(stderr, format, list);
	va_end(list);
	fprintf(stderr, "\n");
}

void warning(const char* format, ...)
{
	fprintf(stderr, "update_dyld_shared_cache warning: ");
	va_list	list;
	va_start(list, format);
	char* msg;
	vasprintf(&msg, format, list);
	va_end(list);
	fprintf(stderr, "%s\n", msg);
	sWarnings.push_back(std::unique_ptr<const char>(msg));
}

void terminate(const char* format, ...)
{
	fprintf(stderr, "update_dyld_shared_cache error: ");
	va_list	list;
	va_start(list, format);
	vfprintf(stderr, format, list);
	va_end(list);
	exit(1);
}

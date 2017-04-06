/* -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
 *
 * Copyright (c) 2016 Apple Inc. All rights reserved.
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

#include <dispatch/dispatch.h>
#include <Security/Security.h>
#include <Security/SecCodeSigner.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/resource.h>
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
#include <libgen.h>
#include <pthread.h>

#include <vector>
#include <array>
#include <set>
#include <map>
#include <algorithm>

#include "MultiCacheBuilder.h"
#include "Manifest.h"

#include "mega-dylib-utils.h"
#include "Logging.h"

#if !__has_feature(objc_arc)
#error The use of libdispatch in this files requires it to be compiled with ARC in order to avoid leaks
#endif

static dispatch_queue_t build_queue = dispatch_queue_create("com.apple.dyld.build", DISPATCH_QUEUE_CONCURRENT);
static dispatch_group_t build_group = dispatch_group_create();
static dispatch_semaphore_t writeLimitingSemaphore = dispatch_semaphore_create(1);

bool copyFile(const std::string& from, const std::string& to)
{
    const uint8_t* p = (uint8_t*)(-1);
    struct stat    stat_buf;
    bool           rp;

    std::tie(p, stat_buf, rp) = fileCache.cacheLoad(from);
    if (p == (uint8_t*)(-1))
        return false;

    dispatch_group_enter(build_group);
    mkpath_np(dirpath(to).c_str(), 0755);

    dispatch_semaphore_wait(writeLimitingSemaphore, DISPATCH_TIME_FOREVER);
    int fd = open(to.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd > 0) {
        ssize_t writtenSize = pwrite(fd, p, stat_buf.st_size, 0);
        if (writtenSize != stat_buf.st_size)
            terminate("write() failure creating cache file, errno=%d (%s)", errno, strerror(errno));

        ::close(fd);
        verboseLog("Wrote out: %s", to.c_str());
    } else {
        terminate("can't create file '%s', errnor=%d (%s)", to.c_str(), errno, strerror(errno));
    }
    dispatch_semaphore_signal(writeLimitingSemaphore);
    dispatch_group_leave(build_group);

    return true;
}

#define kDylibCachePrefix "/AppleInternal/Developer/DylibCaches/"

void createArtifact(Manifest& manifest, const std::string& dylibCachePath, bool includeExecutables)
{
    mkpath_np(dylibCachePath.c_str(), 0755);
    (void)copyFile(manifest.dylibOrderFile, dylibCachePath + "Metadata/dylibOrderFile.txt");
    (void)copyFile(manifest.dirtyDataOrderFile, dylibCachePath + "Metadata/dirtyDataOrderFile.txt");
    (void)copyFile(manifest.metabomFile, dylibCachePath + "Metadata/metabom.bom");

    std::set<std::string> copied;

    for (auto archFiles : manifest.architectureFiles) {
        for (auto& file : archFiles.second.dylibs) {
            std::string installname = file.first;
            if (copied.count(installname) > 0) {
                continue;
            }
            (void)copyFile(file.second.proxy->path, normalize_absolute_file_path(dylibCachePath + "/Root/" + file.first));
            copied.insert(installname);
        }
        if (includeExecutables) {
            for (auto& file : archFiles.second.executables) {
                std::string installname = file.first;
                if (copied.count(installname) > 0) {
                    continue;
                }
                (void)copyFile(file.second.proxy->path, normalize_absolute_file_path(dylibCachePath + "/Root/" + file.first));
                copied.insert(installname);
            }
        }
    }

    log("Artifact dylibs copied");
}

void addArtifactPaths(Manifest &manifest) {
	manifest.dylibOrderFile = "./Metadata/dylibOrderFile.txt";
	manifest.dirtyDataOrderFile = "./Metadata/dirtyDataOrderFile.txt";
        manifest.metabomFile = "./Metadata/metabom.bom";

        for ( auto& projects : manifest.projects ) {
            if ( projects.second.sources[0] != "./Root/" ) {
                projects.second.sources.insert( projects.second.sources.begin(), "./Root/" );
            }
        }
}

int main (int argc, const char * argv[]) {
	@autoreleasepool {
        auto defaultCtx = std::make_shared<LoggingContext>("");
        setLoggingContext(defaultCtx);

        Manifest    manifest;
        std::string masterDstRoot;
        std::string dylibCacheDir;
        std::string resultPath;
        bool        skipWrites = false;
        bool        skipBuilds = false;
        bool        preflight = false;
        std::string manifestPath;

        time_t mytime = time(0);
        log("Started: %s", asctime(localtime(&mytime)));

        // parse command line options
        for (int i = 1; i < argc; ++i) {
            const char* arg = argv[i];
            if (arg[0] == '-') {
                if (strcmp(arg, "-debug") == 0) {
                    setVerbose(true);
                } else if (strcmp(arg, "-skip_writes") == 0) {
                    skipWrites = true;
                } else if (strcmp(arg, "-skip_builds") == 0) {
                    skipBuilds = true;
                } else if (strcmp(arg, "-delete_writes") == 0) {
                    skipWrites = true;
                } else if (strcmp(arg, "-dylib_cache") == 0) {
                    dylibCacheDir = argv[++i];
                } else if (strcmp(arg, "-preflight") == 0) {
                    preflight = true;
                    skipWrites = true;
                } else if (strcmp(arg, "-master_dst_root") == 0) {
                    masterDstRoot = argv[++i];
                    if (masterDstRoot.empty()) {
                        terminate("-master_dst_root missing path argument");
                    }
                } else if (strcmp(arg, "-results") == 0) {
                    resultPath = argv[++i];
                } else if (strcmp(arg, "-plist") == 0) {
                    manifestPath = argv[++i];
                    (void)chdir(dirname(strdup(manifestPath.c_str())));
                    manifest = Manifest(manifestPath);
                } else {
                    // usage();
                    terminate("unknown option: %s", arg);
                }
            } else {
                manifestPath = argv[i];

                (void)chdir(dirname(strdup(argv[i])));
            }
		}

        if ( getenv( "LGG_SKIP_CACHE_FUN" ) != nullptr ) {
            skipBuilds = true;
        }

		if (manifest.build.empty()) {
			terminate("No version found in manifest");
		}
		log("Building Caches for %s", manifest.build.c_str());

		if ( masterDstRoot.empty() ) {
			terminate("-master_dst_root required path argument");
		}

		if (manifest.manifest_version < 4) {
			terminate("must specify valid manifest file");
		}

		struct rlimit rl = {OPEN_MAX, OPEN_MAX};
		(void)setrlimit(RLIMIT_NOFILE, &rl);

		if (!skipWrites) {
			(void)mkpath_np((masterDstRoot + "/Boms/").c_str(), 0755);
		}

		if (manifest.dylibOrderFile.empty()) {
			manifest.dylibOrderFile = toolDir() + "/dylib-order.txt";
		}

		if (manifest.dirtyDataOrderFile.empty()) {
			manifest.dirtyDataOrderFile = toolDir() + "/dirty-data-segments-order.txt";
		}

        auto dylibCacheCtx = std::make_shared<LoggingContext>("DylibCache");
        setLoggingContext(dylibCacheCtx);

        if (!skipWrites) {
            cacheBuilderDispatchGroupAsync(build_group, build_queue, [&] {
                createArtifact(manifest, masterDstRoot + "/Artifact.dlc/", true);
            });

            if (!dylibCacheDir.empty()) {
                cacheBuilderDispatchGroupAsync(build_group, build_queue, [&] {
                    createArtifact(manifest, dylibCacheDir + "/AppleInternal/Developer/DylibCaches/" + manifest.build + ".dlc/", false);
                });
            }
        }
        setLoggingContext(defaultCtx);

        manifest.calculateClosure(false);
        std::shared_ptr<MultiCacheBuilder> builder = std::make_shared<MultiCacheBuilder>(manifest, true, skipWrites, false, skipBuilds);
        dispatch_group_async(build_group, build_queue, [&] { builder->buildCaches(masterDstRoot); });
        dispatch_group_wait(build_group, DISPATCH_TIME_FOREVER);

        if (!preflight) {
            setLoggingContext(dylibCacheCtx);
            addArtifactPaths(manifest);
            setLoggingContext(defaultCtx);

            if (!resultPath.empty()) {
                manifest.write(resultPath);
            }

            setLoggingContext(dylibCacheCtx);
            copyFile(manifestPath, masterDstRoot + "/Artifact.dlc/BNIManifest.plist");
            manifest.write(masterDstRoot + "/Artifact.dlc/Manifest.plist");

            if (!dylibCacheDir.empty()) {
                manifest.write(dylibCacheDir + kDylibCachePrefix + manifest.build + ".dlc/Manifest.plist");
            }
            setLoggingContext(defaultCtx);
        }

        builder->logStats();

        dumpLogAndExit();
	}

	dispatch_main();

	return 0;
}



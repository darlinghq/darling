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

#include <Bom/Bom.h>
#include <dispatch/dispatch.h>
#include <copyfile.h>

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

#include "Manifest.h"
#include "FileUtils.h"
#include "BuilderUtils.h"

#define CACHE_BUILDER_COPY_FILE_MODE COPYFILE_ALL

#if !__has_feature(objc_arc)
#error The use of libdispatch in this files requires it to be compiled with ARC in order to avoid leaks
#endif

static dispatch_queue_t build_queue = dispatch_queue_create("com.apple.dyld.cache-builder.build", DISPATCH_QUEUE_CONCURRENT);

#define kDylibCachePrefix "/AppleInternal/Developer/DylibCaches/"

void createArtifact(Diagnostics& diags, dyld3::Manifest& manifest, const std::string& dylibCachePath, bool includeExecutables)
{
    auto copy_state = copyfile_state_alloc();
    mkpath_np((dylibCachePath + "/Metadata").c_str(), 0755);
    (void)copyfile(manifest.metabomFile().c_str(), (dylibCachePath + "/Metadata/metabom.bom").c_str(), copy_state, CACHE_BUILDER_COPY_FILE_MODE);

    if (!manifest.dylibOrderFile().empty()) {
        (void)copyfile(realPath(manifest.dylibOrderFile()).c_str(), (dylibCachePath + "/Metadata/dylibOrderFile.txt").c_str(), copy_state, CACHE_BUILDER_COPY_FILE_MODE);
    }

    if (!manifest.dirtyDataOrderFile().empty()) {
        (void)copyfile(realPath(manifest.dirtyDataOrderFile()).c_str(), (dylibCachePath + "/Metadata/dirtyDataOrderFile.txt").c_str(), copy_state, CACHE_BUILDER_COPY_FILE_MODE);
    }

    std::set<dyld3::UUID> uuids;
    std::map<std::string, std::string> copy_pairs;

    manifest.forEachConfiguration([&manifest, &uuids](const std::string& config) {
        manifest.configuration(config).forEachArchitecture([&manifest, &config, &uuids](const std::string& arch) {
            auto results = manifest.configuration(config).architecture(arch).results;
            for (const auto& image : results.dylibs) {
                uuids.insert(image.first);
            }
            for (const auto& image : results.bundles) {
                uuids.insert(image.first);
            }
            for (const auto& image : results.executables) {
                uuids.insert(image.first);
            }
        });
    });

    for (auto& uuid : uuids) {
        auto buildPath = manifest.buildPathForUUID(uuid);
        auto installPath = manifest.runtimePathForUUID(uuid);
        assert(!buildPath.empty() && !installPath.empty());
        copy_pairs.insert(std::make_pair(installPath, buildPath));
    }

    for (const auto& copy_pair : copy_pairs) {
        std::string from = realPath(copy_pair.second);
        std::string to = dylibCachePath + "/Root/" + copy_pair.first;
        mkpath_np(dirPath(to).c_str(), 0755);
        int err = copyfile(from.c_str(), to.c_str(), copy_state, CACHE_BUILDER_COPY_FILE_MODE);
        diags.verbose("COPYING (%d) %s -> %s\n", err, from.c_str(), to.c_str());

    }
    copyfile_state_free(copy_state);

    fprintf(stderr, "[Artifact] dylibs copied\n");
}

void addArtifactPaths(Diagnostics& diags, dyld3::Manifest& manifest)
{
    manifest.setDylibOrderFile("./Metadata/dylibOrderFile.txt");
    manifest.setDirtyDataOrderFile("./Metadata/dirtyDataOrderFile.txt");
    manifest.setMetabomFile("./Metadata/metabom.bom");

    for (auto& projects : manifest.projects()) {
        manifest.addProjectSource(projects.first, "./Root", true);
    }
}

int main(int argc, const char* argv[])
{
    @autoreleasepool {
        __block Diagnostics diags;
        bool                verbose = false;
        std::string         masterDstRoot;
        std::string         dylibCacheDir;
        std::string         resultPath;
        std::string         manifestPath;
        bool                preflight = false;
        __block bool        allBuildsSucceeded = true;
        bool                skipWrites = false;
        bool                skipBuilds = false;
        bool                agileChooseSHA256CdHash = false;
        time_t              mytime = time(0);
        fprintf(stderr, "Started: %s", asctime(localtime(&mytime)));

        // parse command line options
        for (int i = 1; i < argc; ++i) {
            const char* arg = argv[i];
            if (arg[0] == '-') {
                if (strcmp(arg, "-debug") == 0) {
                    verbose = true;
                    diags = Diagnostics(true);
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
                        diags.error("-master_dst_root missing path argument");
                    }
                } else if (strcmp(arg, "-results") == 0) {
                    resultPath = argv[++i];
                } else if (strcmp(arg, "-plist") == 0) {
                    manifestPath = argv[++i];
                } else if (strcmp(arg, "-agile_choose_sha256_cdhash") == 0) {
                    agileChooseSHA256CdHash = true;
                } else {
                    // usage();
                    diags.error("unknown option: %s", arg);
                }
            } else {
                manifestPath = argv[i];
            }
        }

        if (getenv("LGG_SKIP_CACHE_FUN") != nullptr) {
            skipBuilds = true;
        }

        if (diags.hasError()) {
            printf("%s\n", diags.errorMessage().c_str());
            exit(-1);
        }

        dispatch_async(dispatch_get_main_queue(), ^{
            if (manifestPath.empty()) {
                fprintf(stderr, "mainfest path argument is required\n");
                exit(-1);
            }

            (void)chdir(dirname(strdup(manifestPath.c_str())));
            __block auto manifest = dyld3::Manifest(diags, manifestPath);

            if (manifest.build().empty()) {
                fprintf(stderr, "No version found in manifest\n");
                exit(-1);
            }

            fprintf(stderr, "Building Caches for %s\n", manifest.build().c_str());

            if (masterDstRoot.empty()) {
                fprintf(stderr, "-master_dst_root required path argument\n");
                exit(-1);
            }

            if (manifest.version() < 4) {
                fprintf(stderr, "must specify valid manifest file\n");
                exit(-1);
            }

            struct rlimit rl = { OPEN_MAX, OPEN_MAX };
            (void)setrlimit(RLIMIT_NOFILE, &rl);

            manifest.calculateClosure();

            if (!skipWrites && !skipBuilds) {
                (void)mkpath_np((masterDstRoot + "/Boms/").c_str(), 0755);
                dispatch_group_async(buildGroup(), build_queue, ^{
                    createArtifact(diags, manifest, masterDstRoot + "/Artifact.dlc/", true);
                });
            }

            if (!dylibCacheDir.empty()) {
                dispatch_group_async(buildGroup(), build_queue, ^{
                    createArtifact(diags, manifest, dylibCacheDir + "/AppleInternal/Developer/DylibCaches/" + manifest.build() + ".dlc/", false);
                });
            }

            if (!skipBuilds) {
                dispatch_group_async(buildGroup(), build_queue, ^{
                    makeBoms(manifest, masterDstRoot);
                });
                allBuildsSucceeded = build(diags, manifest, masterDstRoot, true, verbose, skipWrites,
                                           agileChooseSHA256CdHash, true, false);
            }

            manifest.write(resultPath);

            addArtifactPaths(diags, manifest);
            if (!dylibCacheDir.empty()) {
                manifest.write(dylibCacheDir + "/AppleInternal/Developer/DylibCaches/" + manifest.build() + ".dlc/Manifest.plist");
                manifest.writeJSON(dylibCacheDir + "/AppleInternal/Developer/DylibCaches/" + manifest.build() + ".dlc/Manifest.json");
            }

            if (!skipWrites) {
                mkpath_np((masterDstRoot + "/Artifact.dlc").c_str(), 0755);
                auto copy_state = copyfile_state_alloc();
                (void)copyfile(realPath(manifestPath).c_str(), (masterDstRoot + "/Artifact.dlc/BNIManifest.plist").c_str(), copy_state, COPYFILE_ALL);
                copyfile_state_free(copy_state);
                manifest.write(masterDstRoot + "/Artifact.dlc/Manifest.plist");
                manifest.writeJSON(masterDstRoot + "/Artifact.dlc/Manifest.json");
            }

            dispatch_group_wait(buildGroup(), DISPATCH_TIME_FOREVER);
            time_t mytime = time(0);
            fprintf(stderr, "Finished: %s", asctime(localtime(&mytime)));

            if (preflight && !allBuildsSucceeded) {
                exit(-1);
            }

            exit(0);
        });
    }
    dispatch_main();
    return 0;
}

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


#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/syslimits.h>
#include <mach-o/arch.h>
#include <mach-o/loader.h>
#include <mach-o/dyld_priv.h>
#include <bootstrap.h>
#include <mach/mach.h>
#include <dispatch/dispatch.h>

#include <map>
#include <vector>

#include "DyldSharedCache.h"
#include "FileUtils.h"
#include "StringUtils.h"
#include "ClosureBuilder.h"
#include "ClosurePrinter.h"
#include "ClosureFileSystemPhysical.h"

using dyld3::closure::ImageArray;
using dyld3::closure::Image;
using dyld3::closure::ImageNum;
using dyld3::closure::ClosureBuilder;
using dyld3::closure::LaunchClosure;
using dyld3::closure::DlopenClosure;
using dyld3::closure::PathOverrides;
using dyld3::Array;


// mmap() an shared cache file read/only but laid out like it would be at runtime
static const DyldSharedCache* mapCacheFile(const char* path)
{
    struct stat statbuf;
    if ( ::stat(path, &statbuf) ) {
        fprintf(stderr, "Error: stat failed for dyld shared cache at %s\n", path);
        return nullptr;
    }
        
    int cache_fd = ::open(path, O_RDONLY);
    if (cache_fd < 0) {
        fprintf(stderr, "Error: failed to open shared cache file at %s\n", path);
        return nullptr;
    }

    uint8_t  firstPage[4096];
    if ( ::pread(cache_fd, firstPage, 4096, 0) != 4096 ) {
        fprintf(stderr, "Error: failed to read shared cache file at %s\n", path);
        return nullptr;
    }
    const dyld_cache_header*       header   = (dyld_cache_header*)firstPage;
	const dyld_cache_mapping_info* mappings = (dyld_cache_mapping_info*)(firstPage + header->mappingOffset);

    size_t vmSize = (size_t)(mappings[2].address + mappings[2].size - mappings[0].address);
    vm_address_t result;
    kern_return_t r = ::vm_allocate(mach_task_self(), &result, vmSize, VM_FLAGS_ANYWHERE);
    if ( r != KERN_SUCCESS ) {
        fprintf(stderr, "Error: failed to allocate space to load shared cache file at %s\n", path);
        return nullptr;
	}
    for (int i=0; i < 3; ++i) {
        void* mapped_cache = ::mmap((void*)(result + mappings[i].address - mappings[0].address), (size_t)mappings[i].size,
                                    PROT_READ, MAP_FIXED | MAP_PRIVATE, cache_fd, mappings[i].fileOffset);
        if (mapped_cache == MAP_FAILED) {
            fprintf(stderr, "Error: mmap() for shared cache at %s failed, errno=%d\n", path, errno);
            return nullptr;
        }
    }
    ::close(cache_fd);

    return (DyldSharedCache*)result;
}

static void usage()
{
    printf("dyld_closure_util program to create or view dyld3 closures\n");
    printf("  mode:\n");
    printf("    -create_closure <prog-path>            # create a closure for the specified main executable\n");
    printf("    -list_dyld_cache_closures              # list all launch closures in the dyld shared cache with size\n");
    printf("    -list_dyld_cache_dlopen_closures       # list all dlopen closures in the dyld shared cache with size\n");
    printf("    -print_dyld_cache_closure <prog-path>  # find closure for specified program in dyld cache and print as JSON\n");
    printf("    -print_dyld_cache_dylib <dylib-path>   # print specified cached dylib as JSON\n");
    printf("    -print_dyld_cache_dylibs               # print all cached dylibs as JSON\n");
    printf("    -print_dyld_cache_dlopen <path>        # print specified dlopen closure as JSON\n");
    printf("  options:\n");
    printf("    -cache_file <cache-path>               # path to cache file to use (default is current cache)\n");
    printf("    -build_root <path-prefix>              # when building a closure, the path prefix when runtime volume is not current boot volume\n");
    printf("    -env <var=value>                       # when building a closure, DYLD_* env vars to assume\n");
    printf("    -dlopen <path>                         # for use with -create_closure to simulate that program calling dlopen\n");
    printf("    -verbose_fixups                        # for use with -print* options to force printing fixups\n");
    printf("    -no_at_paths                           # when building a closure, simulate security not allowing @path expansion\n");
    printf("    -no_fallback_paths                     # when building a closure, simulate security not allowing default fallback paths\n");
    printf("    -allow_insertion_failures              # when building a closure, simulate security allowing unloadable DYLD_INSERT_LIBRARIES to be ignored\n");
    printf("    -force_invalid_cache_version           # when building a closure, simulate security the cache version mismatching the builder\n");
}

int main(int argc, const char* argv[])
{
    const char*               cacheFilePath = nullptr;
    const char*               inputMainExecutablePath = nullptr;
    const char*               printCacheClosure = nullptr;
    const char*               printCachedDylib = nullptr;
    const char*               printOtherDylib = nullptr;
    const char*               fsRootPath = nullptr;
    const char*               fsOverlayPath = nullptr;
    bool                      listCacheClosures = false;
    bool                      listCacheDlopenClosures = false;
    bool                      printCachedDylibs = false;
    bool                      verboseFixups = false;
    bool                      allowAtPaths = true;
    bool                      allowFallbackPaths = true;
    bool                      allowInsertionFailures = false;
    bool                      forceInvalidFormatVersion = false;
    bool                      printRaw = false;
    std::vector<const char*>  envArgs;
    std::vector<const char*>  dlopens;
    char                      fsRootRealPath[PATH_MAX];
    char                      fsOverlayRealPath[PATH_MAX];

    if ( argc == 1 ) {
        usage();
        return 0;
    }

    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if ( strcmp(arg, "-cache_file") == 0 ) {
            cacheFilePath = argv[++i];
            if ( cacheFilePath == nullptr ) {
                fprintf(stderr, "-cache_file option requires path to cache file\n");
                return 1;
            }
        }
        else if ( strcmp(arg, "-create_closure") == 0 ) {
            inputMainExecutablePath = argv[++i];
            if ( inputMainExecutablePath == nullptr ) {
                fprintf(stderr, "-create_closure option requires a path to an executable\n");
                return 1;
            }
        }
        else if ( strcmp(arg, "-dlopen") == 0 ) {
            const char* path = argv[++i];
            if ( path == nullptr ) {
                fprintf(stderr, "-dlopen option requires a path to a packed closure list\n");
                return 1;
            }
            dlopens.push_back(path);
        }
        else if ( strcmp(arg, "-verbose_fixups") == 0 ) {
           verboseFixups = true;
        }
        else if ( strcmp(arg, "-no_at_paths") == 0 ) {
            allowAtPaths = false;
        }
        else if ( strcmp(arg, "-no_fallback_paths") == 0 ) {
            allowFallbackPaths = false;
        }
        else if ( strcmp(arg, "-allow_insertion_failures") == 0 ) {
            allowInsertionFailures = true;
        }
        else if ( strcmp(arg, "-raw") == 0 ) {
            printRaw = true;
        }
        else if ( strcmp(arg, "-fs_root") == 0 ) {
            fsRootPath = argv[++i];
            if ( fsRootPath == nullptr ) {
                fprintf(stderr, "-fs_root option requires a path\n");
                return 1;
            }
            if ( realpath(fsRootPath, fsRootRealPath) == nullptr ) {
                fprintf(stderr, "-fs_root option requires a real path\n");
                return 1;
            }
            fsRootPath = fsRootRealPath;
        }
        else if ( strcmp(arg, "-fs_overlay") == 0 ) {
            fsOverlayPath = argv[++i];
            if ( fsOverlayPath == nullptr ) {
                fprintf(stderr, "-fs_overlay option requires a path\n");
                return 1;
            }
            if ( realpath(fsOverlayPath, fsOverlayRealPath) == nullptr ) {
                fprintf(stderr, "-fs_root option requires a real path\n");
                return 1;
            }
            fsOverlayPath = fsOverlayRealPath;
        }
        else if ( strcmp(arg, "-force_invalid_cache_version") == 0 ) {
            forceInvalidFormatVersion = true;
        }
        else if ( strcmp(arg, "-list_dyld_cache_closures") == 0 ) {
            listCacheClosures = true;
        }
        else if ( strcmp(arg, "-list_dyld_cache_dlopen_closures") == 0 ) {
            listCacheDlopenClosures = true;
        }
        else if ( strcmp(arg, "-print_dyld_cache_closure") == 0 ) {
            printCacheClosure = argv[++i];
            if ( printCacheClosure == nullptr ) {
                fprintf(stderr, "-print_dyld_cache_closure option requires a path \n");
                return 1;
            }
        }
        else if ( strcmp(arg, "-print_dyld_cache_dylibs") == 0 ) {
            printCachedDylibs = true;
        }
        else if ( strcmp(arg, "-print_dyld_cache_dylib") == 0 ) {
            printCachedDylib = argv[++i];
            if ( printCachedDylib == nullptr ) {
                fprintf(stderr, "-print_dyld_cache_dylib option requires a path \n");
                return 1;
            }
        }
        else if ( strcmp(arg, "-print_dyld_cache_dlopen") == 0 ) {
            printOtherDylib = argv[++i];
            if ( printOtherDylib == nullptr ) {
                fprintf(stderr, "-print_dyld_cache_dlopen option requires a path \n");
                return 1;
            }
        }
        else if ( strcmp(arg, "-env") == 0 ) {
            const char* envArg = argv[++i];
            if ( (envArg == nullptr) || (strchr(envArg, '=') == nullptr) ) {
                fprintf(stderr, "-env option requires KEY=VALUE\n");
                return 1;
            }
            envArgs.push_back(envArg);
        }
        else {
            fprintf(stderr, "unknown option %s\n", arg);
            return 1;
        }
    }

    envArgs.push_back(nullptr);


    const DyldSharedCache* dyldCache = nullptr;
    bool dyldCacheIsLive = true;
    if ( cacheFilePath != nullptr ) {
        dyldCache = mapCacheFile(cacheFilePath);
        dyldCacheIsLive = false;
    }
    else {
#if __MAC_OS_X_VERSION_MIN_REQUIRED && (__MAC_OS_X_VERSION_MIN_REQUIRED < 101300)
        fprintf(stderr, "this tool needs to run on macOS 10.13 or later\n");
        return 1;
#else
        size_t cacheLength;
        dyldCache = (DyldSharedCache*)_dyld_get_shared_cache_range(&cacheLength);
#endif
    }
    dyld3::Platform            platform = dyldCache->platform();
    const dyld3::GradedArchs&  archs    = dyld3::GradedArchs::forName(dyldCache->archName(), true);

    if ( inputMainExecutablePath != nullptr ) {
        PathOverrides pathOverrides;
        pathOverrides.setFallbackPathHandling(allowFallbackPaths ? dyld3::closure::PathOverrides::FallbackPathMode::classic : dyld3::closure::PathOverrides::FallbackPathMode::none);
        pathOverrides.setEnvVars(&envArgs[0], nullptr, nullptr);
        STACK_ALLOC_ARRAY(const ImageArray*,    imagesArrays, 3+dlopens.size());
        STACK_ALLOC_ARRAY(dyld3::LoadedImage,   loadedArray,  1024);
        imagesArrays.push_back(dyldCache->cachedDylibsImageArray());
        imagesArrays.push_back(dyldCache->otherOSImageArray());

        dyld3::closure::FileSystemPhysical fileSystem(fsRootPath, fsOverlayPath);
        ClosureBuilder::AtPath atPathHanding = allowAtPaths ? ClosureBuilder::AtPath::all : ClosureBuilder::AtPath::none;
        ClosureBuilder builder(dyld3::closure::kFirstLaunchClosureImageNum, fileSystem, dyldCache, dyldCacheIsLive, archs, pathOverrides, atPathHanding, true, nullptr, platform, nullptr);
        if (forceInvalidFormatVersion)
            builder.setDyldCacheInvalidFormatVersion();

        const LaunchClosure* mainClosure = builder.makeLaunchClosure(inputMainExecutablePath, allowInsertionFailures);
        if ( builder.diagnostics().hasError() ) {
            fprintf(stderr, "dyld_closure_util: %s\n", builder.diagnostics().errorMessage());
            return 1;
        }
        ImageNum nextNum = builder.nextFreeImageNum();

        if ( !dlopens.empty() )
            printf("[\n");
        imagesArrays.push_back(mainClosure->images());
        dyld3::closure::printClosureAsJSON(mainClosure, imagesArrays, verboseFixups, printRaw, dyldCache);
        ClosureBuilder::buildLoadOrder(loadedArray, imagesArrays, mainClosure);

        for (const char* path : dlopens) {
            printf(",\n");

            // map unloaded mach-o files for use during closure building
            for (dyld3::LoadedImage& li : loadedArray) {
                if ( li.loadedAddress() != nullptr )
                    continue;
                if ( li.image()->inDyldCache() ) {
                    li.setLoadedAddress((dyld3::MachOLoaded*)((uint8_t*)dyldCache + li.image()->cacheOffset()));
                }
                else {
                    Diagnostics diag;
                    char realerPath[MAXPATHLEN];
                    dyld3::closure::LoadedFileInfo loadedFileInfo = dyld3::MachOAnalyzer::load(diag, fileSystem, li.image()->path(), archs, platform, realerPath);
                    li.setLoadedAddress((const dyld3::MachOAnalyzer*)loadedFileInfo.fileContent);
                }
            }

            ClosureBuilder::AtPath atPathHandingDlopen = allowAtPaths ? ClosureBuilder::AtPath::all : ClosureBuilder::AtPath::onlyInRPaths;
            ClosureBuilder dlopenBuilder(nextNum, fileSystem, dyldCache, dyldCacheIsLive, archs, pathOverrides, atPathHandingDlopen, true, nullptr, platform, nullptr);
            if (forceInvalidFormatVersion)
                dlopenBuilder.setDyldCacheInvalidFormatVersion();

            ImageNum topImageNum;
            const DlopenClosure* dlopenClosure = dlopenBuilder.makeDlopenClosure(path, mainClosure, loadedArray, 0, false, false, false, &topImageNum);
            if ( dlopenBuilder.diagnostics().hasError() ) {
                fprintf(stderr, "dyld_closure_util: %s\n", dlopenBuilder.diagnostics().errorMessage());
                return 1;
            }
            if ( dlopenClosure == nullptr ) {
                if ( topImageNum == 0 ) {
                    fprintf(stderr, "dyld_closure_util: failed to dlopen %s\n", path);
                    return 1;
                }
                printf("{\n   \"dyld-cache-image-num\":  \"0x%04X\"\n}\n", topImageNum);
            }
            else {
                nextNum += dlopenClosure->images()->imageCount();
                imagesArrays.push_back(dlopenClosure->images());
                dyld3::closure::printClosureAsJSON(dlopenClosure, imagesArrays, verboseFixups, printRaw);
                ClosureBuilder::buildLoadOrder(loadedArray, imagesArrays, dlopenClosure);
            }
        }
        if ( !dlopens.empty() )
            printf("]\n");
    }
    else if ( listCacheClosures ) {
        dyldCache->forEachLaunchClosure(^(const char* runtimePath, const dyld3::closure::LaunchClosure* closure) {
            printf("%6lu  %s\n", closure->size(), runtimePath);
        });
    }
    else if ( listCacheDlopenClosures ) {
        dyldCache->forEachDlopenImage(^(const char* runtimePath, const dyld3::closure::Image* image) {
            printf("%6lu  %s\n", image->size(), runtimePath);
        });
    }
    else if ( printCacheClosure ) {
        const dyld3::closure::LaunchClosure* closure = dyldCache->findClosure(printCacheClosure);
        if ( closure != nullptr ) {
            STACK_ALLOC_ARRAY(const ImageArray*, imagesArrays, 3);
            imagesArrays.push_back(dyldCache->cachedDylibsImageArray());
            imagesArrays.push_back(dyldCache->otherOSImageArray());
            imagesArrays.push_back(closure->images());
            dyld3::closure::printClosureAsJSON(closure, imagesArrays, verboseFixups, printRaw, dyldCache);
        }
        else {
            fprintf(stderr, "no closure in cache for %s\n", printCacheClosure);
        }
    }
    else if ( printCachedDylibs ) {
        dyld3::closure::printDyldCacheImagesAsJSON(dyldCache, verboseFixups, printRaw);
    }
    else if ( printCachedDylib != nullptr ) {
        const dyld3::closure::ImageArray* dylibs = dyldCache->cachedDylibsImageArray();
        STACK_ALLOC_ARRAY(const ImageArray*, imagesArrays, 2);
        imagesArrays.push_back(dylibs);
        ImageNum num;
        if ( dylibs->hasPath(printCachedDylib, num) ) {
            dyld3::closure::printImageAsJSON(dylibs->imageForNum(num), imagesArrays, verboseFixups, printRaw, dyldCache);
        }
        else {
            fprintf(stderr, "no such image found\n");
        }
    }
    else if ( printOtherDylib != nullptr ) {
        if ( const dyld3::closure::Image* image = dyldCache->findDlopenOtherImage(printOtherDylib) ) {
            STACK_ALLOC_ARRAY(const ImageArray*, imagesArrays, 2);
            imagesArrays.push_back(dyldCache->cachedDylibsImageArray());
            imagesArrays.push_back(dyldCache->otherOSImageArray());
            dyld3::closure::printImageAsJSON(image, imagesArrays, verboseFixups);
        }
        else {
            fprintf(stderr, "no such image found\n");
        }
    }

    return 0;
}

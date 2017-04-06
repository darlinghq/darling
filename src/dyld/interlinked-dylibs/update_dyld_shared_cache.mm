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
#include <rootless.h>

extern "C" {
#include <dscsym.h>
}

#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <fstream>

#include "MachOProxy.h"
#include "manifest.h"
#include "mega-dylib-utils.h"
#include "Logging.h"

#import "MultiCacheBuilder.h"

#if !__has_feature(objc_arc)
#error The use of libdispatch in this files requires it to be compiled with ARC in order to avoid leaks
#endif

const std::string anchorsDirPath = "/private/var/db/dyld/shared_region_roots";

bool parsePathsFile( const std::string& filePath, std::set<std::string>& paths ) {
    verboseLog( "parsing .paths file '%s'", filePath.c_str() );
    std::ifstream myfile( filePath );
        if ( myfile.is_open() ) {
		std::string line;
		while ( std::getline(myfile, line) ) {
			size_t pos = line.find('#');
			if ( pos != std::string::npos )
				line.resize(pos);
			while ( line.size() != 0 && isspace(line.back()) ) {
				line.pop_back();
			}
                        if ( !line.empty() ) paths.insert( line );
                }
                myfile.close();

                return true;
        }
        return false;
}

static bool parseDirectoryOfPathsFiles(const std::string& dirPath, std::set<std::string>& paths)
{
	DIR* dir = ::opendir(dirPath.c_str());
	if ( dir == NULL )
		return false;

	for (dirent* entry = ::readdir(dir); entry != NULL; entry = ::readdir(dir)) {
		if ( entry->d_type == DT_REG || entry->d_type == DT_UNKNOWN ) {
			// only look at regular files ending in .paths
			if ( strcmp(&entry->d_name[entry->d_namlen-6], ".paths") == 0 ) {
				struct stat statBuf;
				std::string filePathStr = dirPath + "/" + entry->d_name;
				const char* filePath = filePathStr.c_str();
				if ( lstat(filePath, &statBuf) == -1 ) {
					warning("can't access file '%s'", filePath);
				}
				else if ( S_ISREG(statBuf.st_mode) ) {
					parsePathsFile(filePath, paths);
				}
				else {
					warning("not a regular file '%s'", filePath);
				}
			}
			else {
				warning("ignoring file with wrong extension '%s'", entry->d_name);
			}
		}
	}
	::closedir(dir);
	return true;
}

static bool buildInitialPaths(const std::string& volumeRootPath, const std::string& overlayPath, std::set<std::string>& paths)
{
	// in -root mode, look for roots in /rootpath/private/var/db/dyld/shared_region_roots
	if ( volumeRootPath != "/" ) {
		if ( parseDirectoryOfPathsFiles(volumeRootPath + "/" + anchorsDirPath, paths) )
			return true;
		// fallback to .paths files on boot volume
		return parseDirectoryOfPathsFiles(anchorsDirPath, paths);
	}

	// in -overlay mode, look for .paths first in each /overlay/private/var/db/dyld/shared_region_roots
	if ( !overlayPath.empty() ) {
		parseDirectoryOfPathsFiles(overlayPath + "/" + anchorsDirPath, paths);
	}

	// look for .paths files in /private/var/db/dyld/shared_region_roots
	return parseDirectoryOfPathsFiles(anchorsDirPath, paths);
}

bool fileExists(const std::string& path, bool& isSymLink)
{
	struct stat statBuf;
	if ( lstat(path.c_str(), &statBuf) == -1 )
		return false;
	isSymLink = S_ISLNK(statBuf.st_mode);
	return S_ISREG(statBuf.st_mode) || isSymLink;
}

bool tryPath(const std::string& prefix, const std::string& path, std::string& foundPath, std::vector<std::string>& aliases)
{
	foundPath = prefix + path;
	bool isSymLink;
	if ( !fileExists(foundPath, isSymLink) )
		return false;
	if ( isSymLink ) {
		// handle case where install name is a symlink to real file (e.g. libstdc++.6.dylib -> libstdc++.6.0.9.dylib)
		char pathInSymLink[MAXPATHLEN];
		long len = ::readlink(foundPath.c_str(), pathInSymLink, sizeof(pathInSymLink));
		if ( len != -1 ) {
			pathInSymLink[len] = '\0';
			if ( pathInSymLink[0] != '/' ) {
				std::string aliasPath = path;
				size_t pos = aliasPath.rfind('/');
				if ( pos != std::string::npos ) {
					std::string newPath = aliasPath.substr(0,pos+1) + pathInSymLink;
					aliases.push_back(newPath);
				}
			}
		}
	}
	char realPath[MAXPATHLEN];
	if ( ::realpath(foundPath.c_str(), realPath) ) {
		if ( foundPath != realPath ) {
			std::string altPath = realPath;
			if ( !prefix.empty() ) {
				if ( altPath.substr(0, prefix.size()) == prefix ) {
					altPath = altPath.substr(prefix.size());
				}
			}
			if ( altPath != path )
				aliases.push_back(path);
			else
				aliases.push_back(altPath);
		}
	}
	return true;
}

bool improvePath(const char* volumeRootPath, const std::vector<const char*>& overlayPaths,
				 const std::string& path, std::string& foundPath, std::vector<std::string>& aliases)
{
	for (const char* overlay : overlayPaths) {
		if ( tryPath(overlay, path, foundPath, aliases) )
			return true;
	}
	if ( volumeRootPath[0] != '\0' ) {
		if ( tryPath(volumeRootPath, path, foundPath, aliases) )
			return true;
	}
	return tryPath("", path, foundPath, aliases);
}

std::string fileExists( const std::string& path ) {
    const uint8_t* p = (uint8_t*)( -1 );
    struct stat stat_buf;
	bool rootless;

    std::tie( p, stat_buf, rootless ) = fileCache.cacheLoad( path );
    if ( p != (uint8_t*)( -1 ) ) {
        return normalize_absolute_file_path( path );
    }

    return "";
}

void populateManifest(Manifest& manifest, std::set<std::string> archs, const std::string& overlayPath,
                      const std::string& rootPath, const std::set<std::string>& paths) {
	for ( const auto& arch : archs ) {
		auto fallback = fallbackArchStringForArchString(arch);
		std::set<std::string> allArchs = archs;
		std::set<std::string> processedPaths;
		std::set<std::string> unprocessedPaths = paths;
		std::set<std::string> pathsToProcess;
		std::set_difference( unprocessedPaths.begin(), unprocessedPaths.end(), processedPaths.begin(), processedPaths.end(),
							std::inserter( pathsToProcess, pathsToProcess.begin() ) );
		while ( !pathsToProcess.empty() ) {
            for (const std::string path : pathsToProcess) {
                processedPaths.insert(path);
                std::string fullPath;
                if ( rootPath != "/" ) {
                    // with -root, only look in the root path volume
                    fullPath = fileExists(rootPath + path);
                }
                else {
                    // with -overlay, look first in overlay dir
                    if ( !overlayPath.empty() )
                        fullPath = fileExists(overlayPath + path);
                    // if not in overlay, look in boot volume
                    if ( fullPath.empty() )
                        fullPath = fileExists(path);
                }
				if ( fullPath.empty() )
					continue;
				auto proxies = MachOProxy::findDylibInfo(fullPath, true, true);
				auto proxy = proxies.find(arch);
				if (proxy == proxies.end())
					proxy = proxies.find(fallback);
				if (proxy == proxies.end())
					continue;
				
				for ( const auto& dependency : proxy->second->dependencies ) {
					unprocessedPaths.insert( dependency );
				}
				
				if ( proxy->second->installName.empty() ) {
					continue;
				}
				
				proxy->second->addAlias( path );
                manifest.architectureFiles[arch].dylibs.insert(std::make_pair(proxy->second->installName,
                                                                              Manifest::File(proxy->second)));
                manifest.configurations["localhost"].architectures[arch].anchors.push_back( proxy->second->installName );
			}
			
			pathsToProcess.clear();
			std::set_difference( unprocessedPaths.begin(), unprocessedPaths.end(), processedPaths.begin(), processedPaths.end(),
								std::inserter( pathsToProcess, pathsToProcess.begin() ) );
		}
	}
}

static bool runningOnHaswell()
{
    // check system is capable of running x86_64h code
    struct host_basic_info  info;
    mach_msg_type_number_t  count    = HOST_BASIC_INFO_COUNT;
    mach_port_t             hostPort = mach_host_self();
    kern_return_t           result   = host_info(hostPort, HOST_BASIC_INFO, (host_info_t)&info, &count);
    mach_port_deallocate(mach_task_self(), hostPort);

    return ( (result == KERN_SUCCESS) && (info.cpu_subtype == CPU_SUBTYPE_X86_64_H) );
}


#define TERMINATE_IF_LAST_ARG( s )           \
    do {                                     \
        if ( i == argc - 1 ) terminate( s ); \
    } while ( 0 )

int main(int argc, const char* argv[])
{
    std::string rootPath;
    std::string overlayPath;
    std::string platform = "osx";
    std::string dylibListFile;
	bool universal = false;
	bool force = false;
	std::string cacheDir;
	std::set<std::string>         archStrs;
	std::vector<Manifest::Anchor> anchors;

	// parse command line options
	for (int i = 1; i < argc; ++i) {
		const char* arg = argv[i];
		if (arg[0] == '-') {
			if (strcmp(arg, "-debug") == 0) {
				setVerbose(true);
			} else if (strcmp(arg, "-verbose") == 0) {
				setVerbose(true);
			} else if (strcmp(arg, "-dont_map_local_symbols") == 0) {
				//We are going to ignore this
			} else if (strcmp(arg, "-iPhone") == 0) {
				platform = "iphoneos";
			} else if (strcmp(arg, "-dylib_list") == 0) {
				TERMINATE_IF_LAST_ARG("-dylib_list missing argument");
				dylibListFile = argv[++i];
			} else if ((strcmp(arg, "-root") == 0) || (strcmp(arg, "--root") == 0)) {
				TERMINATE_IF_LAST_ARG("-root missing path argument\n");
				rootPath = argv[++i];
			} else if (strcmp(arg, "-overlay") == 0) {
				TERMINATE_IF_LAST_ARG("-overlay missing path argument\n");
				overlayPath = argv[++i];
			} else if (strcmp(arg, "-cache_dir") == 0) {
				TERMINATE_IF_LAST_ARG("-cache_dir missing path argument\n");
				cacheDir = argv[++i];
			} else if (strcmp(arg, "-arch") == 0) {
				TERMINATE_IF_LAST_ARG("-arch missing argument\n");
				archStrs.insert(argv[++i]);
			} else if (strcmp(arg, "-force") == 0) {
				force = true;
			} else if (strcmp(arg, "-sort_by_name") == 0) {
				//No-op, we always do this now
			} else if (strcmp(arg, "-universal_boot") == 0) {
				universal = true;
			} else {
				//usage();
				terminate("unknown option: %s\n", arg);
			}
		} else {
			//usage();
			terminate("unknown option: %s\n", arg);
		}
    }

	setReturnNonZeroOnTerminate();
    setWarnAnErrorPrefixes("update_dyld_shared_cache: warning: ", "update_dyld_shared_cache failed: ");

    if ( !rootPath.empty() & !overlayPath.empty() )
        terminate("-root and -overlay cannot be used together\n");

	//FIXME realpath on root and overlays

    if (rootPath.empty()) {
        rootPath = "/";
    }

    if ( cacheDir.empty() ) {
        // write cache file into -root or -overlay directory, if used
        if ( rootPath != "/" )
            cacheDir = rootPath +  MACOSX_DYLD_SHARED_CACHE_DIR;
        else if ( !overlayPath.empty()  )
            cacheDir = overlayPath +  MACOSX_DYLD_SHARED_CACHE_DIR;
        else
            cacheDir = MACOSX_DYLD_SHARED_CACHE_DIR;
    }

    if (universal) {
        if ( platform == "iphoneos" ) {
            terminate("-iPhoneOS and -universal are incompatible\n");
        }
        archStrs.clear();
    }

    if (archStrs.size() == 0) {
		if ( platform == "iphoneos" ) {
			terminate("Must specify -arch(s) when using -iPhone\n");
		}
		else {
            if ( universal ) {
                // <rdar://problem/26182089> -universal_boot should make all possible dyld caches
                archStrs.insert("i386");
                archStrs.insert("x86_64");
                archStrs.insert("x86_64h");
            }
            else {
                // just make caches for this machine
                archStrs.insert("i386");
                archStrs.insert(runningOnHaswell() ? "x86_64h" : "x86_64");
            }
        }
    }

    int err = mkpath_np(cacheDir.c_str(), S_IRWXU | S_IRGRP|S_IXGRP | S_IROTH|S_IXOTH);
    if (err != 0 && err != EEXIST) {
        terminate("mkpath_np fail: %d", err);
    }

	Manifest manifest;

        std::set<std::string> paths;

        if ( !dylibListFile.empty() ) {
            if ( !parsePathsFile( dylibListFile, paths ) ) {
                terminate( "could not build intiial paths\n" );
            }
        } else if ( !buildInitialPaths( rootPath, overlayPath, paths ) ) {
            terminate( "could not build intiial paths\n" );
        }

        manifest.platform = platform;
        populateManifest( manifest, archStrs, overlayPath, rootPath, paths );

        // If the path we are writing to is trusted then our sources need to be trusted
        // <rdar://problem/21166835> Can't update the update_dyld_shared_cache on a non-boot volume
        bool requireDylibsBeRootlessProtected = isProtectedBySIP(cacheDir);
        manifest.calculateClosure( requireDylibsBeRootlessProtected );
        manifest.pruneClosure();

    for (const std::string& archStr : archStrs) {
        std::string cachePath = cacheDir + "/dyld_shared_cache_" + archStr;
        if ( manifest.sameContentsAsCacheAtPath("localhost", archStr, cachePath) && !force ) {
            manifest.configurations["localhost"].architectures.erase(archStr);
            verboseLog("%s is already up to date", cachePath.c_str());
        }
    }

    // If caches already up to date, do nothing
    if ( manifest.configurations["localhost"].architectures.empty() )
        dumpLogAndExit(false);

    // build caches
    std::shared_ptr<MultiCacheBuilder> builder = std::make_shared<MultiCacheBuilder>(manifest, false, false, false, false, requireDylibsBeRootlessProtected);
    builder->buildCaches(cacheDir);

    // Save off spintrace data
    std::string nuggetRoot = (overlayPath.empty() ? rootPath : overlayPath);
    (void)dscsym_save_nuggets_for_current_caches(nuggetRoot.c_str());

	// Now that all the build commands have been issued lets put a barrier in after then which can tear down the app after
	// everything is written.
	builder->logStats();
	dumpLogAndExit(false);

	dispatch_main();
}


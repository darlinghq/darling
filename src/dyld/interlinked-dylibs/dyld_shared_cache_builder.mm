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
#include <fts.h>

#include <vector>
#include <array>
#include <set>
#include <map>
#include <algorithm>

#include <spawn.h>

#include <Bom/Bom.h>

#include "Manifest.h"
#include "MultiCacheBuilder.h"

#include "mega-dylib-utils.h"
#include "Logging.h"

#if !__has_feature(objc_arc)
#error The use of libdispatch in this files requires it to be compiled with ARC in order to avoid leaks
#endif

extern char** environ;

static dispatch_queue_t build_queue;

inline bool has_suffix(std::string const & value, std::string const & ending)
{
	if (ending.size() > value.size()) return false;
	return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

static const char *tempRootDirTemplate = "/tmp/dyld_shared_cache_builder.XXXXXX";
static char *tempRootDir = nullptr;

void processRoot( const std::string &root, std::set<std::string> &roots ) {
    struct stat sb;
    int res = 0;
        pid_t pid;
	int status;
	const char* args[8];

	res = stat(root.c_str(), &sb);

	if (res == 0 && S_ISDIR(sb.st_mode)) {
            roots.insert( root );
            return;
        } else if ( has_suffix( root, ".cpio" ) || has_suffix( root, ".cpio.gz" ) || has_suffix( root, ".cpgz" ) ||
                    has_suffix( root, ".cpio.bz2" ) || has_suffix( root, ".cpbz2" ) || has_suffix( root, ".pax" ) ||
                    has_suffix( root, ".pax.gz" ) || has_suffix( root, ".pgz" ) || has_suffix( root, ".pax.bz2" ) ||
                    has_suffix( root, ".pbz2" ) ) {
                args[0] = (char *)"/usr/bin/ditto";
		args[1] = (char *)"-x";
		args[2] = (char *)root.c_str();
		args[3] = tempRootDir;
		args[4] = nullptr;
	} else if (has_suffix(root, ".tar")) {
		args[0] = (char *)"/usr/bin/tar";
		args[1] = (char *)"xf";
		args[2] = (char *)root.c_str();
		args[3] = (char *)"-C";
		args[4] = tempRootDir;
		args[5] = nullptr;
	} else if (has_suffix(root, ".tar.gz") || has_suffix(root, ".tgz")) {
		args[0] = (char *)"/usr/bin/tar";
		args[1] = (char *)"xzf";
		args[2] = (char *)root.c_str();
		args[3] = (char *)"-C";
		args[4] = tempRootDir;
		args[5] = nullptr;
	} else if (has_suffix(root, ".tar.bz2")
			   || has_suffix(root, ".tbz2")
			   || has_suffix(root, ".tbz")) {
		args[0] = (char *)"/usr/bin/tar";
		args[1] = (char *)"xjf";
		args[2] = (char *)root.c_str();
		args[3] = (char *)"-C";
		args[4] = tempRootDir;
		args[5] = nullptr;
	} else if (has_suffix(root, ".xar")) {
		args[0] = (char *)"/usr/bin/xar";
		args[1] = (char *)"-xf";
		args[2] = (char *)root.c_str();
		args[3] = (char *)"-C";
		args[4] = tempRootDir;
		args[5] = nullptr;
	} else if (has_suffix(root, ".zip")) {
		args[0] = (char *)"/usr/bin/ditto";
		args[1] = (char *)"-xk";
		args[2] = (char *)root.c_str();
		args[3] = tempRootDir;
		args[4] = nullptr;
	} else {
		terminate("unknown archive type: %s", root.c_str());
	}

	res = posix_spawn(&pid, args[0], nullptr, nullptr, (char**)args, environ);
	if (res != 0) terminate("Failed to spawn %s: %s (%d)", args[0], strerror(res), res);

	do {
		res = waitpid(pid, &status, 0);
	} while (res == -1 && errno == EINTR);
	if (res != -1) {
		if (WIFEXITED(status)) {
			res = WEXITSTATUS(status);
		} else {
			res = -1;
		}
	}

	if (res != 0) terminate("Could not expand archive %s: %s (%d)", root.c_str(), strerror(res), res);

	for (auto &existingRoot : roots) {
		if (existingRoot == tempRootDir)
			return;
	}

        roots.insert( tempRootDir );
}

bool writeRootList( const std::string &dstRoot, const std::set<std::string> &roots ) {
    if ( roots.size() == 0 ) return false;

    std::string rootFile = dstRoot + "/roots.txt";
        FILE* froots = ::fopen(rootFile.c_str(), "w");
	if ( froots == NULL )
		return false;

	for (auto &root : roots) {
			fprintf(froots, "%s\n", root.c_str());
	}

	::fclose(froots);
	return true;
}

std::set<std::string> cachePaths;

BOMCopierCopyOperation filteredCopy(BOMCopier copier, const char *path, BOMFSObjType type, off_t size) {
	std::string absolutePath = &path[1];
	if (cachePaths.count(absolutePath)) {
		return BOMCopierSkipFile;
	}
	return BOMCopierContinue;
}

int main (int argc, const char * argv[]) {
	@autoreleasepool {
		std::set<std::string>    roots;
		std::vector<std::string> inputRoots;
		std::string              dylibCacheDir;
		std::string              release;
		bool                     emitDevCaches = true;
		bool                     emitElidedDylibs = true;
		bool                     listConfigs = false;
		bool                     copyRoots = false;
		std::string              dstRoot;
		std::string              configuration;
		std::string              resultPath;

		time_t mytime = time(0);
		log("Started: %s", asctime(localtime(&mytime)));

		tempRootDir = strdup(tempRootDirTemplate);
		mkdtemp(tempRootDir);

		for (int i = 1; i < argc; ++i) {
			const char* arg = argv[i];
			if (arg[0] == '-') {
				if (strcmp(arg, "-debug") == 0) {
					setVerbose(true);
				} else if (strcmp(arg, "-list_configs") == 0) {
					listConfigs = true;
				} else if (strcmp(arg, "-root") == 0) {
					std::string root = argv[++i];
					inputRoots.push_back(root);
					processRoot(root, roots);
				} else if (strcmp(arg, "-copy_roots") == 0) {
					copyRoots = true;
				} else if (strcmp(arg, "-dylib_cache") == 0) {
					dylibCacheDir = argv[++i];
				} else if (strcmp(arg, "-no_development_cache") == 0) {
					emitDevCaches = false;
				} else if (strcmp(arg, "-no_overflow_dylibs") == 0) {
					emitElidedDylibs = false;
				} else if (strcmp(arg, "-development_cache") == 0) {
					emitDevCaches = true;
				} else if (strcmp(arg, "-overflow_dylibs") == 0) {
					emitElidedDylibs = true;
				} else if (strcmp(arg, "-dst_root") == 0) {
					dstRoot = argv[++i];
				} else if (strcmp(arg, "-release") == 0) {
					release = argv[++i];
				} else if (strcmp(arg, "-results") == 0) {
					resultPath = argv[++i];
				} else {
					//usage();
					terminate("unknown option: %s\n", arg);
				}
			} else {
				if (!configuration.empty()) {
					terminate("You may only specify one configruation");
				}
				configuration = argv[i];
			}
		}

		struct rlimit rl = {OPEN_MAX, OPEN_MAX};
		(void)setrlimit(RLIMIT_NOFILE, &rl);

		if (dylibCacheDir.empty() && release.empty()) {
			terminate("you must specify either -dylib_cache or -release");
		} else if (!dylibCacheDir.empty() && !release.empty()) {
			terminate("you may not use -dylib_cache and -release at the same time");
		}

		if ((configuration.empty() || dstRoot.empty()) && !listConfigs) {
			terminate("Must specify a configuration and a -dst_root OR -list_configs\n");
		}


		if (dylibCacheDir.empty()) {
			dylibCacheDir = std::string("/AppleInternal/Developer/DylibCaches/") + release + ".dlc";
		}

		//Move into the dir so we can use relative path manifests
		chdir(dylibCacheDir.c_str());

		auto manifest = Manifest(dylibCacheDir + "/Manifest.plist", roots);

		if (manifest.build.empty()) {
			terminate("No manifest found at '%s/Manifest.plist'\n", dylibCacheDir.c_str());
		}
		log("Building Caches for %s", manifest.build.c_str());

		if (listConfigs) {
			for (auto& config : manifest.configurations) {
				printf("%s\n", config.first.c_str());
			}
			exit(0);
		}

		std::map<std::string, Manifest::Configuration> filteredConfigs;

		for (auto& config : manifest.configurations) {
			if (config.first == configuration) {
				filteredConfigs[config.first] = config.second;

				for (auto& arch : filteredConfigs[config.first].architectures) {
					arch.second.results = Manifest::Results();
				}
			}
		}

		if ( filteredConfigs.empty() ) {
			terminate( "No config %s. Please run with -list_configs to see configurations available for this %s.\n",
				   configuration.c_str(), manifest.build.c_str() );
		}

		manifest.configurations = filteredConfigs;
		manifest.calculateClosure(false);

		// FIXME: Plumb through no_development

		std::shared_ptr<MultiCacheBuilder> builder = std::make_shared<MultiCacheBuilder>( manifest, false, false, true );
		builder->buildCaches(dstRoot);
		writeRootList(dstRoot, roots);

		if (copyRoots) {
			for (auto& config : manifest.configurations) {
				for (auto& arch : config.second.architectures) {
					for (auto& dylib : arch.second.results.dylibs) {
						if (dylib.second.included) {
							MachOProxy *proxy = manifest.dylibProxy( dylib.first, arch.first );
							cachePaths.insert( proxy->installName );
							for ( auto &alias : proxy->installNameAliases ) {
										cachePaths.insert(alias);
							}
						}
					}
				}
			}

			BOMCopier copier = BOMCopierNewWithSys(BomSys_default());
			BOMCopierSetCopyFileStartedHandler(copier, filteredCopy);
			for (auto& root : roots) {
				BOMCopierCopy(copier, root.c_str(), dstRoot.c_str());
			}
			BOMCopierFree(copier);
		}

        // Create an empty FIPS data in the root
        (void)mkpath_np((dstRoot + "/private/var/db/FIPS/").c_str(), 0755);
        int fd = open((dstRoot + "/private/var/db/FIPS/fips_data").c_str(), O_CREAT | O_TRUNC, 0644);
        close(fd);

		// Now that all the build commands have been issued lets put a barrier in after then which can tear down the app after
		// everything is written.

		builder->logStats();
		if (!resultPath.empty()) {
			manifest.write(resultPath);
		}

		pid_t       pid;
		int         res = 0;
		int         status;
		const char* args[8];

		args[0] = (char*)"/bin/rm";
		args[1] = (char*)"-rf";
		args[2] = (char*)tempRootDir;
        args[3] = nullptr;

        res = posix_spawn(&pid, args[0], nullptr, nullptr, (char**)args, environ);
		if (res != 0)
			terminate("Failed to spawn %s: %s (%d)", args[0], strerror(res), res);

		do {
			res = waitpid(pid, &status, 0);
		} while (res == -1 && errno == EINTR);
		if (res != -1) {
			if (WIFEXITED(status)) {
				res = WEXITSTATUS(status);
			} else {
				res = -1;
			}
		}

		dumpLogAndExit();
	}

        dispatch_main();

        return 0;
}

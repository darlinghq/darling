//
//  SharedCacheBuilder.m
//  dyld
//
//  Created by Louis Gerbarg on 6/15/15.
//
//

#include <CommonCrypto/CommonCrypto.h>
#include <Bom/Bom.h>

#include <sys/types.h>
#include <sys/sysctl.h>
#include <pthread.h>
#include <mach/mach.h>
#include <unistd.h>

#include <cstring>

#include <array>
#include <sstream>
#include <iomanip>      // std::setfill, std::setw
#include "mega-dylib-utils.h"
#include "Logging.h"

#include "MultiCacheBuilder.h"


namespace {
#if BOM_SUPPORT
void insertDirInBom( const std::string& path, const std::string& name, BOMBom bom ) {
    std::string fullPath = path + "/" + name;
    BOMFSObject fso = BOMFSObjectNew( BOMDirectoryType );
    BOMFSObjectSetFlags( fso, B_PATHONLY );
    BOMFSObjectSetPathName( fso, fullPath.c_str(), true );
    BOMFSObjectSetShortName( fso, name.c_str(), true );
    (void)BOMBomInsertFSObject( bom, fso, false );
    BOMFSObjectFree( fso );
}

void insertFileInBom( const std::string& path, const std::string& name, BOMBom bom ) {
    std::string fullPath = path + "/" + name;
    BOMFSObject fso = BOMFSObjectNew( BOMFileType );
    BOMFSObjectSetFlags( fso, B_PATHONLY );
    BOMFSObjectSetPathName( fso, fullPath.c_str(), true );
    BOMFSObjectSetShortName( fso, name.c_str(), true );
    (void)BOMBomInsertFSObject( bom, fso, false );
    BOMFSObjectFree( fso );
}

void insertCacheDirInBom( BOMBom bom ) {
    BOMFSObject fso = BOMFSObjectNew( BOMDirectoryType );
    BOMFSObjectSetFlags( fso, B_PATHONLY );
    BOMFSObjectSetPathName( fso, ".", true );
    BOMFSObjectSetShortName( fso, ".", true );
    (void)BOMBomInsertFSObject( bom, fso, false );
    BOMFSObjectFree( fso );
    insertDirInBom( ".", "System", bom );
    insertDirInBom( "./System", "Library", bom );
    insertDirInBom( "./System/Library", "Caches", bom );
    insertDirInBom( "./System/Library/Caches", "com.apple.dyld", bom );
}
#endif /* BOM_SUPPORT */
}

MultiCacheBuilder::MultiCacheBuilder(Manifest& manifest, bool BNI, bool SW, bool buildRoot, bool skipBuilds, bool enforceRootles)
	: _manifest(manifest), _bniMode(BNI), _skipWrites(SW), _buildRoot(buildRoot), _skipBuilds(skipBuilds), _enforceRootless(enforceRootles),
	_writeQueue(dispatch_queue_create("com.apple.dyld.cache.writeout",
									  dispatch_queue_attr_make_with_qos_class(DISPATCH_QUEUE_SERIAL,
																		  QOS_CLASS_USER_INITIATED, 0))),
	_writeGroup(dispatch_group_create()),
	_buildQueue(dispatch_queue_create("com.apple.dyld.cache.multi-build", DISPATCH_QUEUE_CONCURRENT)) {
		uint64_t thread_count;
		uint64_t ram_size;

		size_t len = sizeof(thread_count);
		sysctlbyname ("hw.logicalcpu",&thread_count,&len,NULL,0);
		len = sizeof(ram_size);
		sysctlbyname ("hw.memsize",&ram_size,&len,NULL,0);

		uint64_t buildCount = MIN((ram_size/(1024*1024*1024)/2), thread_count);
		uint64_t writerCount = MAX((ram_size/((uint64_t)2*1024*1024*1024)) - buildCount, 1);

		_buildQueue = dispatch_queue_create("com.apple.dyld.cache.build", DISPATCH_QUEUE_CONCURRENT);
		_concurrencyLimitingSemaphore = dispatch_semaphore_create(buildCount);
		_writeLimitingSemaphore = dispatch_semaphore_create(writerCount);

		if ( _bniMode ) {
			log("Running: %llu threads", buildCount);
			log("Queuing: %llu writers", writerCount);
		}
}

void MultiCacheBuilder::write_cache(std::string cachePath, const std::set<std::string>& configurations, const std::string& architecture, std::shared_ptr<SharedCache> cache, bool developmentCache)
{
    //FIXME
    dispatch_semaphore_wait(_writeLimitingSemaphore, DISPATCH_TIME_FOREVER);
    dispatch_group_enter(_writeGroup);
    cacheBuilderDispatchAsync(_writeQueue, [=] {
        if (!_skipWrites) {
            verboseLog("Queuing write out: %s", cachePath.c_str());

            //Turn off file caching since we won't read it back
            //(void)fcntl(fd, F_NOCACHE, 1);
            // We should do this after the cache write, but that would involve copying the path string
            std::string tempPath = cachePath;
            cache->writeCacheMapFile(cachePath + ".map");
            char tempEXT[] = ".XXXXXX";
            mktemp(tempEXT);
            tempPath += tempEXT;

            int fd = ::open(tempPath.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0644);
            if (fd == -1) {
                dispatch_group_leave(_writeGroup);
                dispatch_semaphore_signal(_writeLimitingSemaphore);
                terminate("can't create temp file for %s, errnor=%d (%s)", cachePath.c_str(), errno, strerror(errno));
            }

            if (isProtectedBySIP(tempPath, fd) != _enforceRootless) {
                ::close(fd);
                ::unlink(tempPath.c_str());
                dispatch_group_leave(_writeGroup);
                dispatch_semaphore_signal(_writeLimitingSemaphore);
                terminate("SIP protection of output cache file changed (%s)", cachePath.c_str());
            }

            ssize_t writtenSize = pwrite(fd, cache->buffer().get(), cache->fileSize(), 0);
            if (writtenSize != cache->fileSize()) {
                ::close(fd);
                ::unlink(tempPath.c_str());
                dispatch_group_leave(_writeGroup);
                dispatch_semaphore_signal(_writeLimitingSemaphore);
                terminate("write() failure creating cache file, requested %lld, wrote %ld, errno=%d (%s)", cache->fileSize(), writtenSize, errno, strerror(errno));
            }

            ::close(fd);

            if (rename(tempPath.c_str(), cachePath.c_str()) != 0) {
                dispatch_group_leave(_writeGroup);
                dispatch_semaphore_signal(_writeLimitingSemaphore);
                terminate("move() failure creating cache file, errno=%d (%s)", errno, strerror(errno));
            }
            if (_bniMode)
                log("Wrote out: %s", cachePath.c_str());
        } else {
            log("Skipped: %s", cachePath.c_str());
        }
        _filesWritten++;
        _bytesWritten += cache->fileSize();
        dispatch_group_leave(_writeGroup);
        dispatch_semaphore_signal(_writeLimitingSemaphore);
    });
}

//FIXME (make development a type)
void MultiCacheBuilder::buildCache(const std::string cachePath, const std::set<std::string> configurations, const std::string architecture, bool development)
{
    auto& configResults = _manifest.configurations[*configurations.begin()].architectures[architecture].results.dylibs;

    if ( _skipBuilds ) {
        log( "Build Skipped" );

        for ( auto& config : configurations ) {
            for ( auto& dylib : configResults ) {
                _manifest.configurations[config].architectures[architecture].results.dylibs[dylib.first].exclude(
                    "All dylibs excluded" );
            }
        }
        return;
	}

	Manifest::Architecture arch;
	std::vector<std::unique_ptr<MachOProxy>> dylibs;
	std::vector<std::string> emptyList;
	std::shared_ptr<SharedCache> cache = std::make_shared<SharedCache>(_manifest, *configurations.begin(), architecture);

	for (auto& config : configurations) {
		auto& results = _manifest.configurations[config].architectures[architecture].results.dylibs;

		for (auto& dylib : configResults) {
			if (dylib.second.included == false
				&& results.count(dylib.first)
				&& results[dylib.first].included == true) {
				results[dylib.first].exclude(dylib.second.exclusionInfo);
			}
		}
	}

	if (development) {
		cache->buildForDevelopment(cachePath);
	} else {
		cache->buildForProduction(cachePath);
	}

	std::vector<uint64_t> regionStartAddresses;
	std::vector<uint64_t> regionSizes;
	std::vector<uint64_t> regionFileOffsets;

	cache->forEachRegion([&] (void* content, uint64_t vmAddr, uint64_t size, uint32_t permissions) {
		regionStartAddresses.push_back(vmAddr);
		regionSizes.push_back(size);
		regionFileOffsets.push_back((uint8_t*)content - (uint8_t*)cache->buffer().get());
		const char* prot = "RW";
		if ( permissions == (VM_PROT_EXECUTE|VM_PROT_READ) )
			prot = "EX";
		else if ( permissions == VM_PROT_READ )
			prot = "RO";
		for (auto& config : configurations) {
			if (development) {
				_manifest.configurations[config].architectures[architecture].results.developmentCache.regions.push_back({prot, vmAddr,vmAddr+size });
			} else {
				_manifest.configurations[config].architectures[architecture].results.productionCache.regions.push_back({prot, vmAddr,vmAddr+size });
			}
		}
	});

	cache->forEachImage([&](const void* machHeader, const char* installName, time_t mtime,
								ino_t inode, const std::vector<MachOProxy::Segment>& segments) {
		for (auto& seg : segments) {
			uint64_t vmAddr = 0;
			for (int i=0; i < regionSizes.size(); ++i) {
				if ( (seg.fileOffset >= regionFileOffsets[i]) && (seg.fileOffset < (regionFileOffsets[i]+regionSizes[i])) ) {
					vmAddr = regionStartAddresses[i] + seg.fileOffset - regionFileOffsets[i];
				}
			}
			for (auto& config : configurations) {
				_manifest.configurations[config].architectures[architecture].results.dylibs[installName].segments.push_back({seg.name, vmAddr, vmAddr+seg.size});
				if (_manifest.configurations[config].architectures[architecture].results.dylibs[installName].segments.size() == 0) {
					warning("Attempting to write info for excluded dylib");
					_manifest.configurations[config].architectures[architecture].results.dylibs[installName].exclude("Internal Error");
				}
			}
		}
	});
	if (development) {
		verboseLog("developement cache size = %llu", cache->fileSize());
	} else {
		verboseLog("production cache size = %llu", cache->fileSize());
	}
	if ( cache->vmSize()+align(cache->vmSize()/200, sharedRegionRegionAlignment(archForString(architecture))) > sharedRegionRegionSize(archForString(architecture))) {
            warning("shared cache will not fit in shared regions address space.  Overflow amount: %llu",
                cache->vmSize() + align(cache->vmSize() / 200, sharedRegionRegionAlignment(archForString(architecture))) - sharedRegionRegionSize(archForString(architecture)));
            return;
	}
        write_cache(cachePath, configurations, architecture, cache, development);
        for (auto& config : configurations) {
		if (development) {
			_manifest.configurations[config].architectures[architecture].results.developmentCache.cdHash = cache->cdHashString();
		}
		else {
			_manifest.configurations[config].architectures[architecture].results.productionCache.cdHash = cache->cdHashString();
		}
	}
}

void MultiCacheBuilder::runOnManifestConcurrently(std::function<void(const std::string configuration, const std::string architecture)> lambda)
{
    dispatch_group_t runGroup = dispatch_group_create();
    for (auto& config : _manifest.configurations) {
        for (auto& architecture : config.second.architectures) {
            dispatch_semaphore_wait(_concurrencyLimitingSemaphore, DISPATCH_TIME_FOREVER);
            cacheBuilderDispatchGroupAsync(runGroup, _buildQueue, [&] {
                WarningTargets targets;
                targets.first = &_manifest;
                targets.second.insert(std::make_pair(config.first, architecture.first));
                auto ctx = std::make_shared<LoggingContext>(config.first + "/" + architecture.first, targets);
                setLoggingContext(ctx);
                lambda(config.first, architecture.first);
                dispatch_semaphore_signal(_concurrencyLimitingSemaphore);
            });
        }
    }

    dispatch_group_wait(runGroup, DISPATCH_TIME_FOREVER);
}

void MultiCacheBuilder::buildCaches(std::string masterDstRoot) {
	if (_bniMode) {
		std::vector<std::set<std::string>> dedupedCacheSets;
		for (auto& config : _manifest.configurations) {
			bool dupeFound = false;

			for (auto& cacheSet : dedupedCacheSets) {
				if (config.second.equivalent(_manifest.configurations[*cacheSet.begin()])) {
					cacheSet.insert(config.first);
					dupeFound = true;
					break;
				}
			}

			if (!dupeFound) {
				std::set<std::string> temp;
				temp.insert(config.first);
				dedupedCacheSets.push_back(temp);
			}
		}

		for (auto& cacheSet : dedupedCacheSets) {
			//FIXME we may want to consider moving to hashes of UUID sets
			std::string setName;

			for (auto &archName : cacheSet) {
				if (!setName.empty()) {
					setName += "|";
				}
				setName += archName;
			}

			std::stringstream fileNameStream;
			std::array<uint8_t, CC_SHA1_DIGEST_LENGTH> digest = {0};
			CC_SHA1(setName.c_str(), (unsigned int)setName.length(), &digest[0]);

			fileNameStream << std::hex << std::uppercase << std::setfill( '0' );
			for( int c : digest ) {
				fileNameStream << std::setw( 2 ) << c;
			}

			std::string fileName(fileNameStream.str());

			for (auto& config : cacheSet) {
				if (!_skipWrites) {
					int err = symlink(("DedupedConfigs/" + fileName).c_str(), (masterDstRoot + "/" + config).c_str());
					if (err) {
						warning("Could not create symlink '%s' -> 'DedupedConfigs/%s' (%d)", config.c_str(), fileName.c_str(), err);
					}
				}
			}

			for (auto& arch : _manifest.configurations[*cacheSet.begin()].architectures) {
				dispatch_semaphore_wait(_concurrencyLimitingSemaphore, DISPATCH_TIME_FOREVER);
                cacheBuilderDispatchGroupAsync(_writeGroup, _buildQueue, [=] {
                    WarningTargets targets;
                    targets.first = &_manifest;
                    for (auto& config : cacheSet) {
                        targets.second.insert(std::make_pair(config, arch.first));
                    }
                    auto ctx = std::make_shared<LoggingContext>(setName + "/" + arch.first, targets);
                    setLoggingContext(ctx);

                    std::string configPath = masterDstRoot + "/DedupedConfigs/" + fileName + "/System/Library/Caches/com.apple.dyld/";

                    if (!_skipWrites) {
                        int err = mkpath_np(configPath.c_str(), 0755);

                        if (err != 0 && err != EEXIST) {
                            dispatch_semaphore_signal(_concurrencyLimitingSemaphore);
                            terminate("mkpath_np fail: %d", err);
                        }
                    }

                    buildCache(configPath + "dyld_shared_cache_" + arch.first + ".development", cacheSet, arch.first, true);
                    buildCache(configPath + "dyld_shared_cache_" + arch.first, cacheSet, arch.first, false);
                    dispatch_semaphore_signal(_concurrencyLimitingSemaphore);
                });
            }
		}

		dispatch_group_wait(_writeGroup, DISPATCH_TIME_FOREVER);

#if BOM_SUPPORT
                if ( !_skipWrites ) {
                    for ( auto& configuration : _manifest.configurations ) {
                                std::vector<std::string> prodBomPaths;
				std::vector<std::string> devBomPaths;

				for (auto& arch : configuration.second.architectures) {
					std::string cachePath = "dyld_shared_cache_" + arch.first;
					prodBomPaths.push_back(cachePath);
					cachePath += ".development";
					devBomPaths.push_back(cachePath);
					dispatch_group_enter(_writeGroup);
                    cacheBuilderDispatchAsync(_writeQueue, [=] {
                        char buffer[MAXPATHLEN];
                        sprintf(buffer, "%s/Boms/%s.prod.bom", masterDstRoot.c_str(), configuration.first.c_str());
                        BOMBom bom = BOMBomNew(buffer);
                        insertCacheDirInBom(bom);
                        for (auto& path : prodBomPaths) {
                            insertFileInBom("./System/Library/Caches/com.apple.dyld", path, bom);
                        }
                        BOMBomFree(bom);

                        sprintf(buffer, "%s/Boms/%s.dev.bom", masterDstRoot.c_str(), configuration.first.c_str());
                        bom = BOMBomNew(buffer);
                        insertCacheDirInBom(bom);
                        for (auto& path : devBomPaths) {
                            insertFileInBom("./System/Library/Caches/com.apple.dyld", path, bom);
                        }
                        BOMBomFree(bom);

                        sprintf(buffer, "%s/Boms/%s.full.bom", masterDstRoot.c_str(), configuration.first.c_str());
                        bom = BOMBomNew(buffer);
                        insertCacheDirInBom(bom);
                        for (auto& path : prodBomPaths) {
                            insertFileInBom("./System/Library/Caches/com.apple.dyld", path, bom);
                        }
                        for (auto& path : devBomPaths) {
                            insertFileInBom("./System/Library/Caches/com.apple.dyld", path, bom);
                        }
                        BOMBomFree(bom);
                        dispatch_group_leave(_writeGroup);
                    });
                }
			}
		}
#endif /* BOM_SUPPORT */
        } else {
            runOnManifestConcurrently(
                [&](const std::string configuration, const std::string architecture) {
                    cacheBuilderDispatchGroupAsync(_writeGroup, _buildQueue, [=] {
                        std::set<std::string> configurations;
                        configurations.insert( configuration );
                        // FIXME hacky, we make implicit assumptions about dev vs non-dev and layout depending on the flags
                        if ( _buildRoot ) {
                            int err = mkpath_np( ( masterDstRoot + "/System/Library/Caches/com.apple.dyld/" ).c_str(), 0755 );

                            if ( err != 0 && err != EEXIST ) {
                                terminate( "mkpath_np fail: %d", err );
                            }
                            buildCache(masterDstRoot + "/System/Library/Caches/com.apple.dyld/dyld_shared_cache_" + architecture,
                                configurations, architecture, false);
                            buildCache(masterDstRoot + "/System/Library/Caches/com.apple.dyld/dyld_shared_cache_" + architecture + ".development",
                                configurations, architecture, true);
                        } else {
                            buildCache(masterDstRoot + "/dyld_shared_cache_" + architecture, configurations, architecture, true);
                        }
                    });
                });
            dispatch_group_wait(_writeGroup, DISPATCH_TIME_FOREVER);
	}

	int err = sync_volume_np(masterDstRoot.c_str(), SYNC_VOLUME_FULLSYNC | SYNC_VOLUME_WAIT);
	if (err) {
		warning("Volume sync failed errnor=%d (%s)", err, strerror(err));
	}
}

void MultiCacheBuilder::logStats(void) {
	if ( _bniMode )
		log("Processed %llu caches (%.2fGB)", _filesWritten, ((float)_bytesWritten)/(1024*1024*1024));
}


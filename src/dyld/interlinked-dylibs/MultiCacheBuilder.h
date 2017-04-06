//
//  MultiCacheBuilder.h
//  dyld
//
//  Created by Louis Gerbarg on 6/16/15.
//
//

#ifndef MultiCacheBuilder_h
#define MultiCacheBuilder_h

#include <dispatch/dispatch.h>

#include "Manifest.h"

#include "mega-dylib-utils.h"
#include <stdlib.h>

typedef struct _BOMBom* BOMBom;

struct MultiCacheBuilder {
	dispatch_semaphore_t _concurrencyLimitingSemaphore;
	dispatch_semaphore_t _writeLimitingSemaphore;
	dispatch_queue_t _writeQueue;
	dispatch_group_t _writeGroup;
	dispatch_queue_t _buildQueue;
	Manifest& _manifest;

	uint64_t _filesWritten = 0;
	uint64_t _bytesWritten = 0;
	const bool _bniMode;
	const bool _skipWrites;
	const bool _skipBuilds;
	const bool _buildRoot;
	const bool _enforceRootless;

	MultiCacheBuilder(Manifest& manifest, bool BNI = false, bool SW = false, bool buildRoot = false, bool skipBuilds = false, bool enforceRootless = false);

	void buildCaches(std::string masterDstRoot);

	void logStats();
private:
    void runOnManifestConcurrently(std::function<void(const std::string configuration, const std::string architecture)> lambda);
    void buildCache(const std::string cachePath, const std::set<std::string> configurations, const std::string architecture, bool development);
    void write_cache(std::string cachePath, const std::set<std::string>& configurations, const std::string& architecture, std::shared_ptr<SharedCache> cache, bool developmentCache);
};

#endif /* MultiCacheBuilder_h */

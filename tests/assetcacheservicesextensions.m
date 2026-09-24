// Guest test: AssetCacheServicesExtensions exports the metrics database location Apple documents,
// its classes instantiate, and the manager call sequence Activity Monitor was observed to send
// (initWithDelegate:delegateQueue: on the main queue, then isActivated) reports no active cache.
// Exits non-zero on failure.
#import <AssetCacheServicesExtensions/AssetCacheServicesExtensions.h>
#include <objc/runtime.h>
#include <stdio.h>

int main(void)
{
	int failures = 0;
	@autoreleasepool {
		NSString *path = [kACMetricsDatabaseDirectory stringByAppendingPathComponent:kACMetricsDatabaseName];
		if (![path isEqualToString:@"/Library/Application Support/Apple/AssetCache/Metrics/Metrics.db"]) {
			fprintf(stderr, "FAIL: metrics database path is %s\n", path.UTF8String);
			failures++;
		}
		if ([[NSFileManager defaultManager] fileExistsAtPath:path])
			printf("note: a metrics database exists at %s\n", path.UTF8String);
		for (Class c in @[[AssetCacheMetricsReader class], [AssetCacheServicesManager class]]) {
			id instance = [[c alloc] init];
			if (!instance) {
				fprintf(stderr, "FAIL: %s did not instantiate\n", class_getName(c));
				failures++;
			}
			[instance release];
		}
		NSObject *delegate = [[NSObject alloc] init];
		AssetCacheServicesManager *manager = [[AssetCacheServicesManager alloc] initWithDelegate:delegate
			delegateQueue:dispatch_get_main_queue()];
		if (!manager) {
			fprintf(stderr, "FAIL: initWithDelegate:delegateQueue: returned nil\n");
			failures++;
		} else if ([manager isActivated]) {
			fprintf(stderr, "FAIL: isActivated is YES without a content caching service\n");
			failures++;
		}
		[manager release];
		[delegate release];
	}
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}

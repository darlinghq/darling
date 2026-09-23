// Guest test: AssetCacheServicesExtensions exports the metrics database location Apple documents,
// and its classes exist and instantiate. Exits non-zero on failure.
#import <AssetCacheServicesExtensions/AssetCacheServicesExtensions.h>
#include <objc/runtime.h>
#include <stdio.h>

int main(void)
{
	int failures = 0;
	NSString *path = [kACMetricsDatabaseDirectory stringByAppendingPathComponent:kACMetricsDatabaseName];
	if (![path isEqualToString:@"/Library/Application Support/Apple/AssetCache/Metrics/Metrics.db"]) {
		fprintf(stderr, "FAIL: metrics database path is %s\n", path.UTF8String);
		failures++;
	}
	if ([[NSFileManager defaultManager] fileExistsAtPath:path])
		printf("note: a metrics database exists at %s\n", path.UTF8String);
	for (Class c in @[[AssetCacheMetricsReader class], [AssetCacheServicesManager class]]) {
		if (![[c alloc] init]) {
			fprintf(stderr, "FAIL: %s did not instantiate\n", class_getName(c));
			failures++;
		}
	}
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}

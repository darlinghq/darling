// Guest test: the call sequence Activity Monitor was observed to send (alloc/init, -gpus, release)
// gets an empty array, and a selector Darling does not implement still raises. Exits non-zero on failure.
#import <SafeEjectGPU/SafeEjectGPU.h>
#include <stdio.h>

int main(void)
{
	int failures = 0;
	@autoreleasepool {
		SafeEjectGPU *gpu = [[SafeEjectGPU alloc] init];
		if (!gpu) {
			fprintf(stderr, "FAIL: SafeEjectGPU did not instantiate\n");
			return 1;
		}
		NSArray *gpus = [gpu gpus];
		if (![gpus isKindOfClass:[NSArray class]] || [gpus count] != 0) {
			fprintf(stderr, "FAIL: -gpus returned %s, expected an empty array\n",
				[[gpus description] UTF8String]);
			failures++;
		}
		@try {
			[gpu performSelector:NSSelectorFromString(@"notImplementedByDarling")];
			fprintf(stderr, "FAIL: an unimplemented selector did not raise\n");
			failures++;
		} @catch (NSException *e) {
		}
		[gpu release];
	}
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}

// Guest test: SafeEjectGPU exists and instantiates, and an unimplemented selector raises instead of
// silently answering. Exits non-zero on failure.
#import <SafeEjectGPU/SafeEjectGPU.h>
#include <stdio.h>

int main(void)
{
	int failures = 0;
	SafeEjectGPU *gpu = [[SafeEjectGPU alloc] init];
	if (!gpu) {
		fprintf(stderr, "FAIL: SafeEjectGPU did not instantiate\n");
		failures++;
	}
	@try {
		[gpu performSelector:NSSelectorFromString(@"notImplementedByDarling")];
		fprintf(stderr, "FAIL: an unimplemented selector did not raise\n");
		failures++;
	} @catch (NSException *e) {
	}
	printf("%s\n", failures ? "FAIL" : "PASS");
	return failures != 0;
}

// CFLAGS: -framework DeviceCheck -framework Foundation
#import <DeviceCheck/DeviceCheck.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL: %s (line %d)\n", #c, __LINE__); return 1; } } while (0)

int main(void) {
    @autoreleasepool {
        DCAppAttestService *s1 = [DCAppAttestService sharedService];
        DCAppAttestService *s2 = DCAppAttestService.sharedService;
        CHECK(s1 != nil);
        CHECK(s1 == s2);
        CHECK([s1 isSupported] == NO);
        CHECK(DCAppAttestService.sharedService.supported == NO);
    }
    puts("PASS");
    return 0;
}

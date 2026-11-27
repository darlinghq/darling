// CFLAGS: -framework CoreBluetooth -framework Foundation
#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>
#include <stdio.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL: %s (line %d)\n", #c, __LINE__); return 1; } } while (0)

int main(void) {
    @autoreleasepool {
        CHECK([CBCentralManagerOptionShowPowerAlertKey isEqualToString:@"kCBInitOptionShowPowerAlert"]);
        CHECK(![CBCentralManagerOptionShowPowerAlertKey isEqualToString:CBCentralManagerScanOptionAllowDuplicatesKey]);
    }
    puts("PASS");
    return 0;
}

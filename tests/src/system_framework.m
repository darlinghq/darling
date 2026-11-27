#import <Foundation/Foundation.h>
#include <dlfcn.h>
#include <stdio.h>

#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL: %s (line %d)\n", #c, __LINE__); return 1; } } while (0)

int main(void) {
    @autoreleasepool {
        void *handle = dlopen("/System/Library/Frameworks/System.framework/System", RTLD_LAZY);
        CHECK(handle != NULL);
        CHECK(dlsym(handle, "getpid") != NULL);
        dlclose(handle);

        NSBundle *bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/System.framework"];
        CHECK(bundle != nil);
        CHECK([[bundle bundleIdentifier] isEqualToString:@"com.apple.System"]);
        CHECK([[bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"] isEqualToString:@"11"]);
        CHECK([[bundle objectForInfoDictionaryKey:@"CFBundleVersion"] isEqualToString:@"1281.0.0"]);
    }
    puts("PASS");
    return 0;
}

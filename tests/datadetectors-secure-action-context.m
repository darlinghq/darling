// Guest test: the DataDetectors class DDSecureActionContext, which WebKit
// looks up (and treats as missing support if absent) whenever it encodes an
// object for IPC. Observed on macOS 26.7.3 (x86_64): the class exists, is a
// subclass of DDActionContext, and can be instantiated.
#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#include <dlfcn.h>
#include <stdio.h>

static int failures;

static void report(const char *name, BOOL ok)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
    if (!ok)
        failures++;
}

int main(void)
{
    @autoreleasepool {
        report("the framework loads", dlopen("/System/Library/PrivateFrameworks/DataDetectors.framework/DataDetectors", RTLD_NOW) != NULL);
        Class secure = NSClassFromString(@"DDSecureActionContext");
        Class context = NSClassFromString(@"DDActionContext");
        report("DDSecureActionContext exists", secure != Nil);
        report("it is a subclass of DDActionContext", context != Nil && class_getSuperclass(secure) == context);
        id object = [[secure alloc] init];
        report("it can be instantiated", object != nil && [object isKindOfClass: context]);
        [object release];
    }
    return failures != 0;
}

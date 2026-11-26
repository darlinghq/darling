// Copyright (C) 2026 Lubos Dolezel
//
// Guest test: +[LSBundleProxy bundleProxyWithAuditToken:error:].
// Observed on macOS 26.7.3 (x86_64) with the same calls:
// - LSBundleProxy descends from LSResourceProxy, which descends from
//   _LSQueryResult;
// - for a process whose executable is not a bundle's, nil and
//   NSOSStatusErrorDomain paramErr (-50);
// - for a token naming no process (all zero), nil and procNotFound (-600);
// - for a process running a bundle's executable, a proxy with the bundle's
//   identifier, and as its name the bundle's file name without the
//   extension (not CFBundleName or CFBundleDisplayName). The test runs a copy of itself from a bundle it
//   makes in a temporary directory to check this.
#import <Foundation/Foundation.h>
#import <objc/runtime.h>
#include <bsm/libbsm.h>
#include <mach/mach.h>
#include <spawn.h>
#include <sys/wait.h>

@interface LSBundleProxy : NSObject
+ (instancetype)bundleProxyWithAuditToken:(audit_token_t)auditToken error:(NSError **)outError;
@property (nonatomic, readonly) NSString *bundleIdentifier;
@property (nonatomic, readonly) NSString *localizedName;
@end

extern char **environ;

static int failures;

static void report(const char *name, BOOL ok)
{
    printf("%s: %s\n", ok ? "PASS" : "FAIL", name);
    if (!ok)
        failures++;
}

static audit_token_t ownToken(void)
{
    audit_token_t token;
    mach_msg_type_number_t count = TASK_AUDIT_TOKEN_COUNT;
    task_info(mach_task_self(), TASK_AUDIT_TOKEN, (task_info_t) &token, &count);
    return token;
}

int main(int argc, char **argv)
{
    @autoreleasepool {
        if (argc > 1 && strcmp(argv[1], "--in-bundle") == 0) {
            NSError *error = nil;
            LSBundleProxy *proxy = [objc_getClass("LSBundleProxy") bundleProxyWithAuditToken: ownToken() error: &error];
            printf("%s|%s\n", proxy.bundleIdentifier.UTF8String ?: "(none)", proxy.localizedName.UTF8String ?: "(none)");
            return 0;
        }

        Class bundleProxy = objc_getClass("LSBundleProxy");
        report("LSBundleProxy descends from LSResourceProxy", class_getSuperclass(bundleProxy) == objc_getClass("LSResourceProxy"));
        report("LSResourceProxy descends from _LSQueryResult", class_getSuperclass(objc_getClass("LSResourceProxy")) == objc_getClass("_LSQueryResult"));

        NSError *error = nil;
        id proxy = [bundleProxy bundleProxyWithAuditToken: ownToken() error: &error];
        report("not a bundle: nil, paramErr", proxy == nil && [error.domain isEqualToString: NSOSStatusErrorDomain] && error.code == -50);

        audit_token_t zero;
        memset(&zero, 0, sizeof zero);
        error = nil;
        proxy = [bundleProxy bundleProxyWithAuditToken: zero error: &error];
        report("no process: nil, procNotFound", proxy == nil && [error.domain isEqualToString: NSOSStatusErrorDomain] && error.code == -600);

        NSFileManager *files = [NSFileManager defaultManager];
        NSString *root = [NSTemporaryDirectory() stringByAppendingPathComponent: [NSString stringWithFormat: @"bundle-proxy-%d", getpid()]];
        NSString *app = [root stringByAppendingPathComponent: @"Proxy Test.app"];
        NSString *macOS = [app stringByAppendingPathComponent: @"Contents/MacOS"];
        [files createDirectoryAtPath: macOS withIntermediateDirectories: YES attributes: nil error: NULL];
        NSDictionary *info = @{ @"CFBundleIdentifier": @"org.darlinghq.test.bundle-proxy", @"CFBundleName": @"Other Name", @"CFBundleDisplayName": @"Shown Name",
                                @"CFBundleExecutable": @"ProxyTest", @"CFBundlePackageType": @"APPL" };
        [info writeToFile: [app stringByAppendingPathComponent: @"Contents/Info.plist"] atomically: YES];
        NSString *executable = [macOS stringByAppendingPathComponent: @"ProxyTest"];
        NSString *self = [[NSBundle mainBundle] executablePath];
        [files copyItemAtPath: self toPath: executable error: NULL];

        int fds[2];
        pipe(fds);
        posix_spawn_file_actions_t actions;
        posix_spawn_file_actions_init(&actions);
        posix_spawn_file_actions_adddup2(&actions, fds[1], 1);
        posix_spawn_file_actions_addclose(&actions, fds[0]);
        char *childArgs[] = { (char *) executable.fileSystemRepresentation, "--in-bundle", NULL };
        pid_t child;
        int spawned = posix_spawn(&child, childArgs[0], &actions, NULL, childArgs, environ);
        close(fds[1]);
        NSFileHandle *reader = [[[NSFileHandle alloc] initWithFileDescriptor: fds[0] closeOnDealloc: YES] autorelease];
        NSString *output = [[[NSString alloc] initWithData: [reader readDataToEndOfFile] encoding: NSUTF8StringEncoding] autorelease];
        int status = 0;
        if (spawned == 0)
            waitpid(child, &status, 0);
        report("bundle: identifier and name",
               [[output stringByTrimmingCharactersInSet: [NSCharacterSet newlineCharacterSet]] isEqualToString: @"org.darlinghq.test.bundle-proxy|Proxy Test"]);

        [files removeItemAtPath: root error: NULL];
    }
    return failures != 0;
}

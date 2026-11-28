// probe_bundle.h - relaunch the running probe inside a throw-away application bundle whose Info.plist is built
// from the arguments. Public API only (NSFileManager, NSDictionary, NSTask). The bundle is created under the
// temporary directory, holds a copy of the probe's own executable, and is removed afterwards (only what this
// function created). The child inherits stdout, so its PROBE lines appear in the parent's output.
#import <Foundation/Foundation.h>

// docTypes: the CFBundleDocumentTypes array (nil = the key is absent). tag names the variant in the identifier.
// Returns the child's exit status, or -1 if the bundle could not be made.
static inline int probe_run_in_bundle(NSArray *docTypes, NSString *tag, NSArray *args) {
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *tmpl = [NSTemporaryDirectory() stringByAppendingPathComponent:@"probebundle.XXXXXX"];
    char *buf = strdup([tmpl fileSystemRepresentation]);
    if (mkdtemp(buf) == NULL) { free(buf); return -1; }
    NSString *dir = [fm stringWithFileSystemRepresentation:buf length:strlen(buf)];
    free(buf);
    NSString *macos = [dir stringByAppendingPathComponent:@"Probe.app/Contents/MacOS"];
    int status = -1;
    if ([fm createDirectoryAtPath:macos withIntermediateDirectories:YES attributes:nil error:NULL]) {
        NSString *exe = [macos stringByAppendingPathComponent:@"probe"];
        NSMutableDictionary *info = [@{@"CFBundleExecutable": @"probe", @"CFBundleIdentifier": [@"example.invalid.probe." stringByAppendingString:tag],
                                       @"CFBundleName": @"probe", @"CFBundlePackageType": @"APPL", @"CFBundleVersion": @"1", @"NSPrincipalClass": @"NSApplication"} mutableCopy];
        if (docTypes) info[@"CFBundleDocumentTypes"] = docTypes;
        BOOL ok = [fm copyItemAtPath:[[NSBundle mainBundle] executablePath] toPath:exe error:NULL] &&
                  [info writeToFile:[dir stringByAppendingPathComponent:@"Probe.app/Contents/Info.plist"] atomically:YES];
        if (ok) {
            NSTask *t = [[NSTask alloc] init];
            [t setExecutableURL:[NSURL fileURLWithPath:exe]];
            [t setArguments:args];
            NSError *e = nil;
            if ([t launchAndReturnError:&e]) { [t waitUntilExit]; status = [t terminationStatus]; }
        }
    }
    [fm removeItemAtPath:dir error:NULL];
    return status;
}

static inline BOOL probe_has_arg(int argc, char **argv, const char *flag) {
    for (int i = 1; i < argc; i++) if (!strcmp(argv[i], flag)) return YES;
    return NO;
}

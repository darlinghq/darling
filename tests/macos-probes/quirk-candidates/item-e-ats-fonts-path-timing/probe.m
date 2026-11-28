// Item (e): when the fonts named by the Info.plist key ATSApplicationFontsPath become registered for the process:
// before main(), at the start of main(), when NSApplication is created, in applicationWillFinishLaunching:, in
// applicationDidFinishLaunching:, or after the first run-loop turn. The probe is built into an app bundle whose
// Info.plist has ATSApplicationFontsPath = "Fonts" and whose Resources/Fonts holds one font file (build.txt).
// "Registered" is observed through the process-scope list CTFontManagerCopyRegisteredFontDescriptors (macOS 12+;
// before that only the lookup by name is printed) and by looking the font up by its PostScript name.
#define PROBE_APPKIT 1
#import "probe_common.h"
#import <CoreText/CoreText.h>

static NSString *gPS;

static NSString *stage(void) {
    NSMutableArray *parts = [NSMutableArray array];
#if PROBE_SDK_AT_LEAST(120000)
    if (@available(macOS 12.0, *)) {
        CFArrayRef d = CTFontManagerCopyRegisteredFontDescriptors(kCTFontManagerScopeProcess, true);
        NSInteger n = d ? CFArrayGetCount(d) : 0, mine = 0;
        for (NSInteger i = 0; i < n; i++) {
            CTFontDescriptorRef fd = (CTFontDescriptorRef)CFArrayGetValueAtIndex(d, i);
            CFStringRef ps = CTFontDescriptorCopyAttribute(fd, kCTFontNameAttribute);
            if (ps && gPS && [(__bridge NSString *)ps isEqualToString:gPS]) mine++;
            if (ps) CFRelease(ps);
        }
        if (d) CFRelease(d);
        [parts addObject:PF(@"process-scope registered descriptors=%ld of which the bundled font=%ld", (long)n, (long)mine)];
    }
#endif
    CTFontDescriptorRef byName = CTFontDescriptorCreateWithNameAndSize((__bridge CFStringRef)(gPS ?: @""), 12);
    CFURLRef url = byName ? CTFontDescriptorCopyAttribute(byName, kCTFontURLAttribute) : NULL;
    NSString *where = url ? ([[(__bridge NSURL *)url path] hasPrefix:[[NSBundle mainBundle] bundlePath]] ? @"inside this app bundle" : @"outside this app bundle (an installed copy)") : @"not found";
    [parts addObject:PF(@"font by PostScript name resolves to a file %@", where)];
    if (url) CFRelease(url);
    if (byName) CFRelease(byName);
    return [parts componentsJoinedByString:@" ; "];
}

static void logStage(const char *when) { probe_line("item-e.stage", PF(@"%s", when), stage()); }

__attribute__((constructor)) static void early(void) { probe_line("item-e.stage", @"constructor (before main)", @"(no font name known yet; see the later stages)"); }

@interface Delegate : NSObject <NSApplicationDelegate>
@end
@implementation Delegate
- (void)applicationWillFinishLaunching:(NSNotification *)n { logStage("applicationWillFinishLaunching:"); }
- (void)applicationDidFinishLaunching:(NSNotification *)n {
    logStage("applicationDidFinishLaunching:");
    dispatch_async(dispatch_get_main_queue(), ^{ logStage("first main-queue turn after launch"); [NSApp terminate:nil]; });
}
@end

int main(void) {
    @autoreleasepool {
        probe_env("item-e");
        NSString *dir = [[NSBundle mainBundle] objectForInfoDictionaryKey:@"ATSApplicationFontsPath"];
        probe_line("item-e.bundle", @"ATSApplicationFontsPath in Info.plist", dir ? PF(@"\"%@\"", dir) : @"missing (run the bundle built by build.txt)");
        NSString *fontDir = [[[NSBundle mainBundle] resourcePath] stringByAppendingPathComponent:dir ?: @"Fonts"];
        NSArray *files = [[NSFileManager defaultManager] contentsOfDirectoryAtPath:fontDir error:nil];
        for (NSString *f in files) {
            NSData *bytes = [NSData dataWithContentsOfFile:[fontDir stringByAppendingPathComponent:f]];
            CGDataProviderRef dp = bytes ? CGDataProviderCreateWithCFData((__bridge CFDataRef)bytes) : NULL;
            CGFontRef cg = dp ? CGFontCreateWithDataProvider(dp) : NULL;
            if (cg) {
                gPS = (__bridge_transfer NSString *)CGFontCopyPostScriptName(cg);
                CGFontRelease(cg);
            }
            if (dp) CGDataProviderRelease(dp);
            if (gPS) break;
        }
        probe_line("item-e.bundle", @"font file in the bundle", gPS ? PF(@"PostScript name %@", gPS) : @"none found");
        logStage("start of main()");
        [NSApplication sharedApplication];
        logStage("after +[NSApplication sharedApplication]");
        [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
        Delegate *d = [[Delegate alloc] init];
        [NSApp setDelegate:d];
        [NSApp run];
    }
    return 0;
}

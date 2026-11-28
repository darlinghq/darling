// Probe 665: -[NSDocumentController typeForContentsOfURL:error:] for URLs whose type the controller does not
// recognise (and, as controls, ones it does). The probe relaunches itself in throw-away application bundles
// (probe_bundle.h):  b1 = no CFBundleDocumentTypes;  b2 = types declared by extension and by content type
// (an Editor "ProbeText" for extension ptxt and public.plain-text, a Viewer "ProbeView" for pview, a role None
// entry for pnone, an entry with only a content type public.png).
// For each URL the child prints the returned type or nil and the error: domain, code, localized description,
// user-info keys, underlying error. URLs: declared and undeclared extensions (also upper case), no extension,
// existing empty file, a text file with extension txt, a directory (plain and with a declared extension), a
// missing file with declared and undeclared extensions, a non-file URL, a file URL with a fragment.
// Also with an error pointer of NULL, and nil URL (exception name is printed, nothing is raised in the probe).
// Window server: needed for NSApplication only (no window shown, not activated).
#define PROBE_APPKIT 1
#import "probe_common.h"
#import "probe_bundle.h"
#include <unistd.h>

static NSString *err(NSError *e) {
    if (!e) return @"nil";
    NSError *u = [[e userInfo] objectForKey:NSUnderlyingErrorKey];
    return PF(@"%@/%ld \"%@\" reason=\"%@\" userInfoKeys=[%@] underlying=%@", [e domain], (long)[e code], [e localizedDescription] ?: @"", [e localizedFailureReason] ?: @"nil",
              [[[[e userInfo] allKeys] sortedArrayUsingSelector:@selector(compare:)] componentsJoinedByString:@","], u ? PF(@"%@/%ld", [u domain], (long)[u code]) : @"nil");
}

static NSDictionary *T(NSString *name, NSString *role, NSArray *ext, NSArray *uti) {
    NSMutableDictionary *d = [NSMutableDictionary dictionary];
    if (name) d[@"CFBundleTypeName"] = name;
    d[@"CFBundleTypeRole"] = role;
    if (ext) d[@"CFBundleTypeExtensions"] = ext;
    if (uti) d[@"LSItemContentTypes"] = uti;
    return d;
}

static void child(const char *tag) {
    probe_app();
    NSDocumentController *dc = [NSDocumentController sharedDocumentController];
    probe_line("665.child", PF(@"%s defaultType / documentClassNames", tag), PF(@"%@ / %@", [dc defaultType] ?: @"nil", [[dc documentClassNames] componentsJoinedByString:@","] ?: @"nil"));
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent:PF(@"p665.%d", (int)getpid())];
    [fm createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:NULL];
    NSData *txt = [@"probe text\n" dataUsingEncoding:NSUTF8StringEncoding];
    for (NSString *n in @[@"a.ptxt", @"b.PTXT", @"c.pview", @"d.pnone", @"e.zzz", @"f", @"g.txt", @"h.png", @"i.unknownext.ptxt"]) [fm createFileAtPath:[dir stringByAppendingPathComponent:n] contents:txt attributes:nil];
    [fm createFileAtPath:[dir stringByAppendingPathComponent:@"empty.ptxt"] contents:[NSData data] attributes:nil];
    [fm createDirectoryAtPath:[dir stringByAppendingPathComponent:@"folder"] withIntermediateDirectories:NO attributes:nil error:NULL];
    [fm createDirectoryAtPath:[dir stringByAppendingPathComponent:@"folder.ptxt"] withIntermediateDirectories:NO attributes:nil error:NULL];
    NSURL *base = [NSURL fileURLWithPath:dir];
    NSArray *cases = @[
        @[@"declared extension (ptxt)", [base URLByAppendingPathComponent:@"a.ptxt"]],
        @[@"declared extension, upper case (PTXT)", [base URLByAppendingPathComponent:@"b.PTXT"]],
        @[@"declared extension, viewer role (pview)", [base URLByAppendingPathComponent:@"c.pview"]],
        @[@"declared extension, role None (pnone)", [base URLByAppendingPathComponent:@"d.pnone"]],
        @[@"undeclared extension (zzz)", [base URLByAppendingPathComponent:@"e.zzz"]],
        @[@"no extension", [base URLByAppendingPathComponent:@"f"]],
        @[@"txt file (public.plain-text declared in b2)", [base URLByAppendingPathComponent:@"g.txt"]],
        @[@"png extension, text content (public.png declared in b2)", [base URLByAppendingPathComponent:@"h.png"]],
        @[@"two extensions, last declared", [base URLByAppendingPathComponent:@"i.unknownext.ptxt"]],
        @[@"empty file, declared extension", [base URLByAppendingPathComponent:@"empty.ptxt"]],
        @[@"directory, no extension", [base URLByAppendingPathComponent:@"folder"]],
        @[@"directory with declared extension", [base URLByAppendingPathComponent:@"folder.ptxt"]],
        @[@"missing file, declared extension", [base URLByAppendingPathComponent:@"missing.ptxt"]],
        @[@"missing file, undeclared extension", [base URLByAppendingPathComponent:@"missing.zzz"]],
        @[@"non-file URL, declared extension", [NSURL URLWithString:@"http://example.invalid/x.ptxt"]],
        @[@"non-file URL, undeclared extension", [NSURL URLWithString:@"http://example.invalid/x.zzz"]],
        @[@"file URL with fragment", [NSURL URLWithString:[[[base URLByAppendingPathComponent:@"e.zzz"] absoluteString] stringByAppendingString:@"#frag"]]],
    ];
    for (NSArray *c in cases) {
        __block NSError *e = nil; NSError *marker = [NSError errorWithDomain:@"Marker" code:1 userInfo:nil];
        __block NSError *pre = marker;
        __block NSString *type = nil;
        NSURL *u = c[1];
        NSString *ex = probe_try(^{ type = [dc typeForContentsOfURL:u error:&e]; });
        NSString *ex2 = probe_try(^{ (void)[dc typeForContentsOfURL:u error:&pre]; });
        NSString *ex3 = probe_try(^{ (void)[dc typeForContentsOfURL:u error:NULL]; });
        probe_line("665.type", PF(@"%s %@", tag, c[0]), PF(@"%@ ; type=%@ ; error=%@ ; with pre-filled error and a type found, marker %@ ; NULL error: %@", ex, type ? PF(@"\"%@\"", type) : @"nil", err(e), pre == marker ? @"untouched" : @"replaced", ex3));
        (void)ex2;
    }
    __block NSString *t = nil; __block NSError *e = nil;
    NSString *ex = probe_try(^{ t = [dc typeForContentsOfURL:nil error:&e]; });
    probe_line("665.type", PF(@"%s nil URL", tag), PF(@"%@ ; type=%@ ; error=%@", ex, t ?: @"nil", err(e)));
    [fm removeItemAtPath:dir error:NULL];
}

int main(int argc, char **argv) {
    @autoreleasepool {
        if (probe_has_arg(argc, argv, "--child")) { child(argc > 2 ? argv[2] : "?"); return 0; }
        probe_env("665");
        probe_line("665.variant", @"b1: no CFBundleDocumentTypes", @"(child output follows)");
        probe_run_in_bundle(nil, @"b1", @[@"--child", @"b1"]);
        probe_line("665.variant", @"b2: ptxt+public.plain-text Editor, pview Viewer, pnone role None, public.png only", @"(child output follows)");
        probe_run_in_bundle(@[T(@"ProbeText", @"Editor", @[@"ptxt"], @[@"public.plain-text"]), T(@"ProbeView", @"Viewer", @[@"pview"], nil), T(@"ProbeNone", @"None", @[@"pnone"], nil), T(@"ProbePng", @"Editor", nil, @[@"public.png"])], @"b2", @[@"--child", @"b2"]);
    }
    return 0;
}

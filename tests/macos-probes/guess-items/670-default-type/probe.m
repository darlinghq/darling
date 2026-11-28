// Probe 670: -[NSDocumentController defaultType] and what the controller reports for different
// CFBundleDocumentTypes. The probe relaunches itself inside a throw-away application bundle per variant (see
// probe_bundle.h) with an Info.plist whose CFBundleDocumentTypes is the variant, and in each child prints:
// defaultType, documentClassNames, and for each declared type name documentClassForType:, displayNameForType:,
// fileExtensionsFromType:, plus the class of the document -makeUntitledDocumentOfType:error: makes for the default
// type. Variants: roles Editor, Viewer, None, Shell, no role key; entries with no name and no content type,
// with an empty name, with only a content type, with only a name; different orders.
// Window server: needed for NSApplication only (no window is created, the application is not activated).
#define PROBE_APPKIT 1
#import "probe_common.h"
#import "probe_bundle.h"

@interface DocA : NSDocument
@end
@implementation DocA
- (NSData *)dataOfType:(NSString *)t error:(NSError **)e { return [NSData data]; }
- (BOOL)readFromData:(NSData *)d ofType:(NSString *)t error:(NSError **)e { return YES; }
@end
@interface DocB : DocA
@end
@implementation DocB
@end

static NSDictionary *T(NSString *name, NSString *role, NSString *cls, NSArray *ext, NSArray *uti) {
    NSMutableDictionary *d = [NSMutableDictionary dictionary];
    if (name) d[@"CFBundleTypeName"] = name;
    if (role) d[@"CFBundleTypeRole"] = role;
    if (cls) d[@"NSDocumentClass"] = cls;
    if (ext) d[@"CFBundleTypeExtensions"] = ext;
    if (uti) d[@"LSItemContentTypes"] = uti;
    return d;
}

static NSArray *variants(void) {
    NSArray *txt = @[@"public.plain-text"];
    return @[
        @[@"v01 Editor Alpha, Viewer Beta", @[T(@"Alpha", @"Editor", @"DocA", @[@"pa"], nil), T(@"Beta", @"Viewer", @"DocB", @[@"pb"], nil)]],
        @[@"v02 Viewer Beta, Editor Alpha", @[T(@"Beta", @"Viewer", @"DocB", @[@"pb"], nil), T(@"Alpha", @"Editor", @"DocA", @[@"pa"], nil)]],
        @[@"v03 Viewer Beta, Viewer Gamma", @[T(@"Beta", @"Viewer", @"DocB", @[@"pb"], nil), T(@"Gamma", @"Viewer", @"DocA", @[@"pg"], nil)]],
        @[@"v04 no role Alpha, Editor Beta", @[T(@"Alpha", nil, @"DocA", @[@"pa"], nil), T(@"Beta", @"Editor", @"DocB", @[@"pb"], nil)]],
        @[@"v05 only: no role Alpha", @[T(@"Alpha", nil, @"DocA", @[@"pa"], nil)]],
        @[@"v06 role None Alpha, Editor Beta", @[T(@"Alpha", @"None", @"DocA", @[@"pa"], nil), T(@"Beta", @"Editor", @"DocB", @[@"pb"], nil)]],
        @[@"v07 role Shell Alpha, Editor Beta", @[T(@"Alpha", @"Shell", @"DocA", @[@"pa"], nil), T(@"Beta", @"Editor", @"DocB", @[@"pb"], nil)]],
        @[@"v08 Editor with no name and no content type (class and extension only), Editor Beta", @[T(nil, @"Editor", @"DocA", @[@"pa"], nil), T(@"Beta", @"Editor", @"DocB", @[@"pb"], nil)]],
        @[@"v09 Editor with empty name, Editor Beta", @[T(@"", @"Editor", @"DocA", @[@"pa"], nil), T(@"Beta", @"Editor", @"DocB", @[@"pb"], nil)]],
        @[@"v10 Editor with a content type only (no name), Editor Beta", @[T(nil, @"Editor", @"DocA", nil, txt), T(@"Beta", @"Editor", @"DocB", @[@"pb"], nil)]],
        @[@"v11 Editor with a name only (no class, no extension, no content type), Editor Beta", @[T(@"Alpha", @"Editor", nil, nil, nil), T(@"Beta", @"Editor", @"DocB", @[@"pb"], nil)]],
        @[@"v12 only: Editor with no name and no content type", @[T(nil, @"Editor", @"DocA", @[@"pa"], nil)]],
        @[@"v13 only: Viewer Beta", @[T(@"Beta", @"Viewer", @"DocB", @[@"pb"], nil)]],
        @[@"v14 Editor Alpha, Editor Beta (baseline)", @[T(@"Alpha", @"Editor", @"DocA", @[@"pa"], nil), T(@"Beta", @"Editor", @"DocB", @[@"pb"], nil)]],
        @[@"v15 Viewer Alpha with content type, Editor with content type", @[T(@"Alpha", @"Viewer", @"DocA", nil, @[@"public.png"]), T(@"Beta", @"Editor", @"DocB", nil, txt)]],
        @[@"v16 no CFBundleDocumentTypes key", [NSNull null]],
        @[@"v17 empty CFBundleDocumentTypes array", @[]],
    ];
}

static NSString *err(NSError *e) { return e ? PF(@"%@/%ld", [e domain], (long)[e code]) : @"nil"; }

static void child(const char *tag) {
    probe_app();
    NSDocumentController *dc = [NSDocumentController sharedDocumentController];
    NSString *dt = [dc defaultType];
    probe_line("670.child", PF(@"%s defaultType", tag), dt ? PF(@"\"%@\"", dt) : @"nil");
    NSArray *names = [dc documentClassNames];
    probe_line("670.child", PF(@"%s documentClassNames", tag), PF(@"%@", names ? [names componentsJoinedByString:@","] : @"nil"));
    NSDictionary *info = [[NSBundle mainBundle] infoDictionary];
    for (NSDictionary *d in info[@"CFBundleDocumentTypes"]) {
        NSString *n = d[@"CFBundleTypeName"];
        if (!n) { probe_line("670.child", PF(@"%s declared entry without a name", tag), PF(@"role=%@ class=%@", d[@"CFBundleTypeRole"] ?: @"(none)", d[@"NSDocumentClass"] ?: @"(none)")); continue; }
        Class c = [dc documentClassForType:n];
        probe_line("670.child", PF(@"%s type \"%@\"", tag, n), PF(@"documentClassForType=%@ ; displayNameForType=%@ ; fileExtensionsFromType=%@ ; typeFromFileExtension(pa)=%@",
                   c ? NSStringFromClass(c) : @"nil", [dc displayNameForType:n] ?: @"nil", [[dc fileExtensionsFromType:n] componentsJoinedByString:@","] ?: @"nil", [dc typeFromFileExtension:@"pa"] ?: @"nil"));
    }
    NSError *e = nil;
    NSDocument *u = dt ? [dc makeUntitledDocumentOfType:dt error:&e] : nil;
    probe_line("670.child", PF(@"%s makeUntitledDocumentOfType:defaultType", tag), dt ? PF(@"doc=%@ error=%@", u ? NSStringFromClass([u class]) : @"nil", err(e)) : @"skipped (defaultType is nil)");
    e = nil;
    NSDocument *o = [dc openUntitledDocumentAndDisplay:NO error:&e];
    probe_line("670.child", PF(@"%s openUntitledDocumentAndDisplay:NO", tag), PF(@"doc=%@ type=%@ error=%@", o ? NSStringFromClass([o class]) : @"nil", o ? [o fileType] ?: @"nil" : @"-", err(e)));
}

int main(int argc, char **argv) {
    @autoreleasepool {
        if (probe_has_arg(argc, argv, "--child")) { child(argc > 2 ? argv[2] : "?"); return 0; }
        probe_env("670");
        for (NSArray *v in variants()) {
            NSString *label = v[0];
            probe_line("670.variant", label, @"(child output follows)");
            NSString *tag = [label substringToIndex:3];
            probe_run_in_bundle([v[1] isKindOfClass:[NSNull class]] ? nil : v[1], tag, @[@"--child", tag]);
        }
    }
    return 0;
}

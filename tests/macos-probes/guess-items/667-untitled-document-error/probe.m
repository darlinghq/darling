// Probe 667: what -[NSDocumentController makeUntitledDocumentOfType:error:] reports when it cannot make a document.
// The probe relaunches itself in throw-away application bundles (probe_bundle.h):
//   b1 : no CFBundleDocumentTypes at all
//   b2 : types whose document class is missing, is not an NSDocument subclass, whose -initWithType:error: returns
//        nil with an error, nil without an error, or succeeds; and a type with no NSDocumentClass key
// and calls the method for every declared type, an undeclared name, nil and the empty string, with an error
// pointer pre-filled with a marker error and with NULL. Prints the returned class, the error domain, code,
// description and user-info keys (or "nil"), and whether a pre-filled marker survived. Also prints the result of
// -openUntitledDocumentAndDisplay:error: for the default type, and the documentClassForType: answers.
// Window server: needed for NSApplication only (no window shown, not activated).
#define PROBE_APPKIT 1
#import "probe_common.h"
#import "probe_bundle.h"

@interface OkDoc : NSDocument
@end
@implementation OkDoc
- (NSData *)dataOfType:(NSString *)t error:(NSError **)e { return [NSData data]; }
@end
@interface NilErrDoc : NSDocument
@end
@implementation NilErrDoc
- (instancetype)initWithType:(NSString *)t error:(NSError **)e {
    if (e) *e = [NSError errorWithDomain:@"ProbeDomain" code:42 userInfo:@{NSLocalizedDescriptionKey: @"probe init failure"}];
    return nil;
}
@end
@interface NilNoErrDoc : NSDocument
@end
@implementation NilNoErrDoc
- (instancetype)initWithType:(NSString *)t error:(NSError **)e { return nil; }
@end
@interface NotADoc : NSObject
@end
@implementation NotADoc
@end

static NSDictionary *T(NSString *name, NSString *cls) {
    NSMutableDictionary *d = [@{@"CFBundleTypeRole": @"Editor", @"CFBundleTypeExtensions": @[[@"p" stringByAppendingString:[name lowercaseString]]]} mutableCopy];
    if (name) d[@"CFBundleTypeName"] = name;
    if (cls) d[@"NSDocumentClass"] = cls;
    return d;
}

static NSString *err(NSError *e) {
    if (!e) return @"nil";
    NSString *d = [e localizedDescription] ?: @"";
    return PF(@"%@/%ld \"%@\" userInfoKeys=[%@]", [e domain], (long)[e code], d, [[[e userInfo] allKeys] componentsJoinedByString:@","]);
}

static void child(const char *tag) {
    probe_app();
    NSDocumentController *dc = [NSDocumentController sharedDocumentController];
    NSMutableArray *names = [NSMutableArray array];
    for (NSDictionary *d in [[NSBundle mainBundle] infoDictionary][@"CFBundleDocumentTypes"]) if (d[@"CFBundleTypeName"]) [names addObject:d[@"CFBundleTypeName"]];
    probe_line("667.child", PF(@"%s defaultType / documentClassNames", tag), PF(@"%@ / %@", [dc defaultType] ?: @"nil", [[dc documentClassNames] componentsJoinedByString:@","] ?: @"nil"));
    NSArray *inputs = [names arrayByAddingObjectsFromArray:@[@"Undeclared", @"", [NSNull null]]];
    for (id n in inputs) {
        NSString *type = [n isKindOfClass:[NSNull class]] ? nil : n;
        NSString *label = type ? PF(@"\"%@\"", type) : @"nil";
        Class c = type ? [dc documentClassForType:type] : Nil;
        NSError *marker = [NSError errorWithDomain:@"Marker" code:1 userInfo:nil];
        __block NSError *e = marker;
        __block NSDocument *doc = nil;
        NSString *ex = probe_try(^{ doc = [dc makeUntitledDocumentOfType:type error:&e]; });
        probe_line("667.make", PF(@"%s type %@ (documentClassForType=%@), error pre-filled", tag, label, c ? NSStringFromClass(c) : @"nil"),
                   PF(@"%@ ; doc=%@ ; error=%@ ; marker %@", ex, doc ? NSStringFromClass([doc class]) : @"nil", e == marker ? @"untouched" : err(e), e == marker ? @"survived" : @"replaced"));
        e = nil;
        ex = probe_try(^{ doc = [dc makeUntitledDocumentOfType:type error:&e]; });
        probe_line("667.make", PF(@"%s type %@, error pre-set to nil", tag, label), PF(@"%@ ; doc=%@ ; error=%@", ex, doc ? NSStringFromClass([doc class]) : @"nil", err(e)));
        ex = probe_try(^{ doc = [dc makeUntitledDocumentOfType:type error:NULL]; });
        probe_line("667.make", PF(@"%s type %@, error NULL", tag, label), PF(@"%@ ; doc=%@", ex, doc ? NSStringFromClass([doc class]) : @"nil"));
    }
    __block NSError *e = nil;
    __block NSDocument *o = nil;
    NSString *ex = probe_try(^{ o = [dc openUntitledDocumentAndDisplay:NO error:&e]; });
    probe_line("667.open", PF(@"%s openUntitledDocumentAndDisplay:NO error:", tag), PF(@"%@ ; doc=%@ ; error=%@", ex, o ? NSStringFromClass([o class]) : @"nil", err(e)));
}

int main(int argc, char **argv) {
    @autoreleasepool {
        if (probe_has_arg(argc, argv, "--child")) { child(argc > 2 ? argv[2] : "?"); return 0; }
        probe_env("667");
        probe_line("667.variant", @"b1: no CFBundleDocumentTypes", @"(child output follows)");
        probe_run_in_bundle(nil, @"b1", @[@"--child", @"b1"]);
        probe_line("667.variant", @"b2: ok class, missing class, not-a-document class, init fails with error, init fails without error, no class key", @"(child output follows)");
        probe_run_in_bundle(@[T(@"Ok", @"OkDoc"), T(@"MissingClass", @"NoSuchClassAnywhere"), T(@"NotDocument", @"NotADoc"), T(@"InitErr", @"NilErrDoc"), T(@"InitNoErr", @"NilNoErrDoc"), T(@"NoClassKey", nil)], @"b2", @[@"--child", @"b2"]);
    }
    return 0;
}

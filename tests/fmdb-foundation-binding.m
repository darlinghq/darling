// FMDB must bind its imports to Foundation, not to CoreFoundation.
//
// Darling's CoreFoundation defines _NSCocoaErrorDomain, _NSLocalizedDescriptionKey and
// _OBJC_CLASS_$_NSError; the macOS CoreFoundation does not (Foundation does). An FMDB that records them against a
// CoreFoundation ordinal dies at bind next to a macOS CoreFoundation ("Symbol not found: _NSCocoaErrorDomain").
// An import can only bind to a dylib that has a load command, so FMDB must not list CoreFoundation at all and must
// list Foundation, which re-exports CoreFoundation and libobjc in both Darling and macOS.
//
// The check is on the load commands of FMDB, which is Darling's own build output. The second half loads FMDB for
// real and runs a SQLite round trip through it, so the relinked image is proven to work.
//
// Build against Foundation (host cross build, as for the other tests here), then run it with `darling shell`:
//   clang -target aarch64-apple-darwin20 -framework Foundation tests/fmdb-foundation-binding.m -o /tmp/fmdb-test
//   darling shell /tmp/fmdb-test
// Optional argument: path of the FMDB image to test (default: the installed one). To test a staged FMDB, set
// DYLD_FRAMEWORK_PATH inside the guest to a directory holding FMDB.framework/FMDB.
// Prints PASS and exits 0, or prints FAIL and exits 1.
#import <Foundation/Foundation.h>
#import <objc/message.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <mach-o/loader.h>
#include <stdio.h>
#include <string.h>

#ifndef LC_LOAD_UPWARD_DYLIB
#define LC_LOAD_UPWARD_DYLIB (0x23 | LC_REQ_DYLD)
#endif

static const struct mach_header_64 *imageNamed(const char *needle)
{
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const char *name = _dyld_get_image_name(i);
        if (name && strstr(name, needle))
            return (const struct mach_header_64 *)_dyld_get_image_header(i);
    }
    return NULL;
}

static int checkLoadCommands(const struct mach_header_64 *mh)
{
    int sawFoundation = 0, sawCoreFoundation = 0;
    const struct load_command *lc = (const struct load_command *)(mh + 1);
    for (uint32_t i = 0; i < mh->ncmds; i++, lc = (const struct load_command *)((const char *)lc + lc->cmdsize)) {
        uint32_t cmd = lc->cmd;
        if (cmd != LC_LOAD_DYLIB && cmd != LC_LOAD_WEAK_DYLIB && cmd != LC_REEXPORT_DYLIB &&
            cmd != LC_LAZY_LOAD_DYLIB && cmd != LC_LOAD_UPWARD_DYLIB)
            continue;
        const struct dylib_command *dc = (const struct dylib_command *)lc;
        const char *name = (const char *)dc + dc->dylib.name.offset;
        printf("  depends on %s\n", name);
        if (strstr(name, "/Foundation.framework/"))
            sawFoundation = 1;
        if (strstr(name, "/CoreFoundation.framework/"))
            sawCoreFoundation = 1;
    }
    int failures = 0;
    if (!sawFoundation) {
        printf("FAIL: FMDB has no Foundation load command\n");
        failures++;
    }
    if (sawCoreFoundation) {
        printf("FAIL: FMDB links CoreFoundation, so imports bind to it; the macOS CoreFoundation lacks the NSError names\n");
        failures++;
    }
    return failures;
}

static int roundTrip(void)
{
    Class dbClass = NSClassFromString(@"FMDatabase");
    if (!dbClass) {
        printf("FAIL: FMDatabase class not found\n");
        return 1;
    }
    id db = ((id (*)(id, SEL, NSString *))objc_msgSend)(dbClass, @selector(databaseWithPath:), @":memory:");
    if (!((BOOL (*)(id, SEL))objc_msgSend)(db, @selector(open))) {
        printf("FAIL: FMDatabase open\n");
        return 1;
    }
    SEL update = @selector(executeUpdate:);
    BOOL ok = ((BOOL (*)(id, SEL, NSString *))objc_msgSend)(db, update, @"CREATE TABLE t (a INTEGER, b TEXT)");
    ok = ok && ((BOOL (*)(id, SEL, NSString *))objc_msgSend)(db, update, @"INSERT INTO t VALUES (42, 'darling')");
    if (!ok) {
        printf("FAIL: FMDatabase executeUpdate:\n");
        return 1;
    }
    id rs = ((id (*)(id, SEL, NSString *))objc_msgSend)(db, @selector(executeQuery:), @"SELECT a, b FROM t");
    if (!rs || !((BOOL (*)(id, SEL))objc_msgSend)(rs, @selector(next))) {
        printf("FAIL: FMDatabase executeQuery: returned no row\n");
        return 1;
    }
    int a = ((int (*)(id, SEL, int))objc_msgSend)(rs, @selector(intForColumnIndex:), 0);
    NSString *b = ((id (*)(id, SEL, int))objc_msgSend)(rs, @selector(stringForColumnIndex:), 1);
    ((void (*)(id, SEL))objc_msgSend)(rs, @selector(close));
    if (a != 42 || ![b isEqualToString:@"darling"]) {
        printf("FAIL: round trip read a=%d b=%s\n", a, [b UTF8String]);
        return 1;
    }

    // Error paths: each builds an NSError from NSLocalizedDescriptionKey, and validateSQL:error: from
    // NSCocoaErrorDomain too (FMDatabase.m lastError, FMDatabaseAdditions.m validateSQL:error:).
    int failures = 0;
    if (((BOOL (*)(id, SEL, NSString *))objc_msgSend)(db, update, @"INSERT INTO no_such_table VALUES (1)")) {
        printf("FAIL: executeUpdate: succeeded on a missing table\n");
        failures++;
    }
    NSError *last = ((id (*)(id, SEL))objc_msgSend)(db, @selector(lastError));
    NSString *desc = last.userInfo[NSLocalizedDescriptionKey];
    if (![desc isKindOfClass:[NSString class]] || ![desc length]) {
        printf("FAIL: lastError has no NSLocalizedDescriptionKey (%s)\n", [[last description] UTF8String]);
        failures++;
    }
    NSError *invalid = nil;
    BOOL valid = ((BOOL (*)(id, SEL, NSString *, NSError **))objc_msgSend)(db, @selector(validateSQL:error:),
                                                                           @"SELECT * FROM no_such_table", &invalid);
    desc = invalid.userInfo[NSLocalizedDescriptionKey];
    if (valid || ![invalid.domain isEqualToString:NSCocoaErrorDomain] || ![desc isKindOfClass:[NSString class]] ||
        ![desc length]) {
        printf("FAIL: validateSQL:error: gave valid=%d error=%s\n", valid, [[invalid description] UTF8String]);
        failures++;
    }
    ((BOOL (*)(id, SEL))objc_msgSend)(db, @selector(close));
    return failures;
}

int main(int argc, const char *argv[])
{
    @autoreleasepool {
        const char *path = argc > 1 ? argv[1] : "/System/Library/PrivateFrameworks/FMDB.framework/Versions/A/FMDB";
        if (!dlopen(path, RTLD_NOW)) {
            printf("FAIL: dlopen(%s): %s\n", path, dlerror());
            return 1;
        }
        const struct mach_header_64 *mh = imageNamed("FMDB.framework");
        if (!mh) {
            printf("FAIL: FMDB image not found after dlopen\n");
            return 1;
        }
        int failures = checkLoadCommands(mh) + roundTrip();
        printf(failures ? "FAIL: fmdb-foundation-binding\n" : "PASS: fmdb-foundation-binding\n");
        return failures ? 1 : 0;
    }
}

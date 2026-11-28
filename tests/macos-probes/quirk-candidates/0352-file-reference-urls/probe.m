// Probe 0352: -[NSURL fileReferenceURL], -[NSURL filePathURL], -[NSURL isFileReferenceURL] for existing and
// missing files, a moved file, a directory, a symlink and a non-file URL. Prints booleans only, no paths.
#import "probe_common.h"
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

static BOOL samePath(NSString *a, NSString *b) {
    char ra[PATH_MAX], rb[PATH_MAX];
    if (a == nil || b == nil || !realpath([a fileSystemRepresentation], ra) || !realpath([b fileSystemRepresentation], rb))
        return NO;
    return strcmp(ra, rb) == 0;
}
static NSString *yesno(BOOL b) { return b ? @"YES" : @"NO"; }
static NSString *nilOr(id o, NSString *(^f)(id)) { return o == nil ? @"nil" : f(o); }

int main(void) {
    @autoreleasepool {
        probe_env("0352");
        NSFileManager *fm = [NSFileManager defaultManager];
        NSString *dir = [NSTemporaryDirectory() stringByAppendingPathComponent:PF(@"probe0352-%d", getpid())];
        [fm createDirectoryAtPath:dir withIntermediateDirectories:YES attributes:nil error:nil];
        NSString *file = [dir stringByAppendingPathComponent:@"a.txt"];
        NSString *moved = [dir stringByAppendingPathComponent:@"b.txt"];
        NSString *missing = [dir stringByAppendingPathComponent:@"missing.txt"];
        NSString *link = [dir stringByAppendingPathComponent:@"link.txt"];
        [@"x" writeToFile:file atomically:NO encoding:NSUTF8StringEncoding error:nil];
        symlink([file fileSystemRepresentation], [link fileSystemRepresentation]);

        NSURL *u = [NSURL fileURLWithPath:file];
        probe_line("0352.isFileReferenceURL", @"path URL of existing file", yesno([u isFileReferenceURL]));
        NSURL *ref = [u fileReferenceURL];
        probe_line("0352.fileReferenceURL", @"existing file", nilOr(ref, ^(id r) {
            return PF(@"non-nil ; isFileReferenceURL=%@ ; isFileURL=%@ ; scheme=%@ ; absoluteString-has-prefix-file:///.file/id= %@",
                      yesno([r isFileReferenceURL]), yesno([r isFileURL]), [r scheme],
                      yesno([[r absoluteString] hasPrefix:@"file:///.file/id="]));
        }));
        probe_line("0352.filePathURL", @"reference URL of existing file", nilOr([ref filePathURL], ^(id p) {
            return PF(@"non-nil ; isFileReferenceURL=%@ ; same-file-as-original %@", yesno([p isFileReferenceURL]),
                      yesno(samePath([p path], file)));
        }));
        probe_line("0352.filePathURL", @"plain path URL", nilOr([u filePathURL], ^(id p) {
            return PF(@"non-nil ; same-file-as-original %@ ; isFileReferenceURL=%@", yesno(samePath([p path], file)),
                      yesno([p isFileReferenceURL]));
        }));
        probe_line("0352.fileReferenceURL.equal", @"two calls on the same file", yesno([[u fileReferenceURL] isEqual:ref]));

        NSURL *dirRef = [[NSURL fileURLWithPath:dir isDirectory:YES] fileReferenceURL];
        probe_line("0352.fileReferenceURL", @"existing directory", nilOr(dirRef, ^(id r) {
            return PF(@"non-nil ; isFileReferenceURL=%@", yesno([r isFileReferenceURL]));
        }));
        NSURL *linkRef = [[NSURL fileURLWithPath:link] fileReferenceURL];
        probe_line("0352.fileReferenceURL", @"symlink to existing file", nilOr(linkRef, ^(id r) {
            return PF(@"non-nil ; filePathURL-same-file-as-link %@ ; filePathURL-path-equals-link-path %@",
                      yesno(samePath([[r filePathURL] path], file)), yesno([[[r filePathURL] path] hasSuffix:@"link.txt"]));
        }));
        probe_line("0352.fileReferenceURL", @"missing file",
                   nilOr([[NSURL fileURLWithPath:missing] fileReferenceURL], ^(id r) { return @"non-nil"; }));
        probe_line("0352.filePathURL", @"missing file path URL",
                   nilOr([[NSURL fileURLWithPath:missing] filePathURL], ^(id p) { return @"non-nil"; }));
        NSURL *web = [NSURL URLWithString:@"http://example.invalid/a"];
        probe_line("0352.fileReferenceURL", @"http URL", nilOr([web fileReferenceURL], ^(id r) { return @"non-nil"; }));
        probe_line("0352.filePathURL", @"http URL", nilOr([web filePathURL], ^(id r) { return @"non-nil"; }));
        probe_line("0352.isFileReferenceURL", @"http URL", yesno([web isFileReferenceURL]));
        NSURL *dots = [NSURL fileURLWithPath:[dir stringByAppendingPathComponent:@"x/../a.txt"]];
        probe_line("0352.filePathURL", @"path with a '..' component (x does not exist)",
                   nilOr([dots filePathURL], ^(id p) {
            return PF(@"non-nil ; path-contains-dotdot %@", yesno([[p path] containsString:@".."]));
        }));

        // The identity of the file after a move.
        [fm moveItemAtPath:file toPath:moved error:nil];
        probe_line("0352.after-move", @"old reference URL: file still reachable", nilOr(ref, ^(id r) {
            return PF(@"filePathURL-same-file-as-moved %@ ; reference-path-exists %@",
                      yesno(samePath([[r filePathURL] path], moved)), yesno([fm fileExistsAtPath:[r path]]));
        }));
        probe_line("0352.after-move", @"plain URL of the old path: fileReferenceURL",
                   nilOr([u fileReferenceURL], ^(id r) { return @"non-nil"; }));
        probe_line("0352.after-move", @"plain URL of the old path: isFileReferenceURL", yesno([u isFileReferenceURL]));
        [fm removeItemAtPath:dir error:nil];
    }
    return 0;
}

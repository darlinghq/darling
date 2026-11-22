// Probe 147: NSFileWrapper readFromURL:options:error:, matchesContentsOfURL:, writeToURL:options:originalContentsURL:error:.
// Everything happens in one private directory made with mkdtemp under the temporary directory; it is removed
// at the end (only that directory, by the probe, nothing else). No paths are printed. No window server needed.
#import "probe_common.h"
#include <stdlib.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>

static NSFileManager *fm;
static NSString *gRoot;
static int gN;

static NSString *yesno(BOOL b) { return b ? @"YES" : @"NO"; }

static NSString *errs(NSError *e) {
    if (!e) return @"error=nil";
    NSError *u = [e userInfo][NSUnderlyingErrorKey];
    return PF(@"error=%@/%ld%@", [e domain], (long)[e code], u ? PF(@" underlying=%@/%ld", [u domain], (long)[u code]) : @"");
}

// Fresh sandbox directory for one case: gRoot/cN/. Returns it.
static NSString *box(void) {
    NSString *d = [gRoot stringByAppendingPathComponent:PF(@"c%d", ++gN)];
    mkdir([d fileSystemRepresentation], 0755);
    return d;
}
static NSString *sub(NSString *d, NSString *n) { return [d stringByAppendingPathComponent:n]; }
static NSURL *U(NSString *p) { return [NSURL fileURLWithPath:p]; }
static void put(NSString *p, NSString *s) { [[s dataUsingEncoding:NSUTF8StringEncoding] writeToFile:p atomically:NO]; }
static NSString *get(NSString *p) {
    NSData *d = [NSData dataWithContentsOfFile:p];
    return d ? [[NSString alloc] initWithData:d encoding:NSUTF8StringEncoding] : nil;
}
static NSString *kindAt(NSString *p) { // without following links
    struct stat st;
    if (lstat([p fileSystemRepresentation], &st) != 0) return @"absent";
    if (S_ISLNK(st.st_mode)) {
        char buf[1024]; ssize_t n = readlink([p fileSystemRepresentation], buf, sizeof buf - 1);
        return PF(@"link->%@", n > 0 ? [[NSString alloc] initWithBytes:buf length:n encoding:NSUTF8StringEncoding] : @"?");
    }
    return S_ISDIR(st.st_mode) ? @"dir" : S_ISREG(st.st_mode) ? @"file" : @"other";
}
static unsigned long long inode(NSString *p) { struct stat st; return stat([p fileSystemRepresentation], &st) ? 0 : st.st_ino; }
static int nlinkOf(NSString *p) { struct stat st; return stat([p fileSystemRepresentation], &st) ? -1 : (int)st.st_nlink; }
static void setMtime(NSString *p, NSDate *d) { [fm setAttributes:@{NSFileModificationDate: d} ofItemAtPath:p error:nil]; }
static NSString *kindOf(NSFileWrapper *w) {
    return w.isDirectory ? @"directory" : w.isRegularFile ? @"regularFile" : w.isSymbolicLink ? @"symlink" : @"unknown";
}
static NSString *listing(NSString *d) { // entries and kinds, sorted
    NSArray *a = [[fm contentsOfDirectoryAtPath:d error:nil] sortedArrayUsingSelector:@selector(compare:)];
    NSMutableArray *o = [NSMutableArray array];
    for (NSString *n in a) {
        NSString *k = kindAt(sub(d, n));
        [o addObject:[k isEqualToString:@"file"] ? PF(@"%@=%@", n, get(sub(d, n))) : PF(@"%@:%@", n, k)];
    }
    return [o componentsJoinedByString:@" "];
}
static NSString *tree(NSFileWrapper *w) {
    if (!w.isDirectory) return kindOf(w);
    NSMutableArray *o = [NSMutableArray array];
    for (NSString *k in [[w.fileWrappers allKeys] sortedArrayUsingSelector:@selector(compare:)])
        [o addObject:PF(@"%@:%@", k, kindOf(w.fileWrappers[k]))];
    return [o componentsJoinedByString:@" "];
}

static void rd(const char *id, NSString *desc, NSFileWrapper *w, NSString *p, NSFileWrapperReadingOptions o) {
    NSError *e = nil;
    BOOL ok = [w readFromURL:U(p) options:o error:&e];
    probe_line(id, desc, PF(@"returned %@ ; %@ ; wrapper now %@ (%@)", yesno(ok), errs(e), kindOf(w), tree(w)));
}

int main(void) {
    @autoreleasepool {
        probe_env("147");
        fm = [NSFileManager defaultManager];
        char tmpl[PATH_MAX];
        snprintf(tmpl, sizeof tmpl, "%s/probe147.XXXXXX", [NSTemporaryDirectory() fileSystemRepresentation]);
        if (!mkdtemp(tmpl)) { probe_line("147", @"mkdtemp", @"failed"); return 1; }
        gRoot = [fm stringWithFileSystemRepresentation:tmpl length:strlen(tmpl)];
        NSFileWrapperReadingOptions IMM = NSFileWrapperReadingImmediate;
        NSFileWrapperWritingOptions ATOM = NSFileWrapperWritingAtomic;

        // ---------------- R. readFromURL:options:error: ----------------
        {   // R1. node of another kind than the wrapper; the wrapper afterwards
            NSString *b = box();
            put(sub(b, @"f"), @"file-content");
            mkdir([sub(b, @"d") fileSystemRepresentation], 0755);
            put(sub(sub(b, @"d"), @"x"), @"X");
            symlink("f", [sub(b, @"l") fileSystemRepresentation]);
            NSFileWrapper *wf = [[NSFileWrapper alloc] initRegularFileWithContents:[@"orig" dataUsingEncoding:NSUTF8StringEncoding]];
            rd("147.R1.kind", @"regular-file wrapper reads a directory", wf, sub(b, @"d"), IMM);
            probe_line("147.R1.kind", @"  contents of that wrapper afterwards", PF(@"%@", [[NSString alloc] initWithData:wf.regularFileContents ?: [NSData data] encoding:NSUTF8StringEncoding]));
            wf = [[NSFileWrapper alloc] initRegularFileWithContents:[@"orig" dataUsingEncoding:NSUTF8StringEncoding]];
            rd("147.R1.kind", @"regular-file wrapper reads a symlink to a file", wf, sub(b, @"l"), IMM);
            NSFileWrapper *wd = [[NSFileWrapper alloc] initDirectoryWithFileWrappers:@{@"keep": [[NSFileWrapper alloc] initRegularFileWithContents:[@"k" dataUsingEncoding:NSUTF8StringEncoding]]}];
            rd("147.R1.kind", @"directory wrapper reads a regular file", wd, sub(b, @"f"), IMM);
            wd = [[NSFileWrapper alloc] initDirectoryWithFileWrappers:@{@"keep": [[NSFileWrapper alloc] initRegularFileWithContents:[@"k" dataUsingEncoding:NSUTF8StringEncoding]]}];
            rd("147.R1.kind", @"directory wrapper reads a symlink to a file", wd, sub(b, @"l"), IMM);
            NSFileWrapper *wl = [[NSFileWrapper alloc] initSymbolicLinkWithDestinationURL:U(@"/nowhere")];
            rd("147.R1.kind", @"symlink wrapper reads a regular file", wl, sub(b, @"f"), IMM);
            wl = [[NSFileWrapper alloc] initSymbolicLinkWithDestinationURL:U(@"/nowhere")];
            rd("147.R1.kind", @"symlink wrapper reads a symlink", wl, sub(b, @"l"), IMM);
            probe_line("147.R1.kind", @"  destination of that symlink wrapper afterwards", wl.symbolicLinkDestinationURL ? [wl.symbolicLinkDestinationURL lastPathComponent] : @"nil");
            wl = [[NSFileWrapper alloc] initSymbolicLinkWithDestinationURL:U(@"/nowhere")];
            rd("147.R1.kind", @"symlink wrapper reads a directory", wl, sub(b, @"d"), IMM);
            NSFileWrapper *wr = [[NSFileWrapper alloc] initRegularFileWithContents:[@"orig" dataUsingEncoding:NSUTF8StringEncoding]];
            rd("147.R1.missing", @"regular-file wrapper reads a missing path", wr, sub(b, @"missing"), IMM);
            wr = [[NSFileWrapper alloc] initRegularFileWithContents:[@"orig" dataUsingEncoding:NSUTF8StringEncoding]];
            rd("147.R1.same-kind", @"regular-file wrapper reads a regular file (contents follow?)", wr, sub(b, @"f"), IMM);
            probe_line("147.R1.same-kind", @"  contents afterwards", [[NSString alloc] initWithData:wr.regularFileContents ?: [NSData data] encoding:NSUTF8StringEncoding]);
        }
        {   // R2. a directory whose child changed kind, vanished, appeared, or changed content on disk
            NSString *b = box(), *d = sub(b, @"dir");
            mkdir([d fileSystemRepresentation], 0755);
            put(sub(d, @"a"), @"A1"); put(sub(d, @"b"), @"B1"); put(sub(d, @"c"), @"C1"); put(sub(d, @"e"), @"E1");
            put(sub(d, @"g"), @"G1");
            NSFileWrapper *w = [[NSFileWrapper alloc] initWithURL:U(d) options:IMM error:nil];
            NSDictionary *before = [w.fileWrappers copy];
            put(sub(d, @"b"), @"B2-changed");                          // same kind, new content
            [fm removeItemAtPath:sub(d, @"c") error:nil];              // vanished
            [fm removeItemAtPath:sub(d, @"a") error:nil];              // file -> directory
            mkdir([sub(d, @"a") fileSystemRepresentation], 0755); put(sub(sub(d, @"a"), @"inner"), @"I");
            [fm removeItemAtPath:sub(d, @"e") error:nil];              // file -> symlink
            symlink("g", [sub(d, @"e") fileSystemRepresentation]);
            put(sub(d, @"new"), @"N");                                 // appeared
            NSError *err = nil;
            BOOL ok = [w readFromURL:U(d) options:IMM error:&err];
            probe_line("147.R2.children", @"re-read after: b content changed, c removed, a file->dir, e file->symlink, new added",
                       PF(@"returned %@ ; %@ ; children: %@", yesno(ok), errs(err), tree(w)));
            for (NSString *k in @[@"a", @"b", @"c", @"e", @"g", @"new"]) {
                NSFileWrapper *o = before[k], *n = w.fileWrappers[k];
                probe_line("147.R2.identity", PF(@"child %@", k),
                           PF(@"same wrapper object as before: %@ ; before=%@ now=%@ ; contents now=%@", o && n ? yesno(o == n) : @"n/a",
                              o ? kindOf(o) : @"none", n ? kindOf(n) : @"none",
                              n.isRegularFile ? [[NSString alloc] initWithData:n.regularFileContents ?: [NSData data] encoding:NSUTF8StringEncoding] : @"-"));
            }
            // Child wrapper keeps its preferredFilename and filename after a re-read?
            NSFileWrapper *nb = w.fileWrappers[@"b"];
            probe_line("147.R2.names", @"child b names after re-read", PF(@"preferredFilename=%@ filename=%@", nb.preferredFilename, nb.filename));
            probe_line("147.R2.names", @"directory wrapper names after re-read", PF(@"preferredFilename=%@ filename=%@", w.preferredFilename, w.filename));
        }
        {   // R3. a later child cannot be read: earlier children, receiver's child list
            NSString *b = box(), *d = sub(b, @"dir");
            mkdir([d fileSystemRepresentation], 0755);
            put(sub(d, @"a"), @"A1"); put(sub(d, @"z"), @"Z1");
            NSFileWrapper *w = [[NSFileWrapper alloc] initWithURL:U(d) options:IMM error:nil];
            put(sub(d, @"a"), @"A2-changed");
            put(sub(d, @"extra"), @"X");
            chmod([sub(d, @"z") fileSystemRepresentation], 0); // unreadable with ReadingImmediate
            NSDictionary *beforeKids = [w.fileWrappers copy];
            NSError *e = nil;
            BOOL ok = [w readFromURL:U(d) options:IMM error:&e];
            NSFileWrapper *a = w.fileWrappers[@"a"];
            probe_line("147.R3.later-child-fails", @"child a changed (read first), child z unreadable (chmod 0; run as a normal user), extra added",
                       PF(@"returned %@ ; %@ ; children: %@ ; a contents: %@ ; a same object: %@ ; child list same set as before: %@",
                          yesno(ok), errs(e), tree(w), a.isRegularFile ? [[NSString alloc] initWithData:a.regularFileContents ?: [NSData data] encoding:NSUTF8StringEncoding] : @"-",
                          yesno(a == beforeKids[@"a"]), yesno([[NSSet setWithArray:w.fileWrappers.allKeys] isEqualToSet:[NSSet setWithArray:beforeKids.allKeys]])));
            chmod([sub(d, @"z") fileSystemRepresentation], 0644);

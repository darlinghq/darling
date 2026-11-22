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
        }

        // ---------------- M. matchesContentsOfURL: ----------------
        {
            NSString *b = box();
            NSString *f = sub(b, @"f");
            put(f, @"hello");
            NSFileWrapper *w = [[NSFileWrapper alloc] initWithURL:U(f) options:IMM error:nil];
            probe_line("147.M.file", @"wrapper read from the file, unchanged", yesno([w matchesContentsOfURL:U(f)]));
            NSDictionary *at = w.fileAttributes;
            probe_line("147.M.attributes", @"attributes recorded by a wrapper read from a file", PF(@"keys: %@", [[at.allKeys sortedArrayUsingSelector:@selector(compare:)] componentsJoinedByString:@","]));
            NSFileWrapper *fromData = [[NSFileWrapper alloc] initRegularFileWithContents:[@"hello" dataUsingEncoding:NSUTF8StringEncoding]];
            probe_line("147.M.data", @"wrapper made from data (same bytes), fileAttributes keys", [[fromData.fileAttributes.allKeys sortedArrayUsingSelector:@selector(compare:)] componentsJoinedByString:@","]);
            probe_line("147.M.data", @"wrapper made from data matches the file with the same bytes", yesno([fromData matchesContentsOfURL:U(f)]));
            NSDate *recorded = at[NSFileModificationDate];
            setMtime(f, [recorded dateByAddingTimeInterval:10]);
            probe_line("147.M.date", @"file modified later than recorded (same content)", yesno([w matchesContentsOfURL:U(f)]));
            setMtime(f, [recorded dateByAddingTimeInterval:-10]);
            probe_line("147.M.date", @"file older than recorded (same content)", yesno([w matchesContentsOfURL:U(f)]));
            setMtime(f, recorded);
            probe_line("147.M.date", @"file date set back to exactly the recorded date", yesno([w matchesContentsOfURL:U(f)]));
            setMtime(f, [recorded dateByAddingTimeInterval:0.5]);
            probe_line("147.M.date", @"file date 0.5 s later than recorded", yesno([w matchesContentsOfURL:U(f)]));
            setMtime(f, recorded);
            put(f, @"HELLO"); setMtime(f, recorded);
            probe_line("147.M.content", @"same size, different bytes, date restored to the recorded date", yesno([w matchesContentsOfURL:U(f)]));
            put(f, @"hello world!"); setMtime(f, recorded);
            probe_line("147.M.content", @"different size, date restored", yesno([w matchesContentsOfURL:U(f)]));
            put(f, @"hello"); setMtime(f, recorded);
            chmod([f fileSystemRepresentation], 0600);
            probe_line("147.M.perms", @"permissions changed (0600 vs recorded), date and bytes equal", PF(@"%@ ; recorded perms=%@", yesno([w matchesContentsOfURL:U(f)]), at[NSFilePosixPermissions]));
            chmod([f fileSystemRepresentation], [at[NSFilePosixPermissions] intValue]);
            probe_line("147.M.perms", @"permissions restored", yesno([w matchesContentsOfURL:U(f)]));
            probe_line("147.M.missing", @"missing path", yesno([w matchesContentsOfURL:U(sub(b, @"nope"))]));
            mkdir([sub(b, @"d") fileSystemRepresentation], 0755);
            probe_line("147.M.kind", @"file wrapper against a directory", yesno([w matchesContentsOfURL:U(sub(b, @"d"))]));
            probe_line("147.M.kind", @"nil URL", ({ NSString *r = probe_try(^{ [w matchesContentsOfURL:nil]; }); r; }));
            // symlinks
            symlink("tgt", [sub(b, @"l") fileSystemRepresentation]);
            NSFileWrapper *wl = [[NSFileWrapper alloc] initWithURL:U(sub(b, @"l")) options:IMM error:nil];
            probe_line("147.M.symlink", @"symlink wrapper read from link, unchanged", PF(@"%@ ; kind %@", yesno([wl matchesContentsOfURL:U(sub(b, @"l"))]), kindOf(wl)));
            [fm removeItemAtPath:sub(b, @"l") error:nil]; symlink("other", [sub(b, @"l") fileSystemRepresentation]);
            probe_line("147.M.symlink", @"link retargeted", yesno([wl matchesContentsOfURL:U(sub(b, @"l"))]));
            NSFileWrapper *wl2 = [[NSFileWrapper alloc] initSymbolicLinkWithDestinationURL:U(@"other")];
            probe_line("147.M.symlink", @"symlink wrapper made from a destination, same destination as the link", yesno([wl2 matchesContentsOfURL:U(sub(b, @"l"))]));
            // directories
            NSString *d = sub(b, @"tree");
            mkdir([d fileSystemRepresentation], 0755); put(sub(d, @"a"), @"A"); put(sub(d, @"b"), @"B");
            NSFileWrapper *wd = [[NSFileWrapper alloc] initWithURL:U(d) options:IMM error:nil];
            probe_line("147.M.dir", @"directory wrapper, unchanged", yesno([wd matchesContentsOfURL:U(d)]));
            put(sub(d, @"c"), @"C");
            probe_line("147.M.dir", @"entry added on disk", yesno([wd matchesContentsOfURL:U(d)]));
            [fm removeItemAtPath:sub(d, @"c") error:nil];
            probe_line("147.M.dir", @"entry removed again", yesno([wd matchesContentsOfURL:U(d)]));
            NSDate *rb = ((NSFileWrapper *)wd.fileWrappers[@"b"]).fileAttributes[NSFileModificationDate];
            put(sub(d, @"b"), @"B-new");
            probe_line("147.M.dir", @"child b content and date changed", yesno([wd matchesContentsOfURL:U(d)]));
            put(sub(d, @"b"), @"B"); setMtime(sub(d, @"b"), rb);
            probe_line("147.M.dir", @"child b restored (bytes and date)", yesno([wd matchesContentsOfURL:U(d)]));
            NSDate *dirRec = wd.fileAttributes[NSFileModificationDate];
            NSDate *dirNow = [fm attributesOfItemAtPath:d error:nil][NSFileModificationDate];
            probe_line("147.M.dir", @"directory's own date now equals the recorded one", yesno([dirRec isEqual:dirNow]));
            setMtime(d, [dirRec dateByAddingTimeInterval:5]);
            probe_line("147.M.dir", @"directory's own date 5 s later, children unchanged", yesno([wd matchesContentsOfURL:U(d)]));
            NSFileWrapper *wdata = [[NSFileWrapper alloc] initDirectoryWithFileWrappers:@{@"a": [[NSFileWrapper alloc] initRegularFileWithContents:[@"A" dataUsingEncoding:NSUTF8StringEncoding]],
                                                                                          @"b": [[NSFileWrapper alloc] initRegularFileWithContents:[@"B" dataUsingEncoding:NSUTF8StringEncoding]]}];
            probe_line("147.M.dir", @"directory wrapper built from data with the same names and bytes", yesno([wdata matchesContentsOfURL:U(d)]));
            NSFileWrapper *wempty = [[NSFileWrapper alloc] initDirectoryWithFileWrappers:@{}];
            NSString *emptyDir = sub(b, @"empty"); mkdir([emptyDir fileSystemRepresentation], 0755);
            probe_line("147.M.dir", @"empty directory wrapper from data against an empty directory", yesno([wempty matchesContentsOfURL:U(emptyDir)]));
        }

        // ---------------- W. writeToURL:options:originalContentsURL:error: ----------------
        NSData *(^D)(NSString *) = ^NSData *(NSString *s) { return [s dataUsingEncoding:NSUTF8StringEncoding]; };
        NSFileWrapper *(^F)(NSString *) = ^NSFileWrapper *(NSString *s) { return [[NSFileWrapper alloc] initRegularFileWithContents:D(s)]; };
        NSFileWrapper *(^DIR)(NSDictionary *) = ^NSFileWrapper *(NSDictionary *k) { return [[NSFileWrapper alloc] initDirectoryWithFileWrappers:k]; };
        NSString *(^wr)(NSFileWrapper *, NSString *, NSFileWrapperWritingOptions, NSString *) =
            ^NSString *(NSFileWrapper *w, NSString *dest, NSFileWrapperWritingOptions o, NSString *orig) {
                NSError *e = nil;
                BOOL ok = [w writeToURL:U(dest) options:o originalContentsURL:orig ? U(orig) : nil error:&e];
                return PF(@"returned %@ ; %@", yesno(ok), errs(e));
            };

        {   // W1. basic and error codes
            NSString *b = box();
            probe_line("147.W1.basic", @"directory wrapper to a new path, options 0", PF(@"%@ ; result: %@", wr(DIR(@{@"a": F(@"A")}), sub(b, @"n1"), 0, nil), listing(sub(b, @"n1"))));
            probe_line("147.W1.basic", @"directory wrapper to a new path, atomic", PF(@"%@ ; result: %@", wr(DIR(@{@"a": F(@"A")}), sub(b, @"n2"), ATOM, nil), listing(sub(b, @"n2"))));
            probe_line("147.W1.basic", @"regular-file wrapper to a new path", PF(@"%@ ; result: %@", wr(F(@"R"), sub(b, @"n3"), 0, nil), kindAt(sub(b, @"n3"))));
            probe_line("147.W1.basic", @"symlink wrapper to a new path", PF(@"%@ ; result: %@", wr([[NSFileWrapper alloc] initSymbolicLinkWithDestinationURL:U(@"rel/target")], sub(b, @"n4"), 0, nil), kindAt(sub(b, @"n4"))));
            probe_line("147.W1.parent-missing", @"destination under a parent that does not exist", wr(F(@"R"), sub(sub(b, @"nodir"), @"f"), 0, nil));
            put(sub(b, @"afile"), @"x");
            probe_line("147.W1.parent-is-file", @"destination under a regular file", wr(F(@"R"), sub(sub(b, @"afile"), @"f"), 0, nil));
            mkdir([sub(b, @"ro") fileSystemRepresentation], 0555);
            probe_line("147.W1.read-only-parent", @"destination in a read-only directory (run as a normal user)", wr(F(@"R"), sub(sub(b, @"ro"), @"f"), 0, nil));
            chmod([sub(b, @"ro") fileSystemRepresentation], 0755);
            probe_line("147.W1.null-error", @"NULL error pointer, destination under a missing parent", ({
                BOOL ok = [F(@"R") writeToURL:U(sub(sub(b, @"nodir2"), @"f")) options:0 originalContentsURL:nil error:NULL];
                PF(@"returned %@ (no crash)", yesno(ok)); }));
            probe_line("147.W1.nil-url", @"nil destination URL", ({ NSString *r = probe_try(^{ NSError *e = nil; [F(@"R") writeToURL:nil options:0 originalContentsURL:nil error:&e]; }); r; }));
        }
        {   // W2. existing items at the destination
            NSString *b = box();
            put(sub(b, @"file"), @"OLD");
            probe_line("147.W2.dest-file", @"directory wrapper onto an existing regular file, options 0", PF(@"%@ ; result: %@", wr(DIR(@{@"a": F(@"A")}), sub(b, @"file"), 0, nil), kindAt(sub(b, @"file"))));
            put(sub(b, @"file2"), @"OLD");
            probe_line("147.W2.dest-file", @"directory wrapper onto an existing regular file, atomic", PF(@"%@ ; result: %@", wr(DIR(@{@"a": F(@"A")}), sub(b, @"file2"), ATOM, nil), kindAt(sub(b, @"file2"))));
            put(sub(b, @"file3"), @"OLD");
            probe_line("147.W2.dest-file", @"regular wrapper onto an existing regular file, options 0", PF(@"%@ ; result: %@", wr(F(@"NEW"), sub(b, @"file3"), 0, nil), get(sub(b, @"file3"))));
            mkdir([sub(b, @"dirX") fileSystemRepresentation], 0755); put(sub(sub(b, @"dirX"), @"other"), @"keep");
            probe_line("147.W2.dest-dir", @"regular wrapper onto an existing directory, options 0", PF(@"%@ ; result: %@", wr(F(@"NEW"), sub(b, @"dirX"), 0, nil), kindAt(sub(b, @"dirX"))));
            mkdir([sub(b, @"dir") fileSystemRepresentation], 0755); put(sub(sub(b, @"dir"), @"other"), @"keep"); put(sub(sub(b, @"dir"), @"a"), @"OLD-A");
            probe_line("147.W2.dest-dir", @"directory wrapper {a, sub/} onto an existing directory with other and a, options 0",
                       PF(@"%@ ; result: %@", wr(DIR(@{@"a": F(@"NEW-A"), @"sub": DIR(@{@"s": F(@"S")})}), sub(b, @"dir"), 0, nil), listing(sub(b, @"dir"))));
            mkdir([sub(b, @"dirA") fileSystemRepresentation], 0755); put(sub(sub(b, @"dirA"), @"other"), @"keep");
            probe_line("147.W2.dest-dir", @"same, atomic", PF(@"%@ ; result: %@", wr(DIR(@{@"a": F(@"NEW-A")}), sub(b, @"dirA"), ATOM, nil), listing(sub(b, @"dirA"))));
            mkdir([sub(b, @"real") fileSystemRepresentation], 0755); put(sub(sub(b, @"real"), @"keep"), @"K");
            symlink("real", [sub(b, @"dirlink") fileSystemRepresentation]);
            probe_line("147.W2.dest-dirlink", @"directory wrapper onto a symlink to a directory, options 0",
                       PF(@"%@ ; link now: %@ ; real dir now: %@", wr(DIR(@{@"a": F(@"A")}), sub(b, @"dirlink"), 0, nil), kindAt(sub(b, @"dirlink")), listing(sub(b, @"real"))));
            mkdir([sub(b, @"real2") fileSystemRepresentation], 0755);
            symlink("real2", [sub(b, @"dirlink2") fileSystemRepresentation]);
            probe_line("147.W2.dest-dirlink", @"directory wrapper onto a symlink to a directory, atomic",
                       PF(@"%@ ; link now: %@ ; real dir now: %@", wr(DIR(@{@"a": F(@"A")}), sub(b, @"dirlink2"), ATOM, nil), kindAt(sub(b, @"dirlink2")), listing(sub(b, @"real2"))));
            put(sub(b, @"target"), @"TARGET");
            symlink("target", [sub(b, @"filelink") fileSystemRepresentation]);
            probe_line("147.W2.dest-filelink", @"regular wrapper onto a symlink to a file, options 0",
                       PF(@"%@ ; link now: %@ ; target now: %@", wr(F(@"NEW"), sub(b, @"filelink"), 0, nil), kindAt(sub(b, @"filelink")), get(sub(b, @"target"))));
            put(sub(b, @"target2"), @"TARGET");
            symlink("target2", [sub(b, @"filelink2") fileSystemRepresentation]);
            probe_line("147.W2.dest-filelink", @"regular wrapper onto a symlink to a file, atomic",
                       PF(@"%@ ; link now: %@ ; target now: %@", wr(F(@"NEW"), sub(b, @"filelink2"), ATOM, nil), kindAt(sub(b, @"filelink2")), get(sub(b, @"target2"))));
        }
        {   // W3. link children over existing entries
            NSString *b = box();
            NSFileWrapper *(^L)(NSString *) = ^NSFileWrapper *(NSString *dst) { return [[NSFileWrapper alloc] initSymbolicLinkWithDestinationURL:[NSURL URLWithString:dst]]; };
            NSString *kinds[] = {@"file", @"link", @"dir", @"none"};
            for (int o = 0; o < 2; o++)
                for (int k = 0; k < 4; k++) {
                    NSString *d = sub(b, PF(@"w%d%d", o, k));
                    mkdir([d fileSystemRepresentation], 0755);
                    NSString *l = sub(d, @"L");
                    if (k == 0) put(l, @"OLD");
                    if (k == 1) symlink("old-dest", [l fileSystemRepresentation]);
                    if (k == 2) { mkdir([l fileSystemRepresentation], 0755); put(sub(l, @"inside"), @"I"); }
                    probe_line("147.W3.link-child", PF(@"directory {L: symlink->new-dest} into existing dir where L is %@, %@", kinds[k], o ? @"atomic" : @"options 0"),
                               PF(@"%@ ; L now: %@", wr(DIR(@{@"L": L(@"new-dest")}), d, o ? ATOM : 0, nil), kindAt(l)));
                }
            // And a regular child or directory child over a link in the destination.
            NSString *d = sub(b, @"rl"); mkdir([d fileSystemRepresentation], 0755);
            put(sub(b, @"outside"), @"OUTSIDE"); symlink("../outside", [sub(d, @"f") fileSystemRepresentation]);
            probe_line("147.W3.regular-over-link", @"directory {f: regular NEW} into dir where f is a symlink to a file outside, options 0",
                       PF(@"%@ ; f now: %@ ; outside now: %@", wr(DIR(@{@"f": F(@"NEW")}), d, 0, nil), kindAt(sub(d, @"f")), get(sub(b, @"outside"))));
            d = sub(b, @"rl2"); mkdir([d fileSystemRepresentation], 0755);
            put(sub(b, @"outside2"), @"OUTSIDE"); symlink("../outside2", [sub(d, @"f") fileSystemRepresentation]);
            probe_line("147.W3.regular-over-link", @"same, atomic", PF(@"%@ ; f now: %@ ; outside now: %@", wr(DIR(@{@"f": F(@"NEW")}), d, ATOM, nil), kindAt(sub(d, @"f")), get(sub(b, @"outside2"))));
            d = sub(b, @"rl3"); mkdir([d fileSystemRepresentation], 0755);
            mkdir([sub(b, @"outdir") fileSystemRepresentation], 0755); put(sub(sub(b, @"outdir"), @"keep"), @"K");
            symlink("../outdir", [sub(d, @"sub") fileSystemRepresentation]);
            probe_line("147.W3.dir-over-link", @"directory {sub: directory {x}} into dir where sub is a symlink to a directory outside, options 0",
                       PF(@"%@ ; sub now: %@ ; outside dir now: %@", wr(DIR(@{@"sub": DIR(@{@"x": F(@"X")})}), d, 0, nil), kindAt(sub(d, @"sub")), listing(sub(b, @"outdir"))));
        }
        {   // W4. hard links with originalContentsURL
            NSString *b = box();
            NSString *o = sub(b, @"orig");
            mkdir([o fileSystemRepresentation], 0755); put(sub(o, @"a"), @"A"); put(sub(o, @"b"), @"B");
            mkdir([sub(o, @"s") fileSystemRepresentation], 0755); put(sub(sub(o, @"s"), @"c"), @"C");
            NSFileWrapper *w = [[NSFileWrapper alloc] initWithURL:U(o) options:IMM error:nil];
            NSString *d1 = sub(b, @"copy1");
            probe_line("147.W4.hardlink", @"unchanged wrapper from orig, written to a new path with originalContentsURL=orig",
                       PF(@"%@ ; same inode as original: a=%@ b=%@ s/c=%@ ; nlink a=%d", wr(w, d1, 0, o), yesno(inode(sub(d1, @"a")) == inode(sub(o, @"a"))), yesno(inode(sub(d1, @"b")) == inode(sub(o, @"b"))),
                          yesno(inode(sub(sub(d1, @"s"), @"c")) == inode(sub(sub(o, @"s"), @"c"))), nlinkOf(sub(o, @"a"))));
            NSString *d2 = sub(b, @"copy2");
            probe_line("147.W4.hardlink", @"same, originalContentsURL=nil",
                       PF(@"%@ ; same inode as original: a=%@ ; nlink a=%d", wr(w, d2, 0, nil), yesno(inode(sub(d2, @"a")) == inode(sub(o, @"a"))), nlinkOf(sub(o, @"a"))));
            NSString *d3 = sub(b, @"copy3");
            probe_line("147.W4.hardlink", @"same, atomic, originalContentsURL=orig",
                       PF(@"%@ ; same inode as original: a=%@", wr(w, d3, ATOM, o), yesno(inode(sub(d3, @"a")) == inode(sub(o, @"a")))));
            // child modified in the wrapper: new contents must be written, original untouched
            NSFileWrapper *w2 = [[NSFileWrapper alloc] initWithURL:U(o) options:IMM error:nil];
            [w2 removeFileWrapper:w2.fileWrappers[@"a"]];
            [w2 addRegularFileWithContents:D(@"A-NEW") preferredFilename:@"a"];
            NSString *d4 = sub(b, @"copy4");
            probe_line("147.W4.hardlink", @"child a replaced in the wrapper (new data), written with originalContentsURL=orig",
                       PF(@"%@ ; a=%@ ; a same inode as original: %@ ; b same inode: %@ ; original a still %@", wr(w2, d4, 0, o), get(sub(d4, @"a")),
                          yesno(inode(sub(d4, @"a")) == inode(sub(o, @"a"))), yesno(inode(sub(d4, @"b")) == inode(sub(o, @"b"))), get(sub(o, @"a"))));
            // original changed on disk after reading
            put(sub(o, @"b"), @"B-changed-on-disk");
            NSString *d5 = sub(b, @"copy5");
            probe_line("147.W4.hardlink", @"original file b changed on disk after the wrapper read it, originalContentsURL=orig",
                       PF(@"%@ ; copy b=%@ ; copy b same inode as original: %@", wr(w, d5, 0, o), get(sub(d5, @"b")), yesno(inode(sub(d5, @"b")) == inode(sub(o, @"b")))));
            put(sub(o, @"b"), @"B");
            // original in place: destination is the original itself
            NSFileWrapper *w3 = [[NSFileWrapper alloc] initWithURL:U(o) options:IMM error:nil];
            unsigned long long ia = inode(sub(o, @"a"));
            NSDate *ma = [fm attributesOfItemAtPath:sub(o, @"a") error:nil][NSFileModificationDate];
            probe_line("147.W4.in-place", @"wrapper from orig written back onto orig with originalContentsURL=orig",
                       PF(@"%@ ; a inode unchanged: %@ ; a date unchanged: %@ ; listing: %@", wr(w3, o, 0, o), yesno(ia == inode(sub(o, @"a"))),
                          yesno([ma isEqual:[fm attributesOfItemAtPath:sub(o, @"a") error:nil][NSFileModificationDate]]), listing(o)));
            probe_line("147.W4.in-place", @"same with atomic", PF(@"%@ ; a inode unchanged: %@ ; listing: %@", wr(w3, o, ATOM, o), yesno(ia == inode(sub(o, @"a"))), listing(o)));
            // existing hard link at the destination, written by a changed child (not applicable by name)
            NSString *dd = sub(b, @"hl"); mkdir([dd fileSystemRepresentation], 0755);
            link([sub(o, @"a") fileSystemRepresentation], [sub(dd, @"a") fileSystemRepresentation]);
            NSFileWrapper *w4 = DIR(@{@"a": F(@"A-FROM-WRAPPER")});
            probe_line("147.W4.existing-hardlink", @"destination dir has a/ as a hard link to orig/a; directory {a: new data} written to it, options 0",
                       PF(@"%@ ; dest a=%@ ; orig a=%@ ; same inode afterwards: %@ ; nlink of orig a=%d", wr(w4, dd, 0, nil), get(sub(dd, @"a")), get(sub(o, @"a")),
                          yesno(inode(sub(dd, @"a")) == inode(sub(o, @"a"))), nlinkOf(sub(o, @"a"))));
            put(sub(o, @"a"), @"A");
            NSString *dd2 = sub(b, @"hl2"); mkdir([dd2 fileSystemRepresentation], 0755);
            link([sub(o, @"a") fileSystemRepresentation], [sub(dd2, @"a") fileSystemRepresentation]);
            probe_line("147.W4.existing-hardlink", @"same, atomic",
                       PF(@"%@ ; dest a=%@ ; orig a=%@ ; same inode afterwards: %@", wr(DIR(@{@"a": F(@"A-FROM-WRAPPER")}), dd2, ATOM, nil), get(sub(dd2, @"a")), get(sub(o, @"a")),
                          yesno(inode(sub(dd2, @"a")) == inode(sub(o, @"a")))));
        }

        // W5. child names that are not one path component. Destination is two levels down so ".." stays in the sandbox.
        {
            NSString *b = box(), *area = sub(b, @"area");
            mkdir([area fileSystemRepresentation], 0755);
            NSArray *names = @[@"", @".", @"..", @"a/b", @"/abs", @"a\\b", @"normal", @".hidden", @"with space"];
            for (NSString *n in names) {
                NSString *dest = sub(area, PF(@"dest%lu", (unsigned long)[names indexOfObject:n]));
                NSFileWrapper *w = DIR(@{n: F(@"PAYLOAD")});
                NSString *res = wr(w, dest, 0, nil);
                probe_line("147.W5.child-name", PF(@"directory wrapper with child key \"%@\"", n),
                           PF(@"%@ ; dest: %@ ; area: %@ ; sandbox: %@", res, listing(dest), listing(area), listing(b)));
            }
            probe_line("147.W5.preferred-name", @"addRegularFileWithContents:preferredFilename:@\"a/b\" then write", ({
                NSFileWrapper *w = DIR(@{});
                NSString *r1 = probe_try(^{ [w addRegularFileWithContents:D(@"P") preferredFilename:@"a/b"]; });
                NSString *dest = sub(area, @"pf");
                PF(@"%@ ; keys %@ ; write %@ ; dest %@", r1, [w.fileWrappers.allKeys componentsJoinedByString:@","], wr(w, dest, 0, nil), listing(dest)); }));
        }

        // W6. attributes and name updating
        {
            NSString *b = box(), *s = sub(b, @"src"), *d = sub(b, @"dst");
            put(s, @"content"); chmod([s fileSystemRepresentation], 0640);
            NSDate *old = [NSDate dateWithTimeIntervalSince1970:1000000000];
            setMtime(s, old);
            NSFileWrapper *w = [[NSFileWrapper alloc] initWithURL:U(s) options:IMM error:nil];
            NSMutableDictionary *at = [w.fileAttributes mutableCopy];
            at[NSFilePosixPermissions] = @0600;
            at[NSFileModificationDate] = [NSDate dateWithTimeIntervalSince1970:1100000000];
            w.fileAttributes = at;
            NSString *res = wr(w, d, 0, nil);
            NSDictionary *got = [fm attributesOfItemAtPath:d error:nil];
            probe_line("147.W6.attributes", @"regular wrapper with fileAttributes perms 0600 and a 2004 date written (source file was 0640, 2001 date)",
                       PF(@"%@ ; written perms=%o ; written date year=%d", res, [got[NSFilePosixPermissions] intValue],
                          (int)(1970 + [got[NSFileModificationDate] timeIntervalSince1970] / 31556952)));
            NSFileWrapper *dw = DIR(@{@"k": F(@"K")});
            dw.fileAttributes = @{NSFilePosixPermissions: @0700};
            NSString *dd = sub(b, @"dstdir");
            res = wr(dw, dd, 0, nil);
            probe_line("147.W6.attributes", @"directory wrapper with fileAttributes perms 0700 written", PF(@"%@ ; dir perms=%o", res, [[fm attributesOfItemAtPath:dd error:nil][NSFilePosixPermissions] intValue]));
            NSFileWrapper *nw = F(@"n");
            nw.preferredFilename = @"pref.txt";
            NSString *nd = sub(b, @"other-name.txt");
            res = wr(nw, nd, 0, nil);
            probe_line("147.W6.name-updating", @"written without NameUpdating", PF(@"%@ ; filename afterwards=%@", res, nw.filename ?: @"nil"));
            NSFileWrapper *nw2 = F(@"n");
            nw2.preferredFilename = @"pref.txt";
            res = wr(nw2, sub(b, @"other-name2.txt"), NSFileWrapperWritingWithNameUpdating, nil);
            probe_line("147.W6.name-updating", @"written with NameUpdating", PF(@"%@ ; filename afterwards=%@ ; preferredFilename=%@", res, nw2.filename ?: @"nil", nw2.preferredFilename));
            // children's filenames when a directory is written with NameUpdating and with a changed child name
            NSFileWrapper *dir2 = DIR(@{@"k": F(@"K")});
            wr(dir2, sub(b, @"nu-dir"), NSFileWrapperWritingWithNameUpdating, nil);
            probe_line("147.W6.name-updating", @"directory with NameUpdating, child k", PF(@"dir filename=%@ ; child filename=%@", dir2.filename ?: @"nil", ((NSFileWrapper *)dir2.fileWrappers[@"k"]).filename ?: @"nil"));
        }

        [fm removeItemAtPath:gRoot error:nil]; // the probe's own mkdtemp directory
    }
    return 0;
}

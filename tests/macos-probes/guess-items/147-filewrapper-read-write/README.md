# Probe 147: NSFileWrapper readFromURL, matchesContentsOfURL and directory and link writes

PR: darlinghq/darling-foundation#147 (NSFileWrapper: add readFromURL, matchesContentsOfURL and directory and link writes).

**macOS result: NOT YET OBSERVED**

Public API only, written from the public documentation and from the wording of the PR's GUESS items. No private selectors, no private symbols, no Apple binary, header dump or trace was used. The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. It was derived from the GUESS text, not from the PR's code, so it can also show macOS doing something the PR did not expect. Nothing here claims a Mac result; the Darling guest ran nothing.

Window server: no. The probe makes one private directory with `mkdtemp` under the temporary directory, builds every case inside it, and removes it at the end (only that directory). It sets directory permissions to 0 and 0555 in two cases; run as a normal user (root ignores them, and those rows then say so by succeeding). Child-name cases (`147.W5`) write only inside the probe's directory. No path is printed.

Result lines print `returned YES/NO ; error=<domain>/<code> [underlying=<domain>/<code>]` plus the state of the wrapper or of the directory after the call. Read error codes against `NSFileReadUnknownError` 256, `NSFileReadNoSuchFileError` 260, `NSFileWriteUnknownError` 512, `NSFileWriteInvalidFileNameError` 514, `NSFileWriteFileExistsError` 516 (Foundation domain).

## The GUESS items and what the probe prints for each

1. "Read of a node of another kind than the wrapper: fails with `NSFileReadUnknownError` and leaves the wrapper as it was" -> `147.R1.kind` (regular wrapper reading a directory and a symlink; directory wrapper reading a file and a symlink; symlink wrapper reading a file, a symlink, a directory; each with the wrapper's kind and children afterwards), `147.R1.missing`, `147.R1.same-kind`.
   - Return NO with the same domain/code and the wrapper untouched: keep. A different code, YES with the wrapper converted to the new kind, or a changed wrapper after failure: change.
2. "Read of a directory whose child changed kind on disk (or cannot be reread): the child is replaced by a new wrapper" -> `147.R2.children` and `147.R2.identity` (child content changed, removed, file to directory, file to symlink, new entry; `same wrapper object` shows replace versus update in place), `147.R2.names` (preferredFilename and filename afterwards).
3. "Read: if a later child fails, the children already reread stay reread, while the receiver's own child list is unchanged" -> `147.R3.later-child-fails` (child a changed, child z unreadable, extra added; read order by name).
4. "Match: a wrapper that records no modification date or permissions (made from data) matches nothing; dates must be equal (a file older than the recorded date does not match)" -> `147.M.attributes` (keys a wrapper read from a file records), `147.M.data` (keys of a data wrapper and whether it matches a file with the same bytes), `147.M.date` (later by 10 s, earlier by 10 s, exactly equal, 0.5 s later), `147.M.content` (same size different bytes with the date restored, different size), `147.M.perms`, `147.M.missing`, `147.M.kind`, `147.M.symlink`, `147.M.dir` (unchanged, entry added, entry removed, child changed, child restored, directory's own date later, data-built directory wrapper, empty directory).
   - Equal date and bytes matching YES, later or earlier NO: keep. Older matching YES, or same date with different bytes matching NO or YES: tells whether content is read at all. Data wrappers matching: change if YES.
5. "Write, hard links: 'contents have not changed' is taken to mean matches in the sense of -matchesContentsOfURL:, and a file wrapper whose original is the destination itself needs no write" -> `147.W4.hardlink` (unchanged wrapper written with originalContentsURL: same inode per child and nlink; without it; atomic; a replaced child; an original changed on disk after reading), `147.W4.in-place` (wrapper written back over its original: inode and date unchanged, listing).
   - Same inode on macOS where the PR copies, or the reverse: change. In-place rewrite changing the inode or date: the PR's "needs no write" is a deviation.
6. "Each child of a directory is offered the earlier revision at the child's filename" -> visible through `147.W4.hardlink` (nested `s/c` inode row).
7. "Write, an existing directory at the destination is written into (without atomic) and keeps its other entries. A link to a directory is not entered, creating the directory fails and so does the write" -> `147.W2.dest-dir` (options 0 and atomic: other entries kept?), `147.W2.dest-dirlink` (directory wrapper onto a symlink to a directory, options 0 and atomic: link now, real directory now), `147.W2.dest-file`, `147.W1.basic`.
8. "Regular children are always written atomically (replace, not rewrite in place), so an entry that is a hard link to an earlier revision, or a symlink to a file elsewhere, is replaced and what it linked to is not modified" -> `147.W4.existing-hardlink` (destination child is a hard link to the original: contents, same inode afterwards, nlink; options 0 and atomic), `147.W3.regular-over-link` (symlink to a file outside the destination, options 0 and atomic: outside file content), `147.W2.dest-filelink`, `147.W3.dir-over-link`.
   - Outside file or original modified (non-atomic): macOS rewrites in place, the PR's "always atomic" is a deviation. Untouched: keep.
9. "A child name that is not one path component (empty, '.', '..', or containing '/') is refused with `NSFileWriteInvalidFileNameError`" -> `147.W5.child-name` (nine keys including empty, dot, dot-dot, slash, leading slash, backslash, hidden, space: result, destination listing, parent listing, sandbox listing, so a file written outside the destination shows up) and `147.W5.preferred-name`.
10. "A link child is not written over an existing item without atomic: it fails with the error of creating the link (EEXIST), while a regular file replaces what is there and a directory is written into" -> `147.W3.link-child` (a symlink child onto an existing file, symlink, directory and nothing, options 0 and atomic).
11. "Write: `fileAttributes` are not applied to what is written" -> `147.W6.attributes` (permissions 0600 and a 2004 date on a wrapper read from a 0640, 2001-dated file; directory 0700).
12. "A failure that gave no error is reported as `NSFileWriteUnknownError`" -> `147.W1.parent-missing`, `147.W1.parent-is-file`, `147.W1.read-only-parent` (the codes and underlying errno macOS gives), `147.W1.null-error` (NULL error pointer does not crash), `147.W1.nil-url`.
13. Name updating (nearby, not a GUESS): `147.W6.name-updating`.

## Apple documentation

- https://developer.apple.com/documentation/foundation/filewrapper/read(from:options:)
- https://developer.apple.com/documentation/foundation/filewrapper/matchescontents(of:)
- https://developer.apple.com/documentation/foundation/filewrapper/write(to:options:originalcontentsurl:)
- https://developer.apple.com/documentation/foundation/filewrapper

## How to run

Copy this directory to a Mac with the stock Xcode command line tools, then:

```sh
sh build.txt
./probe > results.txt
```

Run as a normal user. Paste the `PROBE` lines back. Nothing in the output contains user names, host names or paths. Do not commit the probe to Darling: it is an observation tool, not product code (Darling AGENTS.md section 6).

Syntax check done on Linux only: `clang -fsyntax-only -target x86_64-apple-macos11 -isysroot <Darling SDK>`. It parses with no stubs.

## What this probe cannot answer

Behaviour under concurrent changes to the destination (the PR's check-versus-use race note); file systems without hard links (the temporary directory is on the Mac's default file system; to test another, set `TMPDIR` before running).

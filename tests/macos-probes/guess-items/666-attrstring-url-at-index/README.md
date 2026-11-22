# Probe 666: -[NSAttributedString URLAtIndex:effectiveRange:]

PR: darlinghq/darling-cocotron#666 (AppKit: NSAttributedString URLAtIndex:effectiveRange:). The method is deprecated; it is called with deprecation warnings suppressed and declared in a category in the probe in case the SDK omits it.

**macOS result: NOT YET OBSERVED**

Public API only, written from the public documentation and from the wording of the PR's GUESS items. No private selectors, no private symbols, no Apple binary, header dump or trace was used. The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. It was derived from the GUESS text, not from the PR's code, so it can also show macOS doing something the PR did not expect. Nothing here claims a Mac result; the Darling guest ran nothing.

Window server: no (AppKit attributed string categories only; no window, no app object).

## The GUESS items and what the probe prints for each

1. "An index at or past the end of the string (so any index of an empty string) raises NSRangeException, as the attribute accessors do" -> `666.A.middle-link` (index 15 = length), `666.B.no-link`, `666.C.empty` (empty string index 0 and 1), `666.C.far` (1000, NSNotFound, -1), `666.F.whole` (index = length). Each line shows `RAISED <name>` or the returned values, and whether the range was written (`range-untouched` = sentinel 777,777 left alone).
   - `RAISED NSRangeException` at index = length: keep. nil with a range, or nil with the range untouched, at the end: change (nil, no exception). Different for the empty string: record separately.
2. "That the effective range is the longest run of equal link values (the documentation says only the range covered by the link or by non-URL text)" -> `666.D.adjacent-different` (two equal neighbours), `666.D.same-url` (same object), `666.D.equal-urls` (two equal NSURL objects), `666.D.url-vs-string` (an NSURL then an NSString with the same address), `666.D.same-link-other-attrs` (one link value, other attributes changing run to run), `666.D.gap` (equal links with plain text between), `666.A.middle-link` for the plain-text ranges before and after, `666.B.attr-runs` (no link, other attributes change), `666.G.compare` (the generic longest-effective-range answer for the same string, to compare).
   - Range spans all equal-valued neighbours and the whole plain stretch: keep (longest). Range stops at attribute-run boundaries (e.g. `same-link-other-attrs` gives one run, or `equal-urls` gives two ranges): the effective range is the plain attribute-run range, not the longest run of equal values: change.
3. "An NSString link that is not a valid URL gives nil and the range of that link run" -> `666.E.string-link` (valid, spaces, empty, relative, absolute path, mailto, non-ASCII, percent-escaped, malformed), `666.E.invalid-runs` (bad string next to a valid URL and to a different bad string), `666.E.other-type` (NSNumber, NSArray, NSDictionary, NSData as the link value).
   - nil with range = the whole run of that link: keep. nil with range = the surrounding non-URL text, or a different URL conversion (for example a string with spaces becoming a URL, a path string becoming a file URL): change.
4. Plain cases not marked GUESS but needed to read the rest: index inside a link, at each boundary, before and after (`666.A.middle-link` indices 0-15), NULL effective range (`666.A.null-range`), no link anywhere (`666.B.no-link`), link at the start and at the end (`666.F.*`), a mutable string (`666.G.mutable`).

## Apple documentation

- https://developer.apple.com/documentation/foundation/nsattributedstring (AppKit category `url(at:effectiveRange:)`, deprecated; search the page for "URLAtIndex" if the member URL moved)
- https://developer.apple.com/documentation/foundation/nsattributedstring/key/link

## How to run

Copy this directory to a Mac with the stock Xcode command line tools, then:

```sh
sh build.txt
./probe > results.txt
```

Paste the `PROBE` lines back. Nothing in the output contains user names, host names or paths. Do not commit the probe to Darling: it is an observation tool, not product code (Darling AGENTS.md section 6).

Syntax check done on Linux only: `clang -fsyntax-only -target x86_64-apple-macos11 -isysroot <Darling SDK>`. It parses with no stubs once the probe's own category declaration is in place (Darling's SDK header does not declare the method; that is the gap the PR fills).

## What this probe cannot answer

Which of the attribute-run or longest-run rules real AppKit uses when the values compare equal only by `isEqual:` and not identity can be read from rows `D.*`; nothing else is outstanding.

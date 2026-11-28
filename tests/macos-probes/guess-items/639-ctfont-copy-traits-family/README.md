# Probe 639: CTFontCreateCopyWithSymbolicTraits and CTFontCreateCopyWithFamily

PR: darlinghq/darling-cocotron#639 (CoreText: copy a font with other symbolic traits or into another family).

**macOS result: NOT YET OBSERVED**

Public API only, written from the public documentation and from the wording of the PR's GUESS items. No private selectors, no private symbols, no Apple binary, header dump or trace was used. The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. It was derived from the GUESS text, not from the PR's code, so it can also show macOS doing something the PR did not expect. Nothing here claims a Mac result; the Darling guest ran nothing.

Window server: no. Needs installed system fonts (Helvetica, Helvetica Neue, Avenir Next, Courier, Menlo, Times, Arial, Zapfino and a few more); a font that is missing is reported as `original not installed`.

## The GUESS items and what the probe prints for each

Result format on every copy line: `PostScript name ; family ; symbolic traits (hex) ; w=<weight trait> ; width=<width trait> ; size`. `NULL` means the function returned NULL.

1. "How value and mask combine: `(original & ~mask) | (value & mask)`" -> section `639.A.combine`: four originals (regular, bold, oblique, bold-oblique Helvetica) against 13 (value, mask) pairs, including mask 0 (does value matter?), value bits outside the mask, and all-ones.
   - Result equals the formula's face for every row: keep. A row that differs (for example mask 0 still applying value, or value bits outside the mask applying): change the combination rule.
2. "Which bits are compared. Inside a family: italic, bold, expanded, condensed, monospace. Across families: only italic and bold. Vertical and the UI, composite, color-glyph and class-mask bits are never compared" -> `639.C.*` (every bit set and cleared on five originals, same family), `639.D.noface` (families that have no such face: NULL or not), `639.F.family` (10 originals x 15 target families, including a mono original to a proportional family and the reverse, condensed to a non-condensed family, an empty and an unknown family name).
   - A trait bit that makes the copy NULL or changes the face in `639.C` means it is compared inside a family. A condensed or monospace original copied by family into a family without such faces returning a non-NULL face means width and monospace are ignored across families (keep); NULL means they are compared (change). Width tie-break (the PR's open question): the `width=` field of `639.F.family` rows for HelveticaNeue-CondensedBold into families that have several widths.
3. "The choice among several faces with the wanted traits: the face whose weight trait is nearest to a target ... the original's weight, or 0.0 when the request clears the bold trait" -> `639.E.*`: every face of eight multi-weight families (Helvetica Neue, Avenir Next, Avenir, Gill Sans, Optima, Futura, Hoefler Text, Palatino) asked for bold on, bold off, italic on, italic off and no change; `w=` shows each result's weight.
   - Bold-off from a heavy face landing on a regular-weight face (nearest 0.0) matches the PR; landing on the next lighter face (for example Semibold from Black) means the real rule is "nearest the original's weight" or similar: change. Bold-on from a regular face landing on the lightest bold face versus the heaviest or the one the family calls Bold: tells whether the PR's open question about `kCTFontWeightBold` matters.
4. "A NULL matrix is passed on unchanged: fonts here carry no matrix" -> `639.B.matrix`: the copy's `CTFontGetMatrix` and ascent with a NULL matrix and with scale 2, plus `639.B.size` for size 0, explicit size and negative size.
   - Matrix on the copy equal to the argument (identity when NULL) is what the PR assumes; any other value means the argument is composed with something.
5. "The original's variation and cascade list are not carried over" -> `639.G.cascade`, `639.G.variation` (Skia varied to the maximum on its first axis, then italic, no-change and same-family copies), `639.G.features`.
   - "carried over: YES/set" on macOS means the PR drops state macOS keeps: change (or document). "NO/none" confirms the PR.

## Apple documentation

- https://developer.apple.com/documentation/coretext/ctfontcreatecopywithsymbolictraits(_:_:_:_:_:)
- https://developer.apple.com/documentation/coretext/ctfontcreatecopywithfamily(_:_:_:_:)
- https://developer.apple.com/documentation/coretext/kctfontweighttrait

## How to run

Copy this directory to a Mac with the stock Xcode command line tools, then:

```sh
sh build.txt
./probe > results.txt
```

Paste the `PROBE` lines back. Nothing in the output contains user names, host names or paths. Do not commit the probe to Darling: it is an observation tool, not product code (Darling AGENTS.md section 6).

Syntax check done on Linux only: `clang -fsyntax-only -target x86_64-apple-macos11 -isysroot <Darling SDK>`. It parses with no stubs.

## What this probe cannot answer

Which font files a Darling guest ships (the faces available decide what the PR's tests can exercise); only the macOS answer for the macOS font set is observed. Fonts missing on the Mac are reported, not guessed.

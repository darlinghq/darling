# Probe 663: NSTextField maximumNumberOfLines

**macOS result: NOT YET OBSERVED**

Pull request: darling-cocotron #663. Window server: yes (text drawing; the field is rendered in a borderless alpha 0 window, never key, no activation).

Public API only, written from the public documentation and from the wording of the GUESS items, not from the PR code. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into behavioural tests (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. Do not commit the probe to Darling (AGENTS.md section 6).

## GUESS items and what the probe prints

### G1: A negative cap is stored as 0 (no limit). Marked GUESS in the code and the test.

- Prints: 663.1 prints, for six field variants (wrapping with word wrap, char wrap and tail truncation; non-wrapping; explicit newlines), the value read back from maximumNumberOfLines after setMaximumNumberOfLines: -1000, -5, -2, -1, 0, 1, 2, 3, n-1, n, n+1, n+5, 1000, 100000 and NSIntegerMax, plus cellSizeForBounds: and intrinsicContentSize.
- What each outcome means for the PR: Reads back 0 after a negative set and the size equals the unlimited size: keep. Reads back the negative number (stored as is) but sizes as unlimited: change to store as given. Negative sizes as a cap of 1 or raises: change.

### G2: The cap above the number of lines changes nothing (the PR's "caps the lines of a wrapping field").

- Prints: 663.1 for caps equal to the natural line count (n0 from 663.0), one above and well above; 663.2 sizeThatFits.
- What each outcome means for the PR: Same height as unlimited for caps >= n0: keep. Taller or different: change. 663.1 variants 3 to 6 show whether the cap applies to non-wrapping fields and to explicit newlines.

### G3: When the context is not flipped the capped text is anchored at the top of the rect (matches the existing drawWithRect: path); not checked on a display.

- Prints: 663.3 renders a wrapping field with caps 0, 2, 3, n-1 and n+3, unflipped and as a subclass whose isFlipped is YES, in a frame taller and a frame shorter than the capped text, and prints the first and last inked pixel rows from the top of the bitmap and the number of text bands. 663.4 renders a one-line text in a tall frame.
- What each outcome means for the PR: Ink starts at the top of the frame in both contexts and shows cap-many bands: keep. In the unflipped field the text is centred or at the bottom, or shows more or fewer bands than the cap: change the anchor or the line count. A difference between flipped and unflipped: the anchor depends on flippedness (the PR's unflipped choice needs a flipped branch).

Also: The sample text, width 120 and the system font 13 are fixed; 663.0 prints the line height and the natural line count so results can be compared across macOS versions.

## How to run

Copy this directory to a Mac with the stock Xcode command line tools, then:

```sh
sh build.txt
./probe
```

`build.txt` holds one command line. `probe_common.h` is the small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Never activate the app while it runs; the windows are at alpha 0.

## Syntax check

Parses with `clang -fsyntax-only -x objective-c -fobjc-arc -target x86_64-apple-macos11` against Darling's SDK tree (`macOS 11.0 (Darling)`) plus a scratch stub header for the public names that SDK lacks (NSEvent eventWithCGEvent:/CGEvent, NSTableView setDropRow:dropOperation: and the autoresizing style names, NSPasteboard pasteboardWithUniqueName, NSArrayController setObjectClass:, NSTextField maximumNumberOfLines, NSColorSpace genericGrayColorSpace and a few typedefs; not every stub applies to every probe). It was not built or run anywhere.

## Public documentation

- https://developer.apple.com/documentation/appkit/nstextfield/maximumnumberoflines
- https://developer.apple.com/documentation/appkit/nscell/cellsize(forbounds:)
- https://developer.apple.com/documentation/appkit/nstextfield/preferredmaxlayoutwidth

## What this probe cannot answer

Whether the shape of text anchoring is a "visual choice" is a maintainer call; the pixels are what the probe prints.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Until that is done this item stays: macOS result: NOT YET OBSERVED.

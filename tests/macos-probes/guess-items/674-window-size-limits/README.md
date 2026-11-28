# Probe 674: NSWindow contentMinSize / contentMaxSize / minSize / maxSize

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #674. Window server: yes (windows are never ordered in, then ordered in at alpha 0 and never activated). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: A content maximum that is not set is stored as zero (not FLT_MAX) and is taken as no limit; the documented default of contentMaxSize was not found.

- Printed: Part 1 prints the four properties for five window kinds and a utility panel (`part 1 defaults`), with `CGFLOAT_MAX` / `FLT_MAX-ish` markers; part 2 prints what setting 0x0 does to maxSize and contentMaxSize.
- Outcomes: contentMaxSize default is CGFLOAT_MAX/FLT_MAX: macOS stores the large value (Darling's zero differs observably; keep "no limit" semantics but consider the stored value). Default is 0x0: keep.

### GUESS: Each dimension decides on its own (a content minimum of 250x0 limits the width only).

- Printed: Part 2 `contentMinSize 250x0` then `setFrame 100x100`; part 3 limits with one dimension.
- Outcomes: Width clamped to 250 and height left at 100: keep. Both dimensions treated as unset (or the whole minimum ignored): change.

### GUESS: (related) which of minSize / contentMinSize wins when they conflict; clamp on move versus resize.

- Printed: Part 2 shows the four values after each set (set order both ways, min above max, negative, huge); part 3 sets limits before and after the window is outside them and prints frames after setFrame:display:, setContentSize:, setFrameOrigin: (move) and setFrameTopLeftPoint:.
- Outcomes: The values the compositor hint gets (min/max after both sets) are the ones printed; if a move changes the size, the move clamps too.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

Run twice inside the probe (windows not shown / shown at alpha 0) to see whether showing changes any clamp.

## Public documentation

- https://developer.apple.com/documentation/appkit/nswindow/contentminsize
- https://developer.apple.com/documentation/appkit/nswindow/contentmaxsize
- https://developer.apple.com/documentation/appkit/nswindow/minsize
- https://developer.apple.com/documentation/appkit/nswindow/maxsize

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

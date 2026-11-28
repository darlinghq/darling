# Probe 668: NSBox sizeToFit, NSClipView documentRect, NSBrowserCell setImage:

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #668. Window server: yes (views only; the host window is never ordered in). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: NSBox sizeToFit moves the subviews (shifts by the union's origin) instead of the content view.

- Printed: Part A (`668.box`): BEFORE and AFTER frames of the box, content view and every subview (A1..A14).
- Outcomes: Subview frames shifted by the union origin and the content view frame unchanged apart from size: keep. Content view origin moved and subviews unchanged: change.

### GUESS: Hidden subviews are counted in the union.

- Printed: A6 (a hidden third subview far away).
- Outcomes: Box grows to include it: keep. Ignored: change.

### GUESS: The width needed for the title is the title cell's size from where -titleRect starts it, plus 10 points.

- Printed: A1/A2/A4/A5 (title lengths and positions) and the printed titleRect / title cell size before and after.
- Outcomes: Compare the AFTER width with title cell width + offsets; a constant difference from 10 shows the rule. Otherwise change.

### GUESS: The box keeps its frame origin; a content view without subviews is left alone.

- Printed: AFTER box origin in all cases; A7 and A14.
- Outcomes: Origin equal and A7 size unchanged: keep.

### GUESS: The space around the content view is read from the box's current geometry, not from contentViewMargins.

- Printed: A9/A10 (margins 20x20 and 0x0) vs A1 (5x5): whether AFTER sizes differ by the margins.
- Outcomes: Sizes follow contentViewMargins: change. Equal for all margins: keep (also check margins in `geometry after`).

### GUESS: NSClipView: no document view gives an empty rectangle.

- Printed: Part B (`668.clip`) B1; also B2-B12 for smaller/larger/flipped/offset documents, scrolling, contentInsets.
- Outcomes: B1 documentRect is {{0,0},{0,0}}: keep. Another value (for example the clip's own bounds): change.

### GUESS: NSBrowserCell setImage: (stub filled).

- Printed: Part C (`668.cell`): image/alternateImage getters, type, cellSize, drawn pixel counts for leaf / non-leaf, normal / highlighted, copy, initImageCell:, non-NSImage argument.
- Outcomes: Informational: the image is returned and drawn (red pixels > 0), alternate image when highlighted, size grows by the image width.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

NSClipView constrainBoundsRect: is missing from Darling's SDK headers; a scratch stub was used for the syntax check only.

## Public documentation

- https://developer.apple.com/documentation/appkit/nsbox/sizetofit()
- https://developer.apple.com/documentation/appkit/nsclipview/documentrect
- https://developer.apple.com/documentation/appkit/nsbrowsercell/image

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

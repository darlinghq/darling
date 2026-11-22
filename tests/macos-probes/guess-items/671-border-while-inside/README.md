# Probe 671: NSButton showsBorderOnlyWhileMouseInside

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #671. Window server: yes (borderless window at alpha 0, ordered front, no activation; the pointer is not moved unless --warp). Arguments: [--warp].

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: The border is hidden when the property is set, because the pointer position is not known until the next enter or exit.

- Printed: Variant "inside" places the (invisible) window so the pointer lies on the button when the property is set; variant "away" places it elsewhere. Steps 1-3 compare renders (bitmap equality with a bordered and a borderless reference) before, right after setting, and 0.5 s after.
- Outcomes: Right after setting YES the render equals the borderless reference in both variants: keep. In the "inside" variant it equals the bordered one (macOS looks at the pointer): change.

### GUESS: The pointer position after the area is replaced is taken from the window's mouse location.

- Printed: Steps 4-5: after the window and the button grew, and after -updateTrackingAreas, the render and the `window pointer ... in button=` value. In "inside" the pointer is unchanged, so a border that appears only after the area is replaced shows the window location is used.
- Outcomes: Render becomes bordered after step 4/5 in the inside variant only: keep. Stays borderless: macOS waits for the event; change.

### GUESS: "While the pointer is over the button" is mapped to NSTrackingActiveInActiveApp.

- Printed: The `trackingAreas` field of every step: option bits (0x40 InActiveApp, 0x80 ActiveAlways, 0x20 ActiveInKeyWindow, 0x10 ActiveWhenFirstResponder, plus 0x01 enter/exit, 0x100 AssumeInside, 0x200 InVisibleRect). The probe does not activate the app, so no real enter event arrives for 0x40.
- Outcomes: Bits contain 0x40: keep. 0x80 or another activity: change. Steps 6a-6d (synthesised enter/exit given straight to the button and the cell) and `--warp` steps 7-8 (real pointer moved onto and off the button) show whether the border follows enter/exit; with an inactive app and 0x40 steps 7-8 should not change the render.

### GUESS: A cell that is not drawn by its own button keeps its border.

- Printed: Lines `671.foreign`: a standalone NSButtonCell with the property set, drawn by another view; its render is compared with the same cell with the property off.
- Outcomes: `==`: the cell keeps its border (keep). `!=`: the cell hides its border when not in its own button: change.

## How to run

```sh
sh build.txt
./probe --warp
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

`--warp` moves the real pointer with CGWarpMouseCursorPosition and restores it; skip it if you do not want your pointer to move. Step 10 and the `671.bezel` lines (five bezel styles) are extra context.

## Public documentation

- https://developer.apple.com/documentation/appkit/nsbutton/showsborderonlywhilemouseinside
- https://developer.apple.com/documentation/appkit/nsbuttoncell/showsborderonlywhilemouseinside
- https://developer.apple.com/documentation/appkit/nstrackingarea/options

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

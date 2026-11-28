# Probe 0113: Toolbar item view sizes for image-only buttons, pull-downs and text buttons, and toolbar pixels

**macOS result: NOT YET OBSERVED**

Purpose: Shows a toolbar window at alpha 0 and prints the window chrome height, the size and place of each item view, item min and max sizes and a pixel row through the toolbar.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

A titled window with a toolbar of five items (image-only button, image-only pull-down, text button, flexible space, second image-only button) is ordered in at alpha 0 for one second. Printed: window frame, content layout size, the extra height taken, and for each item view: frame size, rectangle in window coordinates, `minSize`, `maxSize`, fitting size, visibility priority; then a row of sRGB pixels through the toolbar. `--front` repeats with a key window. Light and dark.

## Why it matters, and Darling code affected

The patch gives glyph-only toolbar buttons a fixed height and a shared capsule background and tints selected items. The item geometry is public; the capsule is paint.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSToolbar.subproj/NSToolbarView.m:116 (-layoutViewsWithWidth:setFrame:) [darling-cocotron origin/master cd700cad]
- AppKit/NSToolbar.subproj/NSToolbarItemView.m:131-166 (selected-item gradient with dark edges) [darling-cocotron origin/master cd700cad]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./probe            # inactive window
./probe --front    # also a key window
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| Image-only buttons are all one height with equal width padding | Geometry a general layout rule could follow; compare with Darling layout. |
| Pull-down item width adds a chevron allowance | Number for the width rule. |
| Pixel row shows one shared rounded background behind adjacent glyph buttons | Evidence for the capsule; a look change. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nstoolbar
- https://developer.apple.com/documentation/appkit/nstoolbaritem
- https://developer.apple.com/documentation/appkit/nswindow/toolbarstyle-swift.property

## What this probe cannot answer

Pressed-state and dimmed-state paint (needs live mouse tracking) and the decision to adopt a look.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

# Probe 0012: Standard control geometry: sizes, inner rectangles, defaults and sample pixels

**macOS result: NOT YET OBSERVED**

Purpose: Prints cell and intrinsic sizes, title and drawing rectangles, slider and progress defaults, segmented widths and rendered pixels of the standard controls at three control sizes in light and dark.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

For push, square, checkbox, radio, pop-up and pull-down buttons, sliders (both orientations), progress bars, segmented controls (labels One, Two, Three) and text fields and labels, at regular, small and mini control size: `-cellSize`, `-intrinsicContentSize`, `fittingSize`, `-titleRectForBounds:`, `-drawingRectForBounds:`, alignment insets, and the frame after `-sizeToFit`. For sliders also the freshly created min/max/value, `-knobThickness` and `-knobRectFlipped:`. A row of `drawInRect:` calls shows whether text is drawn into a rectangle lower than one line. Finally it renders a few controls into bitmaps (light and dark, window not key) and prints non-premultiplied sRGB pixels on a small grid.

## Why it matters, and Darling code affected

The patch replaces Darling-style control metrics with a different set of numbers and a different painted look. Which numbers a given macOS reports, and whether they differ between macOS releases, decides whether any of it can be a general rule or has to be scoped by OS version (AGENTS.md section 3: scope by OS version only if the evidence shows version dependence). Darling already follows a macOS 10.14.4 observation set for several of these (the comments next to the code say a newer macOS differs and is not followed), so the question for a maintainer is whether to track a newer release at all.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSButtonCell.m:1756 (-cellSize) and :1858-1865 (existing QUIRK block: check box and radio metrics, tier 2, macOS 10.14.4; says a newer macOS differs and is not followed) [darling-cocotron origin/master cd700cad]
- AppKit/NSSliderCell.m:135 (new slider max is 1.0, already matches), :198 (-knobThickness), :461-471 (QUIRK knob rectangle) [darling-cocotron origin/master cd700cad]
- AppKit/NSProgressIndicator.m:322-329 (QUIRK, spinning indicator size only; bar -sizeToFit is a no-op) [darling-cocotron origin/master cd700cad]
- AppKit/NSSegmentedControl/NSSegmentedCell.m:613-624 (-cellSize) [darling-cocotron origin/master cd700cad]
- AppKit/NSTextFieldCell.m:242 (_valueRectForBounds:), :334-340 (cellSize padding, tier 2 on 10.14.4) [darling-cocotron origin/master cd700cad]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./probe
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| Same numbers on every macOS you run (for example 14 and 26) | The metric is general; a Darling change needs no version scope and only needs a test that pins the numbers. |
| Numbers differ between macOS releases | Version-dependent. Either keep following the older set Darling already cites, or scope the new set by OS version with the evidence from both runs. That is a maintainer decision. |
| Bar `-sizeToFit` or the text/label rectangles differ from Darling today | A real gap independent of any new look: Darling leaves bar `-sizeToFit` empty (NSProgressIndicator.m:323). |
| Text is drawn in a rectangle lower than one line (0012.drawInRect rows touched > 0) | macOS draws the first line and relies on clipping; a Darling string drawer that draws nothing there is a bug with a public-API reproducer. |
| Pixel alphas and colours | Only evidence for the painted look. Matching a look is a product decision; the geometry numbers are the part that apps can observe through the API. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsbuttoncell
- https://developer.apple.com/documentation/appkit/nscell/cellsize
- https://developer.apple.com/documentation/appkit/nsslidercell/knobthickness
- https://developer.apple.com/documentation/appkit/nsprogressindicator/sizetofit()
- https://developer.apple.com/documentation/appkit/nssegmentedcontrol
- https://developer.apple.com/documentation/appkit/nstextfieldcell

## What this probe cannot answer

Which look to adopt (flat translucent fills, corner radii, accent handling) is a product decision. The probe only supplies numbers.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

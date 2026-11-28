# Probe 0033: NSScroller widths, part rectangles and legacy-style pixels

**macOS result: NOT YET OBSERVED**

Purpose: Prints scroller widths per control size and style, the rectangle of every part for legacy and overlay styles, and sample pixels of a rendered legacy scroller in light and dark.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

`+preferredScrollerStyle`, `+scrollerWidthForControlSize:scrollerStyle:` for the three control sizes and both styles, and for a vertical and a horizontal scroller with a 0.3 knob proportion at 0.5: `-arrowsPosition`, `-knobStyle`, `-rectForPart:` for all seven parts and `-testPart:` at the middle. Pixels of a rendered legacy scroller are sampled on a grid.

## Why it matters, and Darling code affected

The patch removes the arrow buttons and repaints the scroller. The public geometry (which parts exist, how wide the slot is) is what applications see; the look is not documented. The patch itself says its look was not measured. The overlay style is only drawn while scrolling, which this probe cannot trigger, so only the legacy drawing is sampled.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSScroller.m:120 (+preferredScrollerStyle), :136 (+scrollerWidthForControlSize:scrollerStyle:), :438 (-rectForPart:) [darling-cocotron origin/master cd700cad]

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
| Legacy rectForPart shows no decrement/increment line parts on this macOS | Public geometry changed in that release; a Darling change would be a version-scoped geometry change. |
| Overlay and legacy widths equal or different | Tells whether Darling NSScroller.m:136 returns the right numbers for each style. |
| Pixels | Evidence for the painted look only. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsscroller
- https://developer.apple.com/documentation/appkit/nsscroller/rect(for:)
- https://developer.apple.com/documentation/appkit/nsscroller/scrollerwidth(for:scrollerstyle:)

## What this probe cannot answer

Overlay-style drawing (needs a live scroll gesture) and the decision to change the look.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

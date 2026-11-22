# Probe 0057: getRectsBeingDrawn and needsToDrawRect when drawRect: is called outside a display pass

**macOS result: NOT YET OBSERVED**

Purpose: Prints what getRectsBeingDrawn:count: and needsToDrawRect: answer inside drawRect: for ten scenarios (no window, image focus, subview, dirty rects, windows never shown, real display passes).

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

A view subclass records, from inside `-drawRect:`, the result of `-getRectsBeingDrawn:count:` (count and rectangles, or NULL), `-needsToDrawRect:` for one rectangle inside and one outside the bounds, `-visibleRect`, and whether a window and a graphics context exist. The scenarios: the application calls `-drawRect:` itself on a windowless view (no context, image focus, partial rect, after setNeedsDisplayInRect:, subview of a windowless superview, moved bounds origin, view in a never-shown window, bitmap context) and real passes (`displayIfNeeded`, `displayRect:`, `cacheDisplayInRect:`, `displayRectIgnoringOpacity:inContext:`).

## Why it matters, and Darling code affected

An application that draws a view into an image by calling `-drawRect:` directly relies on these answers. The documentation says the rectangles are valid during a drawing pass; what comes back otherwise is only observable.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSView.m:3260-3273 (-needsToDrawRect: uses _visibleRect, empty without a window) and :3276 (-getRectsBeingDrawn:count: derives rectangles from the opaque ancestor or dirty rects, empty outside a display pass) [darling-cocotron origin/master cd700cad]

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
| Outside a display pass macOS returns the whole bounds (one rectangle) and needsToDrawRect is YES inside bounds | Darling returns count 0 / NO today; a general fix: with no display pass in progress answer the view bounds. |
| macOS returns the dirty rectangle passed to drawRect: or NULL / 0 | The fix must be different from "whole bounds"; the patch would be wrong. |
| Results differ between windowless and windowed views (0057.1 versus 0057.7) | Needs two code paths, mirroring macOS. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsview/getrectsbeingdrawn(_:count:)
- https://developer.apple.com/documentation/appkit/nsview/needstodraw(_:)

## What this probe cannot answer

Nothing known; every item is an API-visible value or behaviour.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

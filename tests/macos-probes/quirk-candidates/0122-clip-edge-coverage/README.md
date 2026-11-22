# Probe 0122: Which pixels a clip touches when its edge is a hair off a pixel boundary

**macOS result: NOT YET OBSERVED**

Purpose: Prints the alpha coverage of the pixels around a clip edge moved by tiny offsets, with antialiasing on and off, three clip calls, three CTM scales, and the clip bounding box.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

A 24 by 24 RGBA bitmap context. A device-space square from 4 to 12 is clipped with `CGContextClipToRect`, with a rectangle path and `CGContextClip`, and with `CGContextClipToRects`; one edge (the maximum, then the minimum) is moved by -0.01, -0.002, -0.0005, -0.0001, 0, 0.0001, 0.0005, 0.002, 0.01, 0.25 and 0.5 device pixels; the context has CTM scale 1, 719/479 and 2 (the rectangle is divided by the scale so the device edge is the same); antialiasing on and off. The whole canvas is then filled with opaque white and the alpha of the four pixels around the edge is printed along x and along y, with `CGContextGetClipBoundingBox` mapped back to device units.

## Why it matters, and Darling code affected

The patch rounds a clip outward after allowing a 1/1024 pixel tolerance. The public contract for clipping is geometric coverage; whether an edge a hair past a boundary covers the next pixel is observable.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- Onyx2D/O2Context_builtin.m:230-241 (O2ContextClipViewportToPath rounds the path bounding box outward with O2RectIntegral) and :246-260 (rectangle clips only narrow the viewport) [darling-cocotron origin/master cd700cad]

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
| With antialiasing on, a +0.0001 edge leaves the next pixel at alpha 0 or a tiny coverage | macOS clips by coverage; Darling rounding outward (full pixel) differs for every fractional rectangle clip, not only at 1/1024. |
| With antialiasing off, the next pixel is untouched for tiny offsets | A rounding rule exists; the tolerance question is answered by the exact offset where the pixel turns on. |
| Results identical for the three clip calls and the three scales | The rule is independent of the clip entry point and of scale; one fix point in Onyx2D is enough. |

## Public documentation

- https://developer.apple.com/documentation/coregraphics/cgcontext
- https://developer.apple.com/documentation/coregraphics/cgcontext/setshouldantialias(_:)
- https://developer.apple.com/documentation/coregraphics/cgcontext/boundingboxofclippath

## What this probe cannot answer

The diagnostic-trace hunk of the patch is not an API question.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

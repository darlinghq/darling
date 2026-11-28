# Probe item-a: Does CALayer.contents change an IOSurface use count?

**macOS result: NOT YET OBSERVED**

Purpose: Prints use count, in-use flag and retain count of an IOSurface across assigning it to layers, replacing and clearing contents, rendering, an application-owned use count and layer release.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

A 64 by 64 BGRA IOSurface is assigned to `CALayer.contents` of one and then two layers, re-assigned, replaced by another surface, cleared, rendered with `-renderInContext:`, held by the application through `IOSurfaceIncrementUseCount`, put in a temporary layer that is released, and committed in a layer tree with `CATransaction flush`. After each step `IOSurfaceGetUseCount`, `IOSurfaceIsInUse` and `CFGetRetainCount` are printed.

## Why it matters, and Darling code affected

This item belongs to a layer-contents hunk of a patch outside the author's slice, so the probe is written from the question as phrased: does a layer holding an IOSurface as contents add a use count (as opposed to a retain)? Only the public IOSurface counters can answer it.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- QuartzCore/CALayer.m:390-395 (-setContents: retains the object and releases the previous one; no IOSurface use-count calls) [darling-cocotron origin/master cd700cad]
- QuartzCore/CAContext.m:44 and :78-79 (IOSurface used for remote context frames) [darling-cocotron origin/master cd700cad]

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
| Use count rises by one per layer holding the surface and returns to the base when contents are cleared or the layer is released | macOS balances a use count per layer; Darling retaining only differs for apps that read IOSurfaceGetUseCount or IOSurfaceIsInUse (for example to recycle buffers). |
| Use count stays 0 throughout (only the retain count moves) | No use-count behaviour to replicate. |
| The count changes only after a transaction flush or a render | The count belongs to the render/commit path, not to the property setter. |

## Public documentation

- https://developer.apple.com/documentation/iosurface/iosurfacegetusecount(_:)
- https://developer.apple.com/documentation/iosurface/iosurfaceisinuse(_:)
- https://developer.apple.com/documentation/quartzcore/calayer/contents

## What this probe cannot answer

Which Darling code path should hold the count (layer or renderer) cannot be read off the numbers alone; the 0155 hunk itself was not read.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

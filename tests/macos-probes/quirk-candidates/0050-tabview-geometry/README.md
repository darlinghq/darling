# Probe 0050: NSTabView geometry for each tab type and control size, tab hit runs, rendered pixels

**macOS result: NOT YET OBSERVED**

Purpose: Prints contentRect, minimumSize and intrinsic size per tab view type and control size, where each tab is by hit testing, and a pixel grid of a rendered tab view in light and dark.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

Seven tab view types by three control sizes: `-contentRect`, `-minimumSize`, `-intrinsicContentSize`. For a top-tabs view 300 by 200 with items One, Two, Three it walks `-tabViewItemAtPoint:` across x for several y and reports runs (which item is under which x range). Then it renders the view with item Two selected and prints sRGB pixels on a row grid.

## Why it matters, and Darling code affected

The patch repaints tabs and the pane as rounded segments. The geometry (content rectangle, tab extents) is observable through public API; the painted look is not documented.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSTabView.m:146 (-minimumSize), :245 (-contentRect), :351 (-tabViewItemAtPoint:) [darling-cocotron origin/master cd700cad]
- AppKit/NSInterfacePart/NSGraphicsStyle.m:1103 (-drawTabInRect:...) [darling-cocotron origin/master cd700cad]

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
| contentRect and tab extents equal Darling NSTabViewContentMargins today | Only the paint differs; a look change is a maintainer decision. |
| They differ | Geometry gap observable by any app; a version-scoped fix with these numbers as evidence. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nstabview
- https://developer.apple.com/documentation/appkit/nstabview/contentrect

## What this probe cannot answer

The decision to adopt a newer macOS look.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

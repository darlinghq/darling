# Probe item-b: NSTextFinder / text view find: which ranges are selected for each matching type and case option

**macOS result: NOT YET OBSERVED**

Purpose: Presses find-next repeatedly over a fixed text for each matching type, case option and needle, and prints the selected ranges, for the classic find panel action and the find bar action.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

A fixed text (words differing by case, diacritics, a decomposed accent, full-width letters, punctuation) is searched for twelve needles under four matching types (contains, starts with, full word, ends with) and two case options. The needle and options are written to the find pasteboard (string and the search-options property list). Up to 16 find-next actions are sent to an NSTextView, first through `-performFindPanelAction:` and then through `-performTextFinderAction:` in a text view that uses the find bar; the selected ranges are printed until the search wraps.

## Why it matters, and Darling code affected

The patch behind this item was not available to the author, so the axes are the ones the public NSTextFinder documentation names. Darling's NSTextFinder is a stub that logs and does nothing, so any behaviour will have to be built from the observed results.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSTextFinder.m:1-15 (stub class that logs every call) [darling-cocotron origin/master cd700cad]
- AppKit/NSTextView.subproj/NSTextView.m:3904 (-performFindPanelAction:) [darling-cocotron origin/master cd700cad]

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
| Full word excludes foo.bar, foo-bar, foo_bar and foo123 (or some of them) | Defines the word-boundary rule; pin it exactly. |
| Case-insensitive search also matches diacritic or full-width variants (or not) | Defines what insensitivity covers. |
| Empty needle selects nothing | Pins the empty-needle case. |
| Classic and find-bar results differ | Two entry points need separate treatment. |
| The selection does not move at all | The probe could not drive the find (no window server focus); rerun in a normal login session before concluding. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nstextfinder
- https://developer.apple.com/documentation/appkit/nstextfinder/matchingtype
- https://developer.apple.com/documentation/appkit/nstextview/performfindpanelaction(_:)
- https://developer.apple.com/documentation/appkit/nspasteboard/pasteboardtype/findpanelsearchoptions

## What this probe cannot answer

The exact rule the patch implements is unknown to this author; the probe covers the generic axes only. A maintainer must map the output to the PR's question. Against Darling's SDK tree the probe does not compile as-is (NSTextFinder constants missing).

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

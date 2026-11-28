# Probe 0024: NSArrayController selection beyond the content, and where binding failures surface

**macOS result: NOT YET OBSERVED**

Purpose: Prints selectedObjects and selectionIndexes for selections that name rows the content lacks, and records exceptions from bind, model-to-view and view-to-model changes.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

(1) An NSArrayController with 3 objects is given selection indexes beyond the end, a single out-of-range index, a content replaced by a shorter one, and a selection bound to a model before the content arrives. For each step it prints `-selectedObjects` and `-selectionIndexes` and any exception. (2) A text field and checkboxes are bound to model key paths that do not exist; the probe records whether the exception comes from `-bind:toObject:withKeyPath:options:`, from a model-to-view change, or from a write-back after a click, and what happens to a later change.

## Why it matters, and Darling code affected

The patch clamps the out-of-range selection inside `selectedObjects` and swallows binding exceptions inside the binder. Neither is specified by the public API text, so the observable behaviour of the original is the only evidence. The font and nib hunks of the same patch are not covered here (see below).

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSController/NSArrayController.m:488-494 (-selectedObjects calls objectsAtIndexes: with the raw selection) [darling-cocotron origin/master cd700cad]
- AppKit/NSKeyValueBinding/NSBinder.m:246-280 (observeValueForKeyPath: sets the destination, then restarts observing, no @finally) [darling-cocotron origin/master cd700cad]
- AppKit/NSKeyValueBinding/NSKVOBinder.m:52-70 (startObservingChanges has an @try; :331 observeValueForKeyPath: and :174 writeDestinationToSource have none) [darling-cocotron origin/master cd700cad]

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
| selectedObjects returns only the in-range objects and no exception | macOS tolerates it; Darling raises NSRangeException from objectsAtIndexes: today, so a documented-contract-compatible fix is to drop the out-of-range indexes (general, not app-specific). |
| selectedObjects raises on macOS too | The original also fails; the patch behaviour would be an extension and should not be ported. |
| selectionIndexes itself is clamped by setSelectionIndexes: (0024.1) | The clamp belongs in the setter or the content-change path, not in selectedObjects. |
| Binding to a missing key path raises from -bind: on macOS | Matches Darling raising at the call site; nothing to change. |
| Binding failures are reported (logged) and the change goes on, no exception reaches the caller | macOS isolates a failing binding; the equivalent guard in Darling is a general fix (observeValueForKeyPath: in both binders). |
| Exceptions propagate out of -performClick: (0024.6b) or the model setter (0024.7a) | macOS does not isolate; do not add a guard. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsarraycontroller/selectedobjects
- https://developer.apple.com/documentation/appkit/nsarraycontroller/setselectionindexes(_:)
- https://developer.apple.com/documentation/appkit/cocoa_bindings

## What this probe cannot answer

The nib-decoding hunk (a zero font size archived by a nib) is already handled in Darling (AppKit/NSFont.m, sizeOrDefault) and cannot be probed through public API. The font-substitution hunk depends on a registry that exists only in the port; it has no macOS question.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

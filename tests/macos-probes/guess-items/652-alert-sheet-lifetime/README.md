# Probe 652: NSAlert lifetime while its button ends the sheet

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #652. Window server: yes (host window at alpha 0 ordered front, no activation). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: Whether macOS keeps the alert alive until its completion handler has returned.

- Printed: Built without ARC. An NSAlert subclass logs its dealloc; the creator releases its only reference right after beginSheetModalForWindow:, then the probe presses the alert's button (-performClick:) or ends the sheet with endSheet:returnCode:. Output: the order of handler-start, handler-end, alert-dealloc and the pool markers (scenarios A, B, D, E; C is the control with the creator's reference kept). The probe never touches the alert after it could be freed.
- Outcomes: alert-dealloc after handler-end: macOS keeps it alive through the handler (observably; Darling's fix matches). alert-dealloc before handler-start or between start and end: macOS does not (a use after free in the original code would be real; observation is only that dealloc happens early). Dealloc only at the pool drain: the sheet autoreleases it; still no use-after-free by the sheet.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

The item is observable only through dealloc ordering of an NSAlert subclass; whether some other object holds the alert is not distinguished. The PR says its fix does not depend on the answer.

## Public documentation

- https://developer.apple.com/documentation/appkit/nsalert/beginsheetmodal(for:completionhandler:)
- https://developer.apple.com/documentation/appkit/nsalert

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

# Probe 643: NSSharingServicePicker delegate order and presentation

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #643. Window server: yes (host window at alpha 0 ordered front with orderFrontRegardless; the application is never activated; the picker menu is cancelled by the probe). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: The service's delegate is requested before the picker's delegate is told.

- Printed: Part 2 (`643.choose`): a custom do-nothing service (initWithTitle:image:alternateImage:handler:) is offered through the picker delegate, then chosen by the probe through the picker's standardShareMenuItem submenu (performActionForItemAtIndex:). The event list shows, in order, delegateForSharingService:, the service's own handler, didChooseSharingService:, and the service-delegate calls. Control `643.control` runs performWithItems: without the picker.
- Outcomes: delegateForSharingService before didChooseSharingService: keep. The reverse order, or delegateForSharingService only at a different moment (e.g. when the menu is built): change. If the custom item is not found in the submenu, the output says so and the question stays open (a person would have to choose a service by hand).

### GUESS: The menu opens at the middle of the named edge of the rectangle.

- Printed: Part 1 (`643.show`) for each preferredEdge: while the menu is up (cancelled after at most 0.8 s) the probe lists this process's on-screen windows other than the host (CGWindowListCopyWindowInfo, bounds only) and prints their position relative to the middle of the rectangle in screen coordinates. A stand-in for "where the menu opens".
- Outcomes: Menu horizontal centre equals the rectangle's midX for the minY/maxY edges (and vertical centre for minX/maxX): keep. Menu left edge at the rectangle edge, or centred differently: change. No window listed (the menu is not a window or did not open because the app is inactive): not observable this way; the line says `own windows on screen=0`.

### GUESS: With no service to offer nothing is shown.

- Printed: Cases: delegate returns an empty array, returns nil, empty items array, an unshareable item, no delegate. For each: events, whether NSMenuDidBeginTrackingNotification fired, the menu's item titles, whether any window appeared.
- Outcomes: Tracking never begins and no window appears for the empty cases: keep. A menu with a placeholder item (for example "No services" or a disabled row) appears: change.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

If the shown picker is not a menu on the Mac you run this on (some releases use a popover), `tracking began=0` while a window is listed; that is itself the finding.

## Public documentation

- https://developer.apple.com/documentation/appkit/nssharingservicepicker
- https://developer.apple.com/documentation/appkit/nssharingservicepickerdelegate
- https://developer.apple.com/documentation/appkit/nssharingservice

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

# Probe 650: Pop-up button menu: delegate calls and tracking notifications

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #650. Window server: yes (alpha 0 window ordered front, no activation; tracking cancelled by the probe). Arguments: [--synth].

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: Where -menuDidClose: falls relative to the cell's own selection change.

- Printed: With `--synth` the probe selects an item with public key events (Down, Down, Return posted to this process): lines `650.popup ... (synthesised keys)` show the order of SELECTION-CHANGED (KVO on indexOfSelectedItem), ACTION (the button's action), didClose and EndTracking. Without `--synth` nothing is selected and the item is not observed.
- Outcomes: didClose after the selection change and action: keep (Darling sends it after the window is closed). didClose before the selection change: change the order.

### GUESS: A pop-up button's tracking also posts NSMenuDidBeginTrackingNotification and NSMenuDidEndTrackingNotification.

- Printed: Every `650.popup` line has `begin=` and `end=` counts and the notifications with the menu title (pop-up and pull-down variants).
- Outcomes: begin=1 end=1 for the button's menu: macOS posts them; Darling should (it does not yet for pop-up buttons per the PR). begin=0: keep (do not add).

### GUESS: A pop-up button's list never shows a submenu, so none is updated.

- Printed: The menu carries an item with a submenu whose delegate logs `needsUpdate(pb-sub)`, `willOpen(pb-sub)`. Appears in the events only if the submenu is updated or opened (cancelled before any hover, so only an eager update shows).
- Outcomes: No `pb-sub` calls: keep. `needsUpdate(pb-sub)` while opening: change (submenus are updated with the menu).

## How to run

```sh
sh build.txt
./probe --synth
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

Cases: pop-up with autoenablesItems YES and NO, and a pull-down. Whether the menu opens at all for an inactive app is part of the output (`performClick-returned` before or after the calls).

## Public documentation

- https://developer.apple.com/documentation/appkit/nsmenudelegate
- https://developer.apple.com/documentation/appkit/nspopupbutton
- https://developer.apple.com/documentation/appkit/nsmenu/didbegintrackingnotification

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

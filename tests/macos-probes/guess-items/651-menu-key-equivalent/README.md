# Probe 651: Menu key-equivalent search and the menu delegate

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #651. Window server: needs a window server connection only (no window is created, no activation). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: The delegate is asked only when autoenablesItems is YES.

- Printed: Scenarios 6-8 (main NO), 12-13 (everything NO), 7: `needsUpdate(...)` lines present or absent in `651.search` for a menu with autoenablesItems NO, found and not found.
- Outcomes: Delegates of non-autoenabling menus are asked: change (ask regardless). Not asked: keep.

### GUESS: A submenu whose own autoenablesItems is NO under a YES parent: updated before by the parent's recursive update; now not.

- Printed: Scenarios 9-11 and the `651.control` lines (explicit [main update]) show whether the S1/S2 delegates are asked in each combination during the search and during a plain update.
- Outcomes: S1 delegate asked during the search although S1 is NO: macOS updates it (the old Darling behaviour was right; change). Not asked: keep.

### GUESS: The order submenu delegates are asked: all before any item is looked at, or one by one as the search reaches them.

- Printed: Scenarios 1-5, 20-22: the sequence of needsUpdate(main/S1/S2/S1b), validate(item) lines, and the ACTION line. Found in S1 with S2 still asked afterwards means all are updated first; S2 absent after a hit in S1 means one by one.
- Outcomes: All delegates asked before the first validate: the old order is right (change). Interleaved with the search: keep.

### GUESS: (title) the key equivalent is looked for after the delegate has updated it.

- Printed: Scenarios 14-16: the S1 delegate rewrites Open to cmd-x inside menuNeedsUpdate:; the result for cmd-x and cmd-o.
- Outcomes: cmd-x found and cmd-o not: keep (search after the update). The reverse: change.

### GUESS: (related) menuHasKeyEquivalent:forEvent:target:action:.

- Printed: Scenarios 17-19: a delegate implementing it; whether it is asked, when, and whether its answer runs the action.
- Outcomes: Informational; compare with what the PR does with that call.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

The events are synthesised with NSEvent keyEventWithType (public); nothing is posted to the system.

## Public documentation

- https://developer.apple.com/documentation/appkit/nsmenu/performkeyequivalent(with:)
- https://developer.apple.com/documentation/appkit/nsmenudelegate
- https://developer.apple.com/documentation/appkit/nsmenu/autoenablesitems

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

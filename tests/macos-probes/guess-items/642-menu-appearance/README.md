# Probe 642: NSMenu appearance and effectiveAppearance (extends item-f)

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #642. Window server: yes (one window at alpha 0, never ordered in; no activation). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: Without its own appearance a menu inherits the application's effective appearance (as NSWindow does); whether a submenu looks at its supermenu first is not documented, so the supermenu fallback is left out.

- Printed: Probe item-f (batch 1) already prints the main/sub/subsub/detached chain. This probe adds the pop-up button menu case: lines `642.C` show the menu's effectiveAppearance when only the button, only the button's window, only the application, or only the menu has an appearance, and lines `642.B` show whether effectiveAppearance is the very object that was set and equals NSApp.effectiveAppearance when nothing is set.
- Outcomes: Menu follows the application (or the system) and ignores the button/window appearance: keep. Menu follows the button's window or the button: the fallback is wrong for menus attached to a view; change (and say which view appearance wins). Together with item-f step 2/3 (submenu follows supermenu or not) this decides the supermenu fallback question.

### GUESS: A copy keeps the appearance.

- Printed: Lines `642.A`: copy and mutableCopy of a menu with its own appearance (aqua, dark, vibrantDark), copy of a hierarchy where only main or only sub has one, the copy's submenus, a copy of a submenu alone, and a copy of a menu with nothing set (`642.D`: clearing the copy leaves the original unchanged).
- Outcomes: Copy reports the same own `appearance` (and effectiveAppearance) as the original: keep. Copy has appearance nil while the original had one: change (copy must not carry it). A copy of a hierarchy keeps it on the top menu only or also on submenus: shows what -copy does to submenus.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

Run item-f too (`../../probes/item-f-menu-effective-appearance`); this probe does not repeat its ten steps.

## Public documentation

- https://developer.apple.com/documentation/appkit/nsappearancecustomization/effectiveappearance
- https://developer.apple.com/documentation/appkit/nsappearancecustomization/appearance
- https://developer.apple.com/documentation/appkit/nsmenu

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

# Probe 649: NSMenu delegate menu:willHighlightItem:

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #649. Window server: yes (menus popped up at the pointer, cancelled by the probe after about 0.6 s; the application is not activated unless --manual-menubar is used). Arguments: [--synth] [--manual-menubar].

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: The delegate of the menu bar's menu hears (or not) the call for the bar's top-level titles.

- Printed: `--manual-menubar` only: the probe makes the app regular, activates it (steals focus, 20 s) and asks you to move over the menu bar titles, the dropdown rows and away. Prints every delegate call (bar and dropdown delegates). Without that flag this item is not observed.
- Outcomes: Calls for the bar menu with the titles' items: keep (the bar is included). No calls with the bar menu as `menu`: change (exclude the bar).

### GUESS: Whether a separator row or a disabled row under the pointer counts as "about to highlight".

- Printed: Part 1 pops the menu with the separator, then the disabled row, then enabled rows as the positioning item so the pointer lies on that row; prints the delegate calls and `highlightedItem` after 0.5 s. `--synth` additionally drives the open menu with Quartz events posted to this process (arrow keys, mouse moves down the menu window) and prints the highlight sequence with the row under the pointer.
- Outcomes: A call with the separator or the disabled item: highlight is reported for them (Darling, which sends whatever index tracking selects, matches). No call (or nil) for them: change (do not report non-highlightable rows).

### GUESS: No nil is sent when a menu closes.

- Printed: Every `649.popup` line lists the order of calls through the close: `willHighlight(...: nil)` after the last item and before/after `didClose`, and what `highlightedItem` was.
- Outcomes: A nil call at close (before or after menuDidClose): change (send an unhighlight at close). None: keep.

### GUESS: On the menu bar, the overflow chevron is reported as nil.

- Printed: Not observable: needs a menu bar so full that the overflow control appears, and a person to hover it. Not probed.
- Outcomes: n/a

## How to run

```sh
sh build.txt
./probe --synth --manual-menubar
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

Input: the default run needs no input. `--synth` posts events only to this process and may need the Accessibility permission; if nothing is delivered the lines show no highlight changes.

## Public documentation

- https://developer.apple.com/documentation/appkit/nsmenudelegate/menu(_:willhighlight:)
- https://developer.apple.com/documentation/appkit/nsmenu/popupmenu(positioning:at:in:)

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

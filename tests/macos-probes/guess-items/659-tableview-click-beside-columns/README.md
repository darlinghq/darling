# Probe 659: a click beside every column of an NSTableView

**macOS result: NOT YET OBSERVED**

Pull request: darling-cocotron #659. Window server: yes (alpha 0 titled window ordered front but never made key, no activation). Events are made with +mouseEventWithType:... and delivered by -[NSWindow sendEvent:] and, separately, by calling -[NSTableView mouseDown:] directly (a window that is not key may swallow a first click, so the two routes are printed apart). A 90 second watchdog exits if a tracking loop hangs..

Public API only, written from the public documentation and from the wording of the GUESS items, not from the PR code. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into behavioural tests (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. Do not commit the probe to Darling (AGENTS.md section 6).

## GUESS items and what the probe prints

### G1: A click beside the columns selects the row and sends the action (the documentation is silent; selection and action are what a click on the row does for any other column).

- Prints: 659.1 clicks beside the columns (x=300 and x=395) on row 1, below the last row, between columns and inside a column as control, for five table configurations (action and double action with editable columns; action only; non-editable columns; no columns; three hidden columns), with click count 1 and 2, both delivery routes. Prints selectedRows, clickedRow, clickedColumn during the action call and afterwards, calls in order. 659.2 does the same with row 3 already selected.
- What each outcome means for the PR: Row 1 selected, action sent with clickedRow=1 and clickedColumn=-1: keep. Nothing selected or no action: change. clickedColumn another value than -1: change what the clicked column is reported as.

### G2: A double click there sends the double action and starts no edit.

- Prints: 659.1 with click count 2 (two sequences, count 1 then count 2) prints doubleAction calls and editedRow/editedColumn afterwards, for editable and non-editable columns.
- What each outcome means for the PR: doubleAction sent once and editedRow -1: keep. An edit starts (editedRow >= 0) or the single action is sent instead: change.

Also: Not asked but printed: the same clicks inside a column (control), a table with no columns and a table whose columns are all hidden, and whether the leftover event queue is empty after the tracking loop.

## How to run

Copy this directory to a Mac with the stock Xcode command line tools, then:

```sh
sh build.txt
./probe
```

`build.txt` holds one command line. `probe_common.h` is the small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Never activate the app while it runs; the windows are at alpha 0.

## Syntax check

Parses with `clang -fsyntax-only -x objective-c -fobjc-arc -target x86_64-apple-macos11` against Darling's SDK tree (`macOS 11.0 (Darling)`) plus a scratch stub header for the public names that SDK lacks (NSEvent eventWithCGEvent:/CGEvent, NSTableView setDropRow:dropOperation: and the autoresizing style names, NSPasteboard pasteboardWithUniqueName, NSArrayController setObjectClass:, NSTextField maximumNumberOfLines, NSColorSpace genericGrayColorSpace and a few typedefs; not every stub applies to every probe). It was not built or run anywhere.

## Public documentation

- https://developer.apple.com/documentation/appkit/nstableview/clickedcolumn
- https://developer.apple.com/documentation/appkit/nstableview/clickedrow
- https://developer.apple.com/documentation/appkit/nscontrol/doubleaction
- https://developer.apple.com/documentation/appkit/nsevent/mouseevent(with:location:modifierflags:timestamp:windownumber:context:eventnumber:clickcount:pressure:)

## What this probe cannot answer

Nothing known. A real window-server click may differ from synthesised events when the window is key; run the probe a second time with a visible, key window if the two routes disagree (not scripted: the probe must not steal focus).

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Until that is done this item stays: macOS result: NOT YET OBSERVED.

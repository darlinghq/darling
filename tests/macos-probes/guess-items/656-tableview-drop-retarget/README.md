# Probe 656: NSTableView drop retargeting with setDropRow:dropOperation: and the returned drag operation

**macOS result: NOT YET OBSERVED**

Pull request: darling-cocotron #656. Window server: yes (a table in an alpha 0 titled window, ordered front but never made key; the application is never activated). No user input and no drag session: the table's public NSDraggingDestination methods are called with a fake NSDraggingInfo object the probe implements. Any selector the table calls on it that the probe does not implement is printed as info-selector-missing..

Public API only, written from the public documentation and from the wording of the GUESS items, not from the PR code. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into behavioural tests (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. Do not commit the probe to Darling (AGENTS.md section 6).

## GUESS items and what the probe prints

### G1: _validateDraggedRow: proposes NSTableViewDropAbove for the row under the pointer (the documentation does not say how the table chooses between drop-on and drop-above).

- Prints: 656.B and 656.B2 print validate(row,op) for pointer positions from 2 percent to 98 percent through a row, in the gap between rows, just below the last row, well below the rows, and 656.B3 beside the columns; 656.B4 repeats for the four draggingDestinationFeedbackStyle values.
- What each outcome means for the PR: Always Above for a point in a row: keep. On for the middle of the row and Above near an edge (or On everywhere): change to a position rule; the 656.B lines give the thresholds. Row 5 (numberOfRows) with Above below the rows and -1 with On: the proposal rule for the empty area.

### G2: A data source without validateDrop refuses every drop (the documentation lists the method as optional and does not say what the table does without it).

- Prints: 656.A prints, for nine pointer positions, the operation returned from draggingEntered and draggingUpdated, prepareForDragOperation, performDragOperation and the calls the data source saw, for a data source with acceptDrop but no validateDrop; 656.A2 for a data source with neither.
- What each outcome means for the PR: entered/updated None and perform 0 with no accept call: keep. A non-None operation or an acceptDrop call: change (the table accepts without validation, or the table needs a default operation).

### G3: The look of the drop-on outline (a one-pixel-inset black rectangle around the row, or around the visible rect for row -1).

- Prints: Not observable through the calls this probe makes (it draws nothing). See the report: this is a visual choice.
- What each outcome means for the PR: n/a

### G4: acceptDrop receives the target left by the last validation, not one recomputed at drop time.

- Prints: 656.D validates over row 1, moves the pointer to row 3 and calls performDragOperation without another update; 656.E calls performDragOperation with no prior validation; 656.C prints accept(row,op) after every retarget (On, Above, -1, numberOfRows, 99, -5).
- What each outcome means for the PR: accept(row=1,...) in 656.D: keep. accept(row=3,...): change to recompute at drop time. 656.E: shows whether the table validates itself when none ran (perform 0 and no accept call, or a validate call before accept).

Also: 656.C and C2/C3 also record the drag operation the table returns for each retarget and each data-source return value (None, Copy, Generic, Every, Delete) and for source masks that exclude the returned operation: the title of the PR says the table returns the data source's operation; these lines confirm or correct it, and show how out-of-range rows given to setDropRow: are treated.

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

- https://developer.apple.com/documentation/appkit/nstableview/setdroprow(_:dropoperation:)
- https://developer.apple.com/documentation/appkit/nstableviewdatasource/tableview(_:validatedrop:proposedrow:proposeddropoperation:)
- https://developer.apple.com/documentation/appkit/nsdraggingdestination
- https://developer.apple.com/documentation/appkit/nstableview/dropoperation

## What this probe cannot answer

G3, the drop-on outline, is a visual choice and is not observable through any public API without a screenshot of a live drag (needs a user drag or screen capture permission).

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Until that is done this item stays: macOS result: NOT YET OBSERVED.

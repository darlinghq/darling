# Probe 660: NSTableColumn value binding with a value transformer

**macOS result: NOT YET OBSERVED**

Pull request: darling-cocotron #660. Window server: yes (a table in an alpha 0 titled window, never key, no activation; the cell editor is a field editor in that window).

Public API only, written from the public documentation and from the wording of the GUESS items, not from the PR code. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into behavioural tests (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. Do not commit the probe to Darling (AGENTS.md section 6).

## GUESS items and what the probe prints

### G1: A transformer that cannot reverse makes the column's binding read-only (nothing stored, allowsEditingForRow: NO) instead of raising when an edit ends; the documentation does not state it for table columns.

- Prints: 660.1 to 660.6 run six configurations (no transformer; a non-reversible transformer; a reversible one; one that says YES to reversing but returns nil from the reverse; a non-reversible one by registered name; a non-reversible one with column.editable forced YES) and print what each cell shows, cell and column editability, the transformer calls, whether editColumn:row:withEvent:select: starts an edit, the model after the edit ends and any exception.
- What each outcome means for the PR: Non-reversible: editedRow -1 or an edit that leaves the model unchanged with no exception: keep. An exception when the edit ends, or the model receives the transformed (upper-cased) value: change. cell editable=0 for non-reversible: matches allowsEditingForRow: NO; editable=1 with the write silently dropped: the read-only rule is on write only.

### G2: Null and marker placeholders are not applied by this binder (unchanged); only the value transformer is.

- Prints: 660.7 and 660.8 bind with NSNullPlaceholder, all four placeholders, a placeholder plus a transformer; rows are a string, NSNull, a missing key and a string; prints the cell objectValue/stringValue and placeholderString per row. 660.1 shows the same rows without placeholders; 660.9 the empty controller.
- What each outcome means for the PR: Rows 1 and 2 show the placeholder text (in objectValue, stringValue or placeholderString): macOS applies the null placeholder, change. Rows show nil or empty with no placeholder: keep. The transformer called with nil or NSNull (see "transformer calls"): shows whether the transformer sees the null before the placeholder.

Also: The probe also records the transformer being asked for rows with NSNull and missing values, and what an edit of a null row writes (660.4, 660.5).

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

- https://developer.apple.com/documentation/appkit/nstablecolumn
- https://developer.apple.com/documentation/foundation/nsvaluetransformer
- https://developer.apple.com/documentation/appkit/nsbindingoption/valuetransformer
- https://developer.apple.com/documentation/appkit/nsbindingoption/nullplaceholder
- https://developer.apple.com/documentation/appkit/nsbindingname/value

## What this probe cannot answer

allowsEditingForRow: is not a public API of NSTableColumn in the documentation I know; the probe observes it through whether editColumn:row: starts an edit and through the data cell's isEditable.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Until that is done this item stays: macOS result: NOT YET OBSERVED.

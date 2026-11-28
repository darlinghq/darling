# Probe 657: hidden NSTableColumns in cell-based tables and their header

**macOS result: NOT YET OBSERVED**

Pull request: darling-cocotron #657. Window server: yes (a table in a scroll view in an alpha 0 titled window, ordered front, never key, no activation).

Public API only, written from the public documentation and from the wording of the GUESS items, not from the PR code. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into behavioural tests (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. Do not commit the probe to Darling (AGENTS.md section 6).

## GUESS items and what the probe prints

### G1: A hidden column takes no width at all, its share of the intercell spacing included (the documentation says only that it is hidden and still counted).

- Prints: 657.1 prints, for ten hidden patterns (none, 1, 0, 4, 1+3, 1+2, 0+4, all but 2, all, 2), column widths, rectOfColumn: x and width for every column, headerRectOfColumn:, columnAtPoint: at each column centre and left edge, the table frame width, bounds width and the intercell spacing; 657.3 the cell frames of one row.
- What each outcome means for the PR: Table frame width grows by (width + spacing) for each shown column and by nothing for a hidden one: keep. The frame or following x offsets still include one spacing for a hidden column: change. rectOfColumn: of a hidden column with width 0 versus the old width shows what the rectangle of a hidden column is.

### G2: sizeLastColumnToFit resizes the last column that is shown.

- Prints: 657.4 prints the widths after sizeLastColumnToFit with a clip wider (700) and narrower (300) than the columns, for every pattern, including a hidden last column (patterns 4, 0+4).
- What each outcome means for the PR: The last shown column changes width and the hidden column keeps its width: keep. The hidden last column changes, or nothing changes: change.

### G3: The autoresizing styles (and sizeToFit) leave hidden columns alone and share the width among the columns that are shown.

- Prints: 657.6 builds at clip 600 for six styles (None, Uniform, Sequential, ReverseSequential, LastOnly, FirstOnly) and six hidden patterns, then resizes the window to 800 and 400 and prints widths; 657.5 prints widths after sizeToFit.
- What each outcome means for the PR: Hidden columns keep their widths and the extra width is shared as the style says over the shown columns only: keep. A hidden column changes: change.

### G4: A range returned by columnsInRect: may contain a hidden column between shown ones (it is a contiguous range); callers skip it.

- Prints: 657.1 (columnsInRect of the full width) and 657.2 (seven partial rectangles, including ones that start or end inside a hidden column) print the range as first..last.
- What each outcome means for the PR: Hidden column 2 inside the range of 1..3: keep. The range is split or the hidden column is excluded at the ends of the range: change the rule for ranges whose end falls on a hidden column.

Also: 657.9 compares headerView columnAtPoint: with the table's columnAtPoint: with columns 1 and 3 hidden; 657.7/657.8 hide and show a column after layout; 657.10 column selection of a hidden column. Bonus, not GUESS items.

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

- https://developer.apple.com/documentation/appkit/nstablecolumn/ishidden
- https://developer.apple.com/documentation/appkit/nstableview/rect(ofcolumn:)
- https://developer.apple.com/documentation/appkit/nstableview/columns(in:)
- https://developer.apple.com/documentation/appkit/nstableview/sizelastcolumntofit()
- https://developer.apple.com/documentation/appkit/nstableview/columnautoresizingstyle

## What this probe cannot answer

The probe uses a cell-based table (data source, no view-based delegate). A view-based table is out of scope of this PR.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Until that is done this item stays: macOS result: NOT YET OBSERVED.

# Probe 661: NSTableColumn fontBold and fontItalic bindings

**macOS result: NOT YET OBSERVED**

Pull request: darling-cocotron #661. Window server: yes (cells are prepared in a table in an alpha 0 titled window, never key, no activation).

Public API only, written from the public documentation and from the wording of the GUESS items, not from the PR code. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into behavioural tests (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. Do not commit the probe to Darling (AGENTS.md section 6).

## GUESS items and what the probe prints

### G1: A value of NO takes the trait off the font again; the data cell of a column serves every row, so a row that is not bold must not keep the bold of the row before.

- Prints: 661.1 to 661.3 prepare cells row by row (values YES, NO, absent, NSNull, 1, 0, 2, "YES", the three markers) in order, then in reverse, then again; 661.4 prepares each row first in a fresh table; 661.7/661.8 mix bold and italic values in both orders. Each line prints the font name, size, bold and italic trait flags of the prepared cell.
- What each outcome means for the PR: A row with NO has bold=0 regardless of the earlier rows: keep. A row after a YES row keeps bold, or bold=0 only in one visiting order: the rule differs (the font accumulates). A NO row with an explicitly bold column font (661.9) staying bold or becoming non-bold shows whether NO removes the trait from the column font itself.

### G2: A cell without a font is left alone and answers NO.

- Prints: 661.12 to 661.14 set the data cell font to nil before binding and print the font before and after row preparation.
- What each outcome means for the PR: Font nil stays nil (or the default) for every row: keep. A bold system font appears for a bold YES row: change (the binding supplies a base font).

### G3: A nil value (a row without the key), NSNull and the selection markers ask for no trait.

- Prints: 661.1 to 661.4 rows 3 to 11 (absent key, NSNull, markers, numbers 0/1/2, a string) and 661.5/661.6 with only one binding.
- What each outcome means for the PR: bold=0 italic=0 for those rows, independent of the previous row: keep. A marker or NSNull producing a trait, or keeping the previous row's trait: change.

Also: 661.15 is a model change followed by reloadData; 661.0 is the unbound baseline font. Not covered because not observable by value: the fontName, fontSize and fontFamilyName binding names (the PR lists them as BLOCKED-NEEDS-VALUES): the probe uses only NSFontBoldBinding and NSFontItalicBinding, which are public.

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

- https://developer.apple.com/documentation/appkit/nsbindingname/fontbold
- https://developer.apple.com/documentation/appkit/nsbindingname/fontitalic
- https://developer.apple.com/documentation/appkit/nstablecolumn
- https://developer.apple.com/documentation/appkit/nstableview/preparedcell(atcolumn:row:)

## What this probe cannot answer

The string values of the fontName, fontSize and fontFamilyName binding-name constants: they are constants, not behaviour, and no probe in this set prints them (a one-line NSLog of the public constants on a Mac would settle it; they are public names, so this is allowed, but they are outside this PR's GUESS wording).

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Until that is done this item stays: macOS result: NOT YET OBSERVED.

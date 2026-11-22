# Probe 641: NSPopUpButton content, contentObjects and contentValues bindings

**macOS result: NOT YET OBSERVED**

Pull request: darling-cocotron #641. Window server: yes (NSPopUpButton needs AppKit and a window-server connection; no window is created, nothing is shown, no activation).

Public API only, written from the public documentation and from the wording of the GUESS items, not from the PR code. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into behavioural tests (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. Do not commit the probe to Darling (AGENTS.md section 6).

## GUESS items and what the probe prints

### G1: The object is kept as the item's representedObject (the reference does not name the property).

- Prints: 641.1 to 641.6 print every item's representedObject and title for content only, content + contentValues, contentObjects + contentValues, all three, content + contentObjects, and a contentValues array shorter than content.
- What each outcome means for the PR: representedObject = content object (641.2) or contentObjects element (641.3/641.4): keep. representedObject nil, or something else: change which property carries the object. 641.4 shows which of content and contentObjects wins as represented object.

### G2: Equal descriptions keep separate items.

- Prints: 641.7 (equal strings), 641.8 (number 1, string "1", number 1) and 641.9 (equal titles from contentValues) print numberOfItems and titles.
- What each outcome means for the PR: n equals the content count: keep. Fewer items (duplicates merged, as -addItemWithTitle: does): change to merge equal titles.

### G3: A selectedObject that is not in the content selects nothing.

- Prints: 641.14 (model.sel = "zzz"), 641.15 (nil) and 641.19 (a value that is a content item while contentObjects exist) print indexOfSelectedItem and titleOfSelectedItem.
- What each outcome means for the PR: selIndex -1 and no title: keep. The first item or the previous item stays selected: change. 641.18/641.19 also show whether selectedObject compares against content or against contentObjects.

### G4: After a rebuild the bound selection (the first of selectedObject, selectedValue, selectedTag, selectedIndex that is bound) is put back without writing to the model; a content change no longer overwrites the model's selection. With no selection binding the first item is selected.

- Prints: 641.12/641.13 (selectedObject), 641.21 (selectedValue), 641.22 to 641.24 (selectedIndex) and 641.25 (selectedTag) print selection and the writes the model received (the "writes=[...]" part) across content changes. 641.30 to 641.34 do a content change with no selection binding.
- What each outcome means for the PR: "writes=[]" after a content change and selection restored: keep. Writes such as model.sel<-... appear: macOS writes back, change. Selection after 641.30 to 641.34 other than index 0 (for example the old index kept): change the no-binding rule.

### G5: GUESS: a bound selection survives a content change, and which selection binding wins if several are bound.

- Prints: 641.12/641.13 (survival) and 641.26 to 641.29: four selection bindings with disagreeing model values, bound in two opposite orders, then a content change. The selected item shows the winner; the writes show whether the losers were overwritten.
- What each outcome means for the PR: Same winner for both bind orders and it is selectedObject: keep. A different fixed winner: change the priority list. Winner depends on bind order: the "first bound" rule is wrong or needs the bind order.

Also: 641.35 (pull-down popup: is the first content item the title item) and 641.36 (NSNull and dictionary elements in content) are bonus observations, not GUESS items.

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

- https://developer.apple.com/documentation/appkit/nspopupbutton
- https://developer.apple.com/documentation/appkit/nsbindingname/content
- https://developer.apple.com/documentation/appkit/nsbindingname/contentvalues
- https://developer.apple.com/documentation/appkit/nsbindingname/contentobjects
- https://developer.apple.com/documentation/appkit/nsbindingname/selectedobject

## What this probe cannot answer

Whether the reference "does not name the property" is a documentation statement, not observable. The popup-state side of G1 is observable (representedObject), but the name the reference uses for it is not.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Until that is done this item stays: macOS result: NOT YET OBSERVED.

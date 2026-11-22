# Probe 0056: runOperationModalForWindow callback contract; help calls with no help book

**macOS result: NOT YET OBSERVED**

Purpose: Part 1 prints when and how the delegate of runOperationModalForWindow:... is called for a panel-less save-to-PDF operation. Part 2 (with --ui) lists windows that appear for help calls without a help book.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

Part 1: a print operation for an empty view with job disposition "save" to a temporary PDF, no print panel, no progress panel, run with `runOperationModalForWindow:nil delegate:didRunSelector:contextInfo:`. The probe prints whether the delegate selector was called, before or after the method returned, how many times, the arguments, and whether the file was written. Part 2 (`./probe --ui`): `-[NSApplication showHelp:]`, `-[NSHelpManager openHelpAnchor:inBook:]` and `-[NSHelpManager findString:inBook:]` with no help book in the bundle; a timer lists this process's windows (class, title, text) one second in and ends any modal session.

## Why it matters, and Darling code affected

Darling already has `runOperationModalForWindow:...` (calls the delegate after running), so part 1 pins its contract. Darling's help manager calls are unimplemented; what a real application does there is observable.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSPrintOperation.m:498-506 (runOperationModalForWindow:delegate:didRunSelector:contextInfo:) [darling-cocotron origin/master cd700cad]
- AppKit/NSHelpManager.m:69-77 (findString:inBook: and openHelpAnchor:inBook: call NSUnimplementedMethod) [darling-cocotron origin/master cd700cad]
- AppKit/NSDisplay.m:478 and AppKit/DarlingDisplay.m:338 (runModalPrintPanelWithPrintInfoDictionary: is abstract / unimplemented) [darling-cocotron origin/master cd700cad]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./probe            # part 1 only
./probe --ui       # also part 2: may open windows or another application
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| Delegate called once, after the method returned or before, with success YES and the context pointer | Confirms or corrects the order Darling uses at NSPrintOperation.m:503. |
| Delegate called later on the run loop | Darling calls synchronously; a general ordering fix. |
| Help calls open a window or a modal alert inside the process | A no-help alert is what macOS shows; Darling can show an equivalent instead of raising. |
| Help calls open another application or do nothing inside the process | Nothing to copy; an unimplemented-method exception is worse than either. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsprintoperation/runmodal(for:delegate:didrun:contextinfo:)
- https://developer.apple.com/documentation/appkit/nshelpmanager
- https://developer.apple.com/documentation/appkit/nsprintoperation

## What this probe cannot answer

What a print panel offers and how it is worded (a user-interface design choice, not an API contract). The "save as PDF only" panel stays a maintainer decision.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

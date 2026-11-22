# Probe 0350: Document application launch: untitled document or Open panel, and open-file with no delegate method

**macOS result: NOT YET OBSERVED**

Purpose: Builds two tiny document apps (with and without NSUbiquitousContainers) and prints, 2 s after launch, the windows and documents that exist for each combination of the user default and the delegate answer, with and without a file argument.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

Two bundles are built from the same source: `plain` declares one document type; `ubiq` also has an NSUbiquitousContainers dictionary. `run.sh` starts each executable with every combination of `-NSShowAppCentricOpenPanelInsteadOfUntitledFile` (unset, YES, NO), the delegate's answer to `-applicationShouldOpenUntitledFile:` (YES, NO), and, in an extra run, one file argument. The delegate implements neither `-application:openFile:` nor `-application:openFiles:`. After 2 s the probe prints the list of windows (NSOpenPanel or document windows) and the documents of the shared document controller.

## Why it matters, and Darling code affected

The patch shows the Open panel instead of an untitled document when the bundle declares ubiquity containers, with a user-default override, and makes files go to the document controller when the delegate lacks the open-file methods. The default key and the trigger are not in the public documentation; the observable result is whether an Open panel or an untitled document exists after launch.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSApplication.m:1191-1219 (_openFileNames: routes files to the shared document controller when the delegate lacks -application:openFile:; already done) [darling-cocotron origin/master cd700cad]
- AppKit/NSApplication.m:1306-1331 (_openUntitledFileIfNeeded: asks the delegate, then always newDocument: for document apps; no Open panel path) [darling-cocotron origin/master cd700cad]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./run.sh
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

Run with `./run.sh` (run-all.sh does). Each case runs about 2 seconds and may flash a panel or a window.

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| plain: untitled document; ubiq: Open panel (no document) | The trigger is real and general (a property of the bundle, not of an app); the patch direction is right, and the default key behaves as described if YES/NO override it. |
| Both variants show an untitled document, whatever the default | No such behaviour for this combination; do not port. (A real iCloud container may be needed on a signed app; then say so in the evidence and treat the claim as unverified.) |
| The default YES/NO changes the outcome only in one variant | Records the precedence between default and bundle key. |
| The delegate answer NO still shows the Open panel | The panel does not consult applicationShouldOpenUntitledFile:, matching the patch comment. |
| A file argument opens a document with no delegate method | Confirms what Darling NSApplication.m:1207-1213 already does. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsapplicationdelegate/applicationshouldopenuntitledfile(_:)
- https://developer.apple.com/documentation/appkit/nsapplicationdelegate/application(_:openfile:)
- https://developer.apple.com/documentation/appkit/nsdocumentcontroller
- https://developer.apple.com/documentation/bundleresources/information-property-list/cfbundledocumenttypes

## What this probe cannot answer

NSUbiquitousContainers and NSShowAppCentricOpenPanelInsteadOfUntitledFile have no public documentation page that the author could find; their meaning rests on this observation alone. An unsigned bundle may not behave like a signed iCloud app.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

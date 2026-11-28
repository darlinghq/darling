# Probe item-e: When ATSApplicationFontsPath fonts become registered for the process

**macOS result: NOT YET OBSERVED**

Purpose: Prints, at each launch stage (constructor, start of main, after NSApplication creation, will/did finish launching, first main-queue turn), whether the bundled font is registered in the process scope and where its PostScript name resolves.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

An app bundle whose Info.plist has `ATSApplicationFontsPath` = `Fonts` and one font file in `Contents/Resources/Fonts`. At each stage the probe prints the number of process-scope registered font descriptors (macOS 12 and later) and how many are the bundled font, and whether a lookup of its PostScript name resolves into the bundle or elsewhere. `run.sh` runs the bundle executable directly and through LaunchServices (`open`).

## Why it matters, and Darling code affected

Darling registers application fonts from this key at a point of its own choosing (Onyx2D/O2Font_freetype.m reads the key). Whether a real process has the font before `main`, before NSApplication exists, in `applicationWillFinishLaunching:` or only later affects apps that look fonts up early.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- Onyx2D/O2Font_freetype.m:209 and AppKit/Win32.subproj/Win32Display.m:136 (read the key) [darling-cocotron origin/master cd700cad]
- tests/coretext-font-manager-register-url.m:46-71 (test: a font in ATSApplicationFontsPath is registered at launch) [darling-cocotron origin/master cd700cad]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./run.sh
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

Build and run: `sh build.txt && ./run.sh`.

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| Registered already at "start of main()" (before NSApplication) | Registration happens during process or bundle setup; Darling must register before application code runs, not on the first font lookup. |
| Registered only after "+[NSApplication sharedApplication]" or later | AppKit startup does it; Darling may register there. |
| Direct exec and LaunchServices launch differ | The launch path matters; record which Darling mimics. |
| Never registered in the process scope but lookup by name resolves into the bundle | macOS resolves the key lazily through the font lookup; no explicit registration step to mirror. |

## Public documentation

- https://developer.apple.com/documentation/bundleresources/information-property-list/atsapplicationfontspath
- https://developer.apple.com/documentation/coretext/ctfontmanagerscope
- https://developer.apple.com/documentation/coretext/ctfontmanagercopyregisteredfontdescriptors(_:_:)

## What this probe cannot answer

The process-scope list needs macOS 12 or later; on older releases only the lookup result is printed. The font file is a copy of any font on the test Mac (set PROBE_FONT), not committed.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

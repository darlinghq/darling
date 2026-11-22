# Probe 0044: NSAlert layout, button key equivalents, icons and legacy return codes

**macOS result: NOT YET OBSERVED**

Purpose: Builds NSAlert panels in several configurations without showing them and prints window size, title bar, icon, text, accessory, suppression checkbox and button frames and key equivalents.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

`-layout` is called on alerts with no text, with a message, with a long informative text and three buttons, with a suppression checkbox, an accessory view, long button titles and the help button. The probe prints the window frame, whether it has a title bar, the style mask, the icon size, and for every subview its class (public base class only), frame, font and string; for buttons also key equivalent, modifier mask and tag. It prints the alert style versus the icon, the legacy and modern return constants, and whether `+imageNamed:NSImageNameApplicationIcon` is the same object as `-applicationIconImage`.

## Why it matters, and Darling code affected

The patch re-lays out the alert from numbers measured elsewhere. Window width, margins, icon size, button heights and the default and cancel key equivalents are all observable through public API on a real macOS. Darling today builds a titled panel and lays out with its own constants.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSAlert.m:94 (panel created with NSTitledWindowMask), :303 (-layout), :576 and :571-580 (sheet style mask) [darling-cocotron origin/master cd700cad]
- AppKit/NSPanel.m:86 (NSRunAlertPanel and the legacy alert functions) [darling-cocotron origin/master cd700cad]
- AppKit/NSApplication.m:679 (-applicationIconImage) [darling-cocotron origin/master cd700cad]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./probe
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| First button has key equivalent Return, "Cancel" has Escape, "Don't Save" has Command-D | Public, observable defaults; Darling can adopt them as a general rule (NSAlert addButtonWithTitle: in NSAlert.m). |
| Alert window has no title bar on the macOS you run | Darling NSAlert.m:94 builds a titled panel; the difference is visible to any app. |
| Window width, margins, button heights | Numbers for a version-scoped layout; compare with the constants at NSAlert.m:303. |
| Application icon: imageNamed equals applicationIconImage | Darling NSImage lookup should return the application icon for that name; tiny general fix. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsalert
- https://developer.apple.com/documentation/appkit/nsalert/layout()
- https://developer.apple.com/documentation/appkit/nsalert/icon

## What this probe cannot answer

Anything about an alert that is actually on screen (placement, animation, sheet attach), and the decision to adopt one release's look.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

# Probe item-f: NSMenu effectiveAppearance with and without appearances on parents and on the application

**macOS result: NOT YET OBSERVED**

Purpose: Prints appearance and effectiveAppearance names for a three-level menu hierarchy and a detached menu across ten settings of menu and application appearances.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

Menus main, sub, subsub (linked through submenu items) and a detached menu. Ten steps: nothing set; dark on main; dark on main and aqua on sub; main reset; dark on subsub only; dark on the application only; dark on the application and aqua on main; same with main as the application's main menu; everything cleared; sub removed from main. After each step the probe prints `appearance` and `effectiveAppearance` names for the four menus and the application's effective appearance. The properties are read through key-value coding so the probe also builds against an older SDK.

## Why it matters, and Darling code affected

The question is the fallback of a submenu with no appearance of its own: does it take the supermenu's appearance, the application's, or the system one? Darling has no `effectiveAppearance` for NSMenu at all.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSMenu.subproj/NSMenu.m: no effectiveAppearance or appearance property found by search [darling-cocotron origin/master cd700cad]
- AppKit/NSAppearance.m:141 and AppKit/NSApplication.m:772-800 (application and window effective appearance change notification) [darling-cocotron origin/master cd700cad]

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
| sub follows main (step 2) and subsub follows sub | Supermenu fallback exists; Darling needs a chain walk through -supermenu. |
| A submenu ignores a supermenu and follows the application | No supermenu fallback; the patch's fallback would be wrong. |
| A detached menu reports the application appearance | Fallback for menus with no supermenu is the application. |
| Step 9 (main menu with everything cleared) | Shows whether the main menu has a different fallback than other menus. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsappearancecustomization/effectiveappearance
- https://developer.apple.com/documentation/appkit/nsappearancecustomization/appearance
- https://developer.apple.com/documentation/appkit/nsmenu

## What this probe cannot answer

The patch behind this item was not read; the probe tests the stated question only. Menus that are on screen (context menus, status item menus) may follow other rules; they are not probed.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

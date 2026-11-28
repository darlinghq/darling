# Probe 0108: Does an NSMenuItem getter keep its result alive past the item (autorelease behaviour)?

**macOS result: NOT YET OBSERVED**

Purpose: With manual reference counting, prints whether the object a getter returned is still alive after the item is released but before the autorelease pool drains.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

For title, keyEquivalent, attributedTitle, image, representedObject and toolTip, twice each: once with a call to the getter before the item is released, once without (control). A tracker object attached to the value (associated object) reports when the value is deallocated. Output: whether the value was freed before the pool drained, and after, plus whether the getter returned the very object that was set.

## Why it matters, and Darling code affected

Atomic properties return a retained and autoreleased value; nonatomic ones return the ivar. A caller that keeps a value past the item's life without retaining it works with the first kind only. Darling's own header declares these as nonatomic, so only the observed behaviour of the original can settle whether the getters should autorelease.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSMenu.subproj/NSMenuItem.m:268 (-title), :305 (-keyEquivalent), :313 (-image), :329 (-representedObject): getters return the ivar [darling-cocotron origin/master cd700cad]
- AppKit/include/AppKit/NSMenuItem.h:85 and :94 (title and keyEquivalent declared nonatomic, copy) [darling-cocotron origin/master cd700cad]

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
| With the getter called, the value is NOT freed before the pool drains; control line shows it freed at once | The getter retains and autoreleases; Darling returning the ivar differs observably (a use-after-free for callers that rely on it). |
| Freed before the drain in both runs | macOS returns the ivar like Darling; no change. |
| The getter returns a different object than was set (copy) | The value is a copy; the test needs to look at the copy, not the original. Read 0108.getter lines with that in mind. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsmenuitem
- https://developer.apple.com/documentation/appkit/nsmenuitem/title
- https://developer.apple.com/documentation/appkit/nsmenuitem/representedobject

## What this probe cannot answer

Whether atomic semantics are worth the cost for all getters; the probe only shows what macOS does.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

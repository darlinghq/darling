# Probe 646: precise scrolling deltas and gesture phase on scroll events

**macOS result: NOT YET OBSERVED**

Pull request: darling-cocotron #646. Window server: no window is made and nothing is posted; NSApplication is created for the connection only (no activation).

Public API only, written from the public documentation and from the wording of the GUESS items, not from the PR code. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into behavioural tests (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. Do not commit the probe to Darling (AGENTS.md section 6).

## GUESS items and what the probe prints

### G1: The unit of precise scrolling deltas: the documentation only says to scroll by the returned amount when hasPreciseScrollingDeltas is true and gives no unit.

- Prints: 646.1 creates pixel and line scroll events with CGEventCreateScrollWheelEvent2 (9 delta pairs, both units) and prints hasPreciseScrollingDeltas and scrollingDeltaX/Y of the NSEvent from +eventWithCGEvent:. 646.5 to 646.8 set the point-delta and fixed-point fields alone and in disagreement, which shows which CGEvent field the NSEvent reads. 646.0 prints the screen backing scale factor so points and pixels can be told apart.
- What each outcome means for the PR: Pixel input N gives scrollingDeltaY = N (precise YES): the unit is points as the creator gave them; keep storing what the backend passes. A scaled value (N / backing scale, N * scale, or another factor): the backend must convert to that unit before storing. Line input gives precise NO and a line-unit scrollingDeltaY: shows what unmarked events look like.

### G2: A marked event's deltaX/deltaY are left as the creator set them (the docs do not say what they are for precise events).

- Prints: 646.1 prints deltaX, deltaY, deltaZ next to scrollingDeltaX/Y for pixel and line events; 646.5/646.6/646.7/646.8 show how deltaX/deltaY follow the fields.
- What each outcome means for the PR: deltaY equals the pixel value or the creator-set value: keep. deltaY is a line-like value (the pixel value divided by a factor, rounded, or 0): change to derive deltaX/deltaY from the precise deltas.

### G3: momentumPhase stays none (nothing in Darling produces momentum phases); also the phase of a synthesised event.

- Prints: 646.9 sets the scroll-phase field (Began, Changed, Ended, Cancelled, MayBegin, 0) for pixel and line events, 646.10 sets the momentum phase field, 646.11 sets both, 646.1 (no phase set) shows the default. Prints phase and momentumPhase of the NSEvent as names and raw bits.
- What each outcome means for the PR: phase None and momentumPhase None for events without phase fields: keep. A synthesised pixel event without phase fields reporting a non-None phase: change. Phases that follow the set fields: the NSEvent accessors read the CGEvent fields, so a backend that wants phases must fill them. Momentum fields that do not reach momentumPhase: momentumPhase None is correct.

Also: Also printed for context: isDirectionInvertedFromDevice, windowNumber and the CGEvent fields read back from the NSEvent (round trip), and scrollCount (646.12).

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

- https://developer.apple.com/documentation/appkit/nsevent/hasprecisescrollingdeltas
- https://developer.apple.com/documentation/appkit/nsevent/scrollingdeltay
- https://developer.apple.com/documentation/coregraphics/cgeventcreatescrollwheelevent2(_:_:_:_:_:_:)
- https://developer.apple.com/documentation/appkit/nsevent/phase
- https://developer.apple.com/documentation/appkit/nsevent/momentumphase

## What this probe cannot answer

The Wayland axis semantics (surface-local lengths in the protocol) cannot be observed on macOS: that concerns the Wayland PR, not this probe. A real trackpad produces events no program can synthesise through public API with exact hardware values; the probe sees only what the public CGEvent fields round-trip.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Until that is done this item stays: macOS result: NOT YET OBSERVED.

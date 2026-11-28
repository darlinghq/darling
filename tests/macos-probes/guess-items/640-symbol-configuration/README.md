# Probe 640: NSImageSymbolConfiguration factories and configurationByApplyingConfiguration:

PR: darlinghq/darling-cocotron#640 (AppKit: add NSImageSymbolConfiguration text-style and rendering-preference factories).

**macOS result: NOT YET OBSERVED**

Public API only, written from the public documentation and from the wording of the PR's GUESS items. No private selectors, no private symbols, no Apple binary, header dump or trace was used. The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture. It was derived from the GUESS text, not from the PR's code, so it can also show macOS doing something the PR did not expect. Nothing here claims a Mac result; the Darling guest ran nothing.

Window server: yes for drawing symbol images into a bitmap (no window is created, no activation; it needs a GUI session). Needs macOS 11 or later (SF Symbols); `+configurationPreferringMonochrome` and the palette and multicolor factories exist from macOS 12. Unsupported selectors print `n/a`. The probe uses the multicolor, hierarchical symbol `cloud.sun.rain.fill`.

Configurations have no public getters, so every result is observed through the image it produces: `size=` of the image, `ink=` (pixels with alpha at least 128 in a rendering under the Aqua appearance), `colours=` (distinct colours at 3 bits per channel) and `top=` (the four most common, as `rgb<r><g><b>:<percent>`).

## The GUESS items and what the probe prints for each

1. "When both configurations carry a rendering preference, the argument's replaces all of the receiver's" -> `640.C.preference`: every receiver x argument pair of {size 64 only, monochrome, hierarchical, multicolor, palette(r,g,b), palette(purple)}, then size 64 applied so the symbol is large. Compare each row with the matching single row in `640.B.alone`. Also `640.C.keeps-preference` (a size-only argument keeps the receiver's preference?) and `640.C.adds-preference`.
   - Row equals the argument's single row for every pair: keep. A pair that keeps something of the receiver (for example receiver palette plus argument hierarchical showing palette colours, or multicolor then monochrome staying coloured) shows preferences are merged per field: change.
2. "A text style in the argument replaces the receiver's point size and weight, and a point size in the argument replaces the receiver's text style" -> `640.D.alone` (seven single configurations) and `640.D.size-weight-style` (all 49 receiver x argument pairs of: point 40 bold, point 20 light, title1, body, caption2 with large scale, scale-large only, weight-heavy only), `640.A.textStyle`, `640.A.textStyleScale` for the per-style sizes.
   - Result equal to the argument's single row for the style/size pairs: keep. Result keeping the receiver's weight when the argument is a text style, or the receiver's text style when the argument sets a point size: change. A scale-only or weight-only argument over a text-style receiver shows which fields are independent.
3. "The placeholder symbol is not resized from a text style (point sizes per style are not documented)" -> not observable: macOS has no placeholder symbol. `640.A.textStyle` prints real point-size effects per style (image size and ink), which would settle the point sizes if Darling later wants them.
4. "Not encoded to archives: no archive keys are documented" -> `640.F.round-trip`: secure-coding archive and decode of seven configurations; prints whether it encodes, and the before and after image rows and `isEqual`. (The archive contents are not read.)
   - Round trip preserving the rendering: macOS encodes the state, so not encoding is a visible gap (and keys remain a decision for Darling). Round trip losing state or failing: not encoding matches.
5. "init(colorRenderingMode:) and init(variableValueMode:) are not done: BLOCKED-NEEDS-VALUES (enum values not in Darling's headers)" -> `640.G.*`: the factories called with raw values 0 to 3 so the effect of each value is visible; read the enumerator names and values from the Mac's own `NSImageSymbolConfiguration.h` (macOS 26 and later) and match them to the rows. Rows say `selector n/a or nil` on older systems.
6. Equality and identity (not GUESS, but used by the PR's tests) -> `640.E.identity`, `640.E.equality`, `640.E.nil-argument`: whether `isEqual:` works between configurations, whether applying returns a new object, applying nil.

## Apple documentation

- https://developer.apple.com/documentation/appkit/nsimage/symbolconfiguration-swift.class
- https://developer.apple.com/documentation/appkit/nsimage/symbolconfiguration-swift.class/init(textstyle:)
- https://developer.apple.com/documentation/appkit/nsimage/symbolconfiguration-swift.class/applying(_:)
- https://developer.apple.com/documentation/appkit/nsimage/symbolconfiguration-swift.class/preferringmonochrome()

## How to run

Copy this directory to a Mac with the stock Xcode command line tools, then:

```sh
sh build.txt
./probe > results.txt
```

Paste the `PROBE` lines back. Nothing in the output contains user names, host names or paths. Do not commit the probe to Darling: it is an observation tool, not product code (Darling AGENTS.md section 6).

Syntax check done on Linux only: `clang -fsyntax-only -target x86_64-apple-macos11 -isysroot <Darling SDK>`. It parses with no stubs; every symbol-configuration selector is called by name at run time, and the font text-style constants are declared `extern` in the probe.

## What this probe cannot answer

Exact pixel layouts of symbols (the signature is coarse on purpose); the symbol set of the Darling guest.

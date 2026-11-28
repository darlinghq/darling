# Probe 0215: CTFontCreateCopyWithAttributes on a font made from data: does the copy keep the same face?

**macOS result: NOT YET OBSERVED**

Purpose: Creates a CGFont from font file bytes (not installed), a CTFont from it, and prints identity, size, matrix and metrics of copies made with size, matrix, feature settings, another name and bold traits.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

`./probe [font.ttf]` (default: the file of a stock system font found through `kCTFontURLAttribute`). Copies: size only, matrix only, feature settings only, size in the descriptor, another font name, bold trait. For each: PostScript name, size, matrix, whether the copy's graphics font is the same object or equal to the original, glyph and advance of "A", glyph count; plus `CTFontCreateWithFontDescriptor` on the data font's own descriptor.

## Why it matters, and Darling code affected

The patch makes a copy that changes only size, matrix or rendering attributes keep the actual face, because a font created from memory does not exist in any registry a lookup by name could use. The public contract says copy attributes override the original; which attributes count as identity-changing is observable.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- CoreText/CTFont.m:234-264 (CTFontCreateCopyWithAttributes: copies keep the graphics font when the attributes carry no name, via createFont(copyGraphicsFont); a name attribute goes through CTFontCreateWithName) [darling-cocotron origin/master cd700cad]
- CoreText/CTFont.m:916 (CTFontCreateWithGraphicsFont), :169 (CTFontCreateWithFontDescriptor) [darling-cocotron origin/master cd700cad]

## How to run

Copy this directory to a Mac with the stock Xcode command line tools (macOS 10.14 or newer), then:

```sh
sh build.txt
./probe            # or: ./probe /path/to/any/single-face-font.ttf
```

`build.txt` holds one command line (run it from this directory). `probe_common.h` is a small output helper shared by all probes. Nothing in the output contains paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code (AGENTS.md section 6).

## What each outcome would mean for the Darling change

| Observation | Meaning |
|---|---|
| Size, matrix and feature-settings copies are sameCGFont/equalCGFont and keep the PostScript name | Darling master (CTFont.m:259-264) already behaves this way for those; the patch is mostly redundant. |
| A copy whose descriptor holds only traits (bold) keeps the same face on macOS | Darling would need to keep the face for trait-only attributes; today it ignores traits entirely. |
| A copy with another name switches face | Documented; Darling CTFont.m:243-254 already handles it. |

## Public documentation

- https://developer.apple.com/documentation/coretext/ctfontcreatecopywithattributes(_:_:_:_:)
- https://developer.apple.com/documentation/coretext/ctfontcreatewithgraphicsfont(_:_:_:_:)
- https://developer.apple.com/documentation/coregraphics/cgfont

## What this probe cannot answer

Nothing known; every item is an API-visible value or behaviour.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

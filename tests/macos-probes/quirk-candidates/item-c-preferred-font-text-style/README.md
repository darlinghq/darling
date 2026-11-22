# Probe item-c: NSFont preferredFontForTextStyle sizes and weights

**macOS result: NOT YET OBSERVED**

Purpose: Prints point size, name, weight, traits and metrics of the font for each text style, plus the system font sizes and related class methods.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

For the eleven `NSFontTextStyle` values: the string value of the constant, `+preferredFontForTextStyle:options:` (font name, family, point size, weight trait, symbolic traits, ascender, descender, leading, cap height, x height, font bounding box), and the matching font descriptor. Also `+systemFontSize`, `+smallSystemFontSize`, `+labelFontSize`, `+systemFontSizeForControlSize:` for three sizes, the message, menu, title bar, tool tips and control content fonts, and an unknown style string.

## Why it matters, and Darling code affected

The patch behind this item was not available to the author; the probe covers the public question: which size and weight each text style gets on a macOS release. Darling has the constants (as UICTFontTextStyle strings) but no `preferredFontForTextStyle:options:`.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- AppKit/NSFontDescriptor.m:268 (NSFontTextStyleBody = @"UICTFontTextStyleBody", constants only) and AppKit/include/AppKit/NSFontDescriptor.h:171 [darling-cocotron origin/master cd700cad]
- AppKit/NSFont.m:214 (+systemFontSizeForControlSize:); no +preferredFontForTextStyle:options: found by search [darling-cocotron origin/master cd700cad]

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
| Sizes match the Human Interface Guidelines typography table on this macOS | The documented table can be implemented; the probe supplies the numbers and weights to pin. |
| Sizes differ between macOS releases | Version-scoped table. |
| Constants print as other strings than Darling's UICTFontTextStyle values | Constant values should be corrected where apps compare them. |
| Unknown style returns nil or the body font | Pins the fallback. |

## Public documentation

- https://developer.apple.com/documentation/appkit/nsfont/preferredfont(fortextstyle:options:)
- https://developer.apple.com/documentation/appkit/nsfontdescriptor/preferredfontdescriptor(fortextstyle:options:)
- https://developer.apple.com/design/human-interface-guidelines/typography

## What this probe cannot answer

Nothing; every item is a number. Built against an SDK before macOS 11 the style section is skipped with a message.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

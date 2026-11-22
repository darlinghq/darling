# Probe item-d: CGPDFObjectType enumerator values, and the type reported for known PDF objects

**macOS result: NOT YET OBSERVED**

Purpose: Prints each kCGPDFObjectType enumerator by its public name and the type CGPDFObjectGetType reports for objects of a small PDF the probe writes.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

The nine enumerators are printed by name and value. The probe then writes a one-page PDF into memory, parses it and prints the type of the catalog /Type (a name), /Pages (dictionary), /Count (integer), /Kids (array), page /MediaBox (array) and an element of it (number), /Contents (stream), /Resources (dictionary), an absent key, and the info /Title (string).

## Why it matters, and Darling code affected

Darling's internal enumeration starts at 1 and counts up (null, boolean, integer, real, name, string, array, dictionary, stream); the public header must carry the same numbers because applications can compare or print them. Darling's SDK tree has no public CGPDFObjectType at all.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- Onyx2D/include/Onyx2D/O2PDFObject.h:29-38 (kO2PDFObjectTypeNull = 1 followed by eight enumerators in order) [darling-cocotron origin/master cd700cad]
- CoreGraphics/include/CoreGraphics/CGPDFObject.h (no CGPDFObjectType declared; none found by grep in CoreGraphics/) [darling-cocotron origin/master cd700cad]

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
| Values are 1 to 9 in the order printed | Darling O2PDFObjectType already has the right numbers; declare the public enum with the same values. |
| Any value differs from Darling's order | Darling's internal numbers need a mapping layer. |
| Real page numbers report integer vs real types | Pins when a number is an integer. |

## Public documentation

- https://developer.apple.com/documentation/coregraphics/cgpdfobjecttype
- https://developer.apple.com/documentation/coregraphics/cgpdfobjectgettype(_:)

## What this probe cannot answer

Nothing. Against Darling's SDK tree the probe does not compile as-is because CGPDFObjectType and the CGPDF accessors are not declared there.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

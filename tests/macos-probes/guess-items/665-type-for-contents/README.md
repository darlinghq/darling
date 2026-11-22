# Probe 665: typeForContentsOfURL:error: for an unrecognised type

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #665. Window server: needs a window server connection only (NSApplication; no window; no activation). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: The error code, NSFileReadUnknownError, when the type is unrecognised.

- Printed: The probe relaunches itself in two throw-away app bundles (b1 no document types, b2 several) and prints type or nil plus error domain, code, localized description, failure reason, user-info keys and underlying error, for declared and undeclared extensions, upper case, no extension, directories, missing files, non-file URLs, a fragment, and nil URL (exception name only).
- Outcomes: Domain NSCocoaErrorDomain and code 256 (NSFileReadUnknownError): keep. Another code (for example NSFileReadUnsupportedSchemeError 262, NSFileReadNoSuchFileError 260, NSFileReadCorruptFileError 259) or a different domain: change to what is observed, per kind of input. Whether a type is returned at all for undeclared extensions (not nil) would show a different flow.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

The bundle is created under the temporary directory and removed afterwards; paths are not printed. Needs the stock toolchain only (NSTask to run the copy of the probe).

## Public documentation

- https://developer.apple.com/documentation/appkit/nsdocumentcontroller/typeforcontents(of:)
- https://developer.apple.com/documentation/foundation/nsfilereadunknownerror

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

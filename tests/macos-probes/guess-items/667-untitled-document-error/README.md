# Probe 667: makeUntitledDocumentOfType:error: when no document class

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #667. Window server: needs a window server connection only (NSApplication; no window; no activation). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: The error code (NSFileReadUnknownError) when there is no document class.

- Printed: Two child bundles: b1 has no CFBundleDocumentTypes; b2 declares ok, missing-class, not-a-document-class, init-fails-with-error, init-fails-without-error and no-class-key types. For every declared type, an undeclared name, nil and the empty string it prints the document, the error (domain, code, description, user-info keys), whether a pre-filled error marker survived, the NULL-error behaviour, and openUntitledDocumentAndDisplay:error:.
- Outcomes: NSCocoaErrorDomain/256: keep. Any other code or domain (for example NSFileWriteUnknownError, a different domain, or an exception for nil): change. Marker "survived" together with a non-nil document shows the error is only written on failure.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

The method is missing from Darling's SDK tree headers; a scratch stub declaration was used for the syntax check (see INDEX.md).

## Public documentation

- https://developer.apple.com/documentation/appkit/nsdocumentcontroller/makeuntitleddocument(oftype:)
- https://developer.apple.com/documentation/appkit/nsdocumentcontroller

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

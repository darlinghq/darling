# Probe 670: NSDocumentController defaultType with different CFBundleDocumentTypes

**macOS result: NOT YET OBSERVED**

PR: darling-cocotron #670. Window server: needs a window server connection only (NSApplication; no window; no activation). Arguments: none.

Public API only, written from the public documentation and from the wording of the GUESS items; no private selectors or symbols, no Apple binary or header was inspected, and the PR code was not read. The probe prints one fact per line as `PROBE <id> <input> -> <output>`; its first line gives the macOS version and CPU architecture. Nothing in the output holds paths, user names or host names. Do not commit the probe to Darling: it is an observation tool, not product code.

## GUESS items and what the probe prints

### GUESS: An Editor entry with neither a content type nor a name is passed over for the next Editor entry.

- Printed: Variants v08, v09 (empty name), v10 (content type only), v11 (name only), v12 (only such an entry): `670.child ... defaultType`.
- Outcomes: defaultType is the next entry's name in v08/v09 and nil/another in v12: keep. defaultType is nil, empty or the nameless entry's class/UTI: change.

### GUESS: An entry with no CFBundleTypeRole is not an Editor.

- Printed: Variants v04 (no role then Editor), v05 (only a no-role entry), v06 (role None), v07 (role Shell); plus Viewer-only v03 and v13 and order v01/v02.
- Outcomes: v04 gives the Editor entry's name and v05 gives nil (or the Viewer is skipped): keep. v04/v05 gives the no-role entry's name: change (no role counts as Editor). v03/v13 giving a Viewer name shows macOS falls back to non-Editors, which the PR does not do.

## How to run

```sh
sh build.txt
./probe
```

`build.txt` is one command line for the stock Xcode command line tools. `probe_common.h` (and `probe_bundle.h` where present) are small helpers copied next to the probe. The probe does not activate the application and keeps windows at alpha 0 or never orders them in.

## Notes

Each variant is a throw-away bundle built by the probe itself, so no plist files are needed. Also prints documentClassNames, per-type class, display name and extensions, and the class made by makeUntitledDocumentOfType: for the default type.

## Public documentation

- https://developer.apple.com/documentation/appkit/nsdocumentcontroller/defaulttype
- https://developer.apple.com/documentation/bundleresources/information_property_list/cfbundledocumenttypes

## Where to put the result

Record macOS version, build and architecture, the exact `PROBE` lines, and the date. Until then this item stays: macOS result: NOT YET OBSERVED.

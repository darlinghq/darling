# Probe 0109: ImageIO metadata: enum values, string constants, tag and container behaviour, thumbnails

**macOS result: NOT YET OBSERVED**

Purpose: Prints the values behind the public CGImageMetadata and thumbnail names and exercises tag creation, path set/get/remove, metadata read from a JPEG written by the probe, and thumbnail size rules.

Public API only, written from the public documentation. No private selectors, no private symbols, no Apple binary was inspected. The output is meant to be captured by a human on a real Mac and turned into a behavioural test (Darling AGENTS.md section 3, black-box observation; section 7, record inputs, outputs, OS version). The probe prints one fact per line as `PROBE <id> <input> -> <output>`, and its first line gives the macOS version and CPU architecture.

## Behaviour observed

Constant values by public name: the eight `kCGImageMetadataType*` enumerators, the thumbnail option strings, `kCGImageDestinationMetadata`, `kCGImageDestinationMergeMetadata`, all namespace and prefix strings. Behaviour: tag creation with a registered namespace, with type Default for string and array values, with an unregistered namespace, with a NULL namespace; set/get/remove by path; the order of `CGImageMetadataCopyTags`; string value of an array tag; a 40 by 20 JPEG written with orientation, an EXIF date and a TIFF make and read back with `CGImageSourceCopyMetadataAtIndex`; thumbnail sizes for max pixel 10 and 100 with and without the transform option.

## Why it matters, and Darling code affected

The task was blocked on the numeric values and key strings, which are public but absent from Darling's headers. The probe prints them by name so the values come from a Mac, not from memory.

Darling files (current master at the time of reading; line numbers will move). Statements about Darling behaviour come from reading the source; nothing was run on Darling.

- darling monorepo origin/master 5541b379: src/frameworks/ImageIO/include/ImageIO/CGImageDestination.h:5-7 (typedefs) and :36-38 (CGImageMetadataTagCreate takes a bare int type); no kCGImageMetadataType enumerators anywhere
- darling monorepo origin/master 5541b379: src/frameworks/ImageIO/src/CGImageDestination.m:54-75 (limited metadata stubs; tag is the value itself)
- darling monorepo origin/master 5541b379: src/frameworks/ImageIO/src/CGImageSource.m:11-20 and :89-103 (thumbnail options and CGImageSourceCreateThumbnailAtIndex already present)
- darling monorepo origin/master 5541b379: src/frameworks/ImageIO/include/ImageIO/CGImageProperties.h:45-46 (only the Exif namespace and prefix constants)

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
| The eight enumerator values printed | These are the values to declare in CGImageMetadata.h; they unblock the task (no values are known to this author). |
| Namespace and prefix strings printed | Declare them with these exact strings. |
| Tag with unregistered namespace returns NULL | Darling must refuse the same way; otherwise a general behaviour difference. |
| CopyTags order and array-tag string value | Pin Darling tag containers to this order and rule. |
| Thumbnail sizes | Confirm the already-present Darling thumbnail code (CGImageSource.m:89) rounds and orients the same way. |

## Public documentation

- https://developer.apple.com/documentation/imageio/cgimagemetadatatype
- https://developer.apple.com/documentation/imageio/cgimagemetadata
- https://developer.apple.com/documentation/imageio/cgimagemetadatatagcreate(_:_:_:_:_:)
- https://developer.apple.com/documentation/imageio/cgimagesourcecreatethumbnailatindex(_:_:_:)

## What this probe cannot answer

Nothing; every item is a value or a public-API behaviour. Compile note: against Darling's SDK tree this probe does not compile as-is because those names are missing, which is the gap.

## Where to put the result

Record macOS version, build and architecture, the exact output lines, and the date. Do not paste more than the relevant `PROBE` lines. Until that is done this item stays: macOS result: NOT YET OBSERVED.

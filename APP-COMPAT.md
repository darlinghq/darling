# Cocoa API surface

General API compatibility fixes for documented Cocoa API that Darling's SDK
was missing. No change is keyed to an application's name, bundle ID or version.

## Changes

- AppKit: added the documented `NSToolbarItemValidation` protocol and
  `NSSound.delegate` / `NSSoundDelegate` callback, stored out of line so the
  legacy `NSSound` ivar layout is unchanged.
- QuickLookUI: added the shared preview panel entry points, data source and
  delegate protocols, and the item-list behavior apps use to populate a
  preview panel.
- CoreImage: added the pixel-buffer and orientation entry points used by
  video rotation code, and made the CoreImage context declaration visible
  through AppKit's CoreImage re-export.
- ImageIO: added the `CGImagePropertyOrientation` enum and made its header
  safely re-includable.

The AppKit additions are in darling-cocotron.

## Validation

Host-side checks (no Darling prefix required):

```sh
python3 tests/sdk-clang-modules.py
python3 tests/app-api-compat.py
```

`tests/app-api-compat.py` compiles source-level probes of public API usage.

Runtime check for the AppKit additions (needs a Darling prefix):

```sh
clang -fno-objc-arc -framework AppKit -framework Quartz \
    tests/appkit-app-api-compat.m -o /tmp/appkit-app-api-compat
darling shell /tmp/appkit-app-api-compat
```


## Remaining gaps

- `AVPictureInPictureController` and its delegate protocol are not declared yet.
- The CoreImage additions are declarations over Darling's stub-forwarding
  CIImage and CIContext classes; they compile but do nothing at run time.
- `NSSound` playback is unimplemented, so `-sound:didFinishPlaying:` is never
  sent; the delegate is only stored.

## Provenance

The fixes follow the public API contracts: Apple's published framework headers
and developer documentation for AppKit, Quick Look, Core Image, Core Video and
Image IO, and the EXIF/TIFF orientation tag values. No application code was copied into Darling, and no Apple binary,
disassembly, leaked source or other restricted material was used.

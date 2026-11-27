# VLC, Transmission and Godot compatibility work

This branch collects general API compatibility fixes found while checking the
macOS source of VLC, Transmission and Godot against Darling's SDK. No changes
are keyed to an application's name, bundle ID or version: the apps were used
only to find API surface that Darling was missing.

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

The AppKit additions live in [darling-cocotron#959](https://github.com/darlinghq/darling-cocotron/pull/959);
this superproject branch deliberately leaves `src/external/cocotron` at the pin
from upstream `master`. The maintainer's `Update submodules` action moves the
pin after that change lands upstream. The host checks skip the AppKit probes
until the pinned submodule contains them.

## Validation

Host-side checks (no Darling prefix required):

```sh
python3 tests/sdk-clang-modules.py
python3 tests/app-api-compat.py
```

`tests/app-api-compat.py` compiles source-level probes derived from the three
applications' public API usage. It does not build or run the applications.

Runtime check for the AppKit additions, once a Darling prefix is available and
the submodule pin includes the AppKit change (or the checked-out submodule is
the change's branch):

```sh
clang -fno-objc-arc -framework AppKit -framework Quartz \
    tests/appkit-app-api-compat.m -o /tmp/appkit-app-api-compat
darling shell /tmp/appkit-app-api-compat
```

A real VLC Apple video-output source file
(`modules/video_output/apple/VLCSampleBufferDisplay.m`) was also checked with
`-fsyntax-only` against this SDK.

## Remaining gaps observed

- VLC's Picture-in-Picture module needs `AVPictureInPictureController` and its
  delegate protocol, which this SDK does not declare yet.
- iOS-only VLC modules need UIKit; they are outside this macOS branch.
- No guest runtime validation was possible here because no built Darling prefix
  was available.

## Provenance

The fixes follow the public API contracts: Apple's published framework headers
and developer documentation for AppKit, Quick Look, Core Image, Core Video and
Image IO. The VLC, Transmission and Godot source trees were inspected only to
identify which public APIs were called and which declarations were missing;
no application code was copied into Darling, and no Apple binary, disassembly,
leaked source or other restricted material was used.

# UIKit hello window

An authored public-API probe for the UIKit-on-AppKit bridge: an application
delegate, one hosting window, one root view controller and one label. It proves
that `#import <UIKit/UIKit.h>` resolves from a Darling SDK, that an iOS-shaped
translation unit compiles and links against Darling's UIKit, and that a real
compositor shows the label's pixels.

Provenance: Apple's public `UIApplicationDelegate`, `UIWindow`,
`UIViewController`, `UILabel` and `UIScreen` documentation (clean-room rung 3).
The fixture is hand-written; it is not a template, an `.ipa`, or copied
application code. It exercises no private framework, no account, no user data
and no Keychain.

## Scope

| Exercised | Deliberately absent |
|---|---|
| `UIApplicationMain`, delegate launch callback | touch, gestures, `UIEvent` dispatch |
| `UIWindow` hosting, `makeKeyAndVisible` | scenes, storyboards, nib loading |
| `UIView` tree, flipped coordinates | Auto Layout, constraints, layer animation |
| `UILabel` text, font, colour | `UIButton`, `UITextField`, scrolling |
| `UIScreen` bounds and scale | rotation, status bar, multitasking |
| bounded run loop, clean terminate | background execution, `open`/LaunchServices |

## Prerequisites: the SDK must compile

A checkout's committed `MacOSX.sdk` does not compile `Foundation/Foundation.h`
or `AppKit/AppKit.h` until the header submodules are populated and three
frameworks get their header entries (their `Versions/A` directories are empty in
the committed tree, so `#import <Security/Security.h>`,
`<IOKit/hidsystem/IOLLEvent.h>` and `<CFNetwork/CFNetwork.h>` fail):

```sh
# 1. Header submodules (transitively; include their nested submodules too)
git submodule update --init --recursive \
  src/external/foundation src/external/corefoundation src/external/objc4 \
  src/external/Libc src/external/libplatform src/external/libmalloc \
  src/external/libclosure src/external/libdispatch src/external/libpthread \
  src/external/xnu src/external/cctools src/external/security \
  src/external/IOKitUser src/external/cfnetwork
git -C src/external/cfnetwork submodule update --init --recursive || true
git -C src/external/IOKitUser submodule update --init --recursive || true
git -C src/external/corefoundation submodule update --init --recursive || true

# 2. Restore the framework header entries the committed SDK is missing
misc/sdk-add-source-framework.sh Security   src/external/security/darling/include/macOS/Security
misc/sdk-add-source-framework.sh IOKit      src/external/IOKitUser/darling/include/IOKit
misc/sdk-add-source-framework.sh CFNetwork  src/external/cfnetwork/include/CFNetwork

# 3. UIKit is IOSSUPPORT, i.e. it lives under System/iOSSupport
misc/sdk-add-source-framework.sh UIKit src/frameworks/UIKit/include/UIKit A \
  System/iOSSupport/System/Library/Frameworks
```

The script writes exactly what `cmake/generate_sdk_framework()` writes, so a
scratch `-DREGENERATE_SDK=ON` run can be diffed against it instead of committing
a regenerated 2,896-entry tree.

## Build

Compile against the SDK alone; no source-tree include paths are needed:

```sh
SDK="$PWD/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
clang -target arm64-apple-macos11.0 -fobjc-arc \
  -isysroot "$SDK" \
  -F "$SDK/System/Library/Frameworks" \
  -F "$SDK/System/iOSSupport/System/Library/Frameworks" \
  -F "$SDK/System/Library/Frameworks/CoreServices.framework/Frameworks" \
  -I "$SDK/usr/include" \
  tests/uikit-hello-window.m -o uikit-hello-window \
  -framework UIKit -framework AppKit -framework Foundation \
  -framework CoreGraphics -framework objc
```

Link against the *built* UIKit (`<build>/src/frameworks/UIKit/UIKit`) rather than
the install-name stub when running from a build tree; both are Mach-O images and
the load path is the only difference.

## Run

Stage under a physical `opt/uikit-probes` directory in a fresh private prefix,
then use the same owned headless compositor recipe as
`tests/appkit-launch-lifecycle.md`:

```sh
DARLING_APPKIT_BACKEND=wayland darling shell /opt/uikit-probes/uikit-hello-window
```

## Acceptance

1. The process prints `OK: didFinishLaunching, window ordered, label frame …`
   and `OK: fixture timer fired; requesting termination`, then exits 0.
2. Before the timer expires, the owned compositor's `get_tree` shows exactly one
   native `xdg_shell` surface titled `uikit-hello-window` and no X11 window.
3. A scale-1 screenshot of that output contains the label's glyph pixels inside
   the label rect `(40,40)-(360,88)` of the window content, measured as in
   `tests/appkit-launch-lifecycle.md`: at least 50 non-background pixels, with
   their bounding box inside the rect. Window chrome, placement and any
   compositor-specific decoration are not fixed coordinates.
4. No `Stub called:` line appears on the launch, layout, draw or terminate path.

## Measured baseline

Compile stage verified 2026-10-19 against the committed SDK plus the entries
above: the umbrella, the fixture and all ten UIKit sources type-check with
`-Wall`-clean output, and codegen produces `Mach-O 64-bit arm64` objects whose
fixture imports are `UIApplicationMain`, `NSTimer`, `NSApp`, the five UIKit
classes and the usual `objc`/`CF` runtime symbols.

Run stage is unverified: no built runtime was available in the authoring
environment, so items 1–4 of the acceptance list are open until a build exists.

## What this proves and does not

It proves one window-and-label path through Darling's UIKit on a real
compositor. It does not prove iOS application compatibility, does not make
`.ipa` bundles runnable, and does not cover scenes, storyboards, touch, layout
or animation. `UIView` is an `NSView` subclass and exposes `NSView`'s `-window`
rather than an iOS `UIWindow *`; treat that as a documented deviation until the
bridge grows a compatible accessor.

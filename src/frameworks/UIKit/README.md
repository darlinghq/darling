# UIKit compositional layout values

This first component implements documented layout dimension, size, spacing and
edge-spacing factories and getters, including NSCopying. These objects describe
layout inputs. No collection view renderer or application/window lifecycle is
implemented by this component.

Specification: Apple public documentation (clean-room rung 3):

- https://developer.apple.com/documentation/uikit/nscollectionlayoutdimension?language=objc
- https://developer.apple.com/documentation/uikit/nscollectionlayoutsize?language=objc
- https://developer.apple.com/documentation/uikit/nscollectionlayoutspacing?language=objc
- https://developer.apple.com/documentation/uikit/nscollectionlayoutedgespacing?language=objc

Darling rejects nonfinite layout values, negative dimensions, missing size
dimensions, incorrectly typed edges, and bare init/new. This validation is a
conservative boundary policy (rung 6); Apple's exact invalid-input behavior has
not been observed. Finite negative spacing remains representable. Newer uniform
sibling dimensions and layout items/groups/sections are not implemented here.

`tests/src/uikit_layout_values.m` looks the classes up at run time and loads
the installed UIKit from its iOSSupport path, so without UIKit it fails at the
missing classes instead of at link time. It uses synthetic values only.
`tests/src/uikit_hello_window.m` is an iOS-shaped application (delegate, window,
view controller, label) that needs a display.

The framework is installed under /System/iOSSupport and is built for the 64-bit
architectures only.
No private UIKit contracts or guessed constant values are supplied.

The Objective-C getters on `NSCollectionLayoutSpacing` are `isFixedSpacing` and
`isFlexibleSpacing`, the names Apple documents for Objective-C (`isFixed` and
`isFlexible` are the Swift spellings and must not be added as selectors).

NSCollectionLayout* are also class names in AppKit on macOS 10.15 and later. If
Darling's AppKit ever defines them, the two definitions must be merged rather
than loaded side by side.

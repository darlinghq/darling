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

`tests/layout-values.m` uses runtime class lookup so the same executable can run
against the baseline without UIKit: it fails at the missing classes. Pass the
absolute guest path of the built UIKit dylib to exercise factories and copying.
The test uses synthetic values only and does not load Messages or user data.

The framework is built by the gui component group in src/frameworks/CMakeLists.txt.
No private UIKit contracts or guessed constant values are supplied.

The Objective-C getters on `NSCollectionLayoutSpacing` are `isFixedSpacing` and
`isFlexibleSpacing`, the names Apple documents for Objective-C (`isFixed` and
`isFlexible` are the Swift spellings and must not be added as selectors).

Focused build (use independently cloned dependencies at the superproject pins):

```sh
python3 \
  src/frameworks/UIKit/tests/build-layout-values.py \
  --source-root . --runtime-root "$DARLING_RUNTIME_ROOT" \
  --build-dir "$UIKIT_BUILD_DIR" --linker "$DARLING_DARWIN_LINKER"
```

The runtime root is the read-only `libexec/darling` image directory. This command
compiles one test and one framework serially, saves exact commands, and neither
installs artifacts nor configures the shared build. Run the resulting harness
through Darling in a fresh sanitized prefix, first with no argument (expected
failure), then with the UIKit path (expected success). `--arch` (default `arm64`) must match the
runtime image. With the path given, the test also asserts each class's image name
is `UIKit.framework/Versions/A/UIKit`.

To check the CMake-built product exports the classes (not covered by the focused
build above), after a normal build:

```sh
nm -gU <build>/src/frameworks/UIKit/UIKit | rg 'OBJC_CLASS_\$_NSCollectionLayout(Dimension|Size|Spacing|EdgeSpacing)$'
```
